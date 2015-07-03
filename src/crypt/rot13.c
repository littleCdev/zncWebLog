//
//  rot13.c
//  zncWebLog
//
//  Created by Littlecheetah on 21.05.15.
//  Copyright (c) 2015 Littlecheetah. All rights reserved.
//

#include "rot13.h"

int lcRot13EncodeChar(int c){
    if('a' <= c && c <= 'z'){
        return (((c-'a')+13)%26)+'a';
    } else if ('A' <= c && c <= 'Z') {
        return (((c-'A')+13)%26)+'A';
    } else {
        return c;
    }
}
int lcRot13DecodeChar(int c){
    if(!(c>='a' && c<='z')&& !(c>='A' && c<='Z'))
        return c;
    
    if(c-13 < 'a' && c > 'Z'){
        return 'z'-('a'-c+12);
    }else if(c-13 < 'A'){
        return 'Z'-('A'-c+12);
    }
    
    return c-13;
}

char *lcRot13Decode(char *sString){
    int  iLen   = (int)strlen(sString);
    char *sRet  = malloc(iLen+1);
    int  i      = 0;
        
    for (i=0; i<iLen; i++) {
        sRet[i] = lcRot13DecodeChar(sString[i]);
    }
    sRet[iLen]= (unsigned char)'\0';
    
    return sRet;
}
char *lcRot13Encode(char *sString){
    int iLen    = (int)strlen(sString);
    int i       = 0;
    char *sRet = malloc(iLen+1);
    
    for(i=0;i<iLen;i++){
        sRet[i] = lcRot13EncodeChar(sString[i]);
    }

    sRet[iLen]= '\0';
    return  sRet;
}
