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

#include <string>

#include "CMslkit.h"


CMslkit::CMslkit()
{

}



CMslkit::~CMslkit()
{


}


int CMslkit::init()
{
	for (int c = 0; c < 64; c++)
		tones[c] = 0;
	
	patchcnt = 0;
	tonecnt = 0;

	return 0;
}


int CMslkit::kill()
{

	return 0;
}


void CMslkit::SetName(char *n)
{
	strcpy(name, n);
}




void CMslkit::SetPatch(int p, const CPatch &patch)
{
	patches[p] = patch;

	if (patchcnt <= p)
		patchcnt = p+1;
}


void CMslkit::SetSampleIndex(int t, int i)
{
	tones[t] = i;

	if (tonecnt <= t)
		tonecnt = t+1;
}


void CMslkit::SetToneHeader(int t, CToneHeader &toneheader)
{
	th[t] = toneheader;
}


ofstream& operator<<(ofstream &out, const CMslkit &kit)
{
unsigned char wavecnt;

	out.write((char*)&kit.tonecnt, 1);

	for (int t = 0; t < kit.tonecnt; t++)
	{
		out.write((char*)&kit.tones[t], 2);

		out << kit.th[t];
	}

	out.write((char*)&kit.patchcnt, 1);

	for (int p = 0; p < kit.patchcnt; p++)
	{
		wavecnt = kit.patches[p].GetWaveCount();

		out.write((char*)&wavecnt, 1);

		out << kit.patches[p];
	}

	return out;
}


CMslkit& operator>>(ifstream &in, CMslkit &kit)
{


	return kit;
}
