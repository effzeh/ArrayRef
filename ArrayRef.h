// Distributed under the MIT license. See the end of the file for details.

#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace cxx {

template <typename From, typename To>
using is_array_convertible = std::is_convertible< From (*)[], To (*)[] >;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

template <typename PointerT>
class array_iterator
{
    static_assert(std::is_pointer<PointerT>::value, "invalid template argument");

    template <typename> friend class array_iterator;

public:
    using iterator_category = std::random_access_iterator_tag;
    using element_type      = std::remove_pointer_t<PointerT>;
    using value_type        = std::remove_cv_t<element_type>;
    using reference         = std::add_lvalue_reference_t<element_type>;
    using pointer           = std::add_pointer_t<element_type>;
    using difference_type   = std::ptrdiff_t;

private:
    pointer ptr_ = nullptr;
    difference_type pos_ = 0;
    difference_type size_ = 0;

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
        , size_(it.size_)
    {
    }

    constexpr array_iterator(pointer ptr, difference_type size, difference_type pos = 0) noexcept
        : ptr_(ptr)
        , pos_(pos)
        , size_(size)
    {
        assert((ptr_ == nullptr && size_ == 0) || (ptr_ != nullptr && size_ >= 0));
        assert(pos_ >= 0);
        assert(pos_ <= size_);
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
    using iterator        = array_iterator<T*>;
    using element_type    = typename iterator::element_type;
    using value_type      = typename iterator::value_type;
    using reference       = typename iterator::reference;
    using pointer         = typename iterator::pointer;
    using difference_type = typename iterator::difference_type;

private:
    pointer data_ = nullptr;
    difference_type size_ = 0;

    static constexpr difference_type Min(difference_type x, difference_type y) {
        return y < x ? y : x;
    }

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
        : data_(first.ptr())
        , size_(last - first)
    {
        assert((data_ == nullptr && size_ == 0) || (data_ != nullptr && size_ >= 0));
    }

    template <
        typename U,
        typename = std::enable_if_t<is_array_convertible<U, element_type>::value>
    >
    constexpr array_ref(U* p, difference_type n) noexcept
        : data_(p)
        , size_(n)
    {
    }

    template <
        typename U, size_t N,
        typename = std::enable_if_t< is_array_convertible<U, element_type>::value >
    >
    constexpr array_ref(U (&arr)[N]) noexcept
        : data_(arr)
        , size_(N)
    {
    }

    template <
        typename Rhs,
        typename = std::enable_if_t<
            is_array_convertible<
                std::remove_pointer_t<decltype( std::declval<Rhs&>().data() )>, element_type
            >::value
        >
    >
    constexpr array_ref(Rhs& rhs) noexcept
        : data_(rhs.data())
        , size_(static_cast<difference_type>(rhs.size()))
    {
    }

    template <
        typename Rhs,
        typename = std::enable_if_t<
            std::is_const<element_type>::value &&
            is_array_convertible<
                std::remove_pointer_t<decltype( std::declval<Rhs const&>().data() )>, element_type
            >::value
        >
    >
    constexpr array_ref(Rhs const& rhs) noexcept
        : data_(rhs.data())
        , size_(static_cast<difference_type>(rhs.size()))
    {
    }

    constexpr pointer data() const noexcept {
        return data_;
    }

    constexpr reference operator[](difference_type index) const noexcept {
        assert(index >= 0);
        assert(index < size_);
        return data_[index];
    }

    constexpr difference_type size() const noexcept {
        return size_;
    }

    constexpr difference_type size_in_bytes() const noexcept {
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
    constexpr array_ref take_front(difference_type n = 1) const noexcept {
        n = Min(n, size());
        return { begin(), begin() + n };
    }

    // Returns [end() - n, end())
    constexpr array_ref take_back(difference_type n = 1) const noexcept {
        n = Min(n, size());
        return { end() - n, end() };
    }

    // Returns [begin() + n, end())
    constexpr array_ref drop_front(difference_type n = 1) const noexcept {
        n = Min(n, size());
        return { begin() + n, end() };
    }

    // Returns [begin(), end() - n)
    constexpr array_ref drop_back(difference_type n = 1) const noexcept {
        n = Min(n, size());
        return { begin(), end() - n };
    }

    // Returns [first, last)
    constexpr array_ref subarray(difference_type first, difference_type last) const noexcept {
        return take_front(last).drop_front(first);
    }

    // Returns [first, end())
    constexpr array_ref subarray(difference_type first) const noexcept {
        return drop_front(first);
    }

    // Returns [first, first + n)
    constexpr array_ref slice(difference_type first, difference_type n) const noexcept {
        return drop_front(first).take_front(n);
    }

    // Returns [first, end())
    constexpr array_ref slice(difference_type first) const noexcept {
        return drop_front(first);
    }

#if 1
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

#if __cpp_deduction_guides >= 201606

template <typename T>
array_ref(T*, T*)
    -> array_ref< typename array_iterator<T*>::element_type >;

template <typename P>
array_ref(array_iterator<P>, array_iterator<P>)
    -> array_ref< typename array_iterator<P>::element_type >;

template <typename T>
array_ref(T*, typename array_iterator<T*>::difference_type)
    -> array_ref< typename array_iterator<T*>::element_type >;

template <typename T, size_t N>
array_ref(T (&)[N])
    -> array_ref< typename array_iterator<T*>::element_type >;

template <
    typename ContainerT,
    typename DataT = decltype(std::declval<ContainerT&>().data()),
    typename = std::enable_if_t< std::is_pointer<DataT>::value >
>
array_ref(ContainerT&)
    -> array_ref< typename array_iterator<DataT>::element_type >;

#endif // __cpp_deduction_guides >= 201606

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
