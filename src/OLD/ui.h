#ifndef UI_H_
#define UI_H_

#include <asf.h>

//! \brief Initializes the user interface
void ui_init(void);

//! \brief Enters the user interface in power down mode
void ui_powerdown(void);

//! \brief Exits the user interface of power down mode
void ui_wakeup(void);

/*! \brief Notify the state of loopback
 * It is called when a the loopback is started and stopped.
 *
 * \param b_started    loopback started if true, else stopped
 */
void ui_loop_back_state(bool b_started);

/*! \brief This process is called each 1ms
 * It is called only if the USB interface is enabled.
 *
 * \param framenumber  Current frame number
 */
void ui_process(uint16_t framenumber);

#endif