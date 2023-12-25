#pragma once
#include <CPT/visualization/scatter_plot.hpp>
#include <CPT/visualization/table.hpp>
#include <armadillo>
#include <CPT/logger.hpp>
namespace cpt {
namespace visualization {
using GMM = mlpack::gmm::GMM;
using GMean         = arma::vec;
using GMMean        = std::vector<GMean>;
void show_plot_sample( 
      ScatterPlot&              view
    , const arma::mat&              sample
    , const arma::Row<std::size_t>& labels 
    , int                           ps      = vtkPlotPoints::CIRCLE
) 
{
    auto table = clustermatrix_to_vtk_table(sample, labels);
    std::vector<ScatterPlot::PointConf> plot_confs {
        ScatterPlot::PointConf {
              ps
            , 255, 0, 0, 225              // color
            , 1.0                       // point width 
        }
        , ScatterPlot::PointConf {
              ps
            , 0, 255, 0, 225              // color
            , 1.0                       // point width 
        }
        , ScatterPlot::PointConf {
              ps
            , 0, 0, 255, 225              // color
            , 1.0                       // point width 
        }
    };
    for ( auto&& i : cu::irange_0(table.size()))
    {
        view.add_plot(table[i], 0, 1, plot_confs[i]);
    }
}
void show_plot_sample( 
      ScatterPlot&              view
    , const arma::mat&              sample
    , int                           ps      = vtkPlotPoints::CIRCLE
)
{
    auto table = samplematrix_to_vtk_table(sample);
    std::vector<ScatterPlot::PointConf> plot_confs {
        ScatterPlot::PointConf {
            ps
            , 0, 0, 0, 225              // color
            , 1.0                       // point width 
        }
    };
    for ( auto&& i : cu::irange_0(table.size()))
    {
        view.add_plot(table[i], 0, 1, plot_confs[i]);
    }
}
void show_plot_model( 
      ScatterPlot& view
    , const mg::GMM& gmm 
)
{
    auto table = model_to_vtktable( gmm );
    std::vector<ScatterPlot::PlotConf> plot_confs {
        ScatterPlot::PointConf {
            vtkPlotLine::NONE           // point style
            , 255, 0, 0, 225            // color
            , 3.0                       // point width 
        }
        , ScatterPlot::PointConf {
            vtkPlotLine::NONE       // point style
            , 0, 255, 0, 225              // color
            , 3.0                       // point width 
        }
        , ScatterPlot::PointConf {
            vtkPlotLine::NONE       // point style
            , 0, 0, 255, 225              // color
            , 3.0                       // point width 
        }
    };
    for ( auto&& i : cu::irange_0(table.size()))
    {
        view.add_line(table[i], 0, 1, plot_confs[i]);
    }
}
template<class OS = decltype(cpt::dbg)&>
void show_mean_info( const GMMean& g_m_mean, OS&& os = cpt::dbg) 
{
    os << "   GMM component num : " << g_m_mean.size() << std::endl;
    os << "   GMM means : " << std::endl;
    for ( auto&& mean : g_m_mean )
    {
        os << "       " << mean[0] << ", " << mean[1] << std::endl;
    }
}
template<class OS = decltype(cpt::dbg)& >
void show_ob ( const arma::mat& ob, OS&& os = cpt::dbg) 
{
    for ( auto& col : ob | glue(ca::col_range()) )
    {
        os << "(" << col[0] << ", " 
            << col[1] << ")" << std::endl;
    }
}
template<class OS = decltype(cpt::dbg)& >
void show_model_info( const GMM& gmm, OS&& os = cpt::dbg ) 
{
    os << "gmm model info" << std::endl;
    gmm
        | ca::mdl_components()
        | glue( ::range_indexed(0) )
        | glue( ba::transformed ( 
            [&os] ( auto&& comp_i )
            {
                os << "   sub_component " << comp_i.index() << std::endl;
                os << "   mean : " 
                    << comp_i.value().Mean()[0] 
                    << ", "  << comp_i.value().Mean()[1] << std::endl;
                os << "   cov : " << std::endl;
                os << comp_i.value().Covariance();
                os << std::endl;
                return comp_i.value();
            }
        ))
        | ::endp
        ;
}
}}
