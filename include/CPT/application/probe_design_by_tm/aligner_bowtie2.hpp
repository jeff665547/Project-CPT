#pragma once
#include <CCD/aligner/bowtie2.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

class AlignerBowtie2 : public Bowtie2Aligner
{
    std::string aligner_location_; 
    std::string aligner_index_;
    std::string options_;

    std::string build_file_align_query( const std::string& file )
    {
        return aligner_location_ + 
            " --mm" + 
            " -x" + 
            " " + aligner_index_ +  
            options_ +
            " " +
            file;
    }

  public:

    AlignerBowtie2( const std::string& aligner_location, const std::string& aligner_index )
    : Bowtie2Aligner( aligner_location, aligner_index )
    , aligner_location_( aligner_location )
    , aligner_index_( aligner_index )
    {}

    void set_options(const std::string& options)
    {
        options_ = " " + options;
    }

    template< class SAM = Sam<> >
    Vector< SAM > file_align( const std::string& file_name )
    {
        Vector< SAM > res;
        auto cmd = build_file_align_query( file_name );

        Process process( cmd );
        process.run( 0, 1 );

        auto p_stream = process.get_source( 0 );
        auto p_err = process.get_source( 1 );

        while ( true )
        {
            SAM sam = parse_sam_stream< SAM >( *p_stream );

            if( sam.eof_flag )
            {
                break;
            }

            res.push_back( sam );
        }

        return res;
    }
};

} // probe_design_by_tm
} // application
} // cpt
