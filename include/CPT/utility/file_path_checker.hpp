#pragma once
#include <CPT/forward.hpp>
#include <CPT/logger.hpp>
#include <boost/filesystem.hpp>

namespace cpt {
namespace utility {

class FilePathChecker
{
  private:
    bool flag;

  public:
    FilePathChecker(void)
      : flag(true)
    {}
    void reset(void)
    {
        flag = true;
    }
    void operator()(const bfs::path& path)
    {
        if (bfs::exists(path))
        {
            cpt::msg << "found " << path << '\n';
        }
        else
        {
            cpt::fatal << path << " not found!\n";
            flag = false;
        }
        return;
    }
    void verify(void) const
    {
        if (!flag)
            exit(1);
    }
};

} // utility
} // namespace cpt
