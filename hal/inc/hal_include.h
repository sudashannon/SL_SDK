#ifndef __HAL_INCLUDE_H
#define __HAL_INCLUDE_H

#include "hal_com.h"
#include "hal_gpio.h"
#include "hal_key.h"
#include "hal_spi.h"
#include "hal_qspi.h"

/**
 * @brief Should be called firstly to ensure all defined devices can be used.
 *
 */
extern rte_error_t hal_init(void);

#endif
