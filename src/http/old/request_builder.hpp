#pragma once

#include "http/request.hpp"
#include <vector>
#include <deque>

#include <boost/optional/optional.hpp>

namespace http
{
  /**
  * Used to build http requests.
  *
  * The request builder is used to combine different data packages to a http request.
  */
  class request_builder final
  {
  private:
    using data = std::vector<char>;
    enum class state
    {
      request_line,
      header_fields,
      message_body
    };
  public:
    /**
    * Add more data.
    *
    * @param data the data
    */
    inline void add_data(data&& data) { _data_queue.emplace_back(data); }

    /**
    * Converts the recieved data to a request.
    *
    * If there is not enough data, the result will be empty.
    * If an error occured, an exception is thrown.
    *
    * @return the potential request
    */
    boost::optional<request> next_request();

    //void dump_buffer_info();
  private:
    bool read_request_line();
    bool read_header_field(bool& empty);
    bool read_message_body();
    boost::optional<std::string> read_line();
  private:
    std::deque<data> _data_queue;
    state _current_state;
    request _current_request;
  };
}