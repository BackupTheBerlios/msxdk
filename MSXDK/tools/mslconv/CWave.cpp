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


//Wave class - Implementation



#include "CWave.h"



//default constructor
CWave::CWave()
{


}


//constrcutor
CWave::CWave(unsigned char e, unsigned char t, unsigned char b)
{
	end = e;
	tone = t;
	base = b;
}


//destructor
CWave::~CWave()
{



}



void CWave::SetEnd(unsigned char e)
{
	end = e;
}



void CWave::SetTone(unsigned char t)
{
	tone = t;
}



void CWave::SetBase(unsigned char b)
{
	base = b;
}



unsigned char CWave::GetEnd()
{
	return end;
}



unsigned char CWave::GetTone()
{
	return tone;
}



unsigned char CWave::GetBase()
{
	return base;
}


CWave& CWave::operator=(const CWave &wave)
{
	end = wave.end;
	tone = wave.tone;
	base = wave.base;

	return *this;
}

//overload <<
ofstream& operator<<(ofstream &out, const CWave &wave)
{

	out.write((char*)&wave.end, 1);
	out.write((char*)&wave.tone, 1);
	out.write((char*)&wave.base, 1);

	return out;
}


//overload >>
CWave& operator>>(ifstream &in, CWave &wave)
{
	in.read((char*)&wave.end, 1);
	in.read((char*)&wave.tone, 1);
	in.read((char*)&wave.base, 1);

	return wave;
}
