#include "utf_convert.h"

namespace small_tl::utf_convert
{

  //
  //helpers
  //

  utf16_order utf16_order_of_char(char16_t character)
  {
    if (character >= 0xD800)
    {
      if (character <= 0xDBFF)
        return HIGH_SURROGATE;
      else if (character <= 0xDFFF)
        return LOW_SURROGATE;
    }
    return SINGLE;
  }

  //helper to count the bytes in a sequence
  uint8_t bytes_in_utf8_sequence(char character)
  {
    uint8_t bytes_in_utf8_sequence = 0;
    for (uint8_t mask_location = 7; mask_location >= 5; --mask_location, ++bytes_in_utf8_sequence)
    {
      uint8_t mask = 1 << mask_location;
      uint8_t masked = mask & character;
      if (masked == 0)return std::max<uint8_t>(bytes_in_utf8_sequence, 1);
    }
    return bytes_in_utf8_sequence;
  }

  //
  //characters
  //

  void to_utf8_char(char32_t src_char, std::string &dst)
  {
    int8_t current_char_part = -1;
    uint8_t top_bits, bottom_mask;
    if (src_char <= 0x7F)
      dst.push_back((char)src_char);
    else if (src_char <= 0x7FF)
    {
      current_char_part = 6;
      top_bits = 0xC0;
      bottom_mask = 0x1F;
    }
    else if (src_char <= 0xFFFF)
    {
      current_char_part = 12;
      top_bits = 0xE0;
      bottom_mask = 0x0F;
    }
    else if (src_char <= 0x10FFFF)
    {
      current_char_part = 18;
      top_bits = 0xF0;
      bottom_mask = 0x07;
    }
    else
    {
      to_utf8_char(0xFFFD, dst);
    }

    while (current_char_part >= 0)
    {
      dst.push_back(top_bits | ((src_char >> current_char_part) & bottom_mask));
      current_char_part -= 6;
      top_bits = 0x80;
      bottom_mask = 0x3F;
    }
  }

  void to_utf16_char(char32_t src_char, std::u16string &dst)
  {
    if (src_char <= 0xFFFF)
      dst.push_back((char16_t)src_char);
    else
    {
      src_char -= 0x10000;
      dst.push_back(((src_char >> 10) & 0x3FF) + 0xD800);
      dst.push_back((src_char & 0x3FF) + 0xDC00);
    }
  }

  //utf8 inflate from raw chars
  template<> char32_t to_utf32_char<1>(char const * const src_char, uint8_t src_char_count)
  {
    char32_t result;
    to_utf32_char(src_char, src_char_count, result);
    return result;
  }

  //utf16 inflate from raw chars
  template<> char32_t to_utf32_char<2>(char const * const src_char, uint8_t src_char_count)
  {
    char32_t result;
    to_utf32_char((char16_t const * const)src_char, src_char_count / 2, result);
    return result;
  }

  //utf32 cast from raw chars, with overflow protection
  template<>
  char32_t to_utf32_char<4>(char const * const src_char, uint8_t src_char_count)
  {
    if (src_char_count == 4)
      return *(char32_t const * const)src_char;
    else
    {
      char32_t big_char;
      for (uint8_t i = 0; i < src_char_count; ++i)
        big_char |= ((char32_t)src_char[i]) << ((3 - i) * 8);
      return big_char;
    }
  }
}