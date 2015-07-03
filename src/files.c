#include "files.h"

char *lcFileGetName( char *sFilepath ){
	return strrchr(sFilepath,'/');
}

char *lcFileGetExtension( char *sFileNameOrPath ){
	return strrchr(sFileNameOrPath,'.');	
}

_Bool lcFileExists( char *sFilepath ){
	
	FILE *file;
	file = fopen( sFilepath, "r" );
    if( file ){
        fclose( file );
        return TRUE;
    }
    
    return FALSE;
}

int lcFileSize( char *sFilepath ){
	int iSize = 0;
	FILE *file;
	
	if( lcFileExists( sFilepath ) == FALSE ){
		syslog(LOG_WARNING,  "file %s does not exist returning size -1", sFilepath );
		return -1;
	} 
	
	file = fopen( sFilepath, "r" );
	fseek( file, 0L, SEEK_END );
	
	iSize = (int)ftell( file );
	
	fclose( file );
	
	return iSize;
}
int lcFileSteamSize( FILE *fp ){
	int iSize = 0;

	if( !fp ){
		return 0;
	} 
	
	fseek( fp, 0L, SEEK_END );
	
	iSize = (int)ftell( fp );
		
	return iSize;
}

_Bool lcFileDirExists(char *sPath){
	DIR* dir = opendir(sPath);
	if (dir){
		closedir(dir);
		return TRUE;
	}else
		return FALSE;
}

_Bool lcFileIsDir(char *sPath,_Bool bIgnoreDot){
    
    if(bIgnoreDot == TRUE){
		if(strcmp(sPath,"..")==0 || strcmp(sPath,".")==0|| strcmp(sPath,"../")==0|| strcmp(sPath,"./")==0)
			return FALSE;
	}
	
	if(lcFileDirExists(sPath)==FALSE){
		return FALSE;
	}
    
    struct stat path_stat;
    stat(sPath, &path_stat);
    
    if(!S_ISREG(path_stat.st_mode))
		return TRUE;
	return FALSE;
}

_Bool lcFileIsFile(char *sPath){
    struct stat path_stat;
    stat(sPath, &path_stat);
    
    
    if(S_ISREG(path_stat.st_mode))
		return TRUE;
	return FALSE;
}

_Bool lcFileCopy( char *sSrc, char  *sDest ){
	int ch;
	FILE* in = fopen(sSrc, "rb");

	if( !lcFileExists( sSrc ) ){
		syslog(LOG_WARNING,  "lcFileCopy: file to copy does not exist! %s", sSrc );
		return FALSE;
	}

	FILE* out = fopen( sDest, "wb");

	if( out == NULL ){
		fclose( in );
		syslog(LOG_WARNING, "lcFileCopy: can not open destinationfile %s", sDest );
		return FALSE;
	}

	while( (ch = fgetc( in )) != EOF)
		fputc( ch, out );

	fclose( in );
	fclose( out );
	
	return TRUE;
}

char *lcFileReadLines(char *sFile,int iStart, int iAmount, int *iEndReached){
	
	int iFileSize 	= lcFileSize(sFile);
    FILE *fFile;
    char *sFileContent;
    int i=0;
    *iEndReached = 0;
	
	if(iFileSize == -1){
		return NULL;
	}
    
	fFile = fopen(sFile,"r+" );
    if(fFile == NULL){
        return NULL;
    }

    sFileContent = malloc(iFileSize+1);
    memset(sFileContent, '\0', iFileSize+1);
    fread (sFileContent, 1, iFileSize, fFile);
	fclose(fFile);
	
	int iLines = 0;
	for(i=0;i<iFileSize;i++){
		if(sFileContent[i] == '\n')
			iLines++;
	}
	
	if(iAmount > iLines){
		iAmount = iLines;
	}
	
	if(iStart <0){
		if(iStart + iAmount > 0){
			iAmount 	 = iLines+iStart;
		}
		
		if(iStart+iLines < 0){
			iStart 		 = 0;
			*iEndReached = 1;
		}else	
			iStart = iLines + iStart;
	
	}else if(iStart > 0){
		if(iStart + iAmount > iLines){
			iAmount 	= iLines -iStart;
			*iEndReached = 1;
		}
	}
	
	int iStartPos = -1;
	int iEndPos   = -1;
	
	int iTmp 	  = -1;
	for(i=0;i<iFileSize;i++){
		if(sFileContent[i] == '\n' || i==0){
			iTmp++;

			if(iTmp == iStart){
				iStartPos = i;
			}
		}

		if(iStartPos != -1 && (iTmp ==(iStart+iAmount) || i == iFileSize)){
			iEndPos = i;
		}
	}
	sFileContent[iEndPos] = '\0';
	
	int iLen = iEndPos-iStartPos;
	
	char *sRet = malloc(iLen+1);
	sRet[0] = '\0';
	strcpy(sRet,sFileContent+iStartPos);
	
	sRet[iLen] = '\0';
	
	free(sFileContent);
	
	return sRet;
}



char *lcFileToString(char *sFile, int *iSize){
	FILE *fFile;
	char *sFileContent;
	
	
	*iSize = lcFileSize(sFile);
	
	if(*iSize == -1)
		return NULL;
	
	fFile = fopen(sFile,"r");
	sFileContent = malloc(*iSize+1);
	memset(sFileContent,'\0',*iSize+1);
	fread(sFileContent,1,*iSize,fFile);
	
	fclose(fFile);
	
	return sFileContent;	
}
