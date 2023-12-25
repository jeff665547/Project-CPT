#pragma once
#include <CPT/visualization/vtk_util.hpp>
#include <CPT/visualization/view.hpp>
#include <Nucleona/type_traits/core.hpp>
#include <boost/proto/traits.hpp>
#include <vtkPlotLine.h>
namespace cpt {
namespace visualization {

namespace cv = cpt::visualization;
namespace bp = boost::proto;
class ScatterPlot : public cv::View
{
    using Base = cv::View;
    cv::VtkChartXYPtr chart { cv::VtkChartXYPtr::New() };
  public:
    bool show_legend        { true };
    struct PlotConf
    {
        int marker_style    ;
        unsigned char r     ;
        unsigned char g     ;
        unsigned char b     ;
        unsigned char a     ;
        float width         ;
    };
    using PointConf = PlotConf;
    const static PlotConf   default_point_conf;
    const static PlotConf   default_line_conf ;
    void initialize( )
    {
        Base::prepare_view();
        view->GetScene()->AddItem(chart);
        chart->SetShowLegend(show_legend);
    }
    template<
          class T
        , class XCOL        = int
        , class YCOL        = int
        , class POINT_CONF  = const PointConf&
    >
    void add_plot ( 
          T&&           dataset
        , XCOL&&        xcol        = 0
        , YCOL&&        ycol        = 1
        , POINT_CONF&&  pc          = default_point_conf
        , int           plot_type   = vtkChart::POINTS
    )
    {
        auto&& table ( 
            to_table ( 
                std::forward<T>(dataset) 
            ) 
        ); 
        vtkPlot *points = chart->AddPlot(plot_type);
        points->SetInputData(table, xcol, ycol);
        points->SetColor( pc.r, pc.g, pc.b, pc.a );
        points->SetWidth( pc.width );
        vtkPlotPoints::SafeDownCast(points)
            ->SetMarkerStyle(pc.marker_style);
    }
    template<
          class T
        , class XCOL        = int
        , class YCOL        = int
        , class LINE_CONF   = const PlotConf&
    >
    void add_line( 
          T&&           dataset
        , XCOL&&        xcol        = 0
        , YCOL&&        ycol        = 1
        , LINE_CONF&&   lc          = default_line_conf
    )
    {
        add_plot(
              std::forward< T          >( dataset   )
            , std::forward< XCOL       >( xcol      )
            , std::forward< YCOL       >( ycol      )
            , std::forward< LINE_CONF  >( lc        )
            , vtkChart::LINE
        );
    }
};

#ifdef SINGLE_CPP
const ScatterPlot::PointConf ScatterPlot::default_point_conf = { vtkPlotPoints::CIRCLE , 0, 0, 0, 225, 1.0 };
const ScatterPlot::PointConf ScatterPlot::default_line_conf  = { vtkPlotLine::NONE     , 0, 0, 0, 225, 1.0 };
#endif
}}
