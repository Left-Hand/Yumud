/*
Copyright © 2015 Kean Mariotti <kean.mariotti@gmail.com>
This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
*/

#ifndef _SSTL_DEQUENG__
#define _SSTL_DEQUENG__

#include <utility>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <initializer_list>
#include <array>

#include <sstl_assert.h>

#include "__internal/_aligned_storage.h"
#include "__internal/_iterator.h"
#include "__internal/_deque_iterator.h"
#include "__internal/_hacky_derived_class_access.h"
#include "__internal/_debug.h"

namespace sstl
{

template<class, size_t=static_cast<size_t>(-1)>
class deque;

template<class T>
class deque<T>
{
template<class U, size_t S>
friend class deque; //friend declaration required for derived class' noexcept expressions

friend class _dequeng_iterator<deque>;
friend class _dequeng_iterator<const deque>;

protected:
   using _type_for_hacky_derived_class_access = deque < T, 11 > ;

public:
   using value_type = T;
   using size_type = size_t;
   using reference = value_type&;
   using const_reference = const value_type&;
   using pointer = value_type*;
   using const_pointer = const value_type*;
   using iterator = _dequeng_iterator<deque>;
   using const_iterator = _dequeng_iterator<const deque>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;
   using difference_type = typename iterator::difference_type;

public:
   deque& operator=(const deque& rhs)
      _sstl_noexcept(std::is_nothrow_copy_assignable<value_type>::value
                     && std::is_nothrow_copy_constructible<value_type>::value)
   {
      if(this == &rhs)
         return *this;

      sstl_assert(rhs.size() <= capacity());
      _range_assignment(rhs.cbegin(), rhs.cend());

      return *this;
   }

   deque& operator=(deque&& rhs)
      _sstl_noexcept(std::is_nothrow_move_assignable<value_type>::value
                     && std::is_nothrow_move_constructible<value_type>::value)
   {
      sstl_assert(rhs.size() <= capacity());
      auto move_assignments = std::min(size(), rhs.size());
      auto move_constructions = rhs.size() - move_assignments;
      auto destructions =  move_assignments + move_constructions < size()
                           ? size() - move_assignments - move_constructions
                           : 0;
      auto src = _sstl_member_of_derived_class(&rhs, _first_pointer);
      auto dst = _sstl_member_of_derived_class(this, _first_pointer);

      size_type i;
      #if _sstl_has_exceptions()
      try
      {
      #endif
         for(i=0; i!=move_assignments; ++i)
         {
            *dst = std::move(*src);
            src->~value_type();
            src = rhs._inc_pointer(src);
            dst = _inc_pointer(dst);
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         _sstl_member_of_derived_class(&rhs, _first_pointer) = src;
         _sstl_member_of_derived_class(&rhs, _size) -= i;
         throw;
      }
      #endif

      #if _sstl_has_exceptions()
      try
      {
      #endif
         for(i=0; i!=move_constructions; ++i)
         {
            new(dst) value_type(std::move(*src));
            src->~value_type();
            src = rhs._inc_pointer(src);
            dst = _inc_pointer(dst);
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         auto new_lhs_last = dst;
         new_lhs_last = _dec_pointer(new_lhs_last);
         _sstl_member_of_derived_class(this, _last_pointer) = new_lhs_last;
         _sstl_member_of_derived_class(this, _size) += i;

         _sstl_member_of_derived_class(&rhs, _first_pointer) = src;
         _sstl_member_of_derived_class(&rhs, _size) -= (move_assignments + i);

         throw;
      }
      #endif

      auto new_lhs_last = dst;
      new_lhs_last = _dec_pointer(new_lhs_last);
      _sstl_member_of_derived_class(this, _last_pointer) = new_lhs_last;
      for(auto i=destructions; i!=0; --i)
      {
         dst->~value_type();
         dst = _inc_pointer(dst);
      }
      _sstl_member_of_derived_class(this, _size) = rhs.size();

      auto new_rhs_last = _sstl_member_of_derived_class(&rhs, _first_pointer);
      new_rhs_last = rhs._dec_pointer(new_rhs_last);
      _sstl_member_of_derived_class(&rhs, _last_pointer) = new_rhs_last;
      _sstl_member_of_derived_class(&rhs, _size) = 0;

      return *this;
   }

   deque& operator=(std::initializer_list<value_type> ilist)
      _sstl_noexcept(noexcept(std::declval<deque>()._range_assignment(
         std::declval<std::initializer_list<value_type>>().begin(),
         std::declval<std::initializer_list<value_type>>().end())))
   {
      _range_assignment(ilist.begin(), ilist.end());
      return *this;
   }

   void assign(size_type count, const_reference value)
      _sstl_noexcept(std::is_nothrow_copy_assignable<value_type>::value
                     && std::is_nothrow_copy_constructible<value_type>())
   {
      sstl_assert(count <= capacity());
      auto dst = _sstl_member_of_derived_class(this, _first_pointer);

      auto assignments = std::min(size(), count);
      for(size_type i=0; i<assignments; ++i)
      {
         *dst = value;
         dst = _inc_pointer(dst);
      }

      auto constructions = count - assignments;
      size_type constructions_done;
      #if _sstl_has_exceptions()
      try
      {
      #endif
         for(constructions_done=0; constructions_done<constructions; ++constructions_done)
         {
            new(dst) value_type(value);
            dst = _inc_pointer(dst);
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         dst = _dec_pointer(dst);
         _sstl_member_of_derived_class(this, _last_pointer) = dst;
         _sstl_member_of_derived_class(this, _size) += constructions_done;
         throw;
      }
      #endif

      auto new_last = dst;
      new_last = _dec_pointer(new_last);

      auto destructions = size() < count ? 0 : size() - count;
      for(size_type i=0; i<destructions; ++i)
      {
         dst->~value_type();
         dst = _inc_pointer(dst);
      }

      _sstl_member_of_derived_class(this, _last_pointer) = new_last;
      _sstl_member_of_derived_class(this, _size) = count;
   }

   reference at(size_type idx) _sstl_noexcept(!_sstl_has_exceptions())
   {
      #if _sstl_has_exceptions()
      if(idx >= size())
      {
         throw std::out_of_range(_sstl_debug_message("access out of range"));
      }
      #endif
      sstl_assert(idx < size());
      return *_add_offset_to_pointer(_sstl_member_of_derived_class(this, _first_pointer), idx);
   }

   const_reference at(size_type idx) const
      _sstl_noexcept(noexcept(std::declval<deque>().at(size_type{})))
   {
      return const_cast<deque&>(*this).at(idx);
   }

   reference operator[](size_type idx) _sstl_noexcept_
   {
      return *_add_offset_to_pointer(_sstl_member_of_derived_class(this, _first_pointer), idx);
   }

   const_reference operator[](size_type idx) const _sstl_noexcept_
   {
      return const_cast<deque&>(*this)[idx];
   }

   reference front() _sstl_noexcept_
   {
      sstl_assert(!empty());
      return *_sstl_member_of_derived_class(this, _first_pointer);
   }

   const_reference front() const _sstl_noexcept_
   {
      sstl_assert(!empty());
      return const_cast<deque&>(*this).front();
   }

   reference back() _sstl_noexcept_
   {
      sstl_assert(!empty());
      return *(_sstl_member_of_derived_class(this, _last_pointer));
   }

   const_reference back() const _sstl_noexcept_
   {
      return const_cast<deque&>(*this).back();
   }

   iterator begin() _sstl_noexcept_
   {
      return iterator{ this, empty() ? nullptr : _sstl_member_of_derived_class(this, _first_pointer) };
   }

   const_iterator begin() const _sstl_noexcept_
   {
      return const_iterator{ this, empty() ? nullptr : _sstl_member_of_derived_class(this, _first_pointer) };
   }

   const_iterator cbegin() const _sstl_noexcept_
   {
      return const_iterator{ this, empty() ? nullptr : _sstl_member_of_derived_class(this, _first_pointer) };
   }

   iterator end() _sstl_noexcept_
   {
      return iterator{ this, nullptr };
   }

   const_iterator end() const _sstl_noexcept_
   {
      return const_iterator{ this, nullptr };
   }

   const_iterator cend() const _sstl_noexcept_
   {
      return const_iterator{ this, nullptr };
   }

   reverse_iterator rbegin() _sstl_noexcept_
   {
      return reverse_iterator{ end() };
   }

   const_reverse_iterator rbegin() const _sstl_noexcept_
   {
      return crbegin();
   }

   const_reverse_iterator crbegin() const _sstl_noexcept_
   {
      return const_reverse_iterator{ cend() };
   }

   reverse_iterator rend() _sstl_noexcept_
   {
      return reverse_iterator{ begin() };
   }

   const_reverse_iterator rend() const _sstl_noexcept_
   {
      return crend();
   }

   const_reverse_iterator crend() const _sstl_noexcept_
   {
      return const_reverse_iterator{ cbegin() };
   }

   bool empty() const _sstl_noexcept_
   {
      return size() == 0;
   }

   bool full() const _sstl_noexcept_
   {
      return size() == capacity();
   }

   size_type size() const _sstl_noexcept_
   {
      return _sstl_member_of_derived_class(this, _size);
   }

   size_type max_size() const _sstl_noexcept_
   {
      return capacity();
   }

   size_type capacity() const _sstl_noexcept_
   {
      return _sstl_member_of_derived_class(this, _end_storage) - _begin_storage();
   }

   void clear() _sstl_noexcept(std::is_nothrow_destructible<value_type>::value)
   {
      while (_sstl_member_of_derived_class(this, _size) > 0)
      {
         _sstl_member_of_derived_class(this, _last_pointer)->~value_type();
         _sstl_member_of_derived_class(this, _last_pointer) = _dec_pointer(_sstl_member_of_derived_class(this, _last_pointer));
         --_sstl_member_of_derived_class(this, _size);
      }
   }

   iterator insert(const_iterator pos, const value_type& value)
      _sstl_noexcept(noexcept(std::declval<deque>()._emplace_value(std::declval<const_iterator>(),
                                                                     std::declval<const value_type&>())))
   {
      return _emplace_value(pos, value);
   }

   iterator insert(const_iterator pos, value_type&& value)
      _sstl_noexcept(noexcept(std::declval<deque>()._emplace_value(std::declval<const_iterator>(),
                                                                     std::declval<value_type&&>())))
   {
      return _emplace_value(pos, std::move(value));
   }

   iterator insert(const_iterator pos, size_type count, const_reference value)
      _sstl_noexcept(
            noexcept(std::declval<deque>()._shift_from_begin_to_pos_by_n_positions(  std::declval<size_type>(),
                                                                                       std::declval<difference_type>()))
         && noexcept(std::declval<deque>()._shift_from_pos_to_end_by_n_positions( std::declval<size_type>(),
                                                                                    std::declval<difference_type>()))
         && std::is_nothrow_copy_constructible<value_type>::value
         && std::is_nothrow_copy_assignable<value_type>::value)
   {
      if(count==0)
         return iterator{ this, const_cast<pointer>(pos._pos) };

      sstl_assert(size()+count <= capacity());
      auto distance_to_begin = static_cast<size_type>(std::distance(cbegin(), pos));
      auto distance_to_end = static_cast<size_type>(std::distance(pos, cend()));

      if(distance_to_begin < distance_to_end)
      {
         auto new_region_first_pointer = _shift_from_begin_to_pos_by_n_positions(count, distance_to_begin);
         auto dst = new_region_first_pointer;
         auto number_of_constructions = count > distance_to_begin ? count-distance_to_begin : 0;
         size_type remaining_constructions;
         #if _sstl_has_exceptions()
         try
         {
         #endif
            for(remaining_constructions=number_of_constructions; remaining_constructions>0; --remaining_constructions)
            {
               new(dst) value_type(value);
               dst = _inc_pointer(dst);
            }
         #if _sstl_has_exceptions()
         }
         catch(...)
         {
            auto crt = _sstl_member_of_derived_class(this, _first_pointer);
            while(crt != dst)
            {
               crt->~value_type();
               crt = _inc_pointer(crt);
            }
            for(size_t i=0; i<remaining_constructions; ++i)
            {
               crt = _inc_pointer(crt);
            }
            _sstl_member_of_derived_class(this, _first_pointer) = crt;
            _sstl_member_of_derived_class(this, _size) -= count;
            throw;
         }
         #endif
         auto number_of_assignments = count-number_of_constructions;
         for(size_type i=number_of_assignments; i>0; --i)
         {
            *dst = value;
            dst = _inc_pointer(dst);
         }
         return iterator{ this, new_region_first_pointer };
      }
      else
      {
         auto new_region_last_pointer = _shift_from_pos_to_end_by_n_positions(count, distance_to_end);
         auto dst = new_region_last_pointer;
         auto number_of_constructions = count > distance_to_end ? count-distance_to_end : 0;
         #if _sstl_has_exceptions()
         try
         {
         #endif
            for(size_type i=number_of_constructions; i>0; --i)
            {
               new(dst) value_type(value);
               dst = _dec_pointer(dst);
            }
         #if _sstl_has_exceptions()
         }
         catch(...)
         {
            while (dst != _sstl_member_of_derived_class(this, _last_pointer))
            {
               dst = _inc_pointer(dst);
               dst->~value_type();
            }
            _sstl_member_of_derived_class(this, _last_pointer) =
               _subtract_offset_to_pointer(_sstl_member_of_derived_class(this, _last_pointer), count);
            _sstl_member_of_derived_class(this, _size) -= count;
            throw;
         }
         #endif
         auto number_of_assignments = count - number_of_constructions;
         for(size_type i=number_of_assignments; i>0; --i)
         {
            *dst = value;
            dst = _dec_pointer(dst);
         }
         return iterator{ this, _inc_pointer(dst) };
      }
   }

   template<class TIterator>
   iterator insert(  const_iterator pos,
                     TIterator range_begin,
                     TIterator range_end,
                     typename std::enable_if<_is_input_iterator<TIterator>::value
                                          && !_is_forward_iterator<TIterator>::value>::type* = nullptr)
      _sstl_noexcept(std::is_nothrow_copy_constructible<value_type>::value
                  && noexcept(std::reverse(std::declval<iterator>(), std::declval<iterator>()))
                  && noexcept(std::rotate(std::declval<iterator>(),
                                          std::declval<iterator>(),
                                          std::declval<iterator>())))
   {
      auto old_size = size();
      auto distance_to_begin = std::distance(cbegin(), pos);
      auto distance_to_end = std::distance(pos, cend());
      if(distance_to_begin < distance_to_end)
      {
         std::copy(range_begin, range_end, std::front_inserter(*this));
         auto range_size = size() - old_size;
         std::reverse(begin(), begin()+range_size);
         std::rotate(begin(), begin()+range_size, begin()+range_size+distance_to_begin);
         return begin()+distance_to_begin;
      }
      else
      {
         std::copy(range_begin, range_end, std::back_inserter(*this));
         auto range_size = size() - old_size;
         std::rotate(end()-range_size-distance_to_end, end()-range_size, end());
         return end()-range_size-distance_to_end;
      }
   }

   template<class TIterator>
   iterator insert(  const_iterator pos,
                     TIterator range_begin,
                     TIterator range_end,
                     typename std::enable_if<_is_forward_iterator<TIterator>::value>::type* = nullptr)
      _sstl_noexcept(
            noexcept(std::declval<deque>()._shift_from_begin_to_pos_by_n_positions(  std::declval<size_type>(),
                                                                                       std::declval<difference_type>()))
         && noexcept(std::declval<deque>()._shift_from_pos_to_end_by_n_positions( std::declval<size_type>(),
                                                                                    std::declval<difference_type>()))
         && std::is_nothrow_copy_constructible<value_type>::value
         && std::is_nothrow_copy_assignable<value_type>::value)
   {
      auto count = static_cast<size_type>(std::distance(range_begin, range_end));
      if(count==0)
         return iterator{ this, const_cast<pointer>(pos._pos) };

      sstl_assert(size()+count <= capacity());
      auto distance_to_begin = static_cast<size_type>(std::distance(cbegin(), pos));
      auto distance_to_end = static_cast<size_type>(std::distance(pos, cend()));
      auto range_pos = range_begin;

      if(distance_to_begin < distance_to_end)
      {
         auto new_region_first_pointer = _shift_from_begin_to_pos_by_n_positions(count, distance_to_begin);
         auto dst = new_region_first_pointer;
         auto number_of_constructions = count > distance_to_begin ? count-distance_to_begin : 0;
         size_type remaining_constructions;
         #if _sstl_has_exceptions()
         try
         {
         #endif
            for(remaining_constructions=number_of_constructions; remaining_constructions>0; --remaining_constructions)
            {
               new(dst) value_type(*range_pos);
               ++range_pos;
               dst = _inc_pointer(dst);
            }
         #if _sstl_has_exceptions()
         }
         catch(...)
         {
            auto crt = _sstl_member_of_derived_class(this, _first_pointer);
            while(crt != dst)
            {
               crt->~value_type();
               crt = _inc_pointer(crt);
            }
            for(size_t i=0; i<remaining_constructions; ++i)
            {
               crt = _inc_pointer(crt);
            }
            _sstl_member_of_derived_class(this, _first_pointer) = crt;
            _sstl_member_of_derived_class(this, _size) -= count;
            throw;
         }
         #endif
         auto number_of_assignments = count-number_of_constructions;
         for(size_type i=number_of_assignments; i>0; --i)
         {
            *dst = *range_pos;
            ++range_pos;
            dst = _inc_pointer(dst);
         }
         return iterator{ this, new_region_first_pointer };
      }
      else
      {
         auto new_region_last_pointer = _shift_from_pos_to_end_by_n_positions(count, distance_to_end);
         auto new_region_first_pointer = _subtract_offset_to_pointer(new_region_last_pointer, count > 0 ? count-1 : 0);
         auto dst = new_region_first_pointer;
         auto number_of_constructions = count > distance_to_end ? count-distance_to_end : 0;
         auto number_of_assignments = count - number_of_constructions;

         size_type remaining_assignments = number_of_assignments;
         size_type remaining_constructions = number_of_constructions;
         #if _sstl_has_exceptions()
         try
         {
         #endif
            while(remaining_assignments > 0)
            {
               *dst = *range_pos;
               ++range_pos;
               dst = _inc_pointer(dst);
               --remaining_assignments;
            }
            while(remaining_constructions > 0)
            {
               new(dst) value_type(*range_pos);
               ++range_pos;
               dst = _inc_pointer(dst);
               --remaining_constructions;
            }
         #if _sstl_has_exceptions()
         }
         catch(...)
         {
            for(size_type i=0; i<remaining_assignments; ++i)
            {
               dst = _inc_pointer(dst);
            }
            for(size_type i=0; i<remaining_constructions; ++i)
            {
               dst = _inc_pointer(dst);
            }
            for(size_type i=0; i<distance_to_end; ++i)
            {
               dst->~value_type();
               dst = _inc_pointer(dst);
            }
            auto constructions_done = number_of_constructions-remaining_constructions;
            _sstl_member_of_derived_class(this, _last_pointer) =
               _subtract_offset_to_pointer(_sstl_member_of_derived_class(this, _last_pointer), count - constructions_done);
            _sstl_member_of_derived_class(this, _size) -= (count - constructions_done);
            throw;
         }
         #endif
         return iterator{ this, new_region_first_pointer };
      }
   }

   iterator erase(const_iterator pos)
      _sstl_noexcept(std::is_nothrow_move_assignable<value_type>::value)
   {
      sstl_assert(!empty());
      sstl_assert(pos != cend());
      auto distance_to_begin = static_cast<size_type>(pos - cbegin());
      auto distance_to_end = static_cast<size_type>(cend() - pos);
      auto pos_pointer = const_cast<pointer>(pos._pos);
      if(distance_to_begin < distance_to_end)
      {
         auto src = _dec_pointer(pos_pointer);
         auto dst = pos_pointer;
         while (dst != _sstl_member_of_derived_class(this, _first_pointer))
         {
            *dst = std::move(*src);
            dst = src;
            src = _dec_pointer(src);
         }
         _sstl_member_of_derived_class(this, _first_pointer)->~value_type();
         _sstl_member_of_derived_class(this, _first_pointer) = _inc_pointer(_sstl_member_of_derived_class(this, _first_pointer));
         --_sstl_member_of_derived_class(this, _size);
         return iterator{ this, _inc_pointer(pos_pointer) };
      }
      else
      {
         auto src = _inc_pointer(pos_pointer);
         auto dst = pos_pointer;
         while (dst != _sstl_member_of_derived_class(this, _last_pointer))
         {
            *dst = std::move(*src);
            dst = src;
            src = _inc_pointer(src);
         }
         auto pos_pointer_to_return =  pos_pointer != _sstl_member_of_derived_class(this, _last_pointer)
                                       ? pos_pointer : nullptr;
         _sstl_member_of_derived_class(this, _last_pointer)->~value_type();
         _sstl_member_of_derived_class(this, _last_pointer) = _dec_pointer(_sstl_member_of_derived_class(this, _last_pointer));
         --_sstl_member_of_derived_class(this, _size);
         return iterator{ this, pos_pointer_to_return };
      }
   }

   iterator erase(const_iterator range_begin, const_iterator range_end)
      _sstl_noexcept(std::is_nothrow_move_assignable<value_type>::value)
   {
      if(range_begin == range_end)
         return iterator{ this, const_cast<pointer>(range_begin._pos) };
      
      auto distance_to_begin = static_cast<size_type>(range_begin - cbegin());
      auto distance_to_end = static_cast<size_type>(cend() - range_end);
      auto range_size = range_end - range_begin;
      if(distance_to_begin < distance_to_end)
      {
         auto src = const_cast<pointer>(range_begin._pos);
         auto dst = (range_end != cend())
                  ? const_cast<pointer>(_dec_pointer(range_end._pos))
                  : _sstl_member_of_derived_class(this, _last_pointer);
         while (src != _sstl_member_of_derived_class(this, _first_pointer))
         {
            src = _dec_pointer(src);
            *dst = std::move(*src);
            dst = _dec_pointer(dst);
         }
         auto new_first_pointer = _inc_pointer(dst);
         while(true)
         {
            dst->~value_type();
            if (dst == _sstl_member_of_derived_class(this, _first_pointer))
               break;
            dst = _dec_pointer(dst);
         };
         _sstl_member_of_derived_class(this, _first_pointer) = new_first_pointer;
         _sstl_member_of_derived_class(this, _size) -= range_size;
         return iterator{ this, const_cast<pointer>(range_end._pos) };
      }
      else
      {
         auto src = (range_end != cend())
                  ? const_cast<pointer>(_dec_pointer(range_end._pos))
                  : _sstl_member_of_derived_class(this, _last_pointer);
         auto dst = const_cast<pointer>(range_begin._pos);
         while (src != _sstl_member_of_derived_class(this, _last_pointer))
         {
            src = _inc_pointer(src);
            *dst = std::move(*src);
            dst = _inc_pointer(dst);
         }
         auto new_last_pointer = _dec_pointer(dst);
         while(true)
         {
            dst->~value_type();
            if (dst == _sstl_member_of_derived_class(this, _last_pointer))
               break;
            dst = _inc_pointer(dst);
         }
         pointer return_pos = (range_end._pos != nullptr)
                           ? const_cast<pointer>(range_begin._pos)
                           : nullptr;
         _sstl_member_of_derived_class(this, _last_pointer) = new_last_pointer;
         _sstl_member_of_derived_class(this, _size) -= range_size;
         return iterator{ this, return_pos };
      }
      
   }

   template<class... Args>
   iterator emplace(const_iterator pos, Args&&... args)
      _sstl_noexcept(noexcept(std::declval<deque>()._emplace_value(std::declval<const_iterator>(),
                                                                     std::declval<value_type&&>())))
   {
      value_type value(std::forward<Args>(args)...);
      return _emplace_value(pos, std::move(value));
   }

   void push_front(const_reference value)
      _sstl_noexcept(noexcept(std::declval<deque>().emplace_front(std::declval<const_reference>())))
   {
      emplace_front(value);
   }

   void push_front(value_type&& value)
      _sstl_noexcept(noexcept(std::declval<deque>().emplace_front(std::declval<value_type&&>())))
   {
      emplace_front(std::move(value));
   }

   template<class... Args>
   void emplace_front(Args&&... value)
      _sstl_noexcept(std::is_nothrow_constructible<value_type, typename std::add_rvalue_reference<Args>::type...>::value)
   {
      sstl_assert(!full());
      pointer new_first_pointer;
      if(!empty())
      {
         new_first_pointer = _dec_pointer(_sstl_member_of_derived_class(this, _first_pointer));
      }
      else
      {
         new_first_pointer = _sstl_member_of_derived_class(this, _last_pointer);
      }
      new(new_first_pointer) value_type(std::forward<Args>(value)...);
      _sstl_member_of_derived_class(this, _first_pointer) = new_first_pointer;
      ++_sstl_member_of_derived_class(this, _size);
   }

   void push_back(const_reference value)
      _sstl_noexcept(noexcept(std::declval<deque>().emplace_back(std::declval<const_reference>())))
   {
      emplace_back(value);
   }

   void push_back(value_type&& value)
      _sstl_noexcept(noexcept(std::declval<deque>().emplace_back(std::declval<value_type&&>())))
   {
      emplace_back(std::move(value));
   }

   template<class... Args>
   void emplace_back(Args&&... args)
      _sstl_noexcept(std::is_nothrow_constructible<value_type, typename std::add_rvalue_reference<Args>::type...>::value)
   {
      sstl_assert(!full());
      auto new_last_pointer = _inc_pointer(_sstl_member_of_derived_class(this, _last_pointer));
      new(new_last_pointer) value_type(std::forward<Args>(args)...);
      _sstl_member_of_derived_class(this, _last_pointer) = new_last_pointer;
      ++_sstl_member_of_derived_class(this, _size);
   }

   void pop_back() _sstl_noexcept_
   {
      sstl_assert(!empty());
      _sstl_member_of_derived_class(this, _last_pointer)->~value_type();
      _sstl_member_of_derived_class(this, _last_pointer) = _dec_pointer(_sstl_member_of_derived_class(this, _last_pointer));
      --_sstl_member_of_derived_class(this, _size);
   }

   void pop_front()
      _sstl_noexcept(std::is_nothrow_destructible<value_type>::value)
   {
      sstl_assert(!empty());
      _sstl_member_of_derived_class(this, _first_pointer)->~value_type();
      _sstl_member_of_derived_class(this, _first_pointer) = _inc_pointer(_sstl_member_of_derived_class(this, _first_pointer));
      --_sstl_member_of_derived_class(this, _size);
   }

   void swap(deque& rhs)
      _sstl_noexcept(std::is_nothrow_move_constructible<value_type>::value
                  && std::is_nothrow_move_assignable<value_type>::value)
   {
      if(size() < rhs.size())
      {
         auto pos = std::swap_ranges(begin(), end(), rhs.begin());
         insert(end(), std::make_move_iterator(pos), std::make_move_iterator(rhs.end()));
         rhs.erase(pos, rhs.end());
      }
      else
      {
         auto pos = std::swap_ranges(rhs.begin(), rhs.end(), begin());
         rhs.insert(rhs.cend(), std::make_move_iterator(pos), std::make_move_iterator(end()));
         erase(pos, cend());
      }
   }

protected:
   deque() _sstl_noexcept_ = default;
   deque(const deque&) _sstl_noexcept_ = default;
   deque(deque&&) _sstl_noexcept_ {}; //MSVC (VS2013) does not support default move special member functions
   ~deque() = default;

   void _count_constructor(size_type count, const_reference value)
      _sstl_noexcept(std::is_nothrow_copy_constructible<value_type>::value)
   {
      sstl_assert(count <= capacity());
      auto pos = _sstl_member_of_derived_class(this, _first_pointer);
      auto last_pos = pos+count;
      #if _sstl_has_exceptions()
      try
      {
      #endif
         while(pos != last_pos)
         {
            new(pos) value_type(value);
            ++pos;
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         _sstl_member_of_derived_class(this, _last_pointer) = pos - 1;
         _sstl_member_of_derived_class(this, _size) = pos - _sstl_member_of_derived_class(this, _first_pointer);
         clear();
         throw;
      }
      #endif
      _sstl_member_of_derived_class(this, _size) = count;
      _sstl_member_of_derived_class(this, _last_pointer) = pos - 1;
   }

   template<class TIterator, class = typename std::enable_if<_is_input_iterator<TIterator>::value>::type>
   void _range_constructor(TIterator range_begin, TIterator range_end)
      _sstl_noexcept(std::is_nothrow_copy_constructible<value_type>::value)
   {
      auto src = range_begin;
      auto dst = _begin_storage()-1;
      #if _sstl_has_exceptions()
      try
      {
      #endif
         while(src != range_end)
         {
            sstl_assert(size() < capacity());
            new(++dst) value_type(*src);
            ++_sstl_member_of_derived_class(this, _size);
            ++src;
         }
         _sstl_member_of_derived_class(this, _last_pointer) = dst;
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         _sstl_member_of_derived_class(this, _last_pointer) = dst - 1;
         clear();
         throw;
      }
      #endif
   }

   void _move_constructor(deque&& rhs)
      _sstl_noexcept(std::is_nothrow_move_constructible<value_type>::value)
   {
      sstl_assert(rhs.size() <= capacity());
      auto src = _sstl_member_of_derived_class(&rhs, _first_pointer);
      auto dst = _begin_storage();
      auto remaining_move_constructions = rhs.size();
      #if _sstl_has_exceptions()
      try
      {
      #endif
         while(remaining_move_constructions > 0)
         {
            new(dst) value_type(std::move(*src));
            src->~value_type();
            src = rhs._inc_pointer(src);
            ++dst;
            --remaining_move_constructions;
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         auto number_of_move_constructions = rhs.size() - remaining_move_constructions;
         _sstl_member_of_derived_class(&rhs, _first_pointer) = std::addressof(*src);
         _sstl_member_of_derived_class(&rhs, _size) -= number_of_move_constructions;
         _sstl_member_of_derived_class(this, _last_pointer) = dst - 1;
         _sstl_member_of_derived_class(this, _size) = number_of_move_constructions;
         clear();
         throw;
      }
      #endif
      _sstl_member_of_derived_class(this, _last_pointer) = dst - 1;
      _sstl_member_of_derived_class(this, _size) = rhs.size();
      _sstl_member_of_derived_class(&rhs, _last_pointer) = _sstl_member_of_derived_class(&rhs, _first_pointer) - 1;
      _sstl_member_of_derived_class(&rhs, _size) = 0;
   }

   template<class TIterator, class = typename std::enable_if<_is_input_iterator<TIterator>::value>::type>
   void _range_assignment(TIterator range_begin, TIterator range_end)
      _sstl_noexcept(std::is_nothrow_copy_assignable<value_type>::value
                     && std::is_nothrow_copy_constructible<value_type>::value)
   {
      auto src = range_begin;
      auto dst = _sstl_member_of_derived_class(this, _first_pointer);

      size_type assignments = 0;
      while(src != range_end && assignments < size())
      {
         *dst = *src;
         dst = _inc_pointer(dst);
         ++src;
         ++assignments;
      }

      size_type new_size = assignments;

      #if _sstl_has_exceptions()
      try
      {
      #endif
         while(src != range_end)
         {
            sstl_assert(new_size < capacity());
            new(dst) value_type(*src);
            dst = _inc_pointer(dst);
            ++src;
            ++new_size;
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         dst = _dec_pointer(dst);
         _sstl_member_of_derived_class(this, _last_pointer) = dst;
         _sstl_member_of_derived_class(this, _size) = new_size;
         throw;
      }
      #endif

      auto new_last_pointer = dst;
      new_last_pointer = _dec_pointer(new_last_pointer);

      size_type destructions = new_size < size() ? size() - new_size : 0;
      while(destructions > 0)
      {
         dst->~value_type();
         dst = _inc_pointer(dst);
         --destructions;
      }

      _sstl_member_of_derived_class(this, _last_pointer) = new_last_pointer;
      _sstl_member_of_derived_class(this, _size) = new_size;
   }

   template<class TValue>
   iterator _emplace_value(const_iterator pos, TValue&& value)
      _sstl_noexcept(std::is_nothrow_move_constructible<value_type>()
                  && std::is_nothrow_move_assignable<value_type>())
   {
      sstl_assert(!full());
      auto distance_to_begin = std::distance(cbegin(), pos);
      auto distance_to_end = std::distance(pos, cend());
      if(distance_to_begin < distance_to_end)
      {
         auto src = _sstl_member_of_derived_class(this, _first_pointer);
         auto dst = _dec_pointer(_sstl_member_of_derived_class(this, _first_pointer));
         auto dst_end = _add_offset_to_pointer(dst, distance_to_begin);

         if(distance_to_begin > 0)
         {
            new(dst) value_type(std::move(*src));
            _sstl_member_of_derived_class(this, _first_pointer) = dst;
            ++_sstl_member_of_derived_class(this, _size);

            src = _inc_pointer(src);
            dst = _inc_pointer(dst);

            while(dst != dst_end)
            {
               *dst = std::move(*src);
               src = _inc_pointer(src);
               dst = _inc_pointer(dst);
            }
            *dst_end = std::forward<TValue>(value);
         }
         else
         {
            new(dst_end) value_type(std::forward<TValue>(value));
            _sstl_member_of_derived_class(this, _first_pointer) = dst;
            ++_sstl_member_of_derived_class(this, _size);
         }

         return iterator{this, dst_end};
      }
      else
      {
         auto src = _sstl_member_of_derived_class(this, _last_pointer);
         auto dst = _inc_pointer(_sstl_member_of_derived_class(this, _last_pointer));
         auto dst_end = _subtract_offset_to_pointer(dst, distance_to_end);

         if(distance_to_end > 0)
         {
            new(dst) value_type(std::move(*src));
            _sstl_member_of_derived_class(this, _last_pointer) = dst;
            ++_sstl_member_of_derived_class(this, _size);

            src = _dec_pointer(src);
            dst = _dec_pointer(dst);

            while(dst != dst_end)
            {
               *dst = std::move(*src);
               src = _dec_pointer(src);
               dst = _dec_pointer(dst);
            }
            *dst_end = std::forward<TValue>(value);
         }
         else
         {
            new(dst_end) value_type(std::forward<TValue>(value));
            _sstl_member_of_derived_class(this, _last_pointer) = dst;
            ++_sstl_member_of_derived_class(this, _size);
         }

         return iterator{this, dst_end};
      }
   }

   pointer _shift_from_begin_to_pos_by_n_positions(size_type n, size_type distance_to_begin)
      _sstl_noexcept(   std::is_nothrow_move_constructible<value_type>::value
                     && std::is_nothrow_move_assignable<value_type>::value)
   {
      auto number_of_constructions = std::min(n, distance_to_begin);
      auto number_of_assignments = distance_to_begin - number_of_constructions;

      auto dst_first = _subtract_offset_to_pointer(_sstl_member_of_derived_class(this, _first_pointer), n);
      auto dst = dst_first;
      auto src = _sstl_member_of_derived_class(this, _first_pointer);

      size_type remaining_constructions;
      #if _sstl_has_exceptions()
      try
      {
      #endif
         for(remaining_constructions=number_of_constructions; remaining_constructions>0; --remaining_constructions)
         {
            new(dst) value_type(std::move(*src));
            src = _inc_pointer(src);
            dst = _inc_pointer(dst);
         }

         for(size_type i=number_of_assignments; i>0; i--)
         {
            *dst = std::move(*src);
            src = _inc_pointer(src);
            dst = _inc_pointer(dst);
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         auto destructions = number_of_constructions - remaining_constructions;
         auto ptr = dst_first;
         for(size_type i=destructions; i>0; --i)
         {
            ptr->~value_type();
            ptr = _inc_pointer(ptr);
         }
         throw;
      }
      #endif

      _sstl_member_of_derived_class(this, _first_pointer) =
         _subtract_offset_to_pointer(_sstl_member_of_derived_class(this, _first_pointer), n);
      _sstl_member_of_derived_class(this, _size) += n;

      return _add_offset_to_pointer(_sstl_member_of_derived_class(this, _first_pointer), distance_to_begin);
   }

   pointer _shift_from_pos_to_end_by_n_positions(size_type n, size_type distance_to_end)
      _sstl_noexcept(   std::is_nothrow_move_constructible<value_type>::value
                     && std::is_nothrow_move_assignable<value_type>::value)
   {
      auto number_of_constructions = std::min(n, distance_to_end);
      auto number_of_assignments = distance_to_end - number_of_constructions;

      auto dst_first = _add_offset_to_pointer(_sstl_member_of_derived_class(this, _last_pointer), n);
      auto dst = dst_first;
      auto src = _sstl_member_of_derived_class(this, _last_pointer);

      size_type remaining_constructions;
      #if _sstl_has_exceptions()
      try
      {
      #endif
         for(remaining_constructions=number_of_constructions; remaining_constructions>0; --remaining_constructions)
         {
            new(dst) value_type(std::move(*src));
            src = _dec_pointer(src);
            dst = _dec_pointer(dst);
         }

         for(size_type i=number_of_assignments; i>0; --i)
         {
            *dst = std::move(*src);
            src = _dec_pointer(src);
            dst = _dec_pointer(dst);
         }
      #if _sstl_has_exceptions()
      }
      catch(...)
      {
         auto destructions = number_of_constructions - remaining_constructions;
         auto ptr = dst_first;
         for(size_type i=0; i<destructions; ++i)
         {
            ptr->~value_type();
            ptr = _dec_pointer(ptr);
         }
         throw;
      }
      #endif

      _sstl_member_of_derived_class(this, _last_pointer) =
         _add_offset_to_pointer(_sstl_member_of_derived_class(this, _last_pointer), n);
      _sstl_member_of_derived_class(this, _size) += n;

      return _subtract_offset_to_pointer(_sstl_member_of_derived_class(this, _last_pointer), distance_to_end);
   }

   pointer _inc_pointer(pointer ptr) const _sstl_noexcept_
   {
      ptr += 1;
      if (ptr == _sstl_member_of_derived_class(this, _end_storage))
         ptr = const_cast<pointer>(_begin_storage());
      return ptr;
   }

   const_pointer _inc_pointer(const_pointer ptr) const _sstl_noexcept_
   {
      return _inc_pointer(const_cast<pointer>(ptr));
   }

   pointer _dec_pointer(pointer ptr) const _sstl_noexcept_
   {
      ptr -= 1;
      if(ptr < _begin_storage())
         ptr = const_cast<pointer>(_sstl_member_of_derived_class(this, _end_storage)) - 1;
      return ptr;
   }

   const_pointer _dec_pointer(const_pointer ptr) const _sstl_noexcept_
   {
      return _dec_pointer(const_cast<pointer>(ptr));
   }

   pointer _add_offset_to_pointer(pointer ptr, size_type offset) const _sstl_noexcept_
   {
      auto begin_storage = const_cast<pointer>(_begin_storage());
      auto end_storage = const_cast<pointer>(_sstl_member_of_derived_class(this, _end_storage));

      ptr += offset;
      if(ptr >= end_storage)
      {
         ptr = begin_storage + (ptr - end_storage);
      }
      return ptr;
   }

   pointer _subtract_offset_to_pointer(pointer ptr, size_type offset) const _sstl_noexcept_
   {
      auto begin_storage = const_cast<pointer>(_begin_storage());
      auto end_storage = const_cast<pointer>(_sstl_member_of_derived_class(this, _end_storage));

      ptr -= offset;
      if(ptr < begin_storage)
      {
         ptr = end_storage - (begin_storage - ptr);
      }
      return ptr;
   }

   pointer _apply_offset_to_pointer(pointer ptr, difference_type offset) const _sstl_noexcept_
   {
      if(offset > 0)
      {
         ptr = _add_offset_to_pointer(ptr, offset);
      }
      else
      {
         ptr = _subtract_offset_to_pointer(ptr, -offset);
      }

      return ptr;
   }

   const_pointer _apply_offset_to_pointer(const_pointer ptr, difference_type offset) const _sstl_noexcept_
   {
      return _apply_offset_to_pointer(const_cast<pointer>(ptr), offset);
   }

   bool _is_pointer_one_past_last_pointer(const_pointer ptr) const _sstl_noexcept_
   {
      auto one_past_last_pointer = _sstl_member_of_derived_class(this, _last_pointer);
      one_past_last_pointer = _inc_pointer(one_past_last_pointer);
      return ptr == one_past_last_pointer;
   }

   pointer _begin_storage() _sstl_noexcept_
   {
      return static_cast<pointer>(static_cast<void*>(_sstl_member_of_derived_class(this, _buffer)));
   }

   const_pointer _begin_storage() const _sstl_noexcept_
   {
      return const_cast<deque&>(*this)._begin_storage();
   }
};

template<class T, size_t CAPACITY>
class deque : public deque<T>
{
private:
   using _base = deque<T>;
   using _type_for_hacky_derived_class_access = typename _base::_type_for_hacky_derived_class_access;

   template<class, size_t>
   friend class deque;

   friend class _dequeng_iterator<_base>;
   friend class _dequeng_iterator<const _base>;

public:
   using value_type = typename _base::value_type;
   using size_type = typename _base::size_type;
   using difference_type = typename _base::difference_type;
   using reference = typename _base::reference;
   using const_reference = typename _base::const_reference;
   using pointer = typename _base::pointer;
   using const_pointer = typename _base::const_pointer;
   using iterator = typename _base::iterator;
   using const_iterator = typename _base::const_iterator;
   using reverse_iterator = typename _base::reverse_iterator;
   using const_reverse_iterator = typename _base::const_reverse_iterator;

public:
   deque() _sstl_noexcept_
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
   }

   explicit deque(size_type count, const_reference value = value_type())
      _sstl_noexcept(noexcept(std::declval<_base>()._count_constructor(std::declval<size_type>(), std::declval<value_type>())))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
      _base::_count_constructor(count, value);
   }

   template<class TIterator, class = typename std::enable_if<_is_input_iterator<TIterator>::value>::type>
   deque(TIterator range_begin, TIterator range_end)
      _sstl_noexcept(noexcept(std::declval<_base>()._range_constructor( std::declval<TIterator>(),
                                                                        std::declval<TIterator>())))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
      _base::_range_constructor(range_begin, range_end);
   }

   //copy construction from any instance with same value type (capacity doesn't matter)
   deque(const _base& rhs)
      _sstl_noexcept(noexcept(std::declval<_base>()._range_constructor( std::declval<const_iterator>(),
                                                                        std::declval<const_iterator>())))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
      _base::_range_constructor(const_cast<_base&>(rhs).cbegin(), const_cast<_base&>(rhs).cend());
   }

   deque(const deque& rhs)
      _sstl_noexcept(noexcept(deque(std::declval<const _base&>())))
      : deque(static_cast<const _base&>(rhs))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
   }

   //move construction from any instance with same value type (capacity doesn't matter)
   deque(_base&& rhs)
      _sstl_noexcept(noexcept(std::declval<_base>()._move_constructor(std::declval<_base>())))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
      _base::_move_constructor(std::move(rhs));
   }

   deque(deque&& rhs)
      _sstl_noexcept(noexcept(deque(std::declval<_base>())))
      : deque(static_cast<_base&&>(rhs))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
   }

   deque(std::initializer_list<value_type> init)
      _sstl_noexcept(noexcept(std::declval<_base>()._range_constructor( std::declval<std::initializer_list<value_type>>().begin(),
                                                                        std::declval<std::initializer_list<value_type>>().end())))
   {
      _assert_hacky_derived_class_access_is_valid<deque<value_type>, deque, _type_for_hacky_derived_class_access>();
      _base::_range_constructor(init.begin(), init.end());
   }

   ~deque()
   {
      _base::clear();
   }

   //copy assignment from any instance with same value type (capacity doesn't matter)
   deque& operator=(const _base& rhs)
      _sstl_noexcept(noexcept(std::declval<_base>().operator=(std::declval<const _base&>())))
   {
      return reinterpret_cast<deque&>(_base::operator=(rhs));
   }

   deque& operator=(const deque& rhs)
      _sstl_noexcept(noexcept(std::declval<_base>().operator=(std::declval<const _base&>())))
   {
      return reinterpret_cast<deque&>(_base::operator=(rhs));
   }

   //move assignment from any instance with same value type (capacity doesn't matter)
   deque& operator=(_base&& rhs)
      _sstl_noexcept(noexcept(std::declval<_base>().operator=(std::declval<_base&&>())))
   {
      return reinterpret_cast<deque&>(_base::operator=(std::move(rhs)));
   }

   deque& operator=(deque&& rhs)
      _sstl_noexcept(noexcept(std::declval<_base>().operator=(std::declval<_base&&>())))
   {
      return reinterpret_cast<deque&>(_base::operator=(std::move(rhs)));
   }

   deque& operator=(std::initializer_list<value_type> ilist)
      _sstl_noexcept(noexcept(std::declval<_base>().operator=(std::declval<std::initializer_list<value_type>>())))
   {
      return reinterpret_cast<deque&>(_base::operator=(ilist));
   }

private:
   size_type _size{ 0 };
   pointer _first_pointer{ _base::_begin_storage() };
   pointer _last_pointer{ _base::_begin_storage() + CAPACITY - 1 };
   pointer _end_storage{ _base::_begin_storage() + CAPACITY };
   typename _aligned_storage<sizeof(value_type), std::alignment_of<value_type>::value>::type _buffer[CAPACITY];
};

template<class T>
inline bool operator==(const deque<T>& lhs, const deque<T>& rhs) _sstl_noexcept_
{
   return lhs.size() == rhs.size() && std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template<class T>
inline bool operator!=(const deque<T>& lhs, const deque<T>& rhs) _sstl_noexcept_
{
   return !operator==(lhs, rhs);
}

template<class T>
inline bool operator<(const deque<T>& lhs, const deque<T>& rhs) _sstl_noexcept_
{
   return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template<class T>
inline bool operator<=(const deque<T>& lhs, const deque<T>& rhs) _sstl_noexcept_
{
   return !(rhs < lhs);
}

template<class T>
inline bool operator>(const deque<T>& lhs, const deque<T>& rhs) _sstl_noexcept_
{
   return rhs < lhs;
}

template<class T>
inline bool operator>=(const deque<T>& lhs, const deque<T>& rhs) _sstl_noexcept_
{
   return !(lhs < rhs);
}

template<class T>
void swap(deque<T>& lhs, deque<T>& rhs)
   _sstl_noexcept(noexcept(std::declval<deque<T>&>().swap(std::declval<deque<T>&>())))
{
   lhs.swap(rhs);
}

}

#endif
