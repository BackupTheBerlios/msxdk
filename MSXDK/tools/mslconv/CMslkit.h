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


#ifndef CMSLKIT_H
#define CMSLKIT_H


#include <iostream>
#include "CPatch.h"
#include "CToneHeader.h"


class CMslkit
{
private:
	char name[12];			//name WITHOUT dot

	unsigned char tonecnt;	//number of tones used by this kit
	int tones[64];			//samples used by this kit
	CToneHeader th[64];		//headers for all samples

	unsigned char patchcnt;	//number of patches used by this kit
	CPatch patches[48];		//all the patches

public:
	CMslkit();
	~CMslkit();

	int init();
	int kill();

	void SetName(char *n);
	char *GetName() const { return (char*)name; }

	unsigned char GetPatchCnt() { return patchcnt; }

	void CorrectPatch(int patch) { patches[patch].CorrectMsl(); }

	void SetPatch(int p, const CPatch &patch);

	void SetSampleIndex(int s, int i);
	void SetToneHeader(int t, CToneHeader &th);

	friend ofstream& operator<<(ofstream &out, const CMslkit &kit);
	friend CMslkit& operator>>(ifstream &in, CMslkit &kit);
};




#endif
