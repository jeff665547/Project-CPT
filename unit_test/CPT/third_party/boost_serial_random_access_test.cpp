#include <string>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <iostream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

struct S
{
    std::vector<int> int_seq;
    std::string name;
    int id;
    std::vector<std::string> str_list;
    template<class ARC>
    void serialize ( ARC&& arc, const unsigned int version )
    {
        arc & int_seq;
        arc & name;
        arc & id;
        arc & str_list;
    }
};
auto make_int_seq( int i )
{
    std::vector<int> res;
    for ( int j = 0; j < i; j ++ )
    {
        res.emplace_back(j);
    }
    return res;
}
auto make_name( int i )
{
    std::string res;
    for ( int j = 0; j < i; j ++ )
    {
        res.push_back((j % 26) + 97);
    }
    return res;
}
auto make_string_list( int i )
{
    std::vector<std::string> res;
    for ( int j = 0; j < i; j ++ )
    {
        std::string str;
        for ( int k = 0; k < j + 1; k ++ )
        {
            str.push_back( 'X' );
        }
        res.emplace_back( std::move( str ) );
    }
    return res;
}
int main()
{
    int max_iter ( 500 );
    {
        std::ofstream index("ba_testout.idx");
        std::ofstream fout("ba_testout.archive");
        boost::archive::text_oarchive oa(fout);
        for ( int i = 0; i < max_iter; i ++ )
        {
            S s{
                  make_int_seq(i)
                , make_name(i)
                , i
                , make_string_list(i)
            };
            auto fpos ( fout.tellp() );
            index << fpos << std::endl;
            oa & s;
        }
    }
    {
        std::ifstream index("ba_testout.idx");
        std::ifstream fin("ba_testout.archive");
        boost::archive::text_iarchive ia (fin);
        std::vector<std::size_t> fposs;
        while( index )
        {
            std::size_t fpos;
            index >> fpos;
            fposs.emplace_back( std::move ( fpos ) );
        }
        for ( int i = 0; i < max_iter; i += 4 )
        {
            S s;
            auto pos ( fposs.at(i));
            fin.seekg( pos );
            ia & s;
            assert ( s.id == i );
            std::cout << s.name << std::endl;
        }
    }
}
