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

This project is configured for building with **Visual Studio**.

1.  **Prerequisites:**
    *   Visual Studio (e.g., 2019 or later) with the C++ desktop development workload installed.
    *   OpenCV library installed on your system.
2.  **Dependencies:** Most dependencies (like Dear ImGui, ImGui-Node-Editor) are included within the repository structure. The primary external dependency requiring configuration is OpenCV.
3.  **Open Solution:** Navigate to the directory *containing* the `node-based-image-processor` workspace folder. Locate the Visual Studio Solution file (`.sln`) and open it with Visual Studio.
4.  **Configure OpenCV:**
    *   Ensure Visual Studio can find your OpenCV installation. You typically need to configure the following in the project properties for the `node-based-image-processor` project:
        *   **Include Directories:** (Project Properties -> VC++ Directories -> Include Directories) Add the path to your OpenCV `include` folder (e.g., `C:\opencv\build\include`).
        *   **Library Directories:** (Project Properties -> VC++ Directories -> Library Directories) Add the path to your OpenCV `lib` folder (e.g., `C:\opencv\build\x64\vc15\lib` - adjust the path based on your VS version and OpenCV build).
        *   **Linker Input:** (Project Properties -> Linker -> Input -> Additional Dependencies) Add the necessary OpenCV `.lib` files (e.g., `opencv_worldXXX.lib` or individual module libs for Release/Debug builds - replace `XXX` with your OpenCV version number).
    *   Alternatively, setting the `OPENCV_DIR` environment variable pointing to your OpenCV build directory might be recognized by the project configuration, potentially simplifying the manual path setup.
5.  **Select Build Configuration:** Choose the desired build configuration (e.g., `Release` or `Debug`) and platform (e.g., `x64`) in Visual Studio.
6.  **Build:** Build the solution using `Build > Build Solution` (Ctrl+Shift+B).
7.  **Run:** The executable (`node-based-image-processor.exe`) will be located in the build output directory (e.g., `x64/Release/`).

## Third-Party Libraries

This project utilizes the following third-party libraries:

*   **OpenCV (Open Source Computer Vision Library):** Used for core image loading, processing, and manipulation functions. ([https://opencv.org/](https://opencv.org/))
*   **Dear ImGui:** Used for creating the graphical user interface elements. ([https://github.com/ocornut/imgui](https://github.com/ocornut/imgui))
*   **ImGui-Node-Editor:** An extension for Dear ImGui used to create the node graph interface. ([https://github.com/thedmd/imgui-node-editor](https://github.com/thedmd/imgui-node-editor))
*   **(Platform/Renderer Abstractions):** The `application/` directory contains code likely using libraries like GLFW, DirectX SDK, or OpenGL drivers depending on the specific build target and platform configuration within Visual Studio.
