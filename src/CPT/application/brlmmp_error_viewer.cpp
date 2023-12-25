#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/visualization/table.hpp>
#include <CPT/visualization/scatter_plot.hpp>

namespace cv = cpt::visualization;

double STOD( std::string& string )
{
    std::istringstream s2d( string );
    double res;
    s2d >> res;
    return res;
}

std::string get_name( std::string& path )
{
    std::vector< std::string > vec1;
    boost::iter_split( vec1, path, boost::algorithm::first_finder( "/" ));

    std::vector< std::string > vec2;
    boost::iter_split( vec2, vec1[ vec1.size() -1 ], boost::algorithm::first_finder( "." ));

    return vec2[0];
}

std::vector< std::vector< double >> get_file_data( std::string& file_path )
{
    std::string line;
    std::ifstream file( file_path );
    std::vector< std::vector< double >> data;

    while( std::getline( file, line ))
    {
        std::vector< double > dvec;
        std::vector< std::string > vec;
        boost::iter_split( vec, line, boost::algorithm::first_finder( "\t" ));

        dvec.emplace_back( STOD( vec[0] ));
        dvec.emplace_back( STOD( vec[1] ));
        dvec.emplace_back( STOD( vec[3] ));
        dvec.emplace_back( STOD( vec[4] ));
        data.emplace_back( dvec );
    }

    file.close();
    return data;
}

int main( int argc, char** argv )
{
    if( argc != 2 )
        throw std::runtime_error( "./EXE FileList" );

    std::string file_path;
    std::string file_list = argv[1];
	std::ifstream list( file_list );

    while( std::getline( list, file_path ))
    {
        std::string probe_name = get_name( file_path );
        std::vector< std::vector< double >> data( get_file_data( file_path ));

        cv::ScatterPlot sp;
        sp.initialize();

        std::vector< std::vector< std::pair< double, double >>> datas{
              {}, {}, {}, {}, {}, {}, {}
        };

        std::vector< cv::VtkTablePtr > tables{
              cv::make_vtk_table< double, double >( "X Axis", ( probe_name ).c_str() ) // 0
            , cv::make_vtk_table< double, double >( "X Axis", "AA->AB" ) // 1
            , cv::make_vtk_table< double, double >( "X Axis", "AA->BB" ) // 2
            , cv::make_vtk_table< double, double >( "X Axis", "AB->AA" ) // 3
            , cv::make_vtk_table< double, double >( "X Axis", "AB->BB" ) // 4
            , cv::make_vtk_table< double, double >( "X Axis", "BB->AA" ) // 5
            , cv::make_vtk_table< double, double >( "X Axis", "BB->AB" ) // 6
        };

        for( size_t i = 0; i < data.size(); i++ )
        {
            switch( size_t( data[i][2] - data[i][3] + 2 ))
            {
                case 0: // BB -> AA
                    datas[5].emplace_back( data[i][0], data[i][1] );
                    break;

                case 1: // AB -> AA / BB -> AB
                    switch( size_t( data[i][2] ))
                    {
                        case 0: // BB -> AB
                            datas[6].emplace_back( data[i][0], data[i][1] );
                            break;

                        case 1: // AB -> AA
                            datas[3].emplace_back( data[i][0], data[i][1] );
                            break;
                    }
                    break;

                case 2: // Same Type
                    datas[0].emplace_back( data[i][0], data[i][1] );
                    break;

                case 3: // AA -> AB / AB -> BB
                    switch( size_t( data[i][2] ))
                    {
                        case 1: // AB -> BB
                            datas[4].emplace_back( data[i][0], data[i][1] );
                            break;

                        case 2: // AA -> AB
                            datas[1].emplace_back( data[i][0], data[i][1] );
                            break;
                    }
                    break;
                    
                case 4: // AA -> BB
                    datas[2].emplace_back( data[i][0], data[i][1] );
                    break;
            }
        }

        for( size_t i = 0; i < 7; i++ )
        {
            if( datas[i].empty() )
            {
                continue;
            }

            tables[i]->SetNumberOfRows( datas[i].size() );

            for( size_t j = 0; j < datas[i].size(); ++j )
            {
                tables[i]->SetValue( j, 0, datas[i][j].first );
                tables[i]->SetValue( j, 1, datas[i][j].second );
            }
        }

        for( size_t i = 0; i < 7; i++ )
        {
            if( datas[i].empty() )
            {
                continue;
            }

            switch( i )
            {
                case 0: // SameType
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::PLUS     // point style
                            , 0, 0, 0, 225          // color
                            , 1.0                   // point width 
                        }
                    );
                    break;

                case 1: // AA->AB
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::CIRCLE   // point style
                            , 31, 119, 180, 225     // color
                            , 1.0                   // point width 
                        }
                    );
                    break;

                case 2: // AA->BB
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::CIRCLE   // point style
                            , 255, 127, 14, 225     // color
                            , 1.0                   // point width 
                        }
                    );
                    break;

                case 3: // AB->AA
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::CIRCLE   // point style
                            , 44, 160, 44, 225      // color
                            , 1.0                   // point width 
                        }
                    );
                    break;

                case 4: // AB->BB
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::CIRCLE   // point style
                            , 214, 39, 40, 225      // color
                            , 1.0                   // point width 
                        }
                    );
                    break;

                case 5: // BB->AA
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::CIRCLE   // point style
                            , 148, 103, 189, 225    // color
                            , 1.0                   // point width 
                        }
                    );
                    break;

                case 6: // BB->AB
                    sp.add_plot( tables[i]
                        , 0 // x col in table
                        , 1 // y col in table
                        , cv::ScatterPlot::PointConf {
                            vtkPlotPoints::CIRCLE   // point style
                            , 140, 86, 75, 225      // color
                            , 1.0                   // point width 
                        }
                    );
                    break;
                }
            }

        sp.render_and_start();
    }

    list.close();
    return 0;
}
