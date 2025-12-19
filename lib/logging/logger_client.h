/**
 * @file logger_client.h
 * @brief Logger client API for autoMLOS
 * 
 * Provides client-side interface for communicating with the logger service
 * through IPC, maintaining proper module decoupling.
 */

#ifndef LOGGER_CLIENT_H
#define LOGGER_CLIENT_H

#include "../safety/system_errors.h"
#include "log_common.h"

// Client API for service communication
system_error_t logger_client_send_entry(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                                       uint32_t data1, uint32_t data2);
system_error_t logger_client_get_status(void);
system_error_t logger_client_init(void);

#endif // LOGGER_CLIENT_H