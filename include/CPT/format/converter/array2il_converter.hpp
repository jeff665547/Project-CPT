#pragma once
#include <CPT/format/chip_sample.hpp>
#include <CPT/logger.hpp>
#include <fstream>
namespace cpt{ namespace format{ namespace converter{
namespace cfcs_     = cpt::format::chip_sample;
class Array2Il
{
    template<class ID, class N_COLS>
    static auto index2pos ( ID id, N_COLS n_cols )
    {
        struct Pos
        {
            decltype(id) x;
            decltype(id) y;
        };
        return Pos { id % n_cols, id / n_cols };
    }
  public:
    auto operator()( const cfcs_::Array& arr, const std::string& file_path )
    {
        cpt::warn << "array to intensities list only support one channel dump" << std::endl;
        std::ofstream il ( file_path );
        // auto&& intensities ( arr.extract_intensities(0) );
        auto&& n_cols( arr.feature_columns() );
        auto&& ch( arr.channels().at(0) );
        auto&& intensities ( ch.intensity );
        auto&& pixel_detail( ch.raw_pixel_value );
        auto&& cv_value ( ch.cv_value );
        il << "x\ty\tmean\toverlap_detail(deprecated)\tpixel_detail\tcv" << std::endl;
        for ( std::decay_t<decltype(intensities)>::size_type i (0); i < intensities.size(); i ++ )
        {
            auto&& value( intensities.at(i) );
            auto pos ( index2pos(i, n_cols) );
            il 
                << pos.x << '\t' 
                << pos.y << '\t' 
                << value << '\t' 
                << value << '\t' 
            ;
            for( auto&& v : pixel_detail.at(i) )
            {
                assert( v >= 0);
                il << v << ',';
            }
            il << '\t';
            il << cv_value.at(i ) << std::endl;

        }
        return true;
    }
};

}}}
