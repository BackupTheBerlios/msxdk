// Copyright (c) 2003-2004 Eli-Jean Leyssens
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

// XXX Thanks to GuyveR800 for ideas for the command syntax
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
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <set>
using std::set;
#include <vector>
using std::vector;
#include <string>
using std::string;


#include "FATDisk.h"
#include "OptionParser.h"

#define PATHSIZE (MAX_PATH+1)

typedef int CLUSTER;
#define CLUSTER_NONE (-1)

enum {
	COMMAND_NONE,
	COMMAND_READ,
	COMMAND_WRITE,
	COMMAND_MKDIR,
	COMMAND_DELETE
};

enum {
	OPTION_OVERWRITE_ALWAYS,
	OPTION_OVERWRITE_NEVER,
	OPTION_OVERWRITE_PROMPT
};

static string			g_program_name      = "";
static bool             g_help              = false;
static bool             g_dsk_exist         = false;
static int              g_overwrite         = OPTION_OVERWRITE_PROMPT;
static int              g_format            = FATDisk::FORMAT_NONE;
static int				g_bootblock			= FATDisk::BOOTBLOCK_MSX1;
//xxx implement bootblock option
static int				g_command			= COMMAND_NONE;
static string			g_dsk_filename		= "";
static string			g_image_directory	= "";
static CLUSTER			g_image_directory_cluster;
static string			g_host_directory	= "";
static FATDisk			g_fatdisk;

bool check_host_directory( void)
{
	bool	ret = true;
	
	if ( strlen( g_host_directory.c_str()) > 0)
	{
		struct stat	file_stat;
		string	directory = g_host_directory.substr( 0, g_host_directory.size() - 1);
		if ( stat( directory.c_str(), &file_stat) == 0)
		{
			// At least the "object" exists, now see if it's a directory.
			if ( !S_ISDIR( file_stat.st_mode))
			{
				cerr << directory << " found, but it's not a directory" << endl;
				ret = false;
			}
		}
		else
		{
			report_stat_error( directory);
			ret = false;
		}
	}
	return ret;
}

size_t non_right_space( const char * text)
{
	size_t	pos = strlen( text);
	while ( pos && text[pos-1] == ' ')
	{
		pos--;
	}
	return pos;
}

/** Checks whether the fixed string matches the (possibly wildcarded) wild string.
	@param wild	Wild(carded) string
	@param fixed Fixed string
	@return <code>true</code> if it matches, <code>false</code> otherwise
*/
bool match( string wild, string fixed)
{
	for ( size_t pos = 0 ; pos < wild.size(); ++pos)
	{
		wild[pos] = tolower( wild[pos]);
	}
	for ( size_t pos = 0 ; pos < fixed.size(); ++pos)
	{
		fixed[pos] = tolower( fixed[pos]);
	}
	
	const char * pat = wild.c_str();
	const char * str = fixed.c_str();
	
	/* The following piece of wildcard-match-code was taken from
	   some website. The note that came with it read:

			I got it from a Walnut Creek CD (C/C++ user group library).
			The original code is from "C/C++ Users Journal".
			The author is Mike Cornelison.
			No use restriction is mentioned in the source file or other documentation
			I found in the CD.
			I modified it to prevent matching between '?' and '.' and to use
			the mapCaseTable[] array (the original routine didn't perform case
			insensitive matching) .
			
		Since I've removed both said modifications I assume the code is back in
		its original form, i.e. as Mike Cornelison wrote it and hence the "no use
		restriction" should apply again.		
	*/
	int i, star;

new_segment:

	star = 0;
	if (*pat == '*')
	{
		star = 1;
		do { 
			pat++; 
		} while (*pat == '*');
	}

test_match:

	for (i = 0; pat[i] && (pat[i] != '*'); i++) 
	{
		if (str[i] != pat[i])
		{
			if (!str[i]) return false;
			if (pat[i] == '?') continue;
			if (!star) return false;
			str++;
			goto test_match;
		}
	}
	if (pat[i] == '*') {
		str += i;
		pat += i;
		goto new_segment;
	}
	if (!str[i]) return true;
	if (i && pat[i - 1] == '*') return true;
	if (!star) return false;
	str++;
	goto test_match;
}

/** Checks whether the given name + extension matches the (possibly wildcarded) filename.
	@param filename  Wildcarded filename to match against.
	@param name 	 Name part to match.
	@param extension Extension part to match
	@return <code>true</code> if it matches, <code>false</code> otherwise
*/
bool matches( const char * filename, const char * name, const char * extension)
{
	bool	ret = true;
	string	short_name = string( name).substr( 0, non_right_space( name));
	string 	short_extension = string( extension).substr( 0, non_right_space( extension));
	
	string	wild = filename;	
	size_t dotpos = wild.find( '.');
	if ( dotpos == string::npos)
	{
		ret = match( wild, short_name + short_extension);
	}
	else
	{
		ret = match( wild.substr( 0, dotpos), short_name) &&
			match( wild.substr( dotpos + 1), short_extension);
	}
	return ret;
}

/** Checks whether a given filename contains any wildcards.
	@param filename Filename to be checked.
	@return <code>true</code> if it contains one or more wildcards, <code>false</code> otherwise
*/
bool is_wildcarded( const char * filename)
{
	bool	ret = false;
	
	if ( 
		(strstr( filename, "*") != NULL) ||
		(strstr( filename, "?") != NULL)
		)
	{
		ret = true;
	}
	return ret;
}


/** Splits a path up into a directory- and a leafname-part.
	@param path			Full path
	@param directory	Will be filled in with the directory part of the path (trailing / included, if originally present)
	@param leaf			Will be filled in with the leafname part of the path
*/
void split_path( const string & path, string & directory, string & leafname)
{
	size_t	seperator_pos = path.rfind( '/');
	if ( seperator_pos == string::npos)
	{
		directory = "";
		leafname = path;
	}
	else
	{
		directory = path.substr( 0, seperator_pos + 1);
		leafname = path.substr( seperator_pos + 1);
	}
}

string hostify( const char * name, const char * extension)
{
	string	hostifiedname = string(name).substr( 0, non_right_space( name));
	string 	hostifiedextension = string( extension).substr( 0, non_right_space( extension));
	if ( hostifiedextension.size() > 0)
	{
		hostifiedname += "." + hostifiedextension;
	}
	return hostifiedname;
}

/** Turns the given filename into a host-compatible/-friendly filename.
	@param filename		Fillename to be hostified.
	@return Hostified filename.
*/
string hostify( const string & filename)
{
	size_t	dotpos = filename.rfind( ".");
	if ( dotpos != string::npos)
	{
		return hostify( filename.substr( 0, dotpos - 1).c_str(), filename.substr( dotpos + 1).c_str());
	}
	else
	{
		return hostify( filename.c_str(), "");
	}
}

/** Turns all backslashes into forwardslashes and append a trailing /
	if the path is not empty.
	@param path	Path to be transformed.
*/
void forward_slash_and_terminate( string & path)
{
	if ( path.size() > 0)
	{
		size_t	pos;
		while ( (pos = path.find( '\\')) != string::npos)
		{
			path[ pos] = '/';
		}
		if ( path[ path.size() - 1] != '/')
		{
			path.append( "/");
		}
	}
}

bool directory_to_cluster( const CLUSTER & start_cluster, const string & directory, CLUSTER & cluster)
{
	bool	ret = true;
	
	cluster = start_cluster;
	string	remaining = directory;
	while ( ret && remaining[0] != '\0')
	{
		size_t	seperator_pos = remaining.find( '/');
		if ( seperator_pos == string::npos)
		{
			seperator_pos = remaining.find( '\\');
		}
		string	directory_name;
		if ( seperator_pos == string::npos)
		{
			directory_name = remaining;
			remaining = "";
		}
		else
		{
			directory_name = remaining.substr( 0, seperator_pos);
			remaining = remaining.substr( seperator_pos + 1);
		}
		if ( directory_name.size() > 0)
		{
			int	entries;
			ret = g_fatdisk.get_directory_entries( cluster, entries);
			if ( ret)
			{
				FATDisk::object_info_t	object_info;
				bool	found = false;
				for ( int index = 0 ; ret && !found && index < entries ; ++index)
				{
					ret = g_fatdisk.get_directory_entry( cluster, index, object_info);
					if ( ret)
					{
						if ( 
							( strcmp( object_info.name, ".          ") != 0) &&
							( strcmp( object_info.name, "..         ") != 0) &&
							( object_info.name[0] != '\0') &&
							( object_info.name[0] != DELETED_ENTRY_CHAR) &&
							( matches( directory_name.c_str(), object_info.name, object_info.extension))
							)
						{
							cluster = object_info.first_cluster;
							found = true;
						}
					}
				}
				if ( ret && !found)
				{
					cerr << directory << " not found." << endl;
					ret = false;
				}
			}
		}
	}
	return ret;
}

bool recursive_read( const string host_directory, const string image_directory, const char * name, const CLUSTER cluster)
{
	bool	ret = true;
	
//xxx
cout << "recursive_read: " << host_directory << "," << image_directory << "," << cluster << "," << name << endl;
	bool	found = false;
	int	entries;
	ret = g_fatdisk.get_directory_entries( cluster, entries);
cout << "entries: " << entries << endl;
	if ( ret)
	{
		FATDisk::object_info_t	object_info;
		for ( int index = 0 ; ret && index < entries ; ++index)
		{
			ret = g_fatdisk.get_directory_entry( cluster, index, object_info);
			if ( ret)
			{
				if ( 
					( (strcmp( object_info.name, ".       ") != 0) || (strcmp( object_info.extension, "   ") != 0)) &&
					( (strcmp( object_info.name, "..      ") != 0) || (strcmp( object_info.extension, "   ") != 0)) &&
					( object_info.name[0] != '\0') &&
					( object_info.name[0] != DELETED_ENTRY_CHAR)
					)
				{
					if ( matches( name, object_info.name, object_info.extension))
					{
						found = true;
						string	hostfile = host_directory + hostify( object_info.name, object_info.extension);
						if ( object_info.attributes.directory)
						{							
cout << "directory found: " << hostfile << endl;
					        struct stat	dir_stat;
        					if ( stat( hostfile.c_str(), &dir_stat) == 0)
        					{
        						if ( !S_ISDIR( dir_stat.st_mode))
        						{
        							cerr << "Can not create directory " << hostfile << ", since it already exists as a file/symlink/..." << endl;
        							ret = false;
        						}
        					}
        					else
        					{
								if ( 
#ifdef MINGW
									mkdir( hostfile.c_str())
#else
									mkdir( hostfile.c_str(), 0777)
#endif
									 != 0)
								{
									cerr << "Failed to mkdir " << hostfile << endl;
									ret = false;
								}
							}
							if ( ret)
							{
								ret = recursive_read( hostfile + "/", image_directory + hostify( object_info.name, object_info.extension) + "/", "*", object_info.first_cluster);
							}
						}
						else
						{
cout << "file found: " << hostfile << endl;
							ofstream	outfile;
			                outfile.open( hostfile.c_str(), ios::out|ios::binary);
			                if ( outfile.is_open())
			                {
								ret = g_fatdisk.read_object( object_info.first_cluster, object_info.size, 
										outfile);
								outfile.close();
							}
							else
							{
								cerr << "Failed to create/open file " << hostfile << endl;
								ret = false;
							}
						}
					}
				}
			}
		}
		if ( ret)
		{
			if ( !found && !is_wildcarded( name))
			{
				cerr << image_directory << name << " not found." << endl;
				ret = false;
			}
		}
	}
	
	/*
	found = false
	for each object in image_cluster do
		if matches(object.name, filename) then
			found = true
			if object.type == directory then
				mkdir host_directory+hostify(object.name)
				recursive_read( host_directory + hostify(object.name), image_directory+object.name,
								object.first_cluster, "*")
			else
				copy file from image to host_directory+hostify(object.name)
			endif
		endif
	next
	if wildcarded(filename) && !found then
		error "file not found"
	endif
	*/
	//xxx
	return ret;
}

bool preprocess_filename( const char * filename, string & host_directory, string & image_directory,
							 string & leafname, CLUSTER & image_directory_cluster)
{
	bool	ret = true;

	string	directory;
	split_path( filename, directory, leafname);
	if ( g_host_directory.size() > 0)
	{
		host_directory = g_host_directory + hostify( directory);
		image_directory = g_image_directory + directory;
		ret = directory_to_cluster( g_image_directory_cluster, directory, image_directory_cluster);
	}
	else
	{
		host_directory = directory;
		image_directory = g_image_directory;
		image_directory_cluster = g_image_directory_cluster;
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

bool recursive_write( const string host_directory, const string image_directory, const char * name, const CLUSTER cluster)
{
	bool	ret = true;
	/*	
	DIR * opened_dir = opendir( "C:\\MA*");
	if ( opened_dir != NULL)
	{
		struct dirent * entry;
		
		while ( (entry = readdir( opened_dir)) != NULL)
		{
			cout << entry->d_name << endl;
		}
		closedir( opened_dir);
		opened_dir = NULL;
	}
	else
	{
		report_stat_error( g_host_directory);		
	}
	*/
	return ret;
}

bool recursive_mkdir( const string host_directory, const string image_directory, const char * name, const CLUSTER cluster)
{
	bool    ret = true;
	//xxx
	return ret;
}

bool recursive_delete( const string host_directory, const string image_directory, const char * name, const CLUSTER cluster)
{
	bool    ret = true;
	//xxx
	return ret;
}

bool dodsk_misc( 
	int argc, char ** argv, 
	bool (*process)( const string host_directory, const string image_directory, const char * name, const CLUSTER cluster),
	const bool read_only)
{
	bool    ret = true;

	ret = g_fatdisk.open( g_dsk_filename, (g_dsk_exist ? FATDisk::OPEN_MUSTEXIST : 0) | (read_only ? FATDisk::OPEN_READONLY : 0), g_format, g_bootblock);
	if (ret)
	{
		//xxx		
		ret = check_host_directory() && directory_to_cluster( ROOT_DIRECTORY_CLUSTER, g_image_directory, g_image_directory_cluster);
		if ( ret)
		{
			for ( int arg_index = argc ? argc : -1 ; arg_index < argc ; ++arg_index)
			{
				string	host_directory, image_directory, leafname;
				CLUSTER	image_directory_cluster;
				if ( 
					!preprocess_filename( argc ? argv[ arg_index] : "*", host_directory, image_directory, 
						leafname, image_directory_cluster) ||
					!process( host_directory, image_directory, leafname.c_str(), image_directory_cluster)
					)
				{
					ret = false;
				}
			}
		}
		g_fatdisk.close();
	}

	return ret;
}

void print_syntax( ostream & stream = cerr)
{
	stream << "Writes a file to or reads a file from a .DSK file" << endl;
	stream << endl;
	stream << g_program_name << " r[ead]|w[rite]|m[kdir]|d[elete]" << endl;
	stream << "     [-hokec] [-f NNN] [-i <directory>] [-d <directory>]" << endl;
	stream << "     [<dsk filename> <filename> [<filename>...]]" << endl;
	stream << endl;
	stream << "  -h             Print this information" << endl;
	stream << "  -o             Overwrite existing destination file(s)." << endl;
	stream << "  -k             Keep existing destination file(s)" << endl;
	stream << "  -e             The .DSK image file must already Exist." << endl;
	stream << "  -i <directory> The directory inside the Image to be used as the start directory." << endl;
	stream << "  -d <directory> The local(host) Directory to be used as the start directory." << endl;
	stream << "  -c             Same as ""-d ."", i.e. current directory." << endl;
	stream << "  -f NNN         If the .DSK file does not exist yet it will be created in" << endl;
	stream << "                 the given Format." << endl;
	stream << "                 360 = 360K, single sided, double density" << endl;
	stream << "                 720 = 720K, double sided, double density" << endl;
	stream << endl;
	stream << "Note: options may be given anywhere in the command line, even before the" << endl;
	stream << "action or in between filenames." << endl;
	stream << endl;
}

bool process_options( int argc, char ** argv, int & arg_index)
{
	bool	ret = true;
	
	OptionParser	parser( argc, argv);
	int		option;
	
	while ( (option = parser.GetOption( "hokef:i:d:")) != -1)
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
					if ( strcmp( parser.Argument(), formats[ format_index].string) == 0)
					{
						g_format = formats[ format_index].format;
						break;
					}
				}
				if ( g_format == FATDisk::FORMAT_NONE)
				{
					cerr << "Incorrect -f format value " << parser.Argument() << endl;
					ret = false;
				}
			}			
			break;
		case 'i':
			{
				g_image_directory = parser.Argument();
				forward_slash_and_terminate( g_image_directory);
			}
			break;
		case 'c':
			g_host_directory = "./";
			break;
		case 'd':
			{
				g_host_directory = parser.Argument();
				forward_slash_and_terminate( g_host_directory);
				if ( g_host_directory.size() == 0)
				{
					g_host_directory = "./";
				}
			}
			break;
		default:
			ret = false;
			break;
		}
	}
	arg_index = parser.Index();
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
		case 'd':
			g_command = COMMAND_DELETE;
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
	// MinGW places the FULL executable path in argv[0], AAARGH!
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
			ret = dodsk_misc( argc - arg_index, &argv[ arg_index], recursive_read, true);
			break;
		case COMMAND_WRITE:
			ret = dodsk_misc( argc - arg_index, &argv[ arg_index], recursive_write, false);
			break;
		case COMMAND_MKDIR:
			ret = dodsk_misc( argc - arg_index, &argv[ arg_index], recursive_mkdir, false);
			break;
		case COMMAND_DELETE:
			ret = dodsk_misc( argc - arg_index, &argv[ arg_index], recursive_delete, false);
			break;
		}
		destroy_fatdisk();
	}
	
	return ret ? 0 : 1;
}
