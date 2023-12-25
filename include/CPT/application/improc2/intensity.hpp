#pragma once
#include <CPT/format/hdf5.hpp>

namespace improc {

struct Intensity
{
    using ThisType = Intensity;

    float mean;
    float std;
    int16_t pixels;

    Intensity(
        float mean
      , float std
      , int16_t pixels
    )
      : mean(mean)
      , std(std)
      , pixels(pixels)
    {}

    Intensity(void)
    {}

    static auto hdf5_type(void)
    {
        namespace hdf5 = cpt::format::hdf5;
        auto type = hdf5::create_raw_compound_type<ThisType>();
        type.insert("mean", HOFFSET(ThisType, mean), "float");
        type.insert("std", HOFFSET(ThisType, std), "float");
        type.insert("pixels", HOFFSET(ThisType, pixels), "int16_t");
        return type;
    }
    static auto hdf5_parse(ThisType& buffer)
    {
        namespace hdf5 = cpt::format::hdf5;
        hdf5::parse_buf_basic_type(buffer.mean);
        hdf5::parse_buf_basic_type(buffer.std);
        hdf5::parse_buf_basic_type(buffer.pixels);
    }
};

} // namespace improc
