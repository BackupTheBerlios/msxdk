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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Archive.h"
#include "INIFile.h"
#include "loadsave_file.h"
#include <OptionParser.h>

using std::vector;
using std::string;

string	g_program_name;

bool	g_help       	= false;
bool    g_asmfile_only	= false;
bool    g_force_repack	= false;

#ifdef WIN32
#define CORRECT_SEPERATOR '\\'
#define INCORRECT_SEPERATOR '/'
#else
#define CORRECT_SEPERATOR '/'
#define INCORRECT_SEPERATOR '\\'
#endif

enum {
    STORETYPE_NONE,
    STORETYPE_RAW,
    STORETYPE_PACKED,
};

enum {
    CHUNKTYPE_SKIP,
    CHUNKTYPE_SINGLE,
    CHUNKTYPE_MULTI,
    CHUNKTYPE_REST
};

void underscorenuppercase( char * name)
{
    while ( strstr( name, ".") != NULL)
    {
        *strstr( name, ".") = '_';
    }
    for ( int i = 0 ; i < (int)strlen( name); ++i)
    {
        if ( (name[i] >= 'a') && ( name[i] <= 'z'))
        {
            name[i] &= 0xdf;
        }
    }
}

bool scan_chunksize( const char * p, long & chunksize)
{
    chunksize = 0;
    const char * base = p;
    for ( ; *p && ( *p >= '0') && (*p <= '9') ; ++p)
    {
        chunksize = chunksize * 10 + (*p - '0');
    }
    if ( (*p == 'k') || ( *p == 'K'))
    {
        chunksize = chunksize * 1024;
        ++p;
    }
    if ( *p && (*p != '|'))
    {
        fprintf( stderr, "Incorrect chunksize ");
        for ( ; *base && ( *base != '|'); ++base)
        {
            fprintf( stderr, "%c", *base);
        }
        fprintf( stderr, "\n");
        return false;
    }
    return true;
}

string system_seperators( const string & path)
{
	string	corrected = path;
	
	for ( size_t i = 0 ; i < strlen( path.c_str()); ++i)
	{
		if ( corrected[i] == INCORRECT_SEPERATOR)
		{
			corrected[i] = CORRECT_SEPERATOR;
		}
	}
	return corrected;
}

string unix_seperators( const string & path)
{
	string	corrected = path;
	
	for ( size_t i = 0 ; i < strlen( path.c_str()); ++i)
	{
		if ( corrected[i] == '\\')
		{
			corrected[i] = '/';
		}
	}
	return corrected;
}

bool arcreate( const char * iniarg)
{
  	//XXX
  	INIFile	ifil;  	
  	ifil.load( iniarg);
  	
  	// Make all filenames relative to the directory of the INI file,
  	// by setting the directory of the INI file as the current directory.
  	if ( strstr( iniarg, "/") || strstr( iniarg, "\\"))
  	{
  		char	* directory = strdup( iniarg);
  		if ( directory)
  		{
  			char	* p;
  			for ( p = directory + strlen(directory) - 1; *p != '/' && *p != '\\'; --p);
  			*p = '\0';
  			chdir( directory);
  		}
  		else
  		{
  			fprintf( stderr, "Out of memory\n");
  			return false;
  		}
  	}
  	
  	// Create the .asm file
  	string	asmfile = unix_seperators( ifil.get_value( "CONFIGURATION", "ASMFILE"));
  	if ( strlen( asmfile.c_str()) == 0)
  	{
        fprintf( stderr, "ASMFILE key in the CONFIGURATION section not found or empty\n");
        return false;
  	} 
    FILE    * fhgen = fopen( asmfile.c_str(), "wb");
    if ( fhgen == NULL)
    {
        fprintf( stderr, "Failed to create ASMFILE %s\n", asmfile.c_str());
        return false;
    }

	// Get the name of the packer executable
    string	packer = system_seperators( ifil.get_value( "CONFIGURATION", "PACKER"));

	#ifdef MINGW
    mkdir( "packed");
    #else
    mkdir( "packed", 0777);
    #endif
    int     maxtocsize = 0;
  	const vector<INIFile::section_t> & sections = ifil.get_sections();
  	for ( vector<INIFile::section_t>::const_iterator section_it = sections.begin(); section_it != sections.end(); ++section_it)
  	{
  		const char    * sectionname = section_it->name.c_str();
        if ( strcmp( sectionname, "CONFIGURATION") != 0)
        {
            Archive     archive;
            int         files = 0;
            char        msxdosfilename[ 12];
            int         si = 0;

            for ( int mi = 0 ; mi < 12; mi++)
            {
                if ( (mi < 8) && ( (sectionname[si] == '.') || (sectionname[si] == '\0')))
                {
                    msxdosfilename[mi] = ' ';
                }
                else
                {
                    if ( (mi >= 8) && (sectionname[si] == '.'))
                    {
                        si++;
                    }
                    if ( sectionname[si] != '\0')
                    {
                        if ( (sectionname[si] >= 'a') && (sectionname[si] <= 'z'))
                        {
                            msxdosfilename[mi] = sectionname[si] & 0xdf;
                        }
                        else
                        {
                            msxdosfilename[mi] = sectionname[si];
                        }
                        si++;
                    }
                    else
                    {
                        msxdosfilename[mi] = ' ';
                    }
                }
            }
            msxdosfilename[11] = '\0';

            if ( g_asmfile_only || archive.create( sectionname))
            {
            	if ( section_it->entries.size() == 0)
                {
                    fprintf( stderr, "Can't create empty archives. Archive %s is empty\n", sectionname);
                    return false;
                }
                char archivename[ MAX_PATH];
                strcpy( archivename, sectionname);
                underscorenuppercase( archivename);
                fprintf( fhgen, "%s:\tdb\t\"%s\"\r\n\t\tEXPORT\t%s\r\n", archivename, msxdosfilename, archivename);
		  		for ( vector<INIFile::key_value_t>::const_iterator entries_it = section_it->entries.begin(); entries_it != section_it->entries.end(); ++entries_it)
		  		{            	
                    int storetype = STORETYPE_NONE;
                    const char * key 	= entries_it->key.c_str();
                    const char * value 	= entries_it->value.c_str();
                    char path[ MAX_PATH];
                    vector<int>     chunktypes;
                    vector<long>    chunksizes;
                    if ( strcmp( key, "RAW") == 0)
                    {
                        storetype = STORETYPE_RAW;
                    }
                    if ( strcmp( key, "PACKED") == 0)
                    {
                        storetype = STORETYPE_PACKED;
                    }
                    if ( storetype == STORETYPE_NONE)
                    {
                        fprintf( stderr, "Unrecognized storetype in %s = %s\n", key, value);
                        return false;
                    }                
                    strcpy( path, unix_seperators(value).c_str());
                    if ( strstr( path, "|") != NULL)
                    {
                        char    * p = strstr( path, "|");
                        
                        int     multis  = 0;
                        int     rests   = 0;
                        while ( *p)
                        {
                            p++;
                            long chunksize;
                            switch (*p)
                            {
                            case '\0':
                                break;
                            case 's':
                            case 'S':
                                if ( !scan_chunksize( p + 1, chunksize))
                                {
                                    return false;
                                }
                                chunktypes.push_back( CHUNKTYPE_SKIP);
                                chunksizes.push_back( chunksize);
                                break;
                            case 'c':
                            case 'C':
                                if ( !scan_chunksize( p + 1, chunksize))
                                {
                                    return false;
                                }
                                chunktypes.push_back( CHUNKTYPE_SINGLE);
                                chunksizes.push_back( chunksize);
                                break;
                            case 'm':
                            case 'M':
                                if ( !scan_chunksize( p + 1, chunksize))
                                {
                                    return false;
                                }
                                chunktypes.push_back( CHUNKTYPE_MULTI);
                                chunksizes.push_back( chunksize);
                                multis++;
                                break;
                            case 'r':
                            case 'R':
                                chunktypes.push_back( CHUNKTYPE_REST);
                                chunksizes.push_back( 0);
                                rests++;
                                break;
                            default:
                                fprintf( stderr, "Unrecognized option %c in %s = %s\n", *p, key, value);
                                return false;
                            }
                            for ( ; *p && (*p != '|'); p++);
                        }
                        *strstr( path, "|") = '\0';
                        if ( multis || rests)
                        {
                            if ( multis > 1)
                            {
                                fprintf( stderr, "More than one Multiple detected in %s = %s\n", key, value);
                                return false;
                            }
                            else
                            {
                                if ( rests > 1)
                                {
                                    fprintf( stderr, "More than one Rest detected in %s = %s\n", key, value);
                                    return false;
                                }
                                else
                                {
                                    if ( multis && rests)
                                    {
                                        fprintf( stderr, "Multiple and a Rest detected in %s = %s\n", key, value);
                                        return false;
                                    }
                                    else
                                    {
                                        // Either one multiple or one rest. Must be the last one
                                        // in the list of chunktypes.
                                        if ( 
                                            (chunktypes.back() != CHUNKTYPE_MULTI) &&
                                            (chunktypes.back() != CHUNKTYPE_REST)
                                            )
                                        {
                                            fprintf( stderr, "Multiple or Rest only allowed as the last chunk type. Error in %s = %s\n", key, value);
                                            return false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        chunktypes.push_back( CHUNKTYPE_REST);
                        chunksizes.push_back( 0);
                    }
                    char    filename[MAX_PATH];
                    if ( strstr( path, "/") != NULL)
                    {
                    	char * p;
                        for ( p = path + strlen( path); *p != '/'; --p);
                        strcpy( filename, p);
                    }
                    else
                    {
                        strcpy( filename, path);
                    }
                    underscorenuppercase( filename);
                    fprintf( fhgen, "%s:\tequ\t%d\r\n\t\tEXPORT\t%s\r\n", filename, files, filename);
                    if ( !g_asmfile_only)
                    {
                        archive.addfile( storetype == STORETYPE_PACKED ? 1 : 0);
                        vector<char>    filedata;
                        if ( !load_file( path, filedata))
                        {
                            return false;
                        }
                        struct stat	pathattr;
                        stat( path, &pathattr);

                        int chunks = 0;
                        long offset = 0;
                        size_t chunktype_index = 0;
                        const long filesize = (long)filedata.size();
                        while ( offset < (long)filedata.size())
                        {
                            long    curchunksize;
                            if ( chunktypes[ chunktype_index] != CHUNKTYPE_REST)
                            {
                                curchunksize = chunksizes[ chunktype_index];
                                if ( chunktypes[ chunktype_index] == CHUNKTYPE_MULTI)
                                {
                                    if ( curchunksize > ( filesize - offset))
                                    {
                                        curchunksize = filesize - offset;
                                    }
                                }
                                else
                                {
                                    if ( curchunksize > ( filesize - offset))
                                    {
                                        fprintf( stderr, "Can't %s %ld bytes: only %ld bytes left in file\n", chunktypes[ chunktype_index] == CHUNKTYPE_SKIP ? "skip" : "read", curchunksize, filesize - offset);
                                        return false;
                                    }
                                }
                            }
                            else
                            {
                                curchunksize = filesize - offset;
                            }
                            if ( chunktypes[ chunktype_index] != CHUNKTYPE_SKIP)
                            {
                                char filetoadd[MAX_PATH+1024] = "$$ARC$$.TMP";
                                if ( !save_file( filetoadd, &filedata[offset], curchunksize))
                                {
                                    return false;
                                }
                                if ( storetype == STORETYPE_PACKED)
                                {
                                    char packedfile[MAX_PATH+1024];
                                    sprintf( packedfile, "packed/%s.PCK%d", filename, chunks);
                                    if ( strlen( packer.c_str()) == 0)
                                    {
                                        fprintf( stderr, "PACKED storetype requested, but the PACKER key in the CONFIGURATION section was not found or was empty\n");
                                        return false;
                                    }
                                    // Do NOT pack if the chunk already exists and its timestamps
                                    // matches that of the original file. Since some packers are
                                    // extremely slow we only want to really pack a file when
                                    // it's either a new file or has been changed since the last
                                    // time it was packed.                                    
                                    struct stat packedattr;
                                    packedattr.st_mtime = 0;
                                    if ( 
                                        g_force_repack ||
                                        stat( packedfile, &packedattr) ||
                                        ( pathattr.st_mtime > packedattr.st_mtime)
                                        )
                                    {
                                        char cmdline[ 3*MAX_PATH];
                                        sprintf( cmdline, "%s -v %s %s", packer.c_str(), filetoadd, packedfile);
                                        if ( system( cmdline) != 0)
                                        {
                                            fprintf( stderr, "The specified packer returned a non-zero value (interpreted by %s as an error)\n", g_program_name.c_str());
                                            return false;
                                        }

                                    }
                                    strcpy( filetoadd, packedfile);
                                }
                                if ( !archive.addchunk( filetoadd))
                                {
                                    return false;
                                }
                                chunks++;
                            }
                            if ( 
                                ( chunktypes[ chunktype_index] != CHUNKTYPE_MULTI) &&
                                ( chunktypes[ chunktype_index] != CHUNKTYPE_REST)
                                )
                            {
                                chunktype_index++;
                            }
                            offset += curchunksize;
                        }
                    }
                    files++;
             	}
                if ( !g_asmfile_only)
                {
                    if ( !archive.dump())
                    {
                        return false;
                    }
                }
                if ( (6 * files) > maxtocsize)
                {
                    maxtocsize = 6 * files;
                }
            }
            else
            {
                return false;
            }
        }
    }
    fprintf( fhgen, "\r\nTOC_AREA:\tds\t%d\r\n\t\tEXPORT\tTOC_AREA\r\n", maxtocsize);
    fclose( fhgen);
    remove( "$$ARC$$.TMP");
    fhgen = NULL;
    
    return true;
}	
void print_try( void)
{
	fprintf( stderr, "Try '%s -h' for more information.\n", g_program_name.c_str());
}

void print_syntax( FILE * fhsyntax)
{
    fprintf( fhsyntax, "Syntax: %s [-aph] <archives-INI-file>\n", g_program_name.c_str());
    fprintf( fhsyntax, "\n");
    fprintf( fhsyntax, "  -h   Print this information.\n");
    fprintf( fhsyntax, "  -a   Only generate the ASMFILE. No archives are created.\n");
    fprintf( fhsyntax, "  -p   Forces a rePack of every packed file. Use this if the auto detection of what packed files need repacking fails.\n");
}

bool process_options( int argc, char ** argv, int & arg_index)
{
	bool	ret = true;
	
	OptionParser	parser( argc, argv);
	int		option;
	
	while ( (option = parser.GetOption( "aph")) != -1)
	{
		switch (option)
		{
		case 'h':
			g_help = true;
			break;
		case 'a':
			g_asmfile_only = true;
			break;
		case 'p':
			g_force_repack = true;
			break;
		default:
			fprintf( stderr, "Unrecognized option: -%c\n", (char)parser.Option());
			ret = false;
			break;
		}
	}
	arg_index = parser.Index();
	if ( !ret)
	{
		print_try();
	}
	return ret;
}

bool process_arguments( int argc, char ** argv, int & arg_index)
{
	bool	ret = true;
	
	if ( arg_index < argc)
	{
		if ( (arg_index + 1) < argc)
		{
			fprintf( stderr, "Only one INI-filename allowed.\n");
			ret = false;
		}
	}
	else
	{
		if ( !g_help)
		{
			fprintf( stderr, "Missing INI-filename.\n");
			ret = false;
		}
	}
	
	if ( ret)
	{
		if ( g_help)
		{
			print_syntax( stdout);
		}
		if ( arg_index < argc)
		{
			ret = arcreate( argv[ arg_index]);
		}
	}
	else
	{
		print_try();
	}
	return ret;
}

int main(int argc, char **argv)
{
	bool	ret = true;
	int		arg_index           = 1;

	string	program_name		= argv[0];

#ifdef MINGW
	// MinGW places the FULL executable path in argv[0], AAARGH!
	size_t	program_name_pos = program_name.rfind( "\\");
	if ( program_name_pos != string::npos)
	{
		 program_name= program_name.substr( program_name_pos + 1);
	}
#endif
	g_program_name = program_name;
	ret = process_options( argc, argv, arg_index);
	if ( ret)
	{
		ret = process_arguments( argc, argv, arg_index);
	}
	
    return ret ? 0 : 1;
}