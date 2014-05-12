#ifndef _LICMD_H
#define _LICMD_H

#define MAXLEN  64
#define NCMCMDS 2 

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;

enum { CC_KCS_OK, CC_KCS_IBF, CC_KCS_WRITE, CC_KCS_IDLE, CC_KCS_UNSPECIFIED };

// customized commands
int li_devid(UINT8 *cc);
int li_fwinfo(UINT8 *cc);

#endif // for #ifndef _LICMD_H
