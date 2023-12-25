#pragma once
namespace cpt{ namespace application{ namespace intensities_dump{


template<class T>
struct CompAdapt
{
    struct Parameters
    {
        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
        }
        template <class JSON>
        void config(JSON&& opts)
        {
        }
    };
  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        T::run(buf);
    }
};

}}}
