// preprocess.cpp
// This will be used to define various functions to perform preprocessing on
// an image before moving forward to detect a box.

int BLACK = 0;
int WHITE = 255;

/*------------------------------------------------------------
   Average

   INPUTS
   image - Pointer to a bitmap image

   DESCRIPTION
   Changes the brightness of the image

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Average(bmpBITMAP_FILE &image, int size) {

   int bitmap_width;
   int bitmap_height;
   int average = 0;

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   for (int i = 0; i < bitmap_height-size; i += size) {
      for (int j = 0; j < bitmap_width-size; j += size) {
         average = 0;

         for (int a = i; a < i + size; a++) {
            for (int b = j; b < j + size; b++) {
               average += int(image.image_ptr[a][b]);
            }
         }

         average = average / (size*size);

         for (int a = i; a < i + size; a++) {
            for (int b = j; b < j + size; b++) {
               image.image_ptr[a][b] = average;
            }
         }
      }
   }
}

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
         else if (reduction > 255)
            image.image_ptr[i][j] = 255;
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
   int h_edge;
   int w_edge;

   int pixel_values[9] = {0};

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   h_edge = bitmap_height % 3;
   w_edge = bitmap_width % 3;

   // These 2 outside for loops will scan through the entire image
   for(int i = 0; i < bitmap_height - h_edge; i += 3) {
      for (int j = 0; j < bitmap_width - w_edge; j += 3) {
         median = 0;
         count  = 0;

         // Loop through the specified 3x3 block to find the median
         for(int a = i; a < (i + 3); a++) {
            for(int b = j; b < (j + 3); b++) {
               pixel_values[count] = int(image.image_ptr[a][b]);
               count++;
            }
         }

         // Sort the array, sort built into <algorithm>
         sort(pixel_values, pixel_values + 9);

         // Determine the median
         median = pixel_values[4];

         // Reduce noise in the 3x3 space.
         for(int a = i; ((a < (i + 3)) && (a < bitmap_height)); a++) {
            for(int b = j; ((b < (j + 3)) && (b < bitmap_width)); b++) {
               image.image_ptr[a][b] = ((int(image.image_ptr[a][b]) + median) / 2);
               // image.image_ptr[a][b] = median;
            }
         }

         // Done
      }
   }

   // Out of all for loops.
}

/*------------------------------------------------------------
   Simple_detect_egdes

   INPUTS
   image - Pointer to a bitmap image
   int - Level of difference between pixels

   DESCRIPTION
   Scans the image looking for large differences in pixel value. if the
   difference is larger than the threshold (parameter) then an edge element will
   be marked with black. All other pizels will be colored white.

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Simple_detect_egdes(bmpBITMAP_FILE &image, int threshold) {

   int bitmap_width;
   int bitmap_height;
   int ha;
   int hb;
   int h_diff;
   int va;
   int vb;
   int v_diff;

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   for (int i = 0; i < bitmap_height-1; i++) {
      for (int j = 0; j < bitmap_width-1; j++) {
         ha = int(image.image_ptr[i][j]);
         hb = int(image.image_ptr[i][j+1]);
         vb = int(image.image_ptr[i+1][j+1]);
         h_diff = abs(ha - hb);
         v_diff = abs(ha - vb);

         if(h_diff > threshold || h_diff > threshold)
            image.image_ptr[i][j] = 0;
         else
            image.image_ptr[i][j] = 255;
      }
   }
}

/*------------------------------------------------------------
   Kirsh_detect_egdes

   INPUTS
   image - Pointer to a bitmap image
   int - Level of difference between pixels

   DESCRIPTION
   Scans the image looking for large differences in pixel value. if the
   difference is larger than the threshold (parameter) then an edge element will
   be marked with black. All other pizels will be colored white.

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Kirsh_detect_egdes(bmpBITMAP_FILE &image, int op_size, int threshold) {

   int bitmap_width;
   int bitmap_height;
   int count;
   int h_edge;
   int w_edge;
   int vertical;
   int horizontal;
   int negative_diag;
   int positive_diag;
   int k_three[9] = {0};
   int k_five[25] = {0};
   int k_seven[49] = {0};

   bmpBITMAP_FILE edges;

   Copy_Image(image, edges);

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   h_edge = bitmap_height % op_size;
   w_edge = bitmap_width % op_size;

   int edge_elnt = 0;

   for(int i = 0; i < bitmap_height - (h_edge+1); i++) {
      for (int j = 0; j < bitmap_width - (w_edge+1); j++) {
         count = 0;

         if(op_size == 3) {
            for(int a = i; a < (i + 3); a++) {
               for(int b = j; b < (j + 3); b++) {
                  k_three[count] = int(image.image_ptr[a][b]);
                  count++;
               }
            }

            horizontal    = abs((k_three[0] + k_three[1] + k_three[2]) - (k_three[6] + k_three[7] + k_three[8]));
            positive_diag = abs((k_three[0] + k_three[1] + k_three[3]) - (k_three[5] + k_three[7] + k_three[8]));
            vertical      = abs((k_three[0] + k_three[3] + k_three[6]) - (k_three[2] + k_three[5] + k_three[8]));
            negative_diag = abs((k_three[1] + k_three[2] + k_three[5]) - (k_three[3] + k_three[6] + k_three[7]));

            if(horizontal > threshold || vertical > threshold || positive_diag > threshold || negative_diag > threshold) {
               edge_elnt++;
               edges.image_ptr[i+1][j+1] = 0;
            }
            else
               edges.image_ptr[i+1][j+1] = 255;

         }
         else if(op_size == 5) {
            for(int a = i; a < (i + 5); a++) {
               for(int b = j; b < (j + 5); b++) {
                  k_five[count] = int(image.image_ptr[a][b]);
                  count++;
               }
            }

            horizontal    = abs((k_five[0] + k_five[1] + k_five[2] + k_five[3] + k_five[4] +
                                 k_five[5] + k_five[6] + k_five[7] + k_five[8] + k_five[9]) -
                                (k_five[15] + k_five[16] + k_five[17] + k_five[18] + k_five[19] +
                                 k_five[20] + k_five[21] + k_five[22] + k_five[23] + k_five[24]));

            vertical      = abs((k_five[0] + k_five[5] + k_five[10] + k_five[15] + k_five[20] +
                                 k_five[1] + k_five[6] + k_five[11] + k_five[16] + k_five[21]) -
                                (k_five[3] + k_five[8] + k_five[13] + k_five[18] + k_five[23] +
                                 k_five[4] + k_five[9] + k_five[14] + k_five[19] + k_five[24]));

            positive_diag = abs((k_five[0] + k_five[1] + k_five[2] + k_five[3] + k_five[5] +
                                 k_five[6] + k_five[7] + k_five[10] + k_five[11] + k_five[15]) -
                                (k_five[9] + k_five[13] + k_five[14] + k_five[17] + k_five[18] +
                                 k_five[19] + k_five[21] + k_five[22] + k_five[23] + k_five[24]));

            negative_diag = abs((k_five[1] + k_five[2] + k_five[3] + k_five[4] + k_five[7] +
                                 k_five[8] + k_five[9] + k_five[13] + k_five[14] + k_five[19]) -
                                (k_five[5] + k_five[10] + k_five[11] + k_five[15] + k_five[16] +
                                 k_five[19] + k_five[20] + k_five[21] + k_five[22] + k_five[23]));

            if(horizontal > threshold || vertical > threshold || positive_diag > threshold || negative_diag > threshold) {
               edge_elnt++;
               edges.image_ptr[i+1][j+1] = 0;
            }
            else
               edges.image_ptr[i+1][j+1] = 255;
         }
         else if(op_size == 7) {
            for(int a = i; a < (i + 7); a++) {
               for(int b = j; b < (j + 7); b++) {
                  k_seven[count] = int(image.image_ptr[a][b]);
                  count++;
               }
            }

            horizontal    = abs((k_seven[0] + k_seven[1] + k_seven[2] + k_seven[3] + k_seven[4] + k_seven[5] + k_seven[6] +
                                 k_seven[7] + k_seven[8] + k_seven[9] + k_seven[10] + k_seven[11] + k_seven[12] + k_seven[13] +
                                 k_seven[14] + k_seven[15] + k_seven[16] + k_seven[17] + k_seven[18] + k_seven[19] + k_seven[20]) -
                                (k_seven[28] + k_seven[29] + k_seven[30] + k_seven[31] + k_seven[32] + k_seven[33] + k_seven[34] +
                                 k_seven[35] + k_seven[36] + k_seven[37] + k_seven[38] + k_seven[39] + k_seven[40] + k_seven[41] +
                                 k_seven[42] + k_seven[43] + k_seven[44] + k_seven[45] + k_seven[46] + k_seven[47] + k_seven[48]));

            vertical      = abs((k_seven[0] + k_seven[7] + k_seven[14] + k_seven[21] + k_seven[28] + k_seven[35] + k_seven[42] +
                                 k_seven[1] + k_seven[8] + k_seven[15] + k_seven[22] + k_seven[29] + k_seven[36] + k_seven[43] +
                                 k_seven[2] + k_seven[9] + k_seven[16] + k_seven[23] + k_seven[30] + k_seven[37] + k_seven[44]) -
                                (k_seven[4] + k_seven[11] + k_seven[18] + k_seven[25] + k_seven[32] + k_seven[39] + k_seven[46] +
                                 k_seven[5] + k_seven[12] + k_seven[19] + k_seven[26] + k_seven[33] + k_seven[40] + k_seven[47] +
                                 k_seven[6] + k_seven[13] + k_seven[20] + k_seven[27] + k_seven[34] + k_seven[41] + k_seven[48]));

            positive_diag = abs((k_seven[0] + k_seven[1] + k_seven[2] + k_seven[3] + k_seven[4] + k_seven[5] + k_seven[7] +
                                 k_seven[8] + k_seven[9] + k_seven[10] + k_seven[11] + k_seven[14] + k_seven[15] + k_seven[16] +
                                 k_seven[17] + k_seven[21] + k_seven[22] + k_seven[23] + k_seven[28] + k_seven[29] + k_seven[35]) -
                                (k_seven[13] + k_seven[19] + k_seven[20] + k_seven[25] + k_seven[26] + k_seven[27] + k_seven[31] +
                                 k_seven[32] + k_seven[33] + k_seven[34] + k_seven[37] + k_seven[38] + k_seven[39] + k_seven[40] +
                                 k_seven[41] + k_seven[43] + k_seven[44] + k_seven[45] + k_seven[46] + k_seven[47] + k_seven[48]));

            negative_diag = abs((k_seven[1] + k_seven[2] + k_seven[3] + k_seven[4] + k_seven[5] + k_seven[6] + k_seven[9] +
                                 k_seven[10] + k_seven[11] + k_seven[12] + k_seven[13] + k_seven[17] + k_seven[18] + k_seven[19] +
                                 k_seven[20] + k_seven[25] + k_seven[26] + k_seven[27] + k_seven[33] + k_seven[34] + k_seven[41]) -
                                (k_seven[7] + k_seven[14] + k_seven[15] + k_seven[21] + k_seven[22] + k_seven[23] + k_seven[28] +
                                 k_seven[29] + k_seven[30] + k_seven[31] + k_seven[35] + k_seven[36] + k_seven[37] + k_seven[38] +
                                 k_seven[39] + k_seven[42] + k_seven[43] + k_seven[44] + k_seven[45] + k_seven[46] + k_seven[47]));

            if(horizontal > threshold || vertical > threshold || positive_diag > threshold || negative_diag > threshold) {
               edge_elnt++;
               edges.image_ptr[i+1][j+1] = 0;
            }
            else
               edges.image_ptr[i+1][j+1] = 255;
         }
      }
   }
   cout << "there were: " << edge_elnt << " edge elements detected!" << endl;

   Copy_Image(edges, image);

   Remove_Image(edges);
}


// The following are helper functions for Thin_Edges.
// Thus, they should be thought of as "private".
bool Identical (bmpBITMAP_FILE &a, bmpBITMAP_FILE &b) {
   int height = Assemble_Integer(a.info_header.biHeight);
   int width  = Assemble_Integer(b.info_header.biWidth);
   height--;
   width--;

   for (int i = 1; i < height; i++) {
      for (int j = 1; j < width; j++) {
         if (a.image_ptr[i][j] != b.image_ptr[i][j]) {
            return false;
         }
      }
   }
   return true;
}

bool Lower (bmpBITMAP_FILE &im, int i, int j) {
   if ((im.image_ptr[i][j] == BLACK) && (im.image_ptr[i][j-1] == WHITE)) {
      return true;
   }
   return false;
}

bool Upper (bmpBITMAP_FILE &im, int i, int j) {
   if ((im.image_ptr[i][j] == BLACK) && (im.image_ptr[i][j+1] == WHITE)) {
      return true;
   }
   return false;
}

bool Left (bmpBITMAP_FILE &im, int i, int j) {
   if ((im.image_ptr[i][j] == BLACK) && (im.image_ptr[i-1][j] == WHITE)) {
      return true;
   }
   return false;
}

bool Right (bmpBITMAP_FILE &im, int i, int j) {
   if ((im.image_ptr[i][j] == BLACK) && (im.image_ptr[i+1][j] == WHITE)) {
      return true;
   }
   return false;
}

bool a1 (bmpBITMAP_FILE &im, int i, int j) {
   bool above;
   bool with;
   bool below;

   above = ((im.image_ptr[i-1][j+1] == BLACK) ||
            (im.image_ptr[i][j+1]   == BLACK) ||
            (im.image_ptr[i+1][j+1] == BLACK));
   with  = ((im.image_ptr[i-1][j]   == WHITE) &&
            (im.image_ptr[i][j]     == BLACK) &&
            (im.image_ptr[i+1][j]   == WHITE));
   below = ((im.image_ptr[i-1][j-1] == BLACK) ||
            (im.image_ptr[i][j-1]   == WHITE) ||
            (im.image_ptr[i+1][j-1] == BLACK));
   return (above && with && below);
}

bool a2 (bmpBITMAP_FILE &im, int i, int j) {
   bool left;
   bool with;
   bool right;

   left = ((im.image_ptr[i-1][j-1] == BLACK) ||
           (im.image_ptr[i-1][j]   == BLACK) ||
           (im.image_ptr[i-1][j+1] == BLACK));
   with = ((im.image_ptr[i][j-1]   == WHITE) &&
           (im.image_ptr[i][j]     == BLACK) &&
           (im.image_ptr[i][j+1]   == WHITE));
   right = ((im.image_ptr[i+1][j-1] == BLACK) ||
            (im.image_ptr[i+1][j]   == BLACK) ||
            (im.image_ptr[i+1][j+1] == BLACK));
   return (left && with && right);
}

bool a3 (bmpBITMAP_FILE &im, int i, int j) {
   bool ll;
   bool with;
   bool ur;
   ll = ((im.image_ptr[i-1][j]   == BLACK) ||
         (im.image_ptr[i-1][j-1] == BLACK) ||
         (im.image_ptr[i][j-1]   == BLACK));
   with = ((im.image_ptr[i-1][j+1] == WHITE) &&
           (im.image_ptr[i][j]     == BLACK) &&
           (im.image_ptr[i+1][j-1] == WHITE));
   ur = ((im.image_ptr[i][j+1]   == BLACK) ||
         (im.image_ptr[i+1][j+1] == BLACK) ||
         (im.image_ptr[i+1][j]   == BLACK));
   return (ll && with && ur);
}

bool a4 (bmpBITMAP_FILE &im, int i, int j) {
   bool ul;
   bool with;
   bool lr;
   ul = ((im.image_ptr[i-1][j]   == BLACK) ||
         (im.image_ptr[i-1][j+1] == BLACK) ||
         (im.image_ptr[i][j+1]   == BLACK));
   with = ((im.image_ptr[i-1][j-1] == WHITE) &&
           (im.image_ptr[i][j]     == BLACK) &&
           (im.image_ptr[i+1][j+1] == WHITE));
   lr = ((im.image_ptr[i+1][j]   == BLACK) ||
         (im.image_ptr[i+1][j-1] == BLACK) ||
         (im.image_ptr[i][j-1]   == BLACK));
   return (ul && with && lr);
}

bool IsAnA (bmpBITMAP_FILE &im, int i, int j) {
   return ( a1(im,i,j) || a2(im,i,j) || a3(im,i,j) || a4(im,i,j));
}

bool b1 (bmpBITMAP_FILE &im, int i, int j) {
   bool above;
   bool with;
   above = ((im.image_ptr[i-1][j+1] == BLACK) ||
            (im.image_ptr[i][j+1]   == BLACK) ||
            (im.image_ptr[i+1][j+1] == BLACK));
   with = ((im.image_ptr[i-1][j-1] == WHITE) &&
           (im.image_ptr[i][j-1]   == BLACK) &&
           (im.image_ptr[i][j]     == BLACK) &&
           (im.image_ptr[i+1][j]   == WHITE));
   return (above && with);
}

bool b2 (bmpBITMAP_FILE &im, int i, int j) {
   bool left;
   bool with;
   left = ((im.image_ptr[i-1][j-1] == BLACK) ||
           (im.image_ptr[i-1][j]   == BLACK) ||
           (im.image_ptr[i-1][j+1] == BLACK));
   with = ((im.image_ptr[i][j-1]   == WHITE) &&
           (im.image_ptr[i][j]     == BLACK) &&
           (im.image_ptr[i+1][j]   == BLACK) &&
           (im.image_ptr[i+1][j+1] == WHITE));
   return (left && with);
}

bool b3 (bmpBITMAP_FILE &im, int i, int j) {
   bool below;
   bool with;
   below = ((im.image_ptr[i-1][j-1] == BLACK) ||
            (im.image_ptr[i][j-1]   == BLACK) ||
            (im.image_ptr[i+1][j-1] == BLACK));
   with = ((im.image_ptr[i-1][j]   == WHITE) &&
           (im.image_ptr[i][j]     == BLACK) &&
           (im.image_ptr[i][j+1]   == BLACK) &&
           (im.image_ptr[i+1][j+1] == WHITE));
   return (below && with);
}

bool b4 (bmpBITMAP_FILE &im, int i, int j) {
   bool right;
   bool with;
   right = ((im.image_ptr[i+1][j-1] == BLACK) ||
            (im.image_ptr[i+1][j]   == BLACK) ||
            (im.image_ptr[i+1][j+1] == BLACK));
   with = ((im.image_ptr[i-1][j-1]  == WHITE) &&
           (im.image_ptr[i-1][j]    == BLACK) &&
           (im.image_ptr[i][j]      == BLACK) &&
           (im.image_ptr[i][j+1]    == WHITE));
   return (right && with);
}

/*-----------------------------------------------------------------------------------------------
   Thin_Edges

   INPUTS
   image - Pointer to an image object

   DESCRIPTION
   This subroutine uses the Steinfeld and Rosenfeld algorithm to thin the lines in a given
   image until they are one pixel wide.

   RETURNS
   Nothing
----------------------------------------------------------------------------------------------*/
void Thin_Edges (bmpBITMAP_FILE &image) {
   bmpBITMAP_FILE check_image;
   bmpBITMAP_FILE contour_points;
   bmpBITMAP_FILE final_points;

   int height = Assemble_Integer(image.info_header.biHeight);
   int width  = Assemble_Integer(image.info_header.biWidth);
   int counter = 0;
   int cycle = 0;

   // Readjust height and width so they stay within bounds
   height--;
   width--;

   // Initialize the images
   Copy_Image(image, contour_points);
   Copy_Image(image, final_points);

   // Set the final_points and contour_points image to all white
   Change_Brightness(final_points, WHITE);
   Change_Brightness(contour_points, WHITE);

   // Start the thin loop
   do {
      if (cycle == 0) {
         Copy_Image(image, check_image);
      }

      // Find final points in the image
      for (int i = 1; i < height; i++) {
         for (int j = 1; j < width; j++) {
            if ((IsAnA(image,i,j))                                 ||
                ((cycle == 0) && (b1(image,i,j) || b2(image,i,j))) ||
                ((cycle == 1) && (b3(image,i,j) || b4(image,i,j))) ||
                ((cycle == 2) && (b1(image,i,j) || b4(image,i,j))) ||
                ((cycle == 3) && (b2(image,i,j) || b3(image,i,j)))) {
               final_points.image_ptr[i][j] = BLACK;
            }
         }
      }

      // See if the final_points match the thinned image.
      // If so, break out of the loop.
      if (Identical(final_points, image)) {
         break;
      }

      // Since the final points don't match the thinned image, we need
      // to put the image on a weight loss program.
      for (int i = 1; i < height; i++) {
         for (int j = 1; j < width; j++) {
            if(((cycle == 0) && Lower(image,i,j)) ||
               ((cycle == 1) && Upper(image,i,j)) ||
               ((cycle == 2) && Left(image,i,j))  ||
               ((cycle == 3) && Right(image,i,j))) {
               contour_points.image_ptr[i][j] = BLACK;
            }
            else {
               contour_points.image_ptr[i][j] = WHITE;
            }
         }
      }

      // Thin that thang
      for (int i = 1; i < height; i++) {
         for (int j = 1; j < width; j++) {
            if(contour_points.image_ptr[i][j] == BLACK) {
               image.image_ptr[i][j] = WHITE;
            }
         }
      }

      // But preserve final points
      for (int i = 1; i < height; i++) {
         for (int j = 1; j < width; j++) {
            if(final_points.image_ptr[i][j] == BLACK) {
               image.image_ptr[i][j] = BLACK;
            }
         }
      }

      // Increment the counter, not to exceed 4.
      cycle++;
      counter++;
      if(cycle == 4) {
         cycle = 0;
      }
   } while(!(Identical(image,check_image)));
   // } while(counter < 200);
   cout << "Thinning went through " << counter << " iterations" << endl;

   // At this point, the original image has been thinned. return.
   Remove_Image(final_points);
   Remove_Image(contour_points);
   Remove_Image(check_image);
}


// Check for horizontal (0 degree) lines
int _check_horizontal(bmpBITMAP_FILE &image, int a, int b, int j) {
   int horizontals;

   for(int y = b+1; y < j+100; y++) {
      if(int(image.image_ptr[a][y]) == 0) {
         horizontals++;
      }
   }

   return horizontals;
}

/*------------------------------------------------------------
   Hough_transform

   INPUTS
   image - Pointer to a bitmap image
   int - Level of difference between pixels
   int - Size of window to scan image with
   int - Threshold for difference in standard deviation
   int - Threshold for minimum amount of "presence in the line". When the histogram
         has been created, it checks for lines of a particular type. If there is
         a high enough presence found the line will be drawn.
         Example:

         Checking for a 45 degree angle presence starting at x. Threshold = 4.

         |_|_|_|_|_|x|
         |_|_|_|_|*|_|
         |_|_|_|*|_|_|
         |_|_|_|_|_|_|
         |_|*|_|_|_|_|
         |*|_|_|_|_|_|

         This scenario would pass, and the line would be drawn.

   bool - Show/hide window outline

   DESCRIPTION
   Scans the image looking for large differences in pixel value. if the
   difference is larger than the threshold (parameter) then an edge element will
   be marked with black. All other pizels will be colored white.

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Hough_transform(bmpBITMAP_FILE &image, int reduction, int window, int std_dev_threshold, int presence_threshold, int outline) {

   int bitmap_width;
   int bitmap_height;
   int compare_val;
   int horizontals;
   int threshold;
   int x_inc;
   int y_inc;
   int hough_histogram[16] = {0};

   bmpBITMAP_FILE final_edges;

   Copy_Image(image, final_edges);

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   for (int i = 0; i < bitmap_height; i++) {
      for (int j = 0; j < bitmap_width; j++) {
         final_edges.image_ptr[i][j] = 255;
      }
   }

   // Create Hough Histogram
   // It is setup currently to check for 16 different degrees of angles:
   // We are checking for lines that would be drawn from + to *
   //
   //                      * | *
   //                      * | *
   //                    * * * * *
   //                * * *   |   * * *
   //          - - - - - - - + - * - - - - -
   //                        |
   //
   // The histogram is represented correspondingly:
   // hough_histogram[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
   //
   //                      9 | 7
   //                     10 | 6
   //                   1211 8 5 4
   //               151413   |   3 2 1
   //          - - - - - - - + - 0 - - - - -
   //                        |

   for (int i = 0; i < bitmap_height-window; i += window) {
      for (int j = 0; j < bitmap_width-window; j += window) {

         for(int a = i; a < i+window; a++) {
            for(int b = j; b < j+window; b++) {
               if(int(image.image_ptr[a][b]) == 0) {

                  // Check for horizontal (0 degree) lines
                  x_inc = 2;
                  while(b-x_inc > j) {
                     if(int(image.image_ptr[a][b-x_inc]) == 0) { hough_histogram[0]++; }
                     x_inc += 2;
                  }

                  if(a-1 >= i){

                     // Check for (~11.25 degree) lines
                     x_inc = 4;
                     y_inc = 1;
                     while(b-x_inc > j && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[1]++; }
                        x_inc += 4;
                        y_inc++;
                     }

                     // Check for (~22.5 degree) lines
                     x_inc = 3;
                     y_inc = 1;
                     while(b-x_inc > j && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[2]++; }
                        x_inc += 3;
                        y_inc++;
                     }

                     // Check for (~33.75 degree) lines
                     x_inc = 2;
                     y_inc = 1;
                     while(b-x_inc > j && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[3]++; }
                        x_inc += 2;
                        y_inc++;
                     }

                     // Check for (45 degree) lines
                     x_inc = 2;
                     y_inc = 2;
                     while(b-x_inc > j && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[4]++; }
                        x_inc += 2;
                        y_inc += 2;
                     }

                     if(a-2 >= i){

                        // Check for (~56.25 degree) lines
                        x_inc = 1;
                        y_inc = 2;
                        while(b-x_inc > j && a-y_inc > i) {
                           if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[5]++; }
                           x_inc++;
                           y_inc += 2;
                        }

                        if(a-3 >= i){

                           // Check for (~67.5 degree) lines
                           x_inc = 1;
                           y_inc = 3;
                           while(b-x_inc > j && a-y_inc > i) {
                              if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[6]++; }
                              x_inc++;
                              y_inc += 3;
                           }

                           if(a-4 >= i){

                              // Check for (~78.75 degree) lines
                              x_inc = 1;
                              y_inc = 4;
                              while(b-x_inc > j && a-y_inc > i) {
                                 if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { hough_histogram[7]++; }
                                 x_inc++;
                                 y_inc += 4;
                              }

                              // Check for (90 degree) lines
                              y_inc = 2;
                              while(a-y_inc > i) {
                                 if(int(image.image_ptr[a-y_inc][b]) == 0) { hough_histogram[8]++; }
                                 y_inc += 2;
                              }

                              // Check for (~101.25 degree) lines
                              x_inc = 1;
                              y_inc = 4;
                              while(b+x_inc < j+window && a-y_inc > i) {
                                 if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[9]++; }
                                 x_inc++;
                                 y_inc += 4;
                              }
                           }

                           // Check for (~112.5 degree) lines
                           x_inc = 1;
                           y_inc = 3;
                           while(b+x_inc < j+window && a-y_inc > i) {
                              if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[10]++; }
                              x_inc++;
                              y_inc += 3;
                           }
                        }

                        // Check for (~123.75 degree) lines
                        x_inc = 1;
                        y_inc = 2;
                        while(b+x_inc < j+window && a-y_inc > i) {
                           if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[11]++; }
                           x_inc++;
                           y_inc += 2;
                        }
                     }

                     // Check for (135 degree) lines
                     x_inc = 2;
                     y_inc = 2;
                     while(b+x_inc < j+window && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[12]++; }
                        x_inc += 2;
                        y_inc += 2;
                     }

                     // Check for (~146.25 degree) lines
                     x_inc = 2;
                     y_inc = 1;
                     while(b+x_inc < j+window && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[13]++; }
                        x_inc += 2;
                        y_inc++;
                     }

                     // Check for (~157.5 degree) lines
                     x_inc = 3;
                     y_inc = 1;
                     while(b+x_inc < j+window && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[14]++; }
                        x_inc += 3;
                        y_inc++;
                     }

                     // Check for (~168.75 degree) lines
                     x_inc = 4;
                     y_inc = 1;
                     while(b+x_inc < j+window && a-y_inc > i) {
                        if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { hough_histogram[15]++; }
                        x_inc += 4;
                        y_inc++;
                     }
                  }
               }
            }
         }

         // Measurements of the hough_histogram
         long average = 0;
         long std_dev = 0;
         long intermediate;

         // Find average
         for(int ic = 0; ic < 16; ic++) {
            average += hough_histogram[ic];
         }

         average = average/16;

         // Find standard deviation
         for(int ic = 0; ic < 16; ic++) {
            intermediate = hough_histogram[ic] - average;
            std_dev += pow(intermediate,2);
         }

         std_dev = sqrt(std_dev/16);

         threshold = hough_histogram[0];

         for(int ic = 1; ic < 16; ic++) {
            if(hough_histogram[ic] > threshold) {
               threshold = hough_histogram[ic];
            }
         }

         threshold -= reduction;

         int presence;
         if(std_dev > std_dev_threshold) {

            for(int a = i; a < i+window; a++) {
               for(int b = j; b < j+window; b++) {
                  if(int(image.image_ptr[a][b]) == 0) {

                     // Write in horizontal (0 degree) lines
                     x_inc = 2;
                     presence = 0;

                     // First measure presence
                     while(b-x_inc > j && hough_histogram[0] > threshold) {
                        if(int(image.image_ptr[a][b-x_inc]) == 0) { presence++; }
                        x_inc += 2;
                     }

                     // If presence passes threshold, draw line. (Example in POD)
                     if(presence > presence_threshold) {
                        x_inc = 2;
                        while(b-x_inc > j) {
                           final_edges.image_ptr[a][b-x_inc] = 0;
                           x_inc += 2;
                        }
                     }

                     if(a-1 >= i) {

                        // Write in (~11.25 degree) lines
                        x_inc = 4;
                        y_inc = 1;
                        presence = 0;
                        while(b-x_inc > j && a-y_inc > i && hough_histogram[1] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                           x_inc += 4;
                           y_inc++;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 4;
                           y_inc = 1;
                           while(b-x_inc > j && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                              x_inc += 4;
                              y_inc++;
                           }
                        }

                        // Write in (~22.5 degree) lines
                        x_inc = 3;
                        y_inc = 1;
                        presence = 0;
                        while(b-x_inc > j && a-y_inc > i && hough_histogram[2] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                           x_inc += 3;
                           y_inc++;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 3;
                           y_inc = 1;
                           while(b-x_inc > j && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                              x_inc += 3;
                              y_inc++;
                           }
                        }

                        // Write in (~33.75 degree) lines
                        x_inc = 2;
                        y_inc = 1;
                        presence = 0;
                        while(b-x_inc > j && a-y_inc > i && hough_histogram[3] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                           x_inc += 2;
                           y_inc++;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 2;
                           y_inc = 1;
                           while(b-x_inc > j && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                              x_inc += 2;
                              y_inc++;
                           }
                        }

                        // Write in (45 degree) lines
                        x_inc = 2;
                        y_inc = 2;
                        presence = 0;
                        while(b-x_inc > j && a-y_inc > i && hough_histogram[4] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                           x_inc += 2;
                           y_inc += 2;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 2;
                           y_inc = 2;
                           while(b-x_inc > j && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                              x_inc += 2;
                              y_inc += 2;
                           }
                        }

                        if(a-2 >= i){

                           // Write in (~56.25 degree) lines
                           x_inc = 1;
                           y_inc = 2;
                           presence = 0;
                           while(b-x_inc > j && a-y_inc > i && hough_histogram[5] > threshold) {
                              if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                              x_inc++;
                              y_inc += 2;
                           }

                           if(presence > presence_threshold) {
                              x_inc = 1;
                              y_inc = 2;
                              while(b-x_inc > j && a-y_inc > i) {
                                 final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                                 x_inc++;
                                 y_inc += 2;
                              }
                           }

                           if(a-3 >= i){

                              // Write in (~67.5 degree) lines
                              x_inc = 1;
                              y_inc = 3;
                              presence = 0;
                              while(b-x_inc > j && a-y_inc > i && hough_histogram[6] > threshold) {
                                 if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                                 x_inc++;
                                 y_inc += 3;
                              }

                              if(presence > presence_threshold) {
                                 x_inc = 1;
                                 y_inc = 3;
                                 while(b-x_inc > j && a-y_inc > i) {
                                    final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                                    x_inc++;
                                    y_inc += 3;
                                 }
                              }

                              if(a-4 >= i){

                                 // Write in (~78.75 degree) lines
                                 x_inc = 1;
                                 y_inc = 4;
                                 presence = 0;
                                 while(b-x_inc > j && a-y_inc > i && hough_histogram[7] > threshold) {
                                    if(int(image.image_ptr[a-y_inc][b-x_inc]) == 0) { presence++; }
                                    x_inc++;
                                    y_inc += 4;
                                 }

                                 if(presence > presence_threshold) {
                                    x_inc = 1;
                                    y_inc = 4;
                                    while(b-x_inc > j && a-y_inc > i) {
                                       final_edges.image_ptr[a-y_inc][b-x_inc] = 0;
                                       x_inc++;
                                       y_inc += 4;
                                    }
                                 }

                                 // Write in (90 degree) lines
                                 y_inc = 2;
                                 presence = 0;
                                 while(a-y_inc > i && hough_histogram[8] > threshold) {
                                    if(int(image.image_ptr[a-y_inc][b]) == 0) { presence++; }
                                    y_inc += 2;
                                 }

                                 if(presence > presence_threshold) {
                                    y_inc = 2;
                                    while(a-y_inc > i) {
                                       final_edges.image_ptr[a-y_inc][b] = 0;
                                       y_inc += 2;
                                    }
                                 }

                                 // Write in (~101.25 degree) lines
                                 x_inc = 1;
                                 y_inc = 4;
                                 presence = 0;
                                 while(b+x_inc < j+window && a-y_inc > i && hough_histogram[9] > threshold) {
                                    if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                                    x_inc++;
                                    y_inc += 4;
                                 }

                                 if(presence > presence_threshold) {
                                    x_inc = 1;
                                    y_inc = 4;
                                    while(b+x_inc < j+window && a-y_inc > i) {
                                       final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                                       x_inc++;
                                       y_inc += 4;
                                    }
                                 }
                              }

                              // Write in (~112.5 degree) lines
                              x_inc = 1;
                              y_inc = 3;
                              presence = 0;
                              while(b+x_inc < j+window && a-y_inc > i && hough_histogram[10] > threshold) {
                                 if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                                 x_inc++;
                                 y_inc += 3;
                              }

                              if(presence > presence_threshold) {
                                 x_inc = 1;
                                 y_inc = 3;
                                 while(b+x_inc < j+window && a-y_inc > i) {
                                    final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                                    x_inc++;
                                    y_inc += 3;
                                 }
                              }
                           }

                           // Write in (~123.75 degree) lines
                           x_inc = 1;
                           y_inc = 2;
                           presence = 0;
                           while(b+x_inc < j+window && a-y_inc > i && hough_histogram[11] > threshold) {
                              if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                              x_inc++;
                              y_inc += 2;
                           }

                           if(presence > presence_threshold) {
                              x_inc = 1;
                              y_inc = 2;
                              while(b+x_inc < j+window && a-y_inc > i) {
                                 final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                                 x_inc++;
                                 y_inc += 2;
                              }
                           }
                        }

                        // Write in (135 degree) lines
                        x_inc = 2;
                        y_inc = 2;
                        presence = 0;
                        while(b+x_inc < j+window && a-y_inc > i && hough_histogram[12] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                           x_inc += 2;
                           y_inc += 2;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 2;
                           y_inc = 2;
                           while(b+x_inc < j+window && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                              x_inc += 2;
                              y_inc += 2;
                           }
                        }

                        // Write in (~146.25 degree) lines
                        x_inc = 2;
                        y_inc = 1;
                        presence = 0;
                        while(b+x_inc < j+window && a-y_inc > i && hough_histogram[13] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                           x_inc += 2;
                           y_inc++;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 2;
                           y_inc = 1;
                           while(b+x_inc < j+window && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                              x_inc += 2;
                              y_inc++;
                           }
                        }

                        // Check for (~157.5 degree) lines
                        x_inc = 3;
                        y_inc = 1;
                        presence = 0;
                        while(b+x_inc < j+window && a-y_inc > i && hough_histogram[14] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                           x_inc += 3;
                           y_inc++;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 3;
                           y_inc = 1;
                           while(b+x_inc < j+window && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                              x_inc += 3;
                              y_inc++;
                           }
                        }

                        // Check for (~168.75 degree) lines
                        x_inc = 4;
                        y_inc = 1;
                        presence = 0;
                        while(b+x_inc < j+window && a-y_inc > i && hough_histogram[15] > threshold) {
                           if(int(image.image_ptr[a-y_inc][b+x_inc]) == 0) { presence++; }
                           x_inc += 4;
                           y_inc++;
                        }

                        if(presence > presence_threshold) {
                           x_inc = 4;
                           y_inc = 1;
                           while(b+x_inc < j+window && a-y_inc > i) {
                              final_edges.image_ptr[a-y_inc][b+x_inc] = 0;
                              x_inc += 4;
                              y_inc++;
                           }
                        }
                     }
                  }

                  // Optional parameter allows the outline of the window to be printed out
                  if(outline && (a == i+window-1 || a == i || b == j+window-1 || b == j)) {
                     final_edges.image_ptr[a][b] = 0;
                  }
               }
            }

         }

         // Prints a snap shot of one window of the image.
         // The snap shot will include the number of edge elements that voted for each line,
         // the estabilshed threshold for this window, and the standard deviation of the
         // hough_histogram for this window.
         if(j > 400 && j < 500 && i > 400 && i < 500) {

            for(int a = i; a < i+window; a++) {
               for(int b = j; b < j+window; b++) {
                  if(outline && (a == i+window-3 || a == i || b == j+window-3 || b == j)) {
                     final_edges.image_ptr[a][b] = 0;
                  }
               }
            }

            cout << "Hough Histogram for first block horizontal lines:" << endl;
            for(int i = 0; i < 16; i++) {
               cout << "Angle: " << i << " " << hough_histogram[i] << endl;
            }
            cout << "Threshold: " << threshold << endl;
            cout << "STDDEV: " << std_dev << endl;
         }

         // Reinitilize hough_histogram to zeros.
         for(int i = 0; i < 16; i++) {
            hough_histogram[i] = 0;
         }

      }
   }

   Copy_Image(final_edges, image);
   Remove_Image(final_edges);
}
