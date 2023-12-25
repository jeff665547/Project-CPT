#pragma once
#include <CPT/forward.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/format/tsv.hpp>
#include <functional>
#include <map>
#include <CPT/logger.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class SharedObjectManager
{
  private:
    template <class T>
    struct Deleter
    {
        void operator()(T*& ptr) const
        {
            cpt::dbg << "Release an object"
                      << " at " << static_cast<void*>(ptr) << std::endl;
            delete ptr;
            ptr = nullptr;
        }
    };

  private:
    std::vector<std::function<void(void)>> vec_;

  public:
    template <class T>
    auto require_shared(const std::string& name)
    {
        static std::map<std::string, std::shared_ptr<T>> objects;
        if (objects.empty())
            vec_.emplace_back([&]{ objects.clear(); });
        
        auto& ptr = objects[name];
        if (!ptr)
        {
            ptr.reset(new T, Deleter<T>());
            cpt::dbg << "Create an object named " << name
                      << " at " << static_cast<void*>(ptr.get()) << std::endl;
        }
        return ptr;
    }
    void start_shared_object_management(void)
    {
        for (auto& run: vec_)
            run();
    }
};

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
