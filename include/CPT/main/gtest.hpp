#pragma once 
#include <gtest/gtest.h>
#include <CPT/fs/executable_dir.hpp>
int g_argc;
const char** g_argv;
int main( int argc, char** argv )
{
    ::testing::InitGoogleTest(&argc, argv);
    g_argc = argc;
    g_argv = (const char**)argv;
    cpt::fs::set_program_self( argv[0] );
    return RUN_ALL_TESTS();
}

