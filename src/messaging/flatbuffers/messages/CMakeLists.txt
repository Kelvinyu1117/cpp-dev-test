file(GLOB MESSAGE_HEADER_FILES autogen/*.h)

add_library(flatbuffers-messages INTERFACE)

target_include_directories(flatbuffers-messages INTERFACE 
    INTERFACE autogen/ 
    INTERFACE "${flatbuffers_SOURCE_DIR}/include"
)

install(FILES autogen/PropertyTree.bfbs DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
