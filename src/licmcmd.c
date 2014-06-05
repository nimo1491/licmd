#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "licmd.h"
#include "likcs.h"

static void show_fwinfo(UINT8 *resData);
static void isAST1000(UINT8 *completionCode, UINT8 *major, UINT8 *minor);

/***** Customized commands' template *****/
/* @fn:     start with li_
 * @param:  the address of completion code
 * @ret:    alway return 0, which indicates the existence of this customized command
 * @desc:   please refer to the struct 'cmcmd'
 */

int li_devid(UINT8 *completionCode) 
{
    int    ret = 0;
    UINT8  sa = 32;
    UINT8  netfn = 6;
    UINT8  lun = 0;
    UINT8  cmd = 1;
    UINT8  *reqData;
    UINT8  resData[MAXLEN];
    UINT32 reqLen = 0;
    UINT32 resLen = sizeof(resData);

    printf("Detect BMC...");
    ret = ipmi_command_direct(sa, netfn, lun, cmd, reqData, reqLen, resData, &resLen, completionCode);
    if (ret == 0 && *completionCode == 0) {
        printf("Pass\n");
    }
    else {
        printf("Fail\n");
    }

    return 0;
}

int li_fwinfo(UINT8 *completionCode) 
{
    int    ret = 0;
    UINT8  sa = 32;
    UINT8  netfn = 46;
    UINT8  lun = 0;
    UINT8  cmd = 9;
    UINT8  *reqData;
    UINT8  resData[MAXLEN];
    UINT32 reqLen = 0;
    UINT32 resLen = sizeof(resData);
    UINT8  major;
    UINT8  minor;

    // detect BMC first
    isAST1000(completionCode, &major, &minor);
    if (*completionCode != 0) {
        printf("Skip other commands...\n");
        return 0;
    }
    else if (major == 0 && minor == 0) {
        printf("The firmware belongs to AST1000, this command doesn't support it.\n");
        return 0;
    }

    ret = ipmi_command_direct(sa, netfn, lun, cmd, reqData, reqLen, resData, &resLen, completionCode);
    if (ret == 0 && *completionCode == 0 && resLen == 13) {
        show_fwinfo(resData);
    }
    else {
        printf("Get firmware information failed...\n");
        printf("The BMC is AST1000 or AST2150? This command doesn't support the platforms with those two controllers...\n");
    }
    
    return 0;
}

static void show_fwinfo(UINT8 *resData)
{
    int  fwMbInfo;
    char fwRev[16];

    //get ODM customer
    switch (resData[2]) {
        case 10:
            printf("ODM Customer: %s\n", "Lanner Standard");
            break;
        case 11:
            printf("ODM Customer: %s\n", "Check Point");
            break;
        case 12:
            printf("ODM Customer: %s\n", "Celestix");
            break;
        case 13:
            printf("ODM Customer: %s\n", "A10 Network");
            break;
        case 14:
            printf("ODM Customer: %s\n", "Nomadix");
            break;
        case 15:
            printf("ODM Customer: %s\n", "LS-China");
            break;
        default:
            printf("ODM Customer: %s\n", "Not available");
            break;
    }

    //get motherboard info
    fwMbInfo = resData[3] * 100 + resData[4];
    if (fwMbInfo == 2300)
        printf("Suitable Motherboard: %s\n", "All in one firmware");
    else
        printf("Suitable Motherboard: %s-%d\n", "MB", fwMbInfo);

    //get firmware revision
    if (resData[2] == 11 && resData[0] >= 2 && resData[5] >= 2) { // cp all-in-one firmware has special version format
        sprintf(fwRev, "%d.%02db%02d", resData[0], resData[1], resData[5]);
    }
    else {
        sprintf(fwRev, "%d.%d", resData[0], resData[1]);
    }
    printf("Firmware Revision: %s\n", fwRev);
}

static void isAST1000(UINT8 *completionCode, UINT8 *major, UINT8 *minor)
{
    int    ret = 0;
    UINT8  sa = 32;
    UINT8  netfn = 6;
    UINT8  lun = 0;
    UINT8  cmd = 1;
    UINT8  *reqData;
    UINT8  resData[MAXLEN];
    UINT32 reqLen = 0;
    UINT32 resLen = sizeof(resData);

    printf("Detect BMC...");
    ret = ipmi_command_direct(sa, netfn, lun, cmd, reqData, reqLen, resData, &resLen, completionCode);
    if (ret == 0 && *completionCode == 0) {
        printf("Pass\n");
        *major = resData[2];
        *minor = resData[3];
    }
    else {
        printf("Fail\n");
    }
}
