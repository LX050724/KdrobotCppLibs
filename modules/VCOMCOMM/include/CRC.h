#ifndef KDROBOTCPPLIBS_CRC_H
#define KDROBOTCPPLIBS_CRC_H

#include <vector>
#include <stdint.h>

class CRC {
private:
    /**
     * @brief Descriptions: CRC8 checksum function
     * @param pchMessage Data to check
     * @param dwLength Stream length
     * @param wCRC initialized checksum
     * @return CRC checksum
     */
    static uint8_t Get_CRC8_Check_Sum(uint8_t *pchMessage, uint8_t dwLength, uint8_t ucCRC8);

    /**
     * @brief Descriptions: CRC16 checksum function
     * @param pchMessage Data to check
     * @param dwLength Stream length
     * @param wCRC initialized checksum
     * @return CRC checksum
     */
    static uint16_t Get_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength, uint16_t wCRC);

public:
    /**
     * @brief Descriptions: CRC8 Verify function
     * @param pchMessage Data to Verify
     * @param dwLength Stream length = Data + checksum
     * @return True or False (CRC Verify Result)
     */
    static uint8_t Verify_CRC8_Check_Sum(uint8_t *pchMessage, uint8_t dwLength);

    static inline uint8_t Verify_CRC8_Check_Sum(std::vector<uint8_t> pchMessage) {
        return Verify_CRC8_Check_Sum(pchMessage.data(), (uint8_t) pchMessage.size());
    }

    /**
     * @brief CRC16 Verify function
     * @param pchMessage Data to Verify
     * @param dwLength Stream length = Data + checksum
     * @return True or False (CRC Verify Result)
     */
    static uint16_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);

    static inline uint16_t Verify_CRC16_Check_Sum(std::vector<uint8_t> pchMessage) {
        return Verify_CRC16_Check_Sum(pchMessage.data(), (uint8_t) pchMessage.size());
    }
};


#endif //VCOMCOMM_PC_CRC_H
