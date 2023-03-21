add_executable(TCPClient TCPClient.cpp)

message(${flatbuffers_SOURCE_DIR})
target_include_directories(
    TCPClient 
    PUBLIC ${asio_SOURCE_DIR}
    PUBLIC "${flatbuffers_SOURCE_DIR}/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(
    TCPClient
    PUBLIC asio
    PUBLIC flatbuffers
)

install(TARGETS TCPClient)
