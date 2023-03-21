
#include "asio/detail/throw_error.hpp"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include "flatbuffers/minireflect.h"
#include "messaging/flatbuffers/codec/PropertyTreeCodec.hpp"
#include "messaging/flatbuffers/header/Header.hpp"
#include "messaging/flatbuffers/messages/autogen/PropertyTree_generated.h"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/redirect_error.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <random>
#include <unistd.h>
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;
namespace this_coro = asio::this_coro;
using namespace messaging::fbs::codec;

class TCPServer {
public:
  TCPServer(size_t port_number, const char *reflection_binary_file)
      : port_number(port_number), codec(reflection_binary_file) {}
  void start() {
    asio::io_context io_context(1);

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    co_spawn(io_context, run(), detached);

    io_context.run();
  }

private:
  awaitable<void> send_msg(tcp::socket socket) {
    // assume there is a mock property tree
    mock::PropTree propTree{{{mock::PropType::INT,
                              "a0",
                              "0",
                              {{mock::PropType::INT, "a1", "0"},
                               {mock::PropType::DOUBLE, "a1", "3.14"},
                               {mock::PropType::STRING, "a1", "Hello"}}}}};

    uint8_t write_buffer[100000];
    for (;;) {
      auto bin = codec.encode(propTree);
      messaging::Header header{bin.size()};
      std::cout << "publish - " << header.body_len << " bytes: \n"
                << flatbuffers::FlatBufferToString(
                       bin.data(), Property::PropTreeTypeTable())
                << '\n';
      // copy header into the buffer
      std::memcpy(write_buffer, &header, HEADER_SIZE);
      std::memcpy(write_buffer + HEADER_SIZE, bin.data(), header.body_len);

      size_t n_bytes = HEADER_SIZE + header.body_len;

      asio::error_code ec;
      co_await async_write(socket, asio::buffer(&write_buffer, n_bytes),
                           asio::redirect_error(asio::use_awaitable, ec));

      codec.clear();
      if (ec) {
        asio::detail::throw_error(ec);
        co_return;
      }
    }

    co_return;
  }

  awaitable<void> run() {
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), port_number});
    for (;;) {
      tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
      co_spawn(executor, send_msg(std::move(socket)), detached);
    }
  }

private:
  asio::ip::port_type port_number;
  PropertyTreeCodec<CodecRole::CODEC> codec;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: TCP Server <port> <flatbuffer-bfbs-file> \n";
      return 1;
    }

    TCPServer client(std::atoi(argv[1]), argv[2]);
    client.start();
  } catch (std::exception &e) {
    std::printf("Exception: %s\n", e.what());
  }
}
