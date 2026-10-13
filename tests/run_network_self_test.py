"""Exercise real socket, Print and WebServer request ownership on loopback."""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="simulator-network-test-") as directory:
    temporary = Path(directory)
    (temporary / "Logging.h").write_text(
        "#pragma once\n#define LOG_ERR(...) ((void)0)\n#define LOG_DBG(...) ((void)0)\n")
    binary = temporary / "network-test"
    subprocess.run([*shlex.split(os.environ.get("CXX", "c++")), "-std=c++20", "-pthread",
                    *shlex.split(os.environ.get("CXXFLAGS", "")),
                    "-I" + str(temporary), "-I" + str(root / "src"),
                    str(root / "tests/network_web_self_test.cpp"),
                    str(root / "src/NetworkClient.cpp"), str(root / "src/WebServer.cpp"),
                    "-o", str(binary)], check=True)
    subprocess.run([str(binary)], cwd=temporary, check=True, timeout=15)
print("network/HTTP ownership self-test passed")
