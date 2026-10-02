#pragma once

#include <SecureHttpClient.h>

#include <cstdint>
#include <functional>
#include <string>

namespace freeink {

struct FetchOptions {
  size_t startOffset = 0;
  bool redirectToHttp = false;
};

struct FetchSink {
  std::function<bool(const uint8_t*, size_t)> write;
  std::function<bool()> rewind;
  std::function<void(size_t, size_t)> progress;
};

struct FetchResult {
  int status = 0;
  size_t bytes = 0;
  size_t total = 0;
  bool complete = false;
  bool stopped = false;
  bool aborted = false;
};

// The simulator HTTP transport buffers each response, so it cannot replay a
// partial body. Keep the firmware result contract while doing one host request.
inline FetchResult fetchResumable(
    const std::string& url, const FetchOptions& options,
    const std::function<void(SecureHttpClient&, bool)>& configure,
    const FetchSink& sink,
    const SecureHttpClient::AbortCallback& shouldAbort = nullptr) {
  FetchResult result;
  result.bytes = options.startOffset;
  if (shouldAbort && shouldAbort()) {
    result.aborted = true;
    return result;
  }
  SecureHttpClient http;
  if (!http.begin(url)) {
    result.status = -1;
    return result;
  }
  if (configure) configure(http, true);
  if (options.startOffset)
    http.addHeader("Range", "bytes=" + std::to_string(options.startOffset) + "-");
  result.status = http.GET();
  if (shouldAbort && shouldAbort()) {
    result.aborted = true;
    return result;
  }
  if (result.status < 200 || result.status >= 300 || !http.responseComplete())
    return result;
  if (options.startOffset && result.status == 200) {
    if (!sink.rewind || !sink.rewind()) {
      result.stopped = true;
      return result;
    }
    result.bytes = 0;
  }
  const String body = http.getString();
  if (!body.s.empty() &&
      (!sink.write || !sink.write(
                          reinterpret_cast<const uint8_t*>(body.s.data()),
                          body.s.size()))) {
    result.stopped = true;
    return result;
  }
  result.bytes += body.s.size();
  result.total = result.bytes;
  if (sink.progress) sink.progress(result.bytes, result.total);
  result.complete = true;
  return result;
}

}  // namespace freeink
