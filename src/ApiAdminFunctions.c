#include "lcZncWebLog.h"
#include "mongoose.h"
#include "auth.h"
#include "ApiAdminFunctions.h"
#include "templates.h"
#include "files.h"

lcUser **_ApiAdminListUsers(int *iUsers){
	*iUsers = 0;
	lcUser **ret = malloc(sizeof(lcUser *));
	int iFileSize = 0;
	char *sPasswdFile = lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sFileContent = lcFileToString(sPasswdFile,&iFileSize);
	char *pUser;
	
	if(iFileSize == 0){
		free(sPasswdFile);
		free(sFileContent);
		return ret;
	}
		
	pUser = strtok(sFileContent,"\r\n");
		
	do{
		char *sEntrie = lcStringCreate("%s",pUser);
		// end of the name
		sEntrie[lcStrrStr(sEntrie,"|")] = '\0';

		ret = realloc(ret,((*iUsers)+1)*sizeof(lcUser));

		ret[*iUsers] = malloc(sizeof(lcUser));
		ret[*iUsers]->sName = lcStringCreate("%s",sEntrie+2);
		ret[*iUsers]->UserType = sEntrie[0] == '1' ? lcADMIN:lcUSER;
		
		(*iUsers)++;
		
		free(sEntrie);
		pUser = strtok(NULL,"\r\n");
	}while(pUser != NULL);


	free(sPasswdFile);
	free(sFileContent);
	return ret;
}

void _ApiAdminFreeUserList(lcUser **list,int iUsers){
	int i = 0;
	
	for(i=0;i<iUsers;i++){
		free(list[i]->sName);
		free(list[i]);
	}
	
	free(list);
}

int ApiAdminUsers(struct mg_connection *conn, struct lcUser *User){
	debug("ApiAdminUsers: %s %s",conn->request_method,conn->uri);
	
	LOGINONLY
	ADMINONLY
	
	int		iUsers	= 0;
	lcUser **Users 	= _ApiAdminListUsers(&iUsers);
	int 	i		= 0;
	
	char *sHtmlUserList = lcStringCreate("");
	
		lcStringAdd(sHtmlUserList,"<tr><td>Nickname</td><td>Admin</td><td>Delete</td>");
									
	for(i=0;i<iUsers;i++){
		lcStringAdd(sHtmlUserList,"<tr>"
								"<td><a href=\"/admin/Users!Edit!%s\" title=\"Edit\">%s <span class=\"glyphicon glyphicon-pencil\" aria-hidden=\"true\"></a></td>"
								"<td><span class=\"glyphicon glyphicon-%s\" aria-hidden=\"true\"></span></td>"
								"<td><a href=\"/admin/Users!Delete!%s\"><span class=\"glyphicon glyphicon-trash\" aria-hidden=\"true\"></a></td>"
								"</tr>",
					Users[i]->sName,Users[i]->sName,Users[i]->UserType==lcADMIN?"ok":"remove",Users[i]->sName);

	}
	
	
	struct lcTemplate *tpl = lcTemplateLoad("admin/users.html",User);
	lcTemplateAddVariableString(tpl,"sUserTable",sHtmlUserList);
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	_ApiAdminFreeUserList(Users,iUsers);
	free(sHtmlUserList);
	
	return MG_TRUE;
}

int ApiAdminUserDelete(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	ADMINONLY
	
	int iNamePos = lcStrrStr((char *)conn->uri,"!");
	struct lcTemplate *tpl;
	
	
	if(iNamePos == -1){
		tpl = lcTemplateLoad("error.html",User);
		lcTemplateAddVariableString(tpl,"sErrorMsg","i can't find the user");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);
		
		return MG_TRUE;
	}
	
	char *sName = lcStringCreate("%s",conn->uri+iNamePos+1);
	
	// check if name exists in passwordfile
	int i;
	char *sPasswdFile	= lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sErrorMsg 	= malloc(ERRORLEN);
	char *sFileContent  = lcFileToString(sPasswdFile,&i);
	tpl = lcTemplateLoad("admin/deleteUser.html",User);
		
	iNamePos = lcStrStr(sFileContent,sName);
	if(iNamePos==-1){
		lcTemplateAddVariableString(tpl,"sMsg","the user does not exists");
	}else{
		lcTemplateAddVariableString(tpl,"sUsername",sName);
	}

	// delete user from passwdfile if POST
	if(iNamePos != -1 && strcmp(conn->request_method,"POST")==0){
		
		if(!lcAuthUserDelete(sName,sErrorMsg)){
			lcTemplateAddVariableString(tpl,"sMsg",sErrorMsg);
		}else{
			lcTemplateAddVariableString(tpl,"sMsg","Deleted %s",sName);			
		}
	}

	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	free(sPasswdFile);
	free(sFileContent);	
	free(sErrorMsg);
	free(sName);

	return MG_TRUE;
}

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

int ApiAdminUserEdit(struct mg_connection *conn, struct lcUser *User){
	LOGINONLY
	ADMINONLY
	
	int iNamePos = lcStrrStr((char *)conn->uri,"!");
	struct lcTemplate *tpl;
	
	
	if(iNamePos == -1){
		tpl = lcTemplateLoad("error.html",User);
		lcTemplateAddVariableString(tpl,"sErrorMsg","i can't find the user");
		lcTemplateSend(conn,tpl);
		lcTemplateClean(tpl);
		
		return MG_TRUE;
	}
	
	char *sName = lcStringCreate("%s",conn->uri+iNamePos+1);
	
	// check if name exists in passwordfile
	int i;
	char *sPasswdFile	= lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
	char *sErrorMsg 	= malloc(ERRORLEN);
	char *sFileContent  = lcFileToString(sPasswdFile,&i);
	tpl = lcTemplateLoad("admin/editUser.html",User);
		
	iNamePos = lcStrStr(sFileContent,sName);
	if(iNamePos==-1){
		lcTemplateAddVariableString(tpl,"sMsg","the user does not exists");
	}else{
		lcTemplateAddVariableString(tpl,"sUsername",sName);
		if(sFileContent[iNamePos-2] == '1')
			lcTemplateAddVariableString(tpl,"isAdmin","true");
		debug("isAdmin:%c",sFileContent[iNamePos-2]);
	}

	// update if POST
	if(iNamePos != -1 && strcmp(conn->request_method,"POST")==0){
		char *sPass 	= malloc(PASSMAXLEN+1);
		char *sAdmin  	= malloc(10);
		char *sErrorMsg = malloc(ERRORLEN);
		_Bool bStatus = TRUE;
		
		mg_get_var(conn, "NewPasswd", sPass, PASSMAXLEN+1);
		mg_get_var(conn, "isAdmin", sAdmin, 10);
		
		debug("isAdmin:%s",sAdmin);
		debug("sPass:%s",sPass);
		
		if(strlen(sAdmin) != 0){
			bStatus = lcAuthUserChangeUsertype(sName,lcADMIN,sErrorMsg);
		}else{
			bStatus = lcAuthUserChangeUsertype(sName,lcUSER,sErrorMsg);
		}
		
		if(!bStatus){
			lcTemplateAddVariableString(tpl,"sMsg",sErrorMsg);
		}else{
			if(strlen(sPass) != 0){
				bStatus = lcAuthChangePassword(sName,"",sPass,FALSE,sErrorMsg);
			}
		}
		
		if(!bStatus){
			lcTemplateAddVariableString(tpl,"sMsg",sErrorMsg);
		}else{
			lcTemplateAddVariableString(tpl,"sMsg","updated %s",sName);
			
			if(strlen(sAdmin) != 0){
				lcTemplateAddVariableString(tpl,"isAdmin","true");
			}else{
				lcTemplateAddVariableString(tpl,"isAdmin","");
			}
		}
		
		lcFree(sPass);
		lcFree(sAdmin);
		lcFree(sErrorMsg);
	}

	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	lcFree(sPasswdFile);
	lcFree(sFileContent);	
	lcFree(sErrorMsg);
	lcFree(sName);

	return MG_TRUE;
}

int ApiAdminShowIndex(struct mg_connection *conn, struct lcUser *User){

	struct lcTemplate *tpl;
	tpl = lcTemplateLoad("admin/index.html",User);
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	
	return MG_TRUE;
}
