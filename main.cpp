// main.cpp
// Dustin Johnson
// Micah Most

// Standard header files
#include <fstream.h>
#include <iomanip.h>
#include <iostream>
#include <math.h>
#include <stdlib>
#include <string>

// Classes
#include "image.cpp"
#include "preprocess.cpp"

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
   int FILE_SIZE = 787512;

   string input_file;
   string output_file = "the_box.bmp";

   unsigned char buffer[FILE_SIZE];

   // Get the picture name from the user
   cout << "What is the name of the picture file?" << endl;
   cin >> input_file;

   // Input stream to read in the picture
   ifstream myFile (input_file, ios::in | ios::binary);
   myFile.read (buffer, FILE_SIZE); 

   // Output stream to write out the picture
   ofstream hFile (output_file, ios::out | ios::binary);
   hFile.write (buffer, FILE_SIZE); // Test call, will be omitted later.

}