#pragma once
#include <iostream>

namespace cpt {
namespace format {

struct Converter
{
    virtual void operator()( std::istream& is, std::ostream& os ) = 0;
};
struct FileConverter
{
    virtual void operator()( const std::string& ipath, const std::string& opath ) = 0;
};

} // format
} // cpt

