#pragma once

#include <util.hpp>

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

using RawBytes = std::vector<uint8_t>;

RawBytes from_hex_string(const std::string &input);

char to_base64_char(uint8_t input);

uint8_t top_nibble(uint8_t input);

uint8_t bottom_nibble(uint8_t input);

char to_hex_char(uint8_t input);

std::ostream &to_hex_string(std::ostream &out, const RawBytes &input);

std::ostream &to_hex_string_in_blocks(std::ostream &out, const RawBytes &input,
                                      size_t block_size_bytes);

std::ostream &to_base64_string(std::ostream &out, const RawBytes &input);

std::ostream &to_ascii_string_in_blocks(std::ostream &out,
                                        const RawBytes &input,
                                        size_t block_size_bytes);

std::ostream &to_ascii_string(std::ostream &out, const RawBytes &input);

RawBytes from_ascii_string(const std::string &input);

uint8_t from_base64_char(uint8_t input);

RawBytes from_base64_string(const std::string &input);

RawBytes prepend_bytes(const RawBytes &original, const RawBytes &prefix);

RawBytes operator^(const RawBytes &input_1, const RawBytes &input_2);

RawBytes operator^(const RawBytes &input, uint8_t key);
