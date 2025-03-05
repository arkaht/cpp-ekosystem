cmake_minimum_required(VERSION 3.11)

#  Setup Ekosystem
project(EKOSYSTEM)
set(CMAKE_CXX_STANDARD 20)
set(EKOSYSTEM_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EKOSYSTEM_SOURCE "${EKOSYSTEM_INCLUDE}/ekosystem")
set(EKOSYSTEM_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/assets")

add_compile_definitions(ENABLE_VISDEBUG)

#  Require Suprengine
set(SUPRENGINE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cpp-suprengine/" CACHE FILEPATH "Path to the suprengine project")
add_subdirectory("${SUPRENGINE_PATH}" "suprengine")

#  Find source files
file(GLOB_RECURSE EKOSYSTEM_SOURCES CONFIGURE_DEPENDS "${EKOSYSTEM_SOURCE}/*.cpp")

#  Declare Ekosystem project
add_executable(EKOSYSTEM)
set_target_properties(EKOSYSTEM PROPERTIES OUTPUT_NAME "ekosystem")
target_include_directories(EKOSYSTEM PRIVATE "${EKOSYSTEM_INCLUDE}")
target_sources(EKOSYSTEM PRIVATE "${EKOSYSTEM_SOURCES}")
target_link_libraries(EKOSYSTEM PRIVATE SUPRENGINE)

#  Setup install rules
#  Install executable
install(TARGETS EKOSYSTEM DESTINATION "bin")
#  Install assets
install(
	DIRECTORY "${EKOSYSTEM_ASSETS}/"
	DESTINATION "bin/assets/ekosystem"
	PATTERN "*.blend" EXCLUDE
	PATTERN "*.blend1" EXCLUDE
	PATTERN "*.afphoto" EXCLUDE
)
install(
	DIRECTORY "${SUPRENGINE_ASSETS}/"
	DESTINATION "bin/assets/suprengine"
	PATTERN "*.blend" EXCLUDE 
	PATTERN "*.blend1" EXCLUDE
	PATTERN "*.afphoto" EXCLUDE
)
#  Install DLLs
foreach(FILE ${SUPRENGINE_DLLS})
	install(FILES ${FILE} TYPE BIN)
endforeach()

#  Copy DLLs and assets
suprengine_copy_dlls(EKOSYSTEM)
suprengine_symlink_assets(EKOSYSTEM "ekosystem")