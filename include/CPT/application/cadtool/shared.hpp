#pragma once 
#include <string>
// #include <CPT/application/cadtool/main.hpp>
// #include <CPT/dll_config.hpp>
#include <memory>
#include <CPT/format/converter.hpp>
#include <functional>
#include <CPT/application/cadtool/cad2json.hpp>
#include <CPT/application/cadtool/json2cad.hpp>
namespace cpt { namespace application {
namespace cadtool {
struct Parameters
{
    // std::ifstream input_stream;
    // std::ofstream output_stream;
    std::string ipath;
    std::string opath;
    std::unique_ptr<cpt::format::FileConverter> converter;
};
// template<class FUNC>
void file_convert_to_impl(
    Parameters& p
    //   const std::string& input_file_path
    // , const std::string& output_file_path  
    // , FUNC&& new_converter
)
{
    p.converter->operator()( p.ipath, p.opath );
    
    // cpt::application::cadtool::Parameters params
    // {
    //     input_file_path, output_file_path, nullptr
    // };
    // params.converter.reset( new_converter() );
    // params.converter->operator()(
    //     params.ipath, params.opath
    // );
}
// std::function<cpt::format::FileConverter*(void)>
//     get_converter_creater( const std::string& mode )
// {
//     std::function<cpt::format::FileConverter*(void)> res;
//     if ( mode == "json2cad" )
//         res = []() { return new cpt::application::cadtool::Json2Cad(); };
//     else if ( mode == "cad2json" )
//         res = []() { return new cpt::application::cadtool::Cad2Json(); };
//     else
//         throw std::logic_error ( "unknown mode : " + mode );
//     return res;
// }
std::unique_ptr<cpt::format::FileConverter> 
    get_converter_creater( const std::string& mode )
{
    if ( mode == "json2cad" )
        return std::unique_ptr<cpt::format::FileConverter> ( 
            new cpt::application::cadtool::Json2Cad() );
    else if ( mode == "cad2json" )
        return std::unique_ptr<cpt::format::FileConverter> ( 
            new cpt::application::cadtool::Cad2Json() );
    else
        throw std::logic_error ( "unknown mode : " + mode );
}



void file_convert_to(
      const std::string& input_file_path
    , const std::string& output_file_path  
    , const std::string& mode
)
{
    Parameters p
    {
        input_file_path, output_file_path, get_converter_creater( mode )
    };
    file_convert_to_impl( p );
}
void json2cad_file_convert_to(
      const std::string& input_file_path
    , const std::string& output_file_path  
)
{
    Parameters p
    {
          input_file_path
        , output_file_path
        , std::unique_ptr<cpt::format::FileConverter>(
             new cpt::application::cadtool::Json2Cad()
        )
    };
    file_convert_to_impl( p );
    // file_convert_to_impl( 
    //       input_file_path
    //     , output_file_path
    //     , [](){ return new cpt::application::cadtool::Json2Cad(); }
    // );
}
void cad2json_file_convert_to(
      const std::string& input_file_path
    , const std::string& output_file_path  
)
{
    Parameters p
    {
          input_file_path
        , output_file_path
        , std::unique_ptr<cpt::format::FileConverter>(
             new cpt::application::cadtool::Cad2Json()
        )
    };
    file_convert_to_impl( p );
    // file_convert_to_impl( 
    //       input_file_path
    //     , output_file_path
    //     , [](){ return new cpt::application::cadtool::Cad2Json(); }
    // );
}

}}}
