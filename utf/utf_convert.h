#pragma once
#include <string>
#include <type_traits>
#include <algorithm>

#include "utf_helpers.h"
#include "utf_char_convert.h"

namespace small_tl::utf_convert
{
  //
  //utf8
  //

  //prevents calling to_utf8 with unsuitable types
  template<class src_t, typename std::enable_if<!is_integral_container<src_t>::value, src_t>::type * = nullptr>
  std::string to_utf8(const src_t &src)
  {
    static_assert(false, "src_t must be an integral container");
    return std::string();
  }

  //converts a utf16 string stored in an iterable container to a utf8 string stored in a std::string
  template<class src_t, typename std::enable_if<is_integral_container<src_t>::value && sizeof(typename src_t::value_type) == 2, src_t>::type * = nullptr>
  std::string to_utf8(const src_t &src)
  {
    return to_utf8(to_utf32(src));
  }

  //converts a utf32 string stored in an iterable container to a utf8 string stored in a std::string
  template<class src_t, typename std::enable_if<is_integral_container<src_t>::value && sizeof(typename src_t::value_type) == 4, src_t>::type * = nullptr>
  std::string to_utf8(const src_t &src)
  {
    std::string dst;
    dst.reserve(src.size() * 3);
    for (typename src_t::const_iterator src_char = src.cbegin(); src_char != src.cend(); ++src_char)
    {
      to_utf8_char(*src_char, dst);
    }

    return dst;
  }

  //converts a utf16 or utf32 string stored in an iterable container to a utf8 string stored in a std::string, defaults to utf32
  //!!it is not a good idea to store non utf8 mutlibyte strings in a std::string as it introduces artificial null terminators and breaks c interoperability!!
  template<class src_t, typename std::enable_if<is_integral_container<src_t>::value && sizeof(src_t::value_type) == 1, uint8_t>::type bytes_in_code_point = 4>
  std::string to_utf8(const src_t &src)
  {
    std::string dst;
    dst.reserve(src.size());
    //a multibyte string with too few bytes is malformed
    assert(src.size() % bytes_in_code_point == 0);
    for (typename src_t::const_iterator src_chars = src.cbegin(); src_chars != src.cend();)
    {
      uint8_t remaining = std::min<uint8_t>(bytes_in_code_point, src.cend() - src_chars);
      to_utf8_char(to_utf32_char<bytes_in_code_point>(&*src_chars, remaining), dst);

      src_chars += remaining;
    }
    return dst;
  }


  //
  //utf16
  //

  //prevents calling to_utf16 with unsuitable types
  template<class src_t, typename std::enable_if<!is_integral_container<src_t>::value, src_t>::type * = nullptr>
  std::u16string to_utf16(const src_t &src)
  {
    static_assert(false, "src_t must be an integral container");
    return std::u16string();
  }

  //converts a utf8 string stored in an iterable container to a utf16 string stored in a std::u16string
  template<class src_t, typename std::enable_if<is_integral_container<src_t>::value && sizeof(typename src_t::value_type) == 1, src_t>::type * = nullptr>
  std::u16string to_utf16(const src_t &src)
  {
    return to_utf16(to_utf32(src));
  }

  //converts a utf32 string stored in an iterable container to a utf16 string stored in a std::u16string
  template<class src_t, typename std::enable_if<is_integral_container<src_t>::value && sizeof(typename src_t::value_type) == 4, src_t>::type * = nullptr>
  std::u16string to_utf16(const src_t &src)
  {
    std::u16string dst;
    dst.reserve(src.size());
    for (typename src_t::const_iterator src_char = src.cbegin(); src_char != src.cend(); ++src_char)
    {
      to_utf16_char(*src_char, dst);
    }

    return dst;
  }

  //
  //utf32
  //

  //prevents calling to_utf32 with unsuitable types
  template<class src_t, typename std::enable_if<!is_integral_container<src_t>::value, src_t>::type * = nullptr>
  std::u32string to_utf32(const src_t &src)
  {
    static_assert(false, "src_t must be an integral container");
    return std::u32string();
  }

  //converts a utf8 or utf16 string stored in integral container to a utf32 string stored in a std::u32string
  template<class src_t, typename std::enable_if<is_integral_container<src_t>::value, src_t>::type * = nullptr>
  std::u32string to_utf32(const src_t &src)
  {
    std::u32string dst;
    dst.reserve(src.size());
    char32_t dst_char;
    for (src_t::const_iterator src_char = src.cbegin(); src_char != src.cend();)
    {
      int8_t chars_read = to_utf32_char(&*src_char, src.cend() - src_char, dst_char);
      if (chars_read >= 1)
      {
        src_char += chars_read;
      }
      else ++src_char;
      dst.push_back(dst_char);
    }
    return dst;
  }
}