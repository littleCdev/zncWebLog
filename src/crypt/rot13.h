//
//  rot13.h
//  zncWebLog
//
//  Created by Littlecheetah on 21.05.15.
//  Copyright (c) 2015 Littlecheetah. All rights reserved.
//

#ifndef __zncWebLog__rot13__
#define __zncWebLog__rot13__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


int lcRot13EncodeChar(int c);
char *lcRot13Encode(char *sString);
char *lcRot13Decode(char *sString);

#endif /* defined(__zncWebLog__rot13__) */
