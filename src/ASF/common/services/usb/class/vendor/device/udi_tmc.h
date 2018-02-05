#ifndef _UDI_TMC_H_
#define _UDI_TMC_H_

#include "usb_protocol_tmc.h"
#include "conf_usb.h"
#include "usb_protocol.h"
#include "udd.h"
#include "udc_desc.h"
#include "udi.h"

#ifdef __cplusplus
extern "C" {
#endif

// Configuration Full Speed check
#ifndef UDI_TMC_EPS_SIZE_INT_FS
# error UDI_TMC_EPS_SIZE_INT_FS must be defined in conf_usb.h file.
#endif
#ifndef UDI_TMC_EPS_SIZE_BULK_FS
# error UDI_TMC_EPS_SIZE_BULK_FS must be defined in conf_usb.h file.
#endif
#ifndef UDI_TMC_EPS_SIZE_ISO_FS
# error UDI_TMC_EPS_SIZE_ISO_FS must be defined in conf_usb.h file.
#endif

// Configuration High Speed check
#ifdef USB_DEVICE_HS_SUPPORT
# ifndef UDI_TMC_EPS_SIZE_INT_HS
#   error UDI_TMC_EPS_SIZE_INT_HS must be defined in conf_usb.h file.
# endif
# ifndef UDI_TMC_EPS_SIZE_BULK_HS
#   error UDI_TMC_EPS_SIZE_BULK_HS must be defined in conf_usb.h file.
# endif
# ifndef UDI_TMC_EPS_SIZE_ISO_HS
#   error UDI_TMC_EPS_SIZE_ISO_HS must be defined in conf_usb.h file.
# endif
#endif

/**
 * \addtogroup udi_tmc_group_udc
 * @{
 */
//! Global structure which contains standard UDI interface for UDC
extern UDC_DESC_STORAGE udi_api_t udi_api_tmc;
//@}

/**
 * \ingroup udi_tmc_group
 * \defgroup udi_tmc_group_desc USB interface descriptors
 *
 * The following structures provide predefined USB interface descriptors.
 * It must be used to define the final USB descriptors.
 */
//@{

/**
 * \name Endpoint descriptors
 * @{
 */

# define UDI_TMC_EPS_BULK_DESC \
   .ep_bulk_in.bLength                = sizeof(usb_ep_desc_t),\
   .ep_bulk_in.bDescriptorType        = USB_DT_ENDPOINT,\
   .ep_bulk_in.bEndpointAddress       = UDI_TMC_EP_BULK_IN,\
   .ep_bulk_in.bmAttributes           = USB_EP_TYPE_BULK,\
   .ep_bulk_in.bInterval              = 0,\
   .ep_bulk_out.bLength               = sizeof(usb_ep_desc_t),\
   .ep_bulk_out.bDescriptorType       = USB_DT_ENDPOINT,\
   .ep_bulk_out.bEndpointAddress      = UDI_TMC_EP_BULK_OUT,\
   .ep_bulk_out.bmAttributes          = USB_EP_TYPE_BULK,\
   .ep_bulk_out.bInterval             = 0,

# define UDI_TMC_EPS_BULK_DESC_FS \
   .ep_bulk_in.wMaxPacketSize         = LE16(UDI_TMC_EPS_SIZE_BULK_FS),\
   .ep_bulk_out.wMaxPacketSize        = LE16(UDI_TMC_EPS_SIZE_BULK_FS),

# define UDI_TMC_EPS_BULK_DESC_HS \
   .ep_bulk_in.wMaxPacketSize         = LE16(UDI_TMC_EPS_SIZE_BULK_HS),\
   .ep_bulk_out.wMaxPacketSize        = LE16(UDI_TMC_EPS_SIZE_BULK_HS),

//@}

//! Interface descriptor structure for TMC Class interface
typedef struct {
   usb_iface_desc_t iface0;
   usb_ep_desc_t ep_bulk_in;
   usb_ep_desc_t ep_bulk_out;
} udi_tmc_desc_t;

//! By default no string associated to this interface
#ifndef UDI_TMC_STRING_ID
#define UDI_TMC_STRING_ID     0
#endif

//! Maximum 4 endpoints used by TMC interface
#define UDI_TMC_EP_NB_INT  ((UDI_TMC_EPS_SIZE_INT_FS)?2:0)
#define UDI_TMC_EP_NB_BULK ((UDI_TMC_EPS_SIZE_BULK_FS)?2:0)
#define UDI_TMC_EP_NB_ISO  ((UDI_TMC_EPS_SIZE_ISO_FS)?2:0)
#define UDI_TMC_EP_NB      (UDI_TMC_EP_NB_INT+UDI_TMC_EP_NB_BULK+UDI_TMC_EP_NB_ISO)


//! Content of TMC interface descriptor for all speeds
#define UDI_TMC_DESC      \
   .iface0.bLength            = sizeof(usb_iface_desc_t),\
   .iface0.bDescriptorType    = USB_DT_INTERFACE,\
   .iface0.bInterfaceNumber   = UDI_TMC_IFACE_NUMBER,\
   .iface0.bAlternateSetting  = 0,\
   .iface0.bNumEndpoints      = 2,\
   .iface0.bInterfaceClass    = TMC_CLASS,\
   .iface0.bInterfaceSubClass = TMC_SUBCLASS,\
   .iface0.bInterfaceProtocol = TMC_PROTOCOL,\
   .iface0.iInterface         = UDI_TMC_STRING_ID,\
   UDI_TMC_EPS_BULK_DESC \

//! Content of TMC interface descriptor for full speed only
#define UDI_TMC_DESC_FS \
   {\
   UDI_TMC_DESC \
   UDI_TMC_EPS_BULK_DESC_FS \
   }

//! Content of TMC interface descriptor for high speed only
#define UDI_TMC_DESC_HS \
   {\
   UDI_TMC_DESC \
   UDI_TMC_EPS_BULK_DESC_HS \
   }
//@}


/**
 * \ingroup udi_group
 * \defgroup udi_tmc_group USB Device Interface (UDI) for TMC Class
 *
 * Common APIs used by high level application to use this USB class.
 *
 * These routines are used to transfer data to/from USB TMC endpoints.
 *
 * See \ref udi_tmc_quickstart.
 * @{
 */

#if UDI_TMC_EPS_SIZE_BULK_FS || defined(__DOXYGEN__)
/**
 * \brief Start a transfer on bulk IN
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback is called.
 * The \a callback returns the transfer status and eventually the number of byte transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool udi_tmc_bulk_in_run(uint8_t * buf, iram_size_t buf_size, udd_callback_trans_t callback);

/**
 * \brief Start a transfer on bulk OUT
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback is called.
 * The \a callback returns the transfer status and eventually the number of byte transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool udi_tmc_bulk_out_run(uint8_t * buf, iram_size_t buf_size, udd_callback_trans_t callback);

/** \brief Receive and parse the next command sent by the host to the BulkOUT
 *         endpoint
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
#define UDI_TMC_RECEIVE_BULKOUT_COMMAND() udi_tmc_bulk_out_run(NULL, 0, NULL);


#endif

//@}

#ifdef __cplusplus
}
#endif
#endif // _UDI_TMC_H_
