// main.cpp
// Dustin Johnson
// Micah Most

// Standard header files
#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

// Classes
#include "image.cpp"
#include "preprocess.cpp"
#include "process.cpp"

// Main function
int main() {

   /* Main outline for the program.

   1. Load in the image, declare all variables as necessary.

   2. Perform preprocessing.
      a. Historgram equalization
      b. Increase contrast
      c. Find zeroes across laplacian
      d. Reduce noise throughout

   3. Find the box
      a. Hough Transformation
   */

   // Global variables
   bmpBITMAP_FILE orig_image;
   bmpBITMAP_FILE copy1;

   Load_Bitmap_File(orig_image);

   Display_FileHeader(orig_image.file_header);
   Display_InfoHeader(orig_image.info_header);
   //copies from orig_image to copy1

   Copy_Image(orig_image, copy1);
   cout << "A copy of the file has been "
        << "made in main memory." << endl;

   Remove_Image(orig_image); // frees dynamic memory too

   cout << "The original image has been "
        << "removed from main memory." << endl << endl
        << "Begin image processing..." << endl;

   // Change_Brightness(copy1, -50);

   Average(copy1, 4);

   Histogram_Equalization(copy1);

   Change_Contrast(copy1, 2.5);

   // Reduce_Noise(copy1);

   // Simple_detect_egdes(copy1, 40);

   Kirsh_detect_egdes(copy1, 7, 550);
   // Kirsh_detect_egdes(copy1, 5, 150);

   cout << "Begin thinning the image" << endl;
   Thin_Edges(copy1);

   // Hough_transform(copy1, 60, 93, 0, 10, false);

   // dustin_Hough_Transform(copy1,170);
   outsource_Hough_Transform(copy1, 170);

   // Magic_eraser(copy1, 1100, 93);

   // Thin_Edges(copy1);

   // Hough_transform(copy1, 20, 46, 0, false);

   // Hough_transform(copy1, 50, 10, false);
   // Hough_transform(copy1, 4, 10, 4, false);

   // Thin_Edges(copy1);
   //
   // Hough_transform(copy1, 50, 100);
   // Hough_transform(copy1, 20, 200);


   // Thin_Edges(copy1);
   //
   // Hough_transform(copy1, 700);
   //
   // Thin_Edges(copy1);
   //
   // Hough_transform(copy1, 700);


   cout << endl << "To show that the copy starts as " <<
      "an exact copy of the original,";

   cout << endl << "Save the copy as a bitmap." << endl;
   Save_Bitmap_File(copy1);

   Remove_Image(copy1);

   return 0;

}
