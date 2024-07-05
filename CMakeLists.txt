cmake_minimum_required(VERSION 3.11)

#  Setup Ekosystem
project(EKOSYSTEM)
set(CMAKE_CXX_STANDARD 20)
set(EKOSYSTEM_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EKOSYSTEM_SOURCE "${EKOSYSTEM_INCLUDE}/ekosystem")
set(EKOSYSTEM_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/assets")

#  Require Suprengine
set(SUPRENGINE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cpp-suprengine/" CACHE FILEPATH "Path to the suprengine project")
add_subdirectory("${SUPRENGINE_PATH}" "suprengine")

#  Find source files
file(GLOB_RECURSE EKOSYSTEM_SOURCES CONFIGURE_DEPENDS "${EKOSYSTEM_SOURCE}/*.cpp")

#  Declare test project
add_executable(EKOSYSTEM)
set_target_properties(EKOSYSTEM PROPERTIES OUTPUT_NAME "ekosystem")
target_include_directories(EKOSYSTEM PRIVATE "${EKOSYSTEM_INCLUDE}")
target_sources(EKOSYSTEM PRIVATE "${EKOSYSTEM_SOURCES}")
target_link_libraries(EKOSYSTEM PRIVATE SUPRENGINE)

#  Copy DLLs and assets
suprengine_copy_dlls(EKOSYSTEM)
suprengine_copy_assets(EKOSYSTEM "ekosystem")