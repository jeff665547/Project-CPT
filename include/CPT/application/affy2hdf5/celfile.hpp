#pragma once
#include "fileio.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <memory>
#include <vector>
#include <numeric>
// extern "C"
// {
  #include <hdf5.h>
//   #include <hdf5_hl.h>
// }
#include <Nucleona/format/hdf5.hpp>
namespace affy {

  namespace h5 = nucleona::format::hdf5;

  template <uint8_t MAGIC>
  class CelData;

  class GenericDataHeader
  {
      friend class CelData<59>;

    public:
      using NameValueType = std::tuple<std::string, std::string, std::string>;

    private:
      std::string identifier_;
      std::string guid_;
      std::string date_;
      std::string locale_;
      std::vector<NameValueType> nvts_;
      std::vector<GenericDataHeader> children_;

    public:
      void read(std::istream& is)
      {
          identifier_ = FileIO<std::string,  BigEndian>::read(is);
          guid_       = FileIO<std::string,  BigEndian>::read(is);
          date_       = FileIO<std::wstring, BigEndian>::read(is);
          locale_     = FileIO<std::wstring, BigEndian>::read(is);

          auto num_params = FileIO<int32_t, BigEndian>::read(is);
          for (int32_t i = 0; i != num_params; ++i)
          {
              auto&& name  = FileIO<std::wstring, BigEndian>::read(is);
              auto&& value = FileIO<std::string,  BigEndian>::read(is);
              auto&& type  = FileIO<std::wstring, BigEndian>::read(is);
              nvts_.emplace_back(name, value, type);
          }

          auto num_headers = FileIO<int32_t, BigEndian>::read(is);
          children_.resize(num_headers);
          for (auto& child: children_)
              child.read(is);
      }
  };

  class GenericDataSet
  {
      friend class CelData<59>;

    public:
      using NameValueType = std::tuple<std::string, std::string, std::string>;
      using MemberType = std::tuple<std::string, int8_t, int32_t>;

    private:
      std::string name_;
      uint32_t num_cols_;
      uint32_t num_rows_;
      std::vector<NameValueType> nvts_;
      std::vector<MemberType> descs_;
      std::shared_ptr<char> data_;
  
    public:
      uint32_t tuple_size(void) const
      {
          return std::accumulate(descs_.begin(), descs_.end(), 0u,
                 [](const uint32_t& accu, const auto& desc)
                 { return accu + std::get<2>(desc); });
      }

      const auto& get_typelist(void) const
      {
          return descs_;
      }

      void read(std::istream& is)
      {
          name_ = FileIO<std::wstring, BigEndian>::read(is);
          std::cout << name_ << std::endl;

          auto num_params = FileIO<int32_t, BigEndian>::read(is);
          for (auto i = 0; i != num_params; ++i)
          {
              auto&& name  = FileIO<std::wstring, BigEndian>::read(is);
              auto&& value = FileIO<std::string,  BigEndian>::read(is);
              auto&& type  = FileIO<std::wstring, BigEndian>::read(is);
              nvts_.emplace_back(name, value, type);
          }
  
          num_cols_ = FileIO<uint32_t, BigEndian>::read(is);
          for (auto c = 0u; c != num_cols_; ++c)
          {
              auto&& cname = FileIO<std::wstring, BigEndian>::read(is);
              auto&& ctype = FileIO<int8_t, BigEndian>::read(is);
              auto&& csize = FileIO<int32_t, BigEndian>::read(is);
              descs_.emplace_back(cname, ctype, csize);
          }

          auto size = this->tuple_size();
          num_rows_ = FileIO<uint32_t, BigEndian>::read(is);

          if (size == 0)
              return;

          data_.reset(new char[size * num_rows_]); //TODO fix bug
          is.read(data_.get(), size * num_rows_);

          auto head = data_.get();
          for (auto r = 0u; r != num_rows_; ++r)
          {
              for (auto& desc: descs_)
              {
                  auto& ctype = std::get<1>(desc);
                  auto& csize = std::get<2>(desc);
                  if (ctype >= 7)
                      throw std::invalid_argument("TODO: text format");
                  auto last = csize >> 1;
                  for (auto i = 0u; i < last; ++i)
                      std::swap(head[i], head[csize - i - 1]);
                  head += csize;
              }
          }
      }
  };

  class GenericDataGroup
  {
      friend class CelData<59>;

    private:
      std::string name_;
      std::vector<GenericDataSet> sets_;
  
    public:
      void read(std::istream& is)
      {
          uint32_t epos, fpos = FileIO<uint32_t, BigEndian>::read(is);
          sets_.resize(FileIO<int32_t, BigEndian>::read(is));
          name_ = FileIO<std::wstring, BigEndian>::read(is);
          // std::cout << name_ << std::endl;
          for (auto& set: sets_)
          {
              is.seekg(fpos, std::ios::beg);
              epos = FileIO<uint32_t, BigEndian>::read(is);
              fpos = FileIO<uint32_t, BigEndian>::read(is);
              set.read(is);
          }
      }
  };

  template <>
  class CelData<59>
  //    : public CelFile
  {
    private:
      uint8_t version_;
      GenericDataHeader header_;
      std::vector<GenericDataGroup> groups_;
  
    public:
      template<class LEGACY_GROUPS, class ARRAY>
      void group_mapping( LEGACY_GROUPS&& lg, ARRAY&& ar )
      {
          int counter(0);
          hsize_t lg_subg_num(0);
          std::string prefix ("channel-");
          const static int max_group_name ( 100 );
          char lgs_group_name[max_group_name];
          H5Gget_num_objs(lg.id.get(), &lg_subg_num);
          std::cout << "number in legacy groups : " << lg_subg_num << std::endl;
          for ( counter = 0; counter < lg_subg_num; counter ++ )
          {
              std::string group_name ( prefix + std::to_string(counter) );
              H5Gget_objname_by_idx( lg.id.get(), (hsize_t)counter, lgs_group_name, (size_t)max_group_name);
              if ( std::string(lgs_group_name) == "QC") continue;

              std::cout << "process : " << lgs_group_name << std::endl;
              auto lchannel(lg.open_group(lgs_group_name));
              h5::Group channel( !ar.exist( group_name ) 
                  ? ar.create_group( group_name )
                  : ar.open_group ( group_name )
              );
              auto intensity_g    ( lchannel.open_group("Intensity") );
              auto intensity_d    ( intensity_g.open_dataset("Intensity") );
              if( channel.exist( "intensity" ) ) channel.unlink("intensity");
              intensity_d.hardlink_to( channel, "intensity" );

              auto mask_g         ( lchannel.open_group("Mask") );
              auto mask_d         ( mask_g.open_dataset("Mask") );
              if( channel.exist( "mask" ) ) channel.unlink("mask");
              mask_d.hardlink_to( channel, "mask" );

              auto outlier_g      ( lchannel.open_group("Outlier") );
              auto outlier_d      ( outlier_g.open_dataset("Outlier") );
              if( channel.exist( "outlier" ) ) channel.unlink("outlier");
              outlier_d.hardlink_to ( channel, "outlier" );

              auto pixel_g        ( lchannel.open_group("Pixel") );
              auto pixel_d        ( pixel_g.open_dataset("Pixel") );
              if( channel.exist( "pixel" ) ) channel.unlink("pixel");
              pixel_d.hardlink_to ( channel, "pixel" );

              auto stddev_g       ( lchannel.open_group("StdDev") );
              auto stddev_d       ( stddev_g.open_dataset("StdDev") );
              if( channel.exist( "stddev" ) ) channel.unlink("stddev");
              stddev_d.hardlink_to ( channel, "stddev" );
          }
      }
      bool read(const std::string& fname)
      {
          std::ifstream is(fname, std::ios::binary);
          if (!is)
          {
              std::cerr << "Failed to open: " + fname << '\n';
              exit(1);
              // return false;
          }
          uint8_t magic = FileIO<uint8_t, BigEndian>::read(is);
          if (magic != 59)
          {
              std::cerr << "Failed to read data in format [magic = 59]\n";
              is.close();
              exit(1);
              // return false;
          }
          version_ = FileIO<uint8_t, BigEndian>::read(is);
          groups_.resize(FileIO<int32_t, BigEndian>::read(is));
          auto fpos = FileIO<uint32_t, BigEndian>::read(is);

          header_.read(is);
          for (auto& group: groups_)
          {
              is.seekg(fpos, std::ios::beg);
              fpos = FileIO<uint32_t, BigEndian>::read(is);
              group.read(is);
          }

          is.close();
          return true;
      }
  
      void export_hdf5(const std::string& fname)
      {
          // open an HDF5 file
          h5::File file(fname, H5F_ACC_RDWR);
          // 
          // if (!file) // TODO
          // {
          //     throw std::runtime_error("Failed to open file " + fname);
          // }

          // TODO get root
          h5::Group root = file.open_group("/");
          // if( root.exists("legacy") )
          // {
          //     std::cerr << "/legacy exists. do nothing" << '\n';
          //     return;
          // {

          // TODO create a group named "legacy"
          h5::Group legacy = root.open_group("legacy");

          h5::Attribute magic = legacy.create_attribute_v("magic", (uint8_t)59);

          // TODO insert an attribute named "version" with a scalar = 1
          h5::Attribute version = legacy.create_attribute_v("version", (uint8_t)version_);

          // TODO check if is a initialized file
          {
            h5::Attribute status = root.open_attribute("status");
            auto data = status.read_vec<std::string>();
            if( data[0] == "initialized") 
            {
                std::cout << "the hdf5 file is initialized, please input a pure template file\n";
                exit(1);
            }
            else status.write(std::vector<std::string>({"initialized"}));
          }
          
          // TODO test to create an attribute with an uint8_t indirectly
          // uint8_t value1 = 5;
          // h5::Attribute test1 = legacy.create_attribute("test1", (uint8_t)5);

          // TODO test to create an attribute with an std::string indirectly 
          // std::string value2 = "qq";
          // hdf5::Attribute test2 = legacy.create_attribute("test2", value2);
          // test2.write(value2);

          // create a group named "header" under legacy
          h5::Group header = legacy.create_group("header");
          h5_add_header(header, header_);
          
          // create a group named "groups" under legacy, and then
          // insert all generic data groups
          h5::Group groups = legacy.create_group("groups");
          for (auto& group: groups_)
          {
              std::cout << group.name_ << std::endl;
              h5_add_datagroup(groups, group);
          }
          // TODO fromat match and field transfer
          auto array = root.open_group("array");
          group_mapping(groups, array);
      }
  
    private:
      static std::string resolve_mimetype(int8_t type)
      {
          switch (type)
          {
              case 0: return "int8_t";
              case 1: return "uint8_t";
              case 2: return "int16_t";
              case 3: return "uint16_t";
              case 4: return "int32_t";
              case 5: return "uint32_t";
              case 6: return "float";
              case 7: return "std::string";
              default: throw std::invalid_argument("Unsupported mimetype conversion");
          }
      }
      static void h5_add_datagroup(h5::Group& root, GenericDataGroup& group)
      {
          auto h5g = root.create_group(group.name_);
          // auto colattr = root.open_attribute("feature-columns");
          // auto rowattr = root.open_attribute("feature-rows");
          for (auto& set: group.sets_)
              h5_add_dataset(h5g, set);
      }
      static void h5_add_dataset(
          h5::Group&        root
        , GenericDataSet&   set
      )
      {
          // TODO fix cols and rows attr
          h5::Group group = root.create_group(set.name_);
          
          // add name-value pairs
          h5_add_namevalues(group, set.nvts_);

          // finish this function if zero fields are detected
          if (set.num_cols_ == 0)
              return;
          
          // prepare comptype
          auto& mts = set.get_typelist();
          auto comp_size = std::accumulate( mts.begin(), mts.end(), 0, [](const auto& a, const auto& b){return a + std::get<2>(b);});
          h5::type::Compound comptype(comp_size); // TODO for creating compound type dynamically
          auto mtypes = set.get_typelist();
          auto field_offset = 0;
          for (auto& mtype: set.get_typelist())
          {
              auto&& field_name = std::get<0>(mtype);                   // std::string
              auto&& field_type = resolve_mimetype(std::get<1>(mtype)); // int8_t -> string
              auto&& field_size = std::get<2>(mtype);                   // int32_t
              comptype.insert_str(field_name, field_offset, field_type);
              field_offset += field_size;
          }

          // prepare dimensions
          std::vector<hsize_t> dims { set.num_rows_ };
          
          // TODO create dataset
          h5::Dataset dataset = group.create_dataset_dt(
              set.name_, set.data_.get(), comptype, dims
          );

          // auto compound_size = set.tuple_size();
          // hsize_t dims = set.num_rows_;
          // auto dataspace = ::H5Screate_simple(1, &dims, nullptr);
          // auto datatype = ::H5Tcreate(::H5T_COMPOUND, compound_size);
          // uint32_t offset = 0u;
          // for (auto& desc: set.descs_)
          // {
          //     auto& cname = std::get<0>(desc);
          //     auto& ctype = std::get<1>(desc);
          //     auto& csize = std::get<2>(desc);
          //     hid_t field_id;
          //     switch (ctype)
          //     {
          //         case  0: field_id = ::H5T_STD_I8LE_g;   break;
          //         case  1: field_id = ::H5T_STD_U8LE_g;   break;
          //         case  2: field_id = ::H5T_STD_I16LE_g;  break;
          //         case  3: field_id = ::H5T_STD_U16LE_g;  break;
          //         case  4: field_id = ::H5T_STD_I32LE_g;  break;
          //         case  5: field_id = ::H5T_STD_U32LE_g;  break;
          //         case  6: field_id = ::H5T_IEEE_F32LE_g; break;
          //     }
          //     if (ctype < 7)
          //     {
          //         ::H5Tinsert(datatype, cname.c_str(), offset, field_id);
          //         offset += csize;
          //     }
          // }

          // auto dataset = ::H5Dcreate(
          //                    group_id
          //                  , set.name_.c_str()
          //                  , datatype
          //                  , dataspace
          //                  , H5P_DEFAULT
          //                  , H5P_DEFAULT
          //                  , H5P_DEFAULT
          //                );
          // ::H5Dwrite(
          //     dataset
          //   , datatype
          //   , dataspace
          //   , H5S_ALL
          //   , H5P_DEFAULT
          //   , set.data_.get()
          // );

          // H5Dclose(dataset);
          // H5Tclose(datatype);
          // H5Sclose(dataspace);
          // H5Gclose(group_id);
      }
      static void h5_add_header(h5::Group& root, GenericDataHeader& parent)
      {
          auto group = root.create_group(parent.identifier_);
          group.create_attribute_v("guid", parent.guid_);
          group.create_attribute_v("date", parent.date_);
          group.create_attribute_v("locale", parent.locale_);
          h5_add_namevalues(group, parent.nvts_);
          for (auto& child: parent.children_)
              h5_add_header(group, child);
      }
      static void h5_add_namevalues(
          h5::Group& group
        , std::vector<GenericDataHeader::NameValueType>& nvts
      ) 
      {
          for (auto& nvt: nvts)
          {
              auto&& name = std::get<0>(nvt);
              auto&& temp = std::get<1>(nvt);
              auto&& type = std::get<2>(nvt);

              if (type == "text/plain" or type == "text/ascii")
              {
                  std::string s = (type == "text/plain")
                                  ? mimetype2<std::string>(temp)
                                  : temp;
                  s.erase(std::remove_if(s.begin(), s.end(),
                    [](char c){ return !std::isprint(c); }), s.end());

                  if(!H5Aexists(group.id.get(), name.c_str()))
                    group.create_attribute_v(name, s); // TODO: a better check attr exist function
              }
              else if (type == "text/x-calvin-integer-8")
                  group.create_attribute_v(name, mimetype2<int8_t>(temp));
              else if (type == "text/x-calvin-unsigned-integer-8")
                  group.create_attribute_v(name, mimetype2<uint8_t>(temp));
              else if (type == "text/x-calvin-integer-16")
                  group.create_attribute_v(name, mimetype2<int16_t>(temp));
              else if (type == "text/x-calvin-unsigned-integer-16")
                  group.create_attribute_v(name, mimetype2<uint16_t>(temp));
              else if (type == "text/x-calvin-integer-32")
                  group.create_attribute_v(name, mimetype2<int32_t>(temp));
              else if (type == "text/x-calvin-unsigned-integer-32")
                  group.create_attribute_v(name, mimetype2<uint32_t>(temp));
              else if (type == "text/x-calvin-float")
                  group.create_attribute_v(name, mimetype2<float>(temp));
              else // unknown mimetype
                  group.create_attribute_v(name, "(unknown type: " + type + ")");
          }
      }
  };

  // class CelFile
  // {
  //   public:
  //     static std::shared_ptr<CelData> load(const std::string& fname)
  //     {
  //       std::shared_ptr<class CelData> celdata;
  //       
  //       celdata = std::make_shared<CelData<59>>();
  //       bool success;

  //       std::ifstream fs(fname, std::ios::binary);
  //       if (!fs)
  //       {
  //           std::cerr << "Failed to open " << fname << '\n';
  //       }
  //       else if (check_magic_version<59, 1, BigEndian>(fs))
  //       {
  //           celdata = std::make_shared<CELDataImpl<59, 1>>();
  //         celdata->read(fs);
  //       }
  //       else if (check_magic_version<64, 4, LittleEndian>(fs))
  //       {
  //         std::cerr << "This is versionn 4 format\n";
  //       }
  //       else // Unsupported file format
  //       {
  //         std::cerr << "Unsupported file format\n";
  //       }
  //       fs.clear();
  //       fs.close();
  //       return celdata;
  //     }
  // 
  //  private:
  //   template <uint8_t MAGIC, uint8_t VERSION, EndianType ENDIAN>
  //   static bool check_magic_version(std::istream& is)
  //   {
  //     auto magic   = FileIO<uint8_t, ENDIAN>::read(is);
  //     auto version = FileIO<uint8_t, ENDIAN>::read(is);
  //     if (magic == MAGIC and version == VERSION)
  //       return true;
  //     is.seekg(std::ios::beg);
  //     return false;
  //   }
  // };

} // end of namespace affy

