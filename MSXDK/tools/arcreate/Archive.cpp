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
#include "Archive.h"
#include "loadsave_file.h"

using std::vector;

Archive::Archive()
{
    m_fh = NULL;
}

Archive::~Archive()
{
    if ( m_fh != NULL)
    {
        fclose( m_fh);
        m_fh = NULL;
    }
}

bool Archive::create( const char * path)
{
    m_fh = fopen( path, "wb");
    if ( m_fh == NULL)
    {
        fprintf( stderr, "Failed to create archive %s\n", path);
        return false;
    }
    strcpy( m_arcpath, path);
    return true;
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
    if ( (int)fwrite( info, 1, 2 + tocsize, m_fh) != (2 + tocsize))
    {
        fprintf( stderr, "Failed to write all header bytes to archive file %s\n", m_arcpath);
        return false;
    }
    for ( int i = 0 ; i < filescount; ++i)
    {
        for ( int j = 0 ; j < (int)m_data[i].size(); ++j)
        {
            char chunkinfo[2];

            chunkinfo[ 0] = (char)((m_data[i][j].size() >> 0) & 0xff);
            chunkinfo[ 1] = (char)((m_data[i][j].size() >> 8) & 0xff);
            if ( fwrite( chunkinfo, 1, 2, m_fh) != 2)
            {
                fprintf( stderr, "Failed to write all chunkheader bytes to archive file %s\n", m_arcpath);
                return false;
            }
            else
            {
                if ( fwrite( &m_data[i][j][0], 1, m_data[i][j].size(), m_fh) != m_data[i][j].size())
                {
                    fprintf( stderr, "Failed to write all chunkdata bytes to archive file %s\n", m_arcpath);
                    return false;
                }
            }
        }
    }
    delete info;
    return true;
}
