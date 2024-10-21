#pragma once

#include <aes.hpp>
#include <freq_map.hpp>
#include <raw_bytes.hpp>
#include <util.hpp>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

RawBytes do_xor(const RawBytes &input_1, const RawBytes &input_2);

RawBytes do_xor(const RawBytes &input, uint8_t key);

RawBytes encrypt_repeating_xor(const RawBytes &plain_text, const RawBytes &key);

std::pair<char, double> find_likely_single_xor(const RawBytes &input);

size_t find_likely_key_length(const RawBytes &input, const size_t lower_bound,
                              const size_t upper_bound);

RawBytes find_likely_key(const RawBytes &input, const size_t key_length);

bool detect_ecb(const RawBytes &input);
