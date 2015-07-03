#ifndef LcTEMPLATES
	#define LcTEMPLATES

	#include "lcZncWebLog.h"
	#include "mongoose.h"
	#include "auth.h"
	#include "files.h"
	#include "strings.h"

	typedef enum{
		lcIf,
		lcIfClose,
		lcElse,
		lcEndif
	}lcConditionPositions;

	typedef enum{
		lcNotEqual,
		lcEquals,
		lcSmaller,
		lcSmallerEqual,
		lcBigger,
		lcBiggerEqual,
		lcIsTrue,
		lcIsFalse,
		lcIsSet
	}lcSymbol;

	typedef struct{
		int iStartPos;
		int iEndPos;	
	}lcConditionPosition;

	typedef struct{
		char *sText;
		char *sCondition;
		char *sErrorMsg;
		lcConditionPosition *If;
		lcConditionPosition *Else;
		lcConditionPosition *Endif;
	}codeblock;

	struct variable{
		char *sName;
		char *sValue;
	};

	struct variableContainer{
		struct variable **Variables;
		int iVariables;
	};

	struct lcTemplate{
		char *sText;
		struct variableContainer *vars;
	};
	/*

	struct lcTemplate{
		char *sText = templatecontent 			<--- free
		struct variableContainer *con{			<--- free
			variable **variables = [			<--- free
				0 = struct variable {			<--- free
					char *sName  = "bla";		<--- free
					char *sVlaue = "false";		<--- free
				}
				1 = struct variable {			<--- free
					char *sName  = "ficken?";	<--- free
					char *sValue = "nope";		<--- free
				}
			];
			int iVariables = 2 // count of **variables
		}
	}
	*/	

	int lcTemplateSend(struct mg_connection *conn,struct lcTemplate *tpl);
	int _lcTemplateProcess(struct lcTemplate *tpl);
	int _processCodeblock(codeblock *con, struct lcTemplate *tpl );
	int _lcTemplateCheckCodeblock(struct variableContainer *vars,codeblock *con);
	codeblock *_lcTemplateFindCodeblock(char *s);
	int lcTemplateAddVariableString(struct lcTemplate *tpl, char *sName,char *sValue);
	int lcTemplateAddVariableInt(struct lcTemplate *tpl, char *sName,int iValue);
	int findNext(lcConditionPositions what,codeblock *con,int iStartPos);
	void _lcTemplateCleanCodeblock(codeblock *con);
	void lcTemplateClean(struct lcTemplate *tpl);
	struct lcTemplate *lcTemplateLoad(char *sTemplate, struct lcUser *User);
	int _lcTemplateReplaceVars(struct lcTemplate *tpl);
	int _lcTemplateProcessIncludes(struct lcTemplate *tpl);
#endif
