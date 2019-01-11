/* Traits for Outcome
(C) 2018-2019 Niall Douglas <http://www.nedproductions.biz/> (59 commits)
File Created: March 2018


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BOOST_OUTCOME_TRAIT_STD_EXCEPTION_HPP
#define BOOST_OUTCOME_TRAIT_STD_EXCEPTION_HPP

#include "../config.hpp"

#include <exception>

BOOST_OUTCOME_V2_NAMESPACE_BEGIN

//! Namespace for policies
namespace policy
{
  namespace detail
  {
    /* Pass through `make_exception_ptr` function for `std::exception_ptr`.
    */
    inline std::exception_ptr make_exception_ptr(std::exception_ptr v) { return v; }

    // Try ADL, if not use fall backs above
    template <class T> constexpr inline decltype(auto) exception_ptr(T &&v) { return make_exception_ptr(std::forward<T>(v)); }
  }  // namespace detail

  //! Used by policies to extract a `std::exception_ptr` from some input `T` via ADL discovery of some `make_exception_ptr(T)` function.
  template <class T> constexpr inline decltype(auto) exception_ptr(T &&v) { return detail::exception_ptr(std::forward<T>(v)); }

  namespace detail
  {
    template <bool has_error_payload> struct _rethrow_exception
    {
      template <class Exception> explicit _rethrow_exception(Exception && /*unused*/)  // NOLINT
      {
      }
    };
    template <> struct _rethrow_exception<true>
    {
      template <class Exception> explicit _rethrow_exception(Exception &&excpt)  // NOLINT
      {
        // ADL
        rethrow_exception(policy::exception_ptr(std::forward<Exception>(excpt)));
      }
    };
  }  // namespace detail
}  // namespace policy

//! Namespace for traits
namespace trait
{
  namespace detail
  {
    template <class T> using devoid = BOOST_OUTCOME_V2_NAMESPACE::detail::devoid<T>;

    constexpr inline void make_exception_ptr(...);
    template <class T, typename V = std::decay_t<decltype(make_exception_ptr(std::declval<devoid<T>>()))>> struct has_exception_ptr
    {
      static constexpr bool value = false;
    };
    template <> struct has_exception_ptr<std::exception_ptr, void>
    {
      static constexpr bool value = true;
    };
    template <class T> struct has_exception_ptr<T, std::exception_ptr>
    {
      static constexpr bool value = true;
    };
  }  // namespace detail

  /*! Trait for whether a free function `make_exception_ptr(T)` returning a `std::exception_ptr` exists or not.
  Also returns true if `std::exception_ptr` is convertible from T.
  */
  template <class T> struct has_exception_ptr : detail::has_exception_ptr<std::decay_t<T>>
  {
  };
  /*! Trait for whether a free function `make_exception_ptr(T)` returning a `std::exception_ptr` exists or not.
  Also returns true if `std::exception_ptr` is convertible from T.
  */
  template <class T> constexpr bool has_exception_ptr_v = has_exception_ptr<std::decay_t<T>>::value;

  // std::exception_ptr is an error type
  template <> struct is_error_type<std::exception_ptr>
  {
    static constexpr bool value = true;
  };

}  // namespace trait

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
