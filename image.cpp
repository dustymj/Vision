// image.cpp
// Contains the code that handles bitmap images.

// These headers may not be needed, as they are handled in the main.
#include <stdlib.h>
#include <iomanip.h>
#include <fstream.h>
#include <math.h>

typedef unsigned char byte_t;

struct bmpFILEHEADER {
   byte_t bfType[2]; // Bitmap identifier, Must be "BM"
   byte_t bfSize[4];
   byte_t bfReserved[4];

   // Specifies the location (in bytes) in the file of the image data.
   // Should be equal to sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + sizeof(Palette)
   byte_t bfOffbits[4];
};

struct bmpINFOHEADER {
   
   // Size of the bmpInfoHeader
   byte_t biSize[4];

   // Width of bitmap, in pixels.
   // Change this if size of image is changed.
   byte_t biWidth[4];

   // Height of bitmap, in pixels.
   // Change this if size of image is changed.
   byte_t biHeight[4];

   byte_t biPlanes[2];
   byte_t biBitCount[2];
   byte_t biCompression[4]; // Should be 0 for uncomressed bitmaps
   byte_t biSizeImage[4]; // The size of the padded image, in bytes
   byte_t biXPelsPerMeter[4];
   byte_t biYPelsPermeter[4];
   byte_t biClrUsed[4]; 
   byte_t biClrImportant[4];
};

struct bmpPALLETTE {

   // This will need to be improved if the program is to scale.
   // Unless we change the palette, this will do. 
   byte_t palPalette[1024];
};

// Note: This structure may not be written to file all at once.
//       The two headers may be written normally, but the image
//       requires a write for each line followed by a possilbe
//       1-3 padding bytes.
struct bmpBITMAP_FILE {
   bmpFILEHEADER file_header;
   bmpINFOHEADER info_header;
   bmpPALLETTE   palette;

   // This implementation will not generalize. Fixed at 256 shades of grey. 

   // This points to the image. Allows the allocation of a two 
   // dimensional array dynamically.
   byte_t **image_ptr;
};

/* ----------------------------------------------------------
   open_input_file

   INPUTS
   in_file - Input stream that points to the file. 

   DESCRIPTION
   Gets the name of the input file from the user and opens it for input

   RETURNS
   Nothing
------------------------------------------------------------*/
void open_input_file (ifstream &in_file) {
   char in_file_name[80];

   cout << "Enter the name of the file " << endl << "which contains the bitmap: ";
   cin >> in_file_name;

   in_file.open(in_file_name, ios::in | ios:binary);
   if (!in_file) {
      cerr << "Error opening file \a\a\n", exit(101);
   }

   return;
}

/*-----------------------------------------------------------
   Assemble_Integer

   INPUTS
   bytes - A pointer to an array of unsigned characters (should be 4 bytes)

   DESCRIPTION
   Assembles the bytes into a signed integer and returns the result

   RETURNS
   A signed integer 
-------------------------------------------------------------*/
int Assemble_Integer(unsigned char bytes[]) {
   
   int an_integer;

   an_integer  = int(bytes[0]);
   an_integer += (int(bytes[1]) * 256);
   an_integer += (int(bytes[2]) * 256 * 256);
   an_integer += (int(bytes[3]) * 256 * 256 * 256);

   return an_integer;
}

/*-----------------------------------------------------------
   Display_FileHeader

   INPUTS
   fileheader - A pointer to a bitmaps file header

   DESCRIPTION
   Displays the file header of a bitmap file 

   RETURNS 
   Nothing
------------------------------------------------------------*/
void Display_FileHeader(bmpFILEHEADER &fileheader) {

   cout << "bfType:     " << fileheader.bfType[0] << fileheader.bfType[1] << endl;
   cout << "bfSize:     " << Assemble_Integer(fileheader.bfSize) << endl;
   cout << "bfReserved: " << Assemble_Integer(fileheader.bfReserved) << endl;
   cout << "bfOffbits:  " << Assemble_Integer(fileheader.bfOffbits) << endl;
}

/*----------------------------------------------------------
   Display_InfoHeader

   INPUTS
   infoheader - A pointer to a bitmap's info header

   DESCRIPTION
   Displays the info header of a bitmap file 

   RETURNS 
   Nothing
-----------------------------------------------------------*/
void Display_InfoHeader(bmpINFOHEADER &infoheader) {

   cout << endl << "The bmpInfoHeader contains the following:" << endl;
   cout << "biSize:         " << Assemble_Integer(infoheader.biSize) << endl;
   cout << "biWidth:        " << Assemble_Integer(infoheader.biWidth) << endl;
   cout << "biHeight:       " << Assemble_Integer(infoheader.biHeight) << endl;
   cout << "biPlanes:       " << int(infoheader.biPlanes[0]) + int(infoheader.biPlanes[1]) * 256 << endl;
   cout << "biBitCount:     " << int(infoheader.biBitCount[0]) + int(infoheader.biBitCount[1]) * 256 << endl;
   cout << "biCompression:  " << Assemble_Integer(infoheader.biCompression) << endl;
   cout << "biSizeImage:    " << Assemble_Integer(infoheader.biSizeImage) << endl;
   cout << "biClrUsed:      " << Assemble_Integer(infoheader.biClrUsed) << endl;
   cout << "biClrImportant: " << Assemble_Integer(infoheader.biClrImportant) << endl;
}

/*----------------------------------------------------------
   Calc_Padding

   INPUTS
   pixel_width - int, The width of the image in pixels

   DESCRIPTION
   Each scan line must end on a 4 byte boundry.
   Threfore, if the pixel_width is not evenly divisible
   by 4, extra bytes are added (either 1, 2 or 3 extra
   bytes) when writing each line.  Likewise, when reading
   a bitmap file it may be helpful to remove the padding
   prior to any manipulations.
   This is not needed unless the number of bytes in a row
   are not evenly divisible by 4.

   RETURNS
   int - The number of bytes of pading for an image 
         (either 0,1,2,3)
-------------------------------------------------------------*/
int Calc_Padding(int pixel_width){

   int padding;
   int the_remainder;

   the_remainder = pixel_width % 4;

   switch (the_remainder) {
      case 0: 
         padding = 0;
         break;
      case 1: 
         padding = 3;
         break;
      case 2: 
         padding = 2;
         break;
      case 3:
         padding = 1;
         break;
      default:
         cerr << "Error: Padding was set to " << padding << endl;
         exit(101);
   }

   return padding;
}
                                    
/*------------------------------------------------------------
   Load_Bitmap_File

   INPUTS
   image - A pointer to a bitmap image.

   DESCRIPTION
   Will fill the structure pointed to with info about the .bmp file

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Load_Bitmap_File(bmpBITMAP_FILE &image) {

   ifstream fs_data;

   int bitmap_width;
   int bitmap_height;
   int padding;

   long int cursor1:

   open_input_file(fs_data);
   fs_data.read((char*) &image.fileheader, sizeof(bmpFILEHEADER));
   fs_data.read((char*) &image.infoheader, sizeof(bmpINFOHEADER));
   fs_data.read((char*) &image.palette, sizeof(bmpPALLETTE));

   bitmap_height = Assemble_Integer(image.infoheader.biHeight);
   bitmap_width  = Assemble_Integer(image.infoheader.biWidth);
   padding       = Calc_Padding(bitmap_width);

   // Allocate a 2 dimensional array
   image.image_ptr = new byte_t*[bitmap_height];
   for (int i = 0; i < bitmap_height; i++) {
      image.img_ptr[i] = new byte_t[bitmap_width];
   }

   cursor1 = Assemble_Integer(image.fileheader.bfOffbits);
   fs_data.seekg(cursor1); // Moves cursor to beginning of the image data

   // Load the bytes into the new aray one line at a time
   for (int i = 0; i < bitmap_height; i++) {
      fs_data.read((char*) image.image_ptr[i], bitmap_width);
   }

   fs_data.close();
}

/*------------------------------------------------------------
   Display_Bitmap_File

   INPUTS
   image - Pointer to a bitmap image

   DESCRIPTION
   Displays a bitmap image

   RETURNS
   Nothing
-------------------------------------------------------------*/
void Display_Bitmap_File(bmpBITMAP_FILE &image) {

   int bitmap_width;
   int bitmap_height;

   Display_FileHeader(image.fileheader);
   Display_InfoHeader(image.infoheader);

   bitmap_height = Assemble_Integer(image.infoheader.biHeight);
   bitmap_width  = Assemble_Integer(image.infoheader.biWidth);

   for (int i = 0; i < bitmap_height; i++) {
      for (int j = 0; j < bitmap_width; j++) {
         cout << setw(4) << int(image.image_ptr[i][j]);
      }
   }
}

/*-----------------------------------------------------------
   Copy_Image

   INPUTS
   image_copy - Pointer to the destination image
   image_orig - Pointer to the original image

   DESCRIPTION
   Copies imag_orig to image_copy

   RETURNS
   Nothing
------------------------------------------------------------*/
void Copy_Image(bmpBITMAP_FILE &image_orig,
                bmpBITMAP_FILE &image_copy) {

   int height;
   int width;

   image_copy.fileheader = image_orig.fileheader;
   image_copy.infoheader = image_orig.infoheader;
   image_copy.palette    = image_orig.palette;

   height = Assemble_Integer(image_copy.infoheader.biHeight);
   width  = Assemble_Integer(image_copy.infoheader.biWidth);

   image_copy.image_ptr = new byte_t*[height];

   for (int i = 0; i < height; i++) {
      image_copy.image_ptr[i] = new byte_t[width];
   }

   // Load the bytes into the new array one byte at a time
   for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
         image_copy.image_ptr[i][j] = image_orig.image_ptr[i][j];
      }
   }
}

/*-----------------------------------------------------------
   Remove_Image

   INPUTS
   image - Pointer to an image,
           image.byte_t contains pointers to a 2-dim array

   DESCRIPTION
   Memory that **byte_t points to in the image is freed, and 
   image.bfType[] is set to "XX".

   RETURNS
   Nothing
-----------------------------------------------------------*/
void Remove_Image(bmpBITMAP_FILE &image) {

   int height;
   int width;

   height = Assemble_Integer(image.infoheader.biHeight);
   width  = Assemble_Integer(image.infoheader.biWidth);

   // Delete the dynamic memory
   for (int i = 0; i < height; i++) {
      delete [] image.image_ptr[i];
   }

   delete [] image.image_ptr;

   image.fileheader.bfType[0] = 'X';  // just to mark it as
   image.fileheader.bfType[1] = 'X';  // unused.
   
   // Also, we may wish to initialize all the header
   // info to zero.
}

//================= Save_Bitmap_File =======================
//
void Save_Bitmap_File(bmpBITMAP_FILE &image) {
   
   ofstream fs_data;

   int width;
   int height;
   int padding;
   long int cursor1;
    
   height = Assemble_Integer(image.infoheader.biHeight);
   width  = Assemble_Integer(image.infoheader.biWidth);
    
   Open_Output_File(fs_data);
    
   fs_data.write ((char *) &image.fileheader, sizeof(bmpFILEHEADER));
   
   if (!fs_data.good()) {
      cout << "\aError 101 writing bitmapfileheader";
      cout << " to file.\n";
      exit (101);
   }
    
   fs_data.write ((char *) &image.infoheader, sizeof(bmpINFOHEADER));
   
   if (!fs_data.good()) {
      cout << "\aError 102 writing bitmap";
      cout << " infoheader to file.\n";
      exit (102);
   }
    
   fs_data.write ((char *) &image.palette, sizeof(bmpPALETTE));
   
   if (!fs_data.good()) {
      cout << "\aError 103 writing bitmap palette to";
      cout << "file.\n";
      exit (103);
   }
   
   // This loop writes the image data
   for (int i = 0; i < height; i++) {
      for (int j=0; j<width; j++) {
         fs_data.write((char *) &image.image_ptr[i][j], sizeof(byte_t));
         
         if (!fs_data.good()) {
            cout << "\aError 104 writing bitmap data";
            cout << "to file.\n";
            exit (104);
         }
      }
   }

   fs_data.close();
}

//=================== Open_Output_File =====================
//
void Open_Output_File(ofstream &out_file) {
   char out_file_name[80];

   cout << "Save file as: ";
   cin >> out_file_name;

   out_file.open(out_file_name, ios::out | ios::binary);

   if (!out_file) {
      cout << endl << "Cannot open " << out_file_name << endl;
      exit(101);
   }

   return;
}