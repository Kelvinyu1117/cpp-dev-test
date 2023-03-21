add_executable(TCPServer TCPServer.cpp)

target_include_directories(
    TCPServer 
    PUBLIC ${asio_SOURCE_DIR}
    PUBLIC "${flatbuffers_SOURCE_DIR}/include"
    PUBLIC "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(
    TCPServer
    PUBLIC asio
    PUBLIC asio
    PUBLIC flatbuffers
)

install(TARGETS TCPServer)
