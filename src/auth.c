#include "auth.h"

char *lcGenCookie( char *sName, char *sPass){
    char *sRet      	= NULL;
    char *sMd5Pw    	= NULL;
    char *sRot13Pw  	= NULL;
    char *sBase64Name 	= NULL;
    
    int iLen = 0;
    
    sBase64Name = base64_encode(sName, (int)strlen(sName), &iLen);

	sMd5Pw = lcMd5FromString(sPass);
    sRot13Pw = lcRot13Encode(sMd5Pw);

	sRet = lcStringCreate("%s$%s",sBase64Name,sRot13Pw);

    lcFree(sBase64Name);
    lcFree(sMd5Pw);
    lcFree(sRot13Pw);

    return sRet;
}

_Bool lcAuthCheckLoginData(char *sName,char *sPass, char *sPassMd5){
    
    char *sMd5;
    char *sFileContent;
    char *sPwEntrie;
    char *sPasswdFile;
    
    int iPos;
    int iFileSize;
    FILE *fPassFile;
    
    if(sPass != NULL && sPassMd5 != NULL)
		return FALSE;
	
	if(sPassMd5 != NULL){
		asprintf(&sMd5,"%s",sPassMd5);
	}else{
		sMd5 = lcMd5FromString(sPass);
	}
	
    if(lcStrlen(sMd5)!=32){
        lcFree(sMd5);
        return FALSE;
    }
    
    // load passwordfile into a string
    sPasswdFile = lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
    iFileSize = lcFileSize(sPasswdFile);
    fPassFile= fopen(sPasswdFile,"r+" );
    if(fPassFile == NULL){
        syslog(LOG_CRIT, "could not open passwordfile %s",sPasswdFile);
        lcFree(sPasswdFile);
        lcFree(sMd5);

        return FALSE;
    }

    sFileContent = malloc(iFileSize+1);
    memset(sFileContent, '\0', iFileSize+1);
    fread (sFileContent, 1, iFileSize, fPassFile);
    
    fclose(fPassFile);

    // md5("admin") = 21232f297a57a5a743894a0e4a801fc3
    // decode hash and search in the passwordfile
    sPwEntrie = lcStringCreate("%s|%s\r\n",sName,sMd5);
    
    // check if username with password exists into the passwordfile
    iPos = lcStrStr(sFileContent,sPwEntrie );

	lcFree(sPasswdFile);
	lcFree(sMd5);
	lcFree(sFileContent);
	lcFree(sPwEntrie);
    
    if(iPos==-1){
		return FALSE;
    }
 
    return TRUE;
}

_Bool lcAuthCheckCookie(struct mg_connection *conn,lcUser *User){
    // cookieformat: base64(username)$rot13(md5(password))

    int iSplitPos       = 0;
    int i               = 0;
    int iPos            = 0;
    int iFileSize       = 0;
    
    char *sMd5          = NULL;
    char *sTmp          = NULL;
    char *sName         = NULL;
    char *sPwEntrie     = NULL;
    char *sFileContent  = NULL;
    FILE *fPassFile     = NULL;
    char *sPasswdFile   = NULL;
   
	User->sName			= lcStringCreate("");
	User->sUserDir		= lcStringCreate("");
    
    if(CFG.bFirstRun){
		return FALSE;
	}
    
    char *sCookie       = (char *)mg_get_header(conn, "Cookie");
    if(sCookie == NULL){
        User->login = FALSE;
        return FALSE;
    }
    
    
    iSplitPos = lcStrStr(sCookie, "$");
    if(iSplitPos > NAMEMAXLEN || iSplitPos == -1){
        User->login = FALSE;
        debug("can not find $ or name too long\n");
        return FALSE;
    }
    
    sTmp = malloc(iSplitPos+1);
    strncpy(sTmp, sCookie, iSplitPos);
    sTmp[iSplitPos] = '\0';
    
    // remove "znc=" and decode the name
    lcStrReplace(sTmp, "znc=", "");
    
    sName = base64_decode(sTmp, lcStrlen(sTmp), &i);
	debug("sName: %s\n",sName);

    // free memory and set pointer to the md5-hash
    lcFree(sTmp);

    sTmp = sCookie +iSplitPos+1;

    // get md5 and check if it's ok
    sMd5 = lcRot13Decode(sTmp);

    if(lcStrlen(sMd5)!=32){
        User->login = FALSE;
        lcFree(sMd5);
        debug("strlen of md5 != 32\n");
        return FALSE;
    }
    
    // load passwordfile into a string
    sPasswdFile = lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
    iFileSize = lcFileSize(sPasswdFile);
    fPassFile= fopen(sPasswdFile,"r+" );
    if(fPassFile == NULL){
        debug("could not open passwordfile %s",sPasswdFile);
        syslog(LOG_CRIT, "could not open passwordfile %s",sPasswdFile);
        lcFree(sPasswdFile);

        User->login = FALSE;
        
        return FALSE;
    }

    sFileContent = malloc(iFileSize+1);
    memset(sFileContent, '\0', iFileSize+1);
    fread (sFileContent, 1, iFileSize, fPassFile);
    
    fclose(fPassFile);
    lcFree(sPasswdFile);

    // md5("admin") = 21232f297a57a5a743894a0e4a801fc3
    // decode hash and search in the passwordfile
    sPwEntrie = lcStringCreate("%s|%s\r\n",sName,sMd5);
    lcFree(sMd5);
    debug("sPwEntrie: %s\n",sPwEntrie);
    // check if username with password exists into the passwordfile
    iPos = lcStrStr(sFileContent,sPwEntrie );
    if(iPos==-1){
        lcFree(sFileContent);
        lcFree(sPwEntrie);
		debug("sPwEntrie not found in file!\n");
        User->login = FALSE;
        return FALSE;
    }
    
    User->UserType = sFileContent[iPos-2]=='1'?lcADMIN:lcUSER;
    lcStringAdd(User->sName,"%s",sName);
    User->login = TRUE;
    lcStringAdd(User->sUserDir,"%s%s/moddata/log/",CFG.sZncUserDir,sName);
    
    lcFree(sFileContent);
    lcFree(sPwEntrie);
    free(sName);
    return TRUE;
}

_Bool lcAuthAddUser(char *sName, char *sPass, _Bool Admin, char *sError){
    if(lcStringCountChars(sName, "|")>0){
        SETERROR(sError, "Names aren't allowed to contain |");
        return FALSE;
    }
    if(lcStrlen(sName)>NAMEMAXLEN){
        SETERROR(sError, "Name too long");
        return FALSE;
	}
    if(lcStrlen(sPass)<6 ){
        SETERROR(sError, "Password is too short or too long (6 char min)");
        return FALSE;
	}
    char *sFileContent;
    FILE *fPassFile;
    char *sPasswdFile = lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
    int iFileSize = lcFileSize(sPasswdFile);
    
    
    if(lcFileExists(sPasswdFile)==FALSE){
		fPassFile= fopen(sPasswdFile,"w+" );
	}else{
		fPassFile= fopen(sPasswdFile,"r+" );
	}
	
    if(fPassFile == NULL){
        perror("fopen");
        syslog(LOG_CRIT, "could not open passwordfile %s",sPasswdFile);
        lcFree(sPasswdFile);
        return FALSE;
    }
    
    sFileContent = malloc(iFileSize+1);
    memset(sFileContent, '\0', iFileSize+1);
    fread (sFileContent, 1, iFileSize, fPassFile);
    
    if(lcStrStr(sFileContent, sName)>0){
        fclose(fPassFile);
        lcFree(sPasswdFile);
        lcFree(sFileContent);
        SETERROR(sError, "User already exists");
        return FALSE;
    }
    
    char *sMd5 = lcMd5FromString(sPass);
    
    fseek(fPassFile, 0, SEEK_END);
    fprintf(fPassFile, "%i|%s|%s\r\n",Admin?1:0,sName,sMd5);
    
    lcFree(sFileContent);
    lcFree(sMd5);
    fclose(fPassFile);

    return TRUE;
}

_Bool lcAuthChangePassword(char *sUser, char *sOldPasswd, char *sNewPasswd, char *sErrorMsg){

	if(lcStrlen(sNewPasswd) == 0 || lcStrlen(sOldPasswd) == 0 ){
		SETERROR(sErrorMsg,"invalid passwords");
		return FALSE;
	}
	
	if(lcStrlen(sOldPasswd) < 6){
		SETERROR(sErrorMsg,"old password is invalid");
		return FALSE;
	}
	if(lcStrlen(sNewPasswd) < 6){
		SETERROR(sErrorMsg,"new password is too short (6 char min)");
		return FALSE;
	}
	int  iSize			= 0;
	char *sPwFile		= lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sOldPasswdMd5 = lcMd5FromString(sOldPasswd);
	char *sNewPasswdMd5 = lcMd5FromString(sNewPasswd);
	char *sPwEntrie 	= lcStringCreate("%s|%s",sUser,sOldPasswdMd5);
	char *sNewPwEntrie 	= lcStringCreate("%s|%s",sUser,sNewPasswdMd5);
	char *sFileContent 	= lcFileToString(sPwFile,&iSize);
	
	if(lcStrStr(sFileContent,sPwEntrie)==-1){
		SETERROR(sErrorMsg,"your password is wrong");
		
		free(sOldPasswdMd5);
		free(sPwEntrie);
		free(sPwFile);
		free(sNewPasswdMd5);
		free(sFileContent);
		free(sNewPwEntrie);
		
		return FALSE;
	}
	
	lcStrReplace(sFileContent,sPwEntrie,sNewPwEntrie);
	
	FILE *fPwFile = fopen(sPwFile,"w+");
	if(!fPwFile){
		syslog(LOG_CRIT,"cannot delete/write %s",sPwFile);
		SETERROR(sErrorMsg,"Fatal error: cannot delete/write %s",sPwFile);
		
		free(sOldPasswdMd5);
		free(sPwEntrie);
		free(sPwFile);
		free(sNewPasswdMd5);
		free(sFileContent);
		free(sNewPwEntrie);
		
		return FALSE;
	}
	
	fputs(sFileContent, fPwFile);
	fclose(fPwFile);
	
	free(sOldPasswdMd5);
	free(sPwEntrie);
	free(sPwFile);
	free(sNewPasswdMd5);
	free(sFileContent);
	free(sNewPwEntrie);

	return TRUE;
}
