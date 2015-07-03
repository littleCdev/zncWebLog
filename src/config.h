#ifndef LcCONFIG
	#define LcCONFIG
	
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	
	typedef struct{
		char *ini;
		int size;
	}lcConfig;


	int _lcCfgStrStr( char *sString, char *sCheck );
	lcConfig *lcConfigLoad(char *sFile);
	void lcConfigFree(lcConfig *cfg);
	char *lcConfigGetString(lcConfig *cfg,char *sName);

#endif
