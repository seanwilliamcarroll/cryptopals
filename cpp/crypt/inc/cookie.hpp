#pragma once

#include <util.hpp>

#include <sstream>
#include <string>
#include <string_view>

struct c_ProfileCookie {

  c_ProfileCookie() = default;

  c_ProfileCookie(const std::string &email, const size_t uid,
                  const std::string &role)
      : m_email(email)
      , m_uid(uid)
      , m_role(role) {}

  c_ProfileCookie(const std::string &email)
      : m_email(email)
      , m_uid(m_uid_counter++)
      , m_role("user") {}

  static c_ProfileCookie from_string(const std::string &input) {
    size_t str_pos = 0;

    // Be naive for the moment
    c_ProfileCookie output;

    // FIXME Something is wrong here

    while (str_pos < input.size()) {
      size_t equal_pos = input.find_first_of('=', str_pos);
      std::string key_str = input.substr(str_pos, equal_pos - str_pos);
      size_t and_pos = input.find_first_of('&', equal_pos);
      if (and_pos == std::string_view::npos) {
        and_pos = input.size();
      }
      std::string value_str =
          input.substr(equal_pos + 1, and_pos - (equal_pos + 1));
      if (key_str == "email") {
        output.m_email = value_str;
      } else if (key_str == "uid") {
        output.m_uid = std::stoi(value_str);
      } else if (key_str == "role") {
        output.m_role = value_str;
      } else {
        throw_invalid_argument(key_str);
      }
      str_pos = and_pos + 1;
    }
    return output;
  }

  std::string to_string() const {
    std::stringstream string_stream;

    string_stream << "email=" << c_ProfileCookie::encode_string(m_email);
    string_stream << "&";
    string_stream << "uid=" << m_uid;
    string_stream << "&";
    string_stream << "role=" << m_role;

    return string_stream.str();
  }

  static std::string encode_string(const std::string &input) {
    std::string output;
    output.reserve(input.size());

    for (const auto &character : input) {
      if (character == '&') {
        output += "%26";
      } else if (character == '=') {
        output += "%3D";
      } else {
        output += character;
      }
    }
    return output;
  }

  static size_t m_uid_counter;
  std::string m_email;
  size_t m_uid;
  std::string m_role;
};

std::string profile_for(const std::string &email);
