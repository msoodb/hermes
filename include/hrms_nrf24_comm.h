/*
 * Copyright (C) 2025 Masoud Bolhassani <masoud.bolhassani@gmail.com>
 *
 * This file is part of Hermes.
 *
 * Hermes is released under the GNU General Public License v3 (GPL-3.0).
 * See LICENSE file for details.
 */

#ifndef HRMS_NRF24_COMM_H
#define HRMS_NRF24_COMM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file hrms_nrf24_comm.h
 * @brief nRF24L01 communication module following clean sensor/actuator pattern
 * 
 * This module provides a clean interface for nRF24L01 radio communication,
 * following the same pattern as sensor and actuator modules.
 */

/**
 * Initialize nRF24L01 communication module
 * @return true if initialization successful, false otherwise
 */
bool hrms_nrf24_comm_init(void);

/**
 * Send data via nRF24L01
 * @param data Pointer to data to send
 * @param len Length of data in bytes
 * @return true if send successful, false otherwise
 */
bool hrms_nrf24_comm_send(const uint8_t *data, size_t len);

/**
 * Receive data via nRF24L01
 * @param data Buffer to store received data
 * @param max_len Maximum buffer size
 * @param received_len Pointer to store actual received length
 * @return true if data received, false otherwise
 */
bool hrms_nrf24_comm_receive(uint8_t *data, size_t max_len, size_t *received_len);

/**
 * Process nRF24L01 periodic tasks
 * Should be called regularly from communication hub task
 */
void hrms_nrf24_comm_process(void);

#endif /* HRMS_NRF24_COMM_H */