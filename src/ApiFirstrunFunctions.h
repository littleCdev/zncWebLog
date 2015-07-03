#ifndef LcRISTRUNFUNCTIONS
	#define LcRISTRUNFUNCTIONS
	
	#include "lcZncWebLog.h"
	#include "mongoose.h"
	#include "auth.h"
	#include "templates.h"
	#include "files.h"
	
	
	int ApiFirstRunDefault(struct mg_connection *conn, struct lcUser *User);
	int ApiFirstRunAddRoot(struct mg_connection *conn, struct lcUser *User);

#endif

