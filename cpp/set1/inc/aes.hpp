#pragma once

#include <raw_bytes.hpp>

#include <algorithm>
#include <array>
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

constexpr inline size_t WORD_SIZE_ENTRIES = 4;
constexpr inline size_t BLOCK_SIZE_WORDS = 4;
constexpr inline size_t BLOCK_SIZE_ENTRIES =
    WORD_SIZE_ENTRIES * BLOCK_SIZE_WORDS;

template <typename T> using WordColumn = std::array<T, WORD_SIZE_ENTRIES>;

template <typename T> using Block = std::array<WordColumn<T>, BLOCK_SIZE_WORDS>;

using ByteBlock = Block<uint8_t>;

template <typename CastType, typename DataType>
std::ostream &pretty_print_block(std::ostream &out,
                                 const Block<DataType> &input) {
  for (size_t col_index = 0; col_index < WORD_SIZE_ENTRIES; ++col_index) {
    for (size_t row_index = 0; row_index < BLOCK_SIZE_WORDS; ++row_index) {
      out << CastType(input[col_index][row_index]);
      if (row_index != (BLOCK_SIZE_WORDS - 1)) {
        out << " ";
      }
    }
    out << std::endl;
  }
  return out;
}

ByteBlock from_raw_bytes_to_byte_block(const RawBytes &input,
                                       const size_t block_number = 0);

void from_byte_block_to_raw_bytes(const Block<uint8_t> &input, RawBytes &output,
                                  const size_t block_number = 0);

RawBytes from_byte_block_to_raw_bytes(const Block<uint8_t> &input);
