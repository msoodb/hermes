#include "hrms_config.h"

#if BLFM_ENABLED_BIGSOUND

#ifndef BLFM_BIGSOUND_H
#define BLFM_BIGSOUND_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

#include "hrms_types.h"

void hrms_bigsound_init(QueueHandle_t controller_queue);
void hrms_bigsound_isr_handler(void);

#endif // BLFM_BIGSOUND_H

#endif /* BLFM_ENABLED_BIGSOUND */
