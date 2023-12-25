#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/format/cube_axis.hpp>

TEST( cube_axis_test, print_test )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.print_axis( std::cout );
    std::cout << std::endl;

    axis.set_axis_name( "test" );
    axis.print_axis_name( std::cout );
    std::cout << std::endl;

    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));
    axis.print_labels( std::cout );
    std::cout << std::endl;
}

TEST( cube_axis_test, basic_test )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "get axis";

    if( axis.get_axis() == "x" )
    {
        std::cout << "\rget axis complete\n";
    }
    else
    {
        std::cout << "\rget axis fail\n";
    }

    std::cout << "get axis name";

    if( axis.get_axis_name() == "test" )
    {
        std::cout << "\rget axis name complete\n";
    }
    else
    {
        std::cout << "\rget axis name fail\n";
    }

    std::cout << "get labels";

    if( axis.get_labels() == std::vector< std::string >({ "D", "C", "A", "B" }))
    {
        std::cout << "\rget labels complete\n";
    }
    else
    {
        std::cout << "\rget labels fail\n";
    }
}

TEST( cube_axis_test, copy_constructor )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "copy constructor";

    cpt::format::CubeAxis axis_1( axis );

    if( axis == axis_1 )
    {
        std::cout << "\rcopy constructor complete\n";
    }
    else
    {
        std::cout << "\rcopy constructor fail\n";
    }

    std::cout << "copy assignment operator";

    cpt::format::CubeAxis axis_2 = axis;

    if( axis == axis_2 )
    {
        std::cout << "\rcopy assignment operator complete\n";
    }
    else
    {
        std::cout << "\rcopy assignment operator fail\n";
    }
}

TEST( cube_axis_test, move_constructor )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "move constructor";

    cpt::format::CubeAxis axis1( axis );
    cpt::format::CubeAxis axis2( axis );

    cpt::format::CubeAxis axis_1( std::move( axis1 ));

    if( axis == axis_1 )
    {
        std::cout << "\rmove constructor complete\n";
    }
    else
    {
        std::cout << "\rmove constructor fail\n";
    }

    std::cout << "swap";

    cpt::format::CubeAxis axis_2;
    axis_2.swap( axis2 );

    if( axis == axis_2 )
    {
        std::cout << "\rswap complete\n";
    }
    else
    {
        std::cout << "\rswap fail\n";
    }
}

TEST( cube_axis_test, search_function )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "search function";

    if( axis.search_label( "C" ) == 1 )
    {
        std::cout << "\rsearch function complete\n";
    }
    else
    {
        std::cout << "\rsearch function fail\n";
    }
}

TEST( cube_axis_test, search_label_at )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "search label at";

    if( axis.search_label_at( 1 ) == "C" )
    {
        std::cout << "\rsearch label at complete\n";
    }
    else
    {
        std::cout << "\rsearch label at fail\n";
    }
}

TEST( cube_axis_test, search_label )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "search label via []";
    axis[ 1 ] = "E";

    if( axis[ 1 ] == "E" )
    {
        std::cout << "\rsearch label via [] complete\n";
    }
    else
    {
        std::cout << "\rsearch label via [] fail\n";
    }
}

/*
TEST( cube_axis_test, replace_label )
{
    cpt::format::CubeAxis axis;

    axis.set_axis( "x" );
    axis.set_axis_name( "test" );
    axis.set_labels( std::vector< std::string >({ "D", "C", "A", "B" }));

    std::cout << "replace label";

    axis.replace_label( 2, "Z" );

    if( axis.search_label( "Z" ) == 2 )
    {
        std::cout << "\rreplace label complete\n";
    }
    else
    {
        std::cout << "\rreplace label fail\n";
    }
}
*/
