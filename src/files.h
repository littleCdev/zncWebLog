#ifndef LcFILES
	#define LcFILES

    #include "lcZncWebLog.h"
	#include <errno.h>

	char    *lcFileGetName( char *sFilepath );
	char    *lcFileGetExtension( char *sFileNameOrPath );
	_Bool  lcFileExists( char *sFilepath );
	int     lcFileSize( char *sFilepath );
	int     lcFileSteamSize( FILE *fp );
	_Bool lcFileIsDir(char *sPath,_Bool bIgnoreDot);
	_Bool  lcFileIsFile(char *sPath);
	_Bool  lcFileCopy( char *sSrc, char  *sDest );
	_Bool lcFileDirExists(char *sPath);
	char *lcFileReadLines(char *sFile,int iStart, int iAmount, int *iEndReached);
	char *lcFileToString(char *sFile, int *iSize);
#endif




