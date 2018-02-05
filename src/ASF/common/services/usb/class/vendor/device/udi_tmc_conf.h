#ifndef _UDI_TMC_CONF_H_
#define _UDI_TMC_CONF_H_

#include "conf_usb.h"

/**
 * \addtogroup udi_tmc_group_single_desc
 * @{
 */

//! Control endpoint size
#define  USB_DEVICE_EP_CTRL_SIZE       64

//! Endpoint numbers used by TMC interface.
//! Note: The order of endpoint can depend on USB hardware capability
//! when a specific mapping is used on USB DPRAM.
#if SAM3S || SAM4S || SAM4E
#  define  UDI_TMC_EP_INTERRUPT_IN  (3 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_INTERRUPT_OUT (6 | USB_EP_DIR_OUT)
#  define  UDI_TMC_EP_BULK_IN       (1 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_BULK_OUT      (2 | USB_EP_DIR_OUT)
#  define  UDI_TMC_EP_ISO_IN        (4 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_ISO_OUT       (5 | USB_EP_DIR_OUT)
#elif SAM3U
#  define  UDI_TMC_EP_INTERRUPT_IN  (3 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_INTERRUPT_OUT (4 | USB_EP_DIR_OUT)
#  define  UDI_TMC_EP_BULK_IN       (1 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_BULK_OUT      (2 | USB_EP_DIR_OUT)
#  define  UDI_TMC_EP_ISO_IN        (5 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_ISO_OUT       (6 | USB_EP_DIR_OUT)
#elif SAM3XA
#  define  UDI_TMC_EP_INTERRUPT_IN  (3 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_INTERRUPT_OUT (4 | USB_EP_DIR_OUT)
#  define  UDI_TMC_EP_BULK_IN       (5 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_BULK_OUT      (6 | USB_EP_DIR_OUT)
#  define  UDI_TMC_EP_ISO_IN        (1 | USB_EP_DIR_IN)
#  define  UDI_TMC_EP_ISO_OUT       (2 | USB_EP_DIR_OUT)
#elif SAMG55
#define  UDI_TMC_EP_INTERRUPT_IN  (1 | USB_EP_DIR_IN)
#define  UDI_TMC_EP_INTERRUPT_OUT (2 | USB_EP_DIR_OUT)
#define  UDI_TMC_EP_BULK_IN       (3 | USB_EP_DIR_IN)
#define  UDI_TMC_EP_BULK_OUT      (4 | USB_EP_DIR_OUT)
#else
#define  UDI_TMC_EP_BULK_IN  (1 | USB_EP_DIR_IN)
#define  UDI_TMC_EP_BULK_OUT (2 | USB_EP_DIR_OUT)
#endif

//! Interface number is 0 because it is the unique interface
#define  UDI_TMC_IFACE_NUMBER 0

/**
 * \name UDD Configuration
 */
//@{
//! Maximum 6 endpoints used by TMC interface
#define UDI_TMC_EP_NB_INT  ((UDI_TMC_EPS_SIZE_INT_FS)?2:0)
#define UDI_TMC_EP_NB_BULK ((UDI_TMC_EPS_SIZE_BULK_FS)?2:0)
#define UDI_TMC_EP_NB_ISO  ((UDI_TMC_EPS_SIZE_ISO_FS)?2:0)
#undef USB_DEVICE_MAX_EP   // undefine this definition in header file
#define USB_DEVICE_MAX_EP     (UDI_TMC_EP_NB_INT+UDI_TMC_EP_NB_BULK+UDI_TMC_EP_NB_ISO)
//@}

//@}

#include "udi_tmc.h"

#endif // _UDI_TMC_CONF_H_
