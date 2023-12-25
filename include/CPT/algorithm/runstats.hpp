#pragma once
#include <armadillo>
#include <utility>
#include <CPT/logger.hpp>
namespace cpt {
namespace algorithm {
struct RunStats
{
    std::size_t N {0};
    arma::vec mean;
    arma::mat temp;

    RunStats( std::size_t D )
    : N ( 0 )
    , mean ( D, arma::fill::zeros )
    , temp ( D, D, arma::fill::zeros)
    {}

    void push( const arma::vec& x )
    {
        N += 1;
        auto delta = x - mean;
        mean += delta / N;
        temp += delta * arma::trans(x - mean);
    }

    auto covariance( std::size_t ddof = 1 )
    {
        if ( N - ddof > 0 )
        {
            return ( temp / ( N - ddof ) ).eval();
        }
        else
        {
            std::cerr << "divide by zero" << std::endl;
            auto nan = arma::mat(temp.n_rows, temp.n_cols);
            nan.fill(std::numeric_limits<double>::quiet_NaN());
            return nan;
        }
        // if ( temp.n_cols <= 1 && N <= ddof) 
        // {
        //     std::cerr << "divide by zero" << std::endl;
        //     auto nan = arma::mat(temp.n_rows, temp.n_cols);
        //     nan.fill(std::numeric_limits<double>::quiet_NaN());
        //     return nan;
        // }
        // return ( temp / ( N - ddof ) ).eval();
    }
};
}}
