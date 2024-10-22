#pragma once

#include <block.hpp>
#include <raw_bytes.hpp>

#include <cstdint>
#include <random>

struct c_RandomByteGenerator {
  RawBytes generate_n_random_bytes(const size_t num_bytes) {
    RawBytes output(num_bytes, 0);
    for (auto &nth_byte : output) {
      nth_byte = m_ByteGenerator(m_RandomEngine);
    }
    return output;
  }

  Word generate_random_word() {
    Word output;
    for (auto &nth_byte : output) {
      nth_byte = m_ByteGenerator(m_RandomEngine);
    }
    return output;
  }

  ByteBlock generate_random_byte_block() {
    ByteBlock output;
    for (auto &word : output) {
      word = generate_random_word();
    }
    return output;
  }

private:
  std::mt19937 m_RandomEngine;
  std::uniform_int_distribution<uint8_t> m_ByteGenerator;
};
