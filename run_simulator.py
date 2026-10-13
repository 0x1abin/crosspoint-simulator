"""Register the simulator's PlatformIO run target once per build process."""

Import("env", "projenv")
import builtins
import os

RUN_SIMULATOR_TARGET_KEY = "_crosspoint_run_simulator_target_registered"
RUN_SIMULATOR_TARGET_OWNER_OPTION = "custom_run_simulator_target_owner"
SIMULATOR_HTTP_PORT_OPTION = "custom_simulator_http_port"
env.AppendUnique(CPPDEFINES=[("ARDUINOJSON_ENABLE_ARDUINO_STRING", 1)])
projenv.AppendUnique(CPPDEFINES=[("ARDUINOJSON_ENABLE_ARDUINO_STRING", 1)])

# The web manager uses the SDK's header-only crypto interface and base64 helpers.
content_headers = os.path.join(
    env.subst("$PROJECT_DIR"), "freeink-sdk", "libs", "book",
    "ContentProtection", "include"
)
if os.path.isdir(content_headers):
    env.AppendUnique(CPPPATH=[content_headers])


# --- run_simulator custom target ---

def _run_simulator(source, target, env):
    import subprocess

    binary = env.subst("$BUILD_DIR/program")
    runtime_env = os.environ.copy()
    configured_http_port = env.GetProjectOption(
        SIMULATOR_HTTP_PORT_OPTION, ""
    ).strip()
    if configured_http_port:
        runtime_env["CROSSPOINT_SIM_HTTP_PORT"] = configured_http_port
    subprocess.run([binary], cwd=os.getcwd(), env=runtime_env)


target_owner = env.GetProjectOption(RUN_SIMULATOR_TARGET_OWNER_OPTION, "").strip().lower()

if target_owner != "project" and not getattr(builtins, RUN_SIMULATOR_TARGET_KEY, False):
    setattr(builtins, RUN_SIMULATOR_TARGET_KEY, True)
    env.AddCustomTarget(
        name="run_simulator",
        dependencies="$PROGPATH",
        actions=_run_simulator,
        title="Run Simulator",
        description="Build and run the desktop simulator",
        always_build=True,
    )
