#pragma once
#include <cstddef>
namespace cpt
{
namespace engine
{

class Sample
{
  public :
    virtual std::size_t num_samples ( void ) const = 0;
    virtual std::size_t num_probes ( void ) const = 0;
};

}
}
