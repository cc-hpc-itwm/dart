#pragma once

#include "http/message.hpp"
#include <vector>
#include <deque>

#include <boost/optional/optional.hpp>

namespace http
{
  /**
  * Used to build http messages.
  *
  * The message builder is used to combine different data packages to a http message.
  */
  class message_builder final
  {
  private:
    using data = std::vector<char>;
    enum class state
    {
      start_line,
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
    * Converts the received data to a message.
    *
    * If there is not enough data, the message will be empty.
    * If an error occured, an exception is thrown.
    *
    * @return the potential message
    */
    boost::optional<message> next_message();
  private:
    bool read_start_line();
    bool read_header_field(bool& empty);
    bool read_message_body();
    boost::optional<std::string> read_line();
  private:
    std::deque<data> _data_queue;
    state _current_state = state::start_line;
    message _current_message;
  };
}