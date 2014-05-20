#Lanner IPMI Command Tool
This tool is used to send IPMI command to BMC via KCS directly under Linux or DOS.
We also customized some commands for testbed using on this version.

##Version
v0.1.2

##Usage:
* `licmd <command>`, where command is one of the following:
    * `devid`:  check the existence of BMC.
    * `fwinfo`: get revision, ODM customer, and suitable motherboard of the firmware.
* Or use raw command `licmd SLAVE_ADDR NETFN CMD [DATA]`

##How to Install:
* Invoke `make` to build the executable file.
* Invoke `make clean` to remove the executable file.

