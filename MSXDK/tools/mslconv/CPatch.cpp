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

#include "CPatch.h"



CPatch::CPatch()
{
	//default patch is used in .MWK file, so it always has 8 waves!
	wavecnt = 8;
}


CPatch::~CPatch()
{


}


void CPatch::CorrectMsl()
{
	wavecnt = 1;

	while( (waves[wavecnt-1].GetEnd() < 0x060))
		wavecnt++;
}


void CPatch::CorrectMwk()
{
	wavecnt = 8;
}


ofstream& operator<<(ofstream &out, const CPatch &patch)
{

	//output transpose settings
	out.write((char*)&patch.transpose, 1);

	//output all waves
	for (int w = 0; w < patch.wavecnt; w++)
		out << patch.waves[w];

	return out;
}



CPatch& operator>>(ifstream &in, CPatch &patch)
{
	//read transpose setting
	in.read((char*)&patch.transpose, 1);

	for (int w = 0; w < patch.wavecnt; w++)
		in >> patch.waves[w];

	return patch;
}
