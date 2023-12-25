#pragma once
#include <mlpack/methods/gmm/gmm.hpp>
#include <CPT/visualization/vtk_util.hpp>
#include <CPT/visualization/view.hpp>
#include <Nucleona/type_traits/core.hpp>
#include <boost/proto/traits.hpp>
#include <CPT/visualization/geometrics/ellipse.hpp>
#include <vtkVariantArray.h>
#include <CPT/range.hpp>
#include <CPT/utility/irange.hpp>
#include <CPT/algorithm/mdl_components.hpp>
#include <cmath>
#include <vtkPlotLine.h>
#include <boost/range/combine.hpp>
#include <CPT/range.hpp>
#include <CPT/utility/irange.hpp>
#include <boost/range/adaptors.hpp>
#include <CPT/algorithm/acc_count.hpp>
#include <boost/range/algorithm/max_element.hpp>
namespace cpt {
namespace visualization {
namespace cv = cpt::visualization;
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace cvg = cv::geometrics;
namespace ca = cpt::algorithm;
namespace bt = boost;
namespace ba = boost::adaptors;
namespace cu = cpt::utility;
namespace table_detail {
    template<class T>
    struct MakeCol
    {
        // static auto run()
        // {}
    };
    #define MAKE_COL( BT, sym ) \
    template<> \
    struct MakeCol<BT> \
    { \
        template<class LAB1> \
        inline static auto run(LAB1&& lab1) \
        { \
            cv::Vtk ## sym ## Ptr arr = cv::Vtk ## sym ## Ptr::New(); \
            arr->SetName(std::forward<LAB1>(lab1)); \
            return arr; \
        } \
    };
    MAKE_COL(float              , FloatArray                );
    MAKE_COL(double             , DoubleArray                );
    MAKE_COL(int                , IntArray                  );
    MAKE_COL(long               , LongArray                 );
    MAKE_COL(long long          , LongLongArray             );
    MAKE_COL(short              , ShortArray                );
    MAKE_COL(signed char        , SignedCharArray           );
    MAKE_COL(unsigned char      , UnsignedCharArray         );
    MAKE_COL(unsigned int       , UnsignedIntArray          );
    MAKE_COL(unsigned long      , UnsignedLongArray         );
    MAKE_COL(unsigned long long , UnsignedLongLongArray     );
    MAKE_COL(unsigned short     , UnsignedShortArray        );
    MAKE_COL(std::string        , StringArray               );
    #undef MAKE_COL

    template<class COL1, class... COLS>
    struct TableAddCol
    {
        template<class TAB, class LAB1, class... LABS>
        inline static auto run(TAB& table, LAB1&& lab1, LABS&&... labs)
        {
            table->AddColumn(MakeCol<COL1>::run(
                std::forward<LAB1>(lab1)
            ));
            return TableAddCol<COLS...>::run(
                  table
                , std::forward<LABS>(labs)...
            );
        }
    };

    template<class COL>
    struct TableAddCol<COL>
    {
        template<class TAB, class LAB1>
        inline static auto run(TAB& table, LAB1&& lab1)
        {
            return table->AddColumn(
                MakeCol<COL>::run(
                    std::forward<LAB1>(lab1)
                )
            );
        }
    };
}
template<class... COL, class... LAB >
auto make_vtk_table( LAB&&... lab )
{
    auto table ( cv::VtkTablePtr::New() );
    table_detail::TableAddCol<COL...>::run( table, std::forward<LAB>(lab)... );
    return table;
}
auto clustermatrix_to_vtk_table( 
      const arma::mat&              sample
    , const arma::Row<std::size_t>& labels 
)
{
    std::vector<cv::VtkTablePtr> res;

    cu::irange_0((*bt::max_element(labels)) + 1)
    | ba::transformed(
        [&res](auto&& i)
        {
            res.emplace_back(
                cv::make_vtk_table<double, double>("a_allele", "b_allele")
            );
            return i;
        }
    )
    | endp;

    auto scol = sample | ca::col_range();
    for(auto&& ccol : bt::combine(scol, labels))
    {
        auto row = vtkVariantArray::New();
        row->Resize(2);
        row->InsertValue(0, ccol.get<0>()[0]);
        row->InsertValue(1, ccol.get<0>()[1]);
        res.at(ccol.get<1>())->InsertNextRow(row);
    }
    return res;
}
auto samplematrix_to_vtk_table( 
      const arma::mat&              sample
)
{
    std::vector<cv::VtkTablePtr> res {
          cv::make_vtk_table<double, double>("a_allele", "b_allele")
    };

    auto scol = sample | ca::col_range();
    for(auto&& col : scol)
    {
        auto row = vtkVariantArray::New();
        row->Resize(2);
        row->InsertValue(0, col[0]);
        row->InsertValue(1, col[1]);
        res.at(0)->InsertNextRow(row);
    }
    return res;
}
auto model_to_vtktable ( const md::GaussianDistribution& gd, const std::size_t& polygon_points_num = 1000 )
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
            return model_to_vtktable ( gd, polygon_points_num );
            // auto ef( cvg::ellipse_gd( gd ) );
            // auto line_table(cv::make_vtk_table<float, float>("x", "y"));
            // line_table->SetNumberOfRows(polygon_points_num);
            // for ( auto i : bt::combine(
            //       cu::irange_0(polygon_points_num)
            //     , arma::linspace<arma::vec>(0.0, 2*M_PI, polygon_points_num)
            // ) )
            // {
            //     auto pvec(ef(i.template get<1>()));
            //     line_table->SetValue(i.template get<0>(), 0, pvec[0]);
            //     line_table->SetValue(i.template get<0>(), 1, pvec[1]);
            // }
            // return line_table;
        }
    )
    | ::to_vector
    ;
}

}}
