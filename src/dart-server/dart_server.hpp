#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <memory>

#include <boost/property_tree/ptree.hpp>

#include "dart-server/installation.hpp"
#include "dart-server/job_storage.hpp"
#include "dart-server/gspc_interface.hpp"

#include "rest/router.hpp"
#include "http/server.hpp"


struct server_description
{
  std::string name;
  std::string host;
  unsigned short port;

  std::string key;
  std::unique_ptr<http::client> client;
};

enum class authorization_level
{
  all,
  client,
  server,
  worker
};

class dart_server final : public http::server
{
private:
  using combine_function = std::function<void(http::client*, const std::vector<rest::message>&)>;
public:
  dart_server(net::io_pool& pool, const boost::property_tree::ptree& config, const installation& install);

  /**
  * Handles incoming messages.
  *
  * @param client  the client that send the message
  * @param message the message itself
  */
  virtual void handle_message(http::client* client, const http::message& message) override;

  void run();

  void shutdown(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void get_information(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);

  void post_job(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void get_job(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void stop_job(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void post_tasks(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void get_job_status(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void get_results(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void delete_result(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);

  void add_worker(http::client* client, const rest::message& message, const rest::symbols& symbols);
  void delete_worker(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
  void get_worker(http::client* client, const rest::message& message, const rest::symbols& symbols, const std::vector<rest::message>& peer_responses);
 
  bool is_authorized(http::client* client, const rest::message& request, authorization_level level);

  inline auto& peers() noexcept { return _peers; }
private:
  std::string _hostname;

  installation _install;
  boost::asio::signal_set _signals;

  rest::router _router;
  std::atomic<bool> _should_shutdown;
  std::vector<server_description> _peers;
  std::unique_ptr<gspc_interface> _gspc;
  std::unique_ptr<job_storage> _storage;
};