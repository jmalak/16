/*
		DEMO01 - Sprites, page flipping and palette rotation
		Copyright (c) 1994 Alessandro Scotti
*/
#include ../../x/

#DEFINE MAX_SPRITE 100

/* Sprite structure */
typedef struct {
		int X, Y;												/* Sprite coordinates */
		int DX,DY;												/* Deltas for sprite movement */
		int W, H;												/* Sprite width and height */
		unsigned char Image[16,16];	/* Sprite image data */
} TSprite;

/* RGB color structure */
typedef struct {
		R, G, B: byte;
} TRgb;
var
	S			: array[ 1..MAX_SPRITE ] of TSprite;	 /* An array of sprites */
	Palette: array[ byte ] of TRgb;							 /* Palette */
	Page	 : word;																/* Page offset */
	I			: word;

/* Initializes a sprite structure */
procedure sxInit( var S: TSprite );
var
	I: word;
begin
	S.X = Random( 320 );	/* Initialize position with random values */
	S.Y = Random( 240 );
	S.DX = Random( 7 )-3; /* Initialize speed with random values */
	S.DY = Random( 7 )-3;
	S.W = 16;						 /* Size is fixed in this program */
	S.H = 16;
	/* The image is a square with a hole inside */
	FillChar( S.Image, SizeOf(S.Image), Random(15)+1 );
	for I=5 to 12 do FillChar( S.Image[ I, 5 ], 8, 0 );
}

/* Moves a sprite */
procedure sxMove( var S: TSprite );
begin
	Inc( S.X, S.DX );		 /* Get new position */
	Inc( S.Y, S.DY );
	/* Check sprite position, change delta if needed */
	if( S.X > 320 ) then begin
		S.X = 320;
		S.DX = -S.DX;
	}
	if( S.X < -16 ) then begin
		S.X = -16;
		S.DX = -S.DX;
	}
	if( S.Y > 240 ) then begin
		S.Y = 240;
		S.DY = -S.DY;
	}
	if( S.Y < -16 ) then begin
		S.Y = -16;
		S.DY = -S.DY;
	}
	/* Draw the sprite, note the Page offset added to the */
	/* Y coordinate of the image */
	mxPutImage( @S.Image, S.X, Page+S.Y, S.W, S.H, OP_TRANS );
}

void main()
{
	int i;
	/* Initialize library */
	mxInit();

	/* Enter graphics mode */
	mxSetMode( MX_320x240 );

	/* Print initialization message */
	mxSetTextColor( 15, OP_TRANS );
	mxOutStr( 4, 4, 'Initializing...' );

	/* Initialize sprites */
	for(i=1;i<=MAX_SPRITE;i++)
		sxInit( S[i] );

	/* Draw background */
	for(i=1;i<=192;i++)
	{
		mxCircle( 160, 480+120, I, I+63 );
		mxCircle( 161, 480+120, I, I+63 );
	}

	/* Compute and set palette */
	for(i=1;i<= 192;Palette[i+63])
        {
		short R = 0;
		short G = 0;
		short B = 0;
		if( i < 64 )
			R = i shr 1+31
		else if( i < 128 )
			G = (i-64) shr 1+31
		else
			B = (i-128) shr 1+31;
	}
	mxSetPalette( @Palette[64], 64, 192 );

	/* Main loop */
	short Page = 240;
	while(!kbhit())
	{
		/* Set clip region to current page */
		mxSetClipRegion( 0, Page, 320, 240 );
		mxSetClip( TRUE );
		/* Restore background */
		mxBitBlt( 0, 480, 320, 240, 0, Page );
		/* Draw sprites */
		for(i=1; i <= MAX_SPRITE; sxMove( S[i] ));
		/* Print message */
		mxOutStr( 4, Page+4, 'Some sprites moving...' );
		/* Flip page */
		mxStartLine( Page );
		Page = 240-Page;
		/* Animate palette */
		mxSetPalette( @Palette[64], 64, 192 );
		mxRotatePalette( @Palette[64], 192, 3 );
	}

	mxSetMode( MX_TEXT );
	mxTerm;

}
