#include <iostream>
#include <CPT/range.hpp>
#include <string>
#include <utility>
#include <random>
RANGE_NAMESPACE_SHORTCUT
constexpr auto& loggeri = std::cout;
int main(int argc, const char* argv[])
{
    if ( argc < 4 )
    {
        std::cout << "ps_select [select number] [file name] [seed]" << std::endl;
    }
    else
    {
        std::string seed_str( argv[3] );
        std::seed_seq seed(seed_str.begin(), seed_str.end());
        std::ifstream fin ( argv[2] );
        auto fcontent ( 
            fin | ca::getlined() | ::range_indexed_t<std::string>(0) 
            | ::pipe_redirect([]( auto&& rng ) { 
                loggeri << rng.begin()->second << '\n';
                return boost::make_iterator_range(++rng.begin(), rng.end()); 
            } )
            | ::eval_to_vector 
        );

        std::default_random_engine engine(seed);
        std::uniform_int_distribution<std::size_t> dist;
        std::size_t back = fcontent.size();
        for ( auto i : cu::irange_0(std::stoi(argv[1])) )
        {
            std::swap(fcontent.at(dist(engine)% back), fcontent.at(back-1));
            back --;
        }
        boost::make_iterator_range(fcontent.begin() + back, fcontent.end()) | ca::sort(
            [](auto&& p1, auto&& p2)
            { 
                return p1.first < p2.first; 
            } 
        )
        | ba::transformed([](auto&& p){return p.second;}) | ca::printed(loggeri, '\n') | ::endp;
    }
}
