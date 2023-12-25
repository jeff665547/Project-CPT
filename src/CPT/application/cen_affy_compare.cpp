#include <CPT/logger.hpp>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <bitset>
#include <cassert>
#include <CPT/range.hpp>
RANGE_NAMESPACE_SHORTCUT
struct Matrix
{
    std::map<std::string, uint16_t> header;
    std::map<std::string, uint64_t> label;
    std::vector<std::vector<int8_t>> content;
    template<class STR>
    void set_header ( const std::vector<STR>& _header)
    {
        uint16_t i (0);
        for(auto&& h : _header)
        {
            if ( h != "probeset_id" )
            {
                header[h] = i;
                i++;
            }
        }
    }
    void push_row(const std::string& row, std::vector<int8_t>&& rc  )
    {
        label.emplace(row, label.size());
        content.emplace_back(std::move(rc));
    }
    auto& at(const std::string& r, const std::string& c)
    {
        return content.at(label.at(r)).at(header.at(c));
    }
};
template<typename C>
void split(std::string const& s, char const* d, C& ret)
{
    C output;

    std::bitset<255> delims;
    while( *d )
    {
        unsigned char code = *d++;
        delims[code] = true;
    }
    typedef std::string::const_iterator iter;
    iter beg;
    bool in_token = false;
    for( std::string::const_iterator it = s.begin(), end = s.end();
            it != end; ++it )
    {
        if( delims[*it] )
        {
            if( in_token )
            {
                output.push_back(typename C::value_type(beg, it));
                in_token = false;
            }
        }
        else if( !in_token )
        {
            beg = it;
            in_token = true;
        }
    }
    if( in_token )
        output.push_back(typename C::value_type(beg, s.end()));
    output.swap(ret);
}
void read_affy_data(const std::string& fname, Matrix& ds)
{
    const static std::string consider_prefix ( "" );
    const static std::string header_prefix ( "probeset_id" );
    bool affter_header = false;
    std::ifstream fin ( fname );
    std::string line;
    while(std::getline(fin, line))
    {
        auto mm = std::mismatch(consider_prefix.begin(), consider_prefix.end(), line.begin());
        if( 
                affter_header 
                // && mm.first == consider_prefix.end() 
        )
        {
            std::vector<std::string> d_v;
            split(line, "\t", d_v);
            std::vector<int8_t> d_vi;
            int i = 0;
            for( auto&& s : d_v )
            {
                if( i != 0 )
                {
                    d_vi.emplace_back(std::stoi(s));
                }
                i++;
            }
            ds.push_row(d_v[0], std::move(d_vi));
        }
        else if ( std::mismatch(header_prefix.begin(), header_prefix.end(), line.begin()).first == header_prefix.end() )
        {
            std::vector<std::string> header_v;
            split(line, "\t", header_v);
            for( int i = 1; i < header_v.size(); i ++ )
            {
                header_v[i] = header_v[i].substr(0, header_v[i].length() - 4);
            }
            ds.set_header(header_v);
            affter_header = true;
        }
    }
    // std::cout << header_prefix ;
    // for ( auto& v : ds.header )
    // {
    //     std::cout << '\t' << v.first;
    // }
    // std::cout << '\n';
    // 
    // for ( auto& l : ds.label )
    // {
    //     std::cout << l.first;
    //     for( auto& f : ds.content[l.second] )
    //     {
    //         std::cout << '\t' << (int)f;
    //     }
    //     std::cout << std::endl;
    // }
}
int main(int argc, const char* argv[])
{
    using namespace std::string_literals;
    if ( argc != 4 )
    {
        cpt::msg << "cen_affy_compare [centrillion_genotype_result] [affymatrix genotype_result] [output_prefix]\n";
    }
    else
    {
        /* this is a cen result based comparence */
        std::string centrillion_genotype_result(argv[1]);
        std::string affymatrix_genotype_result(argv[2]);
        std::string output_prefix(argv[3]);

        Matrix m;
        read_affy_data(affymatrix_genotype_result, m);

        std::vector<std::vector<uint64_t>> conf_matrix { 5, std::vector<uint64_t>(5, 0) };
        std::ifstream fin(centrillion_genotype_result);
        std::map<std::string, std::size_t> sample_miss_num;
        std::map<std::string, std::size_t> ps_miss_num;
        std::ofstream detail            (output_prefix + "_cac_detail.tsv"s         );
        std::ofstream miss_smp_profile  (output_prefix + "_cac_smp_profile.tsv"s    );
        std::ofstream miss_ps_profile   (output_prefix + "_cac_ps_profile.tsv"s     );
        std::ofstream cmf               (output_prefix + "_cac_conf_matrix"s        );

        {
            std::string line;
            int i = 0;
            detail << "probeset_name\tsample_name\taffy\tcen" << std::endl;;
            while(std::getline(fin, line))
            {
                if( i == 0 ) { i++; continue; }
                std::vector<std::string> line_f;
                split(line, "\t", line_f);
                int8_t genotype = (int8_t)std::stoi(line_f.at(2));
                // if ( genotype == -2 ) continue; // ignore model fail
                auto& probeset_name = line_f.at(4);
                auto& sample_name = line_f.at(5);
                try
                {
                    auto affy_type = (int)m.at(probeset_name, sample_name );
                    auto cen_type = (int)genotype;
                    assert ( affy_type + 2 < 5 );
                    assert ( cen_type + 2 < 5 );
                    conf_matrix.at( affy_type + 2).at( cen_type+ 2)++;
                    if ( affy_type != cen_type && affy_type >= 0)
                    {
                        detail << probeset_name << '\t' << sample_name << '\t' << affy_type << '\t' << cen_type << std::endl;
                        sample_miss_num [ sample_name   ] ++ ;
                        ps_miss_num     [ probeset_name ] ++ ;
                    }
                }
                catch(const std::out_of_range& e)
                {
                    std::cerr << probeset_name << '\t' << sample_name << std::endl;
                }
            }
            auto value_sorter = []( auto&& rng ) 
                {
                    return rng 
                    | ba::transformed( []( auto&& pp ) { return &pp; } )
                    | ::to_vector
                    | ca::sort( [] ( auto&& p1, auto&& p2 ) { return p1->second < p2->second; } )
                    | glue(ba::transformed( []( auto&& pp ) { return *pp; } ))
                    ;
                }
            ;
            for ( auto&& p : value_sorter( sample_miss_num ))
                miss_smp_profile << p.first << '\t' << p.second << std::endl;
            for ( auto&& p : value_sorter( ps_miss_num  ))
                miss_ps_profile << p.first << '\t' << p.second << std::endl;
            cmf << "-\t-2\t-1\t0\t1\t2\tcen" << std::endl;
            int j = 0;
            for(auto& v : conf_matrix)
            {
                cmf << (( j++ % 5 ) - 2);
                for(auto& g : v )
                {
                    cmf << '\t' << g;
                }
                cmf << std::endl;
            }
            cmf << "affy" << std::endl;

            uint64_t cor(0);
            for(int i = 2 ; i < 5; i ++ )
            {
                cor += conf_matrix[i][i];
            }
            uint64_t all(0);
            for(auto&& vv : conf_matrix | ::range_indexed(0))
            {
                for(auto&& v : vv.value() | ::range_indexed(0))
                {
                    if ( vv.index() >=2 )
                        all += v.value();
                }
            }
            cmf << "acc : " << (float)cor / (float)all << std::endl;
        }

    }
}
