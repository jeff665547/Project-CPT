#pragma once
#include <utility>
#include <CPT/application/intensities_dump/background_fix.hpp>
namespace cpt{ namespace application{ namespace intensities_dump{ namespace background_fix{

struct GlobalBGFactorProcess
{
    struct Parameters
    {
        float global_bg_factor {1.0};

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
        }
        template<class JSON>
        void config( JSON&& opts )
        {
        }
    };

    template < class BUFFER >
    static void run( BUFFER&& buf )
    {
        if ( buf.BackgroundProcess::Parameters::algorithm == "partial_probe_grid_sub_base01" )
        {
            buf.src *= buf.Parameters::global_bg_factor;
        }
        else if ( buf.BackgroundProcess::Parameters::algorithm == "partial_probe_grid_sub_base02" )
        {
        }
        else{}
    }
};

template<class BUF>
float img_global_factor_handle( BUF&& bufs )
{
    if ( bufs.at(0).BackgroundProcess::Parameters::algorithm == "partial_probe_grid_sub_base01" )
    {
        float sum = 0;
        std::size_t n = 0;
        for ( auto&& b : bufs )
        {
            for ( auto&& bgms : b.bg_vals )
            {
                for ( auto&& bgm : bgms )
                {
                    sum += bgm;
                    n ++;
                }
            }
        }
        if ( n == 0 ) return 1.0;
        return sum / n;
    }
    else if ( bufs.at(0).BackgroundProcess::Parameters::algorithm == "partial_probe_grid_sub_base02" )
    {
        return 1.0;
    }
    else return 1.0;
}

}}}}
