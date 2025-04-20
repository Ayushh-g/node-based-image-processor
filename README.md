# Node-Based Image Manipulation Interface

## Project Overview

This project is a C++ desktop application featuring a node-based graphical user interface (GUI) for image manipulation. Users can load images, process them through a pipeline of connected nodes representing various operations, adjust parameters in real-time, and save the final output. The interface is inspired by node-based editors like Substance Designer, allowing for flexible and visual construction of image processing workflows.

## Features Implemented

*   **Graphical User Interface:**
    *   Canvas area for creating, connecting, and arranging processing nodes.
    *   Main menu bar for file operations and node creation.
    *   Node-specific controls embedded within each node for parameter adjustments.
    *   Real-time preview updates within nodes as parameters are changed.
*   **Node System:**
    *   Visual nodes representing image operations.
    *   Input and output pins on nodes for connecting data flow.
    *   Link system to connect nodes and define the processing pipeline.
    *   Topological sorting to determine the correct processing order.
    *   Centralized image data management (`ImageDataManager`) to handle data transfer between nodes.
*   **Implemented Nodes:**
    *   **Image Input:** Loads images (JPG, PNG, BMP) from the file system, displays metadata, and provides options for auto-resizing large images.
    *   **Output:** Saves the processed image to disk with selectable formats (JPEG, PNG, BMP) and quality/compression settings. Displays a preview of the final image.
    *   **Brightness/Contrast:** Adjusts image brightness and contrast using sliders with reset options.
    *   **Color Channel Splitter:** Splits an image into R, G, B, and Alpha channels, with an option to output channels as grayscale.
    *   **Blur:** Applies Gaussian or directional blur with configurable radius, angle, and strength. Visualizes the blur kernel.
    *   **Threshold:** Converts an image to binary using Binary, Adaptive, or Otsu thresholding methods. Includes parameter controls and a histogram display.
    *   **Edge Detection:** Implements Sobel, Canny, and Laplacian edge detection algorithms with configurable parameters.
    *   **Blend:** Combines two images using various blend modes (Normal, Multiply, Screen, Overlay, Difference, Lighten, Darken) with an opacity slider. Handles image resizing and type conversion.
    *   **Noise Generation:** Creates noise images (Uniform Random, Gaussian Random) with configurable dimensions and parameters.
    *   **Convolution Filter:** Applies a custom 3x3 or 5x5 convolution kernel defined by the user. Includes presets for common filters (Identity, Sharpen, Emboss, Edge Enhance).

## Build Instructions

There are two primary ways to build this project: using Visual Studio directly with the provided solution file, or using CMake for more flexibility.

### Visual Studio

1.  **Clone Repository:**
    ```bash
    git clone https://github.com/Ayushh-g/node-based-image-processor.git
    cd node-based-image-processor
    ```
2.  **Prerequisites:**
    *   **Visual Studio:** Install Visual Studio (e.g., 2022 or compatible) with the "Desktop development with C++" workload.
    *   **OpenCV:** Install OpenCV (version 4.x recommended). Add the path to your OpenCV binaries (e.g., `C:\path\to\opencv\build\x64\vc16\bin`) to your system's PATH environment variable. This ensures the necessary OpenCV DLLs can be found at runtime.
3.  **Dependencies:** Other required libraries (Dear ImGui, ImGui-Node-Editor, GLFW, etc.) are included within the `externals/` directory and are already configured in the solution.
4.  **Open Solution:** Navigate to the project's root directory and open the `node-based-image-processor.sln` file with Visual Studio.
5.  **Select Configuration:** In the Visual Studio toolbar, select the **Release** configuration and **x64** platform. *(Using the Release configuration is recommended as project settings are pre-configured for it).*
6.  **Build Solution:** Build the project using the menu `Build > Build Solution` (or press Ctrl+Shift+B).
7.  **Run:** Run the application directly from Visual Studio using the "Local Windows Debugger" button (the green play icon) or by pressing F5. Visual Studio will automatically handle running the executable from the correct output directory (`x64/Release/`).

### CMake

This method uses CMake directly, which might be preferable on systems without Visual Studio or for cross-platform building.

1. **Prerequisites (for Windows using MSYS2/MinGW):**
   *   **Setup MSYS2/MinGW:**
       ```bash
       # Install MSYS2 from https://www.msys2.org

       # Update MSYS2 (run in MSYS2 terminal)
       pacman -Syu
       pacman -Su 
       # (Close and reopen terminal if prompted)

       # Install required packages
       pacman -S --needed base-devel mingw-w64-x86_64-toolchain git cmake make
       pacman -S mingw-w64-x86_64-opencv mingw-w64-x86_64-glfw
       ```
   *   **Add MinGW to PATH:**
       *   Add `C:\msys64\mingw64\bin` (or your MSYS2 install location) to your Windows System PATH environment variable.
       *   Restart your terminal or VS Code for the PATH change to take effect.
   *   **(Note:** Other platforms/compilers will need CMake, Git, a C++17 compiler, and system installations of OpenCV and GLFW findable by CMake.)

2. **Clone Repository:**
    ```bash
    git clone https://github.com/Ayushh-g/node-based-image-processor.git
    cd node-based-image-processor
    ```
3. **Configure and Build:**
    ```bash
    mkdir build
    cd build
    # Choose a generator, e.g., "MinGW Makefiles", "Ninja", "Visual Studio 17 2022"
    cmake -G "MinGW Makefiles" .. 
    mingw32-make # Or 'make', 'ninja', 'cmake --build .' depending on generator
    ```
    *(Note: Replace `"MinGW Makefiles"` with your desired CMake generator if needed. Replace `mingw32-make` with the corresponding build command.)*
4. **Run Application:**
    ```bash
    ./node-based-image-processor.exe 
    ```


## Third-Party Libraries

This project utilizes the following third-party libraries:

*   **OpenCV (Open Source Computer Vision Library):** Used for core image loading, processing, and manipulation functions. ([https://opencv.org/](https://opencv.org/))
*   **Dear ImGui:** Used for creating the graphical user interface elements. ([https://github.com/ocornut/imgui](https://github.com/ocornut/imgui))
*   **ImGui-Node-Editor:** An extension for Dear ImGui used to create the node graph interface. ([https://github.com/thedmd/imgui-node-editor](https://github.com/thedmd/imgui-node-editor))
*   **(Platform/Renderer Abstractions):** The `application/` directory contains code likely using libraries like GLFW, DirectX SDK, or OpenGL drivers depending on the specific build target and platform configuration within Visual Studio.
