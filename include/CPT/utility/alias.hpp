#pragma once
#define FUNC_ALIAS(new_f, old_f) \
template<class...T> \
decltype(auto) new_f(T&&... o) \
{ return old_f(std::forward<T>(o)...); }
