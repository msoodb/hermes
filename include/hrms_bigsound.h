#include "hrms_config.h"

#if HRMS_ENABLED_BIGSOUND

#ifndef HRMS_BIGSOUND_H
#define HRMS_BIGSOUND_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

#include "hrms_types.h"

void hrms_bigsound_init(QueueHandle_t controller_queue);
void hrms_bigsound_isr_handler(void);

#endif // HRMS_BIGSOUND_H

#endif /* HRMS_ENABLED_BIGSOUND */
