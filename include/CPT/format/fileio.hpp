#pragma once
#include <iostream>
#include <algorithm>
#include <tuple>
#include <cstdint>
#include <Nucleona/language.hpp>
// FIXME No document
// FIXME type member indent level 
namespace cpt{ namespace format{

// FIXME suggestion : EndianType can be a class 
// FIXME enum namming should all capital
enum EndianType { LittleEndian = 0, BigEndian };
template < EndianType ENDIAN > struct EndianSwap
{
    template <class T>
    static T& run(T& value) 
    {
        return value;
    }
};
template <>
struct EndianSwap < BigEndian >
{
    template <class T>
    static T run(T&& value)
    {
      if (sizeof(T) > 1)
      {
        uint8_t* bit = reinterpret_cast<uint8_t*>(&value);
        for (size_t i = 0; i != sizeof(T) / 2; ++i)
          std::swap(bit[i], bit[sizeof(T) - i - 1]);
      }
      return value;
    };
};

template <class T, EndianType ENDIAN>
struct FileIO
{
  static T read(std::istream& is)
  {
    T value;
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
    EndianSwap<ENDIAN>::run(value);
    return value;
  }
  static bool write( std::ostream& os, T value )
  {
    EndianSwap<ENDIAN>::run(value);
    os.write( reinterpret_cast<char*>(&value), sizeof(T) );
    return !os.fail();
  }
  static bool write( std::ostream& os, T value, const std::ostream::pos_type& fpos)
  {
    auto now_pos = os.tellp();
    EndianSwap<ENDIAN>::run(value);
    os.seekp(fpos);
    os.write( reinterpret_cast<char*>(&value), sizeof(T) );
    os.seekp(now_pos);
    return !os.fail();
  }
  static bool skip( std::ostream& os)
  {
    T value;
    os.write( reinterpret_cast<char*>(&value), sizeof(T) );
    return !os.fail();
  }
  
  static std::function<bool(T&&)> future_write(std::ostream& os)
  {
      auto helper = [ &os, pos = os.tellp() ]( T&& value )
      {
          // std::cout << "pos : " << pos << " : " << value << std::endl;
          return FileIO<T, ENDIAN>::write( os, FWD(value), pos );
      };
      if( skip( os ) ) return helper;
      else return nullptr;
  }

};
template <EndianType ENDIAN>
struct FileIO<std::string, ENDIAN>
{
  static std::string read(std::istream& is)
  {
    auto len = FileIO<uint32_t, ENDIAN>::read(is);
    std::string value;
    value.reserve(len);
    for (uint32_t i = 0; i != len; ++i)
      value.push_back(FileIO<uint8_t, ENDIAN>::read(is));
    return value;
  }
  static bool write( std::ostream& os, const std::string& str )
  {
      if( !FileIO<uint32_t, ENDIAN>::write( os, str.length() ) ) return false;
      else
      {
          for( auto c : str )
          {
              if ( !FileIO<uint8_t, ENDIAN>::write( os, c ) ) return false;
          }
      }
      return !os.fail();
  }
};
// TODO this is u16string type
template <EndianType ENDIAN>
struct FileIO<std::wstring, ENDIAN>
{
  static std::string read(std::istream& is)
  {
    auto len = FileIO<uint32_t, ENDIAN>::read(is);
    std::string value;
    value.reserve(len);
    for (uint32_t i = 0; i != len; ++i)
      value.push_back(FileIO<uint16_t, ENDIAN>::read(is));
    return value;
  }
  static bool write( std::ostream& os, const std::string& str )
  {
      if( !FileIO<uint32_t, ENDIAN>::write( os, str.length() ) ) return false;
      else
      {
          for( auto c : str )
          {
              if ( !FileIO<uint16_t, ENDIAN>::write( os, c ) ) return false;
          }
      }
      return !os.fail();

  }

};

template <class T>
T mimetype2(const std::string& s)
{
  T value = *reinterpret_cast<const T*>(s.data());
  return EndianSwap<BigEndian>::run(value);
}
template <>
std::string mimetype2(const std::string& s)
{
  std::string value;
  value.reserve(s.size() / 2);
  for (int i = 1; i < s.size(); i += 2)
    value.push_back(s.at(i));
  return value;
}

void print_name_value_types(std::ostream& os,
                            const std::tuple<std::string,
                                             std::string,
                                             std::string>& tup)
{
  auto&& name = std::get<0>(tup);
  auto&& temp = std::get<1>(tup);
  auto&& type = std::get<2>(tup);

  os << '\n' << "  +  name: " << name
     << '\n' << "  + value: ";
  if (type == "text/plain" or type == "text/ascii")
  {
    std::string s = (type == "text/plain")?
                    mimetype2<std::string>(temp): temp;
    s.erase(std::remove_if(s.begin(), s.end(), 
      [](char c){ return !std::isprint(c); }), s.end());
    os << s;
  }
  else if (type == "text/x-calvin-integer-8")
    os << static_cast<int16_t>(mimetype2<int8_t>(temp));
  else if (type == "text/x-calvin-unsigned-integer-8")
    os << static_cast<uint16_t>(mimetype2<uint8_t>(temp));
  else if (type == "text/x-calvin-integer-16")
    os << mimetype2<int16_t>(temp);
  else if (type == "text/x-calvin-unsigned-integer-16")
    os << mimetype2<uint16_t>(temp);
  else if (type == "text/x-calvin-integer-32")
    os << mimetype2<int32_t>(temp);
  else if (type == "text/x-calvin-unsigned-integer-32")
    os << mimetype2<uint32_t>(temp);
  else if (type == "text/x-calvin-float")
    os << mimetype2<float>(temp);
  else
    os << "(Unsupported)\n";
  os << '\n' << "  +  type: " << type << '\n';
}
}}
