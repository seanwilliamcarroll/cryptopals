#include <aes.hpp>

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

template <typename ContainerType>
ByteBlock to_block(const ContainerType &input, const size_t block_number = 0) {
  // input is assumed to be a flat stream of datatype
  ByteBlock output;
  const size_t begin_block_byte = block_number * BLOCK_SIZE_BYTES;
  const size_t end_block_byte = (block_number + 1) * BLOCK_SIZE_BYTES;

  for (size_t iter = begin_block_byte; iter < end_block_byte; ++iter) {
    const size_t column_index = iter % WORD_SIZE_BYTES;
    const size_t row_index = iter / WORD_SIZE_BYTES;
    output[column_index][row_index] = input[iter];
  }
  return output;
}

template <typename ContainerType>
void from_block(const ByteBlock &input, ContainerType &output,
                const size_t block_number = 0) {
  const size_t flat_offset = block_number * BLOCK_SIZE_BYTES;
  for (size_t col_index = 0; col_index < WORD_SIZE_BYTES; ++col_index) {
    for (size_t row_index = 0; row_index < BLOCK_SIZE_WORDS; ++row_index) {
      const size_t flat_index = col_index + (row_index * WORD_SIZE_BYTES);
      output[flat_offset + flat_index] = input[col_index][row_index];
    }
  }
}

template <typename ContainerType>
ContainerType from_block(const ByteBlock &input) {
  ContainerType output(BLOCK_SIZE_BYTES, uint8_t());
  from_block(input, output, 0);
  return output;
}

// External API functions

ByteBlock from_raw_bytes_to_byte_block(const RawBytes &input,
                                       const size_t block_number) {
  return to_block<RawBytes>(input, block_number);
}

void from_byte_block_to_raw_bytes(const ByteBlock &input, RawBytes &output,
                                  const size_t block_number) {
  from_block<RawBytes>(input, output, block_number);
}

RawBytes from_byte_block_to_raw_bytes(const ByteBlock &input) {
  return from_block<RawBytes>(input);
}

void transpose(ByteBlock &input) {
  ByteBlock output;
  for (size_t col_index = 0; col_index < WORD_SIZE_BYTES; ++col_index) {
    for (size_t row_index = 0; row_index < BLOCK_SIZE_WORDS; ++row_index) {
      output[row_index][col_index] = input[col_index][row_index];
    }
  }
  input = output;
}

void rot_word(ByteColumn &input) {
  input = {input[1], input[2], input[3], input[0]};
}

void rot_word(ByteBlock &input, const size_t column_index) {
  rot_word(input[column_index]);
}

void inv_rot_word(ByteColumn &input) {
  input = {input[3], input[0], input[1], input[2]};
}

void inv_rot_word(ByteBlock &input, const size_t column_index) {
  inv_rot_word(input[column_index]);
}

constexpr std::array<std::array<uint8_t, 16>, 16> S_BOX = {
    {{0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b,
      0xfe, 0xd7, 0xab, 0x76},
     {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf,
      0x9c, 0xa4, 0x72, 0xc0},
     {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1,
      0x71, 0xd8, 0x31, 0x15},
     {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2,
      0xeb, 0x27, 0xb2, 0x75},
     {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3,
      0x29, 0xe3, 0x2f, 0x84},
     {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39,
      0x4a, 0x4c, 0x58, 0xcf},
     {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f,
      0x50, 0x3c, 0x9f, 0xa8},
     {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21,
      0x10, 0xff, 0xf3, 0xd2},
     {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d,
      0x64, 0x5d, 0x19, 0x73},
     {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14,
      0xde, 0x5e, 0x0b, 0xdb},
     {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62,
      0x91, 0x95, 0xe4, 0x79},
     {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea,
      0x65, 0x7a, 0xae, 0x08},
     {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f,
      0x4b, 0xbd, 0x8b, 0x8a},
     {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9,
      0x86, 0xc1, 0x1d, 0x9e},
     {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9,
      0xce, 0x55, 0x28, 0xdf},
     {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
      0xb0, 0x54, 0xbb, 0x16}}};

constexpr std::array<std::array<uint8_t, 16>, 16> INV_S_BOX = {
    {{0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e,
      0x81, 0xf3, 0xd7, 0xfb},
     {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44,
      0xc4, 0xde, 0xe9, 0xcb},
     {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b,
      0x42, 0xfa, 0xc3, 0x4e},
     {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49,
      0x6d, 0x8b, 0xd1, 0x25},
     {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc,
      0x5d, 0x65, 0xb6, 0x92},
     {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57,
      0xa7, 0x8d, 0x9d, 0x84},
     {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05,
      0xb8, 0xb3, 0x45, 0x06},
     {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03,
      0x01, 0x13, 0x8a, 0x6b},
     {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce,
      0xf0, 0xb4, 0xe6, 0x73},
     {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8,
      0x1c, 0x75, 0xdf, 0x6e},
     {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e,
      0xaa, 0x18, 0xbe, 0x1b},
     {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe,
      0x78, 0xcd, 0x5a, 0xf4},
     {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59,
      0x27, 0x80, 0xec, 0x5f},
     {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f,
      0x93, 0xc9, 0x9c, 0xef},
     {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c,
      0x83, 0x53, 0x99, 0x61},
     {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63,
      0x55, 0x21, 0x0c, 0x7d}}};

uint8_t do_lookup(const std::array<std::array<uint8_t, 16>, 16> &lookup_table,
                  uint8_t input) {
  return lookup_table[(input >> 4) & 0xF][input & 0xF];
}

void base_sub_word(const std::array<std::array<uint8_t, 16>, 16> &lookup_table,
                   ByteColumn &input) {
  input = ByteColumn{
      do_lookup(lookup_table, input[0]), do_lookup(lookup_table, input[1]),
      do_lookup(lookup_table, input[2]), do_lookup(lookup_table, input[3])};
}

void sub_word(ByteColumn &input) { base_sub_word(S_BOX, input); }

void sub_word(ByteBlock &input, const size_t column_index) {
  sub_word(input[column_index]);
}

void sub_bytes(ByteBlock &input) {
  sub_word(input, 0);
  sub_word(input, 1);
  sub_word(input, 2);
  sub_word(input, 3);
}

void inv_sub_word(ByteColumn &input) { base_sub_word(INV_S_BOX, input); }

void inv_sub_word(ByteBlock &input, const size_t column_index) {
  inv_sub_word(input[column_index]);
}

void inv_sub_bytes(ByteBlock &input) {
  inv_sub_word(input, 0);
  inv_sub_word(input, 1);
  inv_sub_word(input, 2);
  inv_sub_word(input, 3);
}

// AES128Key make_aes128_key() {}

ByteColumn do_xor(const ByteColumn &input_a, const ByteColumn &input_b) {
  return {uint8_t(input_a[0] ^ input_b[0]), uint8_t(input_a[1] ^ input_b[1]),
          uint8_t(input_a[2] ^ input_b[2]), uint8_t(input_a[3] ^ input_b[3])};
}

ByteBlock do_xor(const ByteBlock &input_a, const ByteBlock &input_b) {
  ByteBlock output;
  output[0] = do_xor(input_a[0], input_b[0]);
  output[1] = do_xor(input_a[1], input_b[1]);
  output[2] = do_xor(input_a[2], input_b[2]);
  output[3] = do_xor(input_a[3], input_b[3]);
  return output;
}

constexpr inline std::array<ByteColumn, 11> ROUND_CONSTANT = {{
    {0x0, 0, 0, 0},
    {0x1, 0, 0, 0},
    {0x2, 0, 0, 0},
    {0x4, 0, 0, 0},
    {0x8, 0, 0, 0},
    {0x10, 0, 0, 0},
    {0x20, 0, 0, 0},
    {0x40, 0, 0, 0},
    {0x80, 0, 0, 0},
    {0x1B, 0, 0, 0},
    {0x36, 0, 0, 0},
}};

template <typename KeyScheduleType, typename KeyType>
KeyScheduleType gen_key_schedule(const KeyType &key) {
  constexpr size_t KEY_SCHEDULE_SIZE_WORDS = std::tuple_size<KeyScheduleType>{};
  constexpr size_t KEY_SIZE_WORDS = std::tuple_size<KeyType>{};
  KeyScheduleType key_schedule_words;

  size_t index = 0;
  while (index < KEY_SIZE_WORDS) {
    key_schedule_words[index] = key[index];
    ++index;
  }

  index = KEY_SIZE_WORDS;
  while (index < KEY_SCHEDULE_SIZE_WORDS) {
    ByteColumn temp = key_schedule_words[index - 1];
    if (index % KEY_SIZE_WORDS == 0) {
      rot_word(temp);
      sub_word(temp);
      temp = do_xor(temp, ROUND_CONSTANT[index / KEY_SIZE_WORDS]);
    } else if (KEY_SIZE_WORDS > 6 && index % KEY_SIZE_WORDS == 4) {
      sub_word(temp);
    }
    key_schedule_words[index] =
        do_xor(key_schedule_words[index - KEY_SIZE_WORDS], temp);
    ++index;
  }
  return key_schedule_words;
}

AES128KeySchedule gen_key_schedule(const AES128Key &key) {
  return gen_key_schedule<AES128KeySchedule, AES128Key>(key);
}

AES192KeySchedule gen_key_schedule(const AES192Key &key) {
  return gen_key_schedule<AES192KeySchedule, AES192Key>(key);
}

AES256KeySchedule gen_key_schedule(const AES256Key &key) {
  return gen_key_schedule<AES256KeySchedule, AES256Key>(key);
}

template <typename KeyType> KeyType gen_key(const RawBytes &flat_key) {
  constexpr size_t KEY_SIZE_WORDS = std::tuple_size<KeyType>{};
  KeyType key;
  for (size_t word_index = 0; word_index < KEY_SIZE_WORDS; ++word_index) {
    for (size_t byte_index = 0; byte_index < WORD_SIZE_BYTES; ++byte_index) {
      size_t flat_index = (word_index * KEY_SIZE_WORDS) + byte_index;
      key[word_index][byte_index] = flat_key[flat_index];
    }
  }
  return key;
}

AES128Key gen_aes128_key(const RawBytes &flat_key) {
  return gen_key<AES128Key>(flat_key);
}

AES192Key gen_aes192_key(const RawBytes &flat_key) {
  return gen_key<AES192Key>(flat_key);
}

AES256Key gen_aes256_key(const RawBytes &flat_key) {
  return gen_key<AES256Key>(flat_key);
}

void shift_row(ByteBlock &input, const size_t row_index,
               const size_t shift_amount) {
  ByteColumn output_row;
  for (size_t col_index = 0; col_index < BLOCK_SIZE_WORDS; ++col_index) {
    output_row[col_index] =
        input[row_index][(col_index + shift_amount) % BLOCK_SIZE_WORDS];
  }
  for (size_t col_index = 0; col_index < BLOCK_SIZE_WORDS; ++col_index) {
    input[row_index][col_index] = output_row[col_index];
  }
}

void shift_rows(ByteBlock &input) {
  shift_row(input, 1, 1);
  shift_row(input, 2, 2);
  shift_row(input, 3, 3);
}

void mix_column(ByteColumn &input) {
  ByteColumn input_copy(input);
  ByteColumn input_mul2;

  for (size_t index = 0; index < WORD_SIZE_BYTES; ++index) {
    // Galois multiply by 2
    const uint8_t is_high_bit_set = (input[index] >> 7) & 0x1;
    input_mul2[index] = input[index] << 1;
    if (is_high_bit_set) {
      input_mul2[index] ^= 0x1B;
    }
  }

  // input_copy ^ input_mul2 is multiplying by 3 in Galois field
  input[0] = input_mul2[0] ^ input_copy[3] ^ input_copy[2] ^ input_mul2[1] ^
             input_copy[1];
  input[1] = input_mul2[1] ^ input_copy[0] ^ input_copy[3] ^ input_mul2[2] ^
             input_copy[2];
  input[2] = input_mul2[2] ^ input_copy[1] ^ input_copy[0] ^ input_mul2[3] ^
             input_copy[3];
  input[3] = input_mul2[3] ^ input_copy[2] ^ input_copy[1] ^ input_mul2[0] ^
             input_copy[0];
}

void mix_columns(ByteBlock &input) {
  // ?????
  transpose(input);
  mix_column(input[0]);
  mix_column(input[1]);
  mix_column(input[2]);
  mix_column(input[3]);
  transpose(input);
}

template <typename KeyScheduleType>
ByteBlock get_round_key(const KeyScheduleType &key_schedule,
                        const size_t round_index) {
  ByteBlock round_key = {{
      key_schedule[(round_index * BLOCK_SIZE_WORDS)],
      key_schedule[(round_index * BLOCK_SIZE_WORDS) + 1],
      key_schedule[(round_index * BLOCK_SIZE_WORDS) + 2],
      key_schedule[(round_index * BLOCK_SIZE_WORDS) + 3],
  }};
  transpose(round_key);
  return round_key;
}

template <typename KeyScheduleType>
void add_round_key(ByteBlock &input, const KeyScheduleType &key_schedule,
                   const size_t round_index) {
  const ByteBlock round_key = get_round_key(key_schedule, round_index);
  input = do_xor(input, round_key);
}

void quick_print(const std::string &msg, const ByteBlock &state) {
  RawBytes state_raw = from_byte_block_to_raw_bytes(state);
  std::cout << msg << "  :  ";
  to_hex_string(std::cout, state_raw) << std::endl;
}

template <typename KeyScheduleType>
void AES_cipher(ByteBlock &input, ByteBlock &output,
                const KeyScheduleType &key_schedule) {
  constexpr size_t KEY_SCHEDULE_SIZE_WORDS = std::tuple_size<KeyScheduleType>{};
  constexpr size_t NUM_ROUNDS =
      (KEY_SCHEDULE_SIZE_WORDS / BLOCK_SIZE_WORDS) - 1;

  ByteBlock state(input);

  quick_print("round[ 0].input", state);

  add_round_key(state, key_schedule, 0);

  quick_print("round[ 0].k_sch",
              get_round_key<KeyScheduleType>(key_schedule, 0));

  for (size_t round_index = 1; round_index < NUM_ROUNDS; ++round_index) {
    quick_print("round[" + std::string(round_index < 10 ? " " : "") +
                    std::to_string(round_index) + "].input",
                state);

    sub_bytes(state);
    quick_print("round[" + std::string(round_index < 10 ? " " : "") +
                    std::to_string(round_index) + "].s_box",
                state);

    shift_rows(state);
    quick_print("round[" + std::string(round_index < 10 ? " " : "") +
                    std::to_string(round_index) + "].s_row",
                state);

    mix_columns(state);
    quick_print("round[" + std::string(round_index < 10 ? " " : "") +
                    std::to_string(round_index) + "].m_col",
                state);

    add_round_key(state, key_schedule, round_index);
    quick_print("round[" + std::string(round_index < 10 ? " " : "") +
                    std::to_string(round_index) + "].k_sch",
                get_round_key<KeyScheduleType>(key_schedule, round_index));
  }
  quick_print("round[" + std::to_string(NUM_ROUNDS) + "].input", state);
  sub_bytes(state);
  shift_rows(state);
  add_round_key(state, key_schedule, NUM_ROUNDS);

  output = state;
  quick_print("output         ", state);
}

void AES_128_cipher(ByteBlock &input, ByteBlock &output,
                    const AES128KeySchedule &key_schedule) {
  AES_cipher(input, output, key_schedule);
}

void AES_192_cipher(ByteBlock &input, ByteBlock &output,
                    const AES192KeySchedule &key_schedule) {
  AES_cipher(input, output, key_schedule);
}

void AES_256_cipher(ByteBlock &input, ByteBlock &output,
                    const AES256KeySchedule &key_schedule) {
  AES_cipher(input, output, key_schedule);
}
