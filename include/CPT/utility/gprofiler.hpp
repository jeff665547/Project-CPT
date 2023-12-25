#pragma once

#ifdef PROFILER
#include <gperftools/profiler.h>
#endif

#include <set>
#include <string>
#include <stdexcept>
namespace cpt {
namespace utility {

struct GProfiler
{
    std::set<std::string> check;
    template<class STR>
    GProfiler(STR&& pfname)
    {
#ifdef PROFILER
        auto&& itr ( check.find( pfname ) ) ;
        if ( itr == check.end() )
        {
            check.emplace(pfname);
            ProfilerStart(pfname);
        }
        else
        {
            throw std::logic_error("profiler result file name confilct");
        }
#endif
    }
    ~GProfiler()
    {
#ifdef PROFILER
        ProfilerStop();
#endif
    }
};

}}
