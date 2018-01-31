#include "utf_convert.h"
namespace small_tl
{
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

    while(current_char_part >= 0)
    {
      dst.push_back(top_bits | ((src_char >> current_char_part) & bottom_mask));
      current_char_part -= 6;
      top_bits = 0x80;
      bottom_mask = 0x3F;
    }
  }

  //helper to count the bytes in a sequence
  uint8_t bytes_in_utf8_sequence(char character)
  {
    uint8_t bytes_in_utf8_sequence = 0;
    for (uint8_t mask_location = 7; mask_location >= 5; --mask_location, ++bytes_in_utf8_sequence)
    {
      uint8_t mask = 1 << mask_location;
      uint8_t masked = mask & character;
      if (masked == 0)return std::max<uint8_t>(bytes_in_utf8_sequence,1);
    }
    return bytes_in_utf8_sequence;
  }

  //converts a utf8 string stored in std::string to a utf32 string stored in a std::u32string
  std::u32string to_utf32(const std::string &src)
  {
    std::u32string dst;
    dst.reserve(src.size());
    char32_t dst_char;
    for (std::string::const_iterator src_char = src.cbegin(); src_char != src.cend(); ++src_char)
    {
      uint8_t bytes = bytes_in_utf8_sequence(*src_char);
      uint8_t position = 0;
      switch (bytes)
      {
      case 1:
        dst_char = (uint8_t)(*src_char);
        break;
      case 2:
        position = 6;
        dst_char = (((uint8_t)*src_char) & 0x1F) << position;
        break;
      case 3:
        position = 12;
        dst_char = (((uint8_t)*src_char) & 0x0F) << position;
        break;
      case 4:
        position = 18;
        dst_char = (((uint8_t)*src_char) & 0x07) << position;
        break;
      }

      bool valid_chain = true;
      uint8_t index = 0;
      while(position > 0)
      {
        ++index;

        std::string::const_iterator next_char = src_char + index;
        if (next_char == src.cend() || ((*next_char & 0xC0) != 0x80))
        {
          dst_char = (uint8_t)(*src_char);
          valid_chain = false;
          break;
        }

        position -= 6;
        dst_char |= (((uint8_t)*next_char) & 0x3F) << position;
      }
      if (valid_chain)
      {
        src_char += index;
      }
      dst.push_back(dst_char);
    }
    return dst;
  }
}