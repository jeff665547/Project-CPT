#pragma once
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <memory>
#include <typeinfo>
#include <typeindex>
// extern "C"
// {
  #include <hdf5.h>
  #include <hdf5_hl.h>
// }

namespace cpt  {
namespace hdf5 {

// Smart HDF5 ID management
//_______________________________________________

class H5Id
{
  private:
    struct ID
    {
        hid_t id;

        ID(hid_t n = 0)
          : id(n)
        {
            std::cerr << "create id = " << id << '\n';
        }
        ~ID(void)
        {
            std::cerr << "delete id = " << id << '\n';

            if (id == 0)
              return;
    
            auto type = H5Iget_type(id);
            if (type == H5I_FILE)
                H5Fclose(id);
            else if (type == H5I_GROUP)
                H5Gclose(id);
            else if (type == H5I_DATASET)
                H5Dclose(id);
            else if (type == H5I_DATATYPE)
                H5Tclose(id);
            else if (type == H5I_DATASPACE)
                H5Sclose(id);
            else if (type == H5I_ATTR)
                H5Aclose(id);
            else if (type == H5I_BADID)
                std::cerr << "Bad ID\n";
            else //
                std::cerr << "Unrecognized ID\n";
        }
    };

  public:
    H5Id(void)
      : ptr_(nullptr)
    {}
    H5Id(hid_t id)
      : ptr_(new ID(id))
    {}
    H5Id& operator=(const hid_t& id)
    {
        (id != 0)?
        ptr_.reset(new ID(id)):
        ptr_.reset();
        return *this;
    }
    hid_t id(void) const
    {
        return ptr_->id;
    }

  private:
    std::shared_ptr<ID> ptr_;
};


// Data type convertion between HDF5 and C++
//__________________________________________________

#define INT8   int8_t
#define INT16  int16_t 
#define INT32  int32_t
#define INT64  int64_t 
#define UINT8  uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define FLOAT  float
#define DOUBLE double
#define STRING std::string

std::type_index to_cpptype(hid_t id)
{
    #define IMPL(CPPTYPE,HDFTYPE)\
    else if (H5Tequal(id,HDFTYPE))\
        return typeid(CPPTYPE);

    if (H5Tget_class(id) == H5T_STRING)
    {
        return typeid(std::string);
    }
    IMPL( INT8  , H5T_STD_I8LE_g   )
    IMPL( INT16 , H5T_STD_I16LE_g  )
    IMPL( INT32 , H5T_STD_I32LE_g  )
    IMPL( INT64 , H5T_STD_I64LE_g  )
    IMPL( UINT8 , H5T_STD_U8LE_g   )
    IMPL( UINT16, H5T_STD_U16LE_g  )
    IMPL( UINT32, H5T_STD_U32LE_g  )
    IMPL( UINT64, H5T_STD_U64LE_g  )
    IMPL( FLOAT , H5T_IEEE_F32LE_g )
    IMPL( DOUBLE, H5T_IEEE_F64LE_g )
    else
    {
        throw std::invalid_argument("Unrecognized ID");
    }
    #undef IMPL
}

hid_t to_h5type(std::type_index type)
{
    #define IMPL(CPPTYPE,HDFTYPE)\
    else if (type == typeid(CPPTYPE))\
        return HDFTYPE;

    if (type == typeid(STRING))
    {
        static hid_t strtype = H5Tcopy(H5T_C_S1);
        H5Tset_size(strtype, H5T_VARIABLE);
        return strtype;
    }
    IMPL( INT8  , H5T_STD_I8LE   )
    IMPL( INT16 , H5T_STD_I16LE  )
    IMPL( INT32 , H5T_STD_I32LE  )
    IMPL( INT64 , H5T_STD_I64LE  )
    IMPL( UINT8 , H5T_STD_U8LE   )
    IMPL( UINT16, H5T_STD_U16LE  )
    IMPL( UINT32, H5T_STD_U32LE  )
    IMPL( UINT64, H5T_STD_U64LE  )
    IMPL( FLOAT , H5T_IEEE_F32LE )
    IMPL( DOUBLE, H5T_IEEE_F64LE )
    else
        throw std::invalid_argument("Unrecognized C++ type");

    #undef IMPL
}

std::type_index str2cpptype(const std::string& name)
{
    #define IMPL(CPPTYPE) \
    else if (name == #CPPTYPE) \
        return typeid(CPPTYPE);

    if (false);
    IMPL( INT8   )
    IMPL( INT16  )
    IMPL( INT32  )
    IMPL( INT64  )
    IMPL( UINT8  )
    IMPL( UINT16 )
    IMPL( UINT32 )
    IMPL( UINT64 )
    IMPL( FLOAT  )
    IMPL( DOUBLE )
    IMPL( STRING )
    else
        throw std::invalid_argument("Invalid type string");
}


// C++ wrapper for HDF5
//__________________________________________________

// HDF file
H5Id open_file(const std::string& name, unsigned flag)
{
    if (flag == H5F_ACC_TRUNC or flag == H5F_ACC_EXCL)
        return H5Fcreate(name.c_str(), flag, H5P_DEFAULT, H5P_DEFAULT);
    else if (flag == H5F_ACC_RDONLY or flag == H5F_ACC_RDWR)
        return H5Fopen(name.c_str(), flag, H5P_DEFAULT);
    else
        throw std::invalid_argument("Invalid H5F flag");
}

// HDF group
H5Id open_group(const H5Id& loc, const std::string& name)
{
    if (!H5Iis_valid(loc.id()))
        throw std::invalid_argument("Invalid file id");
    if (!H5Lexists(loc.id(), name.c_str(), H5P_DEFAULT))
        throw std::invalid_argument("No such path for group " + name);
    return H5Gopen(loc.id(), name.c_str(), H5P_DEFAULT);
}
H5Id create_group(const H5Id& loc, const std::string& name)
{
    if (H5Lexists(loc.id(), name.c_str(), H5P_DEFAULT))
    {
        std::cerr << "Group " << name << " exists. Open it\n";
        return H5Gopen(loc.id(), name.c_str(), H5P_DEFAULT);
    }
    return H5Gcreate(loc.id(), name.c_str(), 
                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

// HDF attribute
H5Id open_attribute(const H5Id& loc, const std::string& name)
{
    if (!H5Iis_valid(loc.id()))
        throw std::invalid_argument("Invalid file id");
    if (!H5Aexists(loc.id(), name.c_str()))
        throw std::invalid_argument("No such attribute " + name);
    return H5Aopen(loc.id(), name.c_str(), H5P_DEFAULT);
}
H5Id create_attribute(const H5Id& loc,
                      const std::string& name,
                      const std::vector<hsize_t>& dims,
                      const std::type_index& cpptype)
{
    if (H5Aexists(loc.id(), name.c_str()))
    {
        std::cerr << "Attribute " << name << " exists. Open it\n";
        return H5Aopen(loc.id(), name.c_str(), H5P_DEFAULT);
    }
    H5Id dataspace = H5Screate_simple(dims.size(), dims.data(), nullptr);
    H5Id datatype  = H5Tcopy(to_h5type(cpptype));
    H5Id attribute = H5Acreate(loc.id(), name.c_str(),
                               datatype.id(), dataspace.id(),
                               H5P_DEFAULT, H5P_DEFAULT);
    return attribute;
}
herr_t write_attribute(const H5Id& attribute, const void* data)
{
    if (!H5Iis_valid(attribute.id()))
        throw std::invalid_argument("Invalid attribute id");
    H5Id dataspace = H5Aget_space(attribute.id());
    H5Id datatype  = H5Aget_type(attribute.id());
    return H5Awrite(attribute.id(), datatype.id(), data);
}

// HDF dataset
H5Id open_dataset(const H5Id& loc, const std::string& name)
{
    if (!H5Iis_valid(loc.id()))
        throw std::invalid_argument("Invalid file id");
    if (!H5Lexists(loc.id(), name.c_str(), H5P_DEFAULT))
        throw std::invalid_argument("No such path for datatset " + name);
    return H5Dopen(loc.id(), name.c_str(), H5P_DEFAULT);
}
H5Id create_dataset(const H5Id& loc, const std::string& name, const )
{
    if (H5Lexists(loc.id(), name.c_str(), H5P_DEFAULT))
    {
        std::cerr << "Dataset " << name << " exists. Open it!\n";
        return H5Dopen(loc.id(), name.c_str(), H5P_DEFAULT);
    }
    auto& dims  = layout.dims;
    auto& types = layout.types;
    H5Id dataspace = H5Screate_simple(dims.size(), dims.data(), nullptr);
    H5Id datatype  = H5Tcopy(to_h5type(types.front()));
    H5Id dataset   = H5Dcreate(loc.id(), name.c_str(),
                               datatype.id(), dataspace.id(),
                               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    return dataset;
}
H5Id create_dataset_atomtype(const H5Id& loc, 

herr_t write_dataset(const H5Id& dataset, const void* data)
{
    if (!H5Iis_valid(dataset.id()))
        throw std::invalid_argument("Invalid datatset id");
    H5Id dataspace = H5Dget_space(dataset.id());
    H5Id datatype  = H5Dget_type(dataset.id());
    return H5Dwrite(dataset.id(), datatype.id(), dataspace.id(),
                    dataspace.id(), H5P_DEFAULT, data);
}


} // end of namespace hdf5
} // end of namespace cpt
