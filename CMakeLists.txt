cmake_minimum_required(VERSION 3.20)
project(quantitative-platform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt needs AUTOMOC for Q_OBJECT macro
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---- FetchContent ----
include(FetchContent)

# SQLiteCpp (header-only C++ wrapper around SQLite3)
FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG        3.3.3
)
# Disable SQLiteCpp tests and samples
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SQLITECPP_INTERNAL_SQLITE ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SQLiteCpp)

# ---- Qt6 ----
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SvgWidgets PrintSupport)

# ---- Options ----
option(BUILD_TESTS "Build unit tests" ON)

# ---- Source tree ----
add_subdirectory(src)

# ---- Tests ----
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
