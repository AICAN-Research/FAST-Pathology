FastPathology
===================================

FastPathology is an open-source platform for deep learning-based research and decision support in digital pathology, created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).

![alt-text](data/Videos/pw_predictions.gif)

Install
-----------------------------------
Download an appropriate installer from the [release page](https://github.com/SINTEFMedtek/FAST-Pathology/releases/). Installers for Win10 and Ubuntu (18 and 20) are available.

**NOTE:** FastPathology depends on OpenCL. Most Windows machines have OpenCL by default, whereas Ubuntu does not. Thus, for Ubuntu depending on which CPU/GPU you have and want to use install: [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-10.1-download-archive-base) or [Intel OpenCL SDK](https://software.intel.com/content/www/us/en/develop/tools/opencl-sdk.html).

Test data
-----------------------------------
Data for testing the application can be downloaded from [here](http://folk.ntnu.no/andpeder/). It includes some pretrained models, two WSIs (x200, x400), and some example text pipelines.

Features
-----------------------------------
The software is implemented in C++ based on FAST. A wide range of features has been added to the platform and FAST to make working with Whole Slide Images (WSIs) a piece of cake!
* **Graphical User Interface -** User-friendly GUI for working with WSIs without any code iteraction
* **Deep learning -** Deployment of a wide range of multi-input/output Convolutional Neural Networks (CNNs)
* **Visualization -** Real-time streaming of predictions on top of the WSI with low memory cost
* **Use cases -** Patch-wise classification, low and high-resolution segmentation, and object detection are supported
* **Inference Engines -** FAST includes a variety of different inference engines, e.g. TensorFlow CUDA, TensorRT and OpenVINO.
* **Text pipelines -** Possibility to create your own pipelines using the built-in script editor
* **Formats -** Using OpenSlide FastPathology supports lots of WSI formats

Demos
-----------------------------------
Very simple demonstrations of the platforms can be found on [Youtube](https://www.youtube.com/channel/UC4GM2KW54-vEZ0M1kH5-oig).
Tutorials and more in-depth demonstrations will be added in the future.

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
   cmake --build . --config Release --target fastpathology
   ```
5. Run
   *Linux (Ubuntu)*
   ```bash
   ./fastpathology
   ```
   *Windows*
   ```powershell
   cd Release
   fastpathology.exe
   ```

**NOTE:** Both VS 17 and 19 have been tested with both FAST and FastPathology and works well.
