#include "templates.h"

struct lcTemplate *lcTemplateLoad(char *sTemplate, struct lcUser *User){
    char *sFilePath          	= NULL;
    
    char    *sHTMLPath          = "HTML/";
    
    int     iTemplateSize       = -1;
    FILE    *fTemplate              ;

	struct lcTemplate *tpl 	= malloc(sizeof(struct lcTemplate));
    tpl->vars 				= malloc(sizeof(struct variableContainer));
    tpl->vars->Variables 	= malloc(sizeof(struct variable));
    tpl->vars->iVariables 	= 0;
	tpl->sText				= malloc(1);
		
    sFilePath = malloc(strlen(sHTMLPath)+strlen(CFG.WorkingDirectory)+strlen(sTemplate)+2);
    sprintf(sFilePath,"%s%s%s",CFG.WorkingDirectory,sHTMLPath,sTemplate);
    
    if (lcFileExists(sFilePath)==FALSE) {
        debug("template.c: Template %s does not exist\n\r",sFilePath);
        
        lcFree(sFilePath);
        return NULL;
    }

    // load template into a string
    iTemplateSize = lcFileSize(sFilePath);
    
    fTemplate=fopen(sFilePath,"r");
    
    if(fTemplate==NULL){
        perror("fopen");
        exit(-1);
    }
    
    tpl->sText = realloc(tpl->sText,iTemplateSize+1);
    fread (tpl->sText, 1, iTemplateSize, fTemplate);
    tpl->sText[iTemplateSize] = '\0';
    
    fclose(fTemplate);
    lcFree(sFilePath);

	lcTemplateAddVariableString(tpl,"User.Login",User->login==TRUE?"true":"false");
	lcTemplateAddVariableString(tpl,"User.Admin",User->UserType==lcADMIN?"true":"false");
	lcTemplateAddVariableString(tpl,"User.Name",User->sName);
	lcTemplateAddVariableString(tpl,"User.Root",strcmp(User->sName,CFG.sRootUser)==0?"true":"false");

	return tpl;
}
void lcTemplateClean(struct lcTemplate *tpl){
	// free vars
	int i;
	for(i=0;i<tpl->vars->iVariables;i++){
		lcFree(tpl->vars->Variables[i]->sName);
		lcFree(tpl->vars->Variables[i]->sValue);
		lcFree(tpl->vars->Variables[i]);
	}
	lcFree(tpl->vars->Variables);
	lcFree(tpl->vars);
	lcFree(tpl->sText);
	lcFree(tpl);
}

int lcTemplateAddVariableString(struct lcTemplate *tpl, char *sName,char *sValue){
	
	
	// first let's check if variable already exists
	int i;
	for(i=0;i<tpl->vars->iVariables;i++){
		if(strcmp(sName,tpl->vars->Variables[i]->sName) == 0){
			tpl->vars->Variables[i]->sValue = realloc(tpl->vars->Variables[i]->sValue,strlen(sValue)+1);
			strcpy(tpl->vars->Variables[i]->sValue,sValue);
			tpl->vars->Variables[i]->sValue[strlen(sValue)] = '\0';
			debug("template.c: reset %s to %s\n",sName,sValue);
			return 0;
		}
	}
	
	tpl->vars->Variables = realloc(tpl->vars->Variables,sizeof(struct variable)*(tpl->vars->iVariables+1));
	
	tpl->vars->Variables[tpl->vars->iVariables] = malloc(sizeof(struct variable));
	
	
	int iLen = 0;
	
	iLen = strlen(sName);
	tpl->vars->Variables[tpl->vars->iVariables]->sName  = malloc(iLen+1);
	iLen = strlen(sValue);
	tpl->vars->Variables[tpl->vars->iVariables]->sValue = malloc(iLen+1);
	
	strcpy(tpl->vars->Variables[tpl->vars->iVariables]->sName,sName);
	strcpy(tpl->vars->Variables[tpl->vars->iVariables]->sValue,sValue);
	
	tpl->vars->iVariables++;

	return 0;
}

int lcTemplateAddVariableInt(struct lcTemplate *tpl, char *sName,int iValue){
	char *sValue;
	
	asprintf(&sValue,"%i",iValue);
	
	lcTemplateAddVariableString(tpl,sName,sValue);
	free(sValue);
	
	return 0;
}



codeblock *_lcTemplateFindCodeblock(char *s){
	
	codeblock *con = malloc(sizeof(codeblock));
	
	con->If				= malloc(sizeof(lcConditionPosition*));
	con->If->iStartPos	= -1;
	con->If->iEndPos	= -1;
	con->Else			= malloc(sizeof(lcConditionPosition*));
	con->Else->iStartPos	= -1;
	con->Else->iEndPos	= -1;
	con->Endif			= malloc(sizeof(lcConditionPosition*));
	con->Endif->iStartPos	= -1;
	con->Endif->iEndPos	= -1;
	con->sText 			= s;
	con->sCondition 	= malloc(42); // rellocated later
	con->sErrorMsg		= malloc(ERRORLEN);
	con->sErrorMsg[0] 	= '\0';

	// find all if's
	lcConditionPosition	**Ifs 	= malloc(sizeof(lcConditionPosition));
	int		  			iIfs 	= 0;
	lcConditionPosition	**Endifs = malloc(sizeof(lcConditionPosition));
	int					iEndifs = 0;
	lcConditionPosition	**Elses = malloc(sizeof(lcConditionPosition));
	int	iElses 					= 0;
	int iPos 					= 0;
	int	iSearchPos 				= 0;
	int iTmp 					= 0;
		
	while((iPos=lcStrStr(s+iSearchPos,"{if "))!=-1){
		Ifs = realloc(Ifs,sizeof(lcConditionPosition *)*(iIfs+1));
		Ifs[iIfs] = malloc(sizeof(lcConditionPosition));
		Ifs[iIfs]->iStartPos = iPos+iSearchPos;
		Ifs[iIfs]->iEndPos   = iPos+iSearchPos + lcStrStr(s+iPos+iSearchPos,"}");

		iSearchPos+=iPos+1;
		iIfs++;
	}
		
	iSearchPos = 0;
	while((iPos=lcStrStr(s+iSearchPos,"{endif}"))!=-1){
		Endifs = realloc(Endifs,sizeof(lcConditionPosition *)*(iEndifs+1));
		Endifs[iEndifs] = malloc(sizeof(lcConditionPosition));
		Endifs[iEndifs]->iStartPos = iPos+iSearchPos;
		Endifs[iEndifs]->iEndPos   = iPos+iSearchPos +6;

		iSearchPos+=iPos+1;
		iEndifs++;
	}
		
	iSearchPos = 0;
	while((iPos=lcStrStr(s+iSearchPos,"{else}"))!=-1){
		Elses = realloc(Elses,sizeof(lcConditionPosition *)*(iElses+1));
		Elses[iElses] = malloc(sizeof(lcConditionPosition));
		Elses[iElses]->iStartPos = iPos+iSearchPos;
		Elses[iElses]->iEndPos   = iPos+iSearchPos +5;

		iSearchPos+=iPos+1;
		iElses++;
	}
	
	if(iIfs == 0 || iEndifs == 0){
		for(iTmp=0;iTmp<iIfs;iTmp++)
			free(Ifs[iTmp]);
		free(Ifs);
			
		for(iTmp=0;iTmp<iElses;iTmp++)
			free(Elses[iTmp]);
		free(Elses);
			
		for(iTmp=0;iTmp<iEndifs;iTmp++)
			free(Endifs[iTmp]);
		free(Endifs);
		
		return con;
	}

	con->If->iStartPos = Ifs[0]->iStartPos;
	con->If->iEndPos   = Ifs[0]->iEndPos;
	
	int iEndifsBetween = 0;
	int iIfsBetween = 0;
	int iCnt = 0;

	do{
		iEndifsBetween = 0;
		iIfsBetween = 0;
		
		for(iTmp = 0; iTmp<iIfs; iTmp++){
			if(Ifs[iTmp]->iStartPos > con->If->iStartPos && Ifs[iTmp]->iStartPos < Endifs[iCnt]->iStartPos)
				iIfsBetween++;
		}
		
		for(iTmp = 0; iTmp<iEndifs; iTmp++){
			if(Endifs[iTmp]->iStartPos < Endifs[iCnt]->iStartPos)
				iEndifsBetween++;
				
			if(iTmp == iCnt)
				break;
		}
		
		iCnt++;
	}while(iCnt < iEndifs && iEndifsBetween != iIfsBetween);
	
	if(iCnt == iEndifs && iEndifsBetween != iIfsBetween){
		con->Endif->iStartPos = -1;
		con->Endif->iEndPos   = -1;
		debug("can not find the matching endif\n");
	}else{
		con->Endif->iStartPos = Endifs[iIfsBetween]->iStartPos;
		con->Endif->iEndPos   = Endifs[iIfsBetween]->iEndPos;
	}
	
	if(iElses > 0){
		iCnt = 0;
		do{
			iEndifsBetween = 0;
			iIfsBetween = 0;
			
			for(iTmp = 0; iTmp<iIfs; iTmp++){
				if(Ifs[iTmp]->iStartPos > con->If->iStartPos && Ifs[iTmp]->iStartPos < Elses[iCnt]->iStartPos)
					iIfsBetween++;
			}
			
			for(iTmp = 0; iTmp<iEndifs; iTmp++){
				if(Endifs[iTmp]->iStartPos > con->If->iStartPos && Endifs[iTmp]->iStartPos < con->Endif->iStartPos && Endifs[iTmp]->iStartPos < Elses[iCnt]->iStartPos)
					iEndifsBetween++;
			}		

			iCnt++;		
		}while(iCnt < iElses && iIfsBetween != iEndifsBetween );
		
		if(iCnt == iElses && iIfsBetween != iEndifsBetween ){

			con->Else->iStartPos 	= -1;
			con->Else->iEndPos		= -1;
			debug("no else found\n");
		}else{	
			con->Else->iStartPos	= Elses[iCnt-1]->iStartPos;
			con->Else->iEndPos		= Elses[iCnt-1]->iEndPos;
		}
	}else{
		con->Else->iStartPos 	= -1;
		con->Else->iEndPos		= -1;
	}
	
	// {if !var1}
	//     ^---^   copy the condition into con->sCondition
	int iStartPos = lcStrStr(con->sText+con->If->iStartPos, " ");
	int iLen = con->If->iEndPos - (con->If->iStartPos+iStartPos);
	con->sCondition = realloc(con->sCondition,iLen+2);
	con->sCondition = strncpy(con->sCondition,con->sText+con->If->iStartPos+iStartPos+1,iLen-1);
	con->sCondition[iLen-1] = '\0';
	
	
	// free variables
	for(iTmp=0;iTmp<iIfs;iTmp++)
		free(Ifs[iTmp]);
	free(Ifs);
		
	for(iTmp=0;iTmp<iElses;iTmp++)
		free(Elses[iTmp]);
	free(Elses);
		
	for(iTmp=0;iTmp<iEndifs;iTmp++)
		free(Endifs[iTmp]);
	free(Endifs);
	
	return con;
}
int _lcTemplateCheckCodeblock(struct variableContainer *vars,codeblock *con){
	char *sIfCondition = con->sCondition;
	int iConditionsFound = 0;
	lcSymbol ConditionType = lcIsTrue;

	// check what to do (==,<,<=,>=,>,!,!=)
	if(lcStrStr(sIfCondition, "==") != -1){
		ConditionType = lcEquals;
		iConditionsFound++;
	}
	
	if(lcStrStr(sIfCondition, "<=") != -1){
		ConditionType = lcSmallerEqual;
		iConditionsFound++;			
	}else if(lcStrStr(sIfCondition, "<") != -1){
		ConditionType = lcSmaller;
		iConditionsFound++;	
	}
	
	if(lcStrStr(sIfCondition, ">=") != -1){
		ConditionType = lcBiggerEqual;
		iConditionsFound++;		
	}else if(lcStrStr(sIfCondition, ">") != -1){
		ConditionType = lcBigger;
		iConditionsFound++;			
	}
	
	if(lcStrStr(sIfCondition, "!=") != -1){
		ConditionType = lcNotEqual;
		iConditionsFound++;		
	}else if(lcStrStr(sIfCondition, "!") != -1){
		ConditionType = lcIsFalse;
		iConditionsFound++;		
	}
	
	if(lcStrStr(sIfCondition, "isset(") != -1){
		ConditionType = lcIsSet;
		iConditionsFound++;	
	}
	
	if(iConditionsFound > 1){
		sprintf(con->sErrorMsg,"invalid amount of conditions! '%i'",iConditionsFound);
		debug("template.c: %s\n",con->sErrorMsg);
		return -1;
	}	
	
	char *sVarName  = NULL;
	char *sValue	= NULL;
	int iLen 		= 0;
	
	// we have only the name if it's an boolean
	if(ConditionType == lcIsTrue){
		asprintf(&sValue,"true");
		asprintf(&sVarName,con->sCondition);

	}else if (ConditionType == lcIsFalse){ // remove the !
		asprintf(&sValue,"false");
		iLen = strlen(con->sCondition);
		sVarName = malloc(iLen+1);
		strncpy(sVarName,con->sCondition+1,iLen);
		sVarName[iLen]='\0';
	}else if(ConditionType == lcIsSet){
		int iStartPos = lcStrStr(sIfCondition,"(");
		int iEndPos = lcStrStr(sIfCondition,")");
		
		iStartPos++; // remove the (
		
		iLen = iEndPos-iStartPos;
		sVarName = malloc(iLen+1);
		strncpy(sVarName,con->sCondition+iStartPos,iLen);
		sVarName[iLen]='\0';
	}else{
		// with value (==,!=,<,<=,>,>=)
		char *sDelimiter;
		switch(ConditionType){
			case lcEquals: 		sDelimiter = "==";	break;
			case lcNotEqual: 	sDelimiter = "!=";	break;
			case lcBiggerEqual: sDelimiter = ">=";	break;
			case lcBigger: 		sDelimiter = ">";	break;	
			case lcSmallerEqual:sDelimiter = "<=";	break;
			case lcSmaller: 	sDelimiter = "<";	break;
			default:		
				sprintf(con->sErrorMsg,"invalid conditiontype");
				debug("template.c: %s\n",con->sErrorMsg);
				return -1;			
			break;
		}		

		if(strlen(sDelimiter) == 1){
			asprintf(&sVarName,"%s",strtok(con->sCondition,sDelimiter));
			asprintf(&sValue	,"%s",strtok(NULL,sDelimiter));						
		}else{
			int iPos = lcStrStr(con->sCondition,sDelimiter);
			con->sCondition[iPos] 	= '\0';
			con->sCondition[iPos+1] = '\0';
			asprintf(&sVarName,"%s",con->sCondition);
			asprintf(&sValue 	,"%s",con->sCondition+iPos+2);
		}
		
		if(sVarName == NULL || sValue == NULL){
			sprintf(con->sErrorMsg,"invalid value(%s) or name(%s)",sValue,sVarName);
			debug("template.c: %s\n",con->sErrorMsg);
			return -1;
		}
	}
	
	int i;
	int iFound = 0;
	for(i=0;i<vars->iVariables;i++){
		if(strcmp(sVarName,vars->Variables[i]->sName)==0){
			iFound++;
			debug("template.c: found: %s valuecmp:%s valuesrc:%s\n",sVarName,sValue,vars->Variables[i]->sValue);
			break;
		}
	}
	
	if(iFound != 1 && ConditionType != lcIsSet){
		sprintf(con->sErrorMsg,"undefined variable %s",sVarName);
		debug("template.c: %s\n",con->sErrorMsg);
		free(sVarName);
		free(sValue);
		return -1;
	}
	
	int 	iReturn = 0;
	double 	a		= 0.0
			,b		= 0.0;
	char 	*end;
	/*
	 * 	1	^= OK
	 *  0	^= NOK
	 * -1   ^= Error
	 */
	
	switch(ConditionType){
		case lcEquals: 		
			if(strcmp(vars->Variables[i]->sValue,sValue)!=0)
				iReturn = 0;
		break;
		
		case lcNotEqual: 	
			if(strcmp(vars->Variables[i]->sValue,sValue)!=0)
				iReturn = 1;
		break;
		
		case lcBiggerEqual: 
			a = strtod(vars->Variables[i]->sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",vars->Variables[i]->sValue);
				iReturn = -1;
				break;
			}
			b = strtod(sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",sValue);
				iReturn = -1;
				break;
			}
			
			if(a>=b)
				iReturn = 1;
		break;
		
		
		case lcBigger: 		
			a = strtod(vars->Variables[i]->sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",vars->Variables[i]->sValue);
				iReturn = -1;
				break;
			}
			b = strtod(sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",sValue);
				iReturn = -1;
				break;
			}
			
			if(a>b)
				iReturn = 1;
		break;	
		case lcSmallerEqual:
			a = strtod(vars->Variables[i]->sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",vars->Variables[i]->sValue);
				iReturn = -1;
				break;
			}
			b = strtod(sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",sValue);
				iReturn = -1;
				break;
			}
			
			if(a>=b)
				iReturn = 1;
		break;
		
		case lcSmaller:
			a = strtod(vars->Variables[i]->sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",vars->Variables[i]->sValue);
				iReturn = -1;
				break;
			}
			b = strtod(sValue,&end);
			if(end == vars->Variables[i]->sValue){
				sprintf(con->sErrorMsg,"value(%s) isn't a number",sValue);
				iReturn = -1;
				break;
			}
			
			if(a>=b)
				iReturn = 1;
		break;
		case lcIsFalse:
		case lcIsTrue:
			if(strcmp(vars->Variables[i]->sValue,sValue)==0)
					iReturn = 1;
		break;
		
		case lcIsSet:
			if(iFound>0)
				iReturn = 1;
		break;
	}
	debug("template.c: con->sErrorMsg: %s\n",con->sErrorMsg);
	lcFree(sVarName);
	lcFree(sValue);
	
	return iReturn;
}
int _processCodeblock(codeblock *con, struct lcTemplate *tpl ){
	

	// if there is no condition we don't need to do anything
	if(con->If->iStartPos == -1)
		return 0;
	
	int iResult						= 0;
	int iLen 						= 0;
	struct variableContainer *vars 	= tpl->vars;
		
	iResult =_lcTemplateCheckCodeblock(vars,con);
	
	switch(iResult){
		
		case -10:
			// need to use tpl->sText in this context because we need to relocate it
			
			// copy the old string into a new
			iLen = lcStrlen(tpl->sText);
			char *sPtr = malloc(iLen+1);
			strncpy(sPtr,tpl->sText,iLen);
			sPtr[iLen] = '\0';
			
			// free the old one
			free(tpl->sText);
			
			// make the start to ifposition a own string
			sPtr[con->If->iStartPos] = '\0';
			
			tpl->sText[con->If->iStartPos] = '\0';
			iLen = asprintf(&tpl->sText,"%s<p style=\"font-style:bold;color:red;\">%s</p>%s",sPtr,con->sErrorMsg,sPtr+con->Endif->iEndPos);
			tpl->sText[iLen] = '\0';
			
		break;
		
		case 1: // true
			if(con->Else->iStartPos == -1){
				// remove {endif}
				debug("template.c: Condition was true\n");
				iLen = strlen(con->sText+con->Endif->iEndPos+1);
				memmove(con->sText+con->Endif->iStartPos,con->sText+con->Endif->iEndPos+1,iLen);
				con->sText[con->Endif->iStartPos+iLen] = '\0';
				
				// remove the {if...}
				iLen = strlen(con->sText+con->If->iEndPos+1);
				memmove(con->sText+con->If->iStartPos,con->sText+con->If->iEndPos+1,iLen);
				con->sText[con->If->iStartPos+iLen] = '\0';
			}else{
				debug("template.c: Condition was true, removing elsepart\n");
				// remove {else} -> {endif}
				iLen = strlen(con->sText+con->Endif->iEndPos);
				//														+/- 1 for the }					
				memmove(con->sText+con->Else->iStartPos,con->sText+con->Endif->iEndPos+1,iLen-1);
				con->sText[con->Else->iStartPos+iLen-1] = '\0';
				// remove the {if...}
				iLen = strlen(con->sText+con->If->iEndPos);
				//														+/-1 for the }
				memmove(con->sText+con->If->iStartPos,con->sText+con->If->iEndPos+1,iLen-1);
				con->sText[con->If->iStartPos+iLen-1] = '\0';
			}
		break;
		
		case 0: // false
		case -1: // false
			// remove the true condition only
			if(con->Else->iStartPos == -1){
				debug("template.c: Condition was false\n");
				iLen = strlen(con->sText+con->Endif->iEndPos+1);
				memmove(con->sText+con->If->iStartPos,con->sText+con->Endif->iEndPos+1,iLen);

				con->sText[con->If->iStartPos+iLen] = '\0';

			}else{ // remove true condition and endif
				debug("template.c: Condition was false, removing ifpart\n");
				// remove {endif}
				iLen = strlen(con->sText+con->Endif->iEndPos+1);

				if(iLen == 0){
					con->sText[con->Endif->iStartPos] = '\0';
				}else{
					memmove(con->sText+con->Endif->iStartPos,con->sText+con->Endif->iEndPos+1,iLen);
					con->sText[con->Endif->iStartPos+iLen] = '\0';
				}
				// remove {if ...} ...{else}

				iLen = strlen(con->sText+con->Else->iEndPos+1);
				
				memmove(con->sText+con->If->iStartPos,con->sText+con->Else->iEndPos+1,iLen);
				con->sText[con->If->iStartPos+iLen] = '\0';
			}
		break;
	
	}
	
	return 0;
}

void _lcTemplateCleanCodeblock(codeblock *con){
	free(con->If);
	free(con->Else);
	free(con->Endif);
	free(con->sCondition);
	free(con->sErrorMsg);
	free(con);	
}

int _lcTemplateProcess(struct lcTemplate *tpl){
	codeblock *con;
	int iLoop = -1;
	int iCnt  =  1;

	// replace all includes first
	while(_lcTemplateProcessIncludes(tpl)){
		debug("replaced include %i\n",iCnt);
		iCnt++;		
	}

	do{
		con = _lcTemplateFindCodeblock(tpl->sText);
		_processCodeblock(con,tpl);
		iLoop = con->If->iStartPos;

		_lcTemplateCleanCodeblock(con);
		
	}while(iLoop != -1);
	
	_lcTemplateProcessIncludes(tpl);
	_lcTemplateReplaceVars(tpl);
	
	return 0;
}
int _lcTemplateReplaceVars(struct lcTemplate *tpl){
	int i;
	char *sVarName;
	for(i = 0;i<tpl->vars->iVariables;i++){
		asprintf(&sVarName,"{$%s}",tpl->vars->Variables[i]->sName);
		//~ debug("trying to replace %s\n",sVarName);
		lcStrReplace(tpl->sText,sVarName,tpl->vars->Variables[i]->sValue);
		
		free(sVarName);
	}
	
	return 0;
	
}

int lcTemplateSend(struct mg_connection *conn,struct lcTemplate *tpl){
    
    //mg_send_status(conn, 200);
    mg_send_header(conn, "Content-type", "text/html;charset=utf-8");
	
	_lcTemplateProcess(tpl);

    mg_printf_data(conn,tpl->sText);
    return 1;
}

/*
 *  replace     {include inc.html} with the filecontent
 *iIncStartPos -^        ^       ^
 *               iIncFile^       | 
 *                    iIncEndPos-- 
*/
int _lcTemplateProcessIncludes(struct lcTemplate *tpl){
	
	char *sFileName;
	char *sFileWithPath;
	char *sTmpText;
	
	int iIncStartPos = lcStrStr(tpl->sText,"{include ");
	if( iIncStartPos == -1 )
		return 0;
	
	int iIncEndPos = lcStrStr(tpl->sText + iIncStartPos,"}") +iIncStartPos;
	int iIncFile		= -1;
	int iFileNameLen	= -1;
	
	if(iIncEndPos == -1){
		return 0;
	}
	
	// get the includefile		+ 9  ^= strlen("{include ");
	iIncFile = iIncStartPos + 9;
	
	// filename length = (iIncEndPos - iIncFile)
	iFileNameLen =  iIncEndPos - iIncFile;
		
	sFileName = malloc( iFileNameLen+2);
	//~ debug("iIncStartPos: %i\niIncEndPos:%i\niFileNameLen:%i\n",iIncStartPos,iIncEndPos,iFileNameLen);
		
	strncpy(sFileName,tpl->sText+iIncFile,iFileNameLen);
	sFileName[iFileNameLen] = '\0';
	//~ debug("includefile :%s\n",sFileName);
	
	sFileWithPath = lcStringCreate("%sHTML/%s",CFG.WorkingDirectory,sFileName);

	//~ debug("sFileWithPath: %s\n",sFileWithPath);
	
	
	// replace the {include xx.xx} with an errormessage
	if(lcFileExists(sFileWithPath)==FALSE){
		// split the text in "two" strings to add them via "%s <error> %s"
		tpl->sText[iIncStartPos] = '\0';
		//																						   														+1 for the }
		sTmpText = lcStringCreate("%s <p style=\"color:red;font-weight:bold;\">can not find the file %s</p>%s",tpl->sText,sFileName,tpl->sText+iIncEndPos+1);
		
		free(tpl->sText);
		tpl->sText = sTmpText;
		
		// reset the positionvariabes.. the positions are all gone! >:
		iIncStartPos 	= -1;
		iIncEndPos 		= -1;
		iIncFile		= -1;		
	}else{
		int iFileSize = lcFileSize(sFileWithPath);
		char *sIncFileContent = malloc(iFileSize+1);
		FILE *fFile	= fopen(sFileWithPath,"r+");
		
		if(!fFile){
			debug("file %s was (null)\n",sFileWithPath);
			free(sIncFileContent);
			free(sFileWithPath);
			free(sFileName);
			return 0;
		}
		
		fread (sIncFileContent, 1, iFileSize, fFile);
		fclose(fFile);
		sIncFileContent[iFileSize] = '\0';
		//~ debug("sIncFileContent: %s\n",sIncFileContent),
		
		// split the text in "two" strings to add them via "%s <filecontent> %s"
		tpl->sText[iIncStartPos] = '\0';
		//																					 +1 for the }
		sTmpText = lcStringCreate("%s%s%s",tpl->sText,sIncFileContent,tpl->sText+iIncEndPos+1);
		
		free(sIncFileContent);
		free(tpl->sText);
		tpl->sText = sTmpText;
	}
	
	free(sFileWithPath);
	free(sFileName);
	return 1;
}
