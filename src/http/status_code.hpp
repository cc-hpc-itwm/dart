#pragma once

#include <string>

namespace http
{
  /**
  * A helper class for http status codes.
  */
  class status_code final
  {
  public:
#define STATUS_CODE(code, name) static const status_code name##_##code;
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
  public:
    /**
    * Constructor
    *
    * Creates an invalid status code!
    */
    status_code() = default;

    /**
    * Constructor
    *
    * This constructor can be used to create invalid status codes.
    *
    * @param code the status code
    */
    explicit status_code(unsigned short code) noexcept : _code(code) { }

    /**
    * Constructor
    *
    * Checks whether the code is valid. If not throws an error.
    *
    * @param code the status code string
    */
    explicit status_code(const std::string& code);

    /**
    * Copy Constructor.
    */
    status_code(const status_code&) = default;

    /**
    * Move Constructor.
    */
    status_code(status_code&&) = default;

    /**
    * Copy operator.
    */
    status_code& operator=(const status_code&) = default;

    /**
    * Move operator.
    */
    status_code& operator=(status_code&&) = default;

    /**
    * Destructor.
    */
    ~status_code() = default;

    /**
    * Checks whether the status code is valid.
    *
    * @return true, if it is valid
    */
    inline bool is_valid() const noexcept { return _code >= 100 && _code <= 599; }
    
    /**
    * Returns the message of the status code.
    *
    * If it is an unknown status code, returns "Unknown".
    *
    * @return the status message
    */
    std::string message() const;

    /**
    * Gets the status code.
    *
    * @return the status code
    */
    inline unsigned short code() const noexcept { return _code; }
  private:
    unsigned short _code = 0;
  };
}