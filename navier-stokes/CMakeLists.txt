# The minimum required cmake version
cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME app)

# Include the cmake variables with values for installation directories
include(GNUInstallDirs)

# autodiff requires a c++17 enabled compiler
set(CMAKE_CXX_STANDARD 17)
set(LIB_DIR /home/aswin/anaconda3/envs/enscalo_test/lib/)

set(LIB 
    -lmkl_core 
    -lmkl
)
set(INC_DIR 
    /usr/include
    /opt/intel/compilers_and_libraries_2020.0.166/linux/mkl/include/
)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../autodiff ${INC_DIR})
link_directories( ${LIB_DIR} )

add_executable ( ${TARGET_NAME} main.cc )
target_link_libraries(${TARGET_NAME} ${LIB})
# target_include_directories(${TARGET_NAME} ${INC_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../autodiff)
