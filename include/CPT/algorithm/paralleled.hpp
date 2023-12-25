#pragma once
#include <CPT/utility/typecheck.hpp>
#include <CPT/range/segment.hpp>
#include <CPT/logger.hpp>
#include <thread>
namespace cpt {
namespace algorithm {
namespace cr = cpt::range;
template<class POOL>
struct ParallelEval
{
    POOL pool;
    const std::size_t pack_size;
};
CREATE_TYPECHECKER(ParallelEval);
template<class POOL, class PACK_RUN>
struct ParallelEval2
{
    POOL pool;
    const std::size_t pack_size;
    PACK_RUN pack_run;

    // ParallelEval2( POOL&& pool, std::size_t psize, PACK_RUN&& pr )
    // : ParallelEval<POOL>{ pool, std::move(psize) }
    // , pack_run ( std::forward<PACK_RUN>(pr) )
    // {}
};
CREATE_TYPECHECKER(ParallelEval2);
template<class POOL>
auto parallel_eval ( POOL&& pool, std::size_t pack_size )
{
    return ParallelEval<POOL> { std::forward<POOL> ( pool ), std::move(pack_size) };
}
template<class POOL, class PACK_RUN>
auto parallel_eval2 ( POOL&& pool, std::size_t pack_size, PACK_RUN&& pack_run )
{
    return ParallelEval2<POOL, PACK_RUN> { 
          std::forward<POOL> ( pool )
        , std::move(pack_size)
        , std::forward<PACK_RUN>(pack_run) 
    };
}
template<class RNG, class PAR, FTP_TYPE_CHECK(PAR, ParallelEval)>
auto operator| ( RNG&& rng, PAR&& pe )
{
    std::vector<std::size_t> jlist;
    for( auto&& pack : std::forward<RNG>(rng) | cr::segmented(pe.pack_size))
    {
        jlist.push_back(
            pe.pool.JobPost(
                [p = cu::mms(std::move(pack))] ( )
                {
                    p.storage | ::endp;
                }
            )
        );

    }
    for ( auto&& i : jlist ) pe.pool.FlushOne (i);
}
/* TODO parallel iterator range */
template<class RNG, class PAR, FTP_TYPE_CHECK(PAR, ParallelEval2)>
auto operator| ( RNG&& rng, PAR&& pe )
{
    std::vector<std::size_t> jlist;
    for( auto&& pack : std::forward<RNG>(rng) | cr::segmented(pe.pack_size))
    {
        jlist.push_back(
            pe.pool.JobPost(
                [p = cu::mms(std::move(pack)), &pe] ( )
                {
                    cpt::dbg 
                        << "thread# : " << std::this_thread::get_id() 
                        << "\ttake job addr : " << &(p.storage) << std::endl;
                    pe.pack_run(p.storage | ::to_vector);
                    cpt::dbg 
                        << "thread# : " << std::this_thread::get_id() 
                        << "\tfinish job addr: "    << &(p.storage) << std::endl;
                }
            )
        );

    }
    // pe.pool.flush();
    for ( auto&& i : jlist ) pe.pool.FlushOne(i);
}
}}
