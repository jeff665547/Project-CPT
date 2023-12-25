#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <CPT/utility/sort.hpp>
#include <CPT/logger.hpp>
#include <boost/serialization/access.hpp>
namespace cpt {
namespace format {

class CubeAxis
{
    std::string axis;
    std::string axis_name;

    std::vector< std::string > labels;
    std::vector< size_t > labels_index;

    std::vector< std::string >::iterator labels_it_;
    std::vector< size_t >::iterator labels_index_it_;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & axis           ;
        ar & axis_name      ;
        ar & labels         ;
        ar & labels_index   ;
    }

  public:

    CubeAxis()
        : axis()
        , axis_name()
        , labels()
        , labels_index()
    {}

    CubeAxis( const CubeAxis& cube_axis )
        : axis( cube_axis.axis )
        , axis_name( cube_axis.axis_name )
        , labels( cube_axis.labels )
        , labels_index( cube_axis.labels_index )
    {}

    CubeAxis& operator=( const CubeAxis& cube_axis )
    {
        axis = cube_axis.axis;
        axis_name = cube_axis.axis_name;
        labels = cube_axis.labels;
        labels_index = cube_axis.labels_index;
        return *this;
    }

    CubeAxis( CubeAxis&& cube_axis )
        : axis( std::move( cube_axis.axis ))
        , axis_name( std::move( cube_axis.axis_name ))
        , labels( std::move( cube_axis.labels ))
        , labels_index( std::move( cube_axis.labels_index ))
    {}

    CubeAxis& operator=( CubeAxis&& cube_axis )
    {
        axis = std::move( cube_axis.axis );
        axis_name = std::move( cube_axis.axis_name );
        labels = std::move( cube_axis.labels );
        labels_index = std::move( cube_axis.labels_index );
        return *this;
    }

    bool operator==( const CubeAxis& cube_axis ) const
    {
        bool check( true );

        if( axis != cube_axis.axis )
        {
            check = false;
        }

        if( axis_name != cube_axis.axis_name )
        {
            check = false;
        }

        if( labels != cube_axis.labels )
        {
            check = false;
        }

        if( labels_index != cube_axis.labels_index )
        {
            check = false;
        }

        return check;
    }

    bool operator!=( const CubeAxis& cube_axis ) const
    {
        bool check( true );

        if( axis == cube_axis.axis )
        {
            check = false;
        }

        if( axis_name == cube_axis.axis_name )
        {
            check = false;
        }

        if( labels == cube_axis.labels )
        {
            check = false;
        }

        if( labels_index == cube_axis.labels_index )
        {
            check = false;
        }

        return check;
    }

    /*
     *  set the axis
     */
    void set_axis( const std::string ax )
    {
        /*
        if( axis != "x" && axis != "y" && axis != "z" )
        {
            std::cerr << "Error function using in \" set_axis \" of \" CubeAxis \"\n";
            std::cerr << "The axis must be set with \" x / y / z \" only\n";
            std::cerr << "But your axis is \" " << axis << " \"\n";
            abort();
        }
        */

        axis = ax;
    }

    /*
     *  get the axis
     */
    const std::string& get_axis() const
    {
        return axis;
    }

    /*
     *  print the axis
     */
    template<class T>
    void print_axis( T& print = cpt::msg )
    {
        print << axis;
    }

    /*
     *  set the axis name
     */
    void set_axis_name( const std::string name )
    {
        axis_name = name;
    }

    /*
     *  get the axis name
     */
    const std::string& get_axis_name() const
    {
        return axis_name;
    }

    /*
     *  print the axis name
     */
    template<class T>
    void print_axis_name( T& print = cpt::msg )
    {
        print << axis_name;
    }

    /*
     *  setup labels name
     */
    void set_labels( const std::vector< std::string >& lbs )
    {
        labels = std::move( lbs );
        labels_index = cpt::utility::sort( labels );
    }

    /*
     *  get the axis labels
     */
    const std::vector< std::string > get_labels() const
    {
        std::vector< std::string > temp_vec( labels_index.size(), "" );

        for( int i = 0; i < labels_index.size(); ++i )
        {
            temp_vec[ labels_index[i] ] = labels[i];
        }

        return temp_vec;
    }

    /*
     *  print the axis labels
     */
    template<class T>
    void print_labels( T& print = cpt::msg )
    {
        print << axis_name << " :";

        std::vector< std::string > temp_vec( labels_index.size(), "" );

        for( int i = 0; i < labels_index.size(); ++i )
        {
            temp_vec[ labels_index[i] ] = labels[i];
        }

        for( auto& val : temp_vec )
        {
            print << "\t" << val;
        }
    }

    /*
     *  reorder labels name by the labels index
     */
    void reorder_labels_name()
    {
        std::vector< std::string > temp_vec( labels_index.size(), "" );

        for( int i = 0; i < labels_index.size(); ++i )
        {
            temp_vec[ labels_index[i] ] = std::move( labels[i] );
        }

        std::vector< size_t > idx( labels_index.size() );
        std::iota( idx.begin(), idx.end(), 0 );

        labels_index.swap( idx );
        labels.swap( temp_vec );
    }

    /*
     *  search label by name and return the position
     */
    int search_label( const std::string& name )
    {
        labels_it_ = std::lower_bound( labels.begin(), labels.end(), name );

        if( labels_it_ == labels.end() )
        {
            return -1;
        }
        else
        {
            return labels_index[ labels_it_ - labels.begin() ];
        }
    }

    /*
     *  search label by position and return the name
     */
    std::string& search_label_at( const size_t& pos )
    {
        labels_index_it_ = std::find( labels_index.begin(), labels_index.end(), pos );

        if( labels_index_it_ != labels_index.end() )
        {
            return labels[ labels_index_it_ - labels_index.begin() ];
        }
        else
        {
            throw std::runtime_error( "out of range of axis search label at pos \"" + std::to_string( pos ) + "\"" );
        }
    }

    std::string& operator[]( const size_t& idx )
    {
        return this->search_label_at( idx );
    }


    /*
     *  replace label by name
     */
    void replace_label( const size_t& x, const std::string& name )
    {
        this->reorder_labels_name();
        labels[ x ] = name;
        labels_index = cpt::utility::sort( labels );
    }

    ~CubeAxis()
    {}

    /*
     *  swap the Cube Axis
     */
    void swap( CubeAxis& cube_axis )
    {
        axis.swap( cube_axis.axis );
        axis_name.swap( cube_axis.axis_name );
        labels.swap( cube_axis.labels );
        labels_index.swap( cube_axis.labels_index );
    }
};

} // end of namespace format
} // end of namespace cpt
