
#include <CPT/utility.hpp>
namespace cpt {
std::ofstream make_ofstream ( const boost::filesystem::path& fname )
{
    if ( !boost::filesystem::exists( fname.parent_path() ) )
    {
        boost::filesystem::create_directories( fname.parent_path() );
    }
    return std::ofstream( fname.string() );
}
}