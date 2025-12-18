# CMakeLists.txt for gr-recovery GNU Radio module

cmake_minimum_required(VERSION 3.8)
project(gr-recovery CXX C)

# Find GNU Radio
find_package(Gnuradio "3.8" REQUIRED COMPONENTS blocks)

if(NOT GNURADIO_RUNTIME_FOUND)
    message(FATAL_ERROR "GnuRadio Runtime required to build gr-recovery")
endif()

# Set up our paths
set(GR_RECOVERY_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GR_RECOVERY_SWIG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/swig)

# Find recovery libraries
find_library(RECOVERY_CORE_LIB recovery_core PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/recovery_core)
find_library(RECOVERY_SIGNAL_LIB recovery_signal PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/recovery_signal)

include_directories(
    ${GR_RECOVERY_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/recovery_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/recovery_signal/include
    ${GNURADIO_ALL_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(lib)
add_subdirectory(grc)

# Install header files
install(
    FILES
        include/recovery/signal_recovery_block.h
    DESTINATION include/recovery
)

# Create package config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/gr-recovery.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/gr-recovery.pc
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/gr-recovery.pc
    DESTINATION lib/pkgconfig
)