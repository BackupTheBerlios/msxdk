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


// thanks to Sjoerd for some tips on the encoding of match lengths


#include <fstream>
using std::ifstream;
using std::ofstream;
using std::ios;
using std::filebuf;


#include <iostream>
using std::ostream;
using std::streambuf;
using std::cout;
using std::cerr;
using std::endl;

using std::pair;
                
#include <string>
using std::string;

#include <time.h>

#include "OptionParser.h"


static string g_program_name = "";	// variable to hold the program name
static int max_depth = 12;              // maximum search depth for compression
static string extension = "pck";        // default compressed file extension
static bool g_syntax = false;           // (don't) show the program syntax


unsigned char *data;			// data to crunch
int length;				// length of data to crunch

      
ofstream outfile;                       // stream where compressed data will be written to
unsigned char bitdata;                  // current bit status of bitstream
int bitcount;                           // number of bits written to current byte
const int buffer_length = 64;           // size of buffer for compressed data
unsigned char outbuffer[ buffer_length ];	// buffer with compressed data
int buffer_position;                    // current position to write new bytes to
int bit_position;                       // current position to write bit data to


// result of a string match
// first = match length
// second = match position
typedef pair<int,int> match_result;

// matches for all positions in the data
match_result *match_results;


// struct to store the location of a 2 byte combo 
typedef struct occur
{
	int position;		// location of 2 byte combo
	occur *previous;        // link to previous combo
} occur;


occur *occur_ptr[ 65536 ];      // there are 2^16 possible combo's
occur *occur_pool;             	// array with free occur structs
int occur_index = 0;		// position of next free occur struct

// return the next occur struct (doing so is faster than using new/delete all the time
occur *get_occur()
{
	return &occur_pool[ occur_index++ ];
}


// return length of opened file
int get_file_length( ifstream & file )
{
int current_position;
int length;

	// not opened file?
	if ( file.fail() )
		return -1;

	// get current file position
	current_position = file.tellg();

	// move to end of file
	file.seekg( 0, ios::end );

	// get current file position (=length of file)
	length = file.tellg();

	// set file position back to original position
	file.seekg( current_position, ios::beg );

	// return found filelength
	return length;
}


// read a file specified by the name
// return file: -1 = failure, 0 = success
int readfile( string name )
{
ifstream infile;

	// open file for binary input
	infile.open( name.c_str() , ios::binary | ios::in );

	// check for failure
	if ( infile.fail() )
		return -1;

	// get file length
	if ( ( length = get_file_length( infile ) ) == -1 )
		return -1;

	// allocate space to store data
	data = new unsigned char[ length ];

	// read file content
	infile.read( (char*)data, length );

	// check for failure
	if ( infile.fail() )
		return -1;

	infile.close();

	return 0;
}


// flush data up to the current bit position to outfile
void flush_buffer()
{
int position = 0;

	// write all data before bit position
	outfile.write( (char*)outbuffer, bit_position );

	// set new position to write bytes to
	buffer_position = buffer_position - bit_position;

	// move all data from bit position till end to the beginnen of the buffer
	while ( bit_position < buffer_length )
		outbuffer[ position++ ] = outbuffer[ bit_position++ ];

	// reset bit position
	bit_position = 0;
}


// flush all data from the buffer
void flush_rest()
{
 	// write data, if any
	if ( buffer_position )
		outfile.write( (char*)outbuffer, buffer_position - 1);

	// reset buffer / bit positions
	buffer_position = bit_position = 0;
}


// write byte to buffer
void write_byte( unsigned char value )
{
 	// put byte in stream
	outbuffer[ buffer_position++ ] = value;

	// flush buffer, if full
	if ( buffer_position == buffer_length )
		flush_buffer();
}


// write a bit to the buffer
void write_bit( int value )
{
	//if 8 bits has been added
	if ( bitcount == 8 )
	{	
		// write the bitdata to the buffer
		outbuffer[ bit_position ] = bitdata;

		// set new but position
		bit_position = buffer_position++;

		// if buffer full, flush data
		if ( buffer_position == buffer_length )
			flush_buffer();

		// reset bit count
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


// write value times bits to buffer
void write_bits( int value, int bits )
{
	// write bits to buffer
	for ( int c = 0; c < bits; c++ )
		write_bit( value );
}


// get encoding size of value
int get_gamma_size( int value )
{
int gamma_size = 1;

	// increase size if there's still bits left after shifting one bit out
	while ( value )
	{
		value--;

		gamma_size += 2;	// each time 2 extra bits are needed
 
		value >>= 1;
	}

	// return calculated gamma size
	return gamma_size;
}


// write encoded value to stream
void write_length( int value )
{
unsigned int mask = 32768;

	++value;
	
	// find highest bit set
	while ( !( value & mask ) )
		mask >>= 1;
		
	while( true )
	{
		// if mask equals 1, last bit has been written
		if ( mask == 1 )
		{
			write_bit( 0 );
			return;
		}
		
     		mask >>= 1;
     
     		// more data
     		write_bit(1);
		    
		// write data bit
		write_bit( value & mask );
	}
}

// output compressed data to file name
int write_file( string name )
{
int position = 0;
int offset;
int ext_index = name.rfind( "." );	// find rightmost . in name
string output_name;
ifstream infile;
int output_length;
	
	// if no extension found
	if ( ext_index < 0 )
		output_name = name + "." + extension;	// add extension
	else
		output_name = name.substr( 0, ext_index + 1 ) + extension;	// else replace extension
		
	// open file for binary output
	outfile.open( output_name.c_str() , ios::binary | ios::out );

	// display error if creating file failed
	if ( outfile.fail() )
	{
		cerr << "Failed to create " << output_name.c_str();
		return -1;
	}
		
	// set bit / buffer position
	buffer_position = 1;
	bit_position = 0;

	// output filelength 
	outfile.write( (char*)&length, 4 );

	// loop while there's more compressed data to process
	while ( position < length )
	{
		// if it's a match
		if ( match_results[ position ].first > 1 )
		{
			// calculate offset
			offset = position - match_results[ position ].second - 1;

			// write match marker
			write_bit( 1 );

			// if long offset
			if ( offset > 127 )
			{
				// write lowest 7 bits of offset, plus long offset marker
				write_byte( offset | 128 ); 
				
				// write remaing offset bits
				write_bit( offset & 1024 );
				write_bit( offset & 512 );
				write_bit( offset & 256 );
				write_bit( offset & 128 );
			}
			else
			{
				// write short offset
				write_byte( offset );
			}

			// write length of offset
			write_length( match_results[ position ].first - 2 );

			// updated current position in compressed data
			position += match_results[ position ].first;
		}
		else
		{
			// write literal byte marker
			write_bit( 0 );
			
			// write literal byte
			write_byte( data[ position++ ] );
		}
	}

	
	// write RLE marker
	write_bit( 1 );		

	// write short offset byte
	write_byte( 0 );
	
	// write eof marker
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );   
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );  
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );   
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );  
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );   
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );  
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );   
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );    
	write_bit( 1 ); write_bit( 1 ); write_bit( 1 ); write_bit( 1 );
			
	// write all remaining data
	flush_rest();

	// close output file
	outfile.close();

	// open file for binary input
	infile.open( output_name.c_str() , ios::binary | ios::in );
	
	// get length of compressed data
        output_length = get_file_length( infile );
        
        infile.close();
                       
	return output_length;
}


// get length of match from current position
inline int get_match_length( int new_data, int previous_data )
{
int match_length = 2;

	// increase match length while data matches
	while ( ( new_data < length ) && ( data[ new_data++ ] == data[ previous_data++ ] ) )
		match_length++;

	return match_length;
}


// get position of a 2-byte pair
inline int get_pair( int position )
{
	return data[ position ] + ( data[ position + 1 ] << 8 );
}


// return longest match
match_result get_best_match( int position )
{
match_result best_result;
int temp_match_length;
int pair;
occur *cur_occur;

	best_result.first = 0;		// match length
	best_result.second = -1;	// position

	// get 2 byte pair value
	pair = get_pair( position );

	// get closest match
	cur_occur = occur_ptr[ pair ];

	// keep searching if more matches found
	while ( cur_occur != NULL )
	{
		// if match is still in range of maximum offset
		if ( cur_occur->position > ( position - 0x07ff ) ) 
		{
			// get match length
			temp_match_length = get_match_length( position + 2, cur_occur->position + 2 );

			// if better than best result
			if ( temp_match_length > best_result.first )
			{
				// copy new match result
				best_result.first = temp_match_length;
				best_result.second = cur_occur->position;
			}

			// move to previous match
			cur_occur = cur_occur->previous;
		}
		else
			break;	// match out of range
	}

	return best_result;
}


// find matches for all data
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
		// get match data for current position
		match_result best_result = get_best_match( position );

		// get 2 byte pair value
		pair = get_pair( position );

		// get a new occurance
		cur_occur = get_occur();
		
		// keep a link to previous match
		cur_occur->previous = occur_ptr[ pair ];	
		
		cur_occur->position = position;
                
                // store closest occurance
		occur_ptr[ pair ] = cur_occur;

		// if match found
		if ( best_result.first > 1 )
		{
			result = best_result;

			// write match length / position in match area
			for ( int i = position; i < position + best_result.first; i++)
			{			
				match_results[ i ].first = result.first--;	// match length
				match_results[ i ].second = result.second++;	// match position
			}
			   
			// RLE match found?
			if ( best_result.second == ( position - 1 ) )
			{
				// only skip data if a long match found
				if ( best_result.first > 16 )
					position += best_result.first - 16;
			}   
		}
		
		// continue with next data
		position++;
	}
}


// struct for storing cost / position at current node in search tree
typedef struct match_link
{
	int cost;		// cost in bits to get at this position
	int position;           // current position
	match_link *parent;  	// pointr to parent node
	match_link *literal;    // pointer to next literal data node
	match_link *match;      // pointer to next match data node
} match_link;


int depth;		// current search depth
match_link *best;       // pointer to end of best path


// a pool for getting match_link structs, faster than using new / delete 
match_link match_link_pool[ 65536 ];
int match_link_index;


// find best path from current
void find_best( match_link *current )
{
 	// if maximum search depth reached, or no more data
	if ( ++depth == max_depth || current->position == length )
	{
		// if current position is equal to or better than best position
		if ( current->position >= best->position )
		{
			// if positions are equal
			if ( current->position == best->position )
			{
				// only set best if cost of current is less
				if ( current->cost < best->cost )
					best = current;
			}
			else
				best = current;	// new best end of path
		}
	}
	else
	{
		// get new node for literal path
		match_link *literal = &match_link_pool[ match_link_index++ ];

		// set cost to get here
		literal->cost = current->cost + 9;
		
		// set position 
		literal->position = current->position + 1;
		
		// set parent
		literal->parent = current;
	
		// set literal node
		current->literal = literal;

		// search from literal node
		find_best( literal );

		// if match found
		if ( match_results[ current->position ].first > 1 )
		{
			// get new node for literal match
			match_link *match = &match_link_pool[ match_link_index++ ];

			// set cost according to offset length
			if ( current->position - match_results[ current->position ].second > 128 )
				match->cost = current->cost + 1 + 13;
			else
				match->cost = current->cost + 1 + 8;

			// add encoding size of length to total costs
			match->cost += get_gamma_size( match_results[ current->position ].first - 2);

			// set parent
			match->parent = current;

			// set position from match node
			match->position = current->position + match_results[ current->position ].first;

			// set match node
			current->match = match;

			// search from match node
			find_best( match ) ;
		}
	}

	// decrease search level
	depth--;
}


// remove all matches that won't be encoded
void strip_matches()
{
match_link start;
int position = 0;

	// loop while more matches to remove
	while ( position < length )
	{
		// initialise starting point of tree
		start.cost = 0;
		start.position = position;
		start.parent = NULL;

		// set search level to 0
		depth = 0;

		// start with first element of match_link pool
		match_link_index = 0;

		// set pointer to best path
		best = &start;

		// find best path
		find_best( &start );

		match_link *current = best;

		// backtrack from end of tree
		while ( current->parent != NULL )
		{
			// if literal path was taken
			if ( current->parent->literal == current )
			{
				// go back to previous node
				current = current->parent;

				// remove match 
				match_results[ current->position ].first = 0;
			}
			else
			{
				// go back to previous node
				current = current->parent;
			}
                                                           
		}

		// update position according to best position found
		position = best->position;
	}
}


// compress a file
void compress( string file )
{
int compressed_length;

 	occur_index = 0;	// reset position of first free occur struct
 	
	// try reading the file to be compressed
	if ( readfile( file ) == -1 )
	{
		cerr << "Failed to read " << file << endl;	
		return;
	}	
           
 	cout << "Compressing: " << file << "... ";
 	cout.flush();
 	
	// mark each value pair as non-occurring yet
	for ( int i = 0; i < 256 * 256; i++)
		occur_ptr[ i ] = NULL;

	// create new array for storing match results
	match_results = new match_result[ length ];

	// create new pool for occurances
	occur_pool = new occur[ length ];
	
	// reset match results
	for ( int j = 0; j < length; j++)
	{
		match_results[ j ].first = -1;
		match_results[ j ].second = -1;
	}

	// find matches for the whole file
	find_matches();

	// remove all bad matches
	strip_matches();      

	// write compressed data
	compressed_length = write_file( file );
	
	// output statistics if writing file succeeded
	if ( compressed_length > 0 )
		cout << length << " -> " << compressed_length << endl;
                                      		           
        // remove data
	delete [] occur_pool; 	
	delete [] match_results;
	delete [] data;
}


// print hint for getting syntax help
void print_try( )
{
	cerr << "Try '" << g_program_name << " -h' for more information." << endl;
}


// print program syntax
void print_syntax( ostream & stream = cerr )
{
	stream << "Compress file(s)" << endl;
	stream << endl;
	stream << g_program_name << " [-h] [-s <strength>] [-e <extension>]";
	stream << " <filename> [<filename>...]" << endl;
        stream << endl;
	stream << "  -h             Print this information" << endl;
	stream << "  -s <strength>  Set the compression strength (2...16)" << endl;
	stream << "                 Default strength is 12" << endl;
	stream << "  -e <extension> Set the extension used for compressed file(s)" << endl;
	stream << "                 Default extension is pck" << endl;       
        stream << endl;
}


// process all options
bool process_options( int argc, char ** argv, int & arg_index )
{
bool ret = true;
OptionParser parser( argc, argv );
int option;
	
	// get next option
	while ( ( option = parser.GetOption( "hs:e:" ) ) != -1 )
	{
		switch ( option )
		{
			case 'h':	// print program syntax
			        g_syntax = true;
				break;
				
			case 's':       // set compression strength
				{      	
					// get compression strength
					string strength = parser.Argument();	
					
					// convert to integer
					max_depth = atoi( strength.c_str() );
					
					// print error if < 2 or > 16
					if ( max_depth < 2 || max_depth > 16 )
					{
						cerr << "Incorrect compression strength value." << endl;
						ret = false;
					}	
				}
				break;
				
			case 'e':	// set compressed file extension
			
				//get extension
			        extension = parser.Argument();
				break;
				
			default:	// unrecognized option
				cerr << "Unrecognized option: -" << (char)parser.Option() << endl;
				ret = false;
				break;
		}
	}   
	
	// get index of non-option
	arg_index = parser.Index();
	
	// print some info if errors in options
	if ( !ret )
		print_try();
	
	return ret;
}


// process all arguments, i.e. all files that have to be compressed
bool process_arguments( int argc, char ** argv, int & arg_index )
{
	bool ret = true;
		
	// if files to be compressed are specified
	if ( arg_index < argc )
	{
		// compress all files
		while ( arg_index < argc )
		        compress( argv[ arg_index++ ] );
	}
	else
	{    
		// print some info
		if ( !g_syntax )
			print_try();
			
		ret = false;		
	}	

	return ret;	
}
		
int main( int argc, char **argv )
{
bool ret = true;
int arg_index = 1;

string program_name = argv[ 0 ];

#ifdef MINGW
	// MinGW places the FULL executable path in argv[0], AAARGH!
	size_t	program_name_pos = program_name.rfind( "\\" );
	
	if ( program_name_pos != string::npos )
	{
		 program_name = program_name.substr( program_name_pos + 1 );
	}
#endif
	g_program_name = program_name;

	// process all options
        ret = process_options( argc, argv, arg_index );
        
        // if no errors, compresss files
	if ( ret )
	{       
	        clock_t start, finish;
      
      		// display syntax if needed
		if ( g_syntax )
			print_syntax( cout );
				
		start = clock();   
			
		// process all files	
		ret = process_arguments( argc, argv, arg_index );
				
		finish = clock();    
		
		if ( ret )
			cout << "Time elapsed:" << (double)( finish - start ) / CLOCKS_PER_SEC << endl;
	}
	
	return ret ? 0 : 1;
}

