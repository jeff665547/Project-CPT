#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CCD/analyzer/probe_tm.hpp>

std::string get_seq( std::string seq )
{
    if( seq.at(0) == '[' )
    {
        seq.erase( 0, 5 );
    }
    else
    {
        seq.erase( seq.size()-5, 5 );
    }

    return seq;
}

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
        throw std::runtime_error( "./EXE ProbeTabFile K_mM=50 divalent=0 dntp=0 DNA_nM=50");
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
	std::ofstream outpt_file( get_output_path( file_path ));

	std::ofstream outpt_4050( get_output_path( file_path, "_4050" ));
	std::ofstream outpt_5060( get_output_path( file_path, "_5060" ));
	std::ofstream outpt_6070( get_output_path( file_path, "_6070" ));
	std::ofstream outpt_7080( get_output_path( file_path, "_7080" ));

	std::ofstream outpt_5862( get_output_path( file_path, "_5862" ));
	std::ofstream outpt_5565( get_output_path( file_path, "_5565" ));
	std::ofstream outpt_5070( get_output_path( file_path, "_5070" ));
	std::ofstream outpt_4575( get_output_path( file_path, "_4575" ));



    std::vector< std::tuple< double, std::string, std::string >> seq_vec;

    while( std::getline( probe_file, line ))
    {
        std::vector< std::string > vec;
        boost::iter_split( vec, line, boost::algorithm::first_finder( "\t" ));

        std::string probe_set_id = vec[0]; 
        std::string probe_sequce = get_seq( vec[1] );

        if( probe_set_id == "PROBESET_ID" )
        {
            outpt_file
                << "probe_set_id\t"
                << "probe_sequce\t"
                << "Tm.tmp\n";
                // << "Tm.sym\t"
                // << "Tm.deltaH\t"
                // << "Tm.deltaS\n";

            outpt_4050 << "probeset_id\n";
            outpt_5060 << "probeset_id\n";
            outpt_6070 << "probeset_id\n";
            outpt_7080 << "probeset_id\n";

            outpt_5862 << "probeset_id\n";
            outpt_5565 << "probeset_id\n";
            outpt_5070 << "probeset_id\n";
            outpt_4575 << "probeset_id\n";

            continue;
        }

        probe_tm Tm( probe_sequce, K_mM, divalent, dntp, DNA_nM );
        tmp = Tm.tmp;
        sym = Tm.sym;
        dH  = Tm.deltaH;
        dS  = Tm.deltaS;

        seq_vec.emplace_back( tmp, probe_set_id, probe_sequce );

        // outpt_file
        //     << probe_set_id << "\t"
        //     << probe_sequce << "\t"
        //     << tmp << "\t"
        //     << sym << "\t"
        //     << deltaH << "\t"
        //     << deltaS << "\n";

        if( tmp > 40 && tmp < 50 ) 
        {
            outpt_4050 << probe_set_id << "\n";
        }

        if( tmp > 50 && tmp < 60 ) 
        {
            outpt_5060 << probe_set_id << "\n";
        }

        if( tmp > 60 && tmp < 70 ) 
        {
            outpt_6070 << probe_set_id << "\n";
        }

        if( tmp > 70 && tmp < 80 ) 
        {
            outpt_7080 << probe_set_id << "\n";
        }

        if( tmp > 58 && tmp < 62 ) 
        {
            outpt_5862 << probe_set_id << "\n";
        }

        if( tmp > 55 && tmp < 65 ) 
        {
            outpt_5565 << probe_set_id << "\n";
        }

        if( tmp > 50 && tmp < 70 ) 
        {
            outpt_5070 << probe_set_id << "\n";
        }

        if( tmp > 45 && tmp < 75 ) 
        {
            outpt_4575 << probe_set_id << "\n";
        }
    }

    std::sort( seq_vec.begin(), seq_vec.end(),
        []( const std::tuple< double, std::string, std::string >& a, const std::tuple< double, std::string, std::string >& b )
        { return std::get<0>(a) < std::get<0>(b); });

    for( auto& seq : seq_vec )
    {
        outpt_file << std::get<1>(seq) << "\t" << std::get<2>(seq) << "\t" << std::get<0>(seq) << "\n";
    }

    probe_file.close();
    outpt_file.close();

    return 0;
}
