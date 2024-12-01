# Seam Carving Algorithm

This project implements the **Seam Carving Algorithm**, a content-aware image resizing technique. The algorithm is designed to reduce the width or height of an image by removing seams (paths of pixels) with the lowest energy. This helps resize the image while preserving important visual content.

---

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Input and Output](#input-and-output)
- [Implementation](#implementation)
- [Program Flow](#program-flow)
- [Sample Images](#sample-images)
- [Compilation and Execution](#compilation-and-execution)
- [Results](#results)
- [Dependencies](#dependencies)
- [Future Improvements](#future-improvements)

---

## Introduction

Seam Carving is a powerful algorithm used in content-aware image resizing. Unlike standard resizing methods (which distort the image by scaling pixels), the Seam Carving algorithm removes paths (seams) of least importance, ensuring that critical image details remain intact. This method is particularly useful for applications where image dimensions must be adjusted dynamically while retaining visual fidelity.

---

## Features

1. **Energy Calculation**:
   - Computes an energy map of the image using gradients to identify the importance of each pixel.
   - Higher energy values indicate more important pixels, such as edges and textures.

2. **Seam Identification**:
   - Identifies vertical or horizontal seams with the lowest total energy using dynamic programming.

3. **Seam Removal**:
   - Dynamically removes identified seams, resizing the image while maintaining content integrity.

4. **Extensibility**:
   - Can be adapted to increase image dimensions by inserting seams instead of removing them.

---

## Input and Output

### Input
1. **Image File**: The source image to be resized.
2. **New Dimensions**: The desired width and height of the output image.

### Output
- A resized image with minimal distortion and preserved important details.

---

## Implementation

The implementation follows these steps:

1. **Energy Calculation**:
   - Calculates pixel energy using the gradient of pixel intensity (e.g., Sobel operator).

2. **Dynamic Seam Identification**:
   - Uses dynamic programming to efficiently find the seam with the lowest energy.

3. **Seam Removal**:
   - Removes the identified seam by shifting pixel values to fill the removed path.

4. **Iterative Seam Removal**:
   - Repeats the process iteratively until the image reaches the desired dimensions.

---

## Program Flow

1. Load the input image and extract its RGB pixel values.
2. Calculate the energy map for the image.
3. Use dynamic programming to identify a seam with the lowest energy:
   - Vertical seam for width reduction.
   - Horizontal seam for height reduction.
4. Remove the identified seam from the image.
5. Save and output the resized image.

---

## Sample Images

The repository includes several sample images for testing the algorithm:

| File Name     | Description                |
|---------------|----------------------------|
| `sample1.jpeg`| High-resolution beach image. |
| `sample2.jpeg`| Urban scene with buildings. |
| `sample3.jpg` | Sunset with mountains.       |
| `sample4.jpeg`| Forest and trees in twilight.|
| `sample5.jpeg`| Bird near a reflective lake. |
| `sample6.jpg` | Pool table close-up.         |

---

## Compilation and Execution

### Compilation

To compile the program:
```bash
g++ -o seam_carving 2023202005_A1_Q3.cpp
