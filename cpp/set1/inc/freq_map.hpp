#pragma once

#include <raw_bytes.hpp>

#include <iostream>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

using FreqMap = std::unordered_map<char, double>;

const FreqMap english_freq_map = {
    {' ', 0.127 * 2}, {'e', 0.127},  {'t', 0.091},   {'?', 0.085},
    {'a', 0.082},     {'o', 0.075},  {'i', 0.070},   {'n', 0.067},
    {'s', 0.063},     {'h', 0.061},  {'r', 0.060},   {'d', 0.043},
    {'l', 0.040},     {'c', 0.028},  {'u', 0.028},   {'m', 0.024},
    {'w', 0.024},     {'f', 0.022},  {'g', 0.020},   {'y', 0.020},
    {'p', 0.019},     {'b', 0.015},  {'v', 0.0098},  {'k', 0.0077},
    {'j', 0.0015},    {'x', 0.0015}, {'q', 0.00095}, {'z', 0.00074},
};

FreqMap gen_frequency(const RawBytes &input) {
  FreqMap output(english_freq_map);
  double valid = 0;
  for (auto &[_, value] : output) {
    value = 0;
  }
  for (const char byte : input) {
    if (english_freq_map.find(byte) != english_freq_map.end()) {
      output[byte] += 1.0;
      valid = valid += 1.0;
    } else if (english_freq_map.find(char(std::tolower(byte))) !=
               english_freq_map.end()) {
      output[char(std::tolower(byte))] += 1.0;
      valid = valid += 1.0;
    } else {
      output['?'] += 1.0;
      valid = valid += 1.0;
    }
  }
  for (auto &[key, value] : output) {
    if (valid > 0) {
      value = value / double(valid);
    }
  }
  return output;
}

double score_freq(const FreqMap &input) {
  double accumulate = 0;
  for (auto [key, value] : input) {
    accumulate += std::abs(value - english_freq_map.at(key));
  }
  return accumulate;
}
