


#include <fstream>
using std::ifstream;
using std::ofstream;
using std::ios;
using std::filebuf;


#include <iostream>
using std::streambuf;
using std::cout;
using std::endl;

using std::pair;


#include <time.h>


short *rle_lengths;

ofstream outfile;
int bitstream_position;
unsigned char bitdata;
int bitcount;


unsigned char *data;	//data to crunch
int length;				//length of data to crunch


const int buffer_length = 16*1;	//16384;

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
int readfile( char *name )
{
ifstream infile;

	infile.open( name, ios::binary | ios::in );

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


int writefile( char *name )
{
int position = 0;
int offset;

	outfile.open( "output.dat" , ios::binary | ios::out);

	buffer_position = 1;
	bit_position = 0;

	outfile.write( (char*)&length, 4);

	while (position < length)
	{
		if ( rle_lengths[ position ] > 1 &&
			 rle_lengths[ position ] >= match_results[ position ].first )
		{
			write_bit( 1 );

			write_byte( 0 );

			write_elias_gamma( rle_lengths[ position ] - 2 );

			position += rle_lengths[ position ];
		}
		else
		{
			if ( match_results[position].first > 1 )
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
	}

	
	//lz77/rle mark
	write_bit(1);		

	//offset is zero -> end of file mark will be checked by RLE decompressing
	write_byte(0);
	
	//set 16 bits
	write_bits(1,16);
	
	//gamma bit length terminator
	write_bits(0,8);		

	flush_rest();

	outfile.close();

	return 0;
}

inline int get_match_length( int new_data, int previous_data )
{
int match_length = 2;

	while ( (new_data < length) && (data[new_data++] == data[previous_data++]) )
		match_length++;

	return match_length;
}


//returns the length of a string
inline int get_rle_length( int position, int value )
{
int rle_length = 1;

	while ( data[++position] == value && (position < length-1))
		rle_length++;

	return rle_length;
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
			cur_occur = NULL;
	}

	return best_result;
}


void find_matches()
{
int pair;
occur *cur_occur;
int rle_length = 0;
int position = 0;
int temp_rle_length = 0;

	//loop through all data, except last element
	//since it can't be the start of a value pair

	while ( position < length - 1)
	{
		//get rle length of current string
		rle_length = get_rle_length( position, data[position] );

		//calc pair number
		pair = get_pair( position );

		// rle length of 2 or more characters saves space!
		if ( rle_length > 2)
		{	
			rle_lengths[ position + 1] = rle_length - 1;

			cur_occur = new occur();
			cur_occur->previous = occur_ptr[ pair ];
			cur_occur->position = position;
	
			occur_ptr[ pair ] = cur_occur;

			cur_occur = new occur();
			cur_occur->previous = occur_ptr[ pair ];
			cur_occur->position = position + rle_length - 2;

			cur_occur = occur_ptr[ get_pair( position + rle_length ) ];

			match_result best_result;

			best_result.first = 0;
			best_result.second = 0;

			int match_length;
			int match_start = 0;
			//cout << "RLE: " << rle_length << endl;

			while ( cur_occur != NULL)
			{
				if ( cur_occur->position > position + rle_length - 0x07ff )
				{
					if ( data[ cur_occur->position - 1 ] == data[ position ] )
					{
						match_length = get_match_length( cur_occur->position + 2,
															position + rle_length + 2 );

						int prev_rle_length = 1;
						int prev_rle_position = cur_occur->position - 2;

						while ( prev_rle_position >= 0 &&
							data[ prev_rle_position ] == data[ position ] )
						{
							prev_rle_position--;
							prev_rle_length++;
						}

				//		cout << "PREV:" << prev_rle_length << endl;

						if ( match_length + prev_rle_length > best_result.first )
						{
							if ( prev_rle_length > rle_length )
							{
								best_result.first = rle_length + match_length;
								best_result.second = cur_occur->position - rle_length;
								match_start = position;
							}
							else
							{
								best_result.first = prev_rle_length + match_length;
								best_result.second = cur_occur->position - prev_rle_length;
								match_start = position + rle_length - prev_rle_length;

							//	cout << "START" << match_start << endl;
						//		cout << "LENG:" << match_length << endl;
							}
						}
					}

					cur_occur = cur_occur->previous;
				}
				else
					cur_occur = NULL;	//out of range
			}

			if ( best_result.first > 3 )
			{
				match_results[ match_start ].first = best_result.first;
				match_results[ match_start ].second = best_result.second;
			}

			//skip over rle sequence
			position = position + rle_length;
		}
		else
		{
			match_result best_result = get_best_match( position );

			if ( best_result.first > 1 )
			{
				for (int j = position; j < position + best_result.first; j++)
				{
					pair = get_pair( j );

					cur_occur = new occur();
					cur_occur->previous = occur_ptr[ pair ];
					cur_occur->position = j;

					occur_ptr[ pair ] = cur_occur;
				}

				match_results[ position ].first = best_result.first;		//match length
				match_results[ position ].second = best_result.second;	//match position

				position = position + best_result.first;			
			}
			else
			{
				cur_occur = new occur();
				cur_occur->previous = occur_ptr[pair];
				cur_occur->position = position;

				occur_ptr[pair] = cur_occur;

				position++;
			}
		}

	//	cout << "NEXT:" << endl;
	}
}


int main( int argc, char **argv )
{
clock_t start,finish;

	if (argc > 1)
	{
		start = clock();

		//read some testdata
		readfile( argv[1] );

		//mark each value pair as non-occurring yet
		for ( int i = 0; i < 256 * 256; i++)
			occur_ptr[i] = NULL;

		match_results = new match_result[length];
		rle_lengths = new short[length];

		for ( int j = 0; j < length; j++)
		{
			rle_lengths[j] = -1;
			match_results[j].first = -1;
			match_results[j].second = -1;
		}

		//find matches for the whole file
		find_matches();

		writefile( argv[1] );
/*
		for ( int k = 0; k < length; k++)
			cout << (int)match_results[k].second << " ";
		cout << endl;

		for ( int l = 0; l < length; l++)
			cout << (int)match_results[l].first << " ";
		cout << endl;

		for ( int m = 0; m < length; m++)
			cout << (int)rle_lengths[m] << " ";
		cout << endl;
*/
		delete data;
		delete [] match_results;
		delete [] rle_lengths;

		finish = clock();

		cout << "Time elapsed:" << (double)(finish - start) / CLOCKS_PER_SEC << endl;
	}
	else
		cout << "hmm" << endl;

	return 0;
}

