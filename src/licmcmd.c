#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "licmd.h"
#include "likcs.h"

static void show_fwinfo(UINT8 *resData);

/***** Customized commands' template *****/
/* @fn:     start with li_
 * @param:  the address of completion code
 * @ret:    alway return 0, which indicates the existence of this customized command
 * @desc:   please refer to the struct 'cmcmd'
 */

int li_devid(UINT8 *completionCode) {

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

int li_fwinfo(UINT8 *completionCode) {

    int    ret = 0;
    UINT8  sa = 32;
    UINT8  netfn = 46;
    UINT8  lun = 0;
    UINT8  cmd = 9;
    UINT8  *reqData;
    UINT8  resData[MAXLEN];
    UINT32 reqLen = 0;
    UINT32 resLen = sizeof(resData);

    // detect BMC first
    li_devid(completionCode);
    if (*completionCode != 0) {
        printf("Skip other commands...\n");
        return 0;
    }

    ret = ipmi_command_direct(sa, netfn, lun, cmd, reqData, reqLen, resData, &resLen, completionCode);
    if (ret == 0 && *completionCode == 0) {
        show_fwinfo(resData);
    }
    else {
        printf("Get firmware information failed...\n");
    }
    
    return 0;
}

static void show_fwinfo(UINT8 *resData)
{
    char fwRev[16];
    char fwBuildTime[32];
    const char *Month[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    //get firmware revision
    if (resData[2] == 11 && resData[0] >= 2 && resData[5] >= 2) { // cp all-in-one firmware has special version format
        sprintf(fwRev, "%d.%02db%02d", resData[0], resData[1], resData[5]);
    }
    else {
        sprintf(fwRev, "%d.%d", resData[0], resData[1]);
    }
    printf("Firmware Revision: %s\n", fwRev);

    //get firmware build time
    sprintf(fwBuildTime, "%s %d %d%d %d:%d:%d", Month[resData[8]], resData[9], resData[6], resData[7], resData[10], resData[11], resData[12]);
    printf("Firmware Build Time: %s\n", fwBuildTime);
}
