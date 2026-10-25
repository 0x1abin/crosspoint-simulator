"""Compile and exercise the HTTP shims against a local fixture server."""

from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import os
import subprocess
import tempfile
import threading
import time


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *args):
        pass

    def do_GET(self):
        if self.path == "/slow":
            time.sleep(1.2)
        body = b"book"
        status = 302 if self.path in ("/redirect", "/cross-origin", "/loop") else 200
        self.send_response(status)
        if self.path == "/cross-origin":
            self.send_header("Location", f"http://localhost:{self.server.server_port}/check-auth")
        elif self.path == "/loop":
            self.send_header("Location", "/loop")
        elif self.path == "/redirect":
            self.send_header("Location", "/data")
            self.send_header("X-Redirect-Only", "yes")
        elif self.path == "/check-auth" and self.headers.get("Authorization"):
            body = b"credentials leaked"
        if self.path == "/empty":
            body = b""
        if self.path != "/unknown":
            self.send_header("Content-Length", 10 if self.path == "/truncated" else len(body))
        self.send_header("X-Test", "value")
        self.send_header("Set-Cookie", "first=1")
        self.send_header("Set-Cookie", "second=2")
        self.end_headers()
        try:
            self.wfile.write(body)
        except BrokenPipeError:
            pass


def main():
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="sim-http-test-") as directory:
        binary = str(Path(directory) / "http-client-compat")
        subprocess.run([os.environ.get("CXX", "c++"), "-std=c++20", "-pthread",
                        "-I" + str(root / "src"), str(root / "test/http_client_compat.cpp"),
                        str(root / "src/NetworkClient.cpp"), "-o", binary], check=True)
        with ThreadingHTTPServer(("127.0.0.1", 0), Handler) as server:
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            env = dict(os.environ)
            env.pop("CROSSPOINT_SIM_HTTP_MOCK_ROOT", None)
            env["NO_PROXY"] = env["no_proxy"] = "127.0.0.1,localhost"
            try:
                subprocess.run([binary, f"http://127.0.0.1:{server.server_port}"],
                               env=env, check=True, timeout=20)
            finally:
                server.shutdown()
                thread.join()
    print("PASS HTTP status, headers, lengths, redirects, origin scoping, cancellation")


if __name__ == "__main__":
    main()
