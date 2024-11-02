#include <util.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

std::string load_from_file(const std::string &filename) {
  std::stringstream input_buffer;
  std::ifstream file_pointer(filename, std::ios::in);
  if (!file_pointer.is_open()) {
    throw std::runtime_error("Couldn't open file: " + filename);
    return input_buffer.str();
  }

  input_buffer << file_pointer.rdbuf();
  return input_buffer.str();
}

std::vector<std::string> load_lines_from_file(const std::string &filename) {
  std::vector<std::string> output;
  std::ifstream file_pointer(filename, std::ios::in);
  if (!file_pointer.is_open()) {
    throw std::runtime_error("Couldn't open file: " + filename);
    return output;
  }
  std::string line;

  while (std::getline(file_pointer, line)) {
    output.push_back(line);
  }

  return output;
}

void strip_newlines(std::string &input) {
  input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
}

std::string strip_newlines(const std::string &input) {
  std::string output(input);
  strip_newlines(output);
  return output;
}

std::string load_and_strip(const std::string &filename) {
  const std::string base64_input = load_from_file(filename);
  return strip_newlines(base64_input);
}

void throw_invalid_argument(const std::string &input) {
  std::stringstream error_stream;
  error_stream << "Unexpected string: \"" << input << "\"";
  throw std::invalid_argument(error_stream.str());
}

void throw_invalid_argument(uint8_t input) {
  std::stringstream error_stream;
  error_stream << "Unexpected character: " << char(input)
               << " value: " << int(input);
  throw std::invalid_argument(error_stream.str());
}
