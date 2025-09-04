# Developer Guide – Building **tcppump**

This document provides detailed instructions for compiling and packaging the **tcppump** project. It covers system requirements, dependencies, build commands, configuration options, and packaging for **Linux (Debian/Fedora)** and **Windows**.

---

## Quick Start – Install Dependencies

| Platform        | Build Dependencies                                                                 | Development/Debugging Tools                         |
|-----------------|------------------------------------------------------------------------------------|----------------------------------------------------|
| **Fedora**      | `sudo dnf install gcc g++ cmake libpcap-devel git`                                | `sudo dnf install gdb libasan libubsan valgrind rpm-build` |
| **Debian/Ubuntu** | `sudo apt update && sudo apt install build-essential cmake libpcap-dev git`      | `sudo apt install gdb valgrind libasan6 libubsan1 dh-make devscripts` |
| **Windows**     | Visual Studio (2015+), MinGW, CMake, [WinPcap](https://www.winpcap.org/) or [Npcap](https://nmap.org/npcap/) | – |

---

## System Requirements

- **Build system:** [CMake](https://cmake.org/) (≥ 3.19)  
- **Compiler:** A C++11-compliant compiler (GCC ≥ 4.8, Clang ≥ 3.3, or MSVC ≥ 2015)  
- **Supported platforms:**  
  - Linux (Debian-based distributions, Fedora)  
  - Windows (with WinPcap/NPcap)  


## Build Dependencies

### Linux – Fedora

**Required for building:**
```bash
sudo dnf install gcc g++ cmake libpcap-devel git
```

**Recommended for development/debugging:**
```bash
sudo dnf install gdb libasan libubsan valgrind rpm-build libcap
```


### Linux – Debian/Ubuntu

**Required for building:**
```bash
sudo apt update
sudo apt install build-essential cmake libpcap-dev git
```

**Recommended for development/debugging:**
```bash
sudo apt install gdb valgrind libasan6 libubsan1 dh-make devscripts libcap2-bin
```

> ⚠️ Package names may vary slightly across Debian/Ubuntu versions.  
> - `libasan6` and `libubsan1` correspond to GCC’s Address/Undefined Sanitizer libraries.  
> - For packaging (`dh-make`, `debhelper`) might be required.


### Windows

**Required for building:**
- Visual Studio (2015 or newer, with C++ development tools) or MSYS2 (ucrt64, mingw32, mingw64 or clang64)
- [CMake](https://cmake.org/download/)  
- [WinPcap](https://www.winpcap.org/) or [Npcap](https://nmap.org/npcap/) (recommended replacement for WinPcap). Note: The [WinPcap development files](https://www.winpcap.org/devel.htm) are automatically downloaded during the build.

---

## Build Instructions

```
git clone --recurse-submodules https://github.com/amartin755/tcppump.git
cd tcppump
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Build Configuration Parameters

When running `cmake -B ...`, you can pass additional options via `-D` switch:

- **`CMAKE_BUILD_TYPE`**  
  - `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`  
  - Example: `-DCMAKE_BUILD_TYPE=Debug`

- **`WITH_UNITTESTS`**  
  - Build unit-test binary and run unit tests (default: `ON`)  
  - Example: `-DWITH_UNITTESTS=OFF`

- **`WITH_ASAN`**  
  - Enable/Disable [Address Sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) (default: `OFF`)  
  - Example: `-DWITH_ASAN=ON`

- **`WITH_UBSAN`**  
  - Enable/Disable [Undefined Behavior Sanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) (default: `OFF`)  
  - Example: `-DWITH_UBSAN=ON`

- **`WITH_GCOV`**  
  - Enable/Disable coverage analysis (Linux only)  
  - Example: `-DWITH_GCOV=ON`

- **`WITH_WERROR`**  
  - Treat all compiler warnings as errors (default: `OFF`)  
  - Example: `-DWITH_WERROR=ON`

- **`WITH_CPPCHECK`**  
  - Run static analysis with `cppcheck` during the build (default: `OFF`)  
  - Example: `-DWITH_CPPCHECK=ON`

- **`BUILD_TESTING`**  
  - Enable/Disable ctest test cases (CMake default variable, default: `ON` if tests exist)  

---

## Test
tcppump includes a unittest environment and many ctest based test-cases.
Note: Build config options `WITH_UNITTESTS` and `BUILD_TESTING` must be set to `ON` (default).

Run tests:
```
cd build
ctest
``` 
Run tests with memchecker like valgrind (this will take some time):
```
cd build
ctest -T MemCheck -E "--diff|online-*" -j
``` 
> ⚠️ On some platforms like Windows, `ctest` needs to know the current build configuration. For example: `ctest -C Release`

TODO: documentation of helper scripts!!!

## Packaging (Linux only)

### Create source tarball
Use the bash script `create-source-tarball.sh` to create a tarball of the source code for releases. The created tar.gz archive is the input for further packaging steps (deb, rpm) described below. The script strips all private/local files, prepares changlogs and packaging config files, adds git and version infos and finally creates the archive. As a last step, hash and gpg signature files are created. All intermediate steps are done in a temporary directory and therefore the  content of the current git working directory is not changed.
```
scripts/create-source-tarball.sh [destination directory]
``` 
If no destination directory is provided the files are written to the current directory. 

> ⚠️ The script ignores all uncommitted changes and untracked files. Make sure to commit all changes first. If you want to package something that is not committed to git, use the `--no-clean` argument.

### 5.1 Fedora – RPM package
```bash
scripts/create-rpm.sh tcppump_x.y.z.tar.gz <destination-dir>
```

### 5.2 Debian/Ubuntu – DEB package
```bash
scripts/create-deb.sh tcppump_x.y.z.tar.gz <destination-dir>
```

---

## 6. Debugging & Development Tools

- **gdb** – GNU Debugger  
- **valgrind** – Memory leak detection  
- **asan/ubsan** – Runtime sanitizers  
- **cppcheck** – Static analysis  
