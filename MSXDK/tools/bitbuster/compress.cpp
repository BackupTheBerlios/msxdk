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



#include "compress.h"
#include "encode.h"


unsigned char *data;		// data to crunch
int length;			// length of data to crunch


int max_depth;

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
    
	occur_index = 0;	// reset position of first free occur struct
	
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
void compress( const string & file, const string & output_file, int p_max_depth, int block_length  )
{
int compressed_length = 0;
int file_length;
ofstream outfile;
ifstream infile;
int block_count;
int position;
int remaining_length;

	max_depth = p_max_depth;
	 	
	infile.open( file.c_str() , ios::binary | ios::in );
	
	if ( infile.fail() )
	{
		cerr << "Failed to read " << file << endl;	
		return;
	}	

	remaining_length = file_length = get_file_length( infile );
         
        if ( block_length == -1 )
		block_length = file_length;
		 
	if ( file_length == 0 )
	{
		cerr << "Nothing to compress!" << endl;
		return;
	}	
	  
       	outfile.open( output_file.c_str(), ios::binary | ios::out );
	       
	if ( outfile.fail() )
	{
		cerr << "Failed to open " << output_file << " for output" << endl;
		return;
	}
        
	outfile.write( (char*)&file_length, 4 );
		
	block_count = ( file_length - 1 ) / block_length + 1;
	             	               
	outfile.write( (char*)&block_count, 4 );
						             	   
 	cout << "Compressing: " << file << "... ";
 	cout.flush();
 	
 	while ( block_count-- )
 	{ 		 
	 	position = outfile.tellp();
		
		outfile.write( (char*)&position, 4 );
				 		
 		if ( remaining_length < block_length )
 			length = remaining_length;
 		else
 			length = block_length;
 			
		// mark each value pair as non-occurring yet
		for ( int i = 0; i < 256 * 256; i++)
			occur_ptr[ i ] = NULL;
	
		data = new unsigned char[ length ];
		
		infile.read( (char*)data, length );
	
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
		write_file( &outfile, data, length, match_results );
	          		                            		           
	        // remove data
		delete [] occur_pool; 	
		delete [] match_results;
		delete [] data;
		
		compressed_length = outfile.tellp();
		
		outfile.seekp( position, ios::beg );
		
		position = compressed_length - position - 4;
		
		outfile.write( (char*)&position, 4 );
		
		outfile.seekp( 0, ios::end );
		
		remaining_length -= block_length;
	}
	
	infile.close(); 
	outfile.close();
	
	// output statistics if writing file succeeded
	if ( compressed_length > 0 )
		cout << file_length << " -> " << compressed_length << endl;	 
}


