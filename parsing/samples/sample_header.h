/**
 * @file sample_header.h
 * @brief Sample embedded C header file with various constructs
 * @author AST Parser Demo
 */

#ifndef SAMPLE_HEADER_H
#define SAMPLE_HEADER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Status codes for API functions
 */
typedef enum {
    STATUS_OK = 0,        /**< Operation completed successfully */
    STATUS_ERROR = -1,    /**< General error occurred */
    STATUS_TIMEOUT = -2,  /**< Operation timed out */
    STATUS_INVALID = -3   /**< Invalid parameter */
} StatusCode_t;

/**
 * @brief Configuration structure for the device
 */
typedef struct {
    uint8_t deviceId;     /**< Unique device identifier */
    uint16_t timeout_ms;  /**< Timeout in milliseconds */
    bool enableLogging;   /**< Enable debug logging */
} DeviceConfig_t;

/**
 * @brief Device state information
 */
typedef struct DeviceState {
    uint8_t status;       /**< Current device status */
    uint16_t errorCount;  /**< Number of errors encountered */
    
    /* Internal state variables */
    uint32_t lastUpdateTime;
    uint8_t reserved[4];
} DeviceState_t;

/**
 * @brief Maximum number of devices supported
 */
#define MAX_DEVICES 16

/**
 * @brief Helper macro for checking status codes
 * @param x The status code to check
 * @return true if status is OK, false otherwise
 */
#define IS_STATUS_OK(x) ((x) == STATUS_OK)

#ifdef DEBUG_MODE
/**
 * @brief Debug print macro
 */
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) ((void)0)
#endif

/**
 * @brief Initialize the device with the given configuration
 * @param config Pointer to device configuration
 * @return Status code indicating success or failure
 */
StatusCode_t Device_Init(const DeviceConfig_t* config);

/**
 * @brief Reset the device to default state
 * @return Status code indicating success or failure
 */
StatusCode_t Device_Reset(void);

/**
 * @brief Read the current device state
 * @param state Pointer to store the device state
 * @return Status code indicating success or failure
 */
StatusCode_t Device_GetState(DeviceState_t* state);

/**
 * @brief Process incoming data from the device
 * @param data Pointer to input data buffer
 * @param size Size of the data buffer in bytes
 * @param processed Pointer to store number of bytes processed
 * @return Status code indicating success or failure
 */
StatusCode_t Device_ProcessData(
    const uint8_t* data,
    uint16_t size,
    uint16_t* processed
);

/* Internal function - not part of public API */
void _Device_UpdateInternalState(void);

#endif /* SAMPLE_HEADER_H */



