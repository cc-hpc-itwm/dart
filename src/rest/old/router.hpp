#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

#include <boost/property_tree/ptree.hpp>

#include "http/status_code.hpp"
#include "http/response.hpp"
#include "http/request.hpp"

namespace rest
{
  struct request
  {
    std::string url;
    std::unordered_map<std::string, std::string> symbols;
    std::unordered_map<std::string, std::string> form_data;
    boost::property_tree::ptree content;
  };

  struct response
  {
    http::status_code status_code       = http::status_code_200_OK;
    boost::property_tree::ptree content = {};

    response() = default;
    response(boost::property_tree::ptree&& ptree) 
      : status_code(http::status_code_200_OK), content(ptree) { }

    response(http::status_code code)
      : status_code(code) { }

    response(http::status_code code, boost::property_tree::ptree&& ptree)
      : status_code(code), content(ptree) { }

    ~response() = default;
  };

  class router final
  {
  public:
    using handler = std::function<response(request&&)>;
  private:
    struct tree
    {
      std::unordered_map<std::string, handler> handlers; //!< request_method -> handler map
      std::unordered_map<std::string, std::unique_ptr<tree>> children; // part -> subtree
    };
  public:
    router();
    router(const router&) = delete;
    router(router&&) = default;
    router& operator=(const router&) = delete;
    router& operator=(router&&) = default;

    ~router() = default;
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
    void add_resource(std::string path, const std::string& method, const handler& handler);

    /**
    * Handles a request.
    *
    * @param request the request
    * @param         the response
    */
    response handle_request(http::request&& request) const;
  private:
    tree* get_child(const tree& node, const std::string& part, bool match_mode, std::pair<std::string, std::string>* symbol = nullptr) const;
    const tree* resolve_path(std::string path, std::unordered_map<std::string, std::string>* resolved_symbols) const;
  private:
    std::unique_ptr<tree> _root;
  };
}