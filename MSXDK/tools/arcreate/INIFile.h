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

#ifndef _INIFILE_H_
#define _INIFILE_H_

#include <vector>
#include <string>

class INIFile {
public:
struct key_value_t {
	std::string		key;
	std::string		value;
};
struct section_t {
	std::string		name;
	std::vector<key_value_t>	entries;
};
private:
	std::vector<section_t>	m_sections;	
public:
	INIFile() {};
	~INIFile() {};
	
	bool load( const char * filename);	
	const std::vector<section_t> & get_sections( void);
	
	const std::string & get_value( const char * section, const char * key);
};

#endif // _INIFILE_H_
