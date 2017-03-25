// Distributed under the MIT license. See the end of the file for details.

#pragma once

#include <cassert>
#include <iterator>    // iterator_traits
#include <type_traits> // enable_if, is_const, is_convertible, remove_pointer

namespace cxx {

template <typename From, typename To>
using is_array_convertible = std::is_convertible< From (*)[], To (*)[] >;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

template <typename PointerT>
class array_iterator
{
    template <typename> friend class array_iterator;

public:
    using element_type      = std::remove_pointer_t<PointerT>;
    using iterator_category = typename std::iterator_traits<PointerT>::iterator_category;
    using value_type        = typename std::iterator_traits<PointerT>::value_type;
    using reference         = typename std::iterator_traits<PointerT>::reference;
    using pointer           = typename std::iterator_traits<PointerT>::pointer;
    using difference_type   = typename std::iterator_traits<PointerT>::difference_type;

private:
    pointer ptr_ = nullptr;
    difference_type pos_ = 0;
#ifndef NDEBUG
    difference_type size_ = 0;
#endif

public:
    constexpr array_iterator() noexcept = default;
    constexpr array_iterator(array_iterator const&) noexcept = default;
    constexpr array_iterator& operator=(array_iterator const&) noexcept = default;

    template <
        typename OtherPointerT,
        typename = std::enable_if_t<
            is_array_convertible< typename array_iterator<OtherPointerT>::element_type, element_type >::value
        >
    >
    constexpr array_iterator(array_iterator<OtherPointerT> const& it) noexcept
        : ptr_(it.ptr_)
        , pos_(it.pos_)
#ifndef NDEBUG
        , size_(it.size_)
#endif
    {
    }

    constexpr array_iterator(pointer ptr, difference_type size, difference_type pos = 0) noexcept
        : ptr_(ptr)
        , pos_(pos)
#ifndef NDEBUG
        , size_(size)
#endif
    {
        assert((ptr_ == nullptr && size_ == 0) || (ptr_ != nullptr && size_ >= 0));
        assert(pos_ >= 0);
        assert(pos_ <= size_);

        static_cast<void>(size); // maybe unused
    }

    constexpr pointer ptr() const noexcept {
        return ptr_ + pos_;
    }

    constexpr reference operator*() const noexcept {
        assert(ptr_ != nullptr);
        assert(pos_ < size_);
        return ptr_[pos_];
    }

    constexpr pointer operator->() const noexcept {
        assert(ptr_ != nullptr);
        assert(pos_ < size_);
        return ptr_ + pos_;
    }

    constexpr array_iterator& operator++() noexcept {
        assert(pos_ < size_);
        ++pos_;
        return *this;
    }

    constexpr array_iterator operator++(int) noexcept {
        auto t = *this;
        ++(*this);
        return t;
    }

    constexpr array_iterator& operator--() noexcept {
        assert(pos_ > 0);
        --pos_;
        return *this;
    }

    constexpr array_iterator operator--(int) noexcept {
        auto t = *this;
        --(*this);
        return t;
    }

    constexpr array_iterator& operator+=(difference_type n) noexcept {
        assert(pos_ + n >= 0);
        assert(pos_ + n <= size_);
        pos_ += n;
        return *this;
    }

    constexpr array_iterator operator+(difference_type n) const noexcept {
        auto t = *this;
        t += n;
        return t;
    }

    constexpr friend array_iterator operator+(difference_type n, array_iterator it) noexcept {
        return it + n;
    }

    constexpr array_iterator& operator-=(difference_type n) noexcept {
        assert(pos_ - n >= 0);
        assert(pos_ - n <= size_);
        pos_ -= n;
        return *this;
    }

    constexpr array_iterator operator-(difference_type n) const noexcept {
        auto t = *this;
        t -= n;
        return t;
    }

    constexpr difference_type operator-(array_iterator rhs) const noexcept {
        assert(ptr_ == rhs.ptr_);
        return pos_ - rhs.pos_;
    }

    constexpr reference operator[](difference_type index) const noexcept {
        assert(ptr_ != nullptr);
        assert(pos_ + index >= 0);
        assert(pos_ + index < size_);
        return ptr_[pos_ + index];
    }

    constexpr friend bool operator==(array_iterator lhs, array_iterator rhs) noexcept {
        assert(lhs.ptr_ == rhs.ptr_);
        return lhs.pos_ == rhs.pos_;
    }

    constexpr friend bool operator!=(array_iterator lhs, array_iterator rhs) noexcept {
        return !(lhs == rhs);
    }

    constexpr friend bool operator<(array_iterator lhs, array_iterator rhs) noexcept {
        assert(lhs.ptr_ == rhs.ptr_);
        return lhs.pos_ < rhs.pos_;
    }

    constexpr friend bool operator>(array_iterator lhs, array_iterator rhs) noexcept {
        return rhs < lhs;
    }

    constexpr friend bool operator<=(array_iterator lhs, array_iterator rhs) noexcept {
        return !(rhs < lhs);
    }

    constexpr friend bool operator>=(array_iterator lhs, array_iterator rhs) noexcept {
        return !(lhs < rhs);
    }

#ifdef _MSC_VER
    using _Checked_type = array_iterator;
    using _Unchecked_type = pointer;

    constexpr friend _Unchecked_type _Unchecked(_Checked_type it) noexcept {
        return it.ptr_ + it.pos_;
    }

    constexpr friend _Checked_type& _Rechecked(_Checked_type& it, _Unchecked_type p) noexcept {
        it.pos_ = p - it.ptr_;
        return it;
    }
#endif
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

template <typename T>
class array_ref
{
public:
    using iterator     = array_iterator<T*>;
    using element_type = typename iterator::element_type;
    using value_type   = typename iterator::value_type;
    using reference    = typename iterator::reference;
    using pointer      = typename iterator::pointer;
    using size_type    = typename iterator::difference_type;
    using index_type   = typename iterator::difference_type;

private:
    pointer data_ = nullptr;
    size_type size_ = 0;

public:
    constexpr array_ref() noexcept = default;
    constexpr array_ref(array_ref const&) noexcept = default;
    constexpr array_ref& operator=(array_ref const&) noexcept = default;

    constexpr array_ref(pointer first, pointer last) noexcept
        : data_(first)
        , size_(last - first)
    {
        assert((data_ == nullptr && size_ == 0) || (data_ != nullptr && size_ >= 0));
    }

    constexpr array_ref(iterator first, iterator last) noexcept
        : data_(first.ptr()) // data_(first == last ? pointer{} : &*first)
        , size_(last - first)
    {
        assert((data_ == nullptr && size_ == 0) || (data_ != nullptr && size_ >= 0));
    }

    template <
        typename U,
        typename = std::enable_if_t< is_array_convertible<U, element_type>::value >
    >
    constexpr array_ref(U* p, size_type n) noexcept
        : data_(p)
        , size_(n)
    {
        assert((data_ == nullptr && size_ == 0) || (data_ != nullptr && size_ >= 0));
    }

    template <
        typename U, size_t N,
        typename = std::enable_if_t< is_array_convertible<U, element_type>::value >
    >
    constexpr array_ref(U (&arr)[N], size_type n = N) noexcept
        : data_(arr)
        , size_(n)
    {
        assert(n <= N);
    }

    template <
        typename Rhs,
        typename = std::enable_if_t<
            /*!std::is_const<element_type>::value &&*/
            is_array_convertible<
                std::remove_pointer_t<decltype( std::declval<Rhs&>().data() )>,
                element_type
            >::value
        >
    >
    constexpr array_ref(Rhs& rhs) noexcept
        : data_(rhs.data())
        , size_(static_cast<size_type>(rhs.size()))
    {
    }

    template <
        typename Rhs,
        typename = std::enable_if_t<
            std::is_const<element_type>::value &&
            is_array_convertible<
                std::remove_pointer_t<decltype( std::declval<Rhs const&>().data() )>,
                element_type
            >::value
        >
    >
    constexpr array_ref(Rhs const& rhs) noexcept
        : data_(rhs.data())
        , size_(static_cast<size_type>(rhs.size()))
    {
    }

    constexpr pointer data() const noexcept {
        return data_;
    }

    constexpr reference operator[](index_type index) const noexcept {
        assert(index >= 0);
        assert(index < size());
        return data_[index];
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

    constexpr size_type size_in_bytes() const noexcept {
        return size_ * sizeof(element_type);
    }

    constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    constexpr iterator begin() const noexcept {
        return iterator { data_, size_, 0 };
    }

    constexpr iterator end() const noexcept {
        return iterator { data_, size_, size_ };
    }

    // Returns [begin(), begin() + n)
    constexpr array_ref front(size_type n = 1) const noexcept {
        return { begin(), begin() + n };
    }

    // Returns [end() - n, end())
    constexpr array_ref back(size_type n = 1) const noexcept {
        return { end() - n, end() };
    }

    // Returns [begin() + n, end())
    constexpr array_ref drop_front(size_type n = 1) const noexcept {
        return { begin() + n, end() };
    }

    // Returns [begin(), end() - n)
    constexpr array_ref drop_back(size_type n = 1) const noexcept {
        return { begin(), end() - n };
    }

    // Returns [first, last)
    constexpr array_ref subarray(index_type first, index_type last) const noexcept {
        return { begin() + first, begin() + last };
    }

    // Returns [first, end())
    constexpr array_ref subarray(index_type first) const noexcept {
        return { begin() + first, end() };
    }

    // Returns [first, first + n)
    constexpr array_ref slice(index_type first, size_type n) const noexcept {
        return { begin() + first, begin() + (first + n) };
    }

    // Returns [first, end())
    constexpr array_ref slice(index_type first) const noexcept {
        return { begin() + first, end() };
    }

#if 0
    constexpr friend bool operator==(array_ref lhs, array_ref rhs) noexcept {
        return lhs.data() == rhs.data() && lhs.size() == rhs.size();
    }

    constexpr friend bool operator!=(array_ref lhs, array_ref rhs) noexcept {
        return !(lhs == rhs);
    }

#if 0
    constexpr friend bool operator<(array_ref lhs, array_ref rhs) noexcept {
        return lhs.data() < rhs.data() || (lhs.data() == rhs.data() && lhs.size() < rhs.size());
    }

    constexpr friend bool operator>(array_ref lhs, array_ref rhs) noexcept {
        return rhs < lhs;
    }

    constexpr friend bool operator<=(array_ref lhs, array_ref rhs) noexcept {
        return !(rhs < lhs);
    }

    constexpr friend bool operator>=(array_ref lhs, array_ref rhs) noexcept {
        return !(lhs < rhs);
    }
#endif
#endif
};

#if 0
template <
    typename T,
    typename U,
    typename = std::enable_if_t<
        std::is_convertible<decltype(std::declval<T>() == std::declval<U>()), bool>::value
    >
>
constexpr bool operator==(array_ref<T> lhs, array_ref<U> rhs) noexcept
{
    return lhs.size() == rhs.size() &&
        (lhs.data() == rhs.data() || std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
}

template <
    typename T,
    typename U,
    typename = std::enable_if_t<
        std::is_convertible<decltype(std::declval<T>() == std::declval<U>()), bool>::value
    >
>
constexpr bool operator!=(array_ref<T> lhs, array_ref<U> rhs) noexcept
{
    return !(lhs == rhs);
}

template <
    typename T,
    typename U,
    typename = std::enable_if_t<
        std::is_convertible<decltype(std::declval<T>() < std::declval<U>()), bool>::value
    >
>
constexpr bool operator<(array_ref<T> lhs, array_ref<U> rhs) noexcept
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <
    typename T,
    typename U,
    typename = std::enable_if_t<
        std::is_convertible<decltype(std::declval<T>() < std::declval<U>()), bool>::value
    >
>
constexpr bool operator>(array_ref<T> lhs, array_ref<U> rhs) noexcept
{
    return rhs < lhs;
}

template <
    typename T,
    typename U,
    typename = std::enable_if_t<
        std::is_convertible<decltype(std::declval<T>() < std::declval<U>()), bool>::value
    >
>
constexpr bool operator<=(array_ref<T> lhs, array_ref<U> rhs) noexcept
{
    return !(rhs < lhs);
}

template <
    typename T,
    typename U,
    typename = std::enable_if_t<
        std::is_convertible<decltype(std::declval<T>() < std::declval<U>()), bool>::value
    >
>
constexpr bool operator>=(array_ref<T> lhs, array_ref<T> rhs) noexcept
{
    return !(lhs < rhs);
}
#endif

} // namespace cxx

//------------------------------------------------------------------------------
// Copyright 2017 A. Bolz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
