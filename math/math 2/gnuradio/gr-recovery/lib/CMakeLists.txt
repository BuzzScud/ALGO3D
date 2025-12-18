# CMakeLists.txt for gr-recovery library

add_library(gnuradio-recovery SHARED
    signal_recovery_block_impl.cc
)

target_link_libraries(gnuradio-recovery
    ${GNURADIO_ALL_LIBRARIES}
    ${RECOVERY_CORE_LIB}
    ${RECOVERY_SIGNAL_LIB}
)

set_target_properties(gnuradio-recovery PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

install(TARGETS gnuradio-recovery
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)