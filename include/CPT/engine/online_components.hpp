#pragma once
#include <boost/mpl/list.hpp>
#include <CPT/components/quantile_norm.hpp>
#include <CPT/components/m_a_plot.hpp>
namespace cpt
{
namespace engine
{
using OnlineComponents = boost::mpl::list<
      cpt::component::MAPlot
    , cpt::component::QuantileNorm
> ;
}
}
