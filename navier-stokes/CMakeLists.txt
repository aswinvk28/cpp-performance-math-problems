# The minimum required cmake version
cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME app)

# Include the cmake variables with values for installation directories
include(GNUInstallDirs)

list(APPEND SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/src/basic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/src/cmdparser.cpp
)

# autodiff requires a c++17 enabled compiler
set(CMAKE_CXX_STANDARD 17)
set(LIB_DIR 
    /usr/lib
    /usr/lib/x86_64-linux-gnu/
    /opt/intel/compilers_and_libraries_2018.2.199/linux/mkl/lib/intel64_lin/
)
set(LIB 
    -lmkl_core 
    -lomp5
)
set(INC_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/include
    /opt/intel/compilers_and_libraries_2018.2.199/linux/mkl/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../autodiff ${INC_DIR})
link_directories( ${LIB_DIR} )

set (CMAKE_CXX_FLAGS "-fopenmp -std=c++17")

add_executable ( ${TARGET_NAME} main.cc worker.cc series.cc ${SOURCES} )
target_link_libraries(${TARGET_NAME} ${LIB})
