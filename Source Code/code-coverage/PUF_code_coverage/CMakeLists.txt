cmake_minimum_required(VERSION 3.9)
project(PUF_code_coverage)

set(GCC_GENERATE_TEST_COVERAGE_FILES NO)
set(GCC_INSTRUMENT_PROGRAM_FLOW_ARCS NO)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
        "${CMAKE_BINARY_DIR}/googletest-build")

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
endif()

include_directories(cmake-build-debug/googletest-src/googlemock/include)
include_directories(cmake-build-debug/googletest-src/googletest/include)

#set(SOURCE_FILES main.cpp "test/ClassToTest.h" "test/mainTest.cpp" test/BCHCompleteTest.cpp)
set(SOURCE_FILES main.cpp test/BCHCompleteTest.cpp  test/PUF_Test.cpp test/BCHTest.cpp test/ToolsTest.cpp src/PUF.h src/bch/BCH.cpp src/tools/Tools.cpp src/Crypto/AES256.cpp src/Crypto/Hash.cpp src/Crypto/SHA3.cpp src/Crypto/KeccakCore.cpp src/Crypto/AESCommon.cpp src/Crypto/BlockCipher.cpp)
add_executable(PUF_code_coverage ${SOURCE_FILES} )
target_link_libraries(PUF_code_coverage gtest gtest_main)

# gcov main.gcno && lcov --capture --directory . --output-file project_test.info && genhtml project_test.info --output-directory CODE_COVERAGE && open CODE_COVERAGE/index.html
# find . -name "*.gcda" -type f -delete
