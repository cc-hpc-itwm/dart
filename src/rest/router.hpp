#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

#include <boost/property_tree/ptree.hpp>

#include "http/client.hpp"
#include "rest/message.hpp"

namespace rest
{
  using symbols = std::unordered_map<std::string, std::string>;
  /**
  * The router routes different rest request to different handlers
  */
  class router final
  {
  public:
    using handler = std::function<void(http::client* client, const message&, const symbols&)>;
  private:
    struct tree
    {
      std::unordered_map<std::string, handler> handlers; //!< request_method -> handler map
      std::unordered_map<std::string, std::unique_ptr<tree>> children; // part -> subtree
    };
  public:
    /**
    * Constructor.
    */
    router() : _root(std::make_unique<tree>()) { }

    router(const router&) = delete;
    router(router&&) = default;
    router& operator=(const router&) = delete;
    router& operator=(router&&) = default;

    /**
    * Destructor.
    */
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
    * @param client  the client
    * @param request the request
    */
    void handle_request(http::client* client, const http::message& request) const;
  private:
    tree* get_child(const tree& node, const std::string& part, bool match_mode, std::pair<std::string, std::string>* symbol = nullptr) const;
    const tree* resolve_path(std::string path, std::unordered_map<std::string, std::string>* resolved_symbols) const;
  private:
    std::unique_ptr<tree> _root;
  };
}