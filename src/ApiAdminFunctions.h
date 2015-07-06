#ifndef LcADMINFUNCTIONS
	#define LcADMINFUNCTIONS
	
	int ApiAdminUserAddGET(struct mg_connection *conn, struct lcUser *User);
	int ApiAdminShowIndex(struct mg_connection *conn, struct lcUser *User);
	lcUser **_ApiAdminListUsers(int *iUsers);
	void _ApiAdminFreeUserList(lcUser **list,int iUsers);
	int ApiAdminUsers(struct mg_connection *conn, struct lcUser *User);
	int ApiAdminUserDelete(struct mg_connection *conn, struct lcUser *User);
#endif
