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
#include "compress.h"


static string g_program_name = "";	// variable to hold the program name
static string g_extension = "pck";      // default compressed file extension
static bool g_syntax = false;           // (don't) show the program syntax
static bool g_output_names = false;	// forces every second filename to be taken as the output filename
static bool g_append_extension = false;	// (don't) append extension to output filename
static int g_block_length = -1;		// block length (-1 is use file length)
 
 
 
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
	stream << g_program_name << " [-hoa] [-e <extension>] [-b <size>]";
	stream << " <filename> [<filename>...]" << endl;
        stream << endl;
	stream << "  -h             Print this information" << endl;
	stream << "  -a             Append extension instead of replacing it" << endl;
	stream << "  -o             Forces every second filename to be treated as the Output filename" << endl;
	stream << "                 for the filename preceding it" << endl;
	stream << "  -e <extension> Set the extension used for compressed file(s)" << endl;
	stream << "                 Default extension is pck" << endl;
	stream << "  -b <size>      Set size of data to be compressed per chunk" << endl;
	stream << "                 Default block length is original file size" << endl;      
        stream << endl;
}


// process all options
bool process_options( int argc, char ** argv, int & arg_index )
{
bool ret = true;
OptionParser parser( argc, argv );
int option;
	
	// get next option
	while ( ( option = parser.GetOption( "ahoe:b:" ) ) != -1 )
	{
		switch ( option )
		{
			case 'a':	// append extension
				g_append_extension = true;
				break;
				
			case 'h':	// print program syntax
			        g_syntax = true;
				break;
			
			case 'o':	// every second filename is an output name
					g_output_names = true;
				break;
	
			case 'e':	// set compressed file extension
			        g_extension = parser.Argument();
				break;
			
			case 'b':	// set block size
				{
					// get block size
					string block_size = parser.Argument();
				
					// convert to integer
					g_block_length = atoi( block_size.c_str() );
					
					// print error if < 1
					if ( g_block_length < 128 )
					{
						cerr << "Block length must be > 127" << endl;
						ret = false;
					}	
				}
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
		if ( !g_output_names || (( argc + 1 - arg_index) & 1))
		{
			// compress all files
			while ( arg_index < argc )
			{
				// get name for outputfile
				string output_name = argv[ arg_index + ( g_output_names ? 1 : 0)];
				
				// if outputfiles are not specified, extension has to be added
				if ( !g_output_names)
				{
					// find rightmost . in name
					int ext_index = output_name.rfind( "." );	
					
					// if no extension found
					if ( ext_index < 0 || g_append_extension )
						output_name = output_name + "." + g_extension;	// add extension
					else
						output_name = output_name.substr( 0, ext_index + 1 ) + g_extension;	// else replace extension
				}
								
				compress( argv[ arg_index], output_name, g_block_length );
				
				arg_index += ( g_output_names ? 2 : 1);
			}
		}
		else
		{
			cerr << "Option -o was specified, but an uneven number of filenames was given" << endl;
			ret = false;
		}
	}
	else
	{
		if ( !g_syntax )
		{
			cerr << "No filenames were given" << endl;
			ret = false;
		}
	}	
	if ( !ret)
	{
		// print some info
		print_try();
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
		{
			cout << "Time elapsed: ";
			cout << (double)( finish - start ) / CLOCKS_PER_SEC;
			cout << " seconds" << endl;
		}
	}
	
	return ret ? 0 : 1;
}

