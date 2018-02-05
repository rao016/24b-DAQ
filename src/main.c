#include "main.h"

/// Maximum number of data Bytes to send to the host at a time
#define DEVICE_DATA_BUFFER_SIZE   10000
#define TX_BUF_SIZE 50

static volatile bool g_bulkIN_xfer_active = false;
static volatile uint8_t main_cmd_status;
char cmd_txbuf[TX_BUF_SIZE];
bool cmd_resp = false;

void command_handler(uint8_t* command) {
	char *args[NUM_ARGS];
	uint8_t val, cmd_num, i = 0;
	
	args[i++] = strtok(command, DELIMS);
	while(*command && i < (NUM_ARGS-1)) args[i++] = strtok(NULL, DELIMS);
	args[i] = NULL;
	
	switch(cmd_num = findCommand(args[0])) {
		case CMD_RREG:
			snprintf(cmd_txbuf, TX_BUF_SIZE, "%d", readReg(atoi(args[1])));
			break;
		case CMD_ADD:
			//# Samples, Sample Rate, Channels
			switch (val = add(strtoul(args[1],NULL,10),atof(args[2]),atoi(args[3]))) {
				case OK_RESPONSE:
					strcpy(cmd_txbuf,ADD_RESP_ADD);
					break;
				case INVALID_RESPONSE:
					strcpy(cmd_txbuf,ADD_RESP_INVD);
					break;
				case FULL_RESPONSE:
					strcpy(cmd_txbuf,ADD_RESP_FULL);
					break;
				default:
					strcpy(cmd_txbuf,ADD_RESP_ERR);
					break;
			}
			break;
		case CMD_RM:
			if (val = rm()) strcpy(cmd_txbuf,RM_RESP);
			else strcpy(cmd_txbuf,EMPTY_RESP);
			break;
		case CMD_QRY:
			qryDSet(strtoul(args[1],NULL,TX_BUF_SIZE),cmd_txbuf,TX_BUF_SIZE);
			break;
		case CMD_STOP:
			ss = stop();
			strcpy(cmd_txbuf,STOP_RESP);
			break;
		case CMD_START:
			switch (ss = start()) {
				case START:
					strcpy(cmd_txbuf,START_RESP);
					break;
				case GO:
					strcpy(cmd_txbuf,GOING_RESP);
					break;
				default:
					strcpy(cmd_txbuf, EMPTY_RESP);
					break;
			}
			break;
		case CMD_RST:
			system_reset();
			break;
        case CMD_CRPT:
            if (is_corrupt()) strcpy(cmd_txbuf,"TRUE");
            else strcpy(cmd_txbuf,"FALSE");
            break;
		default:
			cmd_num = CMD_ERR;
            break;
	}
	if (cmd_num == CMD_ERR) strcpy(cmd_txbuf,ERR_RESP);
    cmd_resp = true;
	UDI_TMC_RECEIVE_BULKOUT_COMMAND();
}

//==============================================================================
/// Structures used to send responses for Bulk-IN/OUT abort operations
typedef union
{
   /// Response to an INITIATE_CLEAR request
   uint8_t initiate_clear;

   /// Response to an INITIATE_BULK_IN/OUT_ABORT request
   TMC_initiate_abort_bulk_xfer_response_t initiate_abort;

   /// Response to a CHECK_BULK_OUT_ABORT request
   TMC_check_abort_bulkOUT_status_response_t check_abortOUT;

   /// Response to a CHECK_BULK_IN_ABORT request
   TMC_check_abort_bulkIN_status_response_t check_abortIN;

   /// Response to a CHECK_CLEAR_STATUS request
   TMC_check_clear_status_response_t check_clear;

} Bulk_abort_response_u;


//==============================================================================
/// \brief Data structure used to track the state of a Bulk IN transfer
typedef struct {
   uint8_t bTag;     ///< bTag ID of the transfer
   uint32_t numBytesRemaining;   ///< Number of Bytes left to report
   uint32_t numBytesTransferred; ///< Number of Bytes transferred so far
} DeviceDataRequest_t;

/// Placeholder value for the bTag field of a DeviceDataRequest_t
#define INVALID_bTag    (uint8_t)0

COMPILER_PACK_SET(1)
/// Structure used to send data the host in a DEV_DEP_MSG_IN message
typedef struct {
   /// Message header
   TMC_bulkIN_dev_dep_msg_in_header_t header;

   uint8_t data[DEVICE_DATA_BUFFER_SIZE];
} DeviceDataResponse_t;
COMPILER_PACK_RESET()

COMPILER_WORD_ALIGNED static Bulk_abort_response_u g_bulk_abort_response = {0};

/// Values used to track the active data request
static DeviceDataRequest_t activeDataRequest =
            { INVALID_bTag,   // bTag
              0,              // numBytesRemaining
              0   };          // numBytesTransferred

/// Buffer used for TMCC data
COMPILER_WORD_ALIGNED static DeviceDataResponse_t deviceDataResponse;
//@}


// Function Prototypes
static void abort_tmc_bulkIN_transfer(void);
static void main_req_dev_dep_msg_in_sent(udd_ep_status_t status, iram_size_t nb_transfered, udd_ep_id_t ep);

////////////////////////////////////////////////////////////////////////////////
bool main_tmc_enable(void)
{
   g_bulkIN_xfer_active = true;

   // Start data reception on OUT endpoints
   UDI_TMC_RECEIVE_BULKOUT_COMMAND();

   return true;
}

////////////////////////////////////////////////////////////////////////////////
void main_tmc_disable(void)
{
   abort_tmc_bulkIN_transfer();  // Abort any active transfer
}

////////////////////////////////////////////////////////////////////////////////
void main_sof_action( void )
{
   // Only process frames if enabled
   if ( g_bulkIN_xfer_active )
   {
      uint16_t frame_number = udd_get_frame_number();
      ui_process(frame_number);
   }
}

////////////////////////////////////////////////////////////////////////////////
void main_suspend_action(void)
{
   ui_powerdown();
}

////////////////////////////////////////////////////////////////////////////////
void main_resume_action(void)
{
   ui_wakeup();
}

////////////////////////////////////////////////////////////////////////////////
void main_initiate_abort_bulkOUT(void)
{
   // Bulk-OUT transfers (e.g. data transfers from host computer to USB device)
   // are not supported by this design, so there's nothing to do when a request
   // to abort Bulk-OUT transfers is received

   // Populate fields of the response
   g_bulk_abort_response.initiate_abort.usbtmc_status =
                                          TMC_STATUS_TRANSFER_NOT_IN_PROGRESS;
   g_bulk_abort_response.initiate_abort.bTag = 0;

   udd_g_ctrlreq.payload = (uint8_t*)&g_bulk_abort_response.initiate_abort;
   udd_g_ctrlreq.payload_size = sizeof(TMC_initiate_abort_bulk_xfer_response_t);
}

////////////////////////////////////////////////////////////////////////////////
void main_check_abort_bulkOUT_status(void)
{
   // Bulk-OUT transfers (e.g. data transfers from host computer to USB device)
   // are not supported by this design, so there should never be any Bulk-OUT
   // abort requests pending

   g_bulk_abort_response.check_abortOUT.nbytes_rxd = 0;
   g_bulk_abort_response.check_abortOUT.reserved[0] = 0;
   g_bulk_abort_response.check_abortOUT.reserved[1] = 0;
   g_bulk_abort_response.check_abortOUT.reserved[2] = 0;
   g_bulk_abort_response.check_abortOUT.usbtmc_status = TMC_STATUS_TRANSFER_NOT_IN_PROGRESS;

   udd_g_ctrlreq.payload = (uint8_t*)&g_bulk_abort_response.check_abortOUT;
   udd_g_ctrlreq.payload_size = sizeof(TMC_check_abort_bulkOUT_status_response_t);
}

////////////////////////////////////////////////////////////////////////////////
void main_initiate_abort_bulkIN(void)
{
   // If a BulkIN transfer is active, stop it; otherwise, indicate that there
   // is no transfer active
   if ( INVALID_bTag != activeDataRequest.bTag )
   {
      abort_tmc_bulkIN_transfer();     // Reset the active transfer
      g_bulk_abort_response.initiate_abort.usbtmc_status = TMC_STATUS_SUCCESS;
   }
   {
      g_bulk_abort_response.initiate_abort.usbtmc_status =
                                          TMC_STATUS_TRANSFER_NOT_IN_PROGRESS;
   }

   g_bulk_abort_response.initiate_abort.bTag = activeDataRequest.bTag;

   udd_g_ctrlreq.payload = (uint8_t*)&g_bulk_abort_response.initiate_abort;
   udd_g_ctrlreq.payload_size = sizeof(TMC_initiate_abort_bulk_xfer_response_t);
}


////////////////////////////////////////////////////////////////////////////////
void main_check_abort_bulkIN_status(void)
{
   g_bulk_abort_response.check_abortIN.nbytes_txd =
                                       activeDataRequest.numBytesTransferred;
   g_bulk_abort_response.check_abortIN.bmAbortBulkIn = 0;
   g_bulk_abort_response.check_abortIN.reserved[0] = 0;
   g_bulk_abort_response.check_abortIN.reserved[1] = 0;
   g_bulk_abort_response.check_abortIN.usbtmc_status =
                  g_bulkIN_xfer_active ? TMC_STATUS_SUCCESS :
                                           TMC_STATUS_TRANSFER_NOT_IN_PROGRESS;

   udd_g_ctrlreq.payload = (uint8_t*)&g_bulk_abort_response.check_abortIN;
   udd_g_ctrlreq.payload_size = sizeof(TMC_check_abort_bulkIN_status_response_t);
}

////////////////////////////////////////////////////////////////////////////////
void main_initiate_clear(void)
{
   g_bulk_abort_response.initiate_clear = TMC_STATUS_SUCCESS;
   udd_g_ctrlreq.payload = &g_bulk_abort_response.initiate_clear;
   udd_g_ctrlreq.payload_size = sizeof(uint8_t);

   // TODO: implement clearing input/output buffers
}

////////////////////////////////////////////////////////////////////////////////
void main_check_clear_status(void)
{
   // NOTE: This function presently doesn't do anything special since buffers
   //       are cleared more or less instantaneously.

   // TODO: clear sample buffers

   g_bulk_abort_response.check_clear.usbtmc_status = TMC_STATUS_SUCCESS;
   g_bulk_abort_response.check_clear.bmClear = 0;
   udd_g_ctrlreq.payload = (uint8_t*)&g_bulk_abort_response.check_clear;
   udd_g_ctrlreq.payload_size = sizeof(TMC_check_clear_status_response_t);
}


////////////////////////////////////////////////////////////////////////////////
/** \brief Execution entry point
 *
 *  \remarks
 *    This function invokes initialization routines to configure and start the
 *    USB interface, then enters a permanent idle loop.
 */

void init(void) {
	irq_initialize_vectors();
	cpu_irq_enable();
	system_init();
	sleepmgr_init();
	init_timer();
	initADC();
	sampling_init();
	ui_init();
	udc_start();
}

int main(void) {
	uint32_t temp;
	init();
	
	while (true) {
		sleepmgr_enter_sleep();
		readData();
	}
}

////////////////////////////////////////////////////////////////////////////////
/** \brief Helper function used to abort active/pending Bulk IN transfers
 */
void abort_tmc_bulkIN_transfer(void)
{
   // Reset the active transfer
   activeDataRequest.bTag = INVALID_bTag;
   activeDataRequest.numBytesRemaining = 0;
   activeDataRequest.numBytesTransferred = 0;

   g_bulkIN_xfer_active = false;

   // Tell the dev board API that we are disconnected (uses the API defined for
   // the MattairTech MT-D11 board
   ui_loop_back_state(false);
}

////////////////////////////////////////////////////////////////////////////////
bool main_req_dev_dep_msg_in_received(TMC_bulkOUT_request_dev_dep_msg_in_header_t const* header) {
	TMC_bulkIN_dev_dep_msg_in_header_t* responseHeader = &deviceDataResponse.header;
	TMC_bulkIN_header_t* bulkInHeader = &responseHeader->header;
	uint32_t numBytesTransferred;
    bool sendADCData = false;

	//Find number of bytes to transfer
	//Send it over the line, 0 byte otherwise

	// If a transfer is not currently active, start a new one
	if (activeDataRequest.bTag != header->header.bTag) {
		activeDataRequest.bTag = header->header.bTag;

		// Disallow requests for less data than exists in a sample
		if (header->transferSize < ADC_BYTES_PER_SAMPLE) return 0;

		activeDataRequest.numBytesRemaining = header->transferSize;
		activeDataRequest.numBytesTransferred = 0;
	}

	// Determine if there is data to transfer
	// STATUS:
	//   A request is active, but all requested data Bytes have been
	//   transferred.  This should never happen, and it indicates the host
	//   driver may not be well-behaved.  Return false to signal an error.
	if (0 == activeDataRequest.numBytesRemaining) return 0;

	// Copy sample data into the message
    if (cmd_resp) {
        strcpy(deviceDataResponse.data, cmd_txbuf);
        numBytesTransferred = min(min(activeDataRequest.numBytesRemaining, DEVICE_DATA_BUFFER_SIZE), strlen(cmd_txbuf));
        cmd_resp = false;
    }
    else numBytesTransferred = send_ADC_data(deviceDataResponse.data, min(activeDataRequest.numBytesRemaining, DEVICE_DATA_BUFFER_SIZE));

	// Cannot send nothing... send NULL instead
	if (numBytesTransferred == 0) {
		numBytesTransferred = 1;
		deviceDataResponse.data[0] = NULL;
	}

	// Update request state
	activeDataRequest.numBytesRemaining -= numBytesTransferred;
	activeDataRequest.numBytesTransferred += numBytesTransferred;

	//-------------------------------------------------
	// Set up the response BulkIN header
	//-------------------------------------------------
	bulkInHeader->MsgID = TMC_BULKIN_DEV_DEP_MSG_IN;
	bulkInHeader->bTag = activeDataRequest.bTag;
	bulkInHeader->bTagInverse = ~activeDataRequest.bTag;
	bulkInHeader->reserved = 0;

	//-------------------------------------------------
	// Set up device-dependent data response header
	//-------------------------------------------------
	responseHeader->transferSize = numBytesTransferred;

	// Set bit zero (EOM) if no Bytes remain in the transfer
    responseHeader->bmTransferAttributes = (activeDataRequest.numBytesRemaining > 0) ? 0 : 1;
	responseHeader->reserved[0] = 0;
	responseHeader->reserved[1] = 0;
	responseHeader->reserved[2] = 0;

	// Send the response
    if ((numBytesTransferred % 64) == 52) numBytesTransferred++;
	return 1 == udi_tmc_bulk_in_run((uint8_t*)&deviceDataResponse, (sizeof(TMC_bulkIN_dev_dep_msg_in_header_t) + numBytesTransferred), main_req_dev_dep_msg_in_sent);
}

////////////////////////////////////////////////////////////////////////////////
void main_req_dev_dep_msg_in_sent(udd_ep_status_t status, iram_size_t nb_transfered, udd_ep_id_t ep) {
   UDI_TMC_RECEIVE_BULKOUT_COMMAND();  // Receive the next command
}
