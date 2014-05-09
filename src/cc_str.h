#ifndef _CC_STR_H
#define _CC_STR_H

// KCS completion code string
const char *kcs_cc_string[] = {
    "No error.",
    "Wait for IBF clear error.",
    "Get write status error.",
    "Get idle status error.", 
    "Unspecified error."
};

// IPMI completion code string
#define IPMI_CC_LENGTH  25
const char *ipmi_cc_string[] = {
    "Command Completed Normally.",
    "Node Busy.",
    "Invalid Command.",
    "Command invalid for given LUN.",
    "Timeout while processing command.",
    "Out of space.",
    "Reservation Canceled or Invalid Reservation ID.",
    "Request data truncated.",
    "Request data length invalid",
    "Request data field length limit exceeded.",
    "Parameter out of range.",
    "Cannot return number of requested data bytes.",
    "Requested Sensor, data, or record not present.",
    "Invalid data field in Request.",
    "Command illegal for specified sensor or record type.",
    "Command response could not be provided.",
    "Cannot execute duplicated request.",
    "Command response could not be provided. SDR Repository in update mode.",
    "Command response could not be provided. Device in firmware update mode.",
    "Command response could not be provided. BMC initialization or initialization agent in progess.",
    "Destination unavailable.",
    "Cannot execute command due to insufficient privilege level or other security-based restriction.",
    "Cannot execute command. Command, or request parameter(s), not supported in present state.",
    "Cannot execute command. Parameter is illegal because command sub-function has been disabled or is unavailable.",
    "Unspecified error."
};

#endif // for #ifndef _CC_STR_H
