#pragma once
#include <iostream>
#include <CPT/logger/logger_engine.hpp>

namespace cpt {
#ifdef LOGGER_OPEN_CHANNEL
typedef cpt::logger::LoggerEngine<LOGGER_OPEN_CHANNEL> LoggerGen;
#else
typedef cpt::logger::LoggerEngine<0,1,2,3,4> LoggerGen;
#endif

auto logerr0 = LoggerGen::create<0>(std::cerr);
auto logerr1 = LoggerGen::create<1>(std::cerr);
auto logerr2 = LoggerGen::create<2>(std::cerr);
auto logerr3 = LoggerGen::create<3>(std::cerr);
auto logerr4 = LoggerGen::create<4>(std::cerr);

auto logout0 = LoggerGen::create<0>(std::cout); // msg
auto logout1 = LoggerGen::create<1>(std::cout); // info
auto logout2 = LoggerGen::create<2>(std::cout); // warnning
auto logout3 = LoggerGen::create<3>(std::cout); // debug
auto logout4 = LoggerGen::create<4>(std::cout); // fatal error
auto logout5 = LoggerGen::create<5>(std::cout); // other channel 
auto logout6 = LoggerGen::create<6>(std::cout); // other channel 
auto logout7 = LoggerGen::create<7>(std::cout); // other channel 

auto& msg = logout0;
auto& info = logout1;
auto& warn = logout2;
auto& dbg = logout3;
auto& fatal = logout4;
auto& verbose0 = logout5;
auto& verbose1 = logout6;
auto& verbose2 = logout7;

#define CRASH_LOC logerr3 << __FILE__ << " : " << __LINE__ << '\n'; abort();
} // cpt
