"""
PlatformIO build-time script.
Reads firmware/.env and emits -D<KEY>="<VALUE>" build flags so that
credentials never have to be hard-coded in source files.

Usage in platformio.ini:
    build_flags = !python3 scripts/load_env.py
"""

import os
import sys

ENV_FILE = os.path.join(os.path.dirname(__file__), "..", ".env")

# Keys that should be emitted as integers (no surrounding quotes)
INT_KEYS = {"MQTT_PORT", "HEARTBEAT_INTERVAL"}


def load_flags(path):
    flags = []
    if not os.path.exists(path):
        print(f"Warning: {path} not found — using defaults from config.h", file=sys.stderr)
        return flags

    with open(path) as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue

            key, _, value = line.partition("=")
            key = key.strip()
            value = value.strip()

            # Strip trailing inline comment
            if " #" in value:
                value = value[: value.index(" #")].strip()

            if not value:
                continue

            if key in INT_KEYS:
                flags.append(f"-D{key}={value}")
            else:
                # Escape any embedded quotes to keep the compiler happy
                value = value.replace('"', '\\"')
                flags.append(f'-D{key}=\\"{value}\\"')

    return flags


if __name__ == "__main__":
    print(" ".join(load_flags(ENV_FILE)))
