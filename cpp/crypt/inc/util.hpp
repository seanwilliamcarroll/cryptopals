#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::string load_from_file(const std::string &filename);

std::vector<std::string> load_lines_from_file(const std::string &filename);

void strip_newlines(std::string &input);

std::string strip_newlines(const std::string &input);

std::string load_and_strip(const std::string &filename);

void throw_invalid_argument(const std::string &input);

void throw_invalid_argument(uint8_t input);
