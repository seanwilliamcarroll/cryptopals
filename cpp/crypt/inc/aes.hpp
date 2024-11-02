#pragma once

#include <block.hpp>
#include <rand.hpp>
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

using AES128Key = WordArray<AES_128_KEY_LENGTH_WORDS>;
using AES192Key = WordArray<AES_192_KEY_LENGTH_WORDS>;
using AES256Key = WordArray<AES_256_KEY_LENGTH_WORDS>;

using AES128KeySchedule = WordArray<BLOCK_SIZE_WORDS *(AES_128_NUM_ROUNDS + 1)>;

using AES192KeySchedule = WordArray<BLOCK_SIZE_WORDS *(AES_192_NUM_ROUNDS + 1)>;

using AES256KeySchedule = WordArray<BLOCK_SIZE_WORDS *(AES_256_NUM_ROUNDS + 1)>;

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

AES128Key gen_aes128_key(const RawBytes &flat_key);
AES192Key gen_aes192_key(const RawBytes &flat_key);
AES256Key gen_aes256_key(const RawBytes &flat_key);

AES128KeySchedule gen_key_schedule(const AES128Key &key);
AES192KeySchedule gen_key_schedule(const AES192Key &key);
AES256KeySchedule gen_key_schedule(const AES256Key &key);

RawBytes add_pkcs7_padding(const RawBytes &input,
                           const size_t block_size_bytes = BLOCK_SIZE_BYTES);

RawBytes remove_pkcs7_padding(const RawBytes &input,
                              const size_t block_size_bytes = BLOCK_SIZE_BYTES);

template <typename KeyScheduleType>
ByteBlock get_round_key(const KeyScheduleType &key_schedule,
                        const size_t round_index) {
  ByteBlock round_key = {{
      key_schedule[(round_index * BLOCK_SIZE_WORDS)],
      key_schedule[(round_index * BLOCK_SIZE_WORDS) + 1],
      key_schedule[(round_index * BLOCK_SIZE_WORDS) + 2],
      key_schedule[(round_index * BLOCK_SIZE_WORDS) + 3],
  }};
  return round_key;
}

template <typename KeyScheduleType>
void add_round_key(ByteBlock &input, const KeyScheduleType &key_schedule,
                   const size_t round_index) {
  const ByteBlock round_key = get_round_key(key_schedule, round_index);
  input = (input ^ round_key);
}

template <typename KeyScheduleType>
void AES_cipher(const ByteBlock &input, ByteBlock &output,
                const KeyScheduleType &key_schedule) {
  constexpr size_t KEY_SCHEDULE_SIZE_WORDS = std::tuple_size<KeyScheduleType>{};
  constexpr size_t NUM_ROUNDS =
      (KEY_SCHEDULE_SIZE_WORDS / BLOCK_SIZE_WORDS) - 1;

  ByteBlock state(input);

  add_round_key(state, key_schedule, 0);

  for (size_t round_index = 1; round_index < NUM_ROUNDS; ++round_index) {
    sub_bytes(state);
    shift_rows(state);
    mix_columns(state);
    add_round_key(state, key_schedule, round_index);
  }
  sub_bytes(state);
  shift_rows(state);
  add_round_key(state, key_schedule, NUM_ROUNDS);

  output = state;
}

void AES_128_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES128KeySchedule &key_schedule);
void AES_192_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES192KeySchedule &key_schedule);
void AES_256_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES256KeySchedule &key_schedule);

template <typename KeyScheduleType>
RawBytes AES_ECB_encrypt(const RawBytes &plaintext_raw,
                         const KeyScheduleType &key_schedule) {
  RawBytes plaintext_padded_raw =
      add_pkcs7_padding(plaintext_raw, BLOCK_SIZE_BYTES);
  RawBytes ciphertext_raw(plaintext_padded_raw.size());

  const size_t num_blocks =
      std::ceil<size_t>(double(plaintext_padded_raw.size()) / BLOCK_SIZE_BYTES);
  for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
    ByteBlock plaintext =
        from_raw_bytes_to_byte_block(plaintext_padded_raw, block_index);
    ByteBlock ciphertext;
    AES_cipher(plaintext, ciphertext, key_schedule);

    from_byte_block_to_raw_bytes(ciphertext, ciphertext_raw, block_index);
  }
  return ciphertext_raw;
}

RawBytes AES_128_ECB_encrypt(const RawBytes &plaintext_raw,
                             const AES128KeySchedule &key_schedule);
RawBytes AES_128_ECB_encrypt(const RawBytes &plaintext_raw,
                             const AES128Key &key);
RawBytes AES_128_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw);

RawBytes AES_192_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw);
RawBytes AES_256_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw);

template <typename KeyScheduleType>
void AES_inv_cipher(const ByteBlock &input, ByteBlock &output,
                    const KeyScheduleType &key_schedule) {
  constexpr size_t KEY_SCHEDULE_SIZE_WORDS = std::tuple_size<KeyScheduleType>{};
  constexpr size_t NUM_ROUNDS =
      (KEY_SCHEDULE_SIZE_WORDS / BLOCK_SIZE_WORDS) - 1;

  ByteBlock state(input);

  add_round_key(state, key_schedule, NUM_ROUNDS);

  for (size_t round_index = NUM_ROUNDS - 1; round_index > 0; --round_index) {
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, key_schedule, round_index);
    inv_mix_columns(state);
  }
  inv_shift_rows(state);
  inv_sub_bytes(state);
  add_round_key(state, key_schedule, 0);

  output = state;
}

void AES_128_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES128KeySchedule &key_schedule);
void AES_192_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES192KeySchedule &key_schedule);
void AES_256_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES256KeySchedule &key_schedule);

template <typename KeyScheduleType>
RawBytes AES_ECB_decrypt(const RawBytes &ciphertext_raw,
                         const KeyScheduleType &key_schedule) {
  RawBytes plaintext_raw(ciphertext_raw.size());

  const size_t num_blocks =
      std::ceil<size_t>(double(ciphertext_raw.size()) / BLOCK_SIZE_BYTES);
  for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
    ByteBlock ciphertext =
        from_raw_bytes_to_byte_block(ciphertext_raw, block_index);
    ByteBlock plaintext;
    AES_inv_cipher(ciphertext, plaintext, key_schedule);

    from_byte_block_to_raw_bytes(plaintext, plaintext_raw, block_index);
  }
  return remove_pkcs7_padding(plaintext_raw, BLOCK_SIZE_BYTES);
}

RawBytes AES_128_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw);
RawBytes AES_192_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw);
RawBytes AES_256_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw);

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

template <typename KeyType> KeyType gen_rand_key() {
  static c_RandomByteGenerator generator;
  KeyType key;
  for (auto &word : key) {
    word = generator.generate_random_word();
  }
  return key;
}

AES128Key gen_rand_aes128_key();
AES192Key gen_rand_aes192_key();
AES256Key gen_rand_aes256_key();

AES128Key from_raw_bytes_to_aes_128_key(const RawBytes &);
RawBytes from_aes_128_key_to_raw_bytes(const AES128Key &);

AES192Key from_raw_bytes_to_aes_192_key(const RawBytes &);
RawBytes from_aes_192_key_to_raw_bytes(const AES192Key &);

AES256Key from_raw_bytes_to_aes_256_key(const RawBytes &);
RawBytes from_aes_256_key_to_raw_bytes(const AES256Key &);

RawBytes AES_128_rand_ECB_encrypt(const RawBytes &plaintext_raw);
RawBytes AES_192_rand_ECB_encrypt(const RawBytes &plaintext_raw);
RawBytes AES_256_rand_ECB_encrypt(const RawBytes &plaintext_raw);

RawBytes AES_128_rand_CBC_encrypt(const RawBytes &plaintext_raw);
RawBytes AES_192_rand_CBC_encrypt(const RawBytes &plaintext_raw);
RawBytes AES_256_rand_CBC_encrypt(const RawBytes &plaintext_raw);

RawBytes AES_128_rand_encrypt(const RawBytes &plaintext_raw);

template <typename KeyType, typename KeyScheduleType> struct c_Encrypter {
  c_Encrypter(const KeyType &key)
      : m_key(key)
      , m_key_schedule(gen_key_schedule(m_key)) {}

  c_Encrypter(const RawBytes &key_raw)
      : c_Encrypter(gen_key<KeyType>(key_raw)) {}

  RawBytes decrypt(const RawBytes &ciphertext_raw) const {
    return AES_ECB_decrypt<KeyScheduleType>(ciphertext_raw, m_key_schedule);
  }

  ByteBlock encrypt(const ByteBlock &plaintext) const {
    ByteBlock output;
    AES_cipher<KeyScheduleType>(plaintext, output, m_key_schedule);
    return output;
  }

  RawBytes encrypt(const RawBytes &plaintext_raw) const {
    return AES_ECB_encrypt<KeyScheduleType>(plaintext_raw, m_key_schedule);
  }

  RawBytes encrypt(const RawBytes &plaintext_raw,
                   const RawBytes &prefix) const {
    const RawBytes full_plaintext_raw = prepend_bytes(plaintext_raw, prefix);
    return encrypt(full_plaintext_raw);
  }

  const KeyType m_key;
  const KeyScheduleType m_key_schedule;
};

template <typename KeyType, typename KeyScheduleType>
struct c_SecretKeyEncrypter : public c_Encrypter<KeyType, KeyScheduleType> {
  c_SecretKeyEncrypter()
      : c_Encrypter<KeyType, KeyScheduleType>(gen_rand_key<KeyType>()) {}
};

using c_AES128Encrypter = c_Encrypter<AES128Key, AES128KeySchedule>;
using c_AES192Encrypter = c_Encrypter<AES192Key, AES192KeySchedule>;
using c_AES256Encrypter = c_Encrypter<AES256Key, AES256KeySchedule>;

using c_AES128SecretKeyEncrypter =
    c_SecretKeyEncrypter<AES128Key, AES128KeySchedule>;
using c_AES192SecretKeyEncrypter =
    c_SecretKeyEncrypter<AES192Key, AES192KeySchedule>;
using c_AES256SecretKeyEncrypter =
    c_SecretKeyEncrypter<AES256Key, AES256KeySchedule>;
