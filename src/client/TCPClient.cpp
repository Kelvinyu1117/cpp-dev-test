
#include "asio/awaitable.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include "flatbuffers/reflection.h"
#include "flatbuffers/table.h"
#include "messaging/flatbuffers/codec/PropertyTreeCodec.hpp"
#include "messaging/flatbuffers/header/Header.hpp"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/redirect_error.hpp>
#include <asio/signal_set.hpp>
#include <bit>
#include <cstdio>

#include <cstring>
#include <iostream>
#include <memory>
#include <span>

using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::ip::tcp;
namespace this_coro = asio::this_coro;
using namespace messaging::fbs::codec;

namespace {
void print_space_before(size_t cnt) {
  for (size_t i = 0; i < cnt * 4; i++) {
    std::cout << " ";
  }
}

void print_properties(
    const flatbuffers::Vector<::flatbuffers::Offset<Property::Prop>> *props_ptr,
    size_t cnt = 1) {

  using namespace Property;
  if (!props_ptr)
    return;

  print_space_before(cnt);
  std::cout << "{\n";
  for (auto it = props_ptr->begin(); it != props_ptr->end(); it++) {

    print_space_before(cnt + 1);
    std::cout << "name: " << flatbuffers::GetString(it->name()) << '\n';
    print_space_before(cnt + 1);
    std::cout << "name: " << flatbuffers::GetString(it->value()) << '\n';
    print_space_before(cnt + 1);
    std::cout << "type: " << EnumNamePropType(it->type()) << '\n';
    // print_properties(it->sub_properties());
    print_properties(it->sub_properties(), cnt + 1);
  }
  print_space_before(cnt);
  std::cout << "}\n";
}
} // namespace

class TCPClient {
public:
  TCPClient(size_t port_number, const char *reflection_binary_file)
      : port_number(port_number), codec(reflection_binary_file) {}

  void start() {
    asio::io_context io_context(1);

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    co_spawn(io_context, run(), detached);

    io_context.run();
  }

private:
  awaitable<void> parse_property_tree(std::span<uint8_t> msg) {

    // tried to use full reflection, but fail to get it work for modifying the
    // buffer.....
    flatbuffers::Table *table_ptr = nullptr;
    if (codec.decode({msg.begin(), msg.end()}, table_ptr)) {

      auto root_table = codec.get_reflection_root_table();
      assert(root_table);
      auto table_fields = root_table->fields();
      assert(table_fields);
      auto properties_field = table_fields->LookupByKey("properties");
      assert(properties_field);

      auto v = flatbuffers::GetFieldV<flatbuffers::Offset<Property::Prop>>(
          *table_ptr, *properties_field);

      print_properties(v);
    }

    co_return;
  }
  /**
   * @brief
   * Assume the message is the following structure:
   * [header: 8 bytes][body: variable length depends on the header]
   * @param socket
   * @return awaitable<void>
   */
  awaitable<void> read_msg(tcp::socket socket) {
    uint8_t buffer[10000];

    for (;;) {
      asio::error_code ec;
      // read the header first
      co_await asio::async_read(socket, asio::buffer(buffer, HEADER_SIZE),
                                asio::redirect_error(asio::use_awaitable, ec));

      if (!ec) {
        std::span<uint8_t, HEADER_SIZE> header_view{buffer, HEADER_SIZE};

        Header header;
        std::memcpy(&header, buffer, HEADER_SIZE);

        size_t body_len = header.body_len;
        // read the body
        co_await asio::async_read(
            socket, asio::buffer(buffer + HEADER_SIZE, body_len),
            asio::redirect_error(asio::use_awaitable, ec));
        if (!ec) {
          co_await parse_property_tree({buffer + HEADER_SIZE, body_len});
        } else {
          asio::detail::throw_error(ec);
          co_return;
        }
      } else {
        asio::detail::throw_error(ec);
        co_return;
      }
    }
    co_return;
  }

  awaitable<void> run() {
    auto executor = co_await this_coro::executor;
    tcp::socket socket(executor);

    co_await socket.async_connect({tcp::v4(), port_number}, use_awaitable);
    co_spawn(executor, read_msg(std::move(socket)), detached);
  }

private:
  asio::ip::port_type port_number;
  messaging::fbs::codec::PropertyTreeCodec<CodecRole::DECODER> codec;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: TCP Client <port> <flatbuffer-bfbs-file> \n";
      return 1;
    }

    TCPClient client(std::atoi(argv[1]), argv[2]);
    client.start();
  } catch (std::exception &e) {
    std::printf("Exception: %s\n", e.what());
  }
}
