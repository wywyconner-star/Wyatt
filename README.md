# Auto Decorator (Geode v5.11.2 Mod)

This folder contains the complete source project configured for **Geode v5.11.2** and **Geometry Dash 2.2081**.

> **Important Note on `.geode` Files:**
> A `.geode` file is a compiled mod package that contains compiled C++ binaries (`.dll` for Windows, `.dylib` for macOS, `.so` for Android) inside a ZIP archive. Simply renaming a source code `.zip` folder to `.geode` will cause Geode to reject it as an invalid mod because the C++ source code (`src/main.cpp`) has not been compiled into a binary yet. Follow either Option 1 or Option 2 below to compile it into a working `dev.autodecorator.geode` package!

## Option 1: Build Automatically on GitHub (No C++ Compiler Needed)
1. Unzip this folder and upload all files (including the `.github` folder) to a new **GitHub** repository.
2. Go to the **Actions** tab in your GitHub repository.
3. Click the **Build Geode Mod** workflow run, wait for it to finish, and download the compiled `dev.autodecorator.geode` file from the **Artifacts** section (`Build Output`) at the bottom.
4. Place `dev.autodecorator.geode` inside your Geometry Dash `geode/mods` folder.

## Option 2: Build Locally on Your PC
1. Install the Geode CLI and SDK v5.11.2 (`geode sdk update v5.11.2`).
2. Open a terminal inside the unzipped project folder and run:
   ```bash
   geode build
   ```
3. Your compiled `.geode` package will be created at `build/dev.autodecorator.geode` (and automatically copied to your Geometry Dash `geode/mods` folder).
