#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CCD/analyzer/probe_tm.hpp>

std::string get_output_path( const std::string& pth, const std::string& tag = "" )
{
    std::vector< std::string > path;
    boost::iter_split( path, pth, boost::algorithm::first_finder( "." ));
    return path[0] + tag + ".txt";
}

int main( int argc, char** argv )
{
    if( argc < 6 )
    {
        throw std::runtime_error( "./EXE Snp6AnnoBed K_mM=50 divalent=0 dntp=0 DNA_nM=50");
    }

    double dH;
    double dS;
    double tmp;
    int sym;

    double K_mM = atof( argv[2] );
    double divalent = atof( argv[3] );
    double dntp = atof( argv[4] );
    double DNA_nM = atof( argv[5] );

    std::string line;
    std::string file_path = argv[1];

    std::ifstream probe_file( file_path );
	std::ofstream output_a( get_output_path( file_path, "_A" ));
	std::ofstream output_b( get_output_path( file_path, "_B" ));

    while( std::getline( probe_file, line ))
    {
        std::vector< std::string > vec;
        boost::iter_split( vec, line, boost::algorithm::first_finder( "\t" ));

        std::string probe_set_id = vec[4]; 
        std::string probe_sequ_a = vec[5] + vec[6] + vec[8];
        std::string probe_sequ_b = vec[5] + vec[7] + vec[8];

        probe_tm tm_a( probe_sequ_a, K_mM, divalent, dntp, DNA_nM );
        probe_tm tm_b( probe_sequ_b, K_mM, divalent, dntp, DNA_nM );

        output_a << probe_set_id << "\t" << probe_sequ_a << "\t" << tm_a.tmp << "\n";
        output_b << probe_set_id << "\t" << probe_sequ_b << "\t" << tm_b.tmp << "\n";
    }

    output_a.close();
    output_b.close();

    return 0;
}
