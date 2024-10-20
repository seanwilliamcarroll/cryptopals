#include <crypt.hpp>

#include <openssl/ssl.h>

#include <iomanip>
#include <iostream>

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

void c1() {
  const std::string input("49276d206b696c6c696e6720796f757220627261696e206c696b"
                          "65206120706f69736f6e6f7573206d757368726f6f6d");

  RawBytes output = from_hex_string(input);

  std::cout << "Input    hex string: " << input << std::endl;
  std::cout << "Recreate hex string: ";
  to_hex_string(std::cout, output) << std::endl;
  std::cout << "New   Base64 string: ";
  to_base64_string(std::cout, output) << std::endl;
}

void c2() {
  const std::string input_1("1c0111001f010100061a024b53535009181c");
  const std::string input_2("686974207468652062756c6c277320657965");

  RawBytes output_1 = from_hex_string(input_1);
  RawBytes output_2 = from_hex_string(input_2);

  RawBytes output_xored = do_xor(output_1, output_2);

  std::cout << "Input hex string: " << input_1 << std::endl;
  std::cout << "Input hex string: " << input_2 << std::endl;
  std::cout << "XOR'd hex string: ";
  to_hex_string(std::cout, output_xored) << std::endl;
}

void c3() {
  const std::string input(
      "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736");
  RawBytes output = from_hex_string(input);
  std::cout << "Input    hex string: " << input << std::endl;
  std::cout << "Input  ascii string: ";
  to_ascii_string(std::cout, output) << std::endl;

  const auto [winner, score] = find_likely_single_xor(output);
  RawBytes xord_output = do_xor(output, char(winner));
  std::cout << "XOR with: " << std::to_string(winner) << " (" << winner << ")"
            << std::endl;
  std::cout << "Output ascii string: ";
  to_ascii_string(std::cout, xord_output) << std::endl;
}

void c4() {
  const std::vector<std::string> input_strings =
      load_lines_from_file("/Users/sean/cryptopals/cpp/set1/4.txt");

  std::string best_string;
  double best_score = std::numeric_limits<double>::max();
  char best_winner = 0;
  for (const auto &input_str : input_strings) {
    RawBytes output = from_hex_string(input_str);
    const auto [winner, score] = find_likely_single_xor(output);
    if (score < best_score) {
      best_score = score;
      best_winner = winner;
      best_string = input_str;
    }
  }
  RawBytes output = from_hex_string(best_string);
  std::cout << "Input    hex string: " << best_string << std::endl;
  std::cout << "Input  ascii string: ";
  to_ascii_string(std::cout, output) << std::endl;
  RawBytes xord_output = do_xor(output, char(best_winner));
  std::cout << "XOR with: " << std::to_string(best_winner) << " ("
            << best_winner << ")"
            << " with Score: " << best_score << std::endl;
  std::cout << "Output ascii string: ";
  to_ascii_string(std::cout, xord_output) << std::endl;
}

void c5() {
  const std::string input("Burning 'em, if you ain't quick and nimble\nI go "
                          "crazy when I hear a cymbal");

  RawBytes plain_text = from_ascii_string(input);
  RawBytes key = from_ascii_string(std::string("ICE"));

  RawBytes output = encrypt_repeating_xor(plain_text, key);

  std::cout << "Output hex string: ";
  to_hex_string(std::cout, output) << std::endl;
}

void c6() {
  const std::string base64_input =
      load_and_strip("/Users/sean/cryptopals/cpp/set1/6.txt");
  const RawBytes raw_input = from_base64_string(base64_input);

  const size_t likely_key_length = find_likely_key_length(raw_input, 2, 40);
  std::cout << "Likely Key length: " << likely_key_length << std::endl;

  const RawBytes likely_key = find_likely_key(raw_input, likely_key_length);
  std::cout << "Likely Key: ";
  to_ascii_string(std::cout, likely_key) << std::endl;

  std::cout << "Decrypted text:" << std::endl << std::endl;
  const RawBytes decrypted_raw = encrypt_repeating_xor(raw_input, likely_key);
  to_ascii_string(std::cout, decrypted_raw) << std::endl << std::endl;
}

void c7_easy() {
  const std::string base64_input =
      load_and_strip("/Users/sean/cryptopals/cpp/set1/7.txt");
  RawBytes raw_input = from_base64_string(base64_input);
  const std::string input_key = "YELLOW SUBMARINE";
  RawBytes raw_key = from_ascii_string(input_key);

  EVP_CIPHER_CTX *ctx;

  EVP_CIPHER *cipher = EVP_CIPHER_fetch(NULL, "AES-ECB-128", NULL);

  if (!(ctx = EVP_CIPHER_CTX_new())) {
    throw std::runtime_error("Failed to new cipher ctx!");
  }

  if (1 !=
      EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, raw_key.data(), NULL)) {
    throw std::runtime_error("Failed to decrypt init!");
  }

  int plaintext_len;
  int len;
  unsigned char plaintext[4096];

  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, raw_input.data(),
                             raw_input.size())) {
    throw std::runtime_error("Failed to decrypt update!");
  }
  plaintext_len = len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
    throw std::runtime_error("Failed to decrypt finalize!");
  }
  plaintext_len += len;

  std::cout << "Decrypted length: " << plaintext_len << std::endl;
  std::cout << plaintext << std::endl;

  // Cleanup

  EVP_CIPHER_free(cipher);
  EVP_CIPHER_CTX_free(ctx);
}

void c7_my_aes() {
  const std::string base64_input =
      load_and_strip("/Users/sean/cryptopals/cpp/set1/7.txt");
  RawBytes ciphertext_raw = from_base64_string(base64_input);
  const std::string input_key = "YELLOW SUBMARINE";
  RawBytes key_raw = from_ascii_string(input_key);

  auto aes_128_key = gen_aes128_key(key_raw);
  auto aes_128_key_schedule = gen_key_schedule(aes_128_key);

  RawBytes plaintext_raw(ciphertext_raw.size());

  const size_t num_blocks =
      std::ceil<size_t>(double(ciphertext_raw.size()) / BLOCK_SIZE_BYTES);
  for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
    ByteBlock ciphertext =
        from_raw_bytes_to_byte_block(ciphertext_raw, block_index);
    ByteBlock plaintext;
    AES_128_inv_cipher(ciphertext, plaintext, aes_128_key_schedule);

    const bool is_last_block = (block_index == (num_blocks - 1));

    from_byte_block_to_raw_bytes(plaintext, plaintext_raw, block_index,
                                 is_last_block);
  }
  std::cout << "Decrypted text:" << std::endl << std::endl;
  to_ascii_string(std::cout, plaintext_raw) << std::endl << std::endl;
}

void c7() {

  // c7_easy();

  {
    const std::string base64_input =
        load_and_strip("/Users/sean/cryptopals/cpp/set1/7.txt");
    RawBytes raw_input = from_base64_string(base64_input);
    const std::string input_key = "YELLOW SUBMARINE";
    RawBytes raw_key = from_ascii_string(input_key);

    ByteBlock block = from_raw_bytes_to_byte_block(raw_key);
    std::cout << "Key: ";
    to_ascii_string(std::cout, raw_key) << std::endl;

    std::cout << "Key as block: " << std::endl;
    pretty_print<size_t>(std::cout, block);
    std::cout << "Key as char block: " << std::endl;
    pretty_print<char>(std::cout, block);

    RawBytes reassembled_key = from_byte_block_to_raw_bytes(block);
    std::cout << "Reassembled Key: ";
    to_ascii_string(std::cout, reassembled_key) << std::endl;

    // shift_row(block, 1, 1);
    // shift_row(block, 2, 2);
    // shift_row(block, 3, 3);
    // std::cout << "Key as block: " << std::endl;
    // pretty_print<size_t>(std::cout, block);
    // std::cout << "Key as char block: " << std::endl;
    // pretty_print<char>(std::cout, block);
  }

  // {
  //   std::cout << "Mix Column: " << std::endl << std::hex;
  //   ByteColumn test_vector{0x2d, 0x26, 0x31, 0x4c};
  //   pretty_print<int>(std::cout, test_vector) << std::endl;
  //   mix_column(test_vector);
  //   pretty_print<int>(std::cout, test_vector) << std::endl;
  // }

  // {
  //   RawBytes test_vector(16, 0);
  //   auto aes_128_key = gen_aes128_key(test_vector);
  //   auto aes_128_key_schedule = gen_key_schedule(aes_128_key);
  //   std::cout << std::hex;
  //   pretty_print<int>(std::cout, aes_128_key_schedule);
  //   std::cout << std::endl;
  // }

  {
    RawBytes key_raw{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    auto aes_128_key = gen_aes128_key(key_raw);
    auto aes_128_key_schedule = gen_key_schedule(aes_128_key);
    std::cout << "Key Schedule:" << std::endl;
    std::cout << std::hex;
    pretty_print<int>(std::cout, aes_128_key_schedule);

    RawBytes plaintext_raw{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                           0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    ByteBlock plaintext = from_raw_bytes_to_byte_block(plaintext_raw);
    ByteBlock ciphertext;
    AES_128_cipher(plaintext, ciphertext, aes_128_key_schedule);

    RawBytes ciphertext_raw = from_byte_block_to_raw_bytes(ciphertext);

    std::cout << "Key: " << std::endl;
    to_hex_string(std::cout, key_raw) << std::endl;

    std::cout << "Plaintext: " << std::endl;
    to_hex_string(std::cout, plaintext_raw) << std::endl;

    std::cout << "Ciphertext: " << std::endl;
    to_hex_string(std::cout, ciphertext_raw) << std::endl;

    ByteBlock new_plaintext;
    AES_128_inv_cipher(ciphertext, new_plaintext, aes_128_key_schedule);

    RawBytes new_plaintext_raw = from_byte_block_to_raw_bytes(new_plaintext);

    std::cout << "Key: " << std::endl;
    to_hex_string(std::cout, key_raw) << std::endl;

    std::cout << "Ciphertext: " << std::endl;
    to_hex_string(std::cout, ciphertext_raw) << std::endl;

    std::cout << "Plaintext: " << std::endl;
    to_hex_string(std::cout, new_plaintext_raw) << std::endl;
  }

  c7_my_aes();
}

int main() {
  std::cout << "Cryptopals" << std::endl;
  // c1();
  // c2();
  // c3();
  // c4();
  // c5();
  // c6();
  c7();

  return 0;
}
