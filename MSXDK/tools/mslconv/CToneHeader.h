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



#ifndef CTONEHEADER_H
#define CTONEHEADER_H


#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::ios;

class CToneHeader
{
private:
	unsigned char type;		//type/frequency

	unsigned short address;	//only used for copied rom tone
	unsigned short loop;
	unsigned short end;

	unsigned char lfo;	//lfo/vib
	unsigned char ar;	//ar/d13
	unsigned char dl;	//dl/d2r
	unsigned char rr;	//rr
	unsigned char am;	//am

	unsigned short size;	//only used for new sample

	unsigned char msb;
public:

	CToneHeader();
	~CToneHeader();

	bool operator==(CToneHeader &th);
	bool operator!=(CToneHeader &th);

	void SetType(unsigned char type);
	unsigned char GetType() const;

	unsigned short GetSize();

	CToneHeader& operator=(const CToneHeader &th);

	//Read/Write operations for use in .MWK files
	int Read(ifstream &in);
	int Write(ofstream &out) const ;

	//overload streaming operators for use in .MSL file
	friend ofstream& operator<<(ofstream &out, const CToneHeader &th);
	friend CToneHeader& operator>>(ifstream &in, CToneHeader &th);
};



#endif
