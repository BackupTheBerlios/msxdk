// Copyright (c) 2004 Arjan Bakker
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


#include <fstream>
using std::ifstream;
using std::ofstream;
using std::ios;
using std::filebuf;


#include <iostream>
using std::streambuf;
using std::cout;
using std::cerr;
using std::endl;

using std::pair;
                
#include <string>
using std::string;

#include <time.h>

#include "OptionParser.h"




static string g_program_name = "";
static int max_depth = 12;
static string extension = "pck";

ofstream outfile;
int bitstream_position;
unsigned char bitdata;
int bitcount;



unsigned char *data;	//data to crunch
int length;				//length of data to crunch


const int buffer_length = 64;
unsigned char outbuffer[ buffer_length ];
int buffer_position;
int bit_position;


// result of a string match
// first = match length
// second = match position
typedef pair<int,int> match_result;



match_result *match_results;


typedef struct occur
{
	int position;
	occur *previous;
} occur;


occur *occur_ptr[ 65536 ];


occur occur_pool[ 65536 * 4] ;
int occur_pointer = 0;

occur *get_occur()
{
	return &occur_pool[ occur_pointer++ ];
}


//return length of opened file
int get_file_length( ifstream & file )
{
int current_position;
int length;

	//not opened file?
	if ( file.fail() )
		return -1;

	//get current file position
	current_position = file.tellg();

	//move to end of file
	file.seekg( 0, ios::end );

	//get current file position (=length of file)
	length = file.tellg();

	//set file position back to original position
	file.seekg( current_position, ios::beg );

	//return found filelength
	return length;
}


// read a file specified by the name
// return file: -1 = failure, 0 = success
int readfile( string name )
{
ifstream infile;

	infile.open( name.c_str() , ios::binary | ios::in );

	if ( infile.fail() )
		return -1;

	if ( ( length = get_file_length( infile ) ) == -1 )
		return -1;

	data = new unsigned char[ length ];

	infile.read( (char*)data, length );

	if ( infile.fail() )
		return -1;

	infile.close();

	return 0;
}


void flush_buffer()
{
int position = 0;

	outfile.write( (char*)outbuffer, bit_position );

	buffer_position = buffer_position - bit_position;

	position = 0;

	while ( bit_position < buffer_length )
		outbuffer[ position++ ] = outbuffer[ bit_position++ ];

	bit_position = 0;
}


void flush_rest()
{
	if ( buffer_position )
		outfile.write( (char*)outbuffer, buffer_position - 1);

	buffer_position = bit_position = 0;
}

//write byte to file
void write_byte(unsigned char value)
{
	outbuffer[ buffer_position++ ] = value;

	if ( buffer_position == buffer_length )
		flush_buffer();
}


void write_bit(int value)
{
	//if 8 bits has been added
	if ( bitcount == 8 )
	{	
		outbuffer[ bit_position ] = bitdata;

		bit_position = buffer_position++;

		if ( buffer_position == buffer_length )
			flush_buffer();

		bitcount = 0;
	}

	//shift value
	bitdata <<= 1;		

	//set bit if value is non-zero
	if ( value )
		bitdata++;		

	//increase number of bits added to value
	bitcount++;			
}


void write_bits( int value, int bits )
{
	//loop through all bits
	for ( int c = 0; c < bits; c++ )
	{
		//write one bit
		write_bit( value & 1 );
	}
}



int get_gamma_size( int value )
{
int gamma_size = 1;

	//increase size if there's still bits left after shifting one bit out
	while ( value )
	{
		value--;

		gamma_size += 2;	//each time 2 extra bits are needed
 
		value >>= 1;
	}

	//return calculated gamma size
	return gamma_size;
}


//0		: 0
//1/2	: 10x
//3-6	: 110xx  (00=3, 01=4, 10=5, 11=6)
//7-14	: 1110xxx (000=7,001=8,010=9,011=10,100=11,101=12,110=13,111=14)
void write_elias_gamma( int value )
{
int bit_size = get_gamma_size( value );
int bit_mask = 1;

	//calculate number of bits needed to encode 
	bit_size--;
	bit_size/=2;

	//write bits to determine bitlength
	for ( int c = 0; c < bit_size; c++ )
	{
		write_bit( 1 );

		bit_mask <<= 1;
	}

	//output bitlength terminator
	write_bit( 0 );

	value++;

	bit_mask >>= 1;

	//write bits to encode value
	for ( int b = 0; b < bit_size; b++ )
	{
		write_bit( value & bit_mask );

		bit_mask>>=1;
	}
}




int writefile( string name )
{
int position = 0;
int offset;
int ext_index = name.rfind( "." );
string output_name;
	
	if ( ext_index < 0 )
		output_name = name + "." + extension;
	else
		output_name = name.substr( 0, ext_index + 1 ) + extension;
	
	cout << "Writing to " << output_name << "..." << endl;
	
	outfile.open( output_name.c_str() , ios::binary | ios::out );

	buffer_position = 1;
	bit_position = 0;

	outfile.write( (char*)&length, 4 );

	while ( position < length )
	{
		
		if ( match_results[ position ].first > 1 )
		{
			offset = position - match_results[ position ].second - 1;

			write_bit( 1 );

			if ( offset > 127 )
			{
				write_byte( offset | 128 );
				write_bit( offset & 1024 );
				write_bit( offset & 512 );
				write_bit( offset & 256 );
				write_bit( offset & 128 );
			}
			else
			{
				write_byte( offset & 127 );
			}

			write_elias_gamma( match_results[ position ].first - 2 );

			position += match_results[ position ].first;
		}
		else
		{
			write_bit( 0 );
			write_byte( data[ position++ ] );
		}
	}

	
	//lz77/rle mark
	write_bit( 1 );		

	//offset is zero -> end of file mark will be checked by RLE decompressing
	write_byte( 0 );
	
	//set 16 bits
	write_bits( 1, 16 );
	
	//gamma bit length terminator
	write_bits( 0, 8 );		

	flush_rest();

	outfile.close();

	return 0;
}


inline int get_match_length( int new_data, int previous_data )
{
int match_length = 2;

	while ( ( new_data < length ) && ( data[ new_data++ ] == data[ previous_data++ ] ) )
		match_length++;

	return match_length;
}


inline int get_pair( int position )
{
	return data[ position ] + ( data[ position + 1 ] << 8 );
}


match_result get_best_match( int position )
{
match_result best_result;
int temp_match_length;
int pair;
occur *cur_occur;

	best_result.first = 0;		//match length
	best_result.second = -1;	//position

	pair = get_pair( position );

	cur_occur = occur_ptr[pair];

	while ( cur_occur != NULL )
	{
		if ( cur_occur->position > ( position - 0x07ff ) ) 
		{
			temp_match_length = get_match_length( position + 2, cur_occur->position + 2 );

			if ( temp_match_length > best_result.first )
			{
				best_result.first = temp_match_length;
				best_result.second = cur_occur->position;
			}

			cur_occur = cur_occur->previous;
		}
		else
			break;
	}

	return best_result;
}



void find_matches()
{
int pair;
occur *cur_occur;
match_result result;
int position = 0;

	//loop through all data, except last element
	//since it can't be the start of a value pair

	while ( position < length - 1 )
	{
		match_result best_result = get_best_match( position );

		pair = get_pair( position );

		cur_occur = get_occur();	//new occur();
		cur_occur->previous = occur_ptr[ pair ];
		cur_occur->position = position;

		occur_ptr[ pair ] = cur_occur;

		if ( best_result.first > 1 )
		{		

			result = best_result;

			for (int i = position; i < position + best_result.first; i++)
			{			
				match_results[ i ].first = result.first--;		//match length
				match_results[ i ].second = result.second++;	//match position
			}

			if ( best_result.second == ( position - 1 ) )
			{
				if ( best_result.first > 16 )
					position += best_result.first - 16;
			}
		}
		

		position++;

	}

}


class match_link
{
public:
	int cost;
	int position;
	match_link *previous;
	match_link *literal;
	match_link *match;
};


int depth;
match_link *best;


match_link match_link_pool[ 65536 ];
int match_link_index;


void find_best( match_link *current )
{
	if ( ++depth == max_depth || current->position == length )
	{
		if ( current->position >= best->position )
		{
			if ( current->position == best->position )
			{
				if ( current->cost < best->cost )
				{
					best = current;
				}
			}
			else
				best = current;
		}
	}
	else
	{
		match_link *literal = &match_link_pool[ match_link_index++ ];

		literal->cost = current->cost + 9;
		literal->position = current->position + 1;
		literal->previous = current;
	
		current->literal = literal;

		find_best( literal );

		if ( match_results[ current->position ].first > 1 )
		{
			match_link *match = &match_link_pool[ match_link_index++ ];

			if ( current->position - match_results[ current->position ].second > 128 )
				match->cost = current->cost + 1 + 12;
			else
				match->cost = current->cost + 1 + 8;

			match->cost += get_gamma_size( match_results[ current->position ].first - 2);

			match->previous = current;

			match->position = current->position + match_results[ current->position ].first;

			current->match = match;

			find_best( match ) ;
		}
	}

	depth--;
}


void strip_matches()
{
match_link start;
int position = 0;

	while( position < length )
	{
		start.cost = 0;
		start.position = position;
		start.previous = NULL;

		depth = 0;

		match_link_index = 0;

		best = &start;

		find_best( &start );

		match_link *current = best;

		while ( current->previous != NULL )
		{
			if ( current->previous->literal == current )
			{
				current = current->previous;

				match_results[ current->position ].first = 0;
			}
			else
			{
				current = current->previous;
			}

		}

		position = best->position;
	}
}




void compress( string file )
{
 	cout << "Compressing: " << file << "... ";
 	cout.flush();
 	
	//read some testdata
	readfile( file );

	//mark each value pair as non-occurring yet
	for ( int i = 0; i < 256 * 256; i++)
		occur_ptr[ i ] = NULL;

	match_results = new match_result[ length ];

	for ( int j = 0; j < length; j++)
	{
		match_results[ j ].first = -1;
		match_results[ j ].second = -1;
	}

	//find matches for the whole file
	find_matches();

	strip_matches();

	writefile( file );

	delete [] data;
	delete [] match_results;
	
}


void print_try( void)
{
	cerr << "Try '" << g_program_name << " -h' for more information." << endl;
}


bool process_options( int argc, char ** argv, int & arg_index )
{
	bool	ret = true;
	
	OptionParser	parser( argc, argv );
	int		option;
	
	while ( ( option = parser.GetOption( "hs:e:" ) ) != -1 )
	{
		switch ( option )
		{
			case 'h':
			
				break;
			case 's':
				{
					string strength = parser.Argument();	
					
					max_depth = atoi( strength.c_str() );
					
					if ( max_depth < 2 || max_depth > 16 )
					{
						cerr << "Incorrect compression strength value." << endl;
						ret = false;
					}	
				}
				break;
				
			case 'e':
			        extension = parser.Argument();
				break;
				
			default:
				cerr << "Unrecognized option: -" << (char)parser.Option() << endl;
				ret = false;
				break;
		}
	}   
	
	arg_index = parser.Index();
	
	if ( !ret )
	{
		print_try();
	}
	
	return ret;
}

bool process_arguments( int argc, char ** argv, int & arg_index )
{
	bool ret = true;
		
	if ( arg_index < argc )
	{
		while ( arg_index < argc )
		        compress( argv[ arg_index++ ] );
	}
	else
	{    
		print_try();
		ret = false;		
	}	

	return ret;	
}
		
int main( int argc, char **argv )
{
clock_t start,finish;

bool ret = true;
int arg_index = 1;

string program_name = argv[ 0 ];

#ifdef MINGW
	// MinGW places the FULL executable path in argv[0], AAARGH!
	size_t	program_name_pos = program_name.rfind( "\\" );
	
	if ( program_name_pos != string::npos )
	{
		 program_name= program_name.substr( program_name_pos + 1 );
	}
#endif
	g_program_name = program_name;

        ret = process_options( argc, argv, arg_index );
        
	if ( ret)
	{             
		start = clock();   
				
		ret = process_arguments( argc, argv, arg_index );
				
		finish = clock();    
		
		cout << "Time elapsed:" << (double)( finish - start ) / CLOCKS_PER_SEC << endl;
	}
	
	return ret ? 0 : 1;
}

