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

               

#include "CSample.h"


#include <string>



CSample::CSample()
{
	length = 0;
	data = NULL;
}


CSample::~CSample()
{
//	if (data)
//		delete data;
}


void CSample::init(unsigned short l, char *d)
{
	length = l;

	if (data)
		delete data;

	data = new char[length];

	memcpy(data, d, length);
}


char* CSample::GetData()
{
	return data;
}


unsigned short CSample::GetLength()
{
	return length;
}


bool CSample::operator==(CSample &sample)
{
	if (sample.length != length)
		return false;

	if (data)
	{
		if (memcmp(sample.data, data, length))
			return false;
	}

	return true;
}


bool CSample::operator!=(CSample &sample)
{
	return !(operator==(sample));
}


CSample& CSample::operator=(const CSample &sample)
{
	length = sample.length;

	if (data)
		delete data;

	if (sample.data)
	{
		data = new char[length];

		memcpy(data, sample.data, length);
	}
	else
		data = NULL;

	return *this;
}



int CSample::Write(ofstream &out) const 
{
	out.write(data, length);

	return 0;
}


ofstream& operator<<(ofstream &out, const CSample &sample)
{
	if (sample.length > 0)
	{
		//output sample length
		out.write((char*)&sample.length, 2);

		//output sample data
		out.write(sample.data, sample.length);
	}

	return out;
}


CSample& operator>>(ifstream &in, CSample &sample)
{
	//read sample length
	in.read((char*)&sample.length, 2);

	//read sample data
	in.read(sample.data, sample.length);

	return sample;
}
