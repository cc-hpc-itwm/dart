#include "http/status_code.hpp"

#include <stdexcept>

using namespace http;

#define STATUS_CODE(code, name) \
  const status_code status_code:: name##_##code \
  = status_code( code );

STATUS_CODE(100, Continue);

STATUS_CODE(200, OK);
STATUS_CODE(201, Created);
STATUS_CODE(202, Accepted);
STATUS_CODE(204, NoContent);
STATUS_CODE(400, BadRequest);
STATUS_CODE(401, Unauthorized);
STATUS_CODE(403, Forbidden);
STATUS_CODE(404, NotFound);
STATUS_CODE(405, MethodNotAllowed);
STATUS_CODE(410, Gone);
STATUS_CODE(411, LengthRequired);

STATUS_CODE(500, InternalServerError);
STATUS_CODE(501, NotImplemented);
STATUS_CODE(502, BadGateway);
STATUS_CODE(503, ServiceUnavailable);
#undef STATUS_CODE

/**
* Constructor
*
* Checks whether the code is valid. If not throws an error.
*
* @param code the status code string
*/
status_code::status_code(const std::string& code)
{
  auto temp = std::stoul(code);
  if (temp >= 100 && temp <= 599)
    _code = static_cast<decltype(_code)>(temp);
  else
    throw std::runtime_error("[http::status_code::status_code] invalid code '" + code + "'");
}

/**
* Returns the message of the status code.
*
* If it is an unknown status code, returns "Unknown".
*
* @return the status message
*/
std::string status_code::message() const
{
  switch (_code)
  {
#define STATUS_CODE(code, name) case code: return #name;
    STATUS_CODE(100, Continue);

    STATUS_CODE(200, OK);
    STATUS_CODE(201, Created);
    STATUS_CODE(202, Accepted);
    STATUS_CODE(204, No Content);
    STATUS_CODE(400, Bad Request);
    STATUS_CODE(401, Unauthorized);
    STATUS_CODE(403, Forbidden);
    STATUS_CODE(404, NotFound);
    STATUS_CODE(405, Method Not Allowed);
    STATUS_CODE(410, Gone);
    STATUS_CODE(411, Length Required);

    STATUS_CODE(500, Internal Server Error);
    STATUS_CODE(501, Not Implemented);
    STATUS_CODE(502, Bad Gateway);
    STATUS_CODE(503, Service Unavailable);
#undef STATUS_CODE
  }
  return std::to_string(code()) + " Unknown";
}