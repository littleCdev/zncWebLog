#include "ApiFirstrunFunctions.h"

int ApiFirstRunDefault(struct mg_connection *conn, struct lcUser *User){
	debug("ApiFirstRunDefault: %s %s\n",conn->request_method,conn->uri);
	
	struct lcTemplate *tpl = lcTemplateLoad("firstrun/addRoot.html",User);
	lcTemplateAddVariableString(tpl,"Msg","");
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	return MG_TRUE;
}

int ApiFirstRunAddRoot(struct mg_connection *conn, struct lcUser *User){
	debug("ApiFirstRunAddRoot: %s %s\n",conn->request_method,conn->uri);


	char sName[NAMEMAXLEN+1] = {0},
		 sPass[PASSMAXLEN+1] = {0},
		 sErrorMsg[ERRORLEN] = {0};
		
	mg_get_var(conn, "Username", sName, NAMEMAXLEN+1);
	mg_get_var(conn, "Password", sPass, PASSMAXLEN+1);
	
	// check if username exists in znc
	char *sUserPath = lcStringCreate("%s%s/",CFG.sZncUserDir,sName);
	if(lcFileDirExists(sUserPath) == TRUE){
		free(sUserPath);
		struct lcTemplate *tpl = lcTemplateLoad("firstrun/addRoot.html",User);
		lcTemplateAddVariableString(tpl,"Msg","No znc-users i said...");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);
		return MG_TRUE;		
	}	
	free(sUserPath);	
		
	if(lcStrlen(sName) < 5 || lcStrlen(sPass) < 5){
		struct lcTemplate *tpl = lcTemplateLoad("firstrun/addRoot.html",User);
		lcTemplateAddVariableString(tpl,"Msg","Username or password is missing or too short");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);
		return MG_TRUE;
	}
	
	// write the user to configfile
	int iFileSize 	= lcFileSize(CFGFILE);
    FILE *fFile	= fopen(CFGFILE,"r+" );
    char *sFileContent;
    
    if(fFile == NULL){
		syslog(LOG_CRIT,"can not open configfile %s",CFGFILE);
		exit(1);
    }
    
    sFileContent = malloc(iFileSize+1);
    memset(sFileContent, '\0', iFileSize+1);
    fread (sFileContent, 1, iFileSize, fFile);
	fclose(fFile);
	
	// if Rootuser="" exists replace it
	if(lcStrStr(sFileContent,"Rootuser=") > -1){
		int iStartPos = lcStrStr(sFileContent,"Rootuser=");
		
		// + 2* "
		int iEndPos   = lcStrStr(sFileContent+iStartPos+lcStrStr(sFileContent,"\"")+1,"\"");
	
		sFileContent[iStartPos] = '\0';
		
		char *sNewFileContent = lcStringCreate("%s\r\nRootuser=%s\r\n%s",sFileContent,sName,sFileContent+iEndPos);
		free(sFileContent);
		
		fFile = fopen(CFGFILE,"w+");
		if(fFile == NULL){
			syslog(LOG_CRIT,"can not open configfile %s",CFGFILE);
			exit(1);
		}
		fputs(sNewFileContent,fFile);
		fclose(fFile);		
	}else{
		lcStringAdd(sFileContent,"\r\n# don't touch this line!\r\nRootuser=%s\r\n",sName);
		
		fFile = fopen(CFGFILE,"w+");
		if(fFile == NULL){
			syslog(LOG_CRIT,"can not open configfile %s",CFGFILE);
			exit(1);
		}
		fputs(sFileContent,fFile);
		fclose(fFile);		
	}
	
	syslog(LOG_INFO,"added Rootuser=%s to configfile",sName);
	

	if(lcAuthAddUser(sName,sPass,TRUE,sErrorMsg) == FALSE){
		
		syslog(LOG_DEBUG,"lcAuthAddUser: %s",sErrorMsg);
		
		struct lcTemplate *tpl = lcTemplateLoad("firstrun/addRoot.html",User);
		lcTemplateAddVariableString(tpl,"Msg",sErrorMsg);
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);		
	}else{
		CFG.bFirstRun = FALSE;
		// create cookie an forward to adminpage
		
		char *sString;
        char *sCookieContent = lcGenCookie(sName, sPass);
        sString = lcStringCreate( "HTTP/1.1 301 Moved Permanently \r\nLocation: /admin/Users!Add\r\nSet-Cookie: znc=%s;path=/\r\n\r\n",sCookieContent);
        
        mg_printf(conn, sString);
 
		free(sString);
		free(sCookieContent);
	}

	return MG_TRUE;
}
