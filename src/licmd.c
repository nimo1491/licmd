#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "licmd.h"
#include "likcs.h"
#include "cc_str.h"

static void     print_usage(void);
static UINT8    htoi(UINT8 *);
static int      send_command(UINT8 *, int, UINT8 *, int *);

static struct {
    int  idx;
    char tag[16];
    int  (*cmf)(UINT8 *);
    char desc[128];
} cmcmds[NCMCMDS] = {
    {0, "devid",   li_devid,   "Send \"Get Device ID\" command to check the existence of BMC."},
    {1, "fwinfo",  li_fwinfo,  "Send OEM \"Firmware Info\" command to get the version and build time of current firmware."}
    //{2, "hwid",    li_hwid,    "Send OEM \"Get Hardware ID\" command to get the system's hardware ID."},
    //{3, "netinfo", li_netinfo, "Send \"Get IP/Mask/Gateway\" commands to get the network configurations."}
};

/* Truth table of cmRet and cc that returned from customized commands
 * ------------------------------------------------------------------
 * cmRet    |cc     | Result
 * ---------|-------|-------
 * 0        |0      | Success
 * 0        |!0     | BMC doesn't exist or IPMI command error
 * !0       |0      | Impossible
 * !0       |!0     | Wrong command format
 */
int main(int argc, const char *argv[])
{
    int cmRet = -1;
    int i, reqLen, resLen;
    UINT8 cc = 0xff;
    UINT8 reqData[MAXLEN], resData[MAXLEN]; 

    if (argc < 2) {
        print_usage();
        return -1;
    }

    for (i = 0; i < NCMCMDS; ++i) {
        if (strcmp(argv[1], cmcmds[i].tag) == 0) {
            printf("%s\n", cmcmds[i].desc);
            cmRet = cmcmds[i].cmf(&cc);
            break;
        }
    }

    if (i >= NCMCMDS && argc >= 4) { //send raw command
        reqLen = --argc;
        resLen = sizeof(resData);
        for (i = 0; i < reqLen; i++) {
            reqData[i] = htoi((UINT8 *)argv[i+1]);
            if (reqLen > MAXLEN)
                break;
        }
        send_command(reqData, reqLen, resData, &resLen);
    }
    else if (cmRet == 0) { //the customized command exists
        if (cc != 0) {
            printf("Completion Code: %02X (", cc);
            if (cc > 0 && cc < 255)
                printf("%s)\n", ipmi_cc_string[cc - (UINT8)0xbf]);
            else
                printf("%s)\n", cc == 0 ? ipmi_cc_string[0] : ipmi_cc_string[IPMI_CC_LENGTH-1]);
        }
    }
    else {
        printf("Warning: You may send wrong format of command, please refer to the below usage:\n");
        print_usage();
    }

    return 0;
}

static void print_usage(void)
{
    char *cmmsg = "Usage: licmd <command>\nwhere <command> is one of the following:\n";
    char *rawmsg = "Or use raw command: licmd SLAVEADDR NETFN COMMAND [DATA]\n";
    int i;

    printf("%s", cmmsg);
    for (i = 0; i < NCMCMDS; ++i)
    {
        printf("\t%s\t%s\n", cmcmds[i].tag, cmcmds[i].desc);
    }
    printf("%s", rawmsg);
}

/*
 * @fn:     htoi()
 * @param:  a 2 character string of hex digits.
 * @ret:    a hex byte.
 * @brief:  transform a 2 character string of hex digits to a hex byte 
 */
static UINT8 htoi(UINT8 *inhex)
{
    UINT8 val;
    UINT8 c;

    if (inhex[1] == 0) {
        c = inhex[0] & 0x5f;
        if (c > '9') c += 9;
        val = (c & 0x0f);
    } else {
        c = inhex[0] & 0x5f;
        if (c > '9') c += 9;
        val = (c & 0x0f) << 4;
        c = inhex[1] & 0x5f;
        if (c > '9') c += 9;
        val += (c & 0x0f);
	}

	return val;
}

/*
 * @fn:     send_command()
 * @param:  ipmi request/response data and its length
 * @ret:    if 0, send command successfully
 * @brief:  this function send command to BMC and print it's response
 */
static int send_command(UINT8 *reqData, int reqLen, UINT8 *resData, int *resLen)
{
    int i, ret = 0;
    UINT8 completionCode = 0;
    UINT8 netfn, lun;

    lun = reqData[1] & 0x03;
    netfn = reqData[1] >> 2;

    printf("-----REQUEST DATA-----\n");
    printf("Slave Addr: %02X  NetFn: %02X  LUN: %02X  Command: %02X\n", reqData[0], netfn, lun, reqData[2]);
    if (reqLen > 3) {
        printf("Data: ");
        for (i = 0; i < reqLen - 3; i++) 
            printf("%02X ", reqData[3+i]);
        putchar('\n');
    }

    ret = ipmi_command_direct(reqData[0], netfn, lun, reqData[2], &reqData[3], (UINT32)reqLen - 3, resData, (UINT32 *)resLen, &completionCode);

    if (ret == 0) {
        printf("-----RESPONSE DATA-----\n");
        printf("Completion Code: %02X (", completionCode);
        if (completionCode > 0 && completionCode < 255)
            printf("%s)\n", ipmi_cc_string[completionCode - (UINT8)0xbf]);
        else
            printf("%s)\n", completionCode == 0 ? ipmi_cc_string[0] : ipmi_cc_string[IPMI_CC_LENGTH-1]);
        if (*resLen > 0) {
            printf("Data: ");
            for (i = 0; i < *resLen; i++)
                printf("%02X ", resData[i]);
            putchar('\n');
        }
    }
    else {
        printf("-----Error State-----\n");
        show_status_register();
        printf("%s\n", kcs_cc_string[ret]);
    }

    return ret;
}

