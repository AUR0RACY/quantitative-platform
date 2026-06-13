# Core engine — no Qt dependency
add_library(qp_core STATIC
    core/types.hpp
    core/core.cpp
)
target_include_directories(qp_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Data layer — CTP, SQLite, data storage
add_library(qp_data STATIC
    data/data_layer.hpp
    data/data_layer.cpp
    data/database.hpp
    data/database.cpp
)
target_include_directories(qp_data PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qp_data PUBLIC qp_core SQLiteCpp)

# Engine layer — backtest + strategy infrastructure (no Qt)
add_library(qp_engine STATIC
    engine/strategy_interface.hpp
    engine/report.hpp
    engine/dll_loader.hpp
    engine/dll_loader.cpp
    engine/backtest_engine.hpp
    engine/backtest_engine.cpp
)
target_include_directories(qp_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qp_engine PUBLIC qp_core)

# UI layer — Qt application
add_executable(qp_app
    main.cpp
    ui/mainwindow.hpp
    ui/mainwindow.cpp
)
target_link_libraries(qp_app PRIVATE
    qp_core
    qp_data
    qp_engine
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::SvgWidgets
)
set_target_properties(qp_app PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "quant-platform"
    AUTOMOC ON
)

# ---- Strategy DLLs ----
add_subdirectory(${CMAKE_SOURCE_DIR}/strategies strategies_build)
