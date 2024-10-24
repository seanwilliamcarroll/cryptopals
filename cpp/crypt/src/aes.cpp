#include <aes.hpp>
#include <rand.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
    Word temp = key_schedule_words[index - 1];
    if (index % KEY_SIZE_WORDS == 0) {
      rot_word(temp);
      sub_word(temp);
      temp = (temp ^ ROUND_CONSTANT[index / KEY_SIZE_WORDS]);
    } else if (KEY_SIZE_WORDS > 6 && index % KEY_SIZE_WORDS == 4) {
      sub_word(temp);
    }
    key_schedule_words[index] =
        (key_schedule_words[index - KEY_SIZE_WORDS] ^ temp);
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

void quick_print(const size_t round_index, const std::string &msg,
                 const ByteBlock &state) {
  RawBytes state_raw = from_byte_block_to_raw_bytes(state);
  std::cout << "round[" << std::string(round_index < 10 ? " " : "")
            << std::to_string(round_index) << "]." << msg << "  :  ";
  to_hex_string(std::cout, state_raw) << std::endl;
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
                    const AES128KeySchedule &key_schedule) {
  AES_cipher(input, output, key_schedule);
}

void AES_192_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES192KeySchedule &key_schedule) {
  AES_cipher(input, output, key_schedule);
}

void AES_256_cipher(const ByteBlock &input, ByteBlock &output,
                    const AES256KeySchedule &key_schedule) {
  AES_cipher(input, output, key_schedule);
}

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
                        const AES128KeySchedule &key_schedule) {
  AES_inv_cipher(input, output, key_schedule);
}

void AES_192_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES192KeySchedule &key_schedule) {
  AES_inv_cipher(input, output, key_schedule);
}

void AES_256_inv_cipher(const ByteBlock &input, ByteBlock &output,
                        const AES256KeySchedule &key_schedule) {
  AES_inv_cipher(input, output, key_schedule);
}

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
                             const AES128KeySchedule &key_schedule) {
  return AES_ECB_encrypt<AES128KeySchedule>(plaintext_raw, key_schedule);
}

RawBytes AES_128_ECB_encrypt(const RawBytes &plaintext_raw,
                             const AES128Key &key) {
  const auto aes_128_key_schedule = gen_key_schedule(key);
  return AES_128_ECB_encrypt(plaintext_raw, aes_128_key_schedule);
}

RawBytes AES_128_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw) {
  const auto aes_128_key = gen_aes128_key(key_raw);
  const auto aes_128_key_schedule = gen_key_schedule(aes_128_key);
  return AES_128_ECB_encrypt(plaintext_raw, aes_128_key_schedule);
}

RawBytes AES_192_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw) {
  const auto aes_192_key = gen_aes192_key(key_raw);
  const auto aes_192_key_schedule = gen_key_schedule(aes_192_key);
  return AES_ECB_encrypt<AES192KeySchedule>(plaintext_raw,
                                            aes_192_key_schedule);
}

RawBytes AES_256_ECB_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw) {
  const auto aes_256_key = gen_aes256_key(key_raw);
  const auto aes_256_key_schedule = gen_key_schedule(aes_256_key);
  return AES_ECB_encrypt<AES256KeySchedule>(plaintext_raw,
                                            aes_256_key_schedule);
}

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
                             const RawBytes &key_raw) {
  const auto aes_128_key = gen_aes128_key(key_raw);
  const auto aes_128_key_schedule = gen_key_schedule(aes_128_key);
  return AES_ECB_decrypt<AES128KeySchedule>(ciphertext_raw,
                                            aes_128_key_schedule);
}

RawBytes AES_192_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw) {
  const auto aes_192_key = gen_aes192_key(key_raw);
  const auto aes_192_key_schedule = gen_key_schedule(aes_192_key);
  return AES_ECB_decrypt<AES192KeySchedule>(ciphertext_raw,
                                            aes_192_key_schedule);
}

RawBytes AES_256_ECB_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw) {
  const auto aes_256_key = gen_aes256_key(key_raw);
  const auto aes_256_key_schedule = gen_key_schedule(aes_256_key);
  return AES_ECB_decrypt<AES256KeySchedule>(ciphertext_raw,
                                            aes_256_key_schedule);
}

RawBytes add_pkcs7_padding(const RawBytes &input,
                           const size_t block_size_bytes) {
  const size_t length = input.size();
  const size_t additional_bytes =
      block_size_bytes - (length % block_size_bytes);
  uint8_t padding_byte = uint8_t(additional_bytes);
  if (additional_bytes == block_size_bytes) {
    padding_byte = 0;
  }
  RawBytes output(length + additional_bytes, padding_byte);
  std::copy(std::begin(input), std::end(input), std::begin(output));
  return output;
}

RawBytes remove_pkcs7_padding(const RawBytes &input,
                              const size_t block_size_bytes) {
  const size_t length = input.size();
  const size_t last_byte = size_t(input.back());
  const size_t bytes_to_remove = last_byte == 0 ? block_size_bytes : last_byte;

  RawBytes output(input);
  output.resize(length - bytes_to_remove);
  return output;
}

template <typename KeyScheduleType>
RawBytes AES_CBC_encrypt(const RawBytes &plaintext_raw,
                         const KeyScheduleType &key_schedule,
                         const ByteBlock &iv) {
  RawBytes plaintext_padded_raw =
      add_pkcs7_padding(plaintext_raw, BLOCK_SIZE_BYTES);
  RawBytes ciphertext_raw(plaintext_padded_raw.size());

  const size_t num_blocks =
      std::ceil<size_t>(double(plaintext_padded_raw.size()) / BLOCK_SIZE_BYTES);
  ByteBlock last_ciphertext = iv;
  ;
  for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
    ByteBlock plaintext =
        from_raw_bytes_to_byte_block(plaintext_padded_raw, block_index);
    plaintext = plaintext ^ last_ciphertext;
    ByteBlock ciphertext;
    AES_cipher(plaintext, ciphertext, key_schedule);

    last_ciphertext = ciphertext;

    from_byte_block_to_raw_bytes(ciphertext, ciphertext_raw, block_index);
  }
  return ciphertext_raw;
}

template <typename KeyScheduleType>
RawBytes AES_CBC_encrypt(const RawBytes &plaintext_raw,
                         const KeyScheduleType &key_schedule,
                         const RawBytes &iv_raw) {
  const ByteBlock iv = from_raw_bytes_to_byte_block(iv_raw);
  return AES_CBC_encrypt<KeyScheduleType>(plaintext_raw, key_schedule, iv);
}

RawBytes AES_128_CBC_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw) {
  const auto aes_128_key = gen_aes128_key(key_raw);
  const auto aes_128_key_schedule = gen_key_schedule(aes_128_key);
  return AES_CBC_encrypt<AES128KeySchedule>(plaintext_raw, aes_128_key_schedule,
                                            iv_raw);
}

RawBytes AES_192_CBC_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw) {
  const auto aes_192_key = gen_aes192_key(key_raw);
  const auto aes_192_key_schedule = gen_key_schedule(aes_192_key);
  return AES_CBC_encrypt<AES192KeySchedule>(plaintext_raw, aes_192_key_schedule,
                                            iv_raw);
}

RawBytes AES_256_CBC_encrypt(const RawBytes &plaintext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw) {
  const auto aes_256_key = gen_aes256_key(key_raw);
  const auto aes_256_key_schedule = gen_key_schedule(aes_256_key);
  return AES_CBC_encrypt<AES256KeySchedule>(plaintext_raw, aes_256_key_schedule,
                                            iv_raw);
}

template <typename KeyScheduleType>
RawBytes AES_CBC_decrypt(const RawBytes &ciphertext_raw,
                         const KeyScheduleType &key_schedule,
                         const RawBytes &iv_raw) {
  RawBytes plaintext_raw(ciphertext_raw.size());

  const size_t num_blocks =
      std::ceil<size_t>(double(ciphertext_raw.size()) / BLOCK_SIZE_BYTES);
  ByteBlock last_ciphertext = from_raw_bytes_to_byte_block(iv_raw);
  for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
    ByteBlock ciphertext =
        from_raw_bytes_to_byte_block(ciphertext_raw, block_index);
    ByteBlock plaintext;
    AES_inv_cipher(ciphertext, plaintext, key_schedule);

    plaintext = plaintext ^ last_ciphertext;
    last_ciphertext = ciphertext;

    from_byte_block_to_raw_bytes(plaintext, plaintext_raw, block_index);
  }
  return remove_pkcs7_padding(plaintext_raw, BLOCK_SIZE_BYTES);
}

RawBytes AES_128_CBC_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw) {
  const auto aes_128_key = gen_aes128_key(key_raw);
  const auto aes_128_key_schedule = gen_key_schedule(aes_128_key);
  return AES_CBC_decrypt<AES128KeySchedule>(ciphertext_raw,
                                            aes_128_key_schedule, iv_raw);
}

RawBytes AES_192_CBC_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw) {
  const auto aes_192_key = gen_aes192_key(key_raw);
  const auto aes_192_key_schedule = gen_key_schedule(aes_192_key);
  return AES_CBC_decrypt<AES192KeySchedule>(ciphertext_raw,
                                            aes_192_key_schedule, iv_raw);
}

RawBytes AES_256_CBC_decrypt(const RawBytes &ciphertext_raw,
                             const RawBytes &key_raw, const RawBytes &iv_raw) {
  const auto aes_256_key = gen_aes256_key(key_raw);
  const auto aes_256_key_schedule = gen_key_schedule(aes_256_key);
  return AES_CBC_decrypt<AES256KeySchedule>(ciphertext_raw,
                                            aes_256_key_schedule, iv_raw);
}

ByteBlock gen_rand_block() {
  static c_RandomByteGenerator generator;
  ByteBlock output;
  for (auto &word : output) {
    word = generator.generate_random_word();
  }
  return output;
}

AES128Key gen_rand_aes128_key() { return gen_rand_key<AES128Key>(); }

AES192Key gen_rand_aes192_key() { return gen_rand_key<AES192Key>(); }

AES256Key gen_rand_aes256_key() { return gen_rand_key<AES256Key>(); }

template <typename KeyType>
KeyType from_raw_bytes_to_aes_key(const RawBytes &input) {
  constexpr size_t NUM_WORDS = std::tuple_size<KeyType>{};
  KeyType output;
  to_word_array<RawBytes, NUM_WORDS>(input, output, 0);
  return output;
}

template <typename KeyType>
RawBytes from_aes_key_to_raw_bytes(const KeyType &input) {
  constexpr size_t NUM_WORDS = std::tuple_size<KeyType>{};
  RawBytes output(NUM_WORDS * WORD_SIZE_BYTES, 0);
  from_word_array<RawBytes, NUM_WORDS>(input, output, 0);
  return output;
}

AES128Key from_raw_bytes_to_aes_128_key(const RawBytes &input) {
  return from_raw_bytes_to_aes_key<AES128Key>(input);
}

RawBytes from_aes_128_key_to_raw_bytes(const AES128Key &input) {
  return from_aes_key_to_raw_bytes<AES128Key>(input);
}

AES192Key from_raw_bytes_to_aes_192_key(const RawBytes &input) {
  return from_raw_bytes_to_aes_key<AES192Key>(input);
}

RawBytes from_aes_192_key_to_raw_bytes(const AES192Key &input) {
  return from_aes_key_to_raw_bytes<AES192Key>(input);
}

AES256Key from_raw_bytes_to_aes_256_key(const RawBytes &input) {
  return from_raw_bytes_to_aes_key<AES256Key>(input);
}

RawBytes from_aes_256_key_to_raw_bytes(const AES256Key &input) {
  return from_aes_key_to_raw_bytes<AES256Key>(input);
}

template <typename KeyType, typename KeyScheduleType>
RawBytes AES_rand_ECB_encrypt(const RawBytes &plaintext_raw) {
  const KeyType aes_rand_key = gen_rand_key<KeyType>();
  const KeyScheduleType aes_rand_key_schedule = gen_key_schedule(aes_rand_key);
  return AES_ECB_encrypt<KeyScheduleType>(plaintext_raw, aes_rand_key_schedule);
}

template <typename KeyType, typename KeyScheduleType>
RawBytes AES_rand_CBC_encrypt(const RawBytes &plaintext_raw) {
  const KeyType aes_rand_key = gen_rand_key<KeyType>();
  const ByteBlock rand_iv = gen_rand_block();
  const KeyScheduleType aes_rand_key_schedule = gen_key_schedule(aes_rand_key);
  return AES_CBC_encrypt<KeyScheduleType>(plaintext_raw, aes_rand_key_schedule,
                                          rand_iv);
}

RawBytes AES_128_rand_ECB_encrypt(const RawBytes &plaintext_raw) {
  return AES_rand_ECB_encrypt<AES128Key, AES128KeySchedule>(plaintext_raw);
}

RawBytes AES_192_rand_ECB_encrypt(const RawBytes &plaintext_raw) {
  return AES_rand_ECB_encrypt<AES192Key, AES192KeySchedule>(plaintext_raw);
}

RawBytes AES_256_rand_ECB_encrypt(const RawBytes &plaintext_raw) {
  return AES_rand_ECB_encrypt<AES256Key, AES256KeySchedule>(plaintext_raw);
}

RawBytes AES_128_rand_CBC_encrypt(const RawBytes &plaintext_raw) {
  return AES_rand_CBC_encrypt<AES128Key, AES128KeySchedule>(plaintext_raw);
}

RawBytes AES_192_rand_CBC_encrypt(const RawBytes &plaintext_raw) {
  return AES_rand_CBC_encrypt<AES192Key, AES192KeySchedule>(plaintext_raw);
}

RawBytes AES_256_rand_CBC_encrypt(const RawBytes &plaintext_raw) {
  return AES_rand_CBC_encrypt<AES256Key, AES256KeySchedule>(plaintext_raw);
}

RawBytes AES_128_rand_encrypt(const RawBytes &plaintext_raw) {
  static c_RandomByteGenerator generator;

  const size_t num_random_bytes = generator.generate_in_range(5, 10);
  const RawBytes prefix = generator.generate_n_random_bytes(num_random_bytes);

  const RawBytes new_plaintext_raw = prepend_bytes(plaintext_raw, prefix);

  if (generator.generate_random_byte() & 0x1) {
    std::cout << "Encrypted with CBC" << std::endl;
    return AES_128_rand_CBC_encrypt(new_plaintext_raw);
  } else {
    std::cout << "Encrypted with ECB" << std::endl;
    return AES_128_rand_ECB_encrypt(new_plaintext_raw);
  }
}
