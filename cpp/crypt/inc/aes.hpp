#pragma once

#include <block.hpp>
#include <raw_bytes.hpp>

#include <array>
#include <cstdint>
#include <iostream>

constexpr inline size_t AES_128_KEY_LENGTH_WORDS = 4;
constexpr inline size_t AES_192_KEY_LENGTH_WORDS = 6;
constexpr inline size_t AES_256_KEY_LENGTH_WORDS = 8;

constexpr inline size_t AES_128_NUM_ROUNDS = 10;
constexpr inline size_t AES_192_NUM_ROUNDS = 12;
constexpr inline size_t AES_256_NUM_ROUNDS = 14;

using AES128Key = std::array<ByteColumn, AES_128_KEY_LENGTH_WORDS>;
using AES192Key = std::array<ByteColumn, AES_192_KEY_LENGTH_WORDS>;
using AES256Key = std::array<ByteColumn, AES_256_KEY_LENGTH_WORDS>;

using AES128KeySchedule =
    std::array<ByteColumn, BLOCK_SIZE_WORDS *(AES_128_NUM_ROUNDS + 1)>;

using AES192KeySchedule =
    std::array<ByteColumn, BLOCK_SIZE_WORDS *(AES_192_NUM_ROUNDS + 1)>;

using AES256KeySchedule =
    std::array<ByteColumn, BLOCK_SIZE_WORDS *(AES_256_NUM_ROUNDS + 1)>;

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

AES128Key gen_aes128_key(const RawBytes &flat_key);
AES192Key gen_aes192_key(const RawBytes &flat_key);
AES256Key gen_aes256_key(const RawBytes &flat_key);

AES128KeySchedule gen_key_schedule(const AES128Key &key);
AES192KeySchedule gen_key_schedule(const AES192Key &key);
AES256KeySchedule gen_key_schedule(const AES256Key &key);

void AES_128_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES128KeySchedule &key_schedule);
void AES_192_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES192KeySchedule &key_schedule);
void AES_256_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES256KeySchedule &key_schedule);

RawBytes AES_128_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw);
RawBytes AES_192_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw);
RawBytes AES_256_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw);

void AES_128_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES128KeySchedule &key_schedule);
void AES_192_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES192KeySchedule &key_schedule);
void AES_256_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES256KeySchedule &key_schedule);

RawBytes AES_128_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw);
RawBytes AES_192_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw);
RawBytes AES_256_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw);

RawBytes add_pkcs7_padding(const RawBytes &input,
                           const size_t block_size_bytes = BLOCK_SIZE_BYTES);

RawBytes remove_pkcs7_padding(const RawBytes &input,
                              const size_t block_size_bytes);

RawBytes AES_128_CBC_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw);
RawBytes AES_192_CBC_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw);
RawBytes AES_256_CBC_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw);

RawBytes AES_128_CBC_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw);
RawBytes AES_192_CBC_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw);
RawBytes AES_256_CBC_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw);

AES128Key gen_rand_aes128_key();
AES192Key gen_rand_aes192_key();
AES256Key gen_rand_aes256_key();
