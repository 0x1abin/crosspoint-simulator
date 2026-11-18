#include <SimLog.h>
#include <cassert>
#include <cstring>
#include <thread>

static void line(const std::string &text) {
  sim_log::write(text.data(), text.size());
  sim_log::write("\n", 1);
}
static bool contains(const char *text) {
  return sim_log::findSince(0, std::regex(text), nullptr);
}
int main(int argc, char **argv) {
  assert(argc == 2);
  if (std::strcmp(argv[1], "lines") == 0) {
    line("old-marker");
    for (int i = 0; i < 4096; ++i) line("new-marker");
    assert(!contains("old-marker") && contains("new-marker"));
  } else if (std::strcmp(argv[1], "bytes") == 0) {
    line("old-marker");
    for (int i = 0; i < 65; ++i) line(std::string(16384, 'x'));
    assert(!contains("old-marker") && contains("x"));
  } else if (std::strcmp(argv[1], "partial") == 0) {
    const std::string prefix(16384, 'x');
    sim_log::write(prefix.data(), prefix.size());
    assert(!contains("x"));
    line("tail-marker");
    std::string retained;
    assert(sim_log::findSince(0, std::regex("x"), &retained));
    assert(retained.size() == 16384 && !contains("tail-marker"));
    line("recovered\r");
    assert(contains("^recovered$"));
  } else {
    assert(std::strcmp(argv[1], "threads") == 0);
    auto writer = [](char c) {
      const std::string text = std::string(64, c) + "\n";
      for (int i = 0; i < 100; ++i) sim_log::write(text.data(), text.size());
    };
    std::thread a(writer, 'a'), b(writer, 'b');
    a.join(); b.join();
    assert(contains("^a{64}$") && contains("^b{64}$"));
  }
}
