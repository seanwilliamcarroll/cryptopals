#pragma once

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

RawBytes encrypt_repeating_xor(const RawBytes &plain_text,
                               const RawBytes &key) {
  RawBytes output(plain_text);

  size_t key_pointer = 0;
  for (auto &character : output) {
    character ^= key[key_pointer];
    ++key_pointer;
    if (key_pointer == key.size()) {
      key_pointer = 0;
    }
  }
  return output;
}

std::pair<char, double> find_likely_single_xor(const RawBytes &input) {
  double score = std::numeric_limits<double>::max();
  char winner = 0;
  for (size_t iter = 0; iter < 256; ++iter) {
    RawBytes xord_output = do_xor(input, char(iter));
    FreqMap freq_map = gen_frequency(xord_output);
    double test_score = score_freq(freq_map);
    if (test_score < score) {
      score = test_score;
      winner = char(iter);
    }
  }
  return std::make_pair(winner, score);
}

template <typename IterableType>
size_t calc_hamming_dist(const IterableType &input_a,
                         const IterableType &input_b) {
  size_t distance = 0;

  // Assume same length
  for (size_t iter = 0; iter < input_a.size(); ++iter) {
    distance += std::popcount(size_t(input_a[iter] ^ input_b[iter]));
  }

  return distance;
}

double score_key_length(const RawBytes &input, const size_t key_length,
                        const size_t max_comparisons = 0) {
  double score = 0.0;

  const size_t max_key_sections = (input.size() / key_length);

  if (max_key_sections < 2) {
    throw std::range_error("Key length was longer than half of input!");
    return score;
  }

  const size_t num_comparisons =
      (max_comparisons == 0) ? (max_key_sections - 1) : max_comparisons;

  auto raw_bytes_span = std::span{input};

  for (size_t iter = 0; iter < num_comparisons; ++iter) {
    auto first_section = raw_bytes_span.subspan(iter * key_length, key_length);
    auto second_section =
        raw_bytes_span.subspan((iter + 1) * key_length, key_length);

    score += calc_hamming_dist(first_section, second_section);
  }
  return (score / num_comparisons) / key_length;
}

size_t find_likely_key_length(const RawBytes &input, const size_t lower_bound,
                              const size_t upper_bound) {

  size_t best_key_length = lower_bound;
  double best_score = std::numeric_limits<double>::max();

  for (size_t key_length = lower_bound; key_length <= upper_bound;
       ++key_length) {
    double new_score =
        score_key_length(input, key_length, (input.size() / upper_bound) - 1);
    if (new_score < best_score) {
      best_score = new_score;
      best_key_length = key_length;
    }
  }
  return best_key_length;
}

RawBytes find_likely_key(const RawBytes &input, const size_t key_length) {
  RawBytes key;
  key.reserve(key_length);

  const size_t longest_striping =
      std::ceil<size_t>(double(input.size()) / key_length);

  RawBytes scratch_space;
  scratch_space.reserve(longest_striping);

  for (size_t iter = 0; iter < key_length; ++iter) {

    for (size_t input_ptr = 0; input_ptr < input.size(); ++input_ptr) {
      if ((input_ptr % key_length) == iter) {
        scratch_space.push_back(input[input_ptr]);
      }
    }

    const auto [character, _] = find_likely_single_xor(scratch_space);

    key.push_back(character);
    scratch_space.clear();
  }

  return key;
}
