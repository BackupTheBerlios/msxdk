// Copyright (c) 2003-2004 Arjan Bakker
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of 
// this software and associated documentation files (the "Software"), to deal in 
// the Software without restriction, including without limitation the rights to 
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
// the Software, and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "convert.h"


static unsigned char positions[220];
static unsigned char songlength;
static unsigned char loopposition;
static unsigned char stereo[24];
static unsigned char tempo;
static unsigned char basefreq;
static unsigned char detune[24];
static unsigned char modulation[48];
static unsigned char startwaves[24];
static unsigned char waves[48];
static unsigned char volumes[48];
static unsigned char songname[50];
static unsigned char wavekit[8];
static unsigned short offsets[80+1];
static unsigned short extoffset;
static unsigned char extdat;
static unsigned char *patterndata;
static unsigned char lfodata[25];


static unsigned char highestpattern = 0;


int write( string filename )
{
ofstream output;
char *userid = "MBMS\x010\x008";

	output.open( filename.c_str(), ios::out | ios::binary ); 
	output.write( (char*)userid, 6 );
	output.write( (char*)&songlength, 1 );
	output.write( (char*)&loopposition,1 );
	output.write( (char*)stereo, 24 );
	output.write( (char*)&tempo, 1 );
	output.write( (char*)&basefreq, 1 );
	output.write( (char*)detune,24 );
	output.write( (char*)modulation, 48 );
	output.write( (char*)startwaves, 24 );
	output.write( (char*)waves, 48 );
	output.write( (char*)volumes, 48 );
	output.write( (char*)songname, 50 );
	output.write( (char*)wavekit, 8 );

	for ( int c = 0; c < ( songlength + 1 ); c++ )
		output.write( (char*)&positions[ c ], 1 );

	for ( int c = 0; c < highestpattern; c++ )
	{
		short offset = offsets[ c ] - offsets[ 0 ]
					+ 1 + 1 + 24 + 1 + 1 + 24 + 48 + 24 + 48 + 48 + 50 + 8
					+ ( songlength +  1) + 2 * highestpattern;

		output.write( (char*)&offset, 2 );
	}

	short length = offsets[ highestpattern ] - offsets[ 0 ];
	output.write( (char*)&length, 2 );
	output.write( (char*)&highestpattern, 1 );

	for ( int c = 0; c < highestpattern; c++ )
	{
		short length = offsets[ c + 1 ] - offsets[ c ];

		output.write( (char*)&patterndata[ offsets[ c ] - offsets[ 0 ] ], length );
	}

	output.write( (char*)lfodata, 25 );

	output.close();

	return true;
}

int read( string filename )
{
ifstream input;
char *editid = "MBMS\x010\x007";
char *userid = "MBMS\x010\x008";
char id[6];

	input.open( filename.c_str(), ios::in | ios::binary );
	input.read( (char*)id, 6 );

	if ( memcmp( id, userid, 6 ) == 0 )
	{
		cerr << filename << " is already a USER file!" << endl;
		return false;
	}

	if ( memcmp( id, editid, 6 ) != 0 )
	{
		cerr << filename << " is not a valid MWM file!" << endl;
		return false;
	}

	input.read( (char*)positions, 220 );	//read position data
	input.read( (char*)&songlength, 1 );
	input.read( (char*)&loopposition, 1 );
	input.read( (char*)stereo, 24 );
	input.read( (char*)&tempo, 1 );
	input.read( (char*)&basefreq, 1 );
	input.read( (char*)detune, 24 );
	input.read( (char*)modulation, 48 );
	input.read( (char*)startwaves, 24 );
	input.read( (char*)waves, 48 );
	input.read( (char*)volumes, 48 );
	input.read( (char*)songname, 50 );
	input.read( (char*)wavekit, 8 );
	input.read( (char*)offsets, 160 );
	input.read( (char*)&extoffset, 2 );
	input.read( (char*)&extdat, 1 );

	patterndata = new unsigned char[ extoffset ];
	input.read( (char*)patterndata, extoffset );

	input.read( (char*)lfodata, 25 );

	input.close();

	highestpattern = 0;

	for ( int c = 0; c < ( songlength + 1 ); c++ )
	{
		if ( positions[ c ] > highestpattern )
			highestpattern = positions[ c ];
	}

	highestpattern++;
	
	offsets[ 80 ] = extoffset + offsets[ 0 ];

	return true;
}


bool convert( string input_name, string output_name, bool clear_songname )
{
bool ret = true;

	if ( read( input_name ) == false )
		ret = false;
	else
	{
		if ( clear_songname == true )
			for ( int i = 0; i < 50; i++ )
				songname[i] = ' ';
			
		ret = write( output_name );
	}
        
	return ret;
}

