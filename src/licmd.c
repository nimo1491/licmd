#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "licmd.h"
#include "likcs.h"
#include "cc_str.h"

static void     print_usage(void);
static UINT8    htoi(UINT8 *);
static int      send_command(UINT8 *, int, UINT8 *, int *);

int main(int argc, const char *argv[])
{
    int i, reqLen, resLen;
    UINT8 reqData[MAXLEN], resData[MAXLEN]; 

    reqLen = --argc;
    resLen = sizeof(resData);
    for (i = 0; i < reqLen; i++)  {
        reqData[i] = htoi((UINT8 *)argv[i+1]);
        if (reqLen > MAXLEN)
            break;
    }

    if (reqLen < 3)
        print_usage();
    else
        send_command(reqData, reqLen, resData, &resLen);

    return 0;
}

static void print_usage(void)
{
    printf("licmd SLAVEADDR NETFN COMMAND [DATA]\n");
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

