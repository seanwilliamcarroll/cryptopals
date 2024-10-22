#pragma once

#include <block.hpp>
#include <raw_bytes.hpp>

#include <cstdint>
#include <random>

struct c_RandomByteGenerator {
  RawBytes generate_n_random_bytes(const size_t num_bytes) {
    RawBytes output(num_bytes, 0);
    for (auto &nth_byte : output) {
      nth_byte = generate_random_byte();
    }
    return output;
  }

  Word generate_random_word() {
    Word output;
    for (auto &nth_byte : output) {
      nth_byte = generate_random_byte();
    }
    return output;
  }

  template <size_t WordArrayLength>
  WordArray<WordArrayLength> generate_random_word_array() {
    WordArray<WordArrayLength> output;
    for (auto &word : output) {
      word = generate_random_word();
    }
    return output;
  }

  ByteBlock generate_random_byte_block() {
    return generate_random_word_array<BLOCK_SIZE_WORDS>();
  }

  uint8_t generate_random_byte() { return m_ByteGenerator(m_RandomEngine); }

  size_t generate_in_range(const size_t begin, const size_t end_inclusive) {
    std::uniform_int_distribution<size_t> dist(begin, end_inclusive);
    return dist(m_RandomEngine);
  }

private:
  std::mt19937 m_RandomEngine;
  std::uniform_int_distribution<uint8_t> m_ByteGenerator;
};
