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


#include "CMsl.h"
#include "CMwk.h"



CMsl::CMsl()
{

}



CMsl::~CMsl()
{

}

//initialise
int CMsl::init()
{
	mwkcnt = 0;
	smpcnt = 0;

	return 0;
}


//destroy
int CMsl::kill()
{
	
	return 0;
}


//load specifed .MSL file
int CMsl::Load(char *filename)
{

	return 0;
}


//save to specified .MSL file
int CMsl::Save(char *filename)
{
ofstream file;

	file.open(filename, ios::out | ios::binary);

	for (int k = 0; k < mwkcnt; k++)
		for (int p = 0; p < wavekit[k].GetPatchCnt(); p++)
			wavekit[k].CorrectPatch(p);

	file << *this;

	file.close();

	return 0;
}


//merge specified .MSL with this file
int CMsl::Merge(CMsl *msl)
{

	return 0;
}


//add a sample to the wavekit
//returns the index of the sample in the wavekit
int CMsl::AddSample(CSample &sample)
{
	//loop through all samples
	for (int s = 0; s < smpcnt; s++)
	{
		if (samples[s] == sample)
			return s;
	}

	//copy sample
	samples[smpcnt] = sample;

	//increase number of samples in wavekit
	smpcnt++;

	return (smpcnt-1);	//return index
}


//convert filename to 8.3 format
//warning: when calling multiple times, the previous
//			result will be overwritten
char* CMsl::ConvertName(char *fname)
{
char filename[256];
static char realname[12];
char *dot;

	//make a copy of the filename
	strcpy(filename, fname);

	//fill name with whitespace
	for (int i = 0; i < 11; i++)
		realname[i] = ' ';

	//find dot in filename
	dot = strchr(filename, '.');

	//if dot found
	if (dot)
	{
		//zero terminate name
		*dot = 0;

		//name can't be more than eight characters
		if (strlen(filename) > 8)
		{
			//put dot back
			*dot = '.';

			return NULL;
		}

		//copy name
		strcpy(realname, filename);

		//put dot back
		*dot = '.';

		//move to start of extension
		dot++;

		//extension can't be more than 8 characters
		if (strlen(dot) > 3)
			return NULL;

		//copy extension
		strcpy(&realname[8], dot);
	}
	else	//no dot found
	{	//max length of string is 8 characters
		if (strlen(filename) > 8)
			return NULL;	//invalid filename

		//copy filename
		strcpy(realname, filename);
	}

	for (int i = 0; i < 11; i++)
		if ( (realname[i] < 32) || (realname[i]>'~'))
			realname[i] = ' ';

	return realname;
}



//merge specified .MWK with this file
int CMsl::Merge(char *filename)
{
char *realname;
CMwk mwk;
CTone tone;
CSample sample;
CToneHeader th;

	//convert filename to 8.3 sized name
	realname = ConvertName(filename);

	if (realname == NULL)
		return 1;		//incorrect filename (not 8.3 format)

	//load wavekit
	if (mwk.Load(filename))
		return 2;		//failed loading file

	if (mwk.Failed() == true)
		return 2;		//failed loading file

	wavekit[mwkcnt].init();

	wavekit[mwkcnt].SetName(realname);

	//iterate through all tones
	for (int t = 0; t < mwk.GetToneCnt(); t++)
	{	//get tone
		tone = mwk.GetTone(t);

		if (tone.GetType() && tone.GetFreq() )
		{
			//get sample
			sample = tone.GetSample();

			//get header
			th = tone.GetToneHeader();

			//add sample to collection and fill in sample index
			wavekit[mwkcnt].SetSampleIndex(t, AddSample(sample));

			//copy toneheader to wavekit
			wavekit[mwkcnt].SetToneHeader(t, th);
		}
		else
		{
			CToneHeader th;

			wavekit[mwkcnt].SetSampleIndex(t, 0);
			wavekit[mwkcnt].SetToneHeader(t, th);
		}
	}

	//iterate through all patches
	for (int p = 0; p < mwk.GetPatchCnt(); p++)
	{	//add patch to wavekit
		wavekit[mwkcnt].SetPatch(p, mwk.GetPatch(p));
	}

	mwkcnt++;

	return 0;
}


//remove .mwk file from library
int CMsl::Remove(char *filename)
{
char *realname = ConvertName(filename);	//convert filename to 8.3 format name

	//check if file exists
	for (int k = 0; k < mwkcnt; k++)
	{	//check names
		if (strcmp(realname, wavekit[k].GetName()) == 0)
		{	//remove file

			return 0;
		}
	}

	//file not found!
	return 1;
}


CMwk Extract(char *filename)
{
CMwk mwk;


	return mwk;
}


ofstream& operator<<(ofstream &out, const CMsl &msl)
{
const char version = 0;
const int zero = 0;
int mwkpnt;
int mwkpos[256];
int smppnt;
int smppos[2048];

	//output header
	out.write("MSLF", 4);

	//output version number
	out.write(&version, 1);

	//output number of samplekits used
	out.write( (char*)&msl.mwkcnt, 1);

	//output all filenames
	for (int k = 0; k < msl.mwkcnt; k++)
		out.write(msl.wavekit[k].GetName(), 11);

	//get position of first pointer to mwk data
	mwkpnt = out.tellp();

	//output zero pointer for each wavekit
	for (int k = 0; k < msl.mwkcnt; k++)
		out.write((char*)&zero, 4);

	//output number of samples used
	write_littleendian( out, 2, msl.smpcnt );

	//get position of first pointer to sample data
	smppnt = out.tellp();

	//output zero pointer for each sample
	for (int s = 0; s < msl.smpcnt; s++)
		out.write((char*)&zero, 4);

	//output all wavekits
	for (int k = 0; k < msl.mwkcnt; k++)
	{	//get current position in file
		mwkpos[k] = out.tellp();

		out << msl.wavekit[k];
	}

	//output all samples
	for (int s = 0; s < msl.smpcnt; s++)
	{	//get current position in file
		smppos[s] = out.tellp();

		out << msl.samples[s];
	}

	//move to position of first mwk pointer
	out.seekp(mwkpnt, ios::beg);

	//output all mwk pointers
	for (int k = 0; k < msl.mwkcnt; k++)
		write_littleendian( out, 4, (long)mwkpos[ k ] );

	//move to position of first sample pointer
	out.seekp(smppnt, ios::beg);

	//output all sample pointers
	for (int s = 0; s < msl.smpcnt; s++)
		write_littleendian( out, 4, (long)smppos[ s ] );

	//move to end of file
	out.seekp(0, ios::end);

	return out;
}

// not supported yet
CMsl& operator>>(ifstream &in, CMsl &msl)
{

	return msl;
}

