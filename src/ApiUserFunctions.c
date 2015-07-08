#include "lcZncWebLog.h"
#include "mongoose.h"
#include "auth.h"
#include "ApiUserFunctions.h"
#include "templates.h"
#include "files.h"

int ApiUserListNetworks(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	NOROOTFUNC

	DIR *dp;
	struct dirent *ep;     
	struct lcTemplate *tpl;
	char *sNetworkPath;
	char *sNetworkList;
	sNetworkPath = lcStringCreate("%s",User->sUserDir);
	
	tpl = lcTemplateLoad("listNetworks.html",User);

	debug("ApiUserListNetworks: sNetworkpath: %s\n",sNetworkPath);
	
	sNetworkList = lcStringCreate("<ul class=\"list-group\">\n");
	
	dp = opendir (sNetworkPath);
	if (dp != NULL){
		while ((ep = readdir(dp))){
			if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0)
				continue;
				
			char *sTmp = lcStringCreate("%s%s/",sNetworkPath,ep->d_name);
			
			if(lcFileIsDir(sTmp,TRUE) == TRUE){
				debug("ApiUserListNetworks: found: %s/%s\n",User->sName,ep->d_name);
				lcStringAdd(sNetworkList,"<li class=\"list-group-item\"><a href=\"/network/%s\">%s</a></li>\n",ep->d_name,ep->d_name);			
			}
			free(sTmp);
		}
		(void) closedir (dp);
	}
	
	lcStringAdd(sNetworkList,"</ul>");
	
	lcTemplateAddVariableString(tpl,"sNetworkList",sNetworkList);
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	free(sNetworkList);
	free(sNetworkPath);
	
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
	DIR *dpSubDir;
	struct dirent *epSubDir;
	char *sHtmlList = lcStringCreate(" ");
	char *sEncodeBuffer;
	
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
		while ((ep = readdir(dp))){
			
			if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0)
				continue;
			
			char *sCssClassName = lcStringCreate("%s",ep->d_name);
			lcStrReplace(sCssClassName,"#","_");
			lcStrReplace(sCssClassName,".","_");
			
			lcStringAdd(sHtmlList,
				"<div class=\"panel-heading \">\n"
				"<h3 class=\"panel-title slideup\" data=\"%s\"> <span class=\"glyphicon glyphicon-folder-close %s\" aria-hidden=\"true\"></span>%s</h3>\n"
				"</div>\n"
				"<div class=\"slideup %s\" data=\"%s\">\n",sCssClassName,sCssClassName,ep->d_name,sCssClassName,sCssClassName);
				
			char *sUserOrChanPath = lcStringCreate("%s%s/",sNetworkPath,ep->d_name);
			if(lcFileIsDir(sUserOrChanPath,TRUE) == TRUE){
				
				dpSubDir = opendir(sUserOrChanPath);
				debug("open subdir: %s\n",sUserOrChanPath);
				
				sEncodeBuffer = lcStringCreate("%s",ep->d_name);
				lcStrReplace(sEncodeBuffer,"#","%%%%23");
				
				if(dpSubDir!=NULL){
					while((epSubDir=readdir(dpSubDir))){
						debug("+++%s\n",epSubDir->d_name);
						if(strcmp(epSubDir->d_name,".")==0 || strcmp(epSubDir->d_name,"..")==0)
							continue;

						debug("found Quer/Chane: %s\n",ep->d_name);
						lcStringAdd(sHtmlList,
						"<div class=\"panel-body\">\n"
							"<a href=\"#\" class=\"text-muted\">\n"
								"<span class=\"glyphicon glyphicon-download-alt\" aria-hidden=\"true\"></span>\n"
							"</a>\n"
							"<a href=\"/log/%s/%s/%s/\">%s%s</a>\n"
						"</div>\n",pNetwork,sEncodeBuffer,epSubDir->d_name,ep->d_name,epSubDir->d_name);
					}
					closedir(dpSubDir);
				}else{
					debug("opendir was null: %s\n",sUserOrChanPath);
				}
				
				free(sEncodeBuffer);
			}
			free(sUserOrChanPath);
			free(sCssClassName);
			lcStringAdd(sHtmlList,"</div>\n");
		}
		(void) closedir (dp);
	}
	
	lcTemplateAddVariableString(tpl,"sNetworkName",pNetwork);
	lcTemplateAddVariableString(tpl,"UsersAndChans",sHtmlList);

	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	free(sNetworkPath);
	free(sHtmlList);
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
	int	 iEndReached	= 42;
	int i;
	
	char sStartLine[10] = {0};
	mg_get_var(conn, "Start", sStartLine, 9);
	int iStartLine = atoi(sStartLine);
	iStartLine += CFG.iLogLines+1;
	
	debug("iStartLine: %i\n",iStartLine);
	
	debug("ApiUserShowLog: %s\n",conn->uri);
	syslog(LOG_INFO,"ApiUserShowLog: %s\n",conn->uri);
	
	sFilePath = lcStringCreate("%s%s/%s/%s",User->sUserDir,pNetwork,pChanQuery,pFileName);

	char *sLog = lcFileReadLines(sFilePath,iStartLine*-1,CFG.iLogLines,&iEndReached);
	
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
		
		if(lcAuthChangePassword(User->sName,sOldPw,sNewPw,sErrorMsg) == FALSE){
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
