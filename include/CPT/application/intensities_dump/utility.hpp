#pragma once
#include <CPT/application/intensities_dump/point.hpp>
#include <CPT/application/intensities_dump/static_config.hpp>
#include <boost/filesystem.hpp>
#include <cassert>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <Nucleona/language.hpp>
#include <algorithm>
#include <CPT/application/improc2/utils.hpp>
#include <CPT/utility/unique_ptr.hpp>
#include <regex>
namespace cpt { namespace application {
namespace intensities_dump {
namespace im_ = cpt::application::improc2;
auto pos_str2pair( const std::string& str )
{
    auto pos = str.find('_');
    assert ( str.at(pos) == '_' );
    return std::make_pair( 
          std::stoi( str.substr( 0, pos ) )
        , std::stoi( str.substr( pos + 1, str.length() - (pos + 1) ) )
    );
}

// template<class T, class MAT, class S, class E>
// decltype(auto) at( MAT&& mat, const Point<point::LBB, S, E>& p)
// {
//     auto y = mat.rows - p.y - 1;
//     auto x = p.x;
//     assert( y < mat.rows );
//     assert( x < mat.cols );
//     return mat.template at<T>( y, x );
// }
template<class ITR_T, class MAT>
struct MatAdapt
{
    decltype(auto) begin()
    {
        return m_.template begin<ITR_T>();
    }
    decltype(auto) begin() const
    {
        return m_.template begin<ITR_T>();
    }
    decltype(auto) end()
    {
        return m_.template end<ITR_T>();
    }
    decltype(auto) end() const
    {
        return m_.template end<ITR_T>();
    }
  private:
    MAT& m_;
};
template<class ITR_T, class ARCH>
void cv_mat_serialize_out( ARCH& arch, const cv::Mat& m )
{
    arch & m.rows;
    arch & m.cols;
    arch & m.type();
    for ( int i = 0; i < m.rows; i ++ )
    {
        for ( int j = 0; j < m.cols; j ++ )
        {
            arch & m.at<ITR_T>( i, j );
        }
    }
}
template<class ITR_T, class ARCH, class MAT>
void cv_mat_serialize_in( ARCH& arch, MAT&& m )
{
    int rows, cols, type;
    arch & rows;
    arch & cols;
    arch & type;
    std::decay_t<MAT> tmp( rows, cols, type );
    for ( int i = 0; i < rows; i ++ )
    {
        for ( int j = 0; j < cols; j ++ )
        {
            arch & tmp.template at<ITR_T>( i, j );
        }
    }
    m = std::move(tmp);
}
template<
      class READ_CACHE
    , class NORMAL_IO
    , class WRITE_CACHE
>
inline auto cache_able_section( 
      const std::string& cache_fname
    , READ_CACHE&&  rc
    , NORMAL_IO&&   ni
    , WRITE_CACHE&& wc
    , const bool cache_on = use_cache_debug
)
{
    if (   
           cache_on
        && boost::filesystem::exists( cache_fname ) 
    )
    {
        std::ifstream fin ( cache_fname );
        assert ( fin.is_open() );
        boost::archive::text_iarchive bi( fin );
        rc ( bi );
    }
    else
    {
        ni();
        if ( cache_on )
        {
            std::ofstream fout ( cache_fname );
            boost::archive::text_oarchive bo( fout );
            wc ( bo );
        }
    }
}


auto cv_show_color_mat( const cv::Mat& m )
{
    cv::Mat tmp;
    cv::applyColorMap(m, tmp, cv::COLORMAP_BONE );
    cv::imshow( "verbose", tmp );
    cv::waitKey(0);
}
template<class S>
auto max_threshold( S s, const typename std::decay_t<S>::value_type& t )
{
    for ( auto&& v : s )
    {
        if ( v > t ) v = t;
    }
    return std::move(s);
}
template<class M>
auto trim_outlier( M&& mm, float ltrim_rate, float utrim_rate )
{
    auto m = mm.clone();
    auto size = m.rows * m.cols;
    typename std::decay_t<M>::value_type ltrim_size ( size * ltrim_rate );
    typename std::decay_t<M>::value_type utrim_size ( size * utrim_rate );

    std::nth_element ( m.begin(), m.begin() + ltrim_size, m.end());
    auto lower = *(m.begin() + ltrim_size);

    auto upper_offset = size - utrim_size - 1;
    std::nth_element ( m.begin(), m.begin() + upper_offset, m.end());
    auto upper = *(m.begin() + upper_offset);
    for ( auto&& v : mm )
    {
        if ( v > upper ) v = upper;
        if ( v < lower ) v = lower; 
    }
    return mm;
}
auto verbose_windows_config()
{
}
template <class FUNC, class... ARGS>
void show(int16_t delay, FUNC&& func, ARGS&&... args)
{
    if ( delay >= 0)
    {
        auto image = func(args...);
        cv::imshow("verbose", image);
        cv::waitKey(delay);
    }
}
template<class T, class SM, class DM>
void mat_copy ( 
      const SM& sm
    , DM&& dm
    , const cv::Rect& sr
    , const cv::Rect& dr 
)
{
    assert ( sr.width == dr.width );
    assert ( sr.height == dr.height );
    for ( uint32_t i = 0; i < sr.height; i ++ )
    {
        for ( uint32_t j = 0; j < sr.width; j ++ )
        {
            assert( dr.y + i < dm.rows );
            assert( dr.x + j < dm.cols );
            assert( sr.y + i < sm.rows );
            assert( sr.x + j < sm.cols );
            dm.template at<T>( dr.y + i , dr.x + j ) 
                = sm.template at<T>( sr.y + i, sr.x + j );
        }
    }
}
template<class BUF >
void grid_check( const cv::Mat_<float>& mat, BUF& buf )
{
    show ( 0, [&]
    {
        auto tmp ( mat.clone() );
        trim_outlier( tmp, 0, 0.05 );
        cv::normalize( tmp, tmp, 0, 1, cv::NORM_MINMAX, CV_32F );
        tmp = tmp( cv::Rect(
              cv::Point(0,0)
            , cv::Point( 10, 10)
        ) );
//         std::ofstream fout (
// #ifdef OLD_GRIDDING
//             "grid_check.og.mat"
// #else
//             "grid_check.mat"
// #endif
//         );
        
        // for ( int i (0); i < tmp.rows; i ++ )
        // {
        //     for ( int j (0); j < tmp.cols; j ++ )
        //     {
        //         fout << tmp( i, j );
        //         if ( j < tmp.cols - 1 )
        //             fout << '\t';
        //     }
        //     fout << '\n';
        // }
        // fout.flush();
        return tmp;
    });
}
auto read_img_to_mat( const std::string& path )
{
    if( !boost::filesystem::exists( path ) )
    {
        cpt::fatal << "path : " << path << std::endl;
        abort();
    }
    auto src = im_::imread(path);
    cv::Mat res;
    cv::extractChannel(
          src, res
        , src.channels() - 1
    );
    if (src.elemSize1() == 1)
        res.convertTo(res, CV_16U, 257.0);
    return res;
}
auto make_ofstream ( const boost::filesystem::path& fname )
{
    if ( !boost::filesystem::exists( fname.parent_path() ) )
    {
        boost::filesystem::create_directories( fname.parent_path() );
    }
    return std::ofstream( fname.string() );
}
template<class STR>
auto make_ofstream ( const STR& fname )
{
    return make_ofstream( fname );
}

struct ImagePositionParser
{
    virtual std::pair<int, int> parse( const std::string& name ) const = 0;
};
struct ImNamePosMatcher : public ImagePositionParser
{
    ImNamePosMatcher( const std::string& fname_pat )
    : img_pos_pattern_ ( fname_pat )
    , raw_pat_ ( fname_pat )
    {}

    virtual std::pair<int, int> parse ( const std::string& fname ) const override
    {
        // FOV(\d+)_(\d+)
        std::smatch m;
        if ( std::regex_search ( fname, m, img_pos_pattern_ ) )
        {
            // return pos_str2pair( (m.begin() + 1 )->str() );
            return std::make_pair( std::stoi(m[1]), std::stoi(m[2]) );
        }
        std::stringstream ss;
        ss << "pattern not found : " << std::endl;
        ss << "string : "   << fname    << std::endl;
        ss << "pattern : "  << raw_pat_ << std::endl;
        throw std::logic_error ( ss.str() ); 
    }
  private:
    std::regex img_pos_pattern_;
    std::string raw_pat_;
};
struct ZeroMatcher : public ImagePositionParser
{
    ZeroMatcher(){}
    virtual std::pair<int, int> parse ( const std::string& fname ) const override
    {
        return { 0, 0 };
    }
};
std::unique_ptr<
   ImagePositionParser
> make_impos_parser( const std::string& fname_pat, const std::string& spec )
{
    if( spec=="denali" || fname_pat != "" ) 
        return cpt::utility::make_unique( new ImNamePosMatcher(fname_pat ) );
    else 
        return cpt::utility::make_unique( new ZeroMatcher( ) );
}

}
}}
