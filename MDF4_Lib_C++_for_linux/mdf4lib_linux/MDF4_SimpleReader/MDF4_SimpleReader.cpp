// MDF4_SimpleReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ImportMdf4.h"
#include "mdf4.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[])
{
	CMdf4FileImport *pImport = new CMdf4FileImport;
#ifdef WIN32
	if (pImport->ImportFile("D:\\TestData\\DataSpySampleDataFile.mf4"))
#else
	if (pImport->ImportFile("../Test.mf4"))
#endif
	{
	}
	getchar();
	return 0;
}

