#ifndef LcSTRINGS
	#define LcSTRINGS

	#include "lcZncWebLog.h"

	#define lcStrReplace(sString,sReplace,sWith)     sString=lcStrReplace_(sString,sReplace,sWith)
	#define lcStringAdd(sIn,fmt,...) 			 	 sIn=lcStringAdd_(sIn,fmt,##__VA_ARGS__)
	
	int lcStrStr( char *sString, char *sCheck );
	int lcStrrStr( char *sString, char *sCheck );
	char *lcStrReplace_(char *string,char *replace, char *with);
	int lcStringToUpperCase( char *sString );
	int lcStringContains( char *sString, char *sCheck );
	int lcStringCountChars( char *sString, char *sSpanset );
	int lcStringIsLetters( char *sString, int iCountSpace );
	int lcStrlen( char *sString );
	char *lcStringCreate( char * sFormat, ... );
	char *lcStringAdd_( char *sIn, char *sFormat, ... );
	void lcFree(void *dest);
#endif
