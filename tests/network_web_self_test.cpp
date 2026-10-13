#ifdef NDEBUG
#undef NDEBUG
#endif

#include <IPAddress.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <string>
#include <thread>

static int listener(uint16_t& port) {
  const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  assert(::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
  socklen_t size = sizeof(address);
  assert(::getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size) == 0);
  port = ntohs(address.sin_port);
  return fd;
}

static void waitFor(const std::atomic<bool>& ready) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!ready.load()) {
    assert(std::chrono::steady_clock::now() < deadline);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

static std::string request(uint16_t port, const char* text) {
  const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  timeval timeout{2, 0};
  ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(port);
  assert(::connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
  NetworkClient client(fd);
  Print& output = client;
  assert(output.print(text) == std::strlen(text));
  ::shutdown(fd, SHUT_WR);
  std::string response;
  char buffer[512];
  for (;;) {
    const auto count = ::recv(fd, buffer, sizeof(buffer), 0);
    assert(count >= 0);
    if (count == 0) break;
    response.append(buffer, static_cast<size_t>(count));
  }
  return response;
}

class InspectableServer : public WebServer {
 public:
  using WebServer::WebServer;
  bool argumentsEmpty() const {
    return !_currentArgs && _currentArgCount == 0 && !_postArgs && _postArgsLen == 0;
  }
};

int main() {
  uint16_t port = 0;
  int listening = listener(port);
  assert(::listen(listening, 1) == 0);
  NetworkClient client;
  assert(client.connect(IPAddress(127, 0, 0, 1), port));
  const int peer = ::accept(listening, nullptr, nullptr);
  assert(peer >= 0);
  Print& output = client;
  assert(output.println("shared") == 8);
  char received[8];
  assert(::recv(peer, received, sizeof(received), MSG_WAITALL) == sizeof(received));
  assert(std::string(received, sizeof(received)) == "shared\r\n");
  NetworkClient copy = client;
  copy.stop();
  assert(!client.connected() && !copy.connected() && output.print("closed") == 0);
  ::close(peer);
  ::close(listening);

  listening = listener(port);
  ::close(listening);
  InspectableServer server(port);
  server.on("/args", HTTP_ANY, [&] {
    const auto value = server.arg("q");
    const auto body = server.arg("plain");
    assert(server.args() == (body.isEmpty() ? 1 : 2));
    server.releaseRequestArguments();
    assert(server.argumentsEmpty() && server.args() == 0);
    server.send(200, "text/plain", value + body);
  });
  std::atomic<bool> entered{false}, resume{false}, stopped{false};
  server.on("/hold", HTTP_GET, [&] {
    assert(server.arg("q") == "held");
    entered = true;
    waitFor(resume);
    assert(server.arg("q") == "held");
    server.send(200, "text/plain", "held");
  });
  server.begin();
  assert(request(port, "POST /args?q=one HTTP/1.1\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}").ends_with("one{}"));
  assert(request(port, "GET /args?q=two HTTP/1.1\r\n\r\n").ends_with("two"));
  assert(request(port, "POST /args?q=bad HTTP/1.1\r\nContent-Length: 8\r\n\r\nx").find("400 Bad Request") != std::string::npos);
  assert(request(port, "GET /args?q=after HTTP/1.1\r\n\r\n").ends_with("after"));
  std::string heldResponse;
  std::thread caller([&] { heldResponse = request(port, "GET /hold?q=held HTTP/1.1\r\n\r\n"); });
  waitFor(entered);
  server.releaseRequestArguments();
  std::thread stopper([&] { server.stop(); stopped = true; });
  assert(!stopped);
  resume = true;
  caller.join();
  stopper.join();
  assert(heldResponse.ends_with("held") && stopped && server.argumentsEmpty());
  server.begin();
  assert(request(port, "GET /args?q=restarted HTTP/1.1\r\n\r\n").ends_with("restarted"));
  server.stop();
  assert(server.argumentsEmpty());
}
