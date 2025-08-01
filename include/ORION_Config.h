/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef ORION_CONFIG_H
#define ORION_CONFIG_H

/**
 * @file ORION_Config.h
 * @brief Configuration file for ORION submodule integration with Hermes
 * 
 * This file provides configuration settings for integrating the ORION
 * submodule with the Hermes joystick controller system.
 * 
 * NOTE: The ORION submodule should only be updated from its own project
 * directory, not from within the Hermes project.
 */

//==============================================================================
// ORION INTEGRATION SETTINGS
//==============================================================================

/**
 * Enable ORION integration in Hermes
 * Set to 1 to enable ORION features, 0 to disable
 */
#define ORION_INTEGRATION_ENABLED 1

/**
 * ORION communication interface configuration
 */
#define ORION_USE_UART          1    // Enable UART communication
#define ORION_USE_SPI           0    // Disable SPI communication  
#define ORION_USE_I2C           0    // Disable I2C communication

/**
 * ORION buffer sizes
 */
#define ORION_RX_BUFFER_SIZE    256  // Receive buffer size
#define ORION_TX_BUFFER_SIZE    256  // Transmit buffer size

/**
 * ORION timing configuration
 */
#define ORION_TIMEOUT_MS        1000 // Communication timeout in milliseconds
#define ORION_RETRY_COUNT       3    // Number of retry attempts

/**
 * ORION debug configuration
 */
#define ORION_DEBUG_ENABLED     1    // Enable debug output
#define ORION_VERBOSE_LOGGING   0    // Disable verbose logging

//==============================================================================
// HERMES-SPECIFIC ORION SETTINGS
//==============================================================================

/**
 * Integration with Hermes joystick system
 */
#define ORION_JOYSTICK_INTEGRATION  1    // Enable joystick data forwarding to ORION
#define ORION_RADIO_INTEGRATION     1    // Enable ORION radio coordination

/**
 * ORION task priority in Hermes FreeRTOS system
 */
#define ORION_TASK_PRIORITY         2    // Medium priority
#define ORION_TASK_STACK_SIZE       512  // Stack size in words

/**
 * ORION queue configuration for Hermes integration
 */
#define ORION_COMMAND_QUEUE_SIZE    10   // Command queue depth
#define ORION_RESPONSE_QUEUE_SIZE   10   // Response queue depth

#endif /* ORION_CONFIG_H */