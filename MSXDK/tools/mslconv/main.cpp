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


//mls converter
//puts .mwk files in one mls file


#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
#include <string>


#include "CMsl.h"
#include "CMwk.h"


//error messages
char error_arguments[6][64]={	"",
				"Invalid arguments, run program with -? as argument for help",
				"Please supply output filename",
				"Please supply .mwk filename"};

char outname[256];		//filename for .mls to be created
char appendname[16][256];	//.mwk files to add

unsigned char appendcount;	//number of .mwk files to append
const unsigned char version = 0;	//version number of .mls file


//Display usage message
void ShowHelp()
{
	cout << "Usage:" << endl;
	cout << "-oOUTPUT.MSL" << endl;
	cout << "  specifies filename of sample library that has to be created." << endl;
	cout << "  this field MUST be set!" << endl << endl;
	cout << "-aFILENAME.MWK" << endl;
	cout << "  specifies .mwk file to add to the sample library." << endl;
	cout << "  name will be converted to uppercase!" << endl;
	cout << "  this is optional, but an empty sample library is pretty much useless..." << endl << endl;
	cout << "-?" << endl;
	cout << "  shows this helptext." << endl;
}


//Parse all arguments supplied
//returns 0 if succesful
int ParseArguments(int argc, char *argv[])
{
	appendcount = 0;

	//iterate through all supplied arguments
	for (int c = 1; c < argc; c++)
	{
		//arguments always start with -
		if (argv[c][0] != '-')
			return 1;	//error!

		//detect argument
		switch(argv[c][1])
		{
		case '?':	//show help
			ShowHelp();
			break;
		case 'o':	//specify outputfile
		case 'O':
			strcpy(outname, &argv[c][2]);

			//name should be filled in
			if (strlen(outname) == 0)
				return 2;
			break;
		case 'a':	//specify file to add
		case 'A':
			//filename should be filled in
			if (strlen((const char*)&argv[c][2]))
				strcpy(appendname[appendcount++], &argv[c][2]);
			else
				return 3;
			break;
		default:
			return 1;
		}
	}

	if ( (strlen(outname) == 0) )
		return 2;


	return	0;
}



int main(int argc, char *argv[])
{
int error;
CMsl msl;

	msl.init();

	//if no arguments supplied
	if (argc == 1)
	{	//show help text
		ShowHelp();

		return 0;
	}

	//parse all arguments
	error = ParseArguments(argc, argv);

	//if an error occured
	if (error)
	{	//display error message
		cout << error_arguments[error] << endl;

		return 0;
	}

	//add all wavekits to the sample library
	for (int c = 0; c < appendcount; c++)
	{
		_strupr(appendname[c]);
		msl.Merge(appendname[c]);
	}

	//convert output filename to upper-case
	_strupr(outname);

	//save sample library
	msl.Save(outname);

	//done!
	return 0;
}
