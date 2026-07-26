# 🚀 GitHub Downloader

A high-performance, modern, dark-themed **Native Windows Desktop** & **Web Application** designed to explore, select, and download GitHub repositories, specific branches, folders, or individual files with ease.

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Direct2D](https://img.shields.io/badge/UI-Direct2D%20%2F%20Win32-purple.svg)
![HTML5/CSS3](https://img.shields.io/badge/Web-HTML5%20%2F%20CSS3%20%2F%20JS-orange.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%2F%2011-0078D6.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

---

## 🌟 Key Features

- **⚡ Native Direct2D GPU Acceleration**: High-frame-rate, hardware-accelerated desktop rendering built with Direct2D and DirectWrite.
- **🌐 Parallel Web Interface**: Fully functional standalone Web App with modern glassmorphism UI, vector SVG icons, and interactive tree management.
- **📁 Interactive File Tree Explorer**: Tree view featuring folder expansion/collapse, file-type specific icons (`css`, `js`, `image`, `code`), and right-aligned byte size indicators.
- **🌿 Branch Selector**: Easily switch between `main`, `master`, `dev`, or custom repository branches.
- **📋 One-Click Clipboard Paste**: Instant repository URL pasting from clipboard.
- **🎯 Granular File Selection**: Flexible checkbox system supporting "Select All", "Deselect All", and folder-level recursive selection.
- **📊 Live Speed & Progress Tracking**: Real-time download progress bar, byte transfer counts, and transfer speed metrics.
- **🎨 Glassmorphic Dark Theme**: Curated dark midnight slate (`#0b0f19`) and glowing indigo/purple accent design system.

---

## 🛠️ Technology Stack

### **Native Desktop Engine (C++)**
- **Language Standard**: C++20 (`stdcpp20`)
- **GUI Framework**: Win32 API + Direct2D 1.1 (`d2d1.lib`)
- **Typography & Text Rendering**: DirectWrite (`dwrite.lib`)
- **Networking**: Windows HTTP Services (`winhttp.lib`)
- **System Utilities**: Shell Scaling API, DWM API (`dwmapi.lib`), Shlwapi
- **JSON Parser**: `nlohmann/json`

### **Web Interface**
- **Core**: HTML5 Semantic Structure
- **Styling**: Vanilla CSS3 (Custom Design System Tokens, CSS Grid/Flexbox, Animations, Glassmorphism)
- **Scripting**: ES6+ JavaScript (DOM Manipulation, Tree Traversal Algorithms, Async API Fetching)

---

## 📁 Repository Structure

```text
GitHubDownloader/
 ├── download/           # Download task execution & batch progress tracking
 ├── filesystem/         # File system helpers, path resolution & directory sanitization
 ├── github/             # GitHub REST API client & repository tree parser
 ├── include/            # Precompiled headers (pch.h), data types & color tokens
 ├── network/            # WinHTTP asynchronous HTTP request wrapper
 ├── src/                # Entry point (wWinMain) & application lifecycle manager
 ├── thirdparty/         # Header-only dependencies (nlohmann/json)
 ├── ui/                 # Direct2D renderer engine, custom UI components, main window
 ├── utils/              # Thread pool, thread-safe logger & string helpers
 └── web/                # Standalone Web Application
      ├── index.html     # Web App main view
      ├── css/
      │    └── style.css # Custom design system & theme stylesheet
      └── js/
           └── app.js    # Interactive file tree & selection logic
```

---

## 🔧 Building & Running

### **Prerequisites**
- **Operating System**: Windows 10 / 11 (64-bit)
- **Compiler**: Visual Studio 2022 (MSVC v143 toolset) with *Desktop development with C++* workload.
- **Build System**: MSBuild

### **Building Native Application (MSBuild)**

Run MSBuild from Developer Command Prompt or Terminal:

```powershell
# Build Debug x64
MSBuild.exe GitHubDownloader.sln /p:Configuration=Debug /p:Platform=x64

# Build Release x64
MSBuild.exe GitHubDownloader.sln /p:Configuration=Release /p:Platform=x64
```

The compiled binary will be generated at:
`x64/Debug/GitHubDownloader.exe` or `x64/Release/GitHubDownloader.exe`

### **Running Web Application**

Simply open `web/index.html` in any modern web browser (Chrome, Edge, Firefox, Safari).

---

## 📜 License

This project is licensed under the MIT License - see the `LICENSE` file for details.
