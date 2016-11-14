// preprocess.cpp
// This will be used to define various functions to perform preprocessing on
// an image before moving forward to detect a box.

/*------------------------------------------------------------
   Change_Brightness

   INPUTS
   image - Pointer to a bitmap image
   int   - Level of brightness change

   DESCRIPTION
   Changes the brightness of the image

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Change_Brightness(bmpBITMAP_FILE &image, int level) {

   int bitmap_width;
   int bitmap_height;
   int reduction;

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   for (int i = 0; i < bitmap_height; i++) {
      for (int j = 0; j < bitmap_width; j++) {
         reduction = int(image.image_ptr[i][j]) + level;

         if(reduction < 0)
            image.image_ptr[i][j] = 0;
         else
            image.image_ptr[i][j] = reduction;
      }
   }
}

/*------------------------------------------------------------
   Change_Contrast

   INPUTS
   image - Pointer to a bitmap image
   float - Level of contrast change

   DESCRIPTION
   Changes the contrast in the image.

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Change_Contrast(bmpBITMAP_FILE &image, int level) {

   int bitmap_width;
   int bitmap_height;
   int change_level;

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   for (int i = 0; i < bitmap_height; i++) {
      for (int j = 0; j < bitmap_width; j++) {
         change_level = 127 + level * (int(image.image_ptr[i][j]) - 127);

         if(change_level < 0)
            image.image_ptr[i][j] = 0;
         else if(change_level > 255)
            image.image_ptr[i][j] = 255;
         else
            image.image_ptr[i][j] = change_level;
      }
   }
}

/*------------------------------------------------------------
   Histogram_Equalization

   INPUTS
   image - Pointer to a bitmap image

   DESCRIPTION
   Changes the contrast in the image.

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Histogram_Equalization(bmpBITMAP_FILE &image) {

   int bitmap_width;
   int bitmap_height;
   int current_level;
   int sum = 0;
   int max = 0;
   int pixel_count;
   int cumlative_histogram[256];
   int histogram[256] = {0};

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);
   pixel_count   = bitmap_height * bitmap_width;

   // Populate histogram
   for (int i = 0; i < bitmap_height; i++) {
      for (int j = 0; j < bitmap_width; j++) {
         current_level = int(image.image_ptr[i][j]);
         histogram[current_level]++;

         if(current_level > max)
            max = current_level;
      }
   }

   // Populate and normalize the cumlative histogram
   cumlative_histogram[0] = histogram[0];
   for(int i = 0; i < 256; i++) {
      sum += histogram[i];
      cumlative_histogram[i] = sum * max / pixel_count;
   }

   // Write the equalized values back to the image
   for(int i = 0; i < bitmap_height; i++) {
      for (int j = 0; j < bitmap_width; j++) {
         current_level = image.image_ptr[i][j];
         image.image_ptr[i][j] = cumlative_histogram[current_level];
      }
   }
}

/*------------------------------------------------------
   Reduce Noise

   INPUTS
   image - Pointer to a bitmap image

   DESCRIPTION
   This function will take image and look at a 3x3 block of 
   the image, and find the median in all of the pixels. It will
   then assign the median value of the pixels to the entire range.

   RETURNS
   Nothing

   NOTE: Due to the nature of for loops, the outer edge of the 
         pixels won't be affected. However, with the implementation
         of the Hough Transformation later, anything that happens
         along the outside edge will likely be omitted. 
----------------------------------------------------------*/

void Reduce_Noise(bmpBITMAP_FILE &image) {
   int bitmap_width;
   int bitmap_height;
   int median;
   int count;

   int pixel_values[9];

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   // These 2 outside for loops will scan through the entire image
   for(int i = 0; i < bitmap_height; i += 3) {
      for (int j = 0; j < bitmap_width; j += 3) {
         median = 0;
         count  = 0;

         // Ensure the array is wiped.
         for (int z = 0; z < 9; z++) {
            pixel_values[z] = 0;
         }

         // Loop through the specified 3x3 block to find the median
         for(int a = i; a < (i + 3); a++) {
            for(int b = j; b < (j + 3); b++) {
               pixel_values[count] = image.image_ptr[a][b];
               count++;
            }
         }

         // Sort the array, sort built into <algorithm>
         sort(pixel_values, pixel_values + 9);

         // Determine the median
         median = pixel_values[4];

         // Reduce noise in the 3x3 space.
         for(int a = i; ((a < (i + 3)) && (a < bitmap_height)); i++) {
            for(int b = j; ((b < (j + 3)) && (b < bitmap_width)); j++) {
               image.image_ptr[a][b] = ( (image.image_ptr[a][b] + median) / 2);
            }
         }

         // Done
      }
   }

   // Out of all for loops.
}
