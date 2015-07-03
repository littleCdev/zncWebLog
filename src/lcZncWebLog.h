#ifndef LcZNCWEBLOG
	#define LcZNCWEBLOG
	
	#include <stdbool.h>
	#include <syslog.h>
	
	#define NAMEMAXLEN  	100
	#define PASSMAXLEN  	100
	#define NETWORKMAXLEN  	100
	#define ERRORLEN    	200
	
	#define CFGFILE		"/etc/zncWebLog/zncWebLog.cfg"
	
	#undef _DEBUG_
	#ifdef _DEBUG_
		#define debug(args...) fprintf(stderr,args);fprintf(stderr,"\r\n")
	#else
		#define debug(args...)
	#endif

	#define SETERROR(ptr,format,...)		snprintf(ptr,ERRORLEN,format,##__VA_ARGS__)
	
	// i used my own defines before, now i'm using stdbool.h
	#define TRUE  true
	#define FALSE false
	
	typedef enum {
		lcADMIN,
		lcUSER,
		lcLOGOUT
	} lcUserType;

	typedef struct {
		/* from the configfile*/
		_Bool	bHttp;	
		char 	*sHttpPort;
		
		_Bool 	bHttps;
		char 	*sHttpsPort;
		char 	*sCertFile;

		int 	iLogVerbosity;
		
		char 	*sZncUserDir;
		char 	*sPasswdFile;
		int		iLogLines;
		char 	*sRootUser;
		
		/* the rest */
		char 	*WorkingDirectory;
		_Bool 	bFirstRun;
		_Bool	bRunning;
	}lcCFG;

	#ifdef LC_GLOBALS_MAIN
		lcCFG CFG = {
			
			.bHttp			= true,
			.sHttpPort		= "8000",
			
			.bHttps			= false,
			.sHttpsPort		= "8080",
			.sCertFile		= "",
			
			.iLogVerbosity	= LOG_WARNING,
			.iLogLines		= 200,
			
			.sZncUserDir	= "/home/littlecheetah/.znc/users",
			.WorkingDirectory	= "/etc/zncWebLog/",
			
			.sRootUser 		= "admin",
			
			.sPasswdFile 		= ".zwlpasswd",
			.bFirstRun			= true,
			.bRunning			= true,
		};
		
	#else
		extern lcCFG CFG;
	#endif

	#include <signal.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <errno.h>
	#include <unistd.h> 
	#include <memory.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <regex.h>
	#include <dirent.h>
	
#endif
