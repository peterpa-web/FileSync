// CDVDtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "IsoNativeFile.h"

#pragma comment(lib, "CDVD.lib")

int _tmain(int argc, _TCHAR* argv[])
//int main(int argc, char* argv[])
{
	if ( argc < 2 ) {
		printf( "path is missing\n");
		return 1;
	}
	IsoNativeFile native( argv[1] );
	IsoDirectory root(native);	
	root.TraceFiles();
	IsoDirectory inst( native, root.files.GetAt( 8 ) );		// "install"
	inst.TraceFiles();
	return 0;
}

