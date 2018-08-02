add_executable(iotest iotest.cpp)
target_link_libraries(iotest ${ASAN_LIB} common comanche-core numa dl z ${Nanomsg_LIBRARIES} boost_system boost_program_options pthread ${TBB_LIBRARIES} fuse)
