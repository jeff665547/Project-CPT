#pragma once
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
// #include <Nucleona/parallel/thread_pool.hpp>
#include <Pokemon/thread_pool.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class ParallelThreadPool
{
  public:
    std::unique_ptr<pokemon::ThreadPool> thread_pool;
    
    ParallelThreadPool(unsigned num_threads)
    : thread_pool(new pokemon::ThreadPool(num_threads))
    {}

    ParallelThreadPool()
    : thread_pool(new pokemon::ThreadPool())
    {}

    DISABLE_COPY(ParallelThreadPool);
    DEFAULT_MOVE(ParallelThreadPool);

    ~ParallelThreadPool(void)
    {
        // thread_pool->join_pool();
        thread_pool.reset();
    }
};

} // namespace data_pool
} // namespace engine
} // namespace cpt
