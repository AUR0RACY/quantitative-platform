# Strategy DLLs — each subfolder is a separate strategy DLL
# Strategies link against qp_engine (for IStrategy interface) and qp_core

# ---- SMA Crossover ----
add_library(sma_crossover SHARED
    sma_crossover/sma_crossover.hpp
    sma_crossover/sma_crossover.cpp
)
target_include_directories(sma_crossover PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(sma_crossover PRIVATE qp_engine qp_core)
set_target_properties(sma_crossover PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/strategies"
    PREFIX ""    # no "lib" prefix on Windows
)
