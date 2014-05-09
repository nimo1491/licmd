#include <stdio.h>
#include <string.h>
#ifdef DJGPP
#include <dos.h>
#include <inlines/pc.h>
#else
#include <sys/io.h>
#define outb(x, y) outb(y, x)
#endif
#include "licmd.h"
#include "likcs.h"

#define STATUS_REQ      0x60
#define WR_START        0x61
#define WR_END          0x62
#define READ_BYTE       0x68
#define COMMAND_REG     0xca3
#define STATUS_REG      0xca3
#define DATA_IN_REG     0xca2
#define DATA_OUT_REG    0xca2
#define MAX_KCS_LOOP    5000000  

typedef struct {
	UINT8	DevAdd;
	UINT8	NetFn;
	UINT8	LUN;
	UINT8	Cmd;
	UINT8	Data[MAXLEN];
	UINT32	Len;
	UINT8	CompCode;
} Bmc_Message;

typedef struct {
	UINT8   cmdType;
	UINT8   rsSa;
	UINT8   netFn;
	UINT8   rsLun;
	UINT8   *data;
	UINT32  dataLength;
} Kcs_Request;

static int process_message(Bmc_Message *, Bmc_Message *);
static int send_kcs_request(Kcs_Request *, UINT8 *, int *, UINT8 *);
static int wait_for_IBF_clear(void);
static int wait_for_OBF_set(void);
static inline int get_write_state(void);
static inline int get_read_state(void);
static inline int get_idle_state(void);
static inline void clear_OBF(void);

/*
 * @fn:     ipmi_command_direct()
 * @params: slave address, nutfn/lun, command, request/response data and its length, completion code
 * @ret:    if 0, send command direct successfully
 * @brief:  it's a public function which can send command via KCS directly
 */
int ipmi_command_direct(UINT8 sa, UINT8 netfn, UINT8 lun, UINT8 cmd, 
    UINT8 *reqData, UINT32 reqLen, UINT8 *resData, UINT32 *resLen, UINT8 *cc)
{
	Bmc_Message reqMsg;
	Bmc_Message resMsg;
	int i, ret;

	reqMsg.DevAdd	= sa;
	reqMsg.NetFn	= netfn;
	reqMsg.LUN	    = lun;
	reqMsg.Cmd	    = cmd;
	reqMsg.Len	    = reqLen;
	if (reqMsg.Len > 0) 
	    memcpy(reqMsg.Data, reqData, reqLen);

    ret = process_message(&reqMsg, &resMsg);

    if (ret == CC_KCS_OK) {
        *cc = resMsg.CompCode;
        if (resMsg.Len > 0 ) {
            if (*resLen > MAXLEN) 
                *resLen = MAXLEN;
            else
                *resLen = resMsg.Len;
            memcpy(resData, resMsg.Data, *resLen);
        } 
        else *resLen = 0;
    }

	return ret;
}

/*
 * @fn:     ipmi_command_direct()
 * @params: slave address, nutfn/lun, command, request/response data and its length, completion code
 * @ret:    if 0, send command direct successfully
 * @brief:  it's a public function which can send command to BMC via KCS directly
 */
static int process_message(Bmc_Message *p_reqMsg, Bmc_Message *p_resMsg)
{
	int i, status = 0;
	int resDataLen = MAXLEN;
	Kcs_Request requestData = {0};
	UINT8 cc = 0;

	// Initialize Response Message Data
	for (i = 0; i < MAXLEN; i++)
		p_resMsg->Data[i] = 0;

	// Call into IPMI
	if (p_reqMsg->DevAdd == 0x20) {
		requestData.cmdType	    = p_reqMsg->Cmd;
		requestData.rsSa	    = 0x20;
		requestData.netFn	    = p_reqMsg->NetFn;
		requestData.rsLun	    = p_reqMsg->LUN;
		requestData.data	    = p_reqMsg->Data;
		requestData.dataLength	= p_reqMsg->Len;

        status = send_kcs_request(&requestData, p_resMsg->Data, &resDataLen, &cc);

		// Return response message
        p_resMsg->DevAdd	= p_reqMsg->DevAdd;
        p_resMsg->NetFn	    = p_reqMsg->NetFn;
        p_resMsg->LUN		= p_reqMsg->LUN;
        p_resMsg->Cmd		= p_reqMsg->Cmd;
        p_resMsg->CompCode	= cc;
        p_resMsg->Len		= resDataLen;
	}
    else { // other slave address
        status = CC_KCS_UNSPECIFIED;
    }

	return status;
}

/*
 * @fn:     send_kcs_request()
 * @params: Kcs_Request data, response data and its length, completion code
 * @ret:    if 0, send kcs request successfully
 * @brief:  this protocol is referd to IPMI spec p.88
 */
static int send_kcs_request(Kcs_Request *requestData,
    UINT8 *resData, int *resDataLen, UINT8 *cc)
{
	int i, resBufLen;
	UINT8 dummy, resBuf[64];

#ifndef DJGPP
	iopl(3);
#endif

    // write phase
    if (wait_for_IBF_clear() != 0)  // if get -1, maybe kcs disabled
        return CC_KCS_IBF;
    clear_OBF();
    outb(COMMAND_REG, WR_START);
    wait_for_IBF_clear();
    if (get_write_state() != 0)
        return CC_KCS_WRITE;
    clear_OBF();

    outb(DATA_OUT_REG, (requestData->netFn << 2));
    wait_for_IBF_clear();
    if (get_write_state() != 0)
        return CC_KCS_WRITE;
    clear_OBF();

    if (requestData->dataLength == 0) {
        outb(COMMAND_REG, WR_END);
        wait_for_IBF_clear();
        if (get_write_state() != 0)
            return CC_KCS_WRITE;
        clear_OBF();

        outb(DATA_OUT_REG, requestData->cmdType);
    }
    else {
        outb(DATA_OUT_REG, requestData->cmdType);
        wait_for_IBF_clear();
        if (get_write_state() != 0)
            return CC_KCS_WRITE;
        clear_OBF();
        for (i = 0; i < requestData->dataLength-1; i++) {
            outb(DATA_OUT_REG, requestData->data[i]);
            wait_for_IBF_clear();
            if (get_write_state() != 0)
                return CC_KCS_WRITE;
            clear_OBF();
        }

        outb(COMMAND_REG, WR_END);
        wait_for_IBF_clear();
        if (get_write_state() != 0)
            return CC_KCS_WRITE;
        clear_OBF();

        outb(DATA_OUT_REG, requestData->data[i]);
    }

    // read phase
    resBufLen = *resDataLen;
    *resDataLen = 0;

    while (*resDataLen <= MAXLEN) {
        wait_for_IBF_clear();
        if (get_read_state() != 0) {
            if (get_idle_state() != 0) { // error exit
                return CC_KCS_IDLE;
            } else { // idle state
                wait_for_OBF_set();
                dummy = inb(DATA_IN_REG);
                (*resDataLen) -= 3; // netfn/lun, cmd, cc
                *cc = resBuf[2];
                for (i = 0; i < *resDataLen; i++)
                    resData[i] = resBuf[i+3];
                return CC_KCS_OK;
            }
        } else { // read state
            wait_for_OBF_set();
            resBuf[(*resDataLen)++] = inb(DATA_IN_REG);
            outb(DATA_IN_REG, READ_BYTE);
        }
        if (*resDataLen > resBufLen) break;
    }

    return CC_KCS_OK;
}

static int wait_for_IBF_clear(void)
{
    int i = 0;

    while ((inb(STATUS_REG) & 0x02) == 0x02) {
        if (i > 0 && (i % 100) == 0) usleep(1000); /*sleep for 1 msec*/
        if (i > MAX_KCS_LOOP) 
            return -1;
        i++;
    }

    return 0;
}

static int wait_for_OBF_set(void)
{
    int i = 0;

    while ((inb(STATUS_REG) & 0x01) == 0x00) {
        if (i > 0 && (i % 100) == 0) usleep(1000); /*sleep for 1 msec*/
        if (i > MAX_KCS_LOOP) 
            return -1;
        i++;
    }

    return 0;
}

static inline int get_write_state(void)
{
    if ((inb(STATUS_REG) >> 6) != 0x02)
        return -1;
    return 0;
}

static inline int get_read_state(void)
{
    if ((inb(STATUS_REG) >> 6) != 0x01)
        return -1;
    return 0;
}

static inline int get_idle_state(void)
{
    if ((inb(STATUS_REG) >> 6) != 0x00)
        return -1;
    return 0;
}

static inline void clear_OBF(void)
{
    UINT8 dummy = inb(DATA_IN_REG);
}

void show_status_register(void)
{
    printf("Status Register: %02X\n", inb(STATUS_REG));
}

