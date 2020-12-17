#include "rest/router.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace rest;

router::router()
 : _root(std::make_unique<tree>())
{
}

/**
* Adds a resource to the api.
*
* The path has to have the form [/]<part1>/<part2>/.../<partN>[/]. Each part is either
* a fixed string or of the form {<name>}. If it is of the form {<name>}, the server
* matches any string to this location and saves its match in a resolved_symbols map
* that is passed to the handler.
*
* Example 1: path = /hallo/index.html
*  Only requests on the resource /hallo/index.html get redirected to the handler
* Example 2: path = /hallo/{name}/
*  Requests like /hallo/dart/ and /hallo/gspc/ get redirected to the handler
*
* @param path    the path to the resource
* @param method  the request method that should be redirected to the handler
* @param handler the handler
*/
void router::add_resource(std::string path, const std::string& method,
  const handler& handler)
{
  auto i = path.find_last_not_of('/');
  path = path.erase(i + 1);
  size_t start = path.find_first_of('/', 0) + 1;
  size_t end = path.find_first_of('/', start);
  auto node = _root.get();
  while (end != path.npos)
  {
    auto part = path.substr(start, end + 1 - start);
    if (part != "")
    {
      auto child = get_child(*node, part, false);
      if (child)
        node = child;
      else
      {
        node->children.insert({ part, std::make_unique<tree>() });
        node = node->children[part].get();
      }
    }
    start = end + 1;
    end = path.find_first_of('/', start);
  }
  auto part = path.substr(start, end + 1 - start);
  if (part != "")
  {
    auto child = get_child(*node, part, false);
    if (child)
      node = child;
    else
    {
      node->children.insert({ part, std::make_unique<tree>() });
      node = node->children[part].get();
    }
  }
  node->handlers[method] = handler;
}

/**
* Handles a request.
*
* @param request the request
* @param         the response
*/
response router::handle_request(http::request&& request) const
{
  auto path = request.resource();

  // Process formdata
  std::unordered_map<std::string, std::string> form_data;
  auto i = path.find_first_of('?');
  if(i != path.npos)
  {
    auto form_string = path.substr(i + 1);
    path.erase(i);
    std::vector<std::string> form_elements;
    boost::algorithm::split(form_elements, form_string, boost::algorithm::is_any_of(","));
    for (auto& element : form_elements)
    {
      std::vector<std::string> pair;
      boost::algorithm::split(pair, element, boost::algorithm::is_any_of("="));
      if (pair.size() != 2)
        return http::status_code_400_BadRequest;

      boost::algorithm::replace_all(pair[0], "%20", " ");
      boost::algorithm::replace_all(pair[1], "%20", " ");
      boost::algorithm::trim(pair[0]);
      boost::algorithm::trim(pair[1]);
      form_data[pair[0]] = pair[1];
    }
  }

  auto& method = request.method();
  boost::property_tree::ptree content;
  if (request.get_body().size() > 0)
  {
    try
    {
      std::stringstream stream;
      for (auto& c : request.get_body())
        stream << c;
      boost::property_tree::json_parser::read_json(stream, content);
    }
    catch (const boost::property_tree::json_parser_error & error)
    {
      return http::status_code_400_BadRequest;
    }
  }

  std::unordered_map<std::string, std::string> symbols;
  auto node = resolve_path(path, &symbols);
  if (!node)
  {
    return http::status_code_404_NotFound;
  }

  auto it = node->handlers.find(method);
  if (it == node->handlers.end())
  {
    return http::status_code_405_MethodNotAllowed;
  }
  return (it->second)({ request.resource(), symbols, form_data, content });
}

router::tree* router::get_child(const tree& node, const std::string& part, bool match_mode, std::pair<std::string, std::string>* symbol) const
{
  if (part.front() == '{')
  {
    if (match_mode || part.back() != '}')
      throw std::runtime_error("[server::get_child] {<name>} illformed or not allowed");
    // Check if we have already a {<...>} property here
    for (auto& child : node.children)
    {
      if (child.first[0] == '{' && child.first != part)
        throw std::runtime_error("[server::get_child] all {<name>} parts at the same level have to have the same name");
    }
  }

  for (auto it = node.children.begin(); it != node.children.end(); ++it)
  {
    if (it->first == part)
      return it->second.get();
    else if (match_mode && it->first.front() == '{')
    {
      if (symbol)
        *symbol = { it->first.substr(1, it->first.size() - 2), part };
      return it->second.get();
    }
  }
  return nullptr;
}

const router::tree* router::resolve_path(std::string path, std::unordered_map<std::string, std::string>* resolved_symbols) const
{
  auto i = path.find_last_not_of('/');
  path = path.erase(i + 1);

  size_t start = path.find_first_of('/', 0) + 1;
  size_t end = path.find_first_of('/', start);

  auto node = _root.get();
  while (end != path.npos)
  {
    auto part = path.substr(start, end + 1 - start);
    if (part != "")
    {
      std::pair<std::string, std::string> resolved_symbol;
      auto child = get_child(*node, part, true, &resolved_symbol);
      if (resolved_symbol.first.size())
        resolved_symbols->insert(resolved_symbol);
      if (child)
        node = child;
      else
        return nullptr;
    }
    start = end + 1;
    end = path.find_first_of('/', start);
  }

  auto part = path.substr(start, end + 1 - start);

  if (part != "")
  {
    std::pair<std::string, std::string> resolved_symbol;
    auto child = get_child(*node, part, true, &resolved_symbol);
    if (resolved_symbol.first.size())
      resolved_symbols->insert(resolved_symbol);
    if (child)
      node = child;
    else
      return nullptr;
  }

  return node;
}