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

template <typename DataType, typename ContainerType>
Block<DataType> to_block(const ContainerType &input,
                         const size_t block_number = 0) {
  // input is assumed to be a flat stream of datatype
  Block<DataType> output;
  const size_t begin_block_byte = block_number * BLOCK_SIZE_ENTRIES;
  const size_t end_block_byte = (block_number + 1) * BLOCK_SIZE_ENTRIES;

  for (size_t iter = begin_block_byte; iter < end_block_byte; ++iter) {
    const size_t column_index = iter % WORD_SIZE_ENTRIES;
    const size_t row_index = iter / WORD_SIZE_ENTRIES;
    output[column_index][row_index] = input[iter];
  }
  return output;
}

template <typename ContainerType, typename DataType>
void from_block(const Block<DataType> &input, ContainerType &output,
                const size_t block_number = 0) {
  const size_t flat_offset = block_number * BLOCK_SIZE_ENTRIES;
  for (size_t col_index = 0; col_index < WORD_SIZE_ENTRIES; ++col_index) {
    for (size_t row_index = 0; row_index < BLOCK_SIZE_WORDS; ++row_index) {
      const size_t flat_index = col_index + (row_index * WORD_SIZE_ENTRIES);
      output[flat_offset + flat_index] = input[col_index][row_index];
    }
  }
}

template <typename ContainerType, typename DataType>
ContainerType from_block(const Block<DataType> &input) {
  ContainerType output(BLOCK_SIZE_ENTRIES, DataType());
  from_block(input, output, 0);
  return output;
}

// External API functions

ByteBlock from_raw_bytes_to_byte_block(const RawBytes &input,
                                       const size_t block_number) {
  return to_block<uint8_t>(input, block_number);
}

void from_byte_block_to_raw_bytes(const ByteBlock &input, RawBytes &output,
                                  const size_t block_number) {
  from_block<RawBytes>(input, output, block_number);
}

RawBytes from_byte_block_to_raw_bytes(const ByteBlock &input) {
  return from_block<RawBytes>(input);
}
