#pragma once
#include <iostream>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include <Nucleona/format/hdf5.hpp>
extern "C"
{
#include "hdf5.h"
#include "hdf5_hl.h"
}
namespace cpt{ namespace application{ namespace cenfile_builder{

namespace bpt = boost::property_tree;
namespace bfs = boost::filesystem;
namespace h5  = nucleona::format::hdf5;
size_t size_of(const std::string& type)
{
    #define IMPL(TYPE)\
    else if (type == #TYPE)\
        return sizeof(TYPE);
    IMPL_CPP_TYPES
        throw std::invalid_argument("Invalid typename: " + type);
    #undef IMPL
}

class HDF5Builder
{
  public:
    static void build(const bpt::ptree& parent, h5::Group& group)
    {
        for (auto& child: parent)
        {
            auto id = get_class_and_name(child.first);
            if (id.first == "G")
            {
                auto new_group = group.create_group(id.second);
                build(child.second, new_group);
            }
            else if (id.first == "A")
            {
                add_attribute(id.second, child.second, group);
            }
            else if (id.first == "D")
            {
                add_dataset(id.second, child.second, group);
            }
            else if (id.first == "I")
            {
                // add_image(id.second, child.second, group);
            }
            else throw std::invalid_argument("Invalid HDF5 class");
        }
    }
  
  private:
    static void add_attribute(const std::string& name, 
                              const bpt::ptree& node,
                              h5::Group& group)
    {
        auto type = node.get<std::string>("DATATYPE");

        #define IMPL(TYPE)\
        else if (type == #TYPE)\
        {\
            auto tmp ( node.get<TYPE>("DATA") ); \
            group.create_attribute_v( name, tmp ); \
        }
        IMPL_CPP_TYPES
            throw std::invalid_argument("Invalid typename: " + type);
        #undef IMPL

    //    hsize_t dims[1];
    //    dims[0] = node.get<hsize_t>("DATASPACE");
    //    h5::DataSpace dataspace(1, dims);
 
    //    auto type = node.get<std::string>("DATATYPE");
    //    if (type == "UINT")
    //    {
    //        auto datatype = h5::PredType::NATIVE_UINT32;
    //        auto data = node.get<uint32_t>("DATA");
    //        auto attribute = group.createAttribute(name, datatype, dataspace);
    //        attribute.write(datatype, &data);
    //    }
    //    else if (type == "FLOAT")
    //    {
    //        auto datatype = h5::PredType::NATIVE_FLOAT;
    //        auto data = node.get<float>("DATA");
    //        auto attribute = group.createAttribute(name, datatype, dataspace);
    //        attribute.write(datatype, &data);
    //    }
    //    else if (type == "STRING")
    //    {
    //        auto datatype = h5::StrType(h5::PredType::C_S1, 80);
    //        auto data = node.get<std::string>("DATA");
    //        auto attribute = group.createAttribute(name, datatype, dataspace);
    //        attribute.write(datatype, data);
    //    }
    //    else throw std::invalid_argument("Unknown DATATYPE");
    }

    static void add_dataset_atomtype(const std::string& name,
                                     const bpt::ptree& node,
                                     h5::Group& group)
    {
        // prepare dimensions
        std::vector<hsize_t> space;
        for (auto& item: node.get_child("DATASPACE"))
            space.emplace_back(item.second.get_value<hsize_t>());
        
        // prepare datatype
        auto type = node.get<std::string>("DATATYPE");

        #define IMPL(TYPE)\
        else if (type == #TYPE)\
        {\
            std::vector<TYPE> data;\
            for (auto& elem: node.get_child("DATA"))\
                data.emplace_back(elem.second.get_value<TYPE>());\
            auto dset = group.create_dataset<TYPE>(name, space);\
            dset.write(data);\
        }
        IMPL_CPP_TYPES
            throw std::invalid_argument("Invalid typename: " + type);
        #undef IMPL
    }

    static void add_dataset_comptype(const std::string& name,
                                     const bpt::ptree& node,
                                     h5::Group& group)
    {
        // load member names
        std::vector<std::string> names;
        for (auto& item: node.get_child("MEMBERS"))
            names.emplace_back(item.second.get_value<std::string>());

        // load type names
        std::vector<std::string> types;
        for (auto& item: node.get_child("DATATYPE"))
            types.emplace_back(item.second.get_value<std::string>());

        if (names.size() != types.size())
            throw std::runtime_error("# names != # types");

        // calc data sizes
        std::vector<size_t> sizes;
        for (auto& type: types)
            sizes.emplace_back(size_of(type));

        // load dimensions
        std::vector<hsize_t> space;
        for (auto& item: node.get_child("DATASPACE"))
            space.emplace_back(item.second.get_value<hsize_t>());
        
        // get ptree
        auto& table = node.get_child("DATA");

        // prepare constant
        auto num_rows = table.size();
        auto num_cols = names.size();
        auto tuple_size = std::accumulate(sizes.begin(), sizes.end(), 0);

        // prepare compound type
        h5::type::Compound comptype(tuple_size);
        auto offset = 0;
        for (size_t i = 0; i != num_cols; ++i)
        {
            comptype.insert_str(names[i], offset, types[i]);
            offset += sizes[i];
        }

        // prepare data
        std::vector<std::string> str_pool;
        std::shared_ptr<char> mem(new char [num_rows * tuple_size], []( auto const * p ) { delete[] p; } );
        char* ptr = mem.get();
        for (auto& row: table)
        {
            size_t index = 0;
            for (auto& col: row.second)
            {
                auto& type = types[index];
                #define IMPL(TYPE)\
                else if (type == #TYPE)\
                {\
                    auto value = col.second.get_value<TYPE>();\
                    std::memcpy(ptr, &value, Sizeof<TYPE>::get(value));\
                }
                if (false);
                IMPL(      int8_t )
                IMPL(     int16_t )
                IMPL(     int32_t )
                IMPL(     int64_t )
                IMPL(     uint8_t )
                IMPL(    uint16_t )
                IMPL(    uint32_t )
                IMPL(    uint64_t )
                IMPL(       float )
                IMPL(      double )
                else if ( type == "std::string" )
                {
                    auto value =  col.second.get_value<std::string>();
                    str_pool.push_back( value );
                    std::memcpy(ptr, &str_pool.back(), Sizeof<std::string>::get(value));
                }
                else
                    throw std::invalid_argument("Invalid typename: " + type);
                #undef IMPL
                ptr += sizes[index++];
            }
        }
        // export to HDF file
        group.create_dataset_dt(name, mem.get(), comptype, space );
        // auto dataset_id ( compound_dataset( group, comptype, space ) );
    }
    static void add_dataset(const std::string& name,
                            const bpt::ptree& node,
                            h5::Group& group)
    {
        if (node.count("MEMBERS") == 0)
            add_dataset_atomtype(name, node, group);
        else
            add_dataset_comptype(name, node, group);
 
      // auto type = node.get<std::string>("DATATYPE");
      // if (type == "UINT")
      // {
      //   auto datatype = h5::PredType::NATIVE_UINT32;
      //   std::vector<uint32_t> data;
      //   for (auto& elem: node.get_child("DATA"))
      //     data.emplace_back(elem.second.get_value<uint32_t>());
      //   auto dataset = group.createDataSet(name, datatype, dataspace);
      //   dataset.write(data.data(), datatype);
      // }
      // else if (type == "INT")
      // {
      //   auto datatype = h5::PredType::NATIVE_INT32;
      //   std::vector<int32_t> data;
      //   for (auto& elem: node.get_child("DATA"))
      //     data.emplace_back(elem.second.get_value<int32_t>());
      //   auto dataset = group.createDataSet(name, datatype, dataspace);
      //   dataset.write(data.data(), datatype);
      // }
      // else if (type == "FLOAT")
      // {
      //   auto datatype = h5::PredType::NATIVE_FLOAT;
      //   std::vector<float> data;
      //   for (auto& elem: node.get_child("DATA"))
      //     data.emplace_back(elem.second.get_value<float>());
      //   auto dataset = group.createDataSet(name, datatype, dataspace);
      //   dataset.write(data.data(), datatype);
      // }
    }
    static void add_image(const std::string& name,
                          const bpt::ptree& node,
                          h5::Group& group)
    {
       boost::optional<std::string> path;
       path = node.get_optional<std::string>("PATH");
       if (path and path.get() != "") 
       {
           if (!boost::filesystem::exists(path.get()))
           {
               std::cerr << "Image " << path.get() << " was not found\n";
           }
           cv::Mat image = cv::imread(path.get(), CV_LOAD_IMAGE_COLOR);
           cv::cvtColor(image, image, CV_BGR2RGB);
           H5IMmake_image_24bit (group.id.get(), name.c_str(), image.cols, image.rows, 
                                "INTERLACE_PIXEL", image.data);
       }
    }
    static std::pair<std::string, std::string> 
    get_class_and_name(const std::string& s)
    {
        size_t pos = s.find_first_of("|");
        auto cls = s.substr(0, pos);
        auto str = s.substr(pos +1);
        return std::make_pair(cls, str);
    }
};
}}}
