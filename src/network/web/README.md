# Web Dashboard Frontend (PWA)

Native web assets for the ESP32 Web Dashboard:

- `index.html`: Single Page Application (HTML5, Vanilla CSS, JS WebSocket Telemetry).
- `sw.js`: PWA Service Worker implementing the offline caching strategy.
- `manifest.json`: Web App Manifest for PWA installation (home screen shortcut, theme color).
- `bundle.py`: Bundler script that compiles these 3 files into PROGMEM flash strings in `../WebBundle.h`.

## Automated Build Integration
`bundle.py` auto runs before every build:
1. **Arduino VS Code Extension**: Configured via `buildPreferences` in `.vscode/arduino.json` (`recipe.hooks.sketch.prebuild.1.pattern`).
2. **VS Code Task**: Run via `Ctrl+Shift+B` (`Compile ESP32 (Auto-Bundle)`).
3. **Manual Execution**: Run `python3 src/network/web/bundle.py` directly at any time.
