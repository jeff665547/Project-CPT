#pragma once
#include <cstdint>
#include <limits>

namespace floating_point {

template <class T, intmax_t N, intmax_t D = 1>
struct Rational
{
    static constexpr auto value = static_cast<T>(N) / D;
    static constexpr auto numerator = N;
    static constexpr auto denominator = D;
};
template <class T, intmax_t N>
struct Rational<T, N, 0>
{
    static constexpr auto value = std::numeric_limits<T>::infinity();
    static constexpr intmax_t numerator = 1;
    static constexpr intmax_t denominator = 0;
};

template <class T>
constexpr T pow(T x, uint32_t n)
{
    return n? x * pow(x, n - 1): 1;
}
template <class T>
constexpr intmax_t numerator_of(T x, uint32_t p = 0)
{
    return x == std::numeric_limits<T>::infinity()? 1
         : x < 0? -numerator_of(-x, p)
         : x == intmax_t(x)? x
         : numerator_of(x * 10, p + 1);
}
template <class T>
constexpr intmax_t denominator_of(T x, uint32_t p = 0)
{
    return x == std::numeric_limits<intmax_t>::infinity()? 0
         : x == intmax_t(x)? pow(10, p)
         : denominator_of(x * 10, p + 1);
}

} // namespace floating_point

#define RATIONAL(TYPE,VALUE) \
floating_point::Rational<TYPE \
  , floating_point::numerator_of(VALUE) \
  , floating_point::denominator_of(VALUE) \
>
#define REAL_(VALUE) RATIONAL(decltype(VALUE),VALUE)
#define double_(VALUE) RATIONAL(double,VALUE)
#define float_(VALUE) RATIONAL(float,VALUE)
