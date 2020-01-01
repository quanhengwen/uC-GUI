/**************************************************************

	The program reads an BMP image file and creates a new
	image that is the negative of the input file.

**************************************************************/

#include "qdbmp_conf.h"
#include "qdbmp.h"
#include <stdio.h>

/* Creates a negative image of the input bitmap file */
int main( int argc, char* argv[] )
{
	UCHAR	r, g, b;
	UINT	width, height;
	UINT	x, y;
	BMP*	bmp;
	HANDLE*	f1;
	HANDLE* f2;

	/* Check arguments */
	if ( argc != 3 )
	{
		fprintf( stderr, "Usage: %s <input file> <output file>\n", argv[ 0 ] );
		return 0;
	}

	/* Read an image file */
	/* Open file */
    f1 = CreateFile(argv[ 1 ], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f1 == INVALID_HANDLE_VALUE && GetLastError() != NO_ERROR) {
      return 0;
    }
	bmp = BMP_ReadFile( f1 );
	BMP_CHECK_ERROR( stdout, -1 );

	/* Get image's dimensions */
	width = BMP_GetWidth( bmp );
	height = BMP_GetHeight( bmp );

	/* Iterate through all the image's pixels */
	for ( x = 0 ; x < width ; ++x )
	{
		for ( y = 0 ; y < height ; ++y )
		{
			/* Get pixel's RGB values */
			BMP_GetPixelRGB( bmp, x, y, &r, &g, &b );

			/* Invert RGB values */
			BMP_SetPixelRGB( bmp, x, y, 255 - r, 255 - g, 255 - b );
		}
	}
	/* Open file */
    f2 = CreateFile(argv[ 2 ], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f2 == INVALID_HANDLE_VALUE && GetLastError() != NO_ERROR) {
      return 0;
    }
	/* Save result */
	BMP_WriteFile( bmp, f2 );
	BMP_CHECK_ERROR( stdout, -2 );


	/* Free all memory allocated for the image */
	BMP_Free( bmp );

	return 0;
}

