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

#ifndef _FATDISK_H_
#define _FATDISK_H_

#include <fstream>
#include <string>
#include <vector>

void init_fatdisk( void);
void destroy_fatdisk( void);

void report_stat_error( const std::string & object_name);

class FATDisk {
public:
	typedef	unsigned long	SECTOR;
	typedef unsigned long	CLUSTER;
	#define ROOT_DIRECTORY_CLUSTER	0
	#define DELETED_ENTRY_CHAR ((char)0xe5)
private:
	
	bool			m_fat12;
    std::fstream	m_file;
    size_t			m_bytes_per_sector;
    SECTOR			m_root_directory_sector;
    int				m_root_directory_entries;
    SECTOR			m_sectors_in_root_directory;
    SECTOR			m_fat_sector;
    int				m_fat_copies;
    CLUSTER			m_clusters;
    SECTOR			m_cluster_sector;
    SECTOR			m_sectors_per_fat;
    size_t			m_bytes_per_cluster;
    
    bool	read_sector( SECTOR sector, BYTE * data);
    bool    write_sector( SECTOR sector, const BYTE * data);
	bool 	read_bytes( size_t position, BYTE * data, const size_t size);
	bool 	write_bytes( size_t position, const BYTE * data, const size_t size);
	bool 	set_fat_entry( const CLUSTER cluster, const CLUSTER value);
	bool 	get_fat_entry( const CLUSTER cluster, CLUSTER & value);
	bool	is_valid_first_cluster( const CLUSTER cluster);
	bool	verify_object( const CLUSTER first_cluster, const bool directory, CLUSTER & clusters);
	bool	verify_object( const CLUSTER first_cluster, const bool directory);
	bool 	directory_index_to_cluster_index( const CLUSTER directory, const int index, CLUSTER & cluster, int & sub_index);
	bool	read_cluster( const CLUSTER cluster, std::vector<BYTE> & data);
	bool	write_cluster( const CLUSTER cluster, const std::vector<BYTE> & data);
	bool 	cluster_to_position_size( const CLUSTER cluster, size_t & position, size_t & size);
	
public:
    FATDisk();
    ~FATDisk();

    enum {
        FORMAT_NONE,
        FORMAT_360,
        FORMAT_720
    };
    
    enum {
    	BOOTBLOCK_NONE,
    	BOOTBLOCK_MSX1
    };

    enum {
        OPEN_READONLY       = ( 1 << 0),
        OPEN_MUSTEXIST      = ( 1 << 1)
    };
    bool open( const std::string & dsk, const int mode, const int format_id = FORMAT_NONE, const int bootblock_id = BOOTBLOCK_MSX1);
    bool close( void);

	struct object_info_t {
		CLUSTER			directory;
		int				index;
		
		char			name[8+1];
		char			extension[8+1];
		CLUSTER			first_cluster;
		DWORD			size;
		struct 	
		{
			int read_only : 1;
			int hidden : 1;
			int system : 1;
			int volume_label : 1;
			int directory : 1;
			int archive : 1;
			int reserved : 2;
		} attributes;
		//xxx creation date etc
	};    
	
	bool 	set_object_size( CLUSTER & first_cluster, const CLUSTER clusters, const bool nullify = false);
	bool 	get_directory_entries( const CLUSTER directory, int & entries);
    bool 	get_directory_entry( const CLUSTER directory, const int index, object_info_t & object_info);
    bool 	set_directory_entry( const object_info_t & object_info);
    bool 	new_directory_entry( object_info_t & object_info);
    bool	del_directory_entry( object_info_t & object_info);
	bool 	read_object( const CLUSTER first_cluster, const DWORD size, std::ofstream & output);
	bool 	write_object( const CLUSTER first_cluster, const DWORD size, std::ifstream & input);    
};

#endif // _FATDISK_H_
