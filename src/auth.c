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
	char *sCookie		= NULL;
	char *sCookies;
	User->sName			= lcStringCreate("");
	User->sUserDir		= lcStringCreate("");
    
    if(CFG.bFirstRun){
		return FALSE;
	}

    // cookies may look like:
    // znc=MTIzNDU2$r10nqp3949on59noor56r057s20s883r
    // PHPSESSID=6lpjo6aej17mv3dpa09lcc8874; znc=MTIzNDU2$r10nqp3949on59noor56r057s20s883r
    // PHPSESSID=6lpjo6aej17mv3dpa09lcc8874; znc=MTIzNDU2$r10nqp3949on59noor56r057s20s883r; test=true
    sCookies       = (char *)mg_get_header(conn, "Cookie");
    if(sCookies == NULL){
        User->login = FALSE;
        return FALSE;
    }
    debug("sCookies:%s",sCookies);
    
    int iCookieStart = lcStrStr(sCookies,"znc=");
    if(iCookieStart == -1){
		debug("no cookie found");
		return FALSE;
	}

	// +4 because of "znc="
	iCookieStart+=4;
	
	int iCookieEnd = lcStrStr(sCookies+iCookieStart,";");
	if(iCookieEnd == -1){
		sCookie = lcStringCreate(sCookies+iCookieStart);
	}else{
		int iLen = iCookieEnd-iCookieStart;
		sCookie = malloc(iLen+1);
		memset(sCookie,'\0',iLen+1);
		strncpy(sCookie,sCookies+iCookieStart,iLen);
	}
    
    iSplitPos = lcStrStr(sCookie, "$");
    if(iSplitPos == -1){
        User->login = FALSE;
        debug("can not find $\n");
        lcFree(sCookie);
        
        return FALSE;
    }
    
    sTmp = malloc(iSplitPos+1);
    strncpy(sTmp, sCookie, iSplitPos);
    sTmp[iSplitPos] = '\0';
    
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
        lcFree(sCookie);
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
		lcFree(sCookie);
		
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
		lcFree(sCookie);
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
    lcFree(sCookie);
    lcFree(sName);
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

_Bool lcAuthUserDelete(char *sName, char *sErrorMsg){
	// check if name exists in passwordfile
	int i;
	char *sPasswdFile	= lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sFileContent  = lcFileToString(sPasswdFile,&i);
		
	int iNamePos = lcStrStr(sFileContent,sName);
	if(iNamePos==-1){
		SETERROR(sErrorMsg,"the user does not exists");
		
		lcFree(sPasswdFile);
		lcFree(sFileContent);
		
		return FALSE;
	}
	
	int   iLineEnd 			= iNamePos + lcStrStr(sFileContent+iNamePos,"\n");
	char  *sNewFileContent 	= lcStringCreate("");
	FILE  *fPasswdFile	;
		
	// cut the string at the start of the line (name -2)
	sFileContent[iNamePos-2] = '\0';
		
	if(lcStrlen(sFileContent)>0)
		lcStringAdd(sNewFileContent,"%s",sFileContent);
	if(lcStrlen(sFileContent+iLineEnd)>0)
		lcStringAdd(sNewFileContent,"%s",sFileContent+iLineEnd+1);
			
	fPasswdFile = fopen(sPasswdFile,"w+");
	if(!fPasswdFile){
		SETERROR(sErrorMsg,"failed to open/write passwordfile");
		syslog(LOG_CRIT,"can not open (w+) passwordfile %s",sPasswdFile);
		debug("can not open (w+) passwordfile %s",sPasswdFile);
	}else{
		fputs(sNewFileContent,fPasswdFile);
		fclose(fPasswdFile);
		debug("deleted %s",sName);
		syslog(LOG_INFO,"deleted %s",sName);
		SETERROR(sErrorMsg,"deleted %s",sName);
	}
		
	free(sNewFileContent);
	free(sPasswdFile);
	free(sFileContent);	
	
	return TRUE;
}

_Bool lcAuthChangePassword(char *sUser, char *sOldPasswd, char *sNewPasswd, _Bool bVerifyOldpw, char *sErrorMsg){

	if(bVerifyOldpw && lcStrlen(sOldPasswd) < 6){
		SETERROR(sErrorMsg,"old password is invalid");
		return FALSE;
	}
	if(lcStrlen(sNewPasswd) < 6){
		SETERROR(sErrorMsg,"new password is too short (6 char min)");
		return FALSE;
	}
	
	int  iSize			= 0;
	int  iPwStartPos	= 0;
	int  iLen			= 0;
	int  iNamePos		= 0;
	char *sPwFile		= lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sOldPasswdMd5 = lcMd5FromString(sOldPasswd);
	char *sNewPasswdMd5 = lcMd5FromString(sNewPasswd);
	char *sFileContent 	= lcFileToString(sPwFile,&iSize);
	char *sOldPwFromFile= lcStringCreate("");
	
	iNamePos = lcStrStr(sFileContent,sUser);
	if(iNamePos == -1){
		SETERROR(sErrorMsg,"invalid user");
		
		lcFree(sPwFile);
		lcFree(sOldPasswdMd5);
		lcFree(sNewPasswdMd5);
		lcFree(sFileContent);
		lcFree(sOldPwFromFile);
		
		return FALSE;
	}
	
	iPwStartPos = lcStrStr(sFileContent+iNamePos,"|") + iNamePos+1; // +1 -> remove the |
	iLen		= lcStrStr(sFileContent+iPwStartPos,"\r");
	
	if(iPwStartPos == 0 || iLen == -1){
		SETERROR(sErrorMsg,"can not fing password");
		debug("can not find password?");
		
		lcFree(sPwFile);
		lcFree(sOldPasswdMd5);
		lcFree(sNewPasswdMd5);
		lcFree(sFileContent);
		lcFree(sOldPwFromFile);
		
		return FALSE;
	}
	debug("iPwStartPos:%i",iPwStartPos);
	debug("iLen:%i",iLen);
	sOldPwFromFile = realloc(sOldPwFromFile,iLen+1);
	strncpy(sOldPwFromFile,sFileContent+iPwStartPos,iLen);
	sOldPwFromFile[iLen] = '\0';
	debug("sOldPwFromFile:%s",sOldPwFromFile);
	
	if(bVerifyOldpw && strcmp(sOldPwFromFile,sOldPasswdMd5) != 0){
		SETERROR(sErrorMsg,"Old password is invalid");
		debug("old password is invalid");
	
		lcFree(sPwFile);
		lcFree(sOldPasswdMd5);
		lcFree(sNewPasswdMd5);
		lcFree(sFileContent);
		lcFree(sOldPwFromFile);
		
		return FALSE;
	}
	
	debug("replacing %s with %s",sOldPwFromFile,sNewPasswdMd5);
	lcStrReplace(sFileContent,sOldPwFromFile,sNewPasswdMd5);
	
	FILE *fPwFile = fopen(sPwFile,"w+");
	if(!fPwFile){
		syslog(LOG_CRIT,"cannot delete/write %s",sPwFile);
		SETERROR(sErrorMsg,"Fatal error: cannot delete/write %s",sPwFile);
		
		lcFree(sPwFile);
		lcFree(sOldPasswdMd5);
		lcFree(sNewPasswdMd5);
		lcFree(sFileContent);
		lcFree(sOldPwFromFile);
		
		return FALSE;
	}
	
	fputs(sFileContent, fPwFile);
	fclose(fPwFile);
	
	lcFree(sPwFile);
	lcFree(sOldPasswdMd5);
	lcFree(sNewPasswdMd5);
	lcFree(sFileContent);
	lcFree(sOldPwFromFile);

	return TRUE;
}

_Bool lcAuthUserChangeUsertype(char *sUser,lcUserType userType, char *sErrorMsg){
	int  i 			   = 0;
	int  iNamePos	   = -1;
	char *sPasswdFile  = lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sFileContent = lcFileToString(sPasswdFile,&i);
	
	iNamePos = lcStrStr(sFileContent,sUser);
	if(iNamePos == -1){
		SETERROR(sErrorMsg,"can not find User");
		
		lcFree(sPasswdFile);
		lcFree(sFileContent);
		
		return FALSE;
	}
	
	sFileContent[iNamePos-2] = userType == lcADMIN ? '1':'0';
	
	
	FILE *fPwFile = fopen(sPasswdFile,"w+");
	if(!fPwFile){
		syslog(LOG_CRIT,"cannot delete/write %s",sPasswdFile);
		SETERROR(sErrorMsg,"Fatal error: cannot delete/write %s",sPasswdFile);
		
		lcFree(sPasswdFile);
		lcFree(sFileContent);
		
		return FALSE;
	}
	
	fputs(sFileContent, fPwFile);
	fclose(fPwFile);
	
	lcFree(sPasswdFile);
	lcFree(sFileContent);
	
	syslog(LOG_INFO,"changed %s to %s",sUser,userType==lcADMIN?"Admin":"User");
	debug("changed %s to %s",sUser,userType==lcADMIN?"Admin":"User");
	
	return TRUE;
}
