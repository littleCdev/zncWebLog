#define LC_GLOBALS_MAIN
#include "main.h"

struct RestApi  Api;
lcConfig		*Ini;
_Bool			bHttpFinished;

void  INThandler(int sig){
	CFG.bRunning = FALSE;
    syslog(LOG_INFO,"setting bRunning = FALSE\n");
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
   
    switch (ev) {
    case MG_AUTH: return MG_TRUE;
    case MG_REQUEST:
		// handle api-requst
        return RestApiHandle(&Api,conn);
    default: return MG_FALSE;
  }
}

void *MongoseServe(void *server) {
	while(CFG.bRunning)
		mg_poll_server((struct mg_server *) server, 1000);
  
	bHttpFinished = TRUE;
	syslog(LOG_INFO,"httpd stopped");	
	return NULL;
}

void startDeamon(){
	pid_t pid;
	FILE *fp;
	
	pid = fork();
	if( pid < 0 ){
		fprintf(stderr,"failed to fork\n");
		exit(EXIT_FAILURE);
	}
	
	// kill myself if pid is started
	if( pid > 0 ){
		
		fp=fopen("/var/run/zwl.pid","w+");
		if(!fp){
			fprintf(stderr,"failed to open pidfile\n");
			exit(EXIT_FAILURE);
		}
		fprintf(fp,"%d",pid);
		fclose(fp);
		
		fprintf(stdout,"started deamon...\n");
		exit(EXIT_SUCCESS);
	}
	
	if( setsid() < 0 ){
		exit(EXIT_FAILURE);
	}
/*
	// change working dir
	if(chdir("/") < 0){
		exit(EXIT_FAILURE);
	}
*/		
	// close streams 
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);	
}

void loadIni(){
	if(lcFileExists(CFGFILE)==FALSE){
		syslog(LOG_CRIT,"can not find logfile %s",CFGFILE);
		exit(1);
	}
	
	Ini = lcConfigLoad(CFGFILE);
	CFG.bHttp	 	= strcmp(lcConfigGetString(Ini,"EnableHTTP"),"1")==0?TRUE:FALSE;
	CFG.sHttpPort 	= lcConfigGetString(Ini,"PortHTTP");
	CFG.bHttps	 	= strcmp(lcConfigGetString(Ini,"EnableHTTPS"),"1")==0?TRUE:FALSE;
	CFG.sHttpsPort 	= lcConfigGetString(Ini,"PortHTTPS");
    CFG.sCertFile 	= lcConfigGetString(Ini,"CertFile");
	CFG.sRootUser	= lcConfigGetString(Ini,"Rootuser");
    CFG.sPasswdFile = lcConfigGetString(Ini,"PasswdFile");
    CFG.iLogLines	= strtol(lcConfigGetString(Ini,"logLines"),NULL,0);
    CFG.sZncUserDir	= lcConfigGetString(Ini,"zncUserDir");
    
	char *sLoglevel = lcConfigGetString(Ini,"loglevel");
	if(sLoglevel == NULL){
		printf("loglevel not set!\n");
		exit(1);
	}
	
	if(strcmp("debug",sLoglevel)==0)		CFG.iLogVerbosity = LOG_DEBUG;
	else if(strcmp("info",sLoglevel)==0)	CFG.iLogVerbosity = LOG_INFO;
	else if(strcmp("warn",sLoglevel)==0)	CFG.iLogVerbosity = LOG_WARNING;
	else if(strcmp("error",sLoglevel)==0)	CFG.iLogVerbosity = LOG_CRIT;
	else{
		fprintf(stderr,"invalid loglevel \"%s\"!\n",sLoglevel);
		exit(1);
	}
	setlogmask(LOG_UPTO (CFG.iLogVerbosity));
	
}

/* Rest-Api callback-functions */
int DefaultFunction(struct mg_connection *conn, struct lcUser *User){
	debug("defaultcalltback: %s %s\n",conn->request_method,conn->uri);
	
	struct lcTemplate *tpl = lcTemplateLoad("error.html",User);
	lcTemplateAddVariableString(tpl,"Msg","the page you requested does not exist");
	lcTemplateSend(conn,tpl);
	lcTemplateClean(tpl);
	return MG_TRUE;
}

int ApiUserSendCssJs(struct mg_connection *conn, struct lcUser *User){
	int iFirstpos = lcStrrStr((char *)conn->uri,"/");
	char *sFile = lcStringCreate("%sHTML/%s",CFG.WorkingDirectory,(char *)conn->uri+lcStrrStr((char *)conn->uri+(iFirstpos-1),"/"));
	mg_send_file(conn,sFile,NULL);
	debug("filename: %s\n",sFile);
	free(sFile);
	return MG_MORE;		
}

int ApiFavIco(struct mg_connection *conn, struct lcUser *User){
	return MG_FALSE;
}


int main(void) {
    struct mg_server *server;
	
	bHttpFinished = FALSE;
	
	// ctrl+c => exit
	signal(SIGINT, INThandler);
	signal(SIGUSR2, INThandler);
	
	loadIni();
	
	startDeamon();

    openlog( "zncWebLog", LOG_CONS|LOG_PID|LOG_NDELAY, LOG_LOCAL0);
	syslog(LOG_INFO,"started deamon");
	
    // Create and configure the server
	server = mg_create_server(NULL, ev_handler);
	mg_set_option(server, "listening_port", CFG.sHttpPort);


	// on first run you have to add a rootuser first
	char *sPasswdFile = lcStringCreate("%s%s",CFG.WorkingDirectory,CFG.sPasswdFile);
    if(lcFileExists(sPasswdFile) == FALSE){
		CFG.bFirstRun = TRUE;
		// create restApi
		RestApiInit(&Api,&ApiFirstRunDefault);
		RestApiAddRule(&Api,"^/css/[A-Za-z0-9.-]+$"		,"GET"	,&ApiUserSendCssJs		,REGEX);
		RestApiAddRule(&Api,"^/fonts/[A-Za-z0-9.-]+$"	,"GET"	,&ApiUserSendCssJs		,REGEX);
		RestApiAddRule(&Api,"^/js/[A-Za-z0-9.-]+$"		,"GET"	,&ApiUserSendCssJs		,REGEX);
		RestApiAddRule(&Api,"/addRoot/"					,"POST"	,&ApiFirstRunAddRoot	,STRCMP);

		while(CFG.bFirstRun && CFG.bRunning){
			mg_poll_server(server, 1000);
		}
		
		// reload the config if CFG.iFirstRun is set to 0 when the user was added
		lcConfigFree(Ini);
		loadIni();
		
		debug("reloaded config\n");
		
	}else{
		CFG.bFirstRun = FALSE;
	}

	free(sPasswdFile);
	
	if(!CFG.bFirstRun){
		// create restApi
		RestApiInit(&Api,&DefaultFunction);
		// add Rules	
		RestApiAddRule(&Api,"/"							,"GET"	,&ApiUserLogin			,STRCMP);
		RestApiAddRule(&Api,"/login/"					,"POST"	,&ApiUserLogin			,STRCMP);
		RestApiAddRule(&Api,"/login/"					,"GET"	,&ApiUserLogin			,STRCMP);
		RestApiAddRule(&Api,"/logout/"					,"GET"	,&ApiUserLogout			,STRCMP);
		RestApiAddRule(&Api,"/changepw/"				,"POST"	,&ApiUserChangePassword	,STRCMP);
		RestApiAddRule(&Api,"/changepw/"				,"GET"	,&ApiUserChangePassword	,STRCMP);
		RestApiAddRule(&Api,"/networks/"				,"GET"	,&ApiUserListNetworks	,STRCMP);
		RestApiAddRule(&Api,"^/network/[A-Za-z0-9]+$"	,"GET"	,&ApiUserShowNetwork	,REGEX);
		//							networkname	 	chan/query		   file
		RestApiAddRule(&Api,"^/log/[A-Za-z0-9-_\\.]+/[#A-Za-z0-9_\\.-]+/[A-Za-z0-9_-]+\\.log/$"	,"GET"	,&ApiUserShowLog	,REGEX);
		RestApiAddRule(&Api,"^/log/[A-Za-z0-9-_\\.]+/[#A-Za-z0-9_\\.-]+/[A-Za-z0-9_-]+\\.log/$"	,"POST"	,&ApiUserSendLogJson,REGEX);

		RestApiAddRule(&Api,"/favicon.ico"				,"GET"	,&ApiFavIco				,STRCMP);
		RestApiAddRule(&Api,"^/css/[A-Za-z0-9.-]+$"		,"GET"	,&ApiUserSendCssJs		,REGEX);
		RestApiAddRule(&Api,"^/fonts/[A-Za-z0-9.-]+$"	,"GET"	,&ApiUserSendCssJs		,REGEX);
		RestApiAddRule(&Api,"^/js/[A-Za-z0-9.-]+$"		,"GET"	,&ApiUserSendCssJs		,REGEX);
		
		RestApiAddRule(&Api,"/admin/"					,"GET"	,&ApiAdminShowIndex		,STRCMP);
		RestApiAddRule(&Api,"/admin/Users"				,"GET"	,&ApiAdminShowIndex		,STRCMP);
		RestApiAddRule(&Api,"/admin/Users!Add"			,"GET"	,&ApiAdminUserAddGET	,STRCMP);
		RestApiAddRule(&Api,"/admin/Users!Add"			,"POST"	,&ApiAdminUserAddGET	,STRCMP);
		
		if(CFG.bHttp)
			mg_start_thread(MongoseServe, server);
		
		while(CFG.bRunning){
			usleep(10000);
		}

		// cleanup/free Api
		RestApiDeinit(&Api);
	}
	
	// Cleanup, and free server instance
	int iCnt = 0;
	syslog(LOG_INFO,"wating for httpd to stop");
	while(bHttpFinished==FALSE){
		usleep(10000);
	
		if(iCnt == 100){
			syslog(LOG_WARNING,"abrot wating...");
			break;
		}
		iCnt++;
	}
	mg_destroy_server(&server);
	
	lcConfigFree(Ini);
	closelog();
	
	return 0;
}
