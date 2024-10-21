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

int main() {
  std::cout << "Cryptopals set2" << std::endl;

  c9();

  return 0;
}
