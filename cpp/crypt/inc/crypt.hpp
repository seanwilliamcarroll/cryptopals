#pragma once

#include <aes.hpp>
#include <block.hpp>
#include <cookie.hpp>
#include <freq_map.hpp>
#include <raw_bytes.hpp>
#include <util.hpp>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

RawBytes encrypt_repeating_xor(const RawBytes &plain_text, const RawBytes &key);

std::pair<char, double> find_likely_single_xor(const RawBytes &input);

size_t find_likely_key_length(const RawBytes &input, size_t lower_bound,
                              size_t upper_bound);

RawBytes find_likely_key(const RawBytes &input, size_t key_length);

bool detect_ecb(const RawBytes &input);

size_t detect_block_size(std::function<RawBytes(RawBytes)> encrypt_func);

size_t
detect_length_bytes(size_t block_size_bytes,
                    const RawBytes &target_plaintext_raw,
                    std::function<RawBytes(RawBytes, RawBytes)> encrypt_func);

RawBytes break_ecb_byte_at_a_time(size_t block_size_bytes,
                                  size_t target_plaintext_length_bytes,
                                  const c_AES128SecretKeyEncrypter &encrypter,
                                  const RawBytes &target_plaintext_raw,
                                  bool display = false);
