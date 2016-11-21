// process.cpp
// File that contains the processing functions of the box finding program
// 

// Preprocessor directive to help convert degrees to radians.
// NOTE: M_PI ought to be defined in the cmath header
#define DEG2RAD M_PI / 180.0

/*-----------------------------------------------------------
Hough_Transform

INPUTS 
   image - pointer to an image object. 

DESCRIPTION
   Performs the Hough Transformation on the image will return
   an image with the lines it found. 

RETURNS
   image with the lines that most likely make up the box. 
-----------------------------------------------------------*/
bmpBITMAP_FILE dustin_Hough_Transform(bmpBITMAP_FILE &image) {
   int bitmap_width;
   int bitmap_height;
   bmpBITMAP_FILE hough_image;

   Copy_Image(image, hough_image);
   Change_Brightness(hough_image, WHITE);

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   // The accumulator is a 2D array,
   // with one axis containing the degree of the line, and the other containing the radius. 

   // NOTE: The maxiumum radius is that which extends through the image diagonally,
   //       say, from the upper left down to the lower right. Therefore, before the accumulator
   //       array can be made, we have to know this bound.
   double radius;
   if (bitmap_height > bitmap_width) {
      radius = ((sqrt(2.0) * (double)bitmap_height) / 2.0);
   }
   else {
      radius = ((sqrt(2.0) * (double)bitmap_width) / 2.0);
   }

   // radius is multiplied by 2 so we can account for negative directions.
   // i.e use the diameter for 180 degrees rather than radius for 360 degrees. 
   int accumulator_height = (int)radius * 2;
   int accumulator_width  = 180;

   // Create the 2D array.
   int accumulator[accumulator_width][accumulator_height];

   // Use the center of the image as the reference point for degree and radius. 
   int center_x = bitmap_width / 2;
   int center_y = bitmap_height / 2;

   // Process through the entire image.
   for (int y = 0; y < bitmap_height; y++) {
      for (int x = 0; x < bitmap_width; x++) {

         // If the pixel is marked as an edge element
         if (image.image_ptr[y][x] == BLACK) {

            // Loop through 180 degrees
            for (int degree = 0; degree < 180; degree++) {

               // Determine the radius of the line to the center.
               // r = (x - center)cos(theta) + (y-center)sin(theta)
               int r = round(((double)(x - center_x) * cos((double)degree * DEG2RAD)) + ((double)(y - center_y) * sin((double)degree * DEG2RAD)));
               
               // Increment the location in the array. 
               accumulator[degree][r]++;
            }
         }
      }
   }

   // Now that the accumulator has been created, the points in the 2D array with the most values
   // Will determine where the lines are located in the image. 

   // Process that stuff later.
} 