#pragma once
namespace cpt {
namespace utility {
namespace copy {

template<class PTR>
auto virtual_copy( const PTR& ptr )
{
    if ( ptr != nullptr )
        return ptr->clone();
    else return PTR(nullptr);
}
}}}
