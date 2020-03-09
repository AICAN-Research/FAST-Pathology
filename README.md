FAST Pathology
===================================

FAST pathology is an open-source platform for artificial intelligence-based digital pathology created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).

Install
-----------------------------------
Download an appropriate installer from the [release page](https://github.com/SINTEFMedtek/FAST-Pathology/releases/).

Development setup
-----------------------------------
1. Either
   - [Download a release of FAST](https://github.com/smistad/FAST/releases) and install its [requirements](https://github.com/smistad/FAST/wiki/Requirements).
   - Compile and install FAST on your system: See instructions here for [Windows](https://github.com/smistad/fast/wiki/Windows-instructions) or [Linux (Ubuntu)](https://github.com/smistad/fast/wiki/Linux-instructions).
2. Clone this repository
   ```bash
   git clone https://github.com/SINTEFMedtek/FAST-Pathology.git
   ```
3. Setup build environment using CMake  
   *Linux (Ubuntu)*
   ```bash
   mkdir build
   cd build
   cmake .. -DFAST_DIR=/path/to/FAST/cmake/
   ``` 
   *Windows (Visual Studio)*
   Modify generator -G string to match your Visual studio version. This command will create a visual studio solution in your build folder.
   ```bash
   mkdir build
   cd build
   cmake .. -DFAST_DIR=C:\path\to\FAST\cmake\ -G "Visual Studio 16 2019" -A x64
   ```
4. Build
   ```bash
   cmake --build . --config Release --target fastPathology
   ```
5. Run
   *Linux (Ubuntu)*
   ```bash
   ./fastPathology
   ```
   *Windows*
   ```powershell
   cd Release
   fastPathology.exe
   ```
