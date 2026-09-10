#!/usr/bin/env python3
"""
TLS26 Mikon Amber - Web Asset Bundler
Automatically bundles src/network/web/ (index.html, sw.js, manifest.json)
into PROGMEM C++ string literals in src/network/WebBundle.h
"""

import os
import sys
from pathlib import Path

def generate_bundle():
    script_dir = Path(__file__).resolve().parent
    network_dir = script_dir.parent
    bundle_h_path = network_dir / "WebBundle.h"

    manifest_path = script_dir / "manifest.json"
    sw_path = script_dir / "sw.js"
    html_path = script_dir / "index.html"

    if not manifest_path.exists() or not sw_path.exists() or not html_path.exists():
        print(f"[BUNDLE ERROR] Missing web asset files in {script_dir}", file=sys.stderr)
        sys.exit(1)

    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest_content = f.read().strip()

    with open(sw_path, "r", encoding="utf-8") as f:
        sw_content = f.read().strip()

    with open(html_path, "r", encoding="utf-8") as f:
        html_content = f.read().strip()

    header_content = f"""#ifndef WEB_BUNDLE_H
#define WEB_BUNDLE_H

#include <Arduino.h>

// MANIFEST PWA (manifest.json)
const char PAGE_MANIFEST_JSON[] PROGMEM = R"rawliteral({manifest_content})rawliteral";

// SERVICE WORKER (sw.js)
const char PAGE_SERVICE_WORKER_JS[] PROGMEM = R"rawliteral(
{sw_content}
)rawliteral";

// DASHBOARD SINGLE PAGE APPLICATION (index.html)
const char PAGE_INDEX_HTML[] PROGMEM = R"rawliteral({html_content}
)rawliteral";

#endif // WEB_BUNDLE_H
"""

    # Avoid updating timestamp if file content has not changed
    if bundle_h_path.exists():
        with open(bundle_h_path, "r", encoding="utf-8") as f:
            current_content = f.read()
        if current_content == header_content:
            print("[BUNDLE] WebBundle.h is already up-to-date.")
            return

    with open(bundle_h_path, "w", encoding="utf-8") as f:
        f.write(header_content)

    total_size = len(header_content.encode("utf-8"))
    print(f"[BUNDLE] WebBundle.h generated successfully ({total_size / 1024:.1f} KB).")

if __name__ == "__main__":
    generate_bundle()
