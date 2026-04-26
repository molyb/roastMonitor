import os
import subprocess
import sys

Import("env")

def _safe_run(cmd, cwd):
    """Run a command and stream output to stdout/stderr for visibility.

    Read raw bytes from the child process and decode with utf-8 using
    `errors='replace'` to avoid UnicodeDecodeError on Windows consoles.
    """
    # Use text mode so Popen yields decoded strings directly. Specify
    # encoding and errors to avoid platform-specific decoding failures.
    proc = subprocess.Popen(
        cmd,
        cwd=cwd,
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    for line in proc.stdout:
        sys.stdout.write(line)
    proc.wait()
    return proc.returncode


def build_frontend_assets(source, target, env):
    """
    This is attached to several build/upload hooks to ensure frontend assets
    are built before PlatformIO creates/uploads the filesystem image.
    """
    try:
        frontend_dir = os.path.join(env["PROJECT_DIR"], "frontend")
    except Exception:
        frontend_dir = os.path.join(os.path.dirname(__file__), "..", "frontend")

    frontend_dir = os.path.abspath(frontend_dir)
    node_modules_dir = os.path.join(frontend_dir, "node_modules")

    print(
        "[build_assets] Running frontend build hook. Project dir:",
        env.get("PROJECT_DIR"),
    )
    print("[build_assets] Frontend dir:", frontend_dir)

    if not os.path.isdir(node_modules_dir):
        print("[build_assets] node_modules not found — running 'npm install'...")
        rc = _safe_run("npm install", cwd=frontend_dir)
        if rc != 0:
            print("[build_assets] 'npm install' failed with code:", rc)
            env.Exit(1)
        print("[build_assets] 'npm install' completed successfully.")

    print("[build_assets] Building frontend assets with 'npm run build'...")
    rc = _safe_run("npm run build", cwd=frontend_dir)
    if rc != 0:
        print("[build_assets] 'npm run build' failed with code:", rc)
        env.Exit(1)

    print("[build_assets] Frontend assets built successfully.")


# Attach the pre-action to likely filesystem build targets and common hooks.
# This covers cases where PlatformIO names the filesystem artifact differently
# or when running the 'uploadfs' / 'buildfs' targets directly.
env.AddPreAction("buildfs", build_frontend_assets)
env.AddPreAction("uploadfs", build_frontend_assets)
