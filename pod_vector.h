#pragma once
#include <type_traits>
#include <limits>
#include <memory>
#include <cassert>

namespace small_tl
{
  template<class pod_t>
  class pod_vector
  {
    static_assert(std::is_trivially_copyable<pod_t>::value, "pod_vector requires a trivial type");
  public:

    typedef pod_t* iterator;
    typedef const pod_t* const_iterator;

    pod_vector() noexcept : m_size(0), m_ptr(nullptr, &std::free)
    {
      resize(32);
    }

    pod_t& at(size_t pos) noexcept { return m_ptr.get()[pos]; }
    const pod_t& at(size_t pos) const noexcept { return m_ptr.get()[pos]; }

    pod_t& operator [](size_t pos) noexcept { return at(pos); }
    const pod_t& operator [](size_t pos) const noexcept { return at(pos); }

    pod_t& front() noexcept { return at(0); }
    const pod_t& front() const noexcept { return at(0); }

    pod_t& back() noexcept { return at(m_size-1); }
    const pod_t& back() const noexcept { return at(m_size-1); }

    iterator begin() noexcept { return m_ptr.get(); }
    iterator end() noexcept { return m_ptr.get() + m_size; }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    bool empty() const noexcept { return m_size == 0; }

    size_t size() const noexcept { return m_size; }

    size_t max_size() const noexcept { return std::numeric_limits<size_t>::max(); }

    void reserve(size_t new_cap) noexcept
    {
      if (new_cap < m_capacity)return;
      resize(new_cap + (32 - (new_cap % 32)));
    }

    size_t capacity() const noexcept { return m_capacity; }

    void shrink_to_fit() noexcept { resize(m_size); }

    void clear() noexcept { m_size = 0; }

    iterator insert(iterator pos, const pod_t& value) noexcept { return insert(pos, 1, value); }
    const_iterator insert(const_iterator pos, const pod_t& value) noexcept { return insert(pos, 1, value); }
    iterator insert(iterator pos, size_t count, const pod_t& value) noexcept
    {
      if (count + m_size > m_capacity)
        pos = resize_with_gap(count + m_size, pos-m_ptr.get(), count);
      else
        memmove(pos + count, pos, end() - pos);

      memcpy(pos, &value, sizeof(pod_t));
      iterator dst = pos + 1;
      for (uint16_t i = 0; i < std::floor(sqrt(count)); ++i)
      {
        memcpy(pos, dst, (1 << i) * sizeof(pod_t));
        dst += (1 << i);
      }
      m_size += count;
      return pos;
    }
    iterator insert(iterator pos, const_iterator first, const_iterator last) noexcept
    {
      size_t count = (last - first) / sizeof(pod_t);
      if (count + m_size > m_capacity)
        pos = resize_with_gap(count + m_size, pos - m_ptr.get(), count);
      else
        memmove(pos + count, pos, end() - pos);

      memcpy(pos, first, last - first);
      m_size += count;
      return pos;
    }

    iterator erase(iterator pos) noexcept { return erase(pos, pos); }
    iterator erase(iterator first, iterator last) noexcept
    {
      memmove(first, last, end() - last);
      m_size -= (last - first)/sizeof(pod_t);
      return first;
    }

    void push_back(const pod_t& value) noexcept
    {
      if (m_capacity < m_size)
        resize(m_capacity * 2);
      memcpy(end(), &value, sizeof(pod_t));
      ++m_size;
    }

    void pop_back() noexcept
    {
      --m_size;
    }

    void resize(size_t new_cap) noexcept
    {
      m_size = std::min(new_cap, m_size);
      m_capacity = new_cap;
      pod_t *new_ptr = (pod_t *)std::malloc(m_capacity * sizeof(pod_t));
      memcpy(new_ptr, m_ptr.get(), m_size * sizeof(pod_t));
      m_ptr.reset(new_ptr);
    }

    void resize(size_t count, const pod_t &value) noexcept
    {
      if (count > m_capacity)
        insert(end(), count - m_capacity)
      else
        resize(count);
    }

    void swap(pod_vector<pod_t> other) noexcept
    {
      m_ptr.swap(other.m_ptr);
      std::swap(m_capacity, other.m_capacity);
      std::swap(m_size, other.size);
    }

  private:
    iterator resize_with_gap(size_t new_cap, size_t gap_offset, size_t gap_size) noexcept
    {
      m_size = std::min(new_cap, m_size-gap_size);
      assert(gap_offset < m_size);
      m_capacity = new_cap;
      pod_t *new_ptr = (pod_t *)std::malloc(m_capacity * sizeof(pod_t));
      memcpy(new_ptr, m_ptr.get(), gap_offset*sizeof(pod_t));
      memcpy(new_ptr+((gap_offset+gap_size)*sizeof(pod_t)), m_ptr.get() +(gap_offset * sizeof(pod_t)), gap_size * sizeof(pod_t));
      return new_ptr + (gap_offset * sizeof(pod_t));
    }

    size_t m_capacity;
    size_t m_size;
    std::unique_ptr<pod_t, decltype(&std::free)> m_ptr;
  };
}