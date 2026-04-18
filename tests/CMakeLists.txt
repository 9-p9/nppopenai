cmake_minimum_required(VERSION 3.14)
project(NppOpenAI_Tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Source root and include directories ----
set(SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(JSON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vs.proj/include")

# ---- Production source files under test ----
# Only the pure-logic modules that have no link-time Windows dependencies.
set(SOURCES_UNDER_TEST
    "${SRC_DIR}/api/ResponseParsers.cpp"
    "${SRC_DIR}/api/StreamParser.cpp"
    "${SRC_DIR}/api/RequestFormatters.cpp"
    "${SRC_DIR}/utils/EncodingUtils.cpp"
)

# ---- Test sources ----
set(TEST_SOURCES
    test_main.cpp
    test_ResponseParsers.cpp
    test_StreamParser.cpp
    test_RequestFormatters.cpp
    stubs/globals_stub.cpp
)

add_executable(npp_tests ${TEST_SOURCES} ${SOURCES_UNDER_TEST})

# ---- Include paths (order matters: stubs must shadow real Windows headers) ----
target_include_directories(npp_tests PRIVATE
    # 1. Stub headers shadow <windows.h> and the Windows dialog classes
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"
    # 2. Source root (gives access to src/api/, src/utils/, src/ui/ etc.)
    "${SRC_DIR}"
    # 3. Sub-directories referenced by #include "utils/EncodingUtils.h" etc.
    "${SRC_DIR}/core"
    "${SRC_DIR}/api"
    "${SRC_DIR}/utils"
    "${SRC_DIR}/npp"
    # 4. nlohmann/json header-only library
    "${JSON_DIR}"
    # 5. Test framework header lives in this directory
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# ---- Compiler options ----
if(MSVC)
    target_compile_options(npp_tests PRIVATE /W3 /wd4267 /wd4244)
else()
    target_compile_options(npp_tests PRIVATE
        -Wall -Wextra -Wno-unused-parameter
        # Silence deprecation of codecvt used in EncodingUtils.cpp
        -Wno-deprecated-declarations
    )
endif()

# ---- CTest integration ----
enable_testing()
add_test(NAME unit_tests COMMAND npp_tests)
