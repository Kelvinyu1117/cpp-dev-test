add_executable(TCPServer TCPServer.cpp)

target_include_directories(
    TCPServer 
    PUBLIC ${asio_SOURCE_DIR}
)

target_link_libraries(
    TCPServer
    PUBLIC asio
)
