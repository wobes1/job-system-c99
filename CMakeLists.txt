cmake_minimum_required(VERSION 3.5)
project(job_system LANGUAGES C)

# include utilities for compiler options and warnings

include(cmake/target_default_compile_warnings.cmake)
include(cmake/target_default_compile_options.cmake)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})

include_directories(src)

file(GLOB jobs_src CONFIGURE_DEPENDS "src/*.h" "src/*.c")
add_library(jobs SHARED ${jobs_src})

add_executable(main main.c)
target_link_libraries(main jobs)