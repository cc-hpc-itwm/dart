#include "request_builder.hpp"

#include <boost/algorithm/string.hpp>

#include <stdexcept>
#include <iostream>

using namespace http;

/**
* Converts the recieved data to a request.
*
* If there is not enough data, the result will be empty.
* If an error occured, an exception is thrown.
*
* @return the potential request
*/
boost::optional<request> request_builder::next_request()
{
  if (_current_state == state::request_line)
  {
    if (read_request_line())
      _current_state = state::header_fields;
  }

  if (_current_state == state::header_fields)
  {
    bool empty = false;
    while (read_header_field(empty) && !empty);
    if (empty)
      _current_state = state::message_body;
  }

  if (_current_state == state::message_body)
  {
    if (read_message_body())
    {
      _current_state = state::request_line;
      return std::move(_current_request);
    }
  }
  return boost::none;
}

bool request_builder::read_request_line()
{
  auto line = read_line();
  if (!line)
    return false;

  auto i = line->find_first_of(' ');
  auto j = line->find_last_of(' ');
  
  _current_request = request(line->substr(0, i), line->substr(i+1, j-(i+1)), line->substr(j+1));
  return true;
}

/*void request_builder::dump_buffer_info()
{
  std::cout << "DUMPING BUFFER INFO" << std::endl;
  for (auto data : _data_queue)
  {
    std::cout << "-- Buffer (" << data.size() << ")\n";
    std::cout << std::string(data.data(), data.size()) << std::endl;
    std::cout << "-- Buffer (end)" << std::endl;
  }
}*/

bool request_builder::read_header_field(bool& empty)
{
  auto line = read_line();
  if (!line)
    return false;

  if (line->size() == 0)
  {
    empty = true;
    return true;
  }

  auto i = line->find_first_of(':');
  auto j = line->find_first_not_of(" \t", i+1);
  auto k = line->find_last_not_of(" \t");

  if (i == line->npos || j == line->npos)
  {
    throw std::runtime_error("[request_parser::read_header_field] header line has not format '<name>: <content>'");
  }

  auto field_name = line->substr(0, i);
  auto field_content = line->substr(j, k+1-j);

  _current_request[field_name] = field_content;
  return true;
}

bool request_builder::read_message_body()
{
  if (_current_request.has_field("Transfer-Encoding"))
    throw std::runtime_error("[request_builder::read_message_body] Transfer-Encoding currently not supported");
  if (!_current_request.has_field("Content-Length"))
    return true;
  
  auto content_length = std::stoul(_current_request["Content-Length"]);
  auto available_data = 0u;

  for (auto it = _data_queue.begin(); it != _data_queue.end(); ++it)
  {
    available_data += it->size();
    if (available_data >= content_length)
      break;
  }

  if (available_data < content_length)
    return false;

  std::vector<char> content;
  content.reserve(content_length);
  while (content_length)
  {
    auto& data = _data_queue.front();

    if (data.size() >= content_length)
    {
      // use the whole buffer
      content.insert(content.end(), data.begin(), data.end());
      content_length -= data.size();
      _data_queue.pop_front();
    }
    else
    {
      auto it = data.begin() + content_length;
      content.insert(content.end(), data.begin(), it);
      data.erase(data.begin(), it);
      content_length = 0;
    }
  }

  _current_request.set_body(std::move(content));

  return true;
}

boost::optional<std::string> request_builder::read_line()
{
  bool found_crlf = false;
  bool last_was_cr = false;
  for (auto it = _data_queue.begin(); it != _data_queue.end(); ++it)
  {
    if (it->size() == 0)
      continue;

    if (last_was_cr)
    {
      if (it->front() == '\n')
      {
        found_crlf = true;
        break;
      }
      else
      {
        throw std::runtime_error("[request_builder::read_line] found '\r' not followed by '\n'");
      }
    }
    auto iter = std::find(it->begin(), it->end(), '\r');
    if (iter == it->end())
      continue;

    ++iter;
    if (iter == it->end())
    {
      last_was_cr = true;
    }

    if(*iter != '\n')
      throw std::runtime_error("[request_builder::read_line] found '\r' not followed by '\n'");

    found_crlf = true;
    break;
  }

  if (!found_crlf)
    return boost::none;

  std::string line;
  while (true)
  {
    auto& data = _data_queue.front();
    auto iter = std::find(data.begin(), data.end(), '\r');
    line.insert(line.end(), data.begin(), iter);
    if (iter == data.end())
    {
      _data_queue.pop_front();
      continue;
    }

    if (++iter == data.end())
    {
      _data_queue.pop_front();
      _data_queue.front().erase(_data_queue.front().begin());
      break;
    }

    if (++iter == data.end())
      _data_queue.pop_front();
    else
      data.erase(data.begin(), iter);
    break;
  }

  return line;
}