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

   // double radius = sqrt(pow((bitmap_width/2),2) + pow((bitmap_height/2),2));
   double radius;
   if (bitmap_height > bitmap_width) {
      radius = ((sqrt(2.0) * (double)bitmap_height) / 2.0);
   }
   else {
      radius = ((sqrt(2.0) * (double)bitmap_width) / 2.0);
   }

   // radius is multiplied by 2 so we can account for negative directions.
   // i.e use the diameter for 180 degrees rather than radius for 360 degrees.
   int accumulator_height = (int)radius;
   int accumulator_width  = 180;

   // Create the 2D array.
   int accumulator[accumulator_width][accumulator_height];

   // Initialize the accumulator to 0's
   for(int w = 0; w < accumulator_width; w++) {
      for(int h = 0; h < accumulator_height; h++) {
         accumulator[w][h] = 0;
      }
   }

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
         if(accumulator[d][r] >= threshold) {

            // See if this point is a local maxima. 
            // We only want local maxima in order to only capture the lines with any meaning.
            int max = accumulator[d][r];
            for (int check_y = -5; check_y <= 5; check_y++) {
               for (int check_x = -5; check_x <= 5; check_x++) {

                  // Make sure our selection is within bounds
                  if( ((check_y + r) >= 0) && ((check_y + r) < accumulator_height) && ((check_x + d) >= 0) && ((check_x + d) < 180)) {
                     if(accumulator[check_x + d][check_y + r] > max) {
                        max = accumulator[check_x + d][check_y + r];

                        // Break outta both loops y'all
                        check_y = 6;
                        check_x = 6;
                     }
                  }
               }
            }

            // See if a different max was found. If so, the current value has no meaning to us.
            if(max > accumulator[d][r]) {
               continue;
            }
            x1 = 0;
            x2 = 0;
            y1 = 0;
            y2 = 0;

            // Check if it's a horizontal line.
            if((d >= 45) && (d <= 135)) {

               // y = (r - x cos(degree)) / sin(degree)
               x1 = 1;
               y1 = ( (double)(r) - ((x1 - center_x) * cos(d * DEG2RAD))) / sin(d * DEG2RAD) + center_y;
               x2 = bitmap_width - 1;
               y2 = ( (double)(r) - ((x2 - center_x) * cos(d * DEG2RAD))) / sin(d * DEG2RAD) + center_y;
               pick_flag = 1;
            }
            else {

               // It's a vertical line
               // x = (r - y sin(degree)) / cos(degree)
               y1 = 1;
               x1 = ( (double)(r) - ((y1 - center_y) * sin(d * DEG2RAD))) / cos(d * DEG2RAD) + center_x;
               y2 = bitmap_height - 1;
               x2 = ( (double)(r) - ((y2 - center_y) * sin(d * DEG2RAD))) / cos(d * DEG2RAD) + center_x;
               pick_flag = 1;
            }

            // // Calculate pairs of possible points on the line, using min and max values of x and y
            // low_y = (r - 1 * cos(d)) / sin(d);
            // low_x = (r - 1 * sin(d)) / cos(d);

            // high_y = (r - (bitmap_width -1) * cos(d)) / sin(d);
            // high_x = (r - (bitmap_height-1) * sin(d)) / cos(d);

            // cout << "for the line: " << r << " = x cos(" << d << ") + y sin(" << d << ")" << endl;
            // cout << "low_y: " << low_y << " low_x: " << low_x << " high_y: " << high_y << " high_x: " << high_x << endl;

            // x1 = 1;
            // x2 = 1;
            // y1 = 1;
            // y2 = 1;

            // // choose first pair
            // if(low_y > 0 && low_y < bitmap_width) {
            //    x1 = 1;
            //    y1 = low_y;
            //    pick_flag = 1;
            // }
            // else if(low_x > 0 && low_x < bitmap_height) {
            //    x1 = low_x;
            //    y1 = 1;
            //    pick_flag = 2;
            // }
            // else if(high_y > 0 && high_y < bitmap_width) {
            //    x1 = bitmap_height;
            //    y1 = high_y;
            //    pick_flag = 3;
            // }
            // else if(high_x > 0 && high_x < bitmap_height) {
            //    x1 = high_x;
            //    y1 = bitmap_width;
            //    pick_flag = 4;
            // }
            // else {
            //    pick_flag = 0;
            // }

            // // choose second pair
            // if(low_y > 0 && low_y < bitmap_width && pick_flag != 1) {
            //    x2 = 1;
            //    y2 = low_y;
            // }
            // else if(low_x > 0 && low_x < bitmap_height && pick_flag != 2) {
            //    x2 = low_x;
            //    y2 = 1;
            // }
            // else if(high_y > 0 && high_y < bitmap_width && pick_flag != 3) {
            //    x2 = bitmap_height-1;
            //    y2 = high_y;
            // }
            // else if(high_x > 0 && high_x < bitmap_height && pick_flag != 4) {
            //    x2 = high_x;
            //    y2 = bitmap_width-1;
            // }
            // else {
            //    pick_flag = 0;
            // }

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

   int width_max  = Assemble_Integer(line_image.info_header.biWidth);
   int height_max = Assemble_Integer(line_image.info_header.biHeight);

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

      if (x >= 0 && x < width_max && y >= 0 && y < height_max) {
         if(steep)
            line_image.image_ptr[y][x] = BLACK;
         else
            line_image.image_ptr[x][y] = BLACK;
      }

      error -= dy;
      if(error < 0) {
         y += ystep;
         error += dx;
      }
   }
}

void outsource_Hough_Transform(bmpBITMAP_FILE &image, int threshold) {
   bmpBITMAP_FILE hough_image;
   Copy_Image (image, hough_image);

   int w = Assemble_Integer(hough_image.info_header.biWidth);
   int h = Assemble_Integer(hough_image.info_header.biHeight);
   int _img_w = Assemble_Integer(hough_image.info_header.biWidth);
   int _img_h = Assemble_Integer(hough_image.info_header.biHeight);

   //Create the accu
   double hough_h = ((sqrt(2.0) * (double)(h>w?h:w)) / 2.0);
   int _accu_h = hough_h * 2.0; // -r -> +r
   int _accu_w = 180;

   unsigned int* _accu = (unsigned int*)calloc(_accu_h * _accu_w, sizeof(unsigned int));

   double center_x = w/2;
   double center_y = h/2;


   for(int y=0;y<h;y++)
   {
      for(int x=0;x<w;x++)
      {
         if( image.image_ptr[y][x] > 250 )
         {
            for(int t=0;t<180;t++)
            {
               double r = ( ((double)x - center_x) * cos((double)t * DEG2RAD)) + (((double)y - center_y) * sin((double)t * DEG2RAD));
               _accu[ (int)((round(r + hough_h) * 180.0)) + t]++;
            }
         }
      }
   }

   std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > lines;

   if(_accu == 0)
      return;

   for(int r=0;r<_accu_h;r++)
   {
      for(int t=0;t<_accu_w;t++)
      {
         if((int)_accu[(r*_accu_w) + t] >= threshold)
         {
            //Is this point a local maxima (9x9)
            int max = _accu[(r*_accu_w) + t];
            for(int ly=-4;ly<=4;ly++)
            {
               for(int lx=-4;lx<=4;lx++)
               {
                  if( (ly+r>=0 && ly+r<_accu_h) && (lx+t>=0 && lx+t<_accu_w)  )
                  {
                     if( (int)_accu[( (r+ly)*_accu_w) + (t+lx)] > max )
                     {
                        max = _accu[( (r+ly)*_accu_w) + (t+lx)];
                        ly = lx = 5;
                     }
                  }
               }
            }
            if(max > (int)_accu[(r*_accu_w) + t])
               continue;


            int x1, y1, x2, y2;
            x1 = y1 = x2 = y2 = 0;

            if(t >= 45 && t <= 135)
            {
               //y = (r - x cos(t)) / sin(t)
               x1 = 0;
               y1 = ((double)(r-(_accu_h/2)) - ((x1 - (_img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (_img_h / 2);
               x2 = _img_w - 0;
               y2 = ((double)(r-(_accu_h/2)) - ((x2 - (_img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (_img_h / 2);
            }
            else
            {
               //x = (r - y sin(t)) / cos(t);
               y1 = 0;
               x1 = ((double)(r-(_accu_h/2)) - ((y1 - (_img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (_img_w / 2);
               y2 = _img_h - 0;
               x2 = ((double)(r-(_accu_h/2)) - ((y2 - (_img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (_img_w / 2);
            }

            lines.push_back(std::pair< std::pair<int, int>, std::pair<int, int> >(std::pair<int, int>(x1,y1), std::pair<int, int>(x2,y2)));

         }
      }
   }
      
   std::cout << "lines: " << lines.size() << " " << threshold << std::endl;

   // Draw the results. 
   std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > >::iterator it;
   for(it=lines.begin();it!=lines.end();it++)
   {
      int x1 = it->first.first;
      int y1 = it->first.second;
      int x2 = it->second.first;
      int y2 = it->second.second;
      cout << "Drawing from " << x1 << ", " << y1 << " to " << x2 << ", " << y2 << endl;
      _draw_line(hough_image, x1, y1, x2, y2);
      // cv::line(img_res, cv::Point(it->first.first, it->first.second), cv::Point(it->second.first, it->second.second), cv::Scalar( 0, 0, 255), 2, 8);
   }
   Copy_Image(hough_image,image);
   Remove_Image(hough_image);
   return;
}