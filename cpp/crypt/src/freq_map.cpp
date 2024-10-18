#include <freq_map.hpp>

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
