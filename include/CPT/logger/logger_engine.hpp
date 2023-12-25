#pragma once
#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>
namespace cpt {
namespace logger {
class OpenLog
{
    std::ostream& out_;
public :
    OpenLog(std::ostream& out)
    : out_(out)
    {}
    std::ostream& get_out()
    {
        return out_;
    }
    auto& flush()
    {
        return out_.flush();
    }
};
template<class T>
OpenLog& operator<< (OpenLog& log, T&& x)
{
    using namespace std::chrono_literals;
    log.get_out() << std::forward<T>(x);
#ifdef SLOW_LOG
    std::this_thread::sleep_for( SLOW_LOG );
#endif
    return log;
}
typedef decltype(std::cout) CoutType;
typedef CoutType& (*StandardEndLine)(CoutType&);
OpenLog& operator<< (OpenLog& log, StandardEndLine manip)
{
    manip(log.get_out());
    return log;
}

class CloseLog
{
public:
    CloseLog(std::ostream& out)
    {}
    auto flush()
    {}
};
template<class T>
CloseLog& operator<< (CloseLog& log, T&& x)
{
    return log;
}
CloseLog& operator<< (CloseLog& log, StandardEndLine manip)
{
    return log;
}

template<bool ON>
struct LogSelect
{};

template<>
struct LogSelect<true>
{
    typedef OpenLog LogType;
};
template<>
struct LogSelect<false>
{
    typedef CloseLog LogType;
};
template<int N, int FIRST, int... TAIL>
struct IsIn
{
    static const bool result = ( (N == FIRST) || IsIn<N, TAIL...>::result );
};
template<int N, int LAST>
struct IsIn<N, LAST>
{
    static const bool result = (N == LAST);
};

template<int... TURN_ON_LEVELS>
class LoggerEngine
{
    template<bool ON>
    static typename LogSelect<ON>::LogType kernal_create(std::ostream& out)
    {
        typename LogSelect<ON>::LogType log(out);
        return log;
    }
public:
    template<int N>
    static auto create(std::ostream& out)
    {
        return kernal_create<IsIn<N, TURN_ON_LEVELS...>::result>(out);  
    }

};
}}
