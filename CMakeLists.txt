cmake_minimum_required(VERSION 3.8.0)
project(MyCoro VERSION 0.1.0 LANGUAGES C CXX)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(CheckIncludeFileCXX)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    if(WIN32)
        check_cxx_compiler_flag("/std:c++latest"            support_latest)
        check_cxx_compiler_flag("/clang:-fcoroutines-ts"    support_coroutine)
        check_include_file_cxx("experimental/coroutine" has_coroutine_ts
            "/std:c++latest"
        )
    else()
        check_cxx_compiler_flag("-std=c++2a"          support_latest)
        check_cxx_compiler_flag("-fcoroutines-ts"     support_coroutine)
        check_include_file_cxx("experimental/coroutine" has_coroutine_ts
            "-std=c++2a"
        )
    endif()

elseif(MSVC)
    #
    # Notice that `/std:c++latest` and `/await` is exclusive to each other.
    # With MSVC, we have to distinguish Coroutines TS & C++ 20 Coroutines
    #
    check_cxx_compiler_flag("/std:c++latest"    support_latest)
    check_cxx_compiler_flag("/await"            support_coroutine)
    check_include_file_cxx("coroutine"  has_coroutine
        "/std:c++latest"
    )
    if(NOT has_coroutine)
        message(STATUS "Try <expeirmental/coroutine> (Coroutines TS) instead of <coroutine> ...")
        check_include_file_cxx("experimental/coroutine" has_coroutine_ts
            "/std:c++17"
        )
    endif()
    # has coroutine headers?
    if(NOT has_coroutine AND NOT has_coroutine_ts)
        message(FATAL_ERROR "There are no headers for C++ Coroutines")
    endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    #
    # expect GCC 10 or later
    #
    check_cxx_compiler_flag("-std=gnu++20"        support_latest)
    check_cxx_compiler_flag("-fcoroutines"        support_coroutine)
    check_include_file_cxx("coroutine" has_coroutine
        "-std=gnu++20 -fcoroutines"
    )
    if(APPLE)
        # -isysroot "/usr/local/Cellar/gcc/${CMAKE_CXX_COMPILER_VERSION}/include/c++/${CMAKE_CXX_COMPILER_VERSION}"
        # -isysroot "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include"
        # -isysroot "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1"
    endif()

    if(has_coroutine)
        set(try_compile_flags "-fcoroutines")
    endif()
endif()
# support compiler options for coroutine?
if(NOT support_coroutine)
    message(FATAL_ERROR "The compiler doesn't support C++ Coroutines")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${try_compile_flags}")

add_executable(cctest main.cpp)