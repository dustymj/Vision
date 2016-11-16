// preprocess.cpp
// This will be used to define various functions to perform preprocessing on
// an image before moving forward to detect a box.

int BLACK = 0;
int WHITE = 255;

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

   bmpBITMAP_FILE edges;

   Copy_Image(image, edges);

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   h_edge = bitmap_height % 3;
   w_edge = bitmap_width % 3;

   // for(int i = 0; i < bitmap_height; i++) {
   //    for (int j = 0; j < bitmap_width; j++) {
   //       edges.image_ptr[i][j] = 255;
   //    }
   // }

   int edge_elnt = 0;

   for(int i = 0; i < bitmap_height - (h_edge+4); i++) {
      for (int j = 0; j < bitmap_width - (w_edge+4); j++) {
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
void Magic_eraser(bmpBITMAP_FILE &image, int threshold, int offset) {

   int bitmap_width;
   int bitmap_height;
   int garbage_can;
   int consecutive;

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   for (int i = 0 + offset; i < bitmap_height-11; i += 10) {
      for (int j = 0; j < bitmap_width-11; j += 10) {
         garbage_can = 0;
         // consecutive = 0;

         for(int a = i; a < i+10; a++) {
            for(int b = j; b < j+10; b++) {
               if(int(image.image_ptr[a][b]) == 0) {
                  garbage_can++;
               }
            }
         }

         if(garbage_can > threshold) {

            // count consecutive
            // for(int a = i; a < i+10; a++) {
            //    for(int b = j; b < j+10; b++) {
            //       garbage_can += int(image.image_ptr[a][b]);
            //    }
            // }
            for(int a = i; a < i+10; a++) {
               for(int b = j; b < j+10; b++) {
                  image.image_ptr[a][b] = 255;
               }
            }
         }

      }
   }
}
