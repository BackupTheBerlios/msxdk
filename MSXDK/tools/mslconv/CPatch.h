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


#ifndef CPATCH_H
#define CPATCH_H


#include <iostream>


#include "CWave.h"


class CPatch
{
private:
	unsigned char transpose;	//transpose on/off
	int wavecnt;				//number of waves
	CWave waves[8];				//the waves

public:

	CPatch();
	~CPatch();	

	void AddWave(CWave &wave);	//add wave to list

	int GetWaveCount() const { return wavecnt; }	//get the number of waves

	void CorrectMsl();		//correct wavecnt for use with MSL files
	void CorrectMwk();		//correct wavecnt for use with MWK files

	CWave& GetWave(int w);		//get specified wave

	

	friend ofstream& operator<<(ofstream &out, const CPatch &patch);
	friend CPatch& operator>>(ifstream &in, CPatch &patch);
};






#endif
