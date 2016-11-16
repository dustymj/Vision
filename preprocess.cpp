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

         // // Ensure the array is wiped.
         // for (int z = 0; z < 9; z++) {
         //    pixel_values[z] = 0;
         // }

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
         for(int a = i; a < (i + 3); a++) {
            for(int b = j; b < (j + 3); b++) {
               // image.image_ptr[a][b] = ((int(image.image_ptr[a][b]) + median) / 2);
               image.image_ptr[a][b] = median;
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
void Hough_transform(bmpBITMAP_FILE &image, int threshold, int offset) {

   int bitmap_width;
   int bitmap_height;
   int current_val;
   int compare_val;
   int horizontals;
   int hough_histogram[16] = {0};

   bitmap_height = Assemble_Integer(image.info_header.biHeight);
   bitmap_width  = Assemble_Integer(image.info_header.biWidth);

   // int index = 1;
   // Create Hough Histogram
   for (int i = 0 + offset; i < bitmap_height-100; i += 100) {
      for (int j = 0; j < bitmap_width-100; j += 100) {

         for(int a = i; a < i+100; a++) {
            for(int b = j; b < j+100; b++) {
               if(int(image.image_ptr[a][b]) == 0) {
                  current_val = int(image.image_ptr[a][b]);

                  if(a == i) {
                     // horizontals = _check_horizontal(image, a, b, j);
                     // hough_histogram[0] += horizontals;

                     // We're done, that's all we can check
                     b = j+100;
                  }
                  else { // if(a-1 >= i)
                     // horizontals = _check_horizontal(image, a, b, j);
                     // hough_histogram[0] += horizontals;

                     // Check for (~11.25 degree) lines
                     if(b+4 < j+100)
                        if(int(image.image_ptr[a-1][b+4]) == 0) { hough_histogram[1]++; }

                     // Check for (~22.5 degree) lines
                     if(b+3 < j+100)
                        if(int(image.image_ptr[a-1][b+3]) == 0) { hough_histogram[2]++; }

                     // Check for (~33.75 degree) lines
                     if(b+2 < j+100)
                        if(int(image.image_ptr[a-1][b+2]) == 0) { hough_histogram[3]++; }

                     // Check for (45 degree) lines
                     if(b+1 < j+100)
                        if(int(image.image_ptr[a-1][b+1]) == 0) { hough_histogram[4]++; }

                     if(a-2 >= i){

                        // Check for (45 degree) lines
                        if(b+1 < j+100)
                           if(int(image.image_ptr[a-2][b+1]) == 0) { hough_histogram[5]++; }

                        if(a-3 >= i){

                           // Check for (45 degree) lines
                           if(b+1 < j+100)
                              if(int(image.image_ptr[a-3][b+1]) == 0) { hough_histogram[6]++; }

                           if(a-4 >= i){

                              // Check for (45 degree) lines
                              if(b+1 < j+100)
                                 if(int(image.image_ptr[a-4][b+1]) == 0) { hough_histogram[7]++; }

                              // Check for (90 degree) lines
                              if(int(image.image_ptr[a-4][b]) == 0) { hough_histogram[8]++; }

                              // Check for (45 degree) lines
                              if(b-1 > j)
                                 if(int(image.image_ptr[a-4][b-1]) == 0) { hough_histogram[9]++; }
                           }

                           // Check for (45 degree) lines
                           if(b-1 > j)
                              if(int(image.image_ptr[a-3][b-1]) == 0) { hough_histogram[10]++; }
                        }

                        // Check for (45 degree) lines
                        if(b-1 > j)
                           if(int(image.image_ptr[a-2][b-1]) == 0) { hough_histogram[11]++; }
                     }

                     // Check for (135 degree) lines
                     if(b-1 > j)
                        if(int(image.image_ptr[a-1][b-1]) == 0) { hough_histogram[12]++; }

                     // Check for (135 degree) lines
                     if(b-2 > j)
                        if(int(image.image_ptr[a-1][b-2]) == 0) { hough_histogram[13]++; }

                     // Check for (135 degree) lines
                     if(b-3 > j)
                        if(int(image.image_ptr[a-1][b-3]) == 0) { hough_histogram[14]++; }

                     // Check for (135 degree) lines
                     if(b-4 > j)
                        if(int(image.image_ptr[a-1][b-4]) == 0) { hough_histogram[15]++; }
                  }


               }
            }
         }

   //       index++;
   //    }
   // }
   //
   // cout << "Hough Histogram for first block horizontal lines:" << endl;
   // for(int i = 0; i < 16; i++) {
   //    cout << hough_histogram[i] << endl;
   // }
   //
   // index = 1;
   // // Draw in lines
   // for (int i = 0 + offset; i < bitmap_height-100 && index < 2; i += 100) {
   //    cout << "removing stuff" << endl;
   //    for (int j = 0; j < bitmap_width-100; j += 100) {

         for(int a = i; a < i+100; a++) {
            for(int b = j; b < j+100; b++) {
               if(int(image.image_ptr[a][b]) == 0) {

                  if(a == i && hough_histogram[0] < threshold) {


                     // We're done, that's all we can check
                     b = j+100;
                  }
                  else { // if(a-1 >= i)

                     // Check for (~11.25 degree) lines
                     if(b+4 < j+100 && hough_histogram[1] < threshold)
                        image.image_ptr[a-1][b+4] = 255;

                     // Check for (~22.5 degree) lines
                     if(b+3 < j+100 && hough_histogram[2] < threshold)
                        image.image_ptr[a-1][b+3] = 255;

                     // Check for (~33.75 degree) lines
                     if(b+2 < j+100 && hough_histogram[3] < threshold)
                        image.image_ptr[a-1][b+2] = 255;

                     // Check for (45 degree) lines
                     if(b+1 < j+100 && hough_histogram[4] < threshold)
                        image.image_ptr[a-1][b+1] = 255;

                     if(a-2 >= i){

                        // Check for (45 degree) lines
                        if(b+1 < j+100 && hough_histogram[5] < threshold)
                           image.image_ptr[a-2][b+1] = 255;

                        if(a-3 >= i){

                           // Check for (45 degree) lines
                           if(b+1 < j+100 && hough_histogram[6] < threshold)
                              image.image_ptr[a-3][b+1] = 255;

                           if(a-4 >= i){

                              // Check for (45 degree) lines
                              if(b+1 < j+100 && hough_histogram[7] < threshold)
                                 image.image_ptr[a-4][b+1] = 255;

                              // Check for (90 degree) lines
                              if(hough_histogram[8] < threshold)
                                 image.image_ptr[a-4][b] = 255;

                              // Check for (45 degree) lines
                              if(b-1 > j && hough_histogram[9] < threshold)
                                 image.image_ptr[a-4][b-1] = 255;
                           }

                           // Check for (45 degree) lines
                           if(b-1 > j && hough_histogram[10] < threshold)
                              image.image_ptr[a-3][b-1] = 255;
                        }

                        // Check for (45 degree) lines
                        if(b-1 > j && hough_histogram[11] < threshold)
                           image.image_ptr[a-2][b-1] = 255;
                     }

                     // Check for (135 degree) lines
                     if(b-1 > j && hough_histogram[12] < threshold)
                        image.image_ptr[a-1][b-1] = 255;

                     // Check for (135 degree) lines
                     if(b-2 > j && hough_histogram[13] < threshold)
                        image.image_ptr[a-1][b-2] = 255;

                     // Check for (135 degree) lines
                     if(b-3 > j && hough_histogram[14] < threshold)
                        image.image_ptr[a-1][b-3] = 255;

                     // Check for (135 degree) lines
                     if(b-4 > j && hough_histogram[15] < threshold)
                        image.image_ptr[a-1][b-4] = 255;
                  }


               }
            }
         }

            // cout << "Hough Histogram for first block horizontal lines:" << endl;
            // cout << "At block: " << j << endl;
            for(int i = 0; i < 16; i++) {
               // cout << hough_histogram[i] << endl;
               hough_histogram[i] = 0;
            }

         // index++;
      }
   }


}
