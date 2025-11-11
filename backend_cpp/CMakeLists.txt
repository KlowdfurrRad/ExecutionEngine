cmake_minimum_required(VERSION 3.16)
project(CashFuturesTHV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Threads REQUIRED)

# Include directories for headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

# Add nlohmann/json
include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Fetch Crow framework
FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

# Create executable
add_executable(cash_futures_thv main.cpp)

# Link libraries
target_link_libraries(cash_futures_thv 
    PRIVATE 
    Crow::Crow
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# Compiler flags for optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(cash_futures_thv PRIVATE -O3 -march=native)
endif()

# Enable OpenMP for parallel processing if available
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(cash_futures_thv PRIVATE OpenMP::OpenMP_CXX)
endif()

# Platform specific configurations
if(WIN32)
    target_compile_definitions(cash_futures_thv PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(cash_futures_thv PRIVATE ws2_32 wsock32)
endif()

# Copy configuration files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)