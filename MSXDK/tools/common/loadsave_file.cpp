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
#include "loadsave_file.h"

using std::vector;

bool load_file( const char * path, vector<char> & data)
{
    FILE    *fh = fopen( path, "rb");
    if ( fh != NULL)
    {
        fseek( fh, 0, SEEK_END);
        size_t size = ftell( fh);
        fseek( fh, 0, SEEK_SET);
        data.resize( size);
        if ( fread( &data[0], 1, size, fh) != size)
        {
            fprintf( stderr, "Failed to read all bytes from %s\n", path);
            fclose( fh);
            return false;
        }
        fclose( fh);
        return true;
    }
    else
    {
        fprintf( stderr, "Failed to open %s for input\n", path);
        return false;
    }    
}

bool save_file( const char * path, char * data, size_t size)
{
    FILE    *fh = fopen( path, "wb");
    if ( fh != NULL)
    {
        if ( fwrite( data, 1, size, fh) != size)
        {
            fprintf( stderr, "Failed to write all bytes to %s\n", path);
            fclose( fh);
            return false;
        }
        fclose( fh);
        return true;
    }
    else
    {
        fprintf( stderr, "Failed to open %s for output\n", path);
        return false;
    }
}