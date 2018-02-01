#pragma once

namespace small_tl::utf_convert
{
  //
  //helpers
  //

  enum utf16_order { SINGLE = -1, HIGH_SURROGATE = 0, LOW_SURROGATE = 1 };

  utf16_order utf16_order_of_char(char16_t character);

  //helper to count the bytes in a utf8 sequence
  uint8_t bytes_in_utf8_sequence(char character);

  //helper to deduce if a type is an integral container
  template<class T, class enable = void>
  class is_integral_container : public std::false_type {};

  template<class T>
  class is_integral_container<T, typename std::enable_if
    <
    std::is_member_function_pointer<decltype(&T::cbegin)>::value &&
    std::is_member_function_pointer<decltype(&T::cend)>::value &&
    std::is_integral<typename T::value_type>::value
    >::type> : public std::true_type
  {};
}