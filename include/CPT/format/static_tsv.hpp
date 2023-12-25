#pragma once
#include <CPT/utility/ltuple.hpp>
namespace cu = cpt::utility;
namespace cpt {
namespace format {

template<
      template<class...T> class VEC
    , class TABLE_IDX
    , class... F
>
struct StaticTableRowMajorModel
{
    using Row = cu::LTuple<TABLE_IDX, F...>;
    using RowContainer = VEC<Row>;
    RowContainer content;
};
#define RTGET(tbl, r, label) \
LTGET(label, tbl.content[r])
template<class MODEL>
struct StaticTsv
{
};

}}
