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
#include "INIFile.h"
#include "loadsave_file.h"

using std::vector;
using std::string;

static const string 	empty_string = "";

static const string & string_upper( string & value)
{
	size_t	length = value.length();
	for ( size_t i = 0; i < length; ++i)
	{
		value[i] = toupper( value[i]);
	}
	return value;
}

bool INIFile::load( const char * filename)
{
	bool ret = true;
	vector<char> data;
	
	ret = load_file( filename, data);
	if ( ret)
	{
		data.push_back( '\n');
		for ( vector<char>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if ( (*it == '\r') || ( *it == '\n'))
			{
				*it = '\0';
			}
		}
		for ( vector<char>::const_iterator it = data.begin(); ret && it != data.end(); ++it)
		{
			const char * line = &it[0];
			for ( ; (*it == ' ') || (*it == '\t'); ++it);
			switch ( *it)
			{
			case '\0':
				// Empty line
				break;
			case ';':
				// Comment line
				for ( ++it; *it ; ++it);
				break;
			case '[':
				{
					// Section header
					for ( ++it; (*it == ' ') || (*it == '\t') ; ++it);
					vector<char>::const_iterator	name_begin = it;
					for ( ; *it && (*it != ' ') && (*it != '\t') && (*it != ']'); ++it);
					vector<char>::const_iterator	name_end = it;				
					for ( ; (*it == ' ') || (*it == '\t') ; ++it);
					if ( *it == ']')
					{
						m_sections.resize( m_sections.size() + 1);
						m_sections.back().name.insert( m_sections.back().name.begin(), name_begin, name_end);
						string_upper( m_sections.back().name);
					}
					else
					{
						fprintf( stderr, "Invalid section header: %s\n", line);
						ret = false;
					}
				}
				break;
			default:
				// Key/value pair
				if ( m_sections.size() > 0)
				{
					// First find the key
					for ( ; (*it == ' ') || (*it == '\t') ; ++it);
					vector<char>::const_iterator	key_begin = it;
					for ( ; *it && (*it != ' ') && (*it != '\t') && (*it != '='); ++it);
					vector<char>::const_iterator	key_end = it;				
					for ( ; (*it == ' ') || (*it == '\t') ; ++it);
					if ( *it == '=')				
					{
						// Now find the value
						for ( ++it; (*it == ' ') || (*it == '\t') ; ++it);
						vector<char>::const_iterator	value_begin = it;
						vector<char>::const_iterator	value_end;
						vector<char>::const_iterator	line_end;
						if ( *it)
						{
							for ( ++it; *it ; ++it);
							line_end = it;
							for ( --it; (*it == ' ') || (*it == '\t') ; --it);
							value_end = it;
						}
						else
						{
							// Empty value
							value_end = value_begin;
							line_end = it;
						}
						it = line_end;					
						m_sections.back().entries.resize( m_sections.back().entries.size() + 1);
						m_sections.back().entries.back().key.insert( m_sections.back().entries.back().key.begin(), key_begin, key_end);
						string_upper( m_sections.back().entries.back().key);
						m_sections.back().entries.back().value.insert( m_sections.back().entries.back().value.begin(), value_begin, ++value_end);
						// Unquote value
						string & value = m_sections.back().entries.back().value;
						if ( value[0] == '"')
						{
							if ( value[ value.length() - 1] == '"')
							{
								value.assign( value, 1, value.length() - 2);								
							}
							else
							{
								fprintf( stderr, "Closing quote failing in key=value line: %s\n", line);
								ret = false;
							}
						}
					}
					else
					{
						fprintf( stderr, "Invalid key=value line: %s\n", line);
						ret = false;
					}
				}
				else
				{
					fprintf( stderr, "Key=value line outside a section: %s\n", line);
					ret = false;
				}			
				break;
			}
		}		
	}		
	return ret;
}

const vector<INIFile::section_t> & INIFile::get_sections( void)
{
	return m_sections; 
}

const string & INIFile::get_value( const char * section, const char * key)
{
  	for ( vector<INIFile::section_t>::const_iterator section_it = m_sections.begin(); section_it != m_sections.end(); ++section_it)
  	{
  		if ( strcasecmp( section_it->name.c_str(), section) == 0)
  		{
	  		for ( vector<INIFile::key_value_t>::const_iterator entries_it = section_it->entries.begin(); entries_it != section_it->entries.end(); ++entries_it)
	  		{
	  			if ( strcasecmp( entries_it->key.c_str(), key) == 0)
	  			{
	  				return entries_it->value;
	  			}
	  		}		
  		}
  	}
  	return empty_string;
}
