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
#include <stdio.h>
#include <iostream>
using std::ofstream;
using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <map>
using std::map;
#include "FATDisk.h"
using std::fstream;
using std::ios;
using std::string;
using std::vector;

#define BOOTBLOCK_BYTES_PER_SECTOR         11
#define BOOTBLOCK_SECTORS_PER_CLUSTER      13
#define BOOTBLOCK_RESERVED_SECTORS         14
#define BOOTBLOCK_FAT_COPIES               16
#define BOOTBLOCK_ROOT_DIRECTORY_ENTRIES   17
#define BOOTBLOCK_LOGICAL_SECTORS          19
#define BOOTBLOCK_MEDIA_DESCRIPTOR         21
#define BOOTBLOCK_SECTORS_PER_FAT          22
#define BOOTBLOCK_SECTORS_PER_TRACK        24
#define BOOTBLOCK_SIDES                    26
#define BOOTBLOCK_HIDDEN_SECTORS           28
// Extended info block
#define BOOTBLOCK_BIG_LOGICAL_SECTORS      32

#define DIRECTORY_ENTRY_NAME				0
#define DIRECTORY_ENTRY_EXTENSION			8
#define DIRECTORY_ENTRY_ATTRIBUTE		   11
#define DIRECTORY_ENTRY_ATTRIBUTE_READ_ONLY		(1<<0)
#define DIRECTORY_ENTRY_ATTRIBUTE_HIDDEN		(1<<1)
#define DIRECTORY_ENTRY_ATTRIBUTE_SYSTEM		(1<<2)
#define DIRECTORY_ENTRY_ATTRIBUTE_VOLUME_LABEL	(1<<3)
#define DIRECTORY_ENTRY_ATTRIBUTE_DIRECTORY		(1<<4)
#define DIRECTORY_ENTRY_ATTRIBUTE_ARCHIVE		(1<<5)
#define DIRECTORY_ENTRY_ATTRIBUTE_RESERVED		(3<<6)

#define DIRECTORY_ENTRY_TIME			   22	
#define DIRECTORY_ENTRY_DATE			   24
#define DIRECTORY_ENTRY_FIRST_CLUSTER	   26
#define DIRECTORY_ENTRY_SIZE			   28	

#define MAXIMUM_SECTOR_SIZE 0xffff
#define FAT12_CLUSTERS	0xff0
#define FREE_CLUSTER	0
#define INVALID_CLUSTER	1
#define LAST_CLUSTER	( m_fat12 ? 0xff8 : 0xfff8)
#define IS_ROOT_DIRECTORY_CLUSTER( cluster)	( (cluster) == 0 )
#define IS_FREE_CLUSTER( cluster)			( (cluster) == 0 )
#define IS_INVALID_CLUSTER( cluster) 		( ((cluster) == 1) || ((cluster) >= m_clusters) )
#define IS_RESERVED_CLUSTER( cluster) 		( ((cluster) >= (m_fat12 ? 0xff0 : 0xfff0)) && ((cluster) <= (m_fat12 ? 0xff6 : 0xfff6)) )
#define IS_BADSECTOR_CLUSTER( cluster) 		( ((cluster) == (m_fat12 ? 0xff7 : 0xfff7)) )
#define IS_LAST_CLUSTER( cluster)			( ((cluster) >= (m_fat12 ? 0xff8 : 0xfff8)) && ((cluster) <= (m_fat12 ? 0xfff : 0xffff)) )

#define STRBYTE(address,byte)   ((BYTE*)(address))[0]=(BYTE)(byte)
#define STRWORD(address,word)   ((BYTE*)(address))[0]=(BYTE)(word);((BYTE*)(address))[1]=(BYTE)((word)>>8)
#define STRDWORD(address,dword)	((BYTE*)(address))[0]=(BYTE)(dword);((BYTE*)(address))[1]=(BYTE)((dword)>>8);((BYTE*)(address))[2]=(BYTE)((dword)>>16);((BYTE*)(address))[3]=(BYTE)((dword)>>24)
#define LDRBYTE(address)   		( ((BYTE*)(address))[0])
#define LDRWORD(address)   		( ((BYTE*)(address))[0] | (((BYTE*)(address))[1] << 8))
#define LDRDWORD(address)  		( ((BYTE*)(address))[0] | (((BYTE*)(address))[1] << 8) | (((BYTE*)(address))[2] << 16) | (((BYTE*)(address))[3] << 24))

static const BYTE g_bootblock_msx1[] = {
    
  0xEB,0xFE,0x90,0x44,0x4F,0x44,0x53,0x4B,0x30,0x30,0x31,0x00,0x02,0x02,0x01,0x00,
  0x02,0x70,0x00,0xA0,0x05,0xF9,0x03,0x00,0x09,0x00,0x02,0x00,0x00,0x00,0xD0,0xED,
  0x53,0x58,0xC0,0x32,0xC2,0xC0,0x36,0x55,0x23,0x36,0xC0,0x31,0x1F,0xF5,0x11,0x9D,
  0xC0,0x0E,0x0F,0xCD,0x7D,0xF3,0x3C,0x28,0x28,0x11,0x00,0x01,0x0E,0x1A,0xCD,0x7D,
  0xF3,0x21,0x01,0x00,0x22,0xAB,0xC0,0x21,0x00,0x3F,0x11,0x9D,0xC0,0x0E,0x27,0xCD,
  0x7D,0xF3,0xC3,0x00,0x01,0x57,0xC0,0xCD,0x00,0x00,0x79,0xE6,0xFE,0xFE,0x02,0x20,
  0x07,0x3A,0xC2,0xC0,0xA7,0xCA,0x22,0x40,0x11,0x77,0xC0,0x0E,0x09,0xCD,0x7D,0xF3,
  0x0E,0x07,0xCD,0x7D,0xF3,0x18,0xB4,0x42,0x6F,0x6F,0x74,0x20,0x65,0x72,0x72,0x6F,
  0x72,0x0D,0x0A,0x50,0x72,0x65,0x73,0x73,0x20,0x61,0x6E,0x79,0x20,0x6B,0x65,0x79,
  0x20,0x66,0x6F,0x72,0x20,0x72,0x65,0x74,0x72,0x79,0x0D,0x0A,0x24,0x00,0x4D,0x53,
  0x58,0x44,0x4F,0x53,0x20,0x20,0x53,0x59,0x53,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF3,0x2A,
  0x51,0xF3,0x11,0x00,0x01,0x19,0x01,0x00,0x01,0x11,0x00,0xC1,0xED,0xB0,0x3A,0xEE,
  0xC0,0x47,0x11,0xEF,0xC0,0x21,0x00,0x00,0xCD,0x51,0x52,0xF3,0x76,0xC9,0x18,0x64,
  0x3A,0xAF,0x80,0xF9,0xCA,0x6D,0x48,0xD3,0xA5,0x0C,0x8C,0x2F,0x9C,0xCB,0xE9,0x89,
  0xD2,0x00,0x32,0x26,0x40,0x94,0x61,0x19,0x20,0xE6,0x80,0x6D,0x8A,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

struct format_t {
    size_t			bytes_per_sector;
    FATDisk::SECTOR	sectors_per_cluster;
    FATDisk::SECTOR	reserved_sectors;
    int     		fat_copies;
    int     		root_directory_entries;
    FATDisk::SECTOR	logical_sectors;
    int     		media_descriptor;
    FATDisk::SECTOR	sectors_per_fat;
    FATDisk::SECTOR	sectors_per_track;
    int     		sides;
    FATDisk::SECTOR	hidden_sectors;
};

map<int, format_t>  	g_formats;

struct bootblock_t {
	const BYTE	* data;
	size_t		  size;
};

map<int, bootblock_t>	g_bootblocks;

void init_fatdisk( void)
{
    format_t    format;

    format.bytes_per_sector         = 512;
    format.sectors_per_cluster      = 2;
    format.reserved_sectors         = 1;
    format.fat_copies               = 2;
    format.root_directory_entries   = 112;
    format.logical_sectors          = 1*80*9;
    format.media_descriptor         = 0xf9;
    format.sectors_per_fat          = 3;
    format.sectors_per_track        = 9;
    format.sides                    = 2;
    format.hidden_sectors           = 0;
    g_formats[ FATDisk::FORMAT_360] = format;

    format.bytes_per_sector         = 512;
    format.sectors_per_cluster      = 2;
    format.reserved_sectors         = 1;
    format.fat_copies               = 2;
    format.root_directory_entries   = 112;
    format.logical_sectors          = 2*80*9;
    format.media_descriptor         = 0xf9;
    format.sectors_per_fat          = 3;
    format.sectors_per_track        = 9;
    format.sides                    = 2;
    format.hidden_sectors           = 0;
    g_formats[ FATDisk::FORMAT_720] = format;
    
    bootblock_t	bootblock;
    bootblock.data					= g_bootblock_msx1;
    bootblock.size					= sizeof( g_bootblock_msx1);
    g_bootblocks[ FATDisk::BOOTBLOCK_MSX1] = bootblock;
}

void destroy_fatdisk( void)
{
    g_formats.clear();
    g_bootblocks.clear();
}

void report_stat_error( const string & object_name)
{
	switch ( errno)
	{
	case ENOENT:
        cerr << object_name << " not found " << endl;
        break;
    case EACCES:
    	cerr << "Access denied on some part of the path leading to " << object_name << endl;
    	break;
    case ENOTDIR:
    	cerr << "A component of the path leading to " << object_name << " is not a directory" << endl;
    	break;
    case ENAMETOOLONG:
    	cerr << "Filename too long for " << object_name << endl;
    	break;
    default:
        cerr << "Failed to open " << object_name << " for some obscure reason (errno=" << errno << ")" << endl;
        break;
    }
}

FATDisk::FATDisk()
{
}

FATDisk::~FATDisk()
{
    close();
}

bool FATDisk::read_sector( SECTOR sector, BYTE * data)
{
    bool    ret = true;

	m_file.seekg( static_cast<size_t>(sector) * m_bytes_per_sector);
	if ( !m_file.fail())
	{		
		m_file.read( reinterpret_cast<char*>(data), m_bytes_per_sector);
	}
	if ( m_file.fail())
	{
        cerr << "Failed to read a sector from the disk image. Is it corrupt?" << endl;
        ret = false;
	}
    return ret;
}

bool FATDisk::write_sector( SECTOR sector, const BYTE * data)
{
    bool    ret = true;

	m_file.seekp( static_cast<size_t>(sector) * m_bytes_per_sector);
	if ( !m_file.fail())
	{
		m_file.write( reinterpret_cast<const char*>(data), m_bytes_per_sector);
	}
	if ( m_file.fail())
	{
        cerr << "Failed to write a sector to the disk image. It is now probably corrupt!" << endl;
        ret = false;
    }
    return ret;
}

bool FATDisk::read_bytes( size_t position, BYTE * data, const size_t size)
{
    bool    ret = true;

	m_file.seekg( position);
	if ( !m_file.fail())
	{		
		m_file.read( reinterpret_cast<char*>(data), size);
	}
	if ( m_file.fail())
	{
        cerr << "Failed to read bytes from the disk image. Is it corrupt?" << endl;
        ret = false;
	}
    return ret;
}

bool FATDisk::write_bytes( size_t position, const BYTE * data, const size_t size)
{
    bool    ret = true;

	m_file.seekp( position);
	if ( !m_file.fail())
	{
		m_file.write( reinterpret_cast<const char*>(data), size);
	}
	if ( m_file.fail())
	{
        cerr << "Failed to write bytes to the disk image. It is now probably corrupt!" << endl;
        ret = false;
    }
    return ret;
}

bool FATDisk::open( const string & dsk, const int mode, const int format_id, const int bootblock_id)
{
    bool    ret = true;

    #ifdef DEBUG
    if ( m_file.is_open())
    {
        cerr << "Previous disk image still open! This should not occur! Contact the author of this crappy piece of software." << endl;
        return false;
    }
    #endif

    if ( ret)
    {
        //
        // First make sure the file exists
        //
        struct stat	file_stat;
        if ( stat( dsk.c_str(), &file_stat) == 0)
        {
        	// At least the "object" exists, now see if it's a file.
        	if ( !S_ISREG( file_stat.st_mode))
        	{
        		cerr << dsk << " found, but it's not a regular file" << endl;
        		ret = false;
        	}
        }
        else
        {
            if ((errno == ENOENT) && !(mode & OPEN_READONLY) && !(mode & OPEN_MUSTEXIST))
            {
                // File doesn't exist. Since we're trying to open it for write
                // and the "dsk must exist" option was NOT specified we can try to create it.
                m_file.open( dsk.c_str(), ios::out|ios::binary);
                if ( m_file.is_open())
                {
                    format_t	& format = g_formats[ format_id != FORMAT_NONE ? format_id : FORMAT_720];

                    BYTE buffer[ MAXIMUM_SECTOR_SIZE];

                    // First fill the entire .dsk with zeros.
                    memset( buffer, 0, format.bytes_per_sector);
                    for ( SECTOR sector = 0 ; sector < format.logical_sectors; ++sector)
                    {
                    	m_file.write( reinterpret_cast<const char*>(buffer), format.bytes_per_sector);                    	
                    	if ( m_file.fail())
                        {
                            cerr << "Failed to create the disk image " << dsk << endl;
                            ret = false;
                            break;
                        }
                    }
                    if ( ret)
                    {
                        // Generate and write the bootblock
                        bootblock_t	& bootblock = g_bootblocks[ bootblock_id != BOOTBLOCK_NONE ? bootblock_id : BOOTBLOCK_MSX1];
                        if ( bootblock.size <= format.bytes_per_sector)
                        {
	                        memcpy( buffer, bootblock.data, bootblock.size);
	
	                        STRWORD( buffer + BOOTBLOCK_BYTES_PER_SECTOR,      format.bytes_per_sector);
	                        STRBYTE( buffer + BOOTBLOCK_SECTORS_PER_CLUSTER,   format.sectors_per_cluster);
	                        STRWORD( buffer + BOOTBLOCK_RESERVED_SECTORS,      format.reserved_sectors);
	                        STRBYTE( buffer + BOOTBLOCK_FAT_COPIES,            format.fat_copies);
	                        STRWORD( buffer + BOOTBLOCK_ROOT_DIRECTORY_ENTRIES,format.root_directory_entries);
	                        if ( format.logical_sectors < 0x10000)
	                        {
	                        	STRWORD( buffer + BOOTBLOCK_LOGICAL_SECTORS,       format.logical_sectors);
	                        }
	                        else
	                        {
	                        	STRWORD( buffer + BOOTBLOCK_LOGICAL_SECTORS,       0);
	                        	STRDWORD( buffer + BOOTBLOCK_BIG_LOGICAL_SECTORS,	format.logical_sectors);
	                        }
	                        STRBYTE( buffer + BOOTBLOCK_MEDIA_DESCRIPTOR,      format.media_descriptor);
	                        STRWORD( buffer + BOOTBLOCK_SECTORS_PER_FAT,       format.sectors_per_fat);
	                        STRWORD( buffer + BOOTBLOCK_SECTORS_PER_TRACK,     format.sectors_per_track);
	                        STRWORD( buffer + BOOTBLOCK_SIDES,                 format.sides);
	                        STRWORD( buffer + BOOTBLOCK_HIDDEN_SECTORS,        format.hidden_sectors);

	                        m_bytes_per_sector = format.bytes_per_sector;
	                        if ( ret = write_sector( 0, buffer))
	                        {
	                            // Write the FAT(s)
	                            for ( int fat_index = 0 ; ret && fat_index < format.fat_copies; ++fat_index)
	                            {
	                                memset( buffer, 0, format.bytes_per_sector);
	                                buffer[ 0] = format.media_descriptor;
	                                buffer[ 1] = 0xff;
	                                buffer[ 2] = 0xff;
	                                if ( 
	                                	( 
	                                	2 + ( format.logical_sectors - 
	                                			(
	                                			format.reserved_sectors + 
	                                			format.hidden_sectors + 
	                                			format.fat_copies * format.sectors_per_track +
	                                			(format.root_directory_entries * 32 + format.bytes_per_sector - 1) / format.bytes_per_sector
	                                			) / format.sectors_per_cluster	                                			
	                                		)	                                	
	                                	) > FAT12_CLUSTERS
	                                	)
	                               	{
	                               		// FAT16
	                                	buffer[3] = 0xff;
	                                }
	                                ret = write_sector( format.reserved_sectors + format.hidden_sectors + fat_index * format.sectors_per_fat, buffer);
	                            }
	                    	}
                        }
                        else
                        {
                        	cerr << "Size of bootblock bigger than the sector size" << endl;
                        	ret = false;
                        }                        
                    }
                    m_file.close();
                    if ( !ret)
                    {
                        // Something went wrong, so just delete the created disk image again
                        remove( dsk.c_str());
                    }
                }
                else
                {
                    cerr << "Failed to create file " << dsk << endl;
                    ret = false;
                }
            }
            else
            {
            	report_stat_error( dsk);
                ret = false;
            }
        }
    }

    if ( ret)
    {
        //
        // We garuanteed that the file exists. Now open it.
        //
        m_file.open( dsk.c_str(), mode & OPEN_READONLY ? ios::in|ios::binary : ios::in|ios::out|ios::binary);
        if ( m_file.is_open())
        {
            BYTE buffer[ MAXIMUM_SECTOR_SIZE];
            m_bytes_per_sector = 128;
            if ( ret = read_sector( 0, buffer))
            {
                m_bytes_per_sector 			= LDRWORD( buffer + BOOTBLOCK_BYTES_PER_SECTOR);
				m_fat_copies 				= LDRBYTE( buffer + BOOTBLOCK_FAT_COPIES);
				m_sectors_per_fat			= LDRWORD( buffer + BOOTBLOCK_SECTORS_PER_FAT);
				m_root_directory_entries 	= LDRWORD( buffer + BOOTBLOCK_ROOT_DIRECTORY_ENTRIES);
				SECTOR	logical_sectors 	= LDRWORD( buffer + BOOTBLOCK_LOGICAL_SECTORS);
				BYTE	media_descriptor	= LDRBYTE( buffer + BOOTBLOCK_MEDIA_DESCRIPTOR);
				if ( logical_sectors == 0)
				{
					logical_sectors			= LDRDWORD( buffer + BOOTBLOCK_BIG_LOGICAL_SECTORS);
				}
				m_fat_sector = LDRWORD( buffer + BOOTBLOCK_RESERVED_SECTORS) + LDRWORD( buffer + BOOTBLOCK_HIDDEN_SECTORS);
    			m_root_directory_sector = m_fat_sector + m_fat_copies * LDRWORD( buffer + BOOTBLOCK_SECTORS_PER_FAT);
				m_sectors_in_root_directory = (32 * m_root_directory_entries + m_bytes_per_sector - 1) / m_bytes_per_sector;
    			m_cluster_sector = m_root_directory_sector + m_sectors_in_root_directory;
    			SECTOR	sectors_per_cluster = LDRBYTE( buffer + BOOTBLOCK_SECTORS_PER_CLUSTER);
    			m_clusters = (CLUSTER)( 2 + (logical_sectors - m_cluster_sector) / sectors_per_cluster);
				m_bytes_per_cluster = sectors_per_cluster * m_bytes_per_sector;
    			m_fat12 = ( m_clusters <= FAT12_CLUSTERS);
                
                //
                // Perform some sanity checks
                //
				if ( logical_sectors && m_fat_copies && ((m_fat12 ? 12 : 16) * m_clusters <= (m_sectors_per_fat * m_bytes_per_sector * 8)))
				{               
					//xxx check whether m_bytes_per_sector is a power of 2					
					
	                // Is the image too small?
	                m_file.seekg( m_bytes_per_sector * logical_sectors);
	                if ( !m_file.fail())
	                {
	                	// Do all the FATs contain the correct Media Descriptor?
	                	for ( int fat_index = 0 ; ret && fat_index < m_fat_copies; ++fat_index)
	                	{
	                		ret = read_sector( m_fat_sector + fat_index * m_sectors_per_fat, buffer);
	                		if ( ret)
	                		{
	                			if ( buffer[0] != media_descriptor)
	                			{
	                				cerr << "Invalid disk image: FAT " << fat_index << " does not start with a copy of the media descriptor" << endl;
	                				ret = false;
	                			}
	                		}
	                	}
	                	//xxx should compare FAT 1,2,.. to FAT 0, but only for m_clusters, as bytes
	                	//after the cluster infos in the FAT might be being used to store other info
	                }
	                else
	                {
	                	cerr << "Invalid disk image: (bootblock: bytes per sector * bootblock: number of sectors) > image size" << endl;
	                	ret = false;
	                }
	        	}
	        	else
	        	{
        			cerr << "Invalid disk image: ";
					if ( !logical_sectors)
	        		{
		        		cerr << "bootblock: number of sectors is zero";
	        		}
	        		else if ( !m_fat_copies)
	        		{
	        			cerr << "bootblock: number of FAT copies is zero";
	        		}
	        		else
	        		{
	        			cerr << "not enough room per FAT to store all the cluster numbers needed to cover all sectors" << endl;	        			
	        		}
	        		cerr << endl;
	        		ret = false;
	        	}
            }
        }
        else
        {
        	switch (errno)
        	{
        	case EACCES:
        		cerr << "Access denied on opening file " << dsk << endl;
        		break;
        	default:
            	cerr << "Failed to open file " << dsk << " for some obscure reason (errno=" << errno << ")" << endl;
            	break;
            }
        	ret = false;
        }
    }

	/*
    if ( ret)
    {
	cout << "m_fat12: " << m_fat12 << endl;
    cout << "m_file: " << m_file << endl;
    cout << "m_bytes_per_sector: " << m_bytes_per_sector << endl;
    cout << "m_root_directory_sector: " << m_root_directory_sector << endl;
    cout << "m_root_directory_entries: " << m_root_directory_entries << endl;
    cout << "m_fat_sector: " << m_fat_sector << endl;
    cout << "m_fat_copies: " << m_fat_copies << endl;
    cout << "m_clusters: " << m_clusters << endl;
    cout << "m_cluster_sector: " << m_cluster_sector << endl;
    cout << "m_bytes_per_cluster: " << m_bytes_per_cluster << endl;
    cout << "m_sectors_per_fat: " << m_sectors_per_fat << endl;
    }
    */

    if ( !ret)
    {
        close();
    }
    return ret;
}

bool FATDisk::close( void)
{
    bool    ret = true;

	if (  m_file.is_open())
	{
		m_file.close();
	}
    //xxx
    return ret;
}

bool FATDisk::set_fat_entry( const CLUSTER cluster, const CLUSTER value)
{
	bool	ret = true;
	
	if ( cluster < m_clusters)
	{
		size_t	offset = (cluster * ( m_fat12 ? 12 : 16)) / 8;
		int		shift = m_fat12 ? (cluster & 1)*4 : 0;
		CLUSTER	mask = m_fat12 ? ( cluster & 1 ? 0x000f : 0xf000) : 0xffff;
		
		for ( int fat_copy = 0 ; ret && (fat_copy < m_fat_copies); ++fat_copy)
		{
			BYTE	bytes[2];
			ret = read_bytes( (m_fat_sector * m_bytes_per_sector) + offset, bytes, 2);
			if ( ret)
			{
				bytes[0] = (bytes[0] & mask) | ( cluster << shift);
				bytes[1] = (bytes[1] & (mask >> 8)) |  (cluster >> (8-shift));
				ret = write_bytes( (m_fat_sector * m_bytes_per_sector) + offset, bytes, 2);
			}
		}
	}
	else
	{
		cerr << "Tried to write the FAT entry for a cluster number that's too big" << endl;
		ret = false;
	}
	return ret;
}

bool FATDisk::get_fat_entry( const CLUSTER cluster, CLUSTER & value)
{
	bool	ret = true;

	if ( cluster < m_clusters)
	{
		size_t	offset = (cluster * ( m_fat12 ? 12 : 16)) / 8;
		int		shift = m_fat12 ? (cluster & 1)*4 : 0;

		BYTE	bytes[2];
		
		ret = read_bytes( (m_fat_sector * m_bytes_per_sector) + offset, bytes, 2);
		if ( ret)
		{	
			value = bytes[0] | ( bytes[1] << 8);
			value >>= shift;
			value &= m_fat12 ? 0xfff : 0xffff;
		}
	}
	else
	{
		cerr << "Tried to read the FAT entry for a cluster number that's too big" << endl;
		ret = false;
	}
	return ret;
}

bool FATDisk::verify_object( const CLUSTER first_cluster, const bool directory, CLUSTER & clusters)
{
	bool	ret = true;

	if (
		IS_INVALID_CLUSTER( first_cluster) ||
		IS_RESERVED_CLUSTER( first_cluster) ||
		IS_BADSECTOR_CLUSTER( first_cluster) ||
		IS_LAST_CLUSTER( first_cluster) ||
		(first_cluster >= m_clusters)
		) 
	{
		cerr << "Some object has the invalid first cluster number " << first_cluster << endl;
		ret = false;
	}
	else
	{
		if ( directory && IS_ROOT_DIRECTORY_CLUSTER( first_cluster))
		{
			// Root directory is always just 1 cluster
			clusters = 1;
		}
		else
		{
			if ( IS_FREE_CLUSTER( first_cluster))
			{
				clusters = 0;
			}
			else
			{
				CLUSTER		cluster = first_cluster;
				clusters = 1;
				while ( ret)
				{
					CLUSTER		next;
					ret = get_fat_entry( cluster, next);
					if ( ret)
					{
						if ( IS_LAST_CLUSTER( next))
						{
							break;
						}
						if (
							IS_INVALID_CLUSTER( next) ||
							IS_RESERVED_CLUSTER( next) ||
							IS_BADSECTOR_CLUSTER( next) ||
							(next >= m_clusters) ||
							(clusters >= m_clusters)
							) 
						{
							cerr << "Invalid disk image: the object starting at cluster " << first_cluster << " in the FAT contains ";
							if ( clusters >= m_clusters)
							{
								cerr << "an infinite loop";
							}
							else
							{
								cerr << "an invalid cluster number " << next;
							}
							cerr << endl;
							ret = false;
						}
						else
						{
							clusters++;
							cluster = next;
						}
					}					
				}
			}
		}
	}
	return ret;
}

bool FATDisk::verify_object( const CLUSTER first_cluster, const bool directory)
{
	CLUSTER	clusters;
	return verify_object( first_cluster, directory, clusters);
}

bool FATDisk::set_object_size( CLUSTER & first_cluster, const CLUSTER clusters, const bool nullify)
{
	bool	ret = true;
	
	CLUSTER	old_clusters;
	ret = verify_object( first_cluster, false, old_clusters);
	if ( ret && ( clusters != old_clusters))
	{
		if ( clusters > old_clusters)
		{
			// Grow object
			CLUSTER	free_clusters = 0;
			for ( CLUSTER free_cluster = 0 ; ret && (free_cluster < m_clusters); ++free_cluster)
			{
				CLUSTER	value;
				ret = get_fat_entry( free_cluster, value);
				if ( IS_FREE_CLUSTER( value))
				{
					free_clusters++;
				}
			}
			if ( ret)
			{
				if ( free_clusters < ( clusters - old_clusters))
				{
					cerr << "Not enough free space" << endl;
					ret = false;
				}
			}
			if ( ret)
			{						
				CLUSTER	count = 0;
				CLUSTER cluster = first_cluster;
				while ( ret && ( count < clusters))
				{
					CLUSTER next = LAST_CLUSTER;
					if ( count < old_clusters)
					{
						ret = get_fat_entry( cluster, next);
					}
					if ( IS_LAST_CLUSTER( next))
					{
						next = FREE_CLUSTER;
						for ( CLUSTER free_cluster = 0 ; ret && (free_cluster < m_clusters); ++free_cluster)
						{
							ret = get_fat_entry( free_cluster, next);
							if ( IS_FREE_CLUSTER( next))
							{
								if ( nullify)
								{
									size_t position;
									size_t size;
									ret = cluster_to_position_size( free_cluster, position, size);
									if ( ret)
									{
										vector<BYTE>	nulls;
										nulls.resize( size);
										memset( &nulls[0], 0, size);
										ret = write_bytes( position, &nulls[0], size);
									}									
								}
								next = free_cluster;
								break;
							}
						}
					}
					if ( ret)
					{
						if ( IS_FREE_CLUSTER( cluster))
						{
							first_cluster = next;
						}
						else
						{
							ret = set_fat_entry( cluster, next);
						}
						cluster = next;
					}
				}
				if ( ret)
				{
					ret = set_fat_entry( cluster, LAST_CLUSTER);
				}
			}
		}
		else
		{
			// Shrink object
			CLUSTER	cluster = first_cluster;
			CLUSTER	count = 0;
			while ( ret && !IS_LAST_CLUSTER( cluster))
			{
				CLUSTER	next;
				ret = get_fat_entry( cluster, next);
				if ( ret)
				{
					if ( count >= clusters)
					{
						ret = set_fat_entry( cluster, FREE_CLUSTER);
					}
					cluster = next;
					count++;
				}
			}			
			if ( clusters == 0)				
			{
				first_cluster = FREE_CLUSTER;
			}
		}
	}		
	return ret;
}

bool FATDisk::get_directory_entries( const CLUSTER directory, int & entries)
{
	bool	ret = true;

	if ( IS_ROOT_DIRECTORY_CLUSTER( directory))
	{
		entries = m_root_directory_entries;
	}
	else
	{
		CLUSTER	clusters;
		ret = verify_object( directory, true, clusters);
		if ( ret)
		{
			entries = (int)(clusters * m_bytes_per_cluster / 32);
		}
	}
	return ret;
}

bool FATDisk::directory_index_to_cluster_index( const CLUSTER directory, const int index, CLUSTER & cluster, int & sub_index)
{
	bool	ret = true;
	
	int	entries;
	ret = get_directory_entries( directory, entries);
	if ( ret)
	{
		if ( index < entries)
		{
			if ( IS_ROOT_DIRECTORY_CLUSTER( directory))
			{
				cluster = directory;
				sub_index = index;				
			}
			else
			{
				cluster = directory;
				CLUSTER sub_cluster = index / ( m_bytes_per_cluster / 32);
				while ( ret && sub_cluster--)
				{
					CLUSTER 	next;
					ret = get_fat_entry( cluster, next);
					if ( ret)
					{
						cluster = next;
					}
				}
				sub_index = (int)( index % ( m_bytes_per_cluster / 32));
			}
		}
		else
		{
			cerr << "Directory entry index out of range" << endl;
			ret = false;
		}
	}
	return ret;
}

bool FATDisk::get_directory_entry( const CLUSTER directory, const int index, object_info_t & object_info)
{
	bool	ret = true;
	
	CLUSTER		cluster;
	int			sub_index;
	ret = directory_index_to_cluster_index( directory, index, cluster, sub_index);
	if ( ret)
	{
		vector<BYTE>	data;
		ret = read_cluster( cluster, data);
		if ( ret)
		{
			const BYTE * entry = &data[32 * sub_index];
			object_info.directory		= directory;
			object_info.index 			= index;
			memcpy( object_info.name, entry + DIRECTORY_ENTRY_NAME, 8);
			object_info.name[8] = '\0';
			memcpy( object_info.extension, entry + DIRECTORY_ENTRY_EXTENSION, 3);
			object_info.extension[3] = '\0';
			object_info.first_cluster	= LDRWORD( entry + DIRECTORY_ENTRY_FIRST_CLUSTER);
			object_info.size 			= LDRDWORD( entry + DIRECTORY_ENTRY_SIZE);
			object_info.attributes.read_only 	= entry[ DIRECTORY_ENTRY_ATTRIBUTE] & DIRECTORY_ENTRY_ATTRIBUTE_READ_ONLY;
			object_info.attributes.hidden 		= entry[ DIRECTORY_ENTRY_ATTRIBUTE] & DIRECTORY_ENTRY_ATTRIBUTE_HIDDEN;
			object_info.attributes.system 		= entry[ DIRECTORY_ENTRY_ATTRIBUTE] & DIRECTORY_ENTRY_ATTRIBUTE_SYSTEM;
			object_info.attributes.volume_label = entry[ DIRECTORY_ENTRY_ATTRIBUTE] & DIRECTORY_ENTRY_ATTRIBUTE_VOLUME_LABEL;
			object_info.attributes.directory 	= entry[ DIRECTORY_ENTRY_ATTRIBUTE] & DIRECTORY_ENTRY_ATTRIBUTE_DIRECTORY;
			object_info.attributes.archive		= entry[ DIRECTORY_ENTRY_ATTRIBUTE] & DIRECTORY_ENTRY_ATTRIBUTE_ARCHIVE;			
		}
	}
	return ret;
}

bool FATDisk::set_directory_entry( const object_info_t & object_info)
{
	bool	ret = true;
	
	CLUSTER		cluster;
	int			sub_index;
	ret = directory_index_to_cluster_index( object_info.directory, object_info.index, cluster, sub_index);
	if ( ret)
	{
		vector<BYTE>	data;
		ret = read_cluster( cluster, data);
		if ( ret)
		{
			BYTE * entry = &data[32 * sub_index];
			memcpy( entry + DIRECTORY_ENTRY_NAME, object_info.name, 8);
			memcpy( entry + DIRECTORY_ENTRY_EXTENSION, object_info.extension, 3);
			STRWORD( entry + DIRECTORY_ENTRY_FIRST_CLUSTER, object_info.first_cluster);
			STRDWORD( entry + DIRECTORY_ENTRY_SIZE, object_info.size);
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] &= DIRECTORY_ENTRY_ATTRIBUTE_RESERVED;
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] |= object_info.attributes.read_only 	? DIRECTORY_ENTRY_ATTRIBUTE_READ_ONLY : 0;
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] |= object_info.attributes.hidden 		? DIRECTORY_ENTRY_ATTRIBUTE_HIDDEN : 0;
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] |= object_info.attributes.system 		? DIRECTORY_ENTRY_ATTRIBUTE_SYSTEM : 0;
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] |= object_info.attributes.volume_label? DIRECTORY_ENTRY_ATTRIBUTE_VOLUME_LABEL : 0;
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] |= object_info.attributes.directory 	? DIRECTORY_ENTRY_ATTRIBUTE_DIRECTORY : 0;
			entry[ DIRECTORY_ENTRY_ATTRIBUTE] |= object_info.attributes.archive 	? DIRECTORY_ENTRY_ATTRIBUTE_ARCHIVE : 0;			
			ret = write_cluster( cluster, data);
		}
	}
	return ret;
}

bool FATDisk::new_directory_entry( object_info_t & object_info)
{
	bool	ret = true;
	
	int entries;
	ret = get_directory_entries( object_info.directory, entries);
	if ( ret)
	{
		object_info_t	var_object_info;
		bool			found = false;
		for ( int index = 0 ; ret && !found && ( index < entries); ++index)
		{
			ret = get_directory_entry( object_info.directory, index, var_object_info);
			if ( ret)
			{
				if ( (var_object_info.name[0] == '\0') || (var_object_info.name[0] == DELETED_ENTRY_CHAR))
				{
					object_info.index = index;
					found = true;
				}
			}
		}
		if ( ret && !found)
		{
			// No free entry found, so try to enlarge the directory
			if ( IS_ROOT_DIRECTORY_CLUSTER( object_info.directory))
			{
				cerr << "Root directory full" << endl;
				ret = false;
			}
			else
			{
				CLUSTER clusters;
				ret = verify_object( object_info.directory, true, clusters);
				if ( ret)
				{
					clusters++;
					CLUSTER directory = object_info.directory;
					ret = set_object_size( directory, clusters, true);
					if ( ret)
					{
						object_info.index = entries;
					}
				}				
			}
		}
		if ( ret)
		{
			ret = set_directory_entry( object_info);
		}
	}
	return ret;
}

bool FATDisk::del_directory_entry( object_info_t & object_info)
{
	bool	ret = true;
	
	if ( !IS_FREE_CLUSTER( object_info.first_cluster))
	{
		ret = set_object_size( object_info.first_cluster, 0);
	}		
	if ( ret)
	{
		object_info.name[0] = DELETED_ENTRY_CHAR;
		ret = set_directory_entry( object_info);
	}
	return ret;
}
    
bool FATDisk::cluster_to_position_size( const CLUSTER cluster, size_t & position, size_t & size)
{
	bool	ret = true;
	
	if ( IS_ROOT_DIRECTORY_CLUSTER( cluster))
	{
		position = m_root_directory_sector * m_bytes_per_sector;
		size = m_sectors_in_root_directory * m_bytes_per_sector;
	}
	else
	{
		if ( cluster < m_clusters)
		{
			position = (m_cluster_sector * m_bytes_per_sector) + (( cluster - 2) * m_bytes_per_cluster);
			size = m_bytes_per_cluster;
		}
		else
		{
			cerr << "Cluster number " << cluster << " is too big" << endl;
			ret = false;
		}
	}	
	return ret;	
}

bool FATDisk::read_cluster( const CLUSTER cluster, vector<BYTE> & data)
{
	bool	ret = true;

	size_t 	position;
	size_t	size;
	ret = cluster_to_position_size( cluster, position, size);
	if ( ret)
	{
		data.resize( size);
		ret = read_bytes( position, &data[0], size);
	}
	return ret;
}

bool FATDisk::write_cluster( const CLUSTER cluster, const vector<BYTE> & data)
{
	bool	ret = true;
	
	size_t 	position;
	size_t	size;
	ret = cluster_to_position_size( cluster, position, size);
	if ( ret)
	{
		ret = write_bytes( position, &data[0], size);
	}
	return ret;
}

bool FATDisk::read_object( const CLUSTER first_cluster, const DWORD size, ofstream & output)
{
	bool	ret = true;
	
	ret = verify_object( first_cluster, false);
	if ( ret)
	{
		size_t	left = size;
		CLUSTER	cluster = first_cluster;
		while ( ret && (left > 0))
		{
			vector<BYTE>	data;
			ret = read_cluster( cluster, data);
			if ( ret)
			{
				size_t	write_size = left > data.size() ? data.size() : left;
				output.write( reinterpret_cast<const char*>(&data[0]), write_size);
				if ( !output.fail())
				{
					CLUSTER	next;
					ret = get_fat_entry( cluster, next);
					if ( ret)
					{
						cluster = next;
						left -= write_size;
					}
				}
				else
				{
					cerr << "Failed to write some bytes to the output file" << endl;
					ret = false;
				}
			}
		}
	}
	return ret;
}

bool FATDisk::write_object( const CLUSTER first_cluster, const DWORD size, ifstream & input)
{
	bool	ret = true;
	
	ret = verify_object( first_cluster, false);
	if ( ret)
	{
		size_t	left = size;
		CLUSTER	cluster = first_cluster;
		while ( ret && (left > 0))
		{
			vector<BYTE>	data;
			size_t	read_size = left > data.size() ? data.size() : left;
			ret = read_cluster( cluster, data);
			if ( ret)
			{
				input.read( reinterpret_cast<char*>(&data[0]), read_size);
				if ( !input.fail())
				{
					ret = write_cluster( cluster, data);
					if ( ret)
					{
						CLUSTER	next;
						ret = get_fat_entry( cluster, next);
						if ( ret)
						{
							cluster = next;
							left -= read_size;
						}
					}
				}
				else
				{
					cerr << "Failed to read some bytes from the input file" << endl;
					ret = false;
				}
			}
		}
	}
	return ret;
}
