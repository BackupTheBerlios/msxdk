// Copyright (c) 2003-2004 Arjan Bakker
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


#ifndef CMSL_H
#define CMSL_H


#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ofstream;
using std::ios;


#include "filesnstreams.h"


#include "CMslkit.h"
#include "CMwk.h"


class CMsl
{
private:
	unsigned char mwkcnt;		//number of wavekits
	CMslkit wavekit[256];		//all wavekits
	unsigned short smpcnt;		//number of samples
	CSample samples[2048];		//all tones

public:
	CMsl();
	~CMsl();

	int init();	//initialise
	int kill();	//destroy

	int Load(char *filename);	//load specifed .MSL file
	int Save(char *filename);	//save to specified .MSL file

	char* ConvertName(char *filename); //convert filename to 11 character sized name

	int AddSample(CSample &sample);	//add a sample

	int Merge(CMsl *msl);	//merge specified .MSL with this file
	int Merge(char *filename);	//merge specified .MWK with this file

	CMwk Extract(char *filename);	//extract an .mwk file from the library

	int Remove(char *filename);	//remove .mwk file from library

	friend ofstream& operator<<(ofstream &out, const CMsl &msl);
	friend CMsl& operator>>(ifstream &in, CMsl &msl);
};




#endif
