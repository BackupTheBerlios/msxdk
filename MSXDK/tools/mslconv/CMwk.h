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


#ifndef CMWK_H
#define CMWK_H

#include <iostream> 
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::ios;

#include <string>

#include "CPatch.h"
#include "CTone.h"


class CMwk
{
private:
	unsigned char tonecnt;	//number of tones in .MWK file
	unsigned char patchcnt;	//number of patches in .MWK file
	bool fail;			//true if loading failed

	CPatch patches[48];	//all the instruments
	CTone tones[64];	//all the tones

public:
	CMwk();	
	~CMwk();	

	int init();		//initialise
	int kill();		//destroy

	int Load(char *filename);	//load specified filename
	int Save(char *filename);	//save to mwk file

	bool Failed();		//check if loading failed

	int GetPatchCnt();
	int GetToneCnt();
	
	CPatch GetPatch(int p);
	CTone GetTone(int t);

	friend ofstream& operator<<(ofstream &out, const CMwk &mwk);
	friend CMwk& operator>>(ifstream &in, CMwk &mwk);

};








#endif
