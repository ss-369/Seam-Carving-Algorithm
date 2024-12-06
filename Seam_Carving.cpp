#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

using namespace std;
using namespace cv;

// Function to calculate the energy of a pixel using dual-gradient energy function
double calculateEnergy(const Mat &image, int x, int y)
{
    int height = image.rows;
    int width = image.cols;

    // Handle boundary conditions by mirroring
    int left = (y - 1 + width) % width;
    int right = (y + 1) % width;
    int up = (x - 1 + height) % height;
    int down = (x + 1) % height;

    // Calculate gradients in x direction
    Vec3b leftPixel = image.at<Vec3b>(x, left);
    Vec3b rightPixel = image.at<Vec3b>(x, right);
    int dxR = rightPixel[2] - leftPixel[2];
    int dxG = rightPixel[1] - leftPixel[1];
    int dxB = rightPixel[0] - leftPixel[0];

    // Calculate gradients in y direction
    Vec3b upPixel = image.at<Vec3b>(up, y);
    Vec3b downPixel = image.at<Vec3b>(down, y);
    int dyR = downPixel[2] - upPixel[2];
    int dyG = downPixel[1] - upPixel[1];
    int dyB = downPixel[0] - upPixel[0];

    // Calculate the energy using the dual-gradient energy function
    double energyValue = sqrt(pow(dxR, 2) + pow(dxG, 2) + pow(dxB, 2) +
                              pow(dyR, 2) + pow(dyG, 2) + pow(dyB, 2));
    return energyValue;
}

// Function to compute the cost matrix for vertical seams
bool computeCostMatrix_vertical(const Mat &energyMatrix, Mat &CostMatrix)
{
    int height = energyMatrix.rows;
    int width = energyMatrix.cols;

    if (height == 0 || width == 0)
        return false;

    // Initialize the first row of CostMatrix with energy values
    for (int j = 0; j < width; j++)
    {
        CostMatrix.at<double>(0, j) = energyMatrix.at<double>(0, j);
    }

    // Populate the CostMatrix
    for (int i = 1; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            double minPrevCost;
            if (j == 0)
            {
                minPrevCost = min(CostMatrix.at<double>(i - 1, j), CostMatrix.at<double>(i - 1, j + 1));
            }
            else if (j == width - 1)
            {
                minPrevCost = min(CostMatrix.at<double>(i - 1, j - 1), CostMatrix.at<double>(i - 1, j));
            }
            else
            {
                minPrevCost = min(CostMatrix.at<double>(i - 1, j - 1),
                                  min(CostMatrix.at<double>(i - 1, j), CostMatrix.at<double>(i - 1, j + 1)));
            }
            CostMatrix.at<double>(i, j) = energyMatrix.at<double>(i, j) + minPrevCost;
        }
    }
    return true;
}

// Function to find the vertical seam with the least energy
vector<int> findVerticalSeam(const Mat &CostMatrix)
{
    int height = CostMatrix.rows;
    int width = CostMatrix.cols;
    vector<int> seam(height);

    // Find the position of the smallest element in the last row
    double minCost = CostMatrix.at<double>(height - 1, 0);
    int minIndex = 0;
    for (int j = 1; j < width; j++)
    {
        if (CostMatrix.at<double>(height - 1, j) < minCost)
        {
            minCost = CostMatrix.at<double>(height - 1, j);
            minIndex = j;
        }
    }
    seam[height - 1] = minIndex;

    // Trace back the seam from bottom to top
    for (int i = height - 2; i >= 0; i--)
    {
        int prevJ = seam[i + 1];
        // Handle boundary conditions
        int start = max(prevJ - 1, 0);
        int end = min(prevJ + 1, width - 1);
        // Find the position with the minimum cost in the previous row
        double minPrevCost = CostMatrix.at<double>(i, start);
        int minPos = start;
        for (int j = start + 1; j <= end; j++)
        {
            if (CostMatrix.at<double>(i, j) < minPrevCost)
            {
                minPrevCost = CostMatrix.at<double>(i, j);
                minPos = j;
            }
        }
        seam[i] = minPos;
    }

    return seam;
}

// Function to remove a vertical seam from the image
bool removeVerticalSeam(Mat &image, const vector<int> &seam)
{
    int height = image.rows;
    int width = image.cols;

    if (seam.size() != height)
        return false;

    for (int y = 0; y < height; y++)
    {
        int seamX = seam[y];
        if (seamX < 0 || seamX >= width)
        {
            cerr << "Invalid seam position at row " << y << ": " << seamX << endl;
            return false;
        }
        for (int x = seamX; x < width - 1; x++)
        {
            image.at<Vec3b>(y, x) = image.at<Vec3b>(y, x + 1);
        }
    }
    // Crop the image to remove the last column
    image = image(Rect(0, 0, width - 1, height));
    return true;
}

// Function to transpose the image
Mat transposeImage(const Mat &image)
{
    Mat transposed;
    transpose(image, transposed);
    return transposed;
}

// Function to generate a resized image name based on the original name
string generateResizedImageName(const string &originalName)
{
    fs::path originalPath(originalName);
    string stem = originalPath.stem().string();           // Filename without extension
    string extension = originalPath.extension().string(); // Extension with dot
    string resizedName = stem + "_resized" + extension;
    return resizedName;
}

// Function to list all image files in the current directory
vector<string> listImageFiles()
{
    vector<string> imageFiles;
    // Define acceptable image extensions
    vector<string> extensions = {".jpg", ".jpeg", ".png", ".bmp", ".tiff"};

    for (const auto &entry : fs::directory_iterator(fs::current_path()))
    {
        if (entry.is_regular_file())
        {
            string filePath = entry.path().string();
            string ext = entry.path().extension().string();
            // Convert extension to lowercase for case-insensitive comparison
            transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (find(extensions.begin(), extensions.end(), ext) != extensions.end())
            {
                imageFiles.push_back(entry.path().filename().string());
            }
        }
    }

    return imageFiles;
}

// Function to display the seam on the image
void displaySeam(const Mat &image, const vector<int> &seam, const string &windowName)
{
    Mat displayImage = image.clone();
    for (int y = 0; y < seam.size(); y++)
    {
        int seamX = seam[y];
        if (seamX >= 0 && seamX < displayImage.cols)
        {
            displayImage.at<Vec3b>(y, seamX) = Vec3b(0, 0, 255); // Red in BGR
        }
    }
    imshow(windowName, displayImage);
    // Wait briefly to visualize
    waitKey(1);
}

// Main function
int main()
{
    // List all image files in the current directory
    vector<string> imageFiles = listImageFiles();

    if (imageFiles.empty())
    {
        cout << "No image files found in the current directory." << endl;
        return 1;
    }

    // Display the list of images
    cout << "Available Images in Current Directory:" << endl;
    for (size_t i = 0; i < imageFiles.size(); ++i)
    {
        // Optionally, display file sizes
        string filePath = fs::current_path().string() + "/" + imageFiles[i];
        uintmax_t fileSize = fs::file_size(filePath);
        cout << i + 1 << ". " << imageFiles[i] << " (" << fileSize / 1024 << " KB)" << endl;
    }

    // Prompt user to select an image
    int choice;
    cout << "Enter the number corresponding to the image you want to resize: ";
    cin >> choice;

    if (choice < 1 || choice > static_cast<int>(imageFiles.size()))
    {
        cout << "Invalid choice. Exiting program." << endl;
        return 1;
    }

    string inputImagePath = imageFiles[choice - 1];
    cout << "You selected: " << inputImagePath << endl;

    // Read the input image
    Mat inputImage = imread(inputImagePath);
    if (inputImage.empty())
    {
        cout << "Could not open or find the image: " << inputImagePath << endl;
        return 1;
    }

    // Get original dimensions
    int originalWidth = inputImage.cols;
    int originalHeight = inputImage.rows;

    cout << "Original Width: " << originalWidth << ", Original Height: " << originalHeight << endl;

    // Prompt user for target dimensions
    int targetWidth, targetHeight;
    cout << "Enter target width (<= " << originalWidth << "): ";
    cin >> targetWidth;
    cout << "Enter target height (<= " << originalHeight << "): ";
    cin >> targetHeight;

    // Validate target dimensions
    if (targetWidth > originalWidth || targetHeight > originalHeight)
    {
        cout << "Invalid target dimensions. Target width and height must be less than or equal to the original dimensions." << endl;
        return 1;
    }

    // Initialize the image for seam carving
    Mat image = inputImage.clone();

    // Determine how many seams to remove vertically and horizontally
    int verticalSeamsToRemove = originalWidth - targetWidth;
    int horizontalSeamsToRemove = originalHeight - targetHeight;

    // Perform vertical seam carving if needed
    if (verticalSeamsToRemove > 0)
    {
        cout << "Starting vertical seam carving to reduce width by " << verticalSeamsToRemove << " pixels..." << endl;
        namedWindow("Seam Carving - Vertical", WINDOW_NORMAL);
        for (int n = 0; n < verticalSeamsToRemove; n++)
        {
            // Compute energy matrix
            Mat energyMatrix = Mat::zeros(image.size(), CV_64F);
            for (int y = 0; y < image.rows; y++)
            {
                for (int x = 0; x < image.cols; x++)
                {
                    energyMatrix.at<double>(y, x) = calculateEnergy(image, y, x);
                }
            }

            // Compute cost matrix
            Mat costMatrix = Mat::zeros(image.size(), CV_64F);
            if (!computeCostMatrix_vertical(energyMatrix, costMatrix))
            {
                cerr << "Failed to compute cost matrix at seam " << n + 1 << "." << endl;
                exit(1);
            }

            // Find the minimal vertical seam
            vector<int> seam = findVerticalSeam(costMatrix);

            // Display the seam
            displaySeam(image, seam, "Seam Carving - Vertical");

            // Remove the seam
            if (!removeVerticalSeam(image, seam))
            {
                cerr << "Failed to remove vertical seam at seam " << n + 1 << "." << endl;
                exit(1);
            }

            // Optionally, print progress
            if ((n + 1) % 100 == 0 || n == verticalSeamsToRemove - 1)
            {
                cout << "Removed " << n + 1 << " / " << verticalSeamsToRemove << " vertical seams." << endl;
            }

            // Check for user interruption
            if (waitKey(1) == 27)
            { // Exit if 'Esc' is pressed
                cout << "Seam carving interrupted by user." << endl;
                destroyWindow("Seam Carving - Vertical");
                break;
            }
        }
        destroyWindow("Seam Carving - Vertical");
        cout << "Vertical seam carving completed. Current Width: " << image.cols << endl;
    }

    // Perform horizontal seam carving if needed
    if (horizontalSeamsToRemove > 0)
    {
        cout << "Starting horizontal seam carving to reduce height by " << horizontalSeamsToRemove << " pixels..." << endl;
        // Transpose the image to reuse vertical seam carving
        Mat transposedImage = transposeImage(image);
        namedWindow("Seam Carving - Horizontal", WINDOW_NORMAL);
        for (int n = 0; n < horizontalSeamsToRemove; n++)
        {
            // Compute energy matrix
            Mat energyMatrix = Mat::zeros(transposedImage.size(), CV_64F);
            for (int y = 0; y < transposedImage.rows; y++)
            {
                for (int x = 0; x < transposedImage.cols; x++)
                {
                    energyMatrix.at<double>(y, x) = calculateEnergy(transposedImage, y, x);
                }
            }

            // Compute cost matrix
            Mat costMatrix = Mat::zeros(transposedImage.size(), CV_64F);
            if (!computeCostMatrix_vertical(energyMatrix, costMatrix))
            {
                cerr << "Failed to compute cost matrix at seam " << n + 1 << "." << endl;
                exit(1);
            }

            // Find the minimal vertical seam
            vector<int> seam = findVerticalSeam(costMatrix);

            // Display the seam
            displaySeam(transposedImage, seam, "Seam Carving - Horizontal");

            // Remove the seam
            if (!removeVerticalSeam(transposedImage, seam))
            {
                cerr << "Failed to remove horizontal seam at seam " << n + 1 << "." << endl;
                exit(1);
            }

            // Optionally, print progress
            if ((n + 1) % 50 == 0 || n == horizontalSeamsToRemove - 1)
            {
                cout << "Removed " << n + 1 << " / " << horizontalSeamsToRemove << " horizontal seams." << endl;
            }

            // Check for user interruption
            if (waitKey(1) == 27)
            { // Exit if 'Esc' is pressed
                cout << "Seam carving interrupted by user." << endl;
                destroyWindow("Seam Carving - Horizontal");
                break;
            }
        }
        destroyWindow("Seam Carving - Horizontal");
        // Transpose back to original orientation
        image = transposeImage(transposedImage);
        cout << "Horizontal seam carving completed. Current Height: " << image.rows << endl;
    }

    // Generate resized image name based on original name
    string outputImageName = generateResizedImageName(inputImagePath);
    cout << "Resized image will be saved as: " << outputImageName << endl;

    // Save the final resized image
    if (!imwrite(outputImageName, image))
    {
        cerr << "Failed to save the resized image." << endl;
    }
    else
    {
        cout << "Seam carving completed. Resized image saved as " << outputImageName << endl;
    }

    // Display the final image
    namedWindow("Resized Image", WINDOW_NORMAL);
    imshow("Resized Image", image);
    cout << "Press any key in the image window to exit." << endl;
    waitKey(0); // Wait indefinitely until a key is pressed
    destroyWindow("Resized Image");

    return 0;
}
