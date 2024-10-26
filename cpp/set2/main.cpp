#include <functional>
#include <iostream>

#include <crypt.hpp>

void padding_test(const size_t block_size_bytes) {
  const std::string input_key = "YELLOW SUBMARINE";
  RawBytes key_raw = from_ascii_string(input_key);

  RawBytes key_raw_block_size_bytes =
      add_pkcs7_padding(key_raw, block_size_bytes);

  std::cout << "------------------------------------------------------------"
            << std::endl;
  std::cout << "Block size bytes for padding: " << block_size_bytes
            << std::endl;

  std::cout << "Key: " << std::endl;
  to_hex_string(std::cout, key_raw) << std::endl;

  std::cout << "Key: " << std::endl;
  to_ascii_string(std::cout, key_raw) << std::endl;

  std::cout << "Key_block_size_bytes: " << std::endl;
  to_hex_string(std::cout, key_raw_block_size_bytes) << std::endl;

  std::cout << "Key_block_size_bytes: " << std::endl;
  to_ascii_string(std::cout, key_raw_block_size_bytes) << std::endl;

  RawBytes key_raw_remove_padding =
      remove_pkcs7_padding(key_raw_block_size_bytes, block_size_bytes);

  std::cout << "Key_remove_padding: " << std::endl;
  to_hex_string(std::cout, key_raw_remove_padding) << std::endl;

  std::cout << "Key_remove_padding: " << std::endl;
  to_ascii_string(std::cout, key_raw_remove_padding) << std::endl;

  std::cout << "------------------------------------------------------------"
            << std::endl;
}

void c9() {
  padding_test(1);
  padding_test(16);
  padding_test(20);
  padding_test(15);
}

void c10() {
  const std::string base64_input =
      load_and_strip("/Users/sean/cryptopals/cpp/set2/10.txt");
  const RawBytes ciphertext_raw = from_base64_string(base64_input);
  const std::string input_key = "YELLOW SUBMARINE";
  const RawBytes key_raw = from_ascii_string(input_key);
  const RawBytes iv_raw(16, 0);

  const RawBytes plaintext_raw =
      AES_128_CBC_decrypt(ciphertext_raw, key_raw, iv_raw);
  std::cout << "Decrypted text:" << std::endl << std::endl;
  to_ascii_string(std::cout, plaintext_raw) << std::endl << std::endl;
}

void c11_gen_rand_keys() {
  for (size_t index = 0; index < 10; ++index) {
    const AES128Key key = gen_rand_aes128_key();
    const RawBytes key_raw = from_aes_128_key_to_raw_bytes(key);

    std::cout << "Random key as hex string:" << std::endl;
    to_hex_string(std::cout, key_raw) << std::endl;
  }
}

void c11() {
  // c11_gen_rand_keys();

  const RawBytes plaintext_raw(256, 'X');

  std::cout << "Plaintext:" << std::endl << std::endl;
  to_ascii_string(std::cout, plaintext_raw) << std::endl << std::endl;
  for (size_t index = 0; index < 10; ++index) {
    std::cout << "--------------------------------" << std::endl;
    const RawBytes new_ciphertext_raw = AES_128_rand_encrypt(plaintext_raw);

    if (detect_ecb(new_ciphertext_raw)) {
      std::cout << "Detected ECB" << std::endl;
    } else {
      std::cout << "Detected CBC" << std::endl;
    }
    std::cout << "--------------------------------" << std::endl << std::endl;
  }
}

void c12() {
  const RawBytes target_plaintext_raw = from_base64_string(
      "Um9sbGluJyBpbiBteSA1LjAKV2l0aCBteSByYWctdG9wIGRvd24gc28gbXkgaGFpciBjYW4g"
      "YmxvdwpUaGUgZ2lybGllcyBvbiBzdGFuZGJ5IHdhdmluZyBqdXN0IHRvIHNheSBoaQpEaWQg"
      "eW91IHN0b3A/IE5vLCBJIGp1c3QgZHJvdmUgYnkK");

  c_AES128SecretKeyEncrypter encrypter;

  auto do_encrypt_base = [&encrypter](const RawBytes &input_raw) {
    return encrypter.encrypt(input_raw);
  };
  auto do_encrypt_prefix = [&encrypter](const RawBytes &input_raw,
                                        const RawBytes &prefix_raw) {
    return encrypter.encrypt(input_raw, prefix_raw);
  };

  const size_t block_size_bytes = detect_block_size(do_encrypt_base);

  std::cout << "Detected block size: " << block_size_bytes << std::endl;

  const bool is_ecb =
      detect_ecb(do_encrypt_base(RawBytes(block_size_bytes * 5, 'X')));

  const size_t target_plaintext_length_bytes = detect_length_bytes(
      block_size_bytes, target_plaintext_raw, do_encrypt_prefix);

  std::cout << "Target plaintext length: " << target_plaintext_length_bytes
            << std::endl;

  std::cout << "Is ECB: " << is_ecb << std::endl;

  std::cout << "Break ECB" << std::endl;
  const RawBytes decrypted_raw =
      break_ecb_byte_at_a_time(block_size_bytes, target_plaintext_length_bytes,
                               encrypter, target_plaintext_raw);

  std::cout << "Plaintext after breaking ECB: " << std::endl;
  to_ascii_string(std::cout, decrypted_raw) << std::endl;
}

int main() {
  std::cout << "Cryptopals set2" << std::endl;

  // c9();
  // c10();
  // c11();
  c12();

  return 0;
}
