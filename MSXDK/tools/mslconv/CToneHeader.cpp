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

#include "CToneHeader.h"




CToneHeader::CToneHeader()
{
	type = 0;	

	address = 0;
	loop = 0;
	end = 0;

	lfo = 0;
	ar = 0;	
	dl = 0;	
	rr = 0;	
	am = 0;	

	size = 0;

	msb = 0;
}


CToneHeader::~CToneHeader()
{

}


void CToneHeader::SetType(unsigned char t)
{
	type = t;
}


unsigned char CToneHeader::GetType() const
{
	return type;
}


unsigned short CToneHeader::GetSize()
{
	return size;
}


bool CToneHeader::operator==(CToneHeader &th)
{
	if (th.type != type)
		return false;

	if (th.type & 32)
	{	//rom tone
		if (th.address != address)
			return false;

		if (th.msb != msb)
			return false;
	}
	else
	{	//new sample
		if (th.size != size)
			return false;
	}

	if (th.am != am)
		return false;

	if (th.ar != ar)
		return false;

	if (th.dl != dl)
		return false;

	if (th.end != end)
		return false;

	if (th.lfo != lfo)
		return false;

	if (th.loop != loop)
		return false;

	if (th.rr != rr)
		return false;

	return true;
}


bool CToneHeader::operator!=(CToneHeader &th)
{
	return !(operator==(th));
}


CToneHeader& CToneHeader::operator=(const CToneHeader &th)
{
	type = th.type;

	if (th.type & 32)
	{
		address = th.address;
		msb = th.msb;
	}
	else
		size = th.size;

	am = th.am;
	ar = th.ar;
	dl = th.dl;
	end = th.end;
	lfo = th.lfo;
	loop = th.loop;
	rr = th.rr;
	
	return *this;
}



int CToneHeader::Read(ifstream &in)
{
	//read LSB tone address, just store it here even if we're not dealing with rom-tones
	in.read((char*)&address, 2);

	if ( !(type & 32) )
		address = 0;

	//read loop point
	in.read((char*)&loop, 2);

	//read end point
	in.read((char*)&end, 2);

	//read lfo/vib settings
	in.read((char*)&lfo, 1);

	//read ar/d1r settings
	in.read((char*)&ar, 1);

	//read dl/d2r settings
	in.read((char*)&dl, 1);

	//read rr
	in.read((char*)&rr, 1);

	//read am
	in.read((char*)&am, 1);

	//check tone type
	if (type & 32)
	{	//rom tone
		in.seekg(1, ios::cur);

		in.read((char*)&msb, 1);

		size = 0;
	}
	else
	{	//new sample
		//read size
		in.read((char*)&size, 2);
	}

	return 0;
}


int CToneHeader::Write(ofstream &out) const 
{
	//output LSB starting address (rom tone) OR zero's (new sample)
	out.write((char*)&address, 2);

	out.write((char*)&loop, 2);

	out.write((char*)&end, 2);

	out.write((char*)&lfo, 1);

	out.write((char*)&ar, 1);

	out.write((char*)&dl, 1);

	out.write((char*)&rr, 1);

	out.write((char*)&am, 1);

	if (type & 32)
	{
		//output 0
		out.write((char*)&size, 1);

		out.write((char*)&msb, 1);
	}
	else
		out.write((char*)&size, 2);

	return 0;
}


ofstream& operator<<(ofstream &out, const CToneHeader &th)
{
	//output type
	out.write((char*)&th.type, 1);

	//check tone type
	if (th.type & 32)
	{	//rom tone
		//output MSB tone-address
		out.write((char*)&th.msb, 1);

		//output LSB tone-address 
		out.write((char*)&th.address, 2);
	}
	else
	{	//own sample
		//output filler byte
		out.write((char*)&th.size, 1);

		//output sample size
		out.write((char*)&th.size, 2);
	}

	//output loop point
	out.write((char*)&th.loop, 2);

	//output end point
	out.write((char*)&th.end, 2);

	//output lfo/vib settings
	out.write((char*)&th.lfo, 1);

	//output ar/d1r settings
	out.write((char*)&th.ar, 1);

	//output dl/d2r settings
	out.write((char*)&th.dl, 1);

	//output rr
	out.write((char*)&th.rr, 1);

	//output am
	out.write((char*)&th.am, 1);

	return out;
}


CToneHeader& operator>>(ifstream &in, CToneHeader &th)
{
	//read type
	in.read((char*)&th.type, 1);

	//check tone type
	if (th.type & 32)
	{	//rom tone
		//read MSB tone address
		in.read((char*)&th.msb, 1);

		//read LSB tone address
		in.read((char*)&th.address, 2);
	}
	else
	{	//new sample
		//read filler byte
		in.read((char*)&th.size, 1);

		//read size
		in.read((char*)&th.size, 2);
	}

	//read loop point
	in.read((char*)&th.loop, 2);

	//read end point
	in.read((char*)&th.end, 2);

	//read lfo/vib settings
	in.read((char*)&th.lfo, 1);

	//read ar/d1r settings
	in.read((char*)&th.ar, 1);

	//read dl/d2r settings
	in.read((char*)&th.dl, 1);

	//read rr
	in.read((char*)&th.rr, 1);

	//read am
	in.read((char*)&th.am, 1);

	return th;
}
