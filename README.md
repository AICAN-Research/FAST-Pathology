FastPathology
===================================
[![GitHub Downloads](https://img.shields.io/github/downloads/SINTEFMedtek/FAST-Pathology/total?label=GitHub%20downloads&logo=github)](https://github.com/SINTEFMedtek/FAST-Pathology/releases)

FastPathology is an open-source platform for deep learning-based research and decision support in digital pathology, created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU).

**A paper presenting the software and some benchmarks has been submitted to IEEE Access. A preprint has been made available on [arXiv](https://arxiv.org/abs/2011.06033). The PDF can also be accessed more directly from this [link](https://arxiv.org/pdf/2011.06033.pdf). Corresponding supplementary material can be found on [research gate](https://www.researchgate.net/publication/345804898_Supplementary_material_for_FastPathology_An_open-source_platform_for_deep_learning-based_research_and_decision_support_in_digital_pathology).**

![alt-text](data/Videos/pw_predictions.gif)

Install
-----------------------------------
Download an appropriate installer from the [release page](https://github.com/SINTEFMedtek/FAST-Pathology/releases/). Installers for Win10 and Ubuntu (18 and 20) are available. Currently, there are only pre-releases, as there are still some features to be added/fixed, before we can say that it is ready for use. Beta-testers is much obliged ;)

**NOTE:** FastPathology depends on OpenCL. Most Windows machines have OpenCL by default, whereas Ubuntu does not. Thus, for Ubuntu, OpenCL can be installed by installing either [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-10.1-download-archive-base) or [Intel OpenCL SDK](https://software.intel.com/content/www/us/en/develop/tools/opencl-sdk.html). For both operating systems CUDA is required to run inference on a dedicated GPU.

Test data
-----------------------------------
Data for testing the application can be downloaded from [here](http://folk.ntnu.no/andpeder/). It includes some pretrained models, two WSIs, and some example text pipelines. If [curl](https://curl.se/) and [tar](https://wiki.haskell.org/How_to_unpack_a_tar_file_in_Windows) is installed on the machine (works for both Ubuntu and Windows), it is now possible to download the data directly from the GUI (go to the menu bar -> Edit -> Download test data -> click "Yes" -> when finished, click "Yes" again to visualize the WSIs straight away).

Features
-----------------------------------
The software is implemented in C++ based on FAST. A wide range of features have been added to the platform and FAST to make working with Whole Slide Images (WSIs) a piece of cake!
* **Graphical User Interface -** User-friendly GUI for working with WSIs without any code interaction
* **Deep learning -** Deployment and support for multi-input/output Convolutional Neural Networks (CNNs)
* **Visualization -** Real-time streaming of predictions on top of the WSI with low memory cost
* **Use cases -** Patch-wise classification, low and high-resolution segmentation, and object detection are supported
* **Inference Engines -** FAST includes a variety of different inference engines, e.g. TensorFlow CUDA, TensorRT and OpenVINO.
* **Text pipelines -** Possibility to create your own pipelines using the built-in script editor
* **Formats -** Through OpenSlide FastPathology supports lots of WSI formats

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
