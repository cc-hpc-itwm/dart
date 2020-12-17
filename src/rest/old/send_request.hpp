#pragma once

#include "net/types.hpp"
#include "rest/router.hpp"

#include <functional>

#include <boost/system/error_code.hpp>

namespace rest
{
  /**
  * If an error occured, returns an empty response with code 500, InternalServerError.
  */
  extern rest::response send_request(net::ssl_socket& socket, rest::request& request, boost::system::error_code& e);

  extern void async_send_request(net::ssl_socket& socket, rest::request& request, 
    const std::function<void(rest::response&& response, const boost::system::error_code& e)>& handler);
}