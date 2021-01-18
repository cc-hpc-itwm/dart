#include "dart-server/dart_server.hpp"

#include "dart-server/gspc_interface.hpp"
#include "dart-server/job_storages/ram_storage.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <thread>
#include <regex>

#include "log/log.hpp"

namespace
{
  rest::message unauthorized()
  {
    boost::property_tree::ptree content;
    content.put("error.code", 1);
    content.put("error.message", "unauthorized request");
    content.put("error.category", "invalid request");
    return rest::message(http::status_code::Unauthorized_401, content);
  }

  rest::message internal_error(const boost::system::error_code& code)
  {
    boost::property_tree::ptree content;
    content.put("error.code", code.value());
    content.put("error.message", code.message());
    content.put("error.category", code.category().name());
    return rest::message(http::status_code::InternalServerError_500, content);
  }

  rest::message internal_error(const http::status_code& state, int code, const std::string& message, const std::string& category)
  {
    boost::property_tree::ptree content;
    content.put("error.code", code);
    content.put("error.message", message);
    content.put("error.category", category);
    return rest::message(state, content);
  }

  using handler_t = void(dart_server::*)(http::client*, const rest::message&, const rest::symbols&, const std::vector<rest::message>&);

  void broadcast(http::client* client, const rest::message& message, 
    const rest::symbols& symbols, dart_server* server, 
    handler_t handler, void(*prequel)(dart_server*))
  {
    // Todo check if the infrastructure is up or if we are in
    // a locked state, if so wait until unlock or reply a timeout
    if (message.content().get("broadcast", true))
    {
      if (prequel)
        prequel(server);

      if (!server->is_authorized(client, message, authorization_level::client))
        client->send_message(unauthorized());

      auto new_message = message;
      new_message.content().put("broadcast", false);

      if (server->peers().size() == 0)
      {
        (server->*handler)(client, new_message, symbols, {});
        return;
      }
      auto* responses = new std::vector<rest::message>(server->peers().size());
      auto* counter = new std::atomic<unsigned long>(server->peers().size());

      for (unsigned i = 0; i < server->peers().size(); ++i)
      {
        server->peers()[i].client->send_message(new_message,
          [i, responses, counter, new_message, server, handler, client, symbols]
          (http::client* c, const http::message& response)
          {
            if (!response.is_response())
              c->send_message(http::message(http::status_code::BadRequest_400));

            boost::property_tree::ptree content;
            if (response.get_body().size() > 0)
            {
              try
              {
                std::stringstream stream;
                for (auto& c : response.get_body())
                  stream << c;
                boost::property_tree::json_parser::read_json(stream, content);
              }
              catch (boost::property_tree::json_parser_error&) {};
            }

            (*responses)[i] = rest::message(response.get_status_code(), content);

            if (counter->fetch_sub(1) == 1) // If the counter is zero, i.e., everything processed
            {
              (server->*handler)(client, new_message, symbols, *responses);
              delete responses;
              delete counter;
            }
          });
      }
    }
    else
    {
      if (!server->is_authorized(client, message, authorization_level::server))
        client->send_message(unauthorized());

      if (prequel)
        prequel(server);

      (server->*handler)(client, message, symbols, {});
    }
  }

  struct error
  {
    unsigned http_code;
    int error_code;
    std::string error_category;
    std::string error_message;
  };

  std::vector<error> extract_errors(const std::vector<rest::message>& responses)
  {
    std::vector<error> errors;
    for(auto& res : responses)
    {
      if (res.status().code() >= 400 && res.status().code() <= 599)
      {
        errors.push_back(error{
          res.status().code(),
          res.content().get("error.code", 0),
          res.content().get("error.category", ""),
          res.content().get("error.message", "")
          });
      }
    }
    return errors;
  }

  rest::message create_response(const std::vector<error>& errors)
  {
    boost::property_tree::ptree content;
    std::vector<int> codes;
    boost::property_tree::ptree errors_tree;
    for (auto& err : errors)
    {
      boost::property_tree::ptree error_tree;
      if (err.http_code != 0)
      {
        codes.push_back(err.http_code);
      }
      error_tree.put("code", err.error_code);
      error_tree.put("category", err.error_category);
      error_tree.put("message", err.error_message);
      errors_tree.add_child("", error_tree);
    }
    content.add_child("errors", errors_tree);

    if (codes.size() == 1)
      return rest::message(http::status_code(codes[0]), content);
    if (codes.size() == 0)
      return rest::message(http::status_code::InternalServerError_500, content);

    for (auto& code : codes)
      content.add("response_codes", code);
    return rest::message(http::status_code::InternalServerError_500, content);
  }
}

dart_server::dart_server(net::io_pool& pool, const boost::property_tree::ptree& config, const installation& install)
: http::server(pool, config.get<unsigned short>("port", 0), install.certs())
, _install(install)
, _signals(pool.get_service())
, _router()
, _should_shutdown(false)
, _gspc(std::make_unique<gspc_interface>(_install, config))
, _storage(std::make_unique<job_storages::ram>())
{
  _signals.add(SIGINT);
  _signals.add(SIGTERM);
#ifdef SIGQUIT
  _signals.add(SIGQUIT);
#endif
  _signals.async_wait([this](boost::system::error_code, int) { _should_shutdown.store(true); });

  // TODO: Read server topology
#define broadcast_handler( name ) \
  std::bind(&broadcast, std::placeholders::_1, std::placeholders::_2, \
  std::placeholders::_3, this, &dart_server:: name , nullptr)

#define broadcast_handler_prequel( name , prequel) \
  std::bind(&broadcast, std::placeholders::_1, std::placeholders::_2, \
  std::placeholders::_3, this, &dart_server:: name , prequel )

  _router.add_resource("/server/", "DELETE", 
    broadcast_handler_prequel(shutdown, [](dart_server*) {/*Set restart policy to false*/}));
  _router.add_resource("/server/", "GET", broadcast_handler(get_information));

  _router.add_resource("/job/", "POST", broadcast_handler(post_job));
  _router.add_resource("/job/{job}/", "GET", broadcast_handler(get_job));
  _router.add_resource("/job/{job}/", "DELETE", broadcast_handler(stop_job));
  _router.add_resource("/job/{job}/tasks/", "POST", broadcast_handler(post_tasks));
  _router.add_resource("/job/{job}/status/", "GET", broadcast_handler(get_job_status));
  _router.add_resource("/job/{job}/results/", "GET", broadcast_handler(get_results));
  _router.add_resource("/job/{job}/results/{result}/", "DELETE", broadcast_handler(delete_result));

  _router.add_resource("/worker/", "POST", 
    [this](http::client* client, const rest::message& message, const rest::symbols& symbols)
    {
      this->add_worker(client, message, symbols); // For now do not broadcast, later load balance
    });
  _router.add_resource("/worker/", "DELETE", broadcast_handler(delete_worker));
}

/**
* Handles incoming messages.
*
* @param client  the client that send the message
* @param message the message itself
*/
void dart_server::handle_message(http::client* client, const http::message& message)
{
  try
  {
    log_message::info("[handle_message] " + message.get_method());
    _router.handle_request(client, message);
  }
  catch (const boost::system::error_code & error)
  {
    log_message::error("[dart_server::handle_message] client '"
      + client->host_and_port()
      + "' produced error: "
      + error.message());

    client->send_message(internal_error(error));
  }
}

void dart_server::run()
{
  _should_shutdown.store(false);

  http::server::run();
  
  while (!_should_shutdown.load())
  {
    //log_message::info("[dart_server::run] fetching results");
    auto results = _gspc->fetch_available_results();
    for (auto& result : results)
      _storage->add_result(result);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void dart_server::shutdown(http::client* client, const rest::message& message, const rest::symbols&, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }
  _should_shutdown = true;

  client->send_message(rest::message(http::status_code::OK_200, {}));
}

void dart_server::get_information(http::client* client, const rest::message& message, const rest::symbols&, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  boost::property_tree::ptree content;
  for (auto& response : responses)
  {
    if (!response.content().empty())
      content.add_child("servers", response.content());
  }

  boost::property_tree::ptree my_content;
  my_content.put("host", boost::asio::ip::host_name());
  content.get_child("servers").add_child("", my_content);

  client->send_message(rest::message(http::status_code::OK_200, content));
}


void dart_server::post_job(http::client* client, const rest::message& message, const rest::symbols&, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  // Read job definition
  auto body = message.content();

  auto name = body.get("name", "");

  job_config config;
  config.path_to_module_or_module_content = body.get("module", "");
  config.is_path = body.get("is_module_path", false);
  config.method = body.get("method", "");
  try
  {
    _storage->add_job(name, config);
  }
  catch (std::runtime_error & exc)
  {
    client->send_message(internal_error(http::status_code::InternalServerError_500, 0, exc.what(), ""));
  }

  boost::property_tree::ptree content;
  for (auto& response : responses)
  {
    if (!response.content().empty())
      content.add_child("results", response.content());
  }
  
  client->send_message(rest::message(http::status_code::OK_200, content));
}

void dart_server::post_tasks(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  // Read tasks definition
  std::string job = "";
  if (symbols.find("job") != symbols.end())
    job = symbols.at("job");
  else
  {
    client->send_message(internal_error(http::status_code::BadRequest_400, -1,
      "must specify job name", ""));
    return;
  }

  auto [status, config] = _storage->get_job(job);
  if (status == job_status::unknown)
  {
    client->send_message(rest::message(http::status_code::NotFound_404));
    return;
  }

  auto body = message.content();

  std::list<std::pair<std::string, std::string>> location_and_parameters;

  for (auto& location_and_parameter : body.get_child("location_and_parameters"))
  {
    std::string location = location_and_parameter.second.get("location", "");
    std::string parameter = location_and_parameter.second.get("parameter", "");
    location_and_parameters.push_back({ location, parameter });
  }
  std::string msg;
  for (auto& p : location_and_parameters)
    msg.append("(" + p.first + ", " + p.second + ")\n");
  log_message::info("[dart_server::post_tasks] starting with "+ msg);
  _gspc->start_job(job, config, location_and_parameters);

  log_message::info("[dart_server::post_tasks] start_job returned");
  boost::property_tree::ptree content;
  for (auto& response : responses)
  {
    if (!response.content().empty())
      content.add_child("results", response.content());
  }

  client->send_message(rest::message(http::status_code::OK_200, content));
}

void dart_server::get_job(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  std::string job = "";
  if (symbols.find("job") != symbols.end())
    job = symbols.at("job");
  else
  {
    client->send_message(internal_error(http::status_code::BadRequest_400, -1,
      "must specify job name", ""));
    return;
  }

  if (_storage->get_status(job) == job_status::unknown)
  {
    client->send_message(rest::message(http::status_code::NotFound_404));
    return;
  }

  boost::property_tree::ptree content;

  auto j = _storage->get_job(job);

  content.put("job.id", job);
  content.put("job.status", static_cast<unsigned>(j.first));
  content.put("job.config.module", j.second.path_to_module_or_module_content);
  content.put("job.config.is_module_path", j.second.is_path);
  content.put("job.config.method", j.second.method);

  client->send_message(rest::message(http::status_code::OK_200, content));
}

void dart_server::stop_job(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  std::string job = "";
  if(symbols.find("job") != symbols.end())
    job = symbols.at("job");
  else
  {
    client->send_message(internal_error(http::status_code::BadRequest_400, -1,
      "must specify job name", ""));
    return;
  }

  if (_storage->get_status(job) == job_status::unknown)
  {
    client->send_message(rest::message(http::status_code::NotFound_404));
    return;
  }

  _storage->set_status(job, job_status::stopped);
  _gspc->cancel_job(job);

  client->send_message(rest::message(http::status_code::OK_200));
}

void dart_server::get_job_status(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  std::string job = "";
  if (symbols.find("job") != symbols.end())
    job = symbols.at("job");
  else
  {
    client->send_message(internal_error(http::status_code::BadRequest_400, -1,
      "must specify job name", ""));
    return;
  }

  if (_storage->get_status(job) == job_status::unknown)
  {
    client->send_message(rest::message(http::status_code::NotFound_404));
    return;
  }

  auto state = _storage->get_status(job);
  // The state should be the same for every other server
  for (auto& res : responses)
  {
    if (res.content().get("status", 0u) != static_cast<unsigned>(state))
    {
      // This should not happen
      client->send_message(internal_error(http::status_code::InternalServerError_500, -1,
        "status different for different server", ""));
      return;
    }
  }
  boost::property_tree::ptree content;
  content.put("job.name", job);
  content.put("job.status", static_cast<unsigned>(state));
  client->send_message(rest::message(http::status_code::OK_200, content));
}

void dart_server::get_results(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  std::string job = "";
  if (symbols.find("job") != symbols.end())
    job = symbols.at("job");
  else
  {
    client->send_message(internal_error(http::status_code::BadRequest_400, -1,
      "must specify job name", ""));
    return;
  }

  if (_storage->get_status(job) == job_status::unknown)
  {
    client->send_message(rest::message(http::status_code::NotFound_404));
    return;
  }

  auto amount = message.content().get("amount", 0);
  auto worker_regex = message.content().get("worker_regex", "");

  boost::property_tree::ptree content;
  for (auto& res : responses)
  {
    auto results = res.content().get_child("results");
    for (auto r : results)
    {
      content.add_child("results", r.second);
      --amount;
      if (amount == 0)
        break;
    }
    if (amount == 0)
      break;
  }

  if (amount > 0)
  {
    try
    {
      auto results = _storage->get_results(job, amount, worker_regex);
      boost::property_tree::ptree results_content;
      for (auto r : results)
      {
        boost::property_tree::ptree result_content;
        result_content.put("id", r.first);
        result_content.put("job", r.second.job);
        result_content.put("worker", r.second.worker);
        result_content.put("start_time", r.second.start_time);
        result_content.put("duration", r.second.duration);
        if (r.second.error != "")
          result_content.put("error", r.second.error);
        else
          result_content.put("success", r.second.success);
        results_content.push_back({ "", result_content });
      }
      content.add_child("results", results_content);
    }
    catch (const std::regex_error&)
    {
      client->send_message(internal_error(http::status_code::BadRequest_400, -1,
        "invalid regular expression", ""));
      return;
    }
  }
  content.put("job.id", job);
  content.put("job.status", static_cast<unsigned>(_storage->get_status(job)));
  client->send_message(rest::message(http::status_code::OK_200, content));
}

void dart_server::delete_result(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>&)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  std::string job = "";
  if (symbols.find("job") != symbols.end())
    job = symbols.at("job");

  std::string result = "";
  if (symbols.find("result") != symbols.end())
    result = symbols.at("result");

  _storage->delete_result(job, result);
  client->send_message(rest::message(http::status_code::OK_200));
}


void dart_server::add_worker(http::client* client, const rest::message& message, const rest::symbols&)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  std::vector<std::string> hosts;
  unsigned workers_per_host = message.content().get("workers_per_host", 1);
  std::vector<std::string> capabilities;
  unsigned shm_size = message.content().get("shm_size", 0);

  for (auto& iter : message.content().get_child("hosts"))
  {
    auto host = iter.second.get("", "");
    if(host != "")
      hosts.push_back(host);
  }

  for (auto& iter : message.content().get_child("capabilities"))
  {
    auto capability = iter.second.get("", "");
    if (capability != "")
      capabilities.push_back(capability);
  }


  try
  {
    std::string msg = "\n  hosts (";
    for (auto& host : hosts) msg += host + ",";
    msg += "\n  capabilities (";
    for (auto& cap : capabilities) msg += cap + ",";

    msg += ")\n  workers_per_host (" + std::to_string(workers_per_host) + ")\n  "
      + "shm_size (" + std::to_string(shm_size) + ")";
    log_message::info("[dart_server::add_worker] " + msg);
    _gspc->add_workers(hosts, workers_per_host, capabilities, shm_size);
  }
  catch (std::runtime_error & exception)
  {
    client->send_message(internal_error(http::status_code::InternalServerError_500, 0, exception.what(), ""));
    return;
  }

  client->send_message(rest::message(http::status_code::OK_200));
}


void dart_server::delete_worker(http::client* client, const rest::message& message, const rest::symbols&, const std::vector<rest::message>& responses)
{
  if (!is_authorized(client, message, authorization_level::client))
  {
    client->send_message(unauthorized());
    return;
  }

  auto errors = extract_errors(responses);
  if (errors.size() != 0)
  {
    // TODO better error treatment...
    client->send_message(create_response(errors));
    return;
  }

  std::vector<std::string> hosts;

  for (auto& iter : message.content().get_child("hosts"))
  {
    auto host = iter.second.get("", "");
    if (host != "")
      hosts.push_back(host);
  }

  try
  {
    std::string msg = "\n  hosts (";
    for (auto& host : hosts) msg += host + ",";
    msg += ")";
    log_message::info("[dart_server::delete_worker] " + msg);
    _gspc->remove_workers(hosts);
  }
  catch (std::runtime_error & exception)
  {
    client->send_message(internal_error(http::status_code::InternalServerError_500, 0, exception.what(), ""));
    return;
  }

  client->send_message(rest::message(http::status_code::OK_200));
}


bool dart_server::is_authorized(http::client* client, const rest::message& request, authorization_level level)
{
  (void)client;
  (void)request;
  (void)level;

  return true;
}