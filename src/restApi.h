#ifndef LcRESTAPI
	#define LcRESTAPI
	
	#include "lcZncWebLog.h"
	#include "mongoose.h"
	#include "auth.h"
	
	#define STRCMP 0
	#define REGEX 1

	struct RestApiRule{
		char *pattern;
		char *method;
		int (*callback)(struct mg_connection *conn,struct lcUser*);
		int cmpMethod;
	};

	struct RestApi{
		struct RestApiRule **Rules;
		int iRules;
		int (*defaultCallback)(struct mg_connection *, struct lcUser *);
	};

	int RestApiInit(struct RestApi *Api,int (*defaultCallback)(struct mg_connection *,struct lcUser*));
	int RestApiDeinit(struct RestApi *Api);
	int RestApiAddRule(struct RestApi *Api, char *sPath, char *sMethod, int (*callback)(struct mg_connection *,struct lcUser *),int cmpMethod);
	int RestApiMatch(const char *string, char *pattern);
	int RestApiHandle(struct RestApi *Api,struct mg_connection *);
	
#endif
