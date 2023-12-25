#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <mutex>
namespace bpt = boost::property_tree;
namespace bfs = boost::filesystem;
namespace cpt
{
    namespace engine
    {
        namespace components
        {
            struct NamedComponent;
        }
    }
    namespace format
    {
        template<unsigned>
        class ChipLayout;
        template< 
              class Model
            , template<class T, class... RT> class ROW_ID_CONTAINER
            , template<class T, class... RT> class ROW_MUX_CONTAINER
            , class MUX 
        >
        class TsvTpl;
        template<
              template<class T, class... RT> class ROW_ID_CONTAINER
            , template<class T, class... RT> class ROW_MUX_CONTAINER
            , class TsvType
            , class MUX
        >
        class TsvRefTpl;
        template< 
              template<class T, class... RT> class CONTAINER
            , template<class T, class... RT> class ROW_MUX_CONTAINER
            , class TSV_TYPE
            , class MUX
        > 
        class TsvViewTpl;

        template<
            template<class T, class... RT> class ROW_ID_CONTAINER
            , template<class T, class... RT> class ROW_MUX_CONTAINER
            , class MUX
            , class TSV_TYPE
        >
        class TsvModifierTpl; 

        // namespace cad
        // {
        //     #define PROBESET_TPL_ARG_PACK(c)
        //         c PROBE, c PROBESET_TYPE
        //     template< PROBESET_TPL_ARG_PACK(class) >
        //     struct Probeset;

        //     struct ChannelDes;
        //     struct AlleleBaseMapChannelDes;
        //     struct BaseMapChannelDes;

        //     // template<class PROBE>
        //     // struct GenotypingQC2Channel;
        //     // template<class PROBE>
        //     // struct GenotypingSNP2Channel;
        // }
        // #define CAD_TPL_ARG_PACK(c)
        //     c PROBESET
        // template< CAD_TPL_ARG_PACK(class) >
        // struct Cad;
    }
    namespace algorithm
    {
    }
}
