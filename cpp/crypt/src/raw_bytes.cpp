#include <raw_bytes.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

RawBytes from_hex_string(const std::string &input) {
  const size_t length = std::ceil<size_t>(double(input.size()) / 2);

  RawBytes output(length, 0);
  size_t current_pos = 0;

  for (const auto &character : input) {
    uint8_t value;
    uint8_t offset = 0;
    if (character >= '0' && character <= '9') {
      offset = '0';
    } else if (character >= 'a' && character <= 'f') {
      offset = 'a' - 10;
    } else if (character >= 'A' && character <= 'F') {
      offset = 'A' - 10;
    } else {
      throw_invalid_argument(character);
    }
    value = character - offset;
    const bool is_top_byte = current_pos % 2 == 0;
    if (is_top_byte) {
      output[current_pos / 2] = char(size_t(value) << 4);
    } else {
      output[current_pos / 2] |= value;
    }
    current_pos++;
  }
  return output;
}

char to_base64_char(uint8_t input) {
  if (input >= 0 && input <= 25) {
    return input + 'A';
  } else if (input >= 26 && input <= 51) {
    return (input - 26) + 'a';
  } else if (input >= 52 && input <= 61) {
    return (input - 52) + '0';
  } else if (input == 62) {
    return '+';
  } else {
    return '/';
  }
}

uint8_t top_nibble(uint8_t input) {
  size_t more_space = input;
  return ((more_space & 0xF0) >> 4) & 0xF;
}

uint8_t bottom_nibble(uint8_t input) {
  size_t more_space = input;
  return more_space & 0xF;
}

char to_hex_char(uint8_t input) {
  if (input >= 0 && input <= 9) {
    return input + '0';
  } else if (input >= 10 && input <= 15) {
    return (input - 10) + 'a';
  }
  throw_invalid_argument(input);
  return 0;
}

std::ostream &to_hex_string(std::ostream &out, const RawBytes &input) {
  for (const auto &byte : input) {
    out << to_hex_char(top_nibble(byte)) << to_hex_char(bottom_nibble(byte));
  }
  return out;
}

std::ostream &to_base64_string(std::ostream &out, const RawBytes &input) {
  size_t iter_length = input.size();
  if (input.size() % 3 != 0) {
    iter_length += 1;
  }
  for (size_t iter = 0; iter < iter_length; ++iter) {
    uint8_t prev_byte;
    uint8_t current_byte;
    if (iter >= input.size()) {
      prev_byte = input.back();
      current_byte = 0;
    } else {
      prev_byte = input[iter - 1];
      current_byte = input[iter];
    }
    if (iter % 3 == 0) {
      // Top 6 bits
      uint8_t current_byte = input[iter];
      uint8_t base64_value = (size_t(current_byte) >> 2) & 0x3F;
      out << to_base64_char(base64_value);
    } else if (iter % 3 == 1) {
      // Bottom 2 bits, top 4 bits
      uint8_t base64_value = ((size_t(prev_byte) & 0x3) << 4) |
                             ((size_t(current_byte) >> 4) & 0xF);
      out << to_base64_char(base64_value);
    } else {
      // Bottom 4 bits, top 2 bits
      uint8_t base64_value = ((size_t(prev_byte) & 0xF) << 2) |
                             ((size_t(current_byte) >> 6) & 0x3);
      out << to_base64_char(base64_value);
      // Bottom 6 bits
      if (iter < input.size()) {
        base64_value = (size_t(current_byte) & 0x3F);
        out << to_base64_char(base64_value);
      }
    }
  }
  if (input.size() % 3 == 1) {
    out << '=' << '=';
  } else if (input.size() % 3 == 2) {
    out << '=';
  }
  return out;
}

std::ostream &to_ascii_string(std::ostream &out, const RawBytes &input) {
  for (const auto &byte : input) {
    out << char(byte);
  }
  return out;
}

RawBytes from_ascii_string(const std::string &input) {
  RawBytes output;
  output.reserve(input.size());

  for (const auto character : input) {
    output.push_back(character);
  }
  return output;
}

uint8_t from_base64_char(uint8_t input) {
  if (input >= 'A' && input <= 'Z') {
    return input - 'A';
  } else if (input >= 'a' && input <= 'z') {
    return input - 'a' + 26;
  } else if (input >= '0' && input <= '9') {
    return input - '0' + 52;
  } else if (input == '+') {
    return 62;
  } else if (input == '/') {
    return 63;
  }
  throw_invalid_argument(input);
  return 0;
}

RawBytes from_base64_string(const std::string &input) {
  size_t total_characters = input.size();
  size_t num_padding_chars = std::ranges::count(input, '=');
  const size_t valid_characters = total_characters - num_padding_chars;
  size_t output_length = (total_characters / 4) * 3;
  if (num_padding_chars == 1) {
    output_length += 2;
  } else if (num_padding_chars == 2) {
    output_length += 1;
  }
  RawBytes output;
  output.reserve(output_length);
  size_t current_position = 0;
  while (valid_characters > current_position) {
    if (valid_characters - current_position >= 4) {
      uint8_t char0 = from_base64_char(input[current_position]);
      uint8_t char1 = from_base64_char(input[current_position + 1]);
      uint8_t char2 = from_base64_char(input[current_position + 2]);
      uint8_t char3 = from_base64_char(input[current_position + 3]);

      // char0, plus top 2 bits of char1
      uint8_t byte0 =
          ((size_t(char0) & 0x3F) << 2) | ((size_t(char1) & 0x30) >> 4);
      // bottom 4 bits of char1, plus top 4 bits of char2
      uint8_t byte1 =
          ((size_t(char1) & 0xF) << 4) | ((size_t(char2) & 0x3C) >> 2);
      // bottom 2 bits of char2, plus char3
      uint8_t byte2 = ((size_t(char2) & 0x3) << 6) | (size_t(char3) & 0x3F);

      output.push_back(byte0);
      output.push_back(byte1);
      output.push_back(byte2);
      current_position += 4;
    } else if (valid_characters - current_position == 3) {
      uint8_t char0 = from_base64_char(input[current_position]);
      uint8_t char1 = from_base64_char(input[current_position + 1]);
      uint8_t char2 = from_base64_char(input[current_position + 2]);

      // char0, plus top 2 bits of char1
      uint8_t byte0 =
          ((size_t(char0) & 0x3F) << 2) | ((size_t(char1) & 0x30) >> 4);
      // bottom 4 bits of char1, plus top 4 bits of char2
      uint8_t byte1 =
          ((size_t(char1) & 0xF) << 4) | ((size_t(char2) & 0x3C) >> 2);

      output.push_back(byte0);
      output.push_back(byte1);
      current_position += 3;
    } else {
      // Difference must be 2
      uint8_t char0 = from_base64_char(input[current_position]);
      uint8_t char1 = from_base64_char(input[current_position + 1]);
      // char0, plus top 2 bits of char1
      uint8_t byte0 =
          ((size_t(char0) & 0x3F) << 2) | ((size_t(char1) & 0x30) >> 4);
      output.push_back(byte0);
      current_position += 2;
    }
  }
  return output;
}

RawBytes prepend_bytes(const RawBytes &original, const RawBytes &prefix) {
  RawBytes output(prefix.size() + original.size(), 0);

  std::copy(std::begin(prefix), std::end(prefix), std::begin(output));
  std::copy(std::begin(original), std::end(original),
            std::begin(output) + prefix.size());

  return output;
}
