#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <string.h>

#define MAX_CMD_LEN 6
#define NUM_ARGS 5
#define DELIMS " \t\n"

//ADD responses
#define ADD_RESP_FULL "FULL"
#define ADD_RESP_INVD "INVALID"
#define ADD_RESP_ADD "ADDED"
#define ADD_RESP_ERR "ADD ERROR"

//RM responses
#define RM_RESP "REMOVED"
#define EMPTY_RESP "EMPTY" //Also used with start

//START responses
#define START_RESP "STARTED"
#define GOING_RESP "ALREADY GOING"

//STOP responses
#define STOP_RESP "STOPPED"

//ERR response
#define ERR_RESP "ERROR"

//Valid Commands
#define RREG_CMD "RREG"
#define ADD_CMD "ADD"
#define RM_CMD "RM"
#define STOP_CMD "STOP"
#define START_CMD "START"
#define QRY_CMD "QRY"
#define RST_CMD "RST"
#define CRPT_CMD "CRPT"

typedef enum command {
    CMD_ERR,
    CMD_RREG,
    CMD_ADD,
    CMD_RM,
    CMD_STOP,
    CMD_START,
    CMD_QRY,
	CMD_RST,
    CMD_CRPT,
}cmd;

cmd findCommand(char* command);

#endif
