/* A very simple result type
(C) 2018-2020 Niall Douglas <http://www.nedproductions.biz/> (11 commits)
File Created: Apr 2018


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

#ifndef BOOST_OUTCOME_EXPERIMENTAL_STATUS_RESULT_HPP
#define BOOST_OUTCOME_EXPERIMENTAL_STATUS_RESULT_HPP

#include "../basic_result.hpp"
#include "../policy/fail_to_compile_observers.hpp"

#include "status-code/system_error2.hpp"

BOOST_OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace trait
{
  namespace detail
  {
    // Shortcut this for lower build impact. Used to tell outcome's converting constructors
    // that they can do E => EC or E => EP as necessary.
    template <class DomainType> struct _is_error_code_available<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::status_code<DomainType>>
    {
      static constexpr bool value = true;
      using type = BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::status_code<DomainType>;
    };
    template <class DomainType> struct _is_error_code_available<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::errored_status_code<DomainType>>
    {
      static constexpr bool value = true;
      using type = BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::errored_status_code<DomainType>;
    };
  }    // namespace detail
#if 0  // Do NOT enable weakened implicit construction for these types
  template <class DomainType> struct is_error_type<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::status_code<DomainType>>
  {
    static constexpr bool value = true;
  };
  template <> struct is_error_type<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::errc>
  {
    static constexpr bool value = true;
  };
  template <class DomainType, class Enum> struct is_error_type_enum<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::status_code<DomainType>, Enum>
  {
    static constexpr bool value = boost::system::is_error_condition_enum<Enum>::value;
  };
#endif

  template <class DomainType> struct is_move_bitcopying<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::status_code<DomainType>>
  {
    static constexpr bool value = BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::traits::is_move_bitcopying<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::status_code<DomainType>>::value;
  };
  template <class DomainType> struct is_move_bitcopying<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::errored_status_code<DomainType>>
  {
    static constexpr bool value = BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::traits::is_move_bitcopying<BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::errored_status_code<DomainType>>::value;
  };
}  // namespace trait

namespace detail
{
  // Customise _set_error_is_errno
  template <class State> constexpr inline void _set_error_is_errno(State &state, const BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::generic_code & /*unused*/)
  {
    state._status.set_have_error_is_errno(true);
  }
#ifndef BOOST_OUTCOME_SYSTEM_ERROR2_NOT_POSIX
  template <class State> constexpr inline void _set_error_is_errno(State &state, const BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::posix_code & /*unused*/)
  {
    state._status.set_have_error_is_errno(true);
  }
#endif
  template <class State> constexpr inline void _set_error_is_errno(State &state, const BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::errc & /*unused*/)
  {
    state._status.set_have_error_is_errno(true);
  }

}  // namespace detail

namespace experimental
{
  using namespace BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE;
  using BOOST_OUTCOME_V2_NAMESPACE::failure;
  using BOOST_OUTCOME_V2_NAMESPACE::success;

  namespace policy
  {
    using namespace BOOST_OUTCOME_V2_NAMESPACE::policy;
    template <class T, class EC, class E> struct status_code_throw
    {
      static_assert(!std::is_same<T, T>::value,
                    "policy::status_code_throw not specialised for these types, did you use status_result<T, status_code<DomainType>, E>?");
    };
    template <class T, class DomainType> struct status_code_throw<T, status_code<DomainType>, void> : base
    {
      using _base = base;
      template <class Impl> static constexpr void wide_value_check(Impl &&self)
      {
        if(!base::_has_value(static_cast<Impl &&>(self)))
        {
          if(base::_has_error(static_cast<Impl &&>(self)))
          {
#ifndef BOOST_NO_EXCEPTIONS
            base::_error(static_cast<Impl &&>(self)).throw_exception();
#else
            BOOST_OUTCOME_THROW_EXCEPTION("wide value check failed");
#endif
          }
        }
      }
      template <class Impl> static constexpr void wide_error_check(Impl &&self) { _base::narrow_error_check(static_cast<Impl &&>(self)); }
    };
    template <class T, class DomainType>
    struct status_code_throw<T, errored_status_code<DomainType>, void> : status_code_throw<T, status_code<DomainType>, void>
    {
      status_code_throw() = default;
      using status_code_throw<T, status_code<DomainType>, void>::status_code_throw;
    };

    template <class T, class EC>
    using default_status_result_policy = std::conditional_t<                            //
    std::is_void<EC>::value,                                                            //
    BOOST_OUTCOME_V2_NAMESPACE::policy::terminate,                                            //
    std::conditional_t<is_status_code<EC>::value || is_errored_status_code<EC>::value,  //
                       status_code_throw<T, EC, void>,                                  //
                       BOOST_OUTCOME_V2_NAMESPACE::policy::fail_to_compile_observers          //
                       >>;
  }  // namespace policy

  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class R, class S = errored_status_code<erased<typename system_code::value_type>>,
            class NoValuePolicy = policy::default_status_result_policy<R, S>>  //
  using status_result = basic_result<R, S, NoValuePolicy>;

}  // namespace experimental

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
