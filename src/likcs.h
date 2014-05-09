#ifndef _LIKCS_H
#define _LIKCS_H

int ipmi_command_direct(UINT8, UINT8, UINT8, UINT8, UINT8 *, UINT32, UINT8 *, UINT32 *, UINT8 *);
void show_status_register(void);

#endif // for #ifndef _LIKCS_H
