#include "config.h"

int _lcCfgStrStr( char *sString, char *sCheck ){
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
/*
 * every newline is a new value
 * # is a comment
 * ; will end a value 
 * name=aa;ss -> "aa" only
 * name=test#comment is "test" only
 * 
*/ 

lcConfig *lcConfigLoad(char *sFile){
	
	FILE *fFile = fopen(sFile,"r");
	lcConfig *ret = malloc(sizeof(lcConfig));
	
	
	if(!fFile){
		ret->size = -1;
		return ret;
	}
	
	fseek( fFile, 0L, SEEK_END );
	ret->size = (int)ftell( fFile );
	
	fseek( fFile, 0L, SEEK_SET );
	
	char *sTmp = malloc(ret->size+1);
	
	ret->ini = malloc(ret->size+1);
	memset(ret->ini,'\0',ret->size);
		
	fread(sTmp,1,ret->size,fFile);
	fclose(fFile);
	
	int i = 0;
	int iPos = 0;
	int iComment = 0;
	
	for(i=0;i<ret->size;i++){
		if(iComment == 1){
			if(sTmp[i] == '\n')
				iComment = 0;
				
			continue;
		}
		
		if(sTmp[i] == '#' 
			&& ((i>0 && sTmp[i-1] != '\\') || i==0)){
			iComment = 1;
			continue;
		}
		
		if((sTmp[i] != '\n' && sTmp[i] != '\r' && sTmp[i] != 0x0A)
			&& (sTmp[i] != ';' && i>0 && sTmp[i-1] != '\\'))
			ret->ini[iPos] = sTmp[i];
		else
			ret->ini[iPos] = '\0';
			
		iPos++;
		
	}
	
	free(sTmp);
	return ret;
}

void lcConfigFree(lcConfig *cfg){
	free(cfg->ini);
	free(cfg);	
}

char *lcConfigGetString(lcConfig *cfg,char *sName){
	int iPos = -1;
	int iSearchPos = 0;
	
	do{
		iPos = _lcCfgStrStr(cfg->ini+iSearchPos,sName);
		
		if(iPos > -1 && (iPos > 0 && cfg->ini[iPos+iSearchPos-1] != '\n' && cfg->ini[iPos+iSearchPos-1] != '\r' && cfg->ini[iPos+iSearchPos-1] != 0x0A)){
			iSearchPos += iPos; // skipt the part where the string was found but is invalid
			iPos = -1;
		}else if(iPos > -1){
			break;
		}
		
		iSearchPos++;
	}while(iSearchPos < cfg->size && iPos == -1 );
	
	if(iPos == -1)
		return "\0";
	
	iPos += iSearchPos;

	return cfg->ini+iPos+strlen(sName)+1;
}
