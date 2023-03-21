add_library(flatbuffers-codec STATIC
    PropertyTreeCodec.cpp
    PropertyTreeCodec.hpp
)

target_include_directories(
    flatbuffers-codec
    PUBLIC "${flatbuffers_SOURCE_DIR}/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(
    flatbuffers-codec
    PUBLIC flatbuffers
    PUBLIC flatbuffers-messages
    PUBLIC messaging-mock
)

