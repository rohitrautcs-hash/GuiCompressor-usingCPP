# GUI Compressor v1.0 ⚡

**GUI Compressor v1.0** is a modern, cross-platform Qt6/Qt5 application for lossless folder archiving using a custom DEFLATE-style compression pipeline (LZ77 + Huffman Coding) with IEEE 802.3 CRC-32 checksum integrity verification.

Built for **BIT 2nd Semester** at **Kathmandu Don Bosco College**.

## 👥 Academic Team

- **Rohit Kumar Raut** — Team Lead & Core Engine Architecture
- **Anuj Kumar Sah** — Contributor
- **Prativa Rimal** — Contributor

---

## 🏗️ Architecture & Component Separation

```
GUI-Compressor/
├── CMakeLists.txt              # Cross-platform Qt6/Qt5 build script
├── README.md                   # Multi-platform compilation guide
└── src/
    ├── main.cpp                # Application entry point & theme initialization
    ├── core/                   # Standalone C++17 Lossless Engine (NO Qt dependencies)
    │   ├── types.hpp           # LZ77 token data definitions
    │   ├── bitstream.hpp/cpp   # MSB bit writer & bounds-checked reader
    │   ├── crc32.hpp/cpp       # IEEE 802.3 CRC-32 table calculation
    │   ├── huffman.hpp/cpp     # Canonical Huffman tree codec with single-symbol edge handling
    │   ├── lz77.hpp/cpp        # 4KB sliding window dictionary compression
    │   ├── deflate.hpp/cpp     # DEFLATE pipeline orchestrator (LZ77 → Huffman)
    │   └── archive.hpp/cpp     # GCF1 archive format & atomic temporary files
    ├── controller/             # Qt Worker Thread Bridge
    │   └── worker.hpp/cpp      # QThread background worker for unblocked UI
    └── gui/                    # Qt Widgets UI Layer
        ├── welcomedialog.hpp/cpp # Animated onboarding modal on first run
        ├── logpanel.hpp/cpp     # Docked console log panel with color timestamps
        ├── compresspage.hpp/cpp # Folder compression page with gradient progress bar
        ├── extractpage.hpp/cpp  # Archive extraction page & metadata inspector
        ├── aboutpage.hpp/cpp    # Team & algorithm specification view
        └── mainwindow.hpp/cpp   # Sidebar navigation container & page fade transitions
```

---

## 🛠️ Build & Installation Instructions

### Prerequisites
- C++17 compatible compiler (`g++ 9+`, `clang 10+`, or `MSVC 2019+`)
- CMake `3.16` or newer
- Qt `6.x` or Qt `5.15` (Core & Widgets modules)

---

### 💻 Windows (MinGW / Qt 6)

```powershell
# 1. Open PowerShell and set environment variables (adjust path to your Qt installation)
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\6.11.1\mingw_64\bin;" + $env:PATH

# 2. Configure build directory
cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"

# 3. Build Release Executable
cmake --build build --config Release

# 4. Run Application
.\build\GUICompressor.exe
```

---

### 🐧 Linux (Ubuntu / Debian / Arch)

```bash
# 1. Install Qt6 dependencies
sudo apt update
sudo apt install build-essential cmake qt6-base-dev

# 2. Configure & build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 3. Run Executable
./build/GUICompressor
```

---

### 🍎 macOS (Homebrew)

```bash
# 1. Install Qt via Homebrew
brew install qt cmake

# 2. Configure with CMake
cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)" -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. Launch App
./build/GUICompressor.app/Contents/MacOS/GUICompressor
```

---

## 📊 Archive Specifications (.gcf v1)

- **Magic Header:** `GCF1` (4 bytes)
- **Version:** `1` (16-bit uint)
- **Algorithm:** DEFLATE (LZ77 4KB window + Huffman tree)
- **Checksum:** IEEE 802.3 32-bit CRC (`0xEDB88320`)
- **Atomic File Writing:** Writes payload to `.tmp` prior to atomic rename to prevent file corruption.
- **Auto-store Fallback:** Raw storage (CompressionMethod::Raw) automatically selected when compressed payload exceeds raw file size.
