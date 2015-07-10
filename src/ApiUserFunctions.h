#ifndef LcUSERFUNCTIONS
	#define LcUSERFUNCTIONS
	
	int ApiUserListNetworks(struct mg_connection *conn, struct lcUser *User);	
	int ApiUserShowNetwork(struct mg_connection *conn, struct lcUser *User);
	int ApiUserShowLog(struct mg_connection *conn, struct lcUser *User);
	int ApiUserLogout(struct mg_connection *conn, struct lcUser *User);
	int ApiUserLogin(struct mg_connection *conn, struct lcUser *User);
	int ApiUserChangePassword(struct mg_connection *conn, struct lcUser *User);
	int ApiUserSendLogJson(struct mg_connection *conn, struct lcUser *User);
	int ApiUserDownloadLog(struct mg_connection *conn, struct lcUser *User);
	
#endif

