#pragma once
#include <iostream>
#include <algorithm>
#include <tuple>

template <class T>
T& endian_swap(T& value)
{
  if (sizeof(T) > 1)
  {
    uint8_t* bit = reinterpret_cast<uint8_t*>(&value);
    for (size_t i = 0; i != sizeof(T) / 2; ++i)
      std::swap(bit[i], bit[sizeof(T) - i - 1]);
  }
  return value;
};
enum EndianType { LittleEndian = 0, BigEndian };

template <class T, EndianType ENDIAN>
struct FileIO
{
  static T read(std::istream& is)
  {
    T value;
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
    if (ENDIAN == BigEndian)
      endian_swap(value);
    return value;
  }
  static bool write( std::ostream& os, const T& value )
  {
      return (bool)os.write(reinterpret_cast<char*>(&value), sizeof(T));

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
};
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
};

template <class T>
T mimetype2(const std::string& s)
{
  T value = *reinterpret_cast<const T*>(s.data());
  return endian_swap(value);
}
template <>
std::string mimetype2(const std::string& s)
{
  std::string value;
  value.reserve(s.size() / 2);
  for (decltype(s.size()) i = 1; i < s.size(); i += 2)
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
