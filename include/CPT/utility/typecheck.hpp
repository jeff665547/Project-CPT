#pragma once
#include <utility>
#include <type_traits>
namespace cpt {
namespace utility {
/* create type check for some template type */
#define CREATE_TYPECHECKER(tpl_name) \
template<class T> struct Is##tpl_name : public std::false_type{}; \
template<class... T> struct Is##tpl_name<tpl_name<T...>> : public std::true_type{}; \
template<class T> using Is##tpl_name##Decay \
    = Is##tpl_name<std::decay_t<T>>;


/* use type checker in enable_if */

#define FTP_TYPE_CHECK(st, tt) \
std::enable_if_t< Is##tt##Decay<st>::value, int > = 0
#define FTR_YPE_CHECK(st, tt, ret_t) \
std::enable_if_t< Is##tt##Decay<st>::value, ret_t >

#define TP_TYPE_CHECK(st, tt) \
std::enable_if_t< std::is_same<std::decay_t<st>, tt>::value, int> = 0

#define TR_YPE_CHECK(st, tt, ret_t) \
std::enable_if_t< std::is_same<std::decay_t<st>, tt>::value, ret_t >
}}
