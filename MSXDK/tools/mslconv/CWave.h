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

//Wave class - Definitions



#ifndef CWAVE_H
#define CWAVE_H


#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;

class CWave 
{
private:
	unsigned char end;	//starting tone of next wave
	unsigned char tone;	//tone used by this wave
	unsigned char base;	//base note of this wave

public:

	CWave();
	CWave(unsigned char e, unsigned char t, unsigned char b);
	~CWave();

	//set individual members
	void SetEnd(unsigned char e);
	void SetTone(unsigned char t);
	void SetBase(unsigned char b);

	//get members
	unsigned char GetEnd();
	unsigned char GetTone();
	unsigned char GetBase();

	CWave &operator=(const CWave &wave);
	friend ofstream& operator<<(ofstream &out, const CWave &wave);
	friend CWave& operator>>(ifstream &in, CWave &wave);
};


#endif

