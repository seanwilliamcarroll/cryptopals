#include <iostream>

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using RawBytes = std::vector<uint8_t>;

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

char to_hex_char(uint8_t input) {
  if (input >= 0 && input <= 9) {
    return input + '0';
  } else {
    return (input - 10) + 'a';
  }
}

char to_base64_char(uint8_t input) {
  char out;
  if (input >= 0 && input <= 25) {
    out = input + 'A';
  } else if (input >= 26 && input <= 51) {
    out = (input - 26) + 'a';
  } else if (input >= 52 && input <= 61) {
    out = (input - 52) + '0';
  } else if (input == 62) {
    out = '+';
  } else {
    out = '/';
  }
  // std::cout << "Value: " << std::to_string(input) << " to " << out <<
  // std::endl;

  return out;
}

uint8_t top_nibble(uint8_t input) {
  size_t more_space = input;
  return ((more_space & 0xF0) >> 4) & 0xF;
}

uint8_t bottom_nibble(uint8_t input) {
  size_t more_space = input;
  return more_space & 0xF;
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

RawBytes do_xor(const RawBytes &input_1, const RawBytes &input_2) {
  RawBytes output(input_1);

  for (size_t iter = 0; iter < output.size(); ++iter) {
    output[iter] ^= input_2[iter];
  }
  return output;
}

void c1() {

  const std::string input("49276d206b696c6c696e6720796f757220627261696e206c696b"
                          "65206120706f69736f6e6f7573206d757368726f6f6d");

  RawBytes output = from_hex_string(input);

  std::cout << "Input    hex string: " << input << std::endl;
  std::cout << "Recreate hex string: ";
  to_hex_string(std::cout, output) << std::endl;
  std::cout << "New   Base64 string: ";
  to_base64_string(std::cout, output) << std::endl;
}

void c2() {

  const std::string input_1("1c0111001f010100061a024b53535009181c");
  const std::string input_2("686974207468652062756c6c277320657965");

  RawBytes output_1 = from_hex_string(input_1);
  RawBytes output_2 = from_hex_string(input_2);

  RawBytes output_xored = do_xor(output_1, output_2);

  std::cout << "Input hex string: " << input_1 << std::endl;
  std::cout << "Input hex string: " << input_2 << std::endl;
  std::cout << "XOR'd hex string: ";
  to_hex_string(std::cout, output_xored) << std::endl;
}

int main() {
  std::cout << "Cryptopals" << std::endl;
  // c1();
  c2();

  return 0;
}
