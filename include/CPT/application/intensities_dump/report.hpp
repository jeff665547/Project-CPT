#pragma once
#include <string>
#include <fstream>
#include <CPT/format/json.hpp>
#include <map>
namespace cpt { namespace application {
namespace intensities_dump {

struct Report
{
    Report()
    : image_status( root.create_list("image_status") )
    {}
    template < class BUFFER >
    void process_buf( BUFFER&& buf )
    {
        boost::filesystem::path image_path( buf.img_path);
        auto jimg_st = cpt::format::make_json();
        jimg_st.add(
            image_path.stem().string(), 
            buf.roi_qc_fail ?  "QC_FAIL" : "SUCCESS"
        );
        image_status.push_back( jimg_st );
    }
    template<class OS>
    void write( OS&& os )
    {
        root.dump( os );
    };
    cpt::format::Json<boost::property_tree::ptree> root;
    cpt::format::JsonList<boost::property_tree::ptree&> image_status;
};

}}}
