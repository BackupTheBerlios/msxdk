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


#ifndef CTONE_H
#define CTONE_H


#include "CSample.h"
#include "CToneHeader.h"


class CTone : CToneHeader
{
private:
	CSample sample;
	CToneHeader toneheader;

public:
	CTone();
	~CTone();

	void SetType(unsigned char t);
	unsigned char GetType() const;
	unsigned char GetFreq() const;
	
	CSample GetSample();
	CToneHeader GetToneHeader();

	bool operator==(CTone &tone);
	bool operator!=(CTone &tone);

	CTone& operator=(const CTone &tone);

	//overload streaming operators for use in .MWK files!
	friend ofstream& operator<<(ofstream &out, const CTone &tone);
	friend CTone& operator>>(ifstream &in, CTone &tone);
	
};




#endif
