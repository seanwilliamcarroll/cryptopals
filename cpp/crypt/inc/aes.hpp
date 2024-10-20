#pragma once

#include <raw_bytes.hpp>

#include <array>
#include <cstdint>
#include <iostream>

constexpr inline size_t WORD_SIZE_BYTES = 4;
constexpr inline size_t BLOCK_SIZE_WORDS = 4;
constexpr inline size_t BLOCK_SIZE_BYTES = WORD_SIZE_BYTES * BLOCK_SIZE_WORDS;

constexpr inline size_t AES_128_KEY_LENGTH_WORDS = 4;
constexpr inline size_t AES_192_KEY_LENGTH_WORDS = 6;
constexpr inline size_t AES_256_KEY_LENGTH_WORDS = 8;

constexpr inline size_t AES_128_NUM_ROUNDS = 10;
constexpr inline size_t AES_192_NUM_ROUNDS = 12;
constexpr inline size_t AES_256_NUM_ROUNDS = 14;

using ByteColumn = std::array<uint8_t, WORD_SIZE_BYTES>;
using Word = ByteColumn;
using ByteBlock = std::array<ByteColumn, BLOCK_SIZE_WORDS>;

using AES128Key = std::array<ByteColumn, AES_128_KEY_LENGTH_WORDS>;
using AES192Key = std::array<ByteColumn, AES_192_KEY_LENGTH_WORDS>;
using AES256Key = std::array<ByteColumn, AES_256_KEY_LENGTH_WORDS>;

using AES128KeySchedule =
    std::array<ByteColumn, BLOCK_SIZE_WORDS *(AES_128_NUM_ROUNDS + 1)>;

using AES192KeySchedule =
    std::array<ByteColumn, BLOCK_SIZE_WORDS *(AES_192_NUM_ROUNDS + 1)>;

using AES256KeySchedule =
    std::array<ByteColumn, BLOCK_SIZE_WORDS *(AES_256_NUM_ROUNDS + 1)>;

ByteColumn get_column(const ByteBlock &block, size_t col_index);
void set_column(ByteBlock &block, const ByteColumn &column, size_t col_index);
Word get_row(const ByteBlock &block, size_t row_index);
void set_row(ByteBlock &block, const Word &row, size_t row_index);

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

template <typename CastType, typename KeyScheduleType>
std::ostream &pretty_print(std::ostream &out, const KeyScheduleType &input) {
  constexpr size_t KEY_SCHEDULE_SIZE_WORDS = std::tuple_size<KeyScheduleType>{};
  for (size_t round_index = 0;
       round_index < KEY_SCHEDULE_SIZE_WORDS / BLOCK_SIZE_WORDS;
       ++round_index) {
    for (size_t col_index = 0; col_index < BLOCK_SIZE_WORDS; ++col_index) {
      pretty_print<CastType>(
          out, input[(BLOCK_SIZE_WORDS * round_index) + col_index]);
    }
    out << std::endl;
  }
  return out;
}

ByteBlock from_raw_bytes_to_byte_block(const RawBytes &input,
                                       size_t block_number = 0);
void from_byte_block_to_raw_bytes(const ByteBlock &input, RawBytes &output,
                                  size_t block_number = 0,
                                  bool is_last_block = false);
RawBytes from_byte_block_to_raw_bytes(const ByteBlock &input);

AES128Key gen_aes128_key(const RawBytes &flat_key);
AES192Key gen_aes192_key(const RawBytes &flat_key);
AES256Key gen_aes256_key(const RawBytes &flat_key);

AES128KeySchedule gen_key_schedule(const AES128Key &key);
AES192KeySchedule gen_key_schedule(const AES192Key &key);
AES256KeySchedule gen_key_schedule(const AES256Key &key);

void AES_128_cipher(ByteBlock &input, ByteBlock &output,
                    const AES128KeySchedule &key_schedule);

void AES_192_cipher(ByteBlock &input, ByteBlock &output,
                    const AES192KeySchedule &key_schedule);

void AES_256_cipher(ByteBlock &input, ByteBlock &output,
                    const AES256KeySchedule &key_schedule);

void AES_128_inv_cipher(ByteBlock &input, ByteBlock &output,
                        const AES128KeySchedule &key_schedule);

void AES_192_inv_cipher(ByteBlock &input, ByteBlock &output,
                        const AES192KeySchedule &key_schedule);

void AES_256_inv_cipher(ByteBlock &input, ByteBlock &output,
                        const AES256KeySchedule &key_schedule);
