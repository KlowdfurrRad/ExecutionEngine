cmake_minimum_required(VERSION 3.16)
project(TradingPlatform VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread chrono)
find_package(PkgConfig REQUIRED)

# Find libpqxx for PostgreSQL
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

# Find hiredis for Redis
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIBRARY hiredis)

# Find libcurl
find_package(CURL REQUIRED)

# Source files
set(SOURCES
    src/pricing_engine.cpp
    src/market_data_manager.cpp
    src/order_manager.cpp
    src/risk_manager.cpp
    src/zerodha_connector.cpp
    src/database_manager.cpp
    src/web_server.cpp
    src/main.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${LIBPQXX_LIBRARIES}
    ${HIREDIS_LIBRARY}
    ${CURL_LIBRARIES}
    Threads::Threads
)

# Include directories for dependencies
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${LIBPQXX_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${LIBPQXX_CFLAGS_OTHER}
)

# Testing
enable_testing()
find_package(GTest QUIET)
if(GTest_FOUND)
    set(TEST_SOURCES
        tests/test_pricing_engine.cpp
        tests/test_market_data.cpp
        tests/test_order_manager.cpp
    )
    
    add_executable(tests ${TEST_SOURCES} src/pricing_engine.cpp)
    target_link_libraries(tests 
        ${Boost_LIBRARIES}
        ${LIBPQXX_LIBRARIES}
        ${HIREDIS_LIBRARY}
        ${CURL_LIBRARIES}
        GTest::gtest_main
        Threads::Threads
    )
    
    target_include_directories(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        ${LIBPQXX_INCLUDE_DIRS}
        ${HIREDIS_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
    )
    
    add_test(NAME TradingPlatformTests COMMAND tests)
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Copy configuration files
install(FILES config/trading.conf
    DESTINATION etc/${PROJECT_NAME}
)