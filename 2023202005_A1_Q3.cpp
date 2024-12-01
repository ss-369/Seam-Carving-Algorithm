    #include <iostream>
    #include <cmath>
    #include <opencv2/opencv.hpp>

    using namespace std;
    using namespace cv;

    double energy(int*** rgbMatrix,int x, int y,int height,int width) {

            int x1 =y-1;
            if(x1<0) x1=width-1;
            int x2= y+1;
            if(x2>=width) x2 = 0;
            int y1=x-1;
            if(y1<0) y1=height-1;
            int y2=x+1;
            if(y2>=height) y2= 0;


            
                int dxR = rgbMatrix[x][x2][0] - rgbMatrix[x][x1][0];
                int dxG = rgbMatrix[x][x2][1] - rgbMatrix[x][x1][1];
                int dxB = rgbMatrix[x][x2][2] - rgbMatrix[x][x1][2];
                int dyR = rgbMatrix[y2][y][0] - rgbMatrix[y1][y][0];
                int dyG = rgbMatrix[y2][y][1] - rgbMatrix[y1][y][1];
                int dyB = rgbMatrix[y2][y][2] - rgbMatrix[y1][y][2];
                
                double energy = sqrt(pow(dxR, 2) + pow(dxG, 2) + pow(dxB, 2) + pow(dyR, 2) + pow(dyG, 2) + pow(dyB, 2));
                return energy;



        }

    void CostMatrix_vertical(double** CostMatrix , int height , int width){

        for(int i = 1 ; i < height ; i++){
            for(int j = 0 ; j < width ; j++){
                if(j == 0)
                    CostMatrix[i][j] += min(CostMatrix[i-1][j] , CostMatrix[i-1][j+1]);
                else if(j == width-1)
                    CostMatrix[i][j] += min(CostMatrix[i-1][j-1] , CostMatrix[i-1][j]);
                else
                    CostMatrix[i][j] += min(CostMatrix[i-1][j-1] , min(CostMatrix[i-1][j] , CostMatrix[i-1][j+1]));
            }
        }

    }


    int* findVerticalSeam(int seam[],int*** rgbMatrix,int *width,int *height){

            double **energyMatrix = new double*[*height];
        for (int i = 0; i < *height; i++) {
            energyMatrix[i] = new double[*width];
        }

        for (int i = 0; i < *height; ++i) {
            for (int j = 0; j < *width; ++j) {

                energyMatrix[i][j] = energy(rgbMatrix, i, j, *height, *width);
            }
        }


    // double **CostMatrix = energyMatrix;
    double **CostMatrix = new double*[*height];
    for (int i = 0; i < *height; i++) {
        CostMatrix[i] = new double[*width];
    }
    for (int i = 0; i < *height; ++i) {
        for (int j = 0; j < *width; ++j) {
            CostMatrix[i][j] = energyMatrix[i][j];
        }
    }

        CostMatrix_vertical(CostMatrix, *height, *width);

        // Finding the seam
        double minCost = 10000;
        int minCostIndex = 0;
        for (int j = 0; j < *width; ++j) {
            if (CostMatrix[*height-1][j] < minCost) {
                minCost = CostMatrix[*height-1][j];
                minCostIndex = j;
            }
        }

        seam[*height-1] = minCostIndex;

        for (int i = *height-2; i>=0; --i) {

            // Find the next pixel in the seam based on the previous row's seam pixel
            int prevCol = seam[i+1];
            int j = prevCol;
            int tl=j-1;
            int tc=j;
            int tr=j+1;

            int minCol=0;
            if(tl>=0 and CostMatrix[i][tl]>CostMatrix[i][tc]){
                minCol=tc;

            }
            else{
                minCol=tl;
            }
            if(tr<*width and CostMatrix[i][minCol]>CostMatrix[i][tr]){
                minCol=tr;
            }


            seam[i] = minCol;

        }

    for (int i = 0; i < *height; i++) {
        delete[] CostMatrix[i];
    }
    delete[] CostMatrix;

        // Clean up memory
        for (int i = 0; i < *height; i++) {
            delete[] energyMatrix[i];
        }
        delete[] energyMatrix;

        return seam;
    }

            
    void removeVerticalSeam(int seam[], int ***rgbMatrix, int *height, int *width) {
        // Iterate over each row and shift pixels in the seam to the left
        for (int i = 0; i < *height; ++i) {
            for (int j = seam[i]; j < *width - 1; ++j) {
                // Shift pixels to the left
                //<<"14"<<endl;
                rgbMatrix[i][j][0] = rgbMatrix[i][j + 1][0];
                rgbMatrix[i][j][1] = rgbMatrix[i][j + 1][1];
                rgbMatrix[i][j][2] = rgbMatrix[i][j + 1][2];
            }
        }

        // Update width after removing the seam
        *width--;
    }


    void findHorizontalSeam(int seam[])  {
    
    }

    void removeHorizontalSeam( int seam[]) {
    
    }







    void seamCarving(int*** rgbMatrix, int width, int height, int depth, int targetWidth, int targetHeight, const string& outputFilename) {
        // Create a cv::Mat object from the input matrix
        int widthToRemove = width - targetWidth;
        int heightToRemove = height - targetHeight;

        //namedWindow("Seam Carving", WINDOW_NORMAL);
        int* seam = new int[height];

        while (widthToRemove > 0 ) {
        
                seam=findVerticalSeam(seam,rgbMatrix,&width,&height);
                removeVerticalSeam(seam, rgbMatrix, &height, &width);
                --widthToRemove;
                //height--;
        }

            delete[] seam;
    
        Mat resultImage(Size(targetWidth, targetHeight), CV_8UC3);
        for (int y = 0; y < targetHeight; y++) {
            for (int x = 0; x < targetWidth; x++) {
                resultImage.at<cv::Vec3b>(y, x) = Vec3b(rgbMatrix[y][x][0], rgbMatrix[y][x][1], rgbMatrix[y][x][2]);

            }
        }

        // Save the resulting image
        imwrite(outputFilename, resultImage);

    }



    int main() {
        Mat inputImage = imread("sample4.jpeg");
        if (inputImage.empty()) {
            cout << "Could not open or find the image." << endl;
            return 1;
        }

        int targetWidth, targetHeight;
        cout << "Enter target width: ";
        cin >> targetWidth;
        cout << "Enter target height: ";
        cin >> targetHeight;

        if (targetWidth >= inputImage.cols || targetHeight >= inputImage.rows) {
            cout << "Invalid target dimensions. Dimensions must be smaller than the original image." << endl;
            return 1;
        }


            // Extract individual pixel RGB values and load them into a 3D matrix
        int height = inputImage.rows;
        int width = inputImage.cols;

        int depth = 3;

        //Allocate memory for the 3D matrix
        int*** rgbMatrix = new int**[height];
        for (int i = 0; i < height; i++) {
            rgbMatrix[i] = new int*[width];
            for (int j = 0; j < width; j++) {
                rgbMatrix[i][j] = new int[depth];
            }
        }    
    //cout<<"2"<<endl;



    // Copy data from the input image to the 3D matrix
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Vec3b pixel = inputImage.at<Vec3b>(y, x);
            rgbMatrix[y][x][0] = pixel[0];
            rgbMatrix[y][x][1] = pixel[1];
            rgbMatrix[y][x][2] = pixel[2];
        }
    }


        // Perform seam carving and generate the sample image
    seamCarving(rgbMatrix, width, height, depth, targetWidth, targetHeight, "ok.jpg");
    // Deallocate memory for the 3D matrix
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            delete[] rgbMatrix[i][j];
        }
        delete[] rgbMatrix[i];
    }
        delete[] rgbMatrix;


        return 0;
    }

