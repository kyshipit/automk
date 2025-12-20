/**
 * @file log_client.h
 * @brief Logging client API for autoMLOS (using libipc)
 * 
 * Provides client-side interface for communicating with the logger service
 * through libipc, maintaining proper module decoupling and eliminating IPC naming.
 */

#ifndef LOG_CLIENT_H
#define LOG_CLIENT_H

#include "../safety/system_errors.h"
#include "log_common.h"

// Client API for service communication using libipc
system_error_t log_client_send_entry(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                                   uint32_t data1, uint32_t data2);
system_error_t log_client_get_status(void);
system_error_t log_client_init(void);

// Enhanced client functions with timeout support
system_error_t log_client_send_entry_timeout(uint8_t level, uint8_t tag_id, uint16_t message_id,
                                           uint32_t data1, uint32_t data2, uint32_t timeout_ms);
system_error_t log_client_get_status_timeout(uint32_t timeout_ms);

#endif // LOG_CLIENT_H