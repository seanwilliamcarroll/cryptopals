#include <cookie.hpp>

size_t c_ProfileCookie::m_uid_counter = 0;

std::string profile_for(const std::string &email) {
  std::string encoded_email = c_ProfileCookie::encode_string(email);
  return c_ProfileCookie(encoded_email).to_string();
}
