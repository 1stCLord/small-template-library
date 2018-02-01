#pragma once
namespace small_tl::utf_convert
{
  //
  //character convertors
  //

  void to_utf8_char(char32_t src_char, std::string &dst);

  void to_utf16_char(char32_t src_char, std::u16string &dst);

  //unsupported src type
  template<class src_char_t, typename std::enable_if<!std::is_integral<src_char_t>::value, src_char_t>::type * = nullptr>
  int8_t to_utf32_char(src_char_t const * const src_char, uint8_t max_src_char_count, char32_t &result) { static_assert(false, "src_char_t must be an integral type"); }
  template<class src_char_t, typename std::enable_if<std::is_integral<src_char_t>::value && (sizeof(src_char_t) > 2), src_char_t>::type * = nullptr>
  int8_t to_utf32_char(src_char_t const * const src_char, uint8_t max_src_char_count, char32_t &result) { static_assert(false, "src_char_t must 8 or 16 bit"); }

  //typed utf8 inflate, -1 indicates it could only read a single character but that that character implied it was part of a sequence
  template<class src_char_t, typename std::enable_if<std::is_integral<src_char_t>::value && sizeof(src_char_t) == 1, src_char_t>::type * = nullptr>
  int8_t to_utf32_char(src_char_t const * const src_char, uint8_t max_src_char_count, char32_t &result)
  {
    result = 0;
    uint8_t bytes = bytes_in_utf8_sequence(*src_char);
    uint8_t position = 0;
    switch (bytes)
    {
    case 1:
      result = (uint8_t)(*src_char);
      break;
    case 2:
      position = 6;
      result = (((uint8_t)*src_char) & 0x1F) << position;
      break;
    case 3:
      position = 12;
      result = (((uint8_t)*src_char) & 0x0F) << position;
      break;
    case 4:
      position = 18;
      result = (((uint8_t)*src_char) & 0x07) << position;
      break;
    }

    int8_t chars_read = 1;
    while (position > 0)
    {
      src_char_t const * next_char = src_char + chars_read;
      if (chars_read >= max_src_char_count || ((*next_char & 0xC0) != 0x80))
      {
        result = (uint8_t)(*src_char);
        return -1;//sequence error
      }

      ++chars_read;
      position -= 6;
      result |= (((uint8_t)*next_char) & 0x3F) << position;
    }
    return chars_read;
  }

  //typed utf16 inflate, -1 indicates it could only read a single character but that that character implied it was part of a sequence
  template<class src_char_t, typename std::enable_if<std::is_integral<src_char_t>::value && sizeof(src_char_t) == 2, src_char_t>::type * = nullptr>
  int8_t to_utf32_char(src_char_t const * const src_char, uint8_t max_src_char_count, char32_t &result)
  {
    result = *src_char;
    utf16_order surrogate = utf16_order_of_char(*src_char);

    char16_t high_surrogate, low_surrogate;
    switch (surrogate)
    {
    case SINGLE:return 1;
    case HIGH_SURROGATE:
      high_surrogate = *src_char;
      low_surrogate = *(src_char + 1);
    case LOW_SURROGATE:
      high_surrogate = *(src_char + 1);
      low_surrogate = *src_char;
    }
    utf16_order other_surrogate = utf16_order_of_char(*(src_char + 1));
    if (other_surrogate != (surrogate + 1) % 2)
      return -1;
    if (max_src_char_count < 2)
      return -1;
    result = 0x10000 + ((((char32_t)(high_surrogate - 0xD800) & 0x3FF) << 10) | ((low_surrogate - 0xDC00) & 0x3FF));
    return 2;
  }

  //unsupported raw type
  template<uint8_t bytes_in_base_type>
  char32_t to_utf32_char(char const * const src_char, uint8_t src_char_count) { static_assert(false, "unsupported conversion to utf32"); }

  //utf8 inflate from raw chars
  template<> char32_t to_utf32_char<1>(char const * const src_char, uint8_t src_char_count);

  //utf16 inflate from raw chars
  template<> char32_t to_utf32_char<2>(char const * const src_char, uint8_t src_char_count);

  //utf32 cast from raw chars, with overflow protection
  template<>
  char32_t to_utf32_char<4>(char const * const src_char, uint8_t src_char_count);
}