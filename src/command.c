#include "command.h"

/******************************************************************
 *
 * Description: Returns the string command that was input as a number.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
cmd findCommand(char *command) {
    if (0 == strcmp(command, RREG_CMD)) return CMD_RREG;
    else if (0 == strcmp(command, ADD_CMD)) return CMD_ADD;
    else if (0 == strcmp(command, RM_CMD)) return CMD_RM;
    else if (0 == strcmp(command, STOP_CMD)) return CMD_STOP;
    else if (0 == strcmp(command, START_CMD)) return CMD_START;
    else if (0 == strcmp(command, QRY_CMD)) return CMD_QRY;
	else if (0 == strcmp(command, RST_CMD)) return CMD_RST;
    else if (0 == strcmp(command, CRPT_CMD)) return CMD_CRPT;
    else return CMD_ERR;
}
