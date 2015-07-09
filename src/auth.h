#ifndef LcAUTH
	#define LcAUTH

	#include "lcZncWebLog.h"
	#include "mongoose.h"
	#include "strings.h"
	#include "crypt/base64.h"
	#include "crypt/rot13.h"
	#include "crypt/md5.h"


	#define ADMINONLY		if(User->UserType != lcADMIN){\
		struct lcTemplate *tpl=lcTemplateLoad("error.html",User);\
		lcTemplateAddVariableString(tpl,"Msg","you have no access to this page");\
		lcTemplateSend(conn,tpl);\
		lcTemplateClean(tpl);\
		return MG_TRUE;\
	}
	#define LOGINONLY		if(User->login != TRUE){\
		struct lcTemplate *tpl=lcTemplateLoad("error.html",User);\
		lcTemplateAddVariableString(tpl,"Msg","you have to login");\
		lcTemplateSend(conn,tpl);\
		lcTemplateClean(tpl);\
		return MG_TRUE;\
	}
	#define NOROOTFUNC		if(strcmp(User->sName,CFG.sRootUser)==0){\
		struct lcTemplate *tpl=lcTemplateLoad("error.html",User);\
		lcTemplateAddVariableString(tpl,"Msg","the rootuser isn't allowed here");\
		lcTemplateSend(conn,tpl);\
		lcTemplateClean(tpl);\
		return MG_TRUE;\
	}

	typedef struct lcUser{
		char        *sName;
		lcUserType  UserType;
		_Bool      login;
		char 		*sUserDir;
	}lcUser;

	char *lcGenCookie( char *sName, char *sPass);
	_Bool lcAuthCheckCookie(struct mg_connection *conn,lcUser *User);
	_Bool lcAuthAddUser(char *sName, char *sPass, _Bool Admin, char *sError);
	_Bool lcAuthCheckLoginData(char *sName,char *sPass, char *sPassMd5);
	_Bool lcAuthChangePassword(char *sUser, char *sOldPasswd, char *sNewPasswd, _Bool bVerifyOldpw, char *sErrorMsg);
	_Bool lcAuthUserDelete(char *sName, char *sErrorMsg);
	_Bool lcAuthUserChangeUsertype(char *sUser,lcUserType userType, char *sErrorMsg);
#endif
