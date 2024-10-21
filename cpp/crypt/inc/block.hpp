#pragma once

#include <raw_bytes.hpp>

#include <array>
#include <cstdint>
#include <iostream>

constexpr inline size_t WORD_SIZE_BYTES = 4;
constexpr inline size_t BLOCK_SIZE_WORDS = 4;
constexpr inline size_t BLOCK_SIZE_BYTES = WORD_SIZE_BYTES * BLOCK_SIZE_WORDS;

using ByteColumn = std::array<uint8_t, WORD_SIZE_BYTES>;
using Word = ByteColumn;
using ByteBlock = std::array<ByteColumn, BLOCK_SIZE_WORDS>;

ByteColumn get_column(const ByteBlock &, size_t col_index);
void set_column(ByteBlock &, const ByteColumn &, size_t col_index);
Word get_row(const ByteBlock &, size_t row_index);
void set_row(ByteBlock &, const Word &, size_t row_index);

template <typename CastType>
std::ostream &pretty_print(std::ostream &out, const Word &input) {
  for (size_t row_index = 0; row_index < BLOCK_SIZE_WORDS; ++row_index) {
    out << CastType(input[row_index]);
    out << " ";
  }
  return out;
}

template <typename CastType>
std::ostream &pretty_print(std::ostream &out, const ByteBlock &input) {
  for (size_t row_index = 0; row_index < BLOCK_SIZE_WORDS; ++row_index) {
    Word row = get_row(input, row_index);
    for (size_t col_index = 0; col_index < WORD_SIZE_BYTES; ++col_index) {
      out << CastType(row[col_index]);
      out << " ";
    }
    out << std::endl;
  }
  return out;
}

ByteBlock from_raw_bytes_to_byte_block(const RawBytes &,
                                       size_t block_number = 0);
void from_byte_block_to_raw_bytes(const ByteBlock &input, RawBytes &output,
                                  size_t block_number = 0);
RawBytes from_byte_block_to_raw_bytes(const ByteBlock &);

void rot_word(Word &);
void sub_word(Word &);

void sub_bytes(ByteBlock &);
void inv_sub_bytes(ByteBlock &);

void shift_rows(ByteBlock &);
void inv_shift_rows(ByteBlock &);

void mix_columns(ByteBlock &);
void inv_mix_columns(ByteBlock &);

Word operator^(const Word &, const Word &);
ByteBlock operator^(const ByteBlock &, const ByteBlock &);
