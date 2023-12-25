#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/algorithm/g_m_m_util/access.hpp>
namespace mg = mlpack::distribution;
TEST( g_m_m, access )
{
    std::vector<mlpack::distribution::GaussianDistribution> gs_vec({
          mg::GaussianDistribution{ 
              arma::vec{ 0, 0 } 
            , arma::mat{ { 1, 0 }, { 0, 1 } }
          }
        , mg::GaussianDistribution{ 
              arma::vec{ 0, 0 }
            , arma::mat{ { 2, 0 }, { 0, 2 } }
          }
    });
    mlpack::gmm::GMM gmm(
          gs_vec
        , arma::vec { 0.7, 0.3 }
    );
    auto comps = cpt::algorithm::g_m_m_util::components( gmm );
    arma::vec mean0 { 0, 0 };
    std::cout << (comps[0].Mean() == mean0) << std::endl;
    // EXPECT_EQ( comps[0].Mean(), mean0 );
}
