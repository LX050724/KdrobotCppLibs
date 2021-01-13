/**
 * @file CRC.h
 * @author yao
 * @date 2021年1月13日
 * @brief Keil使用的纯C语言实现的CRC
 */

#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

/**
 * @brief Descriptions: CRC8 Verify function
 * @param pchMessage Data to Verify
 * @param dwLength Stream length = Data + checksum
 * @return True or False (CRC Verify Result)
 */
uint8_t Verify_CRC8_Check_Sum(uint8_t *pchMessage, uint8_t dwLength);


/**
 * @brief CRC16 Verify function
 * @param pchMessage Data to Verify
 * @param dwLength Stream length = Data + checksum
 * @return True or False (CRC Verify Result)
 */
uint16_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);

#endif
