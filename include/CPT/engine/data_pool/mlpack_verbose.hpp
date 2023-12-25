#pragma once
#include <CPT/forward.hpp>
#include <mlpack/core/util/cli.hpp>
#include <mlpack/core/util/log.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class MLPackVerbose
{
  public:
    MLPackVerbose(void) 
    {
        // bool verbose = false;

        // if (verbose)
        // {
        //     const char* args[] = { "run", "--verbose" };
        //     mlpack::CLI::ParseCommandLine(
        //         sizeof(args)/sizeof(char*)
        //       , const_cast<char**>(args)
        //     );
        //     mlpack::Log::Info << "MLPack verbose is enabled\n";
        // }
        // else
        // {
        //     const char* args[] = { "run" };
        //     mlpack::CLI::ParseCommandLine(
        //         sizeof(args)/sizeof(char*)
        //       , const_cast<char**>(args)
        //     );
        //     cpt::msg << "MLPack verbose is disabled" << std::endl;
        // }
    }
};

} // namespace data_pool
} // namespace engine
} // namespace cpt
