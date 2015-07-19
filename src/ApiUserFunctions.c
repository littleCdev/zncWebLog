#include "lcZncWebLog.h"
#include "mongoose.h"
#include "auth.h"
#include "ApiUserFunctions.h"
#include "templates.h"
#include "files.h"

int _qsortCompareFunction(const void *name1, const void *name2)
{
    const char *name1_ = *(const char **)name1;
    const char *name2_ = *(const char **)name2;
    return strcmp(name1_, name2_);
}

int ApiUserListNetworks(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	NOROOTFUNC

	DIR *dp;
	struct dirent *ep;     
	struct lcTemplate *tpl;
	char *sNetworkPath;
	char *sNetworkList;
	char **asNetworks 	= malloc(1*sizeof(char*));
	int	 iNetworks	  	= 0;
	int	 i				= 0;
	sNetworkPath = lcStringCreate("%s",User->sUserDir);
	
	tpl = lcTemplateLoad("listNetworks.html",User);

	debug("ApiUserListNetworks: sNetworkpath: %s\n",sNetworkPath);
	
	dp = opendir (sNetworkPath);
	if (dp != NULL){
		while ((ep = readdir(dp))){
			if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0)
				continue;
				
			char *sTmp = lcStringCreate("%s%s/",sNetworkPath,ep->d_name);
			
			if(lcFileIsDir(sTmp,TRUE) == TRUE){
				debug("ApiUserListNetworks: found: %s/%s\n",User->sName,ep->d_name);
				
				asNetworks = realloc(asNetworks,(iNetworks+1)*sizeof(char *));
				asNetworks[iNetworks] = lcStringCreate(ep->d_name);
				iNetworks++;
			}
			free(sTmp);
		}
		(void) closedir (dp);
	}
	
	qsort(asNetworks,iNetworks,sizeof(char *),_qsortCompareFunction);
	
	
	sNetworkList = lcStringCreate("<ul class=\"list-group\">\n");
	for(i=0;i<iNetworks;i++)
			lcStringAdd(sNetworkList,"<li class=\"list-group-item\"><a href=\"/network/%s\">%s</a></li>\n",asNetworks[i],asNetworks[i]);			
	lcStringAdd(sNetworkList,"</ul>");


	
	lcTemplateAddVariableString(tpl,"sNetworkList",sNetworkList);
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	free(sNetworkList);
	free(sNetworkPath);
	for(i=0;i<iNetworks;i++)
		free(asNetworks[i]);
	free(asNetworks);
	
	return MG_TRUE;
}

int ApiUserShowNetwork(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	NOROOTFUNC
	
	struct lcTemplate *tpl;
	char *pNetwork;
	char *sNetworkPath;
	DIR *dp;
	struct dirent *ep;    
	char *sHtmlList = lcStringCreate(" ");
	char *sEncodeBuffer;
	
	char **asChanQueries= malloc(sizeof(char *));
	int	 iChanQueries	= 0;
	char **asLogFiles  	= malloc(sizeof(char *));
	int  iLogFiles		= 0;
	int  i				= 0;
	int  b				= 0;
	
	
	tpl = lcTemplateLoad("showNetwork.html",User);
	
	pNetwork = strrchr(conn->uri,'/')+1;
	debug("ApiUserShowNetwork: network: %s\n",pNetwork);
	sNetworkPath = lcStringCreate("%s%s/",User->sUserDir,pNetwork);
	debug("ApiUserShowNetwork: sNetworkPath: %s\n",sNetworkPath);
	
	if(lcFileIsDir(sNetworkPath,TRUE)==FALSE){
		lcTemplateAddVariableString(tpl,"UsersAndChans","network does not exist! :(");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);		
		return MG_TRUE;
	}
	
	debug("opening: %s\n",sNetworkPath);
	dp = opendir (sNetworkPath);
	
	if (dp != NULL){
		while( (ep = readdir(dp)) ){
			if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0)
				continue;
			
			asChanQueries = realloc(asChanQueries,(iChanQueries+1)*sizeof(char *));
			asChanQueries[iChanQueries] = lcStringCreate(ep->d_name);
			iChanQueries++;
		}
	
		(void) closedir (dp);
	}
	debug("read chans & queries (%i)",iChanQueries);
	
	qsort(asChanQueries,iChanQueries,sizeof(char *),_qsortCompareFunction);

	for(i=0;i<iChanQueries;i++){
		char *sChanQueryPath = lcStringCreate("%s%s",sNetworkPath,asChanQueries[i]);
		debug("opening chan/querie: %s",sChanQueryPath);
			
		dp = opendir(sChanQueryPath);
		if( dp != NULL ){
			
			while( (ep = readdir(dp)) ){
				if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0)
					continue;
				
				asLogFiles = realloc(asLogFiles,(iLogFiles+1)*sizeof(char *));
				asLogFiles[iLogFiles] = lcStringCreate(ep->d_name);
				iLogFiles++;
			}
			
			closedir(dp);
		
			qsort(asLogFiles,iLogFiles,sizeof(char *),_qsortCompareFunction);
			
			char *sCssClassName = lcStringCreate("%s",asChanQueries[i]);
			lcStrReplace(sCssClassName,"#","_");
			lcStrReplace(sCssClassName,".","_");
			
			lcStringAdd(sHtmlList,
			"<div class=\"panel-heading \">\n"
				"<h3 class=\"panel-title slideup\" data=\"%s\"> <span class=\"glyphicon glyphicon-folder-close %s\" aria-hidden=\"true\"></span>%s</h3>\n"
				"</div>\n"
				"<div class=\"slideup %s\" data=\"%s\">\n",sCssClassName,sCssClassName,asChanQueries[i],sCssClassName,sCssClassName);
				
			sEncodeBuffer = lcStringCreate("%s",asChanQueries[i]);
			lcStrReplace(sEncodeBuffer,"#","%%%%23");
			
			for(b=0;b<iLogFiles;b++){
				lcStringAdd(sHtmlList,
						"<div class=\"panel-body\">\n"
							"<a href=\"/log/download!%s/%s/%s\" class=\"text-muted\">\n"
								"<span class=\"glyphicon glyphicon-download-alt\" aria-hidden=\"true\"></span>\n"
							"</a>\n"
							"<a href=\"/log/%s/%s/%s/\">%s%s</a>\n"
						"</div>\n",pNetwork,sEncodeBuffer,asLogFiles[b],pNetwork,sEncodeBuffer,asLogFiles[b],asChanQueries[i],asLogFiles[b]);
				free(asLogFiles[b]);
			}
			lcStringAdd(sHtmlList,"</div>\n");
			
			free(sCssClassName);
			free(sEncodeBuffer);
			free(asLogFiles);
			asLogFiles = malloc(sizeof(char *));
			iLogFiles = 0;
		}else{
	//		syslog(LOG_WARN,"failed to open: %s",sChanQueryPath);
		}
		free(sChanQueryPath);
	}

	lcTemplateAddVariableString(tpl,"sNetworkName",pNetwork);
	lcTemplateAddVariableString(tpl,"UsersAndChans",sHtmlList);

	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	free(sNetworkPath);
	free(sHtmlList);

	for(i=0;i<iChanQueries;i++)	free(asChanQueries[i]);
	free(asChanQueries);
	for(i=0;i<iLogFiles;i++)	free(asLogFiles[i]);
	free(asLogFiles);

	return MG_TRUE;
}


	
int ApiUserShowLog(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	char *sUri = lcStringCreate("%s",conn->uri);
	
	strtok(sUri,"/");
	char *pNetwork		= strtok(NULL,"/");
	char *pChanQuery	= strtok(NULL,"/");
	char *pFileName		= strtok(NULL,"/");
	char *sFilePath	;
	struct lcTemplate *tpl;
	int	 iEndReached	= 42;
		
	debug("ApiUserShowLog: %s\n",conn->uri);
	
	sFilePath = lcStringCreate("%s%s/%s/%s",User->sUserDir,pNetwork,pChanQuery,pFileName);

	char *sLog = lcFileReadLines(sFilePath,CFG.iLogLines*-1,CFG.iLogLines,&iEndReached);
	
	if(lcStrlen(sLog)==0){
		tpl = lcTemplateLoad("error.html",User);
		lcTemplateAddVariableString(tpl,"Msg","your requested log doesn't exist");
		syslog(LOG_INFO,"%s does not exist",sLog);
	}else{
		lcStrReplace(sLog,">","&gt;");
		lcStrReplace(sLog,"<","&lt;");

		tpl = lcTemplateLoad("showLog.html",User);
		lcTemplateAddVariableString(tpl,"sNetworkName",pNetwork);
		lcTemplateAddVariableString(tpl,"sFileName",pFileName);
		lcTemplateAddVariableString(tpl,"sChanQuery",pChanQuery);
		lcTemplateAddVariableString(tpl,"bEndReached",iEndReached==1?"true":"false");
		lcTemplateAddVariableInt(tpl,"iLines",CFG.iLogLines);
		lcTemplateAddVariableString(tpl,"sLog",sLog);
	}
	
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	lcFree(sFilePath);
	lcFree(sUri);
	lcFree(sLog);
	return MG_TRUE;	
}

int ApiUserSendLogJson(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	char *sUri = lcStringCreate("%s",conn->uri);
	
	strtok(sUri,"/");
	char *pNetwork		= strtok(NULL,"/");
	char *pChanQuery	= strtok(NULL,"/");
	char *pFileName		= strtok(NULL,"/");
	char *sFilePath	;
	char *sLog;
	
	int	 iEndReached	= 42;
	int i;
	int iStartLine		= -1;
	
	char sStartLine[10] = {0};
	mg_get_var(conn, "Start", sStartLine, 9);

	debug("ApiUserShowLog: %s\n",conn->uri);
	syslog(LOG_INFO,"ApiUserShowLog: %s\n",conn->uri);
	sFilePath = lcStringCreate("%s%s/%s/%s",User->sUserDir,pNetwork,pChanQuery,pFileName);



	if(strcmp(sStartLine,"-1")==0){
		sLog = lcFileToString(sFilePath,&i);
	}else{
		iStartLine = atoi(sStartLine);
		iStartLine += CFG.iLogLines+1;
		
		debug("iStartLine: %i\n",iStartLine);
	
		sLog = lcFileReadLines(sFilePath,iStartLine*-1,CFG.iLogLines,&iEndReached);
	}
	
	if(lcStrlen(sLog)==0){

		char *sMsgBase64 = base64_encode("your requested log doesn't exist",strlen("your requested log doesn't exist"),&i);
		syslog(LOG_INFO,"%s does not exist",sLog);
		mg_send_header(conn, "Content-type", "text/html;charset=utf-8");
		mg_printf_data(conn,"{"
						  "\"error\":\"MQ==\","
						  "\"msg\":\"%s\""
						"}",sMsgBase64);
						
		free(sMsgBase64);
	}else{
		lcStrReplace(sLog,">","&gt;");
		lcStrReplace(sLog,"<","&lt;");

		//tpl = lcTemplateLoad("showLog.html",User);
		//lcTemplateAddVariableString(tpl,"sNetworkName",pNetwork);
		//lcTemplateAddVariableString(tpl,"sLog",sLog);
		
		char *sLogBase64 = base64_encode(sLog,strlen(sLog),&i);
		char *sEndReachedBas64 = base64_encode((iEndReached==1?"1":"0"),1,&i);
		mg_send_header(conn, "Content-type", "text/html;charset=utf-8");
		mg_printf_data(conn,"{"
						  "\"error\":\"MTMzNw==\","
						  "\"endReached\":\"%s\","
						  "\"log\":\"%s\""
						"}",sEndReachedBas64,sLogBase64);
									
		free(sLogBase64);
		free(sEndReachedBas64);
	}
	
	lcFree(sFilePath);
	lcFree(sUri);
	lcFree(sLog);
	return MG_TRUE;	
}

int ApiUserLogout(struct mg_connection *conn, struct lcUser *User){
	debug("ApiLogout: %s %s\n",conn->request_method,conn->uri);
    LOGINONLY

	if(User->login == TRUE){
		debug("loggin out\n");
		User->login = FALSE;
		User->UserType = lcLOGOUT;
		User->sName[0] = '\0'; // will be freed later
		User->sUserDir[0] = '\0'; // will be free later
	
		struct lcTemplate *tpl = lcTemplateLoad("login.html",User);
		lcTemplateAddVariableString(tpl,"Msg","Bye!");
		mg_send_header(conn, "Set-Cookie", "znc=;path=/;");
		lcTemplateSend(conn, tpl);
		lcTemplateClean(tpl);
	}else{
		struct lcTemplate *tpl = lcTemplateLoad("error.html",User);
		lcTemplateAddVariableString(tpl,"Msg","why are you trying to logout if you are not logged in?");
		mg_send_header(conn, "Set-Cookie", "znc=;path=/;");
		lcTemplateSend(conn, tpl);
		lcTemplateClean(tpl);	
	}

    return MG_TRUE;
}

int ApiUserLogin(struct mg_connection *conn, struct lcUser *User){
    
    debug("ApiLogin: %s %s\n",conn->request_method,conn->uri);
	char name[NAMEMAXLEN+1] = {0},
		 pass[PASSMAXLEN+1] = {0};
    
    mg_get_var(conn, "Username", name, NAMEMAXLEN);
	mg_get_var(conn, "Password", pass, PASSMAXLEN);

    if(strlen(name) == 0 || strlen(pass) == 0){
		struct lcTemplate *tpl = lcTemplateLoad("login.html",User);
		lcTemplateAddVariableString(tpl, "Msg", "");
		lcTemplateSend(conn, tpl);
        lcTemplateClean(tpl);

    }else if(lcAuthCheckLoginData(name,pass,NULL)==FALSE){
        struct lcTemplate *tpl = lcTemplateLoad("login.html",User);
        lcTemplateAddVariableString(tpl, "Msg", "Username or password wrong!");
		mg_send_header(conn, "Set-Cookie", "znc=;path=/;");
        lcTemplateSend(conn, tpl);
        lcTemplateClean(tpl);
    }else{
		debug("Login ok, %s\n",name);
        
        char *sString;
        char *sCookieContent = lcGenCookie(name, pass);
        
        if(strcmp(CFG.sRootUser,name)==0)
			sString = lcStringCreate( "HTTP/1.1 301 Moved Permanently \r\nLocation: /admin/\r\nSet-Cookie: znc=%s;path=/\r\n\r\n",sCookieContent);
		else
			sString = lcStringCreate( "HTTP/1.1 301 Moved Permanently \r\nLocation: /networks/\r\nSet-Cookie: znc=%s;path=/\r\n\r\n",sCookieContent);
        
        mg_printf(conn, sString);
        mg_send_header(conn,"connection","close true");
 
        lcFree(sString);
        lcFree(sCookieContent);
	}
	return MG_TRUE;
}

int ApiUserChangePassword(struct mg_connection *conn, struct lcUser *User){
	debug("ApiUserChangePassword: %s %s\n",conn->request_method,conn->uri);
	LOGINONLY
	
	if(strcmp(conn->request_method,"POST")==0){
		
		char *sOldPw = malloc(PASSMAXLEN+1);
		char *sNewPw = malloc(PASSMAXLEN+1);
		char *sErrorMsg = malloc(ERRORLEN+1);
		memset(sErrorMsg,'\0',ERRORLEN+1);
		struct lcTemplate *tpl;
		
		mg_get_var(conn, "newPasswd", sNewPw, NAMEMAXLEN);
		mg_get_var(conn, "oldPasswd", sOldPw, PASSMAXLEN);
		
		if(lcAuthChangePassword(User->sName,sOldPw,sNewPw,TRUE,sErrorMsg) == FALSE){
			tpl = lcTemplateLoad("changePw.html",User);
			lcTemplateAddVariableString(tpl,"Msg", sErrorMsg);		
		}else{
			User->sName[0]		= '\0';
			User->login			= FALSE;
			User->UserType		= lcLOGOUT;
			User->sUserDir[0] 	= '\0';
			tpl = lcTemplateLoad("login.html",User);
			lcTemplateAddVariableString(tpl,"Msg", "Password changed!<br>Please login with your new password");
			mg_send_header(conn, "Set-Cookie", "znc=;path=/;");
		}

		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);			

		free(sOldPw);
		free(sNewPw);
		free(sErrorMsg);
	}else{ // GET, only show the normal form
	
		struct lcTemplate *tpl = lcTemplateLoad("changePw.html",User);
		lcTemplateSend(conn, tpl);
		lcTemplateClean(tpl);
	}
	
	return MG_TRUE;
}

int ApiUserDownloadLog(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY

	struct lcTemplate *tpl;
	
	// url = /log/download/$Network/$chanQuery/$file
	char *sUri 			= lcStringCreate("%s",conn->uri);
	strtok(sUri,"!");
	char *pNetwork		= strtok(NULL,"/");
	char *pChanQuery	= strtok(NULL,"/");
	char *pFileName		= strtok(NULL,"/");
	char *sFilePath	;
	
	
	debug("pNetwork:%s\npChanQuery:%s\npFileName:%s\n",pNetwork,pChanQuery,pFileName);
		
	if(!pNetwork || !pChanQuery || !pFileName){
		tpl = lcTemplateLoad("error.html",User);
		lcTemplateAddVariableString(tpl,"Msg","your url is invalid");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);
		
		lcFree(sUri);
		
		return MG_TRUE;
	}
	
	sFilePath = lcStringCreate("%s%s/%s/%s",User->sUserDir,pNetwork,pChanQuery,pFileName);

	if(!lcFileExists(sFilePath)){
		tpl = lcTemplateLoad("error.html",User);
		lcTemplateAddVariableString(tpl,"Msg","your requested log does not exist");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);
		
		lcFree(sUri);
		lcFree(sFilePath);
		
		return MG_TRUE;		
	}
	
	// header('Content-Description: File Transfer');
	// header('Content-Disposition: attachment; filename="'.$filename.'"');
	// header('Content-Type: application/force-download'); Non-standard MIME-Type, incompatible with Samsung C3050 for example.
	int  iSize				= -1;
	char *sHeaderFilename 	= lcStringCreate("attachment; filename=\"%s_%s_%s\"",pNetwork,pChanQuery,pFileName);
	char *sFileContent		= lcFileToString(sFilePath,&iSize);
	
	debug("forcing download: %s",sHeaderFilename);
	mg_send_header(conn,"Content-Description","File Transfer");
	mg_send_header(conn,"Content-Disposition",sHeaderFilename);
	mg_printf_data(conn,sFileContent);
	
	
	lcFree(sHeaderFilename);
	lcFree(sFileContent);
	lcFree(sUri);
	lcFree(sFilePath);
	
	return MG_TRUE;
}
