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
#include <iostream>
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::ios;
#include "Archive.h"
#include "filesnstreams.h"

using std::vector;

bool Archive::create( const char * path)
{
	bool	ret = true;
	
	m_fh.open( path, ios::in|ios::binary|ios::out|ios::trunc);
	if ( m_fh)
	{
	    m_arcpath = path;
	}
	else
	{
        cerr << "Failed to create archive " << path << endl;
        ret = false;
    }
    return ret;
}

void Archive::addfile( char attributes)
{
    m_data.push_back( vector<vector<char> >());
    m_attributes.push_back( attributes);
}

bool Archive::addchunk( const char * path)
{
    m_data.back().push_back( vector<char>());
    return load_file( path, m_data.back().back());
}

bool Archive::dump( void)
{
	bool	ret = true;
    const int filescount = (int)m_data.size();

    int tocsize = (int)(6 * filescount);

    char * info = new char[ 2 + tocsize];
    info[0] = (char)((tocsize >> 0) & 0xff);
    info[1] = (char)((tocsize >> 8) & 0xff);
    long dataoffset = 2 + tocsize;
    for ( int i = 0 ; i < filescount; ++i)
    {
        int tocoffset = 6 * i;
        info[ 2 + tocoffset + 0] = (char)((dataoffset >> 0) & 0xff);
        info[ 2 + tocoffset + 1] = (char)((dataoffset >> 8) & 0xff);
        info[ 2 + tocoffset + 2] = (char)((dataoffset >>16) & 0xff);
        info[ 2 + tocoffset + 3] = (char)((m_data[i].size() >> 0) & 0xff);
        info[ 2 + tocoffset + 4] = (char)((m_data[i].size() >> 8) & 0xff);
        info[ 2 + tocoffset + 5] = m_attributes[i];
        for ( int j = 0 ; j < (int)m_data[i].size(); ++j)
        {
            dataoffset += 2 + (long)(m_data[i][j].size());
        }
    }
    m_fh.write( info, 2 + tocsize);
    if ( m_fh)
    {
	    for ( int i = 0 ; ret && i < filescount; ++i)
	    {
	        for ( int j = 0 ; ret && j < (int)m_data[i].size(); ++j)
	        {
	            char chunkinfo[2];
	
	            chunkinfo[ 0] = (char)((m_data[i][j].size() >> 0) & 0xff);
	            chunkinfo[ 1] = (char)((m_data[i][j].size() >> 8) & 0xff);
	            m_fh.write( chunkinfo, 2);
	            if ( !m_fh)
	            {
	                cerr << "Failed to write all chunkheader bytes to archive file " << m_arcpath << endl;
	                ret = false;
	            }
	            else
	            {
	            	m_fh.write( &m_data[i][j][0], m_data[i][j].size());
	            	if ( !m_fh)
	                {
	                    cerr << "Failed to write all chunkdata bytes to archive file " << m_arcpath << endl;
	                    ret = false;
	                }
	            }
	        }
	    }
    }
    else
    {
        cerr << "Failed to write all header bytes to archive file " << m_arcpath << endl;
        ret = false;
    }
    delete info;
    return ret;
}
