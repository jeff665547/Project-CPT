#pragma once
#include <CPT/forward.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/utility/assert.hpp>
#include <CPT/utility/unique_ptr.hpp>
#include <CPT/application/ui_img_index/indexer.hpp>
#include <CPT/application/ui_img_index/grid_based_indexer.hpp>
// #include <CPT/application/ui_img_index/pixel_based_indexer.hpp>
#include <CPT/application/ui_img_index/option.hpp>

namespace cpt { namespace application {
namespace ui_img_index {
template<class OPTION_PARSER>
class Main
{
    OPTION_PARSER args_;
    auto make_indexer()
    {
        return cpt::utility::make_unique( 
            (Indexer*)new GridBasedIndexer() );
    }
  public:
    Main( OPTION_PARSER&& args )
    : args_( std::forward<OPTION_PARSER>( args ) )
    {}

    void operator()()
    {
        auto&& indexer = make_indexer( );
        indexer->operator()( args_ );
    }
};

template<class OPTION_PARSER>
auto make ( OPTION_PARSER&& option_parser )
{
    return Main<OPTION_PARSER> ( 
        std::forward < OPTION_PARSER > ( option_parser )
    );
}

}}}
