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


#include "CTone.h"



CTone::CTone()
{

}


CTone::~CTone()
{

}


void CTone::SetType(unsigned char t)
{
	toneheader.SetType(t);
}


unsigned char CTone::GetType() const
{
	return toneheader.GetType();
}


unsigned char CTone::GetFreq() const
{
unsigned char type = toneheader.GetType() & 63;

	switch (type)
	{
	case 1:
	case 3:
	case 5:
		return 5;
	default:
		return 0;
	}

	return 0;
}


CSample CTone::GetSample()
{
	return sample;
}


CToneHeader CTone::GetToneHeader()
{
	return toneheader;
}


bool CTone::operator==(CTone &tone)
{
	if (tone.sample != sample)
		return false;

	if (tone.toneheader != toneheader)
		return false;

	return true;
}


bool CTone::operator!=(CTone &tone)
{
	return !(operator==(tone));
}


CTone& CTone::operator=(const CTone &tone)
{
	sample = tone.sample;

	toneheader = tone.toneheader;

	return *this;
}


ofstream& operator<<(ofstream &out, const CTone &tone)
{

	tone.toneheader.Write(out);

	//write sample if tone is not a rom-tone
	if (!(tone.GetType() & 32) )
		tone.sample.Write(out);

	return out;
}


CTone& operator>>(ifstream &in, CTone &tone)
{
char *data;

	tone.toneheader.Read(in);

	//read sample if tone is not a rom-tone
	if (!(tone.GetType() & 32) )
	{
		data = new char[tone.toneheader.GetSize()];

		in.read(data, tone.toneheader.GetSize());

		tone.sample.init(tone.toneheader.GetSize(), data);

		delete data;
	}

	return tone;
}
