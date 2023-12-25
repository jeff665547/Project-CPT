#include <CPT/algorithm/bic.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#ifndef PROJECT_ROOT
#define PROJECT_ROOT "./"
#endif
namespace ca = cpt::algorithm;
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace cf = cpt::format;
using SampleReaderTrait = cpt::format::TraitPrototype<
      std::tuple
    , cf::PMT<0, double>
    , cf::PMT<1, double>
>;
TEST(algorithm, bic_test)
{
    cf::TupleParser<SampleReaderTrait> sample_reader("\t");
    std::ifstream samplef( PROJECT_ROOT + std::string("/unit_test/data/gmm/bic/sample2.txt"));
    std::string line;
    arma::mat sample (2, 100);
    int i = 0;
    while( std::getline( samplef, line ) )
    {
        auto tup ( sample_reader(line) );
        sample.col(i)[0] = std::get<0>(tup);
        sample.col(i)[1] = std::get<1>(tup);
        i ++;
    }
    std::vector<md::GaussianDistribution> glist { 
        {
              arma::vec { 7.09971114,  4.60433914 }
            , arma::mat {
                  { 0.02127888,  0.00661509 }
                , { 0.00661509,  0.03328458 }
              }
        } 
        , {
              arma::vec { 6.12938855,  6.03324855 }
            , arma::mat {
                  { 0.54344043 , -0.21954522 }
                , { -0.21954522,  0.12135993 }
              }
        } 
    };
    mg::GMM mdl( glist, { 0.48997277,  0.51002723 } );
    std::cout << ca::bic( mdl, sample ) << std::endl;
}
