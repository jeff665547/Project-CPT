#pragma once
#include <CPT/format/converter.hpp>
#include <CPT/format/cad.hpp>
namespace cpt{
namespace application{
namespace cadtool {

namespace cf = cpt::format;
struct CadTrait
{
    using Probe         = cf::cad::Probe;
    using ProbesetType  = cf::cad::ProbesetType<Probe>;
    using Probeset      = cf::cad::Probeset<Probe, ProbesetType>;
    using Cad           = cf::Cad<Probeset>;
};

}}}
