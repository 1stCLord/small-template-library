#pragma once
#include <string>
#include <type_traits>
#include <algorithm>

namespace small_tl
{
  void to_utf8_char(char32_t src_char, std::string &dst);

  //helper to count the bytes in a sequence
  uint8_t bytes_in_utf8_sequence(char character);

  //converts a utf8 string stored in std::string to a utf32 string stored in a std::u32string
  std::u32string to_utf32(const std::string &src);

  //helper to deduce if a type is an integral container
  template<class T, class enable = void>
  class is_integral_container : public std::false_type{};

  template<class T>
  class is_integral_container<T, typename std::enable_if
    <
    std::is_member_function_pointer<decltype(&T::cbegin)>::value &&
    std::is_member_function_pointer<decltype(&T::cend)>::value &&
    std::is_integral<typename T::value_type>::value
    >::type> : public std::true_type{};

  //prevents calling to_utf8 with unsuitable types
  template<class src_t, typename std::enable_if<!is_integral_container<src_t>(), src_t>::type * = nullptr>
  std::string to_utf8(const src_t &src)
  {
    static_assert(false, "src_t must be an integral container");
    return std::string();
  }

  //converts a utf32 string stored in an iterable container to a utf8 string stored in a std::string
  template<class src_t, typename std::enable_if<is_integral_container<src_t>() && sizeof(typename src_t::value_type) != 1, src_t>::type * = nullptr>
  std::string to_utf8(const src_t &src)
  {
    std::string dst;
    dst.reserve(src.size()*3);
    for (typename src_t::const_iterator src_char = src.cbegin(); src_char != src.cend(); ++src_char)
    {
      to_utf8_char(*src_char, dst);
    }

    return dst;
  }

  //raw multibyte utf32 string specialisation
  //!!it is not a good idea to store non utf8 mutlibyte strings in a std::string as it introduces artificial null terminators and breaks c interoperability!!
  template<class src_t, typename std::enable_if<is_integral_container<src_t>() && sizeof(src_t::value_type) == 1, src_t>::type * = nullptr>
  std::string to_utf8(const src_t &src)
  {
    std::string dst;
    dst.reserve(src.size());
    //a multibyte string with too few bytes is malformed
    assert(src.size() % 4 == 0);
    for (typename src_t::const_iterator src_chars = src.cbegin(); src_chars != src.cend();)
    {
      char32_t big_char;
      //todo big_char
      to_utf8_char(big_char, dst);
    }
    return dst;
  }
}