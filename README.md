<div align="center">
<img src="data/Icons/fastpathology_logo.png" width="128">
<h1 align="center">FastPathology</h1>
<h3 align="center">Open platform for deep learning-based research and decision support in digital pathology</h3>

[![GitHub Downloads](https://img.shields.io/github/downloads/SINTEFMedtek/FAST-Pathology/total?label=GitHub%20downloads&logo=github)](https://github.com/SINTEFMedtek/FAST-Pathology/releases)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)
![CI](https://github.com/AICAN-Research/FAST-Pathology/workflows/Build%20Windows/badge.svg?branch=master&event=push)
![CI](https://github.com/AICAN-Research/FAST-Pathology/workflows/Build%20Ubuntu/badge.svg?branch=master&event=push)
![CI](https://github.com/AICAN-Research/FAST-Pathology/workflows/Build%20macOS/badge.svg?branch=master&event=push)
 
**FastPathology** was created by SINTEF Medical Technology and the Norwegian University of Science and Technology (NTNU). A paper presenting the software and some benchmarks has been published in [IEEE Access](https://ieeexplore.ieee.org/document/9399433).
 
 <img src="data/Videos/fp_demo_v1.gif" style="background-color:black">
</div>

## 💻 Getting started
To install FastPathology, follow the instructions for your operating system:

<details open>
<summary>

### Windows (10 or newer) </summary>
* Download and install the [Microsoft Visual C++ Redistributable 2015-2019 (64bit/x64)](https://aka.ms/vs/16/release/vc_redist.x64.exe).
* Download and run the Windows installer from the [release page](https://github.com/AICAN-Research/FAST-Pathology/releases/). 
  *Note: Windows might prompt you with a security warning, to proceed you must press "More info" followed by "Run anyway".*
* Run **fastpathology** from your start menu.
* To **uninstall** the application, go to start menu -> remove programs -> find fastpathology and select uninstall.
  Optionally you can also delete your C:/Users/"your username"/fastpathology/ which includes stored project results, pipelines and models.
  And the folder C:/ProgramData/FAST/ which contains a cache.
  
</details>

<details>
<summary>

### Ubuntu Linux (18.04 or newer)</summary>

- Install OpenCL for Linux by downloading an implementation depending on the CPU/GPU you have:
   - **NVIDIA** - Install [CUDA](https://developer.nvidia.com/cuda-downloads)
   - **Intel** - Install the [OpenCL NEO driver](https://github.com/intel/compute-runtime/releases)
   - **AMD** - Install the [ROCm stack](https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html)
   - If none of the above fits, you can try the [Portable Computing Lanauge (PCOL)](http://portablecl.org), although reduced performance is likely.
* Download the debian package from the [release page](https://github.com/AICAN-Research/FAST-Pathology/releases/).
* Install the debian package from the terminal or by double-clicking it:
```bash
sudo dpkg -i fastpathology_ubuntu*.deb
```
* Go the folder /opt/fastpathology/bin and run the **fastpathology** executable, or run it from the ubuntu menu (windows button->type fastpathology).
* To **uninstall** the application, run the following in your terminal:
```bash
sudo apt remove fastpathology
# Optionally, you can also delete your fastpathology folder 
# which includes stored project results, pipelines and models.
# and the FAST folder which stores cache files.
rm -Rf $HOME/fastpathology
rm -Rf $HOME/FAST
```

</details>

<details>
<summary>

### macOS (10.13 or newer)</summary>

*Note that the macOS version of FastPathology is experimental.*

* Install [homebrew](https://brew.sh/) if you don't already have it. Then, install the following packages using homebrew:
```bash
brew install openslide libomp
```
* Download the tar.xz package from the [release page](https://github.com/AICAN-Research/FAST-Pathology/releases/).
* Extract the archive to somewhere on your drive
* Disable the gatekeeper from your terminal:
```bash
sudo spctl --master-disable
```
* Go to extracted folder and find the bin folder and run the executable **fastpathology**.
* To **uninstall** the application, delete the extracted folder.
  Optionally, you can also delete the  /Users/"your username"/fastpathology folder, which includes stored project results, pipelines and models.
  And the folder /Users/"your username"/FAST which contains a cache.

</details>

<details>
<summary>

### Optional: NVIDIA GPU Inference</summary>
If you have an NVIDIA GPU on your machine you can enable high-speed inference by downloading and installing the following:
* [CUDA 11](https://developer.nvidia.com/cuda-toolkit-archive)
* [cuDNN 8.2](https://developer.nvidia.com/rdp/cudnn-archive)
* [TensorRT 8.2](https://developer.nvidia.com/nvidia-tensorrt-download)

**Note: Make sure to download the correct versions. NVIDIA GPU inference is not supported on Mac.**

</details>

## 📹 Demos and tutorials
Very simple demonstrations of the platforms can be found on [Youtube](https://www.youtube.com/channel/UC4GM2KW54-vEZ0M1kH5-oig). More in-depth demonstrations will be added in the future. Wikis and tutorials can be found in the [wiki](https://github.com/SINTEFMedtek/FAST-Pathology/wiki). More information can be found from the **pages** section on the right in the wiki home.

[![Watch the video](figures/youtube-thumbnail.jpg)](https://youtu.be/1s7jU6T7S3U?t=435).

## 🎊 Features
The software is implemented in C++ based on [FAST](https://github.com/smistad/FAST). A wide range of features have been added to the platform and FAST to make working with Whole Slide Images (WSIs) a piece of cake!
* **Graphical User Interface -** User-friendly GUI for working with WSIs without any code interaction
* **Deep learning -** Deployment and support for multi-input/output Convolutional Neural Networks (CNNs)
* **Visualization -** Real-time streaming of predictions on top of the WSI with low memory cost
* **Use cases -** Patch-wise classification, low and high-resolution segmentation, and object detection are supported
* **Inference Engines -** FAST includes a variety of different inference engines, i.e. TensorFlow CPU/CUDA (support both TF v1 and v2 models), TensorRT (UFF and ONNX), and OpenVINO (CPU/GPU/VPU)
* **Text pipelines -** Possibility to create your own pipelines using the built-in script editor
* **Formats -** Through OpenSlide FastPathology supports various WSI formats, as well as additional support for the CellSens VSI format through FAST


## 🔬 Applications of FastPathology
* Pettersen et al., Code-free development and deployment of deep segmentation models for digital pathology (2022), Frontiers in Medicine, https://www.frontiersin.org/articles/10.3389/fmed.2021.816281/full
* Pedersen et al., Hybrid guiding: A multi-resolution refinement approach for semantic segmentation of gigapixel histopathological images (2021), preprint arXiv: https://arxiv.org/abs/2112.03455

<details>
<summary>

## 🔨 Development setup </summary>

1. Either
   - [Download and install a release of FAST](https://fast.eriksmistad.no/install.html).
   - [Compile and install FAST on your system](https://fast.eriksmistad.no/building-fast.html).
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

**NOTE:** Visual Studio 19 have been tested with both FAST and FastPathology and works well.

</details>

## ✨ How to cite
Please, consider citing our paper, if you find the work useful:
<pre>
  @ARTICLE{9399433,
  author={Pedersen, André and Valla, Marit and Bofin, Anna M. and De Frutos, Javier Pérez and Reinertsen, Ingerid and Smistad, Erik},
  journal={IEEE Access}, 
  title={FastPathology: An Open-Source Platform for Deep Learning-Based Research and Decision Support in Digital Pathology}, 
  year={2021},
  volume={9},
  number={},
  pages={58216-58229},
  doi={10.1109/ACCESS.2021.3072231}}
</pre>

