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




#include "CMwk.h"



CMwk::CMwk()
{

}



CMwk::~CMwk()
{

}


int CMwk::init()
{
	fail = false;
	patchcnt = 0;
	tonecnt = 0;

	return 0;
}


int CMwk::kill()
{

	return 0;
}


bool CMwk::Failed()
{
	return fail;
}


int CMwk::Load(char *filename)
{
ifstream file;

	//open file
	file.open(filename, ios::in | ios::binary);

	if (file.bad())
		return 1;	//no access to file

	//stream file to wavekit
	file >> *this;

	if (fail)
		return 2;	//error loading file

	file.close();

	return 0;
}


int CMwk::Save(char *filename)
{
ofstream file;

	//create file
	file.open(filename, ios::out | ios::binary);

	if (file.bad())
		return 1;	//file creation failed

	//stream wavekit to file
	file << *this;

	file.close();

	return 0;
}


int CMwk::GetPatchCnt()
{
	return patchcnt;
}

CPatch CMwk::GetPatch(int p)
{
	return patches[p];
}

int CMwk::GetToneCnt()
{
	return tonecnt;
}

CTone CMwk::GetTone(int t)
{
	return tones[t];
}


//overload operator <<
ofstream& operator<<(ofstream &out, const CMwk &mwk)
{
const unsigned char ver = 0x010;
const unsigned char format = 0x0d;
const unsigned char empty = 0;
char type;

	//write header
	out << "MBMS";

	//write version number
	out.write((char*)&ver, 1);

	//write format type
	out.write((char*)&format, 1);

	//write three empty bytes
	out.write((char*)&empty, 1);
	out.write((char*)&empty, 1);
	out.write((char*)&empty, 1);

	//write number of patches
	out.write((char*)&mwk.patchcnt, 1);

	//write tone information
	for (int t = 0; t < 64; t++)
	{
		type = mwk.tones[t].GetType();
		out.write((char*)&type, 1);
	}

	//output all patches
	for (int p = 0; p < mwk.patchcnt; p++)
	{
		out << mwk.patches[p];
	}

	//output all tones
	for (int t = 0; t < mwk.tonecnt; t++)
	{
		//only output tone if it exists
		if (mwk.tones[t].GetType() && mwk.tones[t].GetFreq() )
			out << mwk.tones[t];
	}

	return out;
}


//overload operator >>
CMwk& operator>>(ifstream &in, CMwk &mwk)
{
char buffer[5];		//space for putting header in
unsigned char ver;	//file format version
unsigned char format;	//file type (0x0d = USER, 0x0c = edit)

	//no errors detected yet
	mwk.fail = false;

	//read header
	in.read(buffer, 4);

	//zero terminate
	buffer[4] = 0;

	//header should be MBMS
	if (strcmp(buffer, "MBMS"))	//header not correct
	{
		mwk.fail = true;

		return mwk;		//there's an error!
	}

	//read file format version
	in.read((char*)&ver, 1);

	//read file type (USER or EDIT)
	in.read((char*)&format, 1);

	//read 3 empty bytes plus number of patches
	in.read(buffer, 4);

	//read number of patches
	mwk.patchcnt = buffer[3];

	mwk.tonecnt = 0;

	//read all tone info
	for (int c = 0; c < 64; c++)
	{
		unsigned char type;

		//read tone info
		in.read((char*)&type, 1);

		//set type
		mwk.tones[c].SetType(type);

		//if tone used
		if (type && mwk.tones[c].GetFreq() )	//set number of highest tone used
			mwk.tonecnt = c + 1;
	}

	//read all patches
	for (int p = 0; p < mwk.patchcnt; p++)
		in >> mwk.patches[p];

	//if kit is EDIT format
	if (format == 0x0c)	//skip 16*patchcnt bytes (skip all patch names...)
		in.seekg(mwk.patchcnt * 16, ios::cur);

	//read all tones
	for (int t = 0; t < mwk.tonecnt; t++)
	{	//if tone is present
		if ( mwk.tones[t].GetType() && mwk.tones[t].GetFreq() )
			in >> mwk.tones[t];	//read tone from input stream
	}

	return mwk;
}
