// Copyright (c) 2003 Eli-Jean Leyssens
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

#include "stdinc.h"
#include <iostream>
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::ios;
#include <unistd.h>
#include <set>
using std::set;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "FATDisk.h"

void strcpysafe( char * dest, int destsize, const char * src, int reserve = 0)
{
    strncpy( dest, src, destsize - reserve - 1);
    dest[ destsize - reserve - 1] = '\0';
}

#define PATHSIZE (MAX_PATH+1)

typedef int CLUSTER;
#define CLUSTER_NONE (-1)

enum {
    COMMAND_NONE,
    COMMAND_READ,
    COMMAND_WRITE,
    COMMAND_MKDIR
};

enum {
    OPTION_OVERWRITE_ALWAYS,
    OPTION_OVERWRITE_NEVER,
    OPTION_OVERWRITE_PROMPT
};

static string			g_program_name      = "";
static bool             g_help              = false;
static bool             g_dsk_exist         = false;
static bool             g_directory_exist   = false;
static int              g_overwrite         = OPTION_OVERWRITE_PROMPT;
static int              g_format            = FATDisk::FORMAT_NONE;
static int				g_bootblock			= FATDisk::BOOTBLOCK_MSX1;
//xxx implement bootblock option
static int				g_command			= COMMAND_NONE;
static string			g_dskdir_seperator	= "@";
static string			g_dsk_filename		= "";
static string			g_directory_in_dsk	= "";

bool dodsk_read( int argc, char ** argv)
{
    bool    ret = true;
    FATDisk fatdisk;

    ret = fatdisk.open( g_dsk_filename, g_dsk_exist ? FATDisk::OPEN_MUSTEXIST : FATDisk::OPEN_READONLY, g_format, g_bootblock);
    if (ret)
    {
    	//xxx
    	int entries;
    	if ( fatdisk.get_directory_entries( ROOT_DIRECTORY_CLUSTER, entries))
    	{
    		cout << "Root directory contains " << entries << " entries" << endl;
    		FATDisk::object_info_t	object_info;
    		for ( int index = 0 ; index < entries; ++index)
    		{
    			if ( fatdisk.get_directory_entry( ROOT_DIRECTORY_CLUSTER, index, object_info))
    			{
    				if ( object_info.name[0] != '\0')
    				{
    					cout << object_info.name << "." << object_info.extension << endl;
    					ofstream	output;
    					char fname[12];
    					sprintf( fname, "%s.%s", object_info.name, object_info.extension);
    					output.open( fname, ios::out|ios::binary);
    					if ( !output.fail())
    					{
    						fatdisk.read_object( object_info.first_cluster, object_info.size, output);
    						output.close();
    					}
    				}
    			}
    		}
    	}
    	fatdisk.close();
    }

    return ret;
}

bool write_files( CLUSTER directory_cluster, char * wildcardedname)
{
    bool    ret = true;

/*XXX
    char directory[PATHSIZE];
    char path[PATHSIZE];
    LPTSTR  lpFilePart;

    GetFullPathName( wildcardedname, PATHSIZE, directory, &lpFilePart);
    *lpFilePart = '\0';

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    hFind = FindFirstFile(wildcardedname, &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            sprintf( path, "%s%s", directory, FindFileData.cFileName);
            //xxx ret = write_file( path, FindFileData.cFileName, directory_cluster);
        } while ( FindNextFile( hFind, &FindFileData));
        FindClose(hFind);
    }
    else
    {
        switch (GetLastError())
        {
        case ERROR_FILE_NOT_FOUND:
            fprintf( stderr, "No file or directory found matching the (wildcarded) name %s\n", wildcardedname);
            break;
        case ERROR_PATH_NOT_FOUND:
            fprintf( stderr, "Path not found for the (wildcarded) name %s\n", wildcardedname);
            break;
        case ERROR_INVALID_NAME:
            fprintf( stderr, "Incorrect syntax for the (wildcarded) name %s\n", wildcardedname);
            break;
        default:
            fprintf( stderr, "Failed to find a file or directory for the (wildcarded) name %s. GetLastError reports %d\n", wildcardedname, GetLastError());
            break;
        }
        ret = false;
    }
*/
    return ret;
}

bool dodsk_write( int argc, char ** argv)
{
    bool    ret = true;
    CLUSTER directory_cluster = CLUSTER_NONE;
    
    FATDisk fatdisk;

    ret = fatdisk.open( g_dsk_filename, g_dsk_exist ? FATDisk::OPEN_MUSTEXIST : 0, g_format, g_bootblock);
    //xxx open .dsk file

    // translate g_directory_in_dsk to directory_cluster
    // if not found then
    //   if g_directory_exist then
    //     print error("dir not exist")
    //   else
    //     mkdir(dir)
    //   endif
    // endif

    cout << "g_dsk_filename = " << g_dsk_filename << endl;
    cout << "g_directory_in_dsk = " << g_directory_in_dsk << endl;

    if (ret)
    {
        for ( int name_index = 1 ; name_index < argc ; ++name_index)
        {
            char wildcardedname[PATHSIZE];
            strcpysafe( wildcardedname, sizeof(wildcardedname), argv[name_index], 3);
            if ( wildcardedname[ strlen( wildcardedname) - 1] == '\\')
            {
                strcat( wildcardedname, "*.*");
            }
            // xxx pass on the dsk object to write_files
            ret = write_files( directory_cluster, wildcardedname);
            if ( !ret)
            {
                break;
            }
        }
    }

    //xxx close the .dsk file

    return ret;
}

bool dodsk_mkdir( int argc, char ** argv)
{
    bool    ret = true;

    //xxx open .dsk file

    if (ret)
    {
    }

    return ret;
}

void print_syntax( ostream & stream = cerr)
{
    stream << "Writes a file to or reads a file from a .DSK file" << endl;
    stream << endl;
    stream << g_program_name << " [-hoked] [-f NNN] [-s <dskdir seperator>] r[ead]|w[rite]|m[kdir]" << endl;
    stream << "     [<dsk filename>[<dskdir seperator><directory>] <filename> [<filename>...]]" << endl;
    stream << endl;
    stream << "  -h           Print this information" << endl;
    stream << "  -o           Overwrite existing destination file(s)." << endl;
    stream << "  -k           Keep existing destination file(s)" << endl;
    stream << "  -d           The Directory in the .DSK image must already exist." << endl;
    stream << "  -e           The .DSK image file must already Exist." << endl;
    stream << "  -f NNN       If the .DSK file does not exist yet it will be created in" << endl;
    stream << "               the given Format." << endl;
    stream << "               360 = 360K, single sided, double density" << endl;
    stream << "               720 = 720K, double sided, double density" << endl;
    stream << "  -s seperator The character(s) that seperate the dsk filename and directory." << endl;
    stream << "               The default seperator is @" << endl;
    stream << endl;
}

bool process_options( int argc, char ** argv, int & arg_index)
{
	bool	ret = true;
	
	int		option;
	
	while ( (option = getopt( argc, argv, "hokdef:s:")) != -1)
	{
		switch (option)
		{
		case 'h':
			g_help = true;			
			break;
		case 'o':
			g_overwrite = OPTION_OVERWRITE_ALWAYS;
			break;
		case 'k':
			g_overwrite = OPTION_OVERWRITE_NEVER;
			break;
		case 'd':
			g_directory_exist = true;
			break;
		case 'e':			
			g_dsk_exist = true;
			break;
		case 'f':
            {
                struct format_string{
                    int format;
                    char * string;
                };
                static const format_string formats[] = {
                    {FATDisk::FORMAT_360, "360"},
                    {FATDisk::FORMAT_720, "720"}
                };
                for ( size_t format_index = 0 ; format_index < sizeof( formats)/sizeof(format_string); ++format_index)
                {
                    if ( strcmp( optarg, formats[ format_index].string) == 0)
                    {
                        g_format = formats[ format_index].format;
                        break;
                    }
                }
                if ( g_format == FATDisk::FORMAT_NONE)
                {
                    cerr << "Incorrect -f format value " << optarg << endl;
                    ret = false;
                }
            }			
			break;
		case 's':
			g_dskdir_seperator = optarg;
			break;
		default:
			ret = false;
			break;
		}
	}
	arg_index = optind;
	return ret;
}

bool process_arguments( int argc, char ** argv, int & arg_index)
{
	bool	ret = true;
	
    if ( arg_index < argc)
    {
        //
        // Determine the command
        //
        switch ( argv[arg_index][0])
        {
        case 'r':
            g_command = COMMAND_READ;
            break;
        case 'w':
            g_command = COMMAND_WRITE;
            break;
        case 'm':
            g_command = COMMAND_MKDIR;
            break;
        default:
            cerr << "Unrecognized command: " << argv[arg_index] << endl;
            ret = false;
            break;
        }
        arg_index++;
    }
    else
    {
    	if ( !g_help)
    	{
        	print_syntax();
	        ret = false;
        }
    }
    
    if ( ret)
    {
        //
        // Process remaining (common) arguments
        //
        if ( g_command != COMMAND_NONE)
        {
            if ( arg_index < argc)
            {
            	g_dsk_filename = argv[ arg_index];
                arg_index++;
                
                size_t	seperator_pos = g_dsk_filename.find( g_dskdir_seperator);
                if ( seperator_pos != string::npos)
                {
                	g_directory_in_dsk = g_dsk_filename.substr( seperator_pos + g_dskdir_seperator.size());
                	g_dsk_filename.resize( seperator_pos);
                }
            }
            else
            {
                cerr << "dsk filename missing" << endl;
                ret = false;
            }
        }
    }
    return ret;
}

int main( int argc, char ** argv)
{
    bool            ret                 = true;
    int             arg_index           = 1;

    string			program_name		= argv[0];

#ifdef MINGW
	// MinGW places the full executable path in argv[0], AAARGH!
	size_t	program_name_pos = program_name.rfind( "\\");
	if ( program_name_pos != string::npos)
	{
		 program_name= program_name.substr( program_name_pos + 1);
	}
#endif
	g_program_name = program_name;
	if ( 
		(ret = process_options( argc, argv, arg_index)) &&
		(ret = process_arguments( argc, argv, arg_index))
		)
	{
		if ( g_help)
		{
			print_syntax( cout);
		}
        init_fatdisk();
        //
        // Execute the given command
        //
        switch ( g_command)
        {
        case COMMAND_NONE:
        	// Nothing to do
            break;
        case COMMAND_READ:
            ret = dodsk_read( argc - arg_index, &argv[ arg_index]);
            break;
        case COMMAND_WRITE:
            ret = dodsk_write( argc - arg_index, &argv[ arg_index]);
            break;
        case COMMAND_MKDIR:
            ret = dodsk_mkdir( argc - arg_index, &argv[ arg_index]);
            break;
        }
        destroy_fatdisk();
	}
    
    return ret ? 0 : 1;
}
