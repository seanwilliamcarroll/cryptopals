#include <crypt.hpp>
#include <freq_map.hpp>
#include <raw_bytes.hpp>
#include <util.hpp>

#include <iostream>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
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
      load_from_file("/Users/sean/cryptopals/cpp/set1/6.txt");
  const std::string base64_input_stripped = strip_newlines(base64_input);
  const RawBytes raw_input = from_base64_string(base64_input_stripped);

  const size_t likely_key_length = find_likely_key_length(raw_input, 2, 40);
  std::cout << "Likely Key length: " << likely_key_length << std::endl;

  const RawBytes likely_key = find_likely_key(raw_input, likely_key_length);
  std::cout << "Likely Key: ";
  to_ascii_string(std::cout, likely_key) << std::endl;

  std::cout << "Decrypted text:" << std::endl << std::endl;
  const RawBytes decrypted_raw = encrypt_repeating_xor(raw_input, likely_key);
  to_ascii_string(std::cout, decrypted_raw) << std::endl << std::endl;
}

int main() {
  std::cout << "Cryptopals" << std::endl;
  // c1();
  // c2();
  // c3();
  // c4();
  // c5();
  c6();

  return 0;
}
