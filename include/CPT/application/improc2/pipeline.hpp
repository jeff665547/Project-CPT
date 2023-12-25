#pragma once
#include <utility>
#include <type_traits>

namespace cpt {
namespace application {
namespace improc2 {

template <class STAGE, class... STAGES>
class Pipe
{
  public:
    template <class OBJ, class... ARGS>
    static OBJ run(OBJ&& obj, ARGS&&... args)
    {
        STAGE::run(obj, args...);
        return Pipe<STAGES...>::template run(
            std::forward<OBJ>(obj)
          , std::forward<ARGS>(args)...
        );
    }
};
template <class STAGE>
class Pipe<STAGE>
{
  public:
    template <class OBJ, class... ARGS>
    static OBJ run(OBJ&& obj, ARGS&&... args)
    {
        STAGE::run(obj, args...);
        return std::forward<OBJ>(obj);
    }
};

} // namespace improc
} // namespace application
} // namespace cpt
