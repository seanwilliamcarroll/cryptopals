#include <iostream>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <limits>
#include <unordered_map>

using RawBytes = std::vector<uint8_t>;
using FreqMap = std::unordered_map<char, double>;

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


RawBytes do_xor(const RawBytes &input_1, const RawBytes &input_2) {
  RawBytes output(input_1);

  for (size_t iter = 0; iter < output.size(); ++iter) {
    output[iter] ^= input_2[iter];
  }
  return output;
}

RawBytes do_xor(const RawBytes &input, uint8_t key) {
  RawBytes output(input);

  for (size_t iter = 0; iter < output.size(); ++iter) {
    output[iter] ^= key;
  }
  return output;
}

const FreqMap english_freq_map = {
  {'e', 0.127},
  {'t', 0.091},
  {'a', 0.082},
  {'o', 0.075},
  {'i', 0.070},
  {'n', 0.067},
  {'s', 0.063},
  {'h', 0.061},
  {'r', 0.060},
  {'d', 0.043},
  {'l', 0.040},
  {'c', 0.028},
  {'u', 0.028},
  {'m', 0.024},
  {'w', 0.024},
  {'f', 0.022},
  {'g', 0.020},
  {'y', 0.020},
  {'p', 0.019},
  {'b', 0.015},
  {'v', 0.0098},
  {'k', 0.0077},
  {'j', 0.0015},
  {'x', 0.0015},
  {'q', 0.00095},
  {'z', 0.00074},
};

FreqMap gen_frequency(const RawBytes& input){
  FreqMap output(english_freq_map);
  double valid = 0;
  for (auto& [_, value] : output) {
    value = 0;
  }
  for (const char byte : input) {
    if (byte >= 'a' && byte <= 'z') {
      output[byte] += 1.0;
      valid = valid += 1.0;
    } else if (byte >= 'A' && byte <= 'Z') {
      output[byte - 'A' + 'a'] += 1.0;
      valid = valid += 1.0;
    }
  }
  for (auto& [key, value] : output) {
    if (valid > 0) {
      value = value / double(valid);
    }
  }  
  return output;
}

double score_freq(const FreqMap& input) {
  double accumulate = 0;
  for (auto [key, value] : input) {
    accumulate += std::abs(value - english_freq_map.at(key));
  }
  return accumulate;
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

void c3() {
  const std::string input("1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736");

  RawBytes output = from_hex_string(input);

  std::cout << "Input    hex string: " << input << std::endl;
  std::cout << "Input  ascii string: ";
  to_ascii_string(std::cout, output) << std::endl;

  std::cout << "Test strings: " << std::endl;

  double score = std::numeric_limits<double>::max();
  char winner = 0;
  for (size_t iter = 0; iter < 256; ++iter) {
    RawBytes xord_output = do_xor(output, char(iter));
    FreqMap freq_map = gen_frequency(xord_output);
    double test_score = score_freq(freq_map);
    if (test_score < score) {
      score = test_score;
      winner = char(iter);
    }
  }
  RawBytes xord_output = do_xor(output, char(winner));
  std::cout << "XOR with: " << std::to_string(winner) << std::endl;
  std::cout << "Output ascii string: ";
  to_ascii_string(std::cout, xord_output) << std::endl;
}

int main() {
  std::cout << "Cryptopals" << std::endl;
  // c1();
  // c2();
  c3();

  return 0;
}
