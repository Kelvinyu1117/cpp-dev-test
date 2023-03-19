
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/redirect_error.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
#include <cstdio>

#include <iostream>
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;
namespace this_coro = asio::this_coro;

awaitable<bool> echo_once(tcp::socket &socket) {
  char data[128];

  asio::error_code ec;

  size_t n = co_await socket.async_read_some(
      asio::buffer(data), asio::redirect_error(asio::use_awaitable, ec));

  if (ec == asio::error::eof) {
    co_return false;
  }

  std::cout << "received "
            << "[nBytes: " << n << "]: ";

  for (size_t i = 0; i < n; i++) {
    std::cout << data[i];
  }

  co_await async_write(socket, asio::buffer(data, n), use_awaitable);
  co_return true;
}

awaitable<void> echo(tcp::socket socket) {
  try {
    for (;;) {
      // The asynchronous operations to echo a single chunk of data have been
      // refactored into a separate function. When this function is called, the
      // operations are still performed in the context of the current
      // coroutine, and the behaviour is functionally equivalent.
      bool success = co_await echo_once(socket);
      if (!success) {
        std::cout << "socket is closed\n";
        break;
      }
    }
  } catch (std::exception &e) {
    std::printf("echo Exception: %s\n", e.what());
  }
}

awaitable<void> listener() {
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, {tcp::v4(), 55555});
  for (;;) {
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    co_spawn(executor, echo(std::move(socket)), detached);
  }
}

int main() {
  try {
    asio::io_context io_context(1);

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    co_spawn(io_context, listener(), detached);

    io_context.run();
  } catch (std::exception &e) {
    std::printf("Exception: %s\n", e.what());
  }
}
