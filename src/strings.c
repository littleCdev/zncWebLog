#include "strings.h"

int lcStrStr( char *sString, char *sCheck ){
    int i,
    d,
    iRet = -1,
    iStrinLen = (int)strlen( sString ),
    iCheckLen = (int)strlen( sCheck );
    
    for( i = 0; i < iStrinLen; i++ ){
        iRet = i;
        
        if(i+iCheckLen > iStrinLen)
            return -1;
        
        for( d = 0; d < iCheckLen; d++ ){
            if( sString[i+d] != sCheck[d] ){
                iRet = -1;
                break;
            }
        }
        
        if(iRet != -1)
            return iRet;
    }
    
    return iRet;
}

int lcStrrStr( char *sString, char *sCheck ){
    int i,
    d,
    iRet = -1,
    iStrinLen = (int)strlen( sString ),
    iCheckLen = (int)strlen( sCheck );
    
    for( i = iStrinLen; i>0; i-- ){
        iRet = i;
        
        if(i+iCheckLen > iStrinLen)
            continue;
        
        for( d = 0; d < iCheckLen; d++ ){
            if( sString[i+d] != sCheck[d] ){
                iRet = -1;
                break;
            }
        }
        
        if(iRet != -1)
            return iRet;
    }
    
    return iRet;
}

char *lcStrReplace_(char *string,char *replace, char *with){
    int iNext 		= -1;
    char *pos 		= string;
    int iFindLen 	= (int)strlen(replace);
    int iReplaceLen = (int)strlen(with);
    char *sRet		= NULL;
    int iCnt 		= 0;
    
    while((iNext=lcStrStr(pos,replace))!=-1){
        iCnt++;
        pos +=iNext+iFindLen;
    }
    
    int iTmp = (int)strlen(string)-iFindLen*iCnt + iReplaceLen*iCnt +2;
    sRet = malloc(iTmp);
    memset(sRet, '\0', iTmp);
    
    
    pos = string;
    while((iNext=lcStrStr(pos,replace))!=-1){
        strncat(sRet,pos,iNext);
        strncat(sRet,with,iReplaceLen);
        pos +=iNext+iFindLen;
    }
    // and add the last rest if there is nothing to find more
    strcat(sRet,pos);
    
    string = realloc(string,strlen(sRet)+2);
    memset(string, '\0', strlen(sRet)+2);
    strncat(string,sRet,strlen(sRet));
    
    lcFree(sRet);

    return string;
}
int lcStringToLowercase( char *sString ){
    int i;
    int iChangedChars = 0;
    int iLen = lcStrlen( sString	);
    
    for( i = 0; i < iLen; i++ ){
        if( sString[i] >= 'A' && sString[i] <= 'Z' ){
            sString[i]+=32;
            iChangedChars++;
        }
    }
    
    return iChangedChars;
}

int lcStringToUpperCase( char *sString ){
    int i;
    int iChangedChars = 0;
    int iLen = lcStrlen( sString );
    
    for( i = 0; i < iLen; i++){
        if( sString[i] >= 'a' && sString[i] <= 'z' ){
            sString[i] -= 32;
            iChangedChars++;
        }
    }
    
    return iChangedChars;
}

int lcStringCountChars( char *sString, char *sSpanset ){
    int i,
    d,
    iRet	  = 0,
    iStrinLen = lcStrlen( sString ),
    iCheckLen = lcStrlen( sSpanset );
    
    for( i = 0; i < iStrinLen; i++ ){
        for( d = 0; d < iCheckLen; d++ ){
            if( sString[i] == sSpanset[d] )
                iRet++;
        }
    }
    
    return iRet;
}

int lcStringIsLetters( char *sString, int iCountSpace ){
    
    int i,
    iLen = lcStrlen( sString );
    
    for( i = 0; i < iLen; i++){
        if(
           !(
             ( // check for alphabet
              // upper letters
              sString[i] >= 'A' &&
              sString[i] <= 'Z' &&
              // lower letters
              sString[i] >= 'a' &&
              sString[i] <= 'z'
              )||  // or space if wished
             (
              iCountSpace == 1 &&
              sString[i] == ' '
              
              )
             )
           )
            return 0;
    }
    
    return 0;
}

int lcStrlen( char *sString ){
    if( sString == NULL ){
        return 0;
    }
    
    return (int)strlen( sString );
}

int lcStringFormatGetType( char *sParse, int z ){
    int iLen	= lcStrlen( sParse );
    int i = 0;
    int cnt = 0;
    
    for( i = 0; i < iLen; i++ ){
        if( sParse[i] == '%' && sParse[i-1] != '\\' ){
            if(
               sParse[i+1] == 's' ||
               sParse[i+1] == 'c' ||
               sParse[i+1] == 'i' ||
               sParse[i+1] == 'f'
               ){
                cnt++;
                if( cnt == z )
                {
                    switch( sParse[i+1] )
                    {
                        case 'c':	return 1;	break;
                        case 's':	return 2;	break;
                        case 'f':	return 3;	break;
                        case 'i':	return 4;	break;
                    }
                }
            }
        }
    }
    
    return -1;
}

char *lcStringCreate( char * sFormat, ... ){
    va_list arg;
    char *pRet;
    
    if(lcStrlen(sFormat) == 0){
		pRet = malloc(1);
		pRet[0] = '\0';
		
		return pRet;
	}
    
    va_start( arg, sFormat );
    if(!vasprintf(&pRet,sFormat,arg)){
		perror("vasprintf");
		return NULL;
	}
    va_end(arg);
    
    return pRet;
}


char *lcStringAdd_( char *sIn, char *sFormat, ... ){

    va_list arg;
    
    char *sNewString;
    char *buffer;
    int iFormatLen;

    va_start( arg, sFormat );
	iFormatLen = vasprintf(&sNewString,sFormat,arg);
	va_end( arg );
    
    buffer = malloc(iFormatLen+lcStrlen(sIn)+1);
   
	sprintf(buffer,"%s%s",sIn,sNewString);
	sIn 	= realloc(sIn,lcStrlen(buffer)+1);
    sIn[0] 	= '\0';
    
    strcpy(sIn,buffer);
   
    lcFree( buffer );
    lcFree( sNewString );
    
    return sIn;
}

void lcFree(void *dest){
    if(dest != NULL)
		free(dest);	
	else
		printf("pointer was NULL\n");
	dest = NULL;
}
