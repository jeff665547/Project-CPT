#pragma once
#include <CPT/format/fileio.hpp>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <memory>
#include <vector>
#include <numeric>
#include <CPT/logger.hpp>
#include <CPT/format/chip_sample.hpp>
#include <cassert>
#include <Nucleona/language.hpp>
// http://media.affymetrix.com/support/developer/powertools/changelog/gcos-agcc/generic.html
// FIXME namespace
// FIXME document
// FIXME type member indent level
// FIXME public member no tail '_'
// TODO wstring should use 16 bit char string (u16string)
namespace cpt{ namespace format{
static constexpr EndianType GENERIC_CEL_ENDIAN_TYPE = BigEndian;
constexpr auto get_endian( uint8_t magic, uint8_t version )
{
    if ( magic == 59 && version == 1 )
        return BigEndian;
    else return LittleEndian;
}
class GenericDataHeader
{
 public:
  std::string identifier_;
  std::string guid_;
  std::string date_;
  std::string locale_;

  using NameValueType = std::tuple<std::string, std::string, std::string>;
  std::vector<NameValueType> nvts_;
  std::vector<GenericDataHeader> list_;
  
 public:
  void read(std::istream& is)
  {
    identifier_ = FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::read(is);
    guid_       = FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::read(is);
    date_       = FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    locale_     = FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is);

    auto num_params = FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    for (int32_t i = 0; i != num_params; ++i)
    {
      auto&& name (FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is));
      auto&& value(FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::read(is));
      auto&& type (FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is));
      nvts_.emplace_back(name, value, type);
    }
    auto num_headers = FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    list_.resize(num_headers);
    for (auto& header: this->list_)
      header.read(is);
  }
  bool write( std::ostream& os ) const
  {
    if ( !FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::write(os, identifier_ ) ) return false;
    if ( !FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::write(os, guid_       ) ) return false;
    if ( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write(os, date_       ) ) return false;
    if ( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write(os, locale_     ) ) return false;

    if ( !FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::write(os, nvts_.size()) ) return false;
    for ( auto&& nvt : nvts_ )
    {
        
        if( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write(os, std::get<0>(nvt)) ) 
            return false;
        if( !FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::write(os, std::get<1>(nvt)) ) 
            return false;
        if( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write(os, std::get<2>(nvt)) ) 
            return false;
    }
    if ( !FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::write(os, list_.size())) return false;
    for (auto& header: this->list_)
      if ( !header.write(os) ) return false;
    return true;
  }

  void print(std::ostream& os)
  {
    os << "Identifier: " << identifier_ << '\n'
       << "      GUID: " << guid_       << '\n'
       << "      Date: " << date_       << '\n'
       << "    Locale: " << locale_     << '\n'
       << "Parameters: "                << '\n';

    for (auto& nvt: nvts_)
      print_name_value_types(os, nvt);
    os << '\n';
    for (auto& item: list_)
      item.print(os);
  }
};
class GenericDataSet
{
 public:
  std::string name_;
  uint32_t num_cols_;
  uint32_t num_rows_;
  std::vector<std::string> cnames_;
  std::vector<int8_t> ctypes_;
  std::vector<int32_t> csizes_;
  std::vector<std::shared_ptr<char>> data_;

  using NameValueType = std::tuple<std::string, std::string, std::string>;
  std::vector<NameValueType> nvts_;

 public:
  void parse(std::istream& is) // TODO pass epos item 1 information
  {
    // static int xxxcounter = 0;
    // static int xxxcounter2 = 0;
    // if ( xxxload2 ) xxxcounter2 ++;
    // else xxxcounter ++;

    // std::cout << (uint32_t)is.tellg() << std::endl;
    name_ = FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is);

    auto num_params = FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    for (auto i = 0; i != num_params; ++i)
    {
      auto&& name  = FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is);
      auto&& value = FileIO<std::string,  GENERIC_CEL_ENDIAN_TYPE>::read(is);
      auto&& type  = FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is);
      nvts_.emplace_back(name, value, type);
    }

    num_cols_ = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    for (auto c = 0u; c != num_cols_; ++c)
    {
      cnames_.emplace_back(FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is));
      ctypes_.emplace_back(FileIO<int8_t, GENERIC_CEL_ENDIAN_TYPE>::read(is));
      csizes_.emplace_back(FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is));
    }

    auto size = std::accumulate(csizes_.begin(), csizes_.end(), 0u);
    num_rows_ = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    if ( size != 0 )
    {
        for (auto r = 0u; r != num_rows_; ++r)
        {
          auto ptr = new char[size];
          is.read(ptr, size);
          data_.emplace_back(ptr, [](char* parr){
            delete[] parr;
          }); // TODO : BUG pointer to array
          for (auto c = 0u; c != num_cols_; ++c)
          {
            auto csize = csizes_[c];
            if (ctypes_[c] < 7)
              for (auto i = 0u; i < csize / 2; ++i)
                std::swap(ptr[i], ptr[csize - i - 1]);
            else
              throw std::runtime_error("BUG: (w)string w/ big endian format");
            ptr += csize;
          }
        }
    }
  }
  bool serialize( 
      std::ostream& os
    , const std::function<bool(uint32_t&&)>& wfep 
  ) const
  {
    // TODO last dataset shall be 1 byte issue 
    // std::cout << (uint32_t)os.tellp() << std::endl;
    if( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write(os, name_))
        return false;
    if( !FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::write(os, nvts_.size() ))
        return false;
    for ( auto&& nvt : nvts_ )
    {
        if( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write( 
            os, std::get<0>( nvt ) 
        ) ) return false;
        if( !FileIO<std::string, GENERIC_CEL_ENDIAN_TYPE>::write( 
            os, std::get<1>( nvt ) 
        ) ) return false;
        if( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write( 
            os, std::get<2>( nvt ) 
        ) ) return false;
    }
    if( !FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::write( os, num_cols_ ) )
        return false;
    for (auto c = 0u; c != num_cols_; ++c)
    {
        if( !FileIO<std::wstring  , GENERIC_CEL_ENDIAN_TYPE>::write(os, cnames_.at(c))) return false;
        if( !FileIO<int8_t        , GENERIC_CEL_ENDIAN_TYPE>::write(os, ctypes_.at(c))) return false;
        if( !FileIO<int32_t       , GENERIC_CEL_ENDIAN_TYPE>::write(os, csizes_.at(c))) return false;
    }

    auto size = std::accumulate(csizes_.begin(), csizes_.end(), 0u);
    if ( !FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::write(os, num_rows_) ) 
        return false;
    if ( size != 0 )
    {
        // if ( !wfep( (uint32_t)os.tellp() ) ) return false;
        auto ptr = new char[size];
        for (auto r = 0u; r != num_rows_; ++r)
        {
            auto sptr = data_.at(r).get();
            // auto ptr = new char[size];
            // std::cout << ptr << std::endl;
            std::copy(sptr, sptr + size, ptr );
            // std::copy(sptr, sptr, ptr );
            auto tptr = ptr;
            for (auto c = 0u; c != num_cols_; ++c)
            {
              auto csize = csizes_[c];
              if (ctypes_[c] < 7) // 8bit
                for (auto i = 0u; i < csize / 2; ++i)
                  std::swap(ptr[i], ptr[csize - i - 1]);
              else // 16bit
                throw std::runtime_error("BUG: (w)string w/ big endian format");
              ptr += csize;
            }
            ptr = tptr;
            if( !os.write(ptr, size ) ) return false;
        }
        delete[] ptr;
    }
    return true;
  }
  void print(std::ostream& os)
  {
    os << "+ DataSet name: " << this->name_ << '\n';
    for (auto& nvt: this->nvts_)
      print_name_value_types(os, nvt);
    os << '\n';
    for (auto& cname: this->cnames_)
      os << std::setw(20) << cname;
    os << '\n';
    for (auto& ctype: this->ctypes_)
      os << std::setw(20) << static_cast<int16_t>(ctype);
    os << '\n' << '\n';
    for (uint32_t r = 0; r != num_rows_; ++r)
    {
      for (uint32_t c = 0; c != num_cols_; ++c)
      {
        os << std::setw(20);
        print_value(os, r, c);
      }
      os << '\n';
    }
    os << '\n';
  }

  template <class T>
  std::vector<T> get_vector(const uint32_t c) const 
  {
    std::vector<T> res;
    for (auto& tuple: data_)
    {
      char* ptr = tuple.get();
      for (auto i = 0u; i < c; ++i)
        ptr += csizes_[i];
      res.emplace_back(*(T*)ptr);
    }
    return res;
  }
  template <class T>
  std::vector<T> get_vector(const std::string& name) const 
  {
    std::vector<T> res;
    for (auto& tuple: data_)
    {
      char* ptr = tuple.get();
      for (auto c = 0u; c < num_cols_; ++c)
      {
        if (cnames_[c] != name)
          ptr += csizes_[c];
        else
          res.emplace_back(*(T*)ptr);
      }
    }
    return res;
  }

  void print_value(std::ostream& os, uint32_t r, uint32_t c)
  {
    char* ptr = data_[r].get();
    for (uint32_t i = 0; i < c; ++i)
      ptr += csizes_[i];

    switch (this->ctypes_[c])
    {
      case 0 : os << static_cast<int16_t>(*(int8_t*)ptr);   break;
      case 1 : os << static_cast<uint16_t>(*(uint8_t*)ptr); break;
      case 2 : os << *reinterpret_cast<int16_t*>(ptr);      break;
      case 3 : os << *reinterpret_cast<uint16_t*>(ptr);     break;
      case 4 : os << *reinterpret_cast<int32_t*>(ptr);      break;
      case 5 : os << *reinterpret_cast<uint32_t*>(ptr);     break;
      case 6 : os << *reinterpret_cast<float*>(ptr);        break;
      default: os << "(Unknown type)";
    }
  }
};

class GenericDataGroup
{
 public:
  std::string name_;
  std::vector<GenericDataSet> sets_;

 public:
  void parse(std::istream& is)
  {
    // static int xxxcounter = 0;
    // static int xxxcounter2 = 0;
    // if ( xxxload2 ) xxxcounter2 ++;
    // else xxxcounter ++;

    

    uint32_t epos, fpos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    sets_.resize(FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is));
    name_ = FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    for (auto& set: sets_)
    {
      is.seekg(fpos, std::ios::beg);
      epos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
      fpos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
      // std::cout << "ds parse start : " << (uint32_t)is.tellg() << std::endl;
      set.parse(is);
    }
  }
  bool serialize( std::ostream& os ) const
  {
      // static int xxxcounter = 0;
      // static int xxxcounter2 = 0;
      // if ( xxxload2 ) xxxcounter2 ++;
      // else xxxcounter ++;

      auto ffpos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::future_write( os );
      if ( !ffpos ) return false;
      if( !FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::write( os, (int32_t)sets_.size())) return false;
      if( !FileIO<std::wstring, GENERIC_CEL_ENDIAN_TYPE>::write( os, name_ )) return false;
      for ( auto& s : sets_ )
      {
          if( !ffpos( (uint32_t)os.tellp() ) ) return false;

          auto wfep = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::future_write( os );
          if( !wfep ) return false;

          ffpos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::future_write( os );
          if ( !ffpos ) return false;

          // std::cout << "ds serialize start : " << (uint32_t)os.tellp() << std::endl;
          s.serialize( os, wfep );
      }
      return !os.fail();
  }
  void print(std::ostream& os)
  {
    os << "DataGroup name: " << name_ << '\n';
    for (auto& set: sets_)
      set.print(os);
  }
};

// class CELData
// {
//  public:
//   virtual ~CELData(void) { }
//   // virtual bool read(std::istream& is) = 0;
//   virtual void print_all(std::ostream& os) = 0;
//   virtual void print_schema(std::ostream& os) { };
//   virtual std::vector<float> extract_intensities(uint32_t) = 0;
//   virtual std::vector<std::vector<float>> extract_intensities(void) = 0;
// };

template <uint8_t MAGIC, uint8_t VERSION>
class CELDataImpl;

template <>
class CELDataImpl<59,1>
    : public cpt::format::chip_sample::IData
{
 private:
  GenericDataHeader header_;
  std::vector<GenericDataGroup> groups_;

 public:
  bool read(std::istream& is)
  {
    auto gsize = FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is);
    groups_.resize(gsize);
    auto fpos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is); // 1st group
    header_.read(is);
    for (auto& group: groups_)
    {
      is.seekg(fpos, std::ios::beg);
      fpos = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::read(is); // next group
      group.parse(is);
    }
    return true;
  }
  bool write( std::ostream& os ) const 
  {
    auto gsize = groups_.size();
    if( !FileIO<int32_t, GENERIC_CEL_ENDIAN_TYPE>::write(os, groups_.size() ) ) return false;
    auto wfgp ( FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::future_write( os ) );
    if ( !wfgp ) return false;
    header_.write(os);
    for (auto& group: groups_)
    {
      wfgp( (uint32_t)os.tellp() );
      wfgp = FileIO<uint32_t, GENERIC_CEL_ENDIAN_TYPE>::future_write ( os );
      group.serialize( os ); 
    }
    wfgp( 0 );
    return (bool)os.flush();
    // return true;
  }
  virtual std::vector<float> extract_intensities(uint32_t i) const override
  {
    auto&& group = groups_[i];
    return group.sets_[0].get_vector<float>(0);
  }
  virtual std::vector<std::vector<float>> extract_intensities(void) const override
  {
    std::vector<std::vector<float>> res;
    for (auto& group: groups_)
    {
      for (auto& set: group.sets_)
        if (set.name_ == "Intensity")
          res.emplace_back(set.get_vector<float>(0));
    }
    return res;
  }

  virtual void print_all(std::ostream& os, int il = 0) override 
  {
    header_.print(os);
    for (auto& group: groups_)
      group.print(os);
  }

  virtual void print_schema(std::ostream& os, int il = 0) override
  {
    for (auto& group: groups_)
    {
      os << group.name_ << '\n';
      for (auto& set: group.sets_)
      {
        for (auto& tuple: set.nvts_)
          os << "  o " << std::get<0>(tuple) << '\n';  
        os << "+ " << set.name_ << '\n';
        for (auto& cname: set.cnames_)
          os << "  - " << cname << '\n';
      }
    }
  }
  GETTER(GenericDataHeader&, header(), { return header_; } );
  GETTER(std::vector<GenericDataGroup>&, groups(), { return groups_; } );
};

class CELFile
{
    using CELData = cpt::format::chip_sample::IData;
 public:
  static std::shared_ptr<CELData> load(const std::string& fname)
  {
    std::shared_ptr<CELData> celdata;
    std::ifstream fs(fname, std::ios::binary);
    if (!fs)
    {
      cpt::verbose0 << "Failed to open " << fname << std::endl;
    }
    else if (check_magic_version<59, 1, GENERIC_CEL_ENDIAN_TYPE>(fs))
    {
      // celdata = std::make_shared<CELDataImpl<59, 1>>();
      auto _celdata = new CELDataImpl<59, 1>();
      _celdata->read(fs);
      celdata.reset( _celdata );
    }
    else if (check_magic_version<64, 4, LittleEndian>(fs))
    {
      cpt::msg << "This is versionn 4 format" << std::endl;
    }
    else // Unsupported file format
    {
      cpt::msg << "Unsupported file format" << std::endl;
    }
    fs.clear();
    fs.close();
    return celdata;
  }

  template<uint8_t m , uint8_t v>
  static bool save ( const std::string& fname, const CELDataImpl<m, v>& cel_data )
  {
      std::ofstream fs ( fname, std::ios::binary );
      if ( !fs )
      {
          cpt::verbose0 << "Failed to open " << fname << std::endl;
          return false;
      }
      else
      {
          if ( !FileIO<uint8_t, get_endian(m, v)>::write(fs, m) ) return false;
          if ( !FileIO<uint8_t, get_endian(m, v)>::write(fs, v) ) return false;
          return cel_data.write(fs);
      }
  }

 private:
  template <uint8_t MAGIC, uint8_t VERSION, EndianType ENDIAN>
  static bool check_magic_version(std::istream& is)
  {
    auto magic   = FileIO<uint8_t, ENDIAN>::read(is);
    auto version = FileIO<uint8_t, ENDIAN>::read(is);
    if (magic == MAGIC and version == VERSION)
      return true;
    is.seekg(std::ios::beg);
    return false;
  }

};
}}
