#pragma once
#include <CPT/engine/data_pool.hpp>
#include <CPT/format/json.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <string>
#include <armadillo>
#include <boost/range/adaptors.hpp>
#include <CPT/algorithm/sort.hpp>
#include <CPT/algorithm/argsort.hpp>
#include <boost_addon/range_vector.hpp>
#include <CPT/algorithm/distance.hpp>
#include <boost_addon/range_indexed.hpp>
#include <map>
#include <boost/range/combine.hpp>
#include <CPT/algorithm/getlined.hpp>
#include <CPT/logger.hpp>

#include <CPT/algorithm/matrix_range.hpp>
#include <CPT/components/birdseed/typedef.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#ifndef NO_VTK
#include <vtkVariantArray.h>
#include <CPT/visualization/vtk_util.hpp>
#include <CPT/visualization/table.hpp>
#include <CPT/visualization/scatter_plot.hpp>
#include <vtkStdString.h>
#include <vtkPlotLine.h>
#endif
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <CPT/visualization/geometrics/ellipse.hpp>
#include <CPT/algorithm/g_m_m_util/access.hpp>
#include <CPT/algorithm/mdl_components.hpp>
#include <CPT/utility/irange.hpp>
#include <cmath>
#ifndef PROJECT_ROOT
#define PROJECT_ROOT "./"
#endif 
namespace cf = cpt::format;
namespace ce = cpt::engine;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace ced = cpt::engine::data_pool;
namespace ccb = cpt::component::birdseed;
namespace bt = boost;
namespace cv = cpt::visualization;
namespace md = mlpack::distribution;
namespace mg = mlpack::gmm;
namespace cvg = cv::geometrics;
using namespace std::string_literals;
struct Option
{
    std::ifstream pipeline_schema_stream_ ;
    std::stringstream result_schema_stream_ ;
};
template<class D1, class D2, class S1, class S2>
struct GenotypeDataProto
{
    D1 a_allele;
    D2 b_allele;
    S1 probeset_name;
    S2 sample_name;
private: 
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & a_allele;
        ar & b_allele;
        ar & probeset_name;
        ar & sample_name;
    }
};
using GenotypeFileTrait = cpt::format::TraitPrototype<
      GenotypeDataProto
    , cf::PMT<0, double>
    , cf::PMT<1, double>
    , cf::PMT<4, std::string>
    , cf::PMT<5, std::string>
>;
using GenotypeFileParser = cf::TupleParser<GenotypeFileTrait>;
template<class RNG>
auto group(RNG&& rng)
{
    std::map<typename std::decay_t<RNG>::value_type, std::size_t> res;
    std::size_t i(0);
    for ( auto&& v : rng )
    {
        auto p = res.emplace( v, i );
        if ( p.second ) i++;
    }
    return res;
}
auto read_genotype_file(
      cf::Cube<double>& cube
    , const std::string& genotype_path = PROJECT_ROOT + "/unit_test/data/components/birdseed/calling_genotype_small.tsv"s
)
{
    using GenotypeData = typename GenotypeFileParser::Entry;
    GenotypeFileParser parser("\t");

    std::vector<GenotypeData> genotype_datas;
    const std::string cache("./genotype_datas.archive");
    if ( !bt::filesystem::exists(cache) )
    {
        auto raw_line_range = std::ifstream(genotype_path) | ca::getlined();
        for ( auto&& line : boost::make_iterator_range(++ raw_line_range.begin(), raw_line_range.end() )) // TODO performance issue
        {
            genotype_datas.emplace_back(parser(std::move(line)));
        }
        cpt::msg << "read all datas" << std::endl;
        {
            std::ofstream fout(cache);
            bt::archive::text_oarchive oa(fout);
            oa << genotype_datas;
        }
    }
    else
    {
        std::ifstream fin(cache);
        bt::archive::text_iarchive ia(fin);
        ia >> genotype_datas;
    }

    auto probeset_names = genotype_datas | ba::transformed(
        []( auto&& gd ){ return gd.probeset_name;}
    );
    auto ps_gindex ( group( probeset_names ) );

    auto sample_names = genotype_datas | ba::transformed(
        []( auto&& gd ){ return gd.sample_name;}
    );
    auto smp_gindex ( group( sample_names ) );
   
    cf::Cube<double> _cube(2, smp_gindex.size(), ps_gindex.size(), arma::fill::zeros);
    for ( auto&& gd : genotype_datas)
    {
        auto smp_pos    = smp_gindex.at(gd.sample_name);
        auto ps_pos     = ps_gindex.at(gd.probeset_name);
        _cube(0, smp_pos, ps_pos) = gd.a_allele;
        _cube(1, smp_pos, ps_pos) = gd.b_allele;
    }
    cube.swap(_cube);
    cpt::msg << "cube ready" << std::endl;
    return std::make_tuple(
          std::move( ps_gindex)
        , std::move(smp_gindex) 
    );
}
#ifndef NO_VTK
auto clustermatrix_to_vtk_table( 
      const arma::mat&              sample
    , const arma::Row<std::size_t>& labels 
)
{
    std::vector<cv::VtkTablePtr> res {
          cv::make_vtk_table<double, double>("a_allele", "b_allele")
        , cv::make_vtk_table<double, double>("a_allele", "b_allele")
        , cv::make_vtk_table<double, double>("a_allele", "b_allele")
    };

    auto scol = sample | ca::col_range();
    for(auto&& ccol : bt::combine(scol, labels))
    {
        auto row = vtkVariantArray::New();
        row->Resize(2);
        row->InsertValue(0, ccol.get<0>()[0]);
        row->InsertValue(1, ccol.get<0>()[1]);
        res[ccol.get<1>()]->InsertNextRow(row);
    }
    return res;
}
auto scatter_plot(
    const std::vector<cv::VtkTablePtr>& table
)
{
    cv::ScatterPlot sp;
    sp.show_legend = false;
    sp.initialize();
    std::vector<cv::ScatterPlot::PointConf> plot_confs {
        cv::ScatterPlot::PointConf {
            vtkPlotPoints::CIRCLE       // point style
            , 255, 0, 0, 225              // color
            , 1.0                       // point width 
        }
        , cv::ScatterPlot::PointConf {
            vtkPlotPoints::CIRCLE       // point style
            , 0, 255, 0, 225              // color
            , 1.0                       // point width 
        }
        , cv::ScatterPlot::PointConf {
            vtkPlotPoints::CIRCLE       // point style
            , 0, 0, 255, 225              // color
            , 1.0                       // point width 
        }
    };
    for ( auto& i : {0, 1, 2 } )
    {
        sp.add_plot(table[i], 0, 1, plot_confs[i]);
    }
    return sp;
}
auto model_to_vtktable ( const mg::GMM& gmm, const std::size_t& polygon_points_num = 1000 )
{
    return 
    gmm 
    | ca::mdl_components()
    | ba::transformed(
        [
            &polygon_points_num
        ]( const auto& gd )
        {
            auto ef( cvg::ellipse_gd( gd ) );
            auto line_table(cv::make_vtk_table<float, float>("x", "y"));
            line_table->SetNumberOfRows(polygon_points_num);
            for ( auto i : bt::combine(
                  cu::irange_0(polygon_points_num)
                , arma::linspace<arma::vec>(0.0, 2*M_PI, polygon_points_num)
            ) )
            {
                auto pvec(ef(i.template get<1>()));
                line_table->SetValue(i.template get<0>(), 0, pvec[0]);
                line_table->SetValue(i.template get<0>(), 1, pvec[1]);
            }
            return line_table;
        }
    )
    | ::to_vector
    ;
}

auto ellipse_plot(
    cv::ScatterPlot& sp
    , const std::vector<cv::VtkTablePtr>& table
)
{
    sp.show_legend = false;
    sp.initialize();
    std::vector<cv::ScatterPlot::PlotConf> plot_confs {
        cv::ScatterPlot::PointConf {
            vtkPlotLine::NONE      // point style
            , 255, 0, 0, 225              // color
            , 3.0                       // point width 
        }
        , cv::ScatterPlot::PointConf {
            vtkPlotLine::NONE       // point style
            , 0, 255, 0, 225              // color
            , 3.0                       // point width 
        }
        , cv::ScatterPlot::PointConf {
            vtkPlotLine::NONE       // point style
            , 0, 0, 255, 225              // color
            , 3.0                       // point width 
        }
    };
    for ( auto& i : {0, 1, 2 } )
    {
        sp.add_line(table[i], 0, 1, plot_confs[i]);
    }
    // return sp;
}
#endif
