#include "lcZncWebLog.h"
#include "mongoose.h"
#include "auth.h"
#include "ApiAdminFunctions.h"
#include "templates.h"
#include "files.h"

int ApiAdminUserAddGET(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	ADMINONLY
	
	char *sUsersList	= malloc(1);	sUsersList[0] = '\0';
	DIR *dp;
	struct dirent *ep;
	struct lcTemplate *tpl;

	tpl = lcTemplateLoad("admin/addUser.html",User);

	// try to add a user when the request is via POST
	if(strcmp(conn->request_method,"POST")==0){
		debug("trying to add a new user\n");
		char sName[NAMEMAXLEN+1] = {0},
			 sPass[PASSMAXLEN+1] = {0},
			 sAdmin[10]={0};
		char sErrorMsg[ERRORLEN] = {0};
		
		mg_get_var(conn, "Nickname", sName, NAMEMAXLEN+1);
		mg_get_var(conn, "Passwd", sPass, PASSMAXLEN+1);
		mg_get_var(conn, "usertype", sAdmin, 10);
		
		if(lcAuthAddUser(sName,sPass,(strcmp(sAdmin,"Yes")==0?TRUE:FALSE),sErrorMsg) == FALSE){
			lcTemplateAddVariableString(tpl,"sErrorMsg",sErrorMsg);
			debug("sErrorMsg: %s\n",sErrorMsg);
		}else{
			char *sTmp;
			asprintf(&sTmp,"added: %s!",sName);
			lcTemplateAddVariableString(tpl,"sErrorMsg",sTmp);
			debug("sErrorMsg: %s\n",sTmp);
			free(sTmp);
		}
	}
	
	dp = opendir (CFG.sZncUserDir);
	
	debug("opening %s\n",CFG.sZncUserDir);
	
	if (dp != NULL){
		while ((ep = readdir(dp))){
			if(strcmp(ep->d_name,".")==0 || strcmp(ep->d_name,"..")==0)
							continue;

			char *sTmpPath = lcStringCreate("%s%s/",CFG.sZncUserDir,ep->d_name);
			
			if(lcFileIsDir(sTmpPath,TRUE) == TRUE){
				lcStringAdd(sUsersList,"<option>%s</option>\n",ep->d_name);			
			}
			
			free(sTmpPath);
		}
		(void) closedir (dp);
	}
	
	lcTemplateAddVariableString(tpl,"UsersList",sUsersList);
	
	free(sUsersList);
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
    
    return MG_TRUE;
}

int ApiAdminShowIndex(struct mg_connection *conn, struct lcUser *User){

	struct lcTemplate *tpl;
	tpl = lcTemplateLoad("admin/index.html",User);
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	return MG_TRUE;
}
