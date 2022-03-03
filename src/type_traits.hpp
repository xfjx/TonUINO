#ifndef SRC_TYPE_TRAITS_HPP_
#define SRC_TYPE_TRAITS_HPP_


//! compile-time if
template<bool cond, typename T1, typename T2> struct if_              { typedef T1 result_type; };
template<           typename T1, typename T2> struct if_<false,T1,T2> { typedef T2 result_type; };
//! bool-to-type
template<bool b>         struct bool_ { enum { value = b }; typedef bool_ result_type; };
//! determines whether two types have the same type
template<typename T1, typename T2> struct is_same_type      : bool_<false> {};
template<typename T>               struct is_same_type<T,T> : bool_<true > {};

template<bool B, class T, class F>
struct conditional { typedef T type; };
template<class T, class F>
struct conditional<false, T, F> { typedef F type; };


#endif /* SRC_TYPE_TRAITS_HPP_ */
