#pragma once
#include <utility>
#include <algorithm>
namespace cpt {
namespace utility {
namespace constexpr_ds {

struct String
{
  public:
    template<int N>
    constexpr String( char const (&s)[N] )
    : content_   ( s )
    , length_    ( N - 1 )
    {}
    constexpr String( int n, const char* s )
    : content_ ( s )
    , length_  ( n )
    {}
    constexpr bool operator== ( const String& s ) const
    {
        if ( length_ == s.length_ )
        {
            for ( std::size_t i(0); i < length_; i ++ )
            {
                if ( s.content_[i] != content_[i] ) return false;
            }
            return true;
        }
        else return false;
    }
    constexpr bool operator< ( const String& s ) const
    {
        for( std::size_t i (0); i < std::min(s.length_, length_); i ++ )
        {
            if ( content_[i] == s.content_[i] ) continue;
            else return content_[i] < s.content_[i];
        }
        return length_ < s.length_;
    }
    constexpr std::size_t length() const
    {
        return length_;
    }
    constexpr const char* content() const
    {
        return content_;
    }
    constexpr const char& head() const
    {
        return content_[0];
    }
    constexpr String tail() const
    {
        return String( length_ - 1, content_ + 1 );
    }
  private:
    const char* content_;
    std::size_t length_;
};

}}}
