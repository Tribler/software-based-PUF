cmake_minimum_required(VERSION 2.8.2)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG master
    SOURCE_DIR "/Users/ades17/Documents/Thesis/software-based-Physical-Unclonable-Function/Source Code/code-coverage/PUF_code_coverage/cmake-build-debug/googletest-src"
    BINARY_DIR "/Users/ades17/Documents/Thesis/software-based-Physical-Unclonable-Function/Source Code/code-coverage/PUF_code_coverage/cmake-build-debug/googletest-build"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)
