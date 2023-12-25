#pragma once
#include <CPT/format/chip_sample.hpp>
#include <CPT/logger.hpp>
#include <CPT/format/cen.hpp>
namespace cpt{ namespace format{ namespace converter{
namespace cf_       = cpt::format;
namespace cfcs_     = cpt::format::chip_sample;
class Array2Cen
{
  public:
    auto operator()( const cfcs_::Array& arr, const std::string& cen_file_path )
    {
        cf_::cen::File file(cen_file_path, H5F_ACC_TRUNC);
        return file.fill_data( arr );
    }
};

}}}
