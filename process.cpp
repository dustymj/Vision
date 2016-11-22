// process.cpp
// File that contains the processing functions of the box finding program
//

// Preprocessor directive to help convert degrees to radians.
// NOTE: M_PI ought to be defined in the cmath header
#define DEG2RAD M_PI / 180.0

// Helper function to draw lines found from Hough Trasform
void _draw_line(bmpBITMAP_FILE, float, float, float, float);

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
void dustin_Hough_Transform(bmpBITMAP_FILE &image, int threshold) {
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

   double radius = sqrt(pow((bitmap_width/2),2) + pow((bitmap_height/2),2));
   // double radius;
   // if (bitmap_height > bitmap_width) {
   //    radius = ((sqrt(2.0) * (double)bitmap_height) / 2.0);
   // }
   // else {
   //    radius = ((sqrt(2.0) * (double)bitmap_width) / 2.0);
   // }

   // radius is multiplied by 2 so we can account for negative directions.
   // i.e use the diameter for 180 degrees rather than radius for 360 degrees.
   int accumulator_height = (int)radius;
   int accumulator_width  = 180;

   // Create the 2D array.
   int accumulator[accumulator_width][accumulator_height] = {0};

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
               // r = (x - center)cos(theta) + (y - center)sin(theta)
               int r = round(((double)(x - center_x) * cos((double)degree * DEG2RAD)) + ((double)(y - center_y) * sin((double)degree * DEG2RAD)));

               // Increment the location in the array.
               accumulator[degree][r]++;
            }
         }
      }
   }

   int low_x;
   int low_y;
   int high_x;
   int high_y;
   int x1;
   int x2;
   int y1;
   int y2;

   int pick_flag;

   // Scan accumulator and draw lines that have more votes than the threshold
   for(int d = 0; d < 180; d++) {
      for(int r = 0; r < accumulator_height; r++) {
         if(accumulator[d][r] > threshold) {

            // Calculate pairs of possible points on the line, using min and max values of x and y
            low_y = (r - 1 * cos(d)) / sin(d);
            low_x = (r - 1 * sin(d)) / cos(d);

            high_y = (r - (bitmap_width -1) * cos(d)) / sin(d);
            high_x = (r - (bitmap_height-1) * sin(d)) / cos(d);

            cout << "for the line: " << r << " = x cos(" << d << ") + y sin(" << d << ")" << endl;
            cout << "low_y: " << low_y << " low_x: " << low_x << " high_y: " << high_y << " high_x: " << high_x << endl;

            x1 = 1;
            x2 = 1;
            y1 = 1;
            y2 = 1;

            // choose first pair
            if(low_y > 0 && low_y < bitmap_width) {
               x1 = 1;
               y1 = low_y;
               pick_flag = 1;
            }
            else if(low_x > 0 && low_x < bitmap_height) {
               x1 = low_x;
               y1 = 1;
               pick_flag = 2;
            }
            else if(high_y > 0 && high_y < bitmap_width) {
               x1 = bitmap_height;
               y1 = high_y;
               pick_flag = 3;
            }
            else if(high_x > 0 && high_x < bitmap_height) {
               x1 = high_x;
               y1 = bitmap_width;
               pick_flag = 4;
            }
            else {
               pick_flag = 0;
            }

            // choose second pair
            if(low_y > 0 && low_y < bitmap_width && pick_flag != 1) {
               x2 = 1;
               y2 = low_y;
            }
            else if(low_x > 0 && low_x < bitmap_height && pick_flag != 2) {
               x2 = low_x;
               y2 = 1;
            }
            else if(high_y > 0 && high_y < bitmap_width && pick_flag != 3) {
               x2 = bitmap_height-1;
               y2 = high_y;
            }
            else if(high_x > 0 && high_x < bitmap_height && pick_flag != 4) {
               x2 = high_x;
               y2 = bitmap_width-1;
            }
            else {
               pick_flag = 0;
            }

            if(pick_flag != 0) {
               cout << "Drawing from [" << x1 << "," << y1 << "] to [" << x2 << "," << y2 << "]" << endl;
               _draw_line(hough_image,x1,y1,x2,y2);
            }

         }
      }
   }

   Copy_Image(hough_image, image);
   Remove_Image(hough_image);
}


void _draw_line(bmpBITMAP_FILE line_image, float x1, float y1, float x2, float y2) {

   // Bresenham's line algorithm
   bool steep = (fabs(y2 - y1) > fabs(x2 - x1));

   if(steep) {
      swap(x1, y1);
      swap(x2, y2);
   }

   if(x1 > x2) {
      swap(x1, x2);
      swap(y1, y2);
   }

   float dx = x2 - x1;
   float dy = fabs(y2 - y1);

   float error = dx / 2.0f;
   int ystep = (y1 < y2) ? 1 : -1;
   int y = (int)y1;

   int maxX = (int)x2;

   for(int x=(int)x1; x<maxX; x++) {
      if(steep)
         line_image.image_ptr[y][x] = BLACK;
      else
         line_image.image_ptr[x][y] = BLACK;

      error -= dy;
      if(error < 0) {
         y += ystep;
         error += dx;
      }
   }
}
