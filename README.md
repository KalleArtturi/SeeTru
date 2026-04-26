# STONE2025 — Project Setup Guide

## Prerequisites

### 1. Install Homebrew
If you don't have Homebrew installed:
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Verify installation:
```bash
brew --version
```

---

### 2. Install FFmpeg
```bash
brew install ffmpeg
```

Verify installation:
```bash
ffmpeg -version
```

---

### 3. Install pkg-config
```bash
brew install pkg-config
```

---

### 4. Set Up Environment Variables
Add Homebrew and pkg-config to your path:
```bash
export PATH="/opt/homebrew/bin:$PATH"
export PKG_CONFIG_PATH="/opt/homebrew/Cellar/ffmpeg/8.1_1/lib/pkgconfig"
```

To make these permanent, add them to your `~/.zshrc` file:
```bash
echo 'export PATH="/opt/homebrew/bin:$PATH"' >> ~/.zshrc
echo 'export PKG_CONFIG_PATH="/opt/homebrew/Cellar/ffmpeg/8.1_1/lib/pkgconfig"' >> ~/.zshrc
source ~/.zshrc
```

---

## Project Structure

```
STONE2025/
├── config.txt        ← configuration file
├── sender.cpp        ← sends UDP data
├── receiver.cpp      ← receives UDP data
└── README.md         ← this file
```

---

## config.txt
```
PORT=49201
```

---

## Compiling

### Receiver
```bash
g++ receiver.cpp -o receiver
```

### Sender
```bash
g++ sender.cpp -o sender
```

---

## Running

### Step 1 — Start the receiver first
```bash
./receiver
```

### Step 2 — Run the sender in a second terminal
```bash
./sender
```

### Step 3 — Stop a running program
```bash
Ctrl + C
```

---

## Finding Your Local IP
```bash
ifconfig | grep "inet "
```
Look for a line starting with `inet 130.230.x.x` — that is your local IP.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| `command not found: pkg-config` | `brew install pkg-config` |
| `command not found: ffmpeg` | `brew install ffmpeg` |
| Config file not found | Make sure `config.txt` is in the same folder as your compiled program |
| Receiver prints nothing | It is waiting for data — run the sender in a second terminal |
| Changes not working | Make sure you saved the file (`Cmd + S`) before recompiling |

---

## Dependencies

| Library | Purpose | Install |
|---|---|---|
| FFmpeg | Video encoding/decoding | `brew install ffmpeg` |
| pkg-config | Library path management | `brew install pkg-config` |
| libavcodec | H.264 encoding | Included with FFmpeg |
| libavformat | Video formats | Included with FFmpeg |
| libswscale | Frame conversion | Included with FFmpeg |

---

## Notes
- Always run the **receiver before the sender**
- Both programs must have `config.txt` in the same folder
- The port `49201` must be the same in both `config.txt` files
- FFmpeg version used: `8.1_1`
