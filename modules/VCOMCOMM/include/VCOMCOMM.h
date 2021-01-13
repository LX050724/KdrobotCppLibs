/**
 * @file VCOMCOMM.h
 * @author yao
 * @date 2021年1月13日
 */

#ifndef KDROBOTCPPLIBS_VCOMCOMM_PC_H
#define KDROBOTCPPLIBS_VCOMCOMM_PC_H

#include <QSerialPort>
#include <spdlog/spdlog.h>

class VCOMCOMM : public QSerialPort {
Q_OBJECT
private:
    uint16_t pid, vid;
    Qt::HANDLE thread_id;
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<spdlog::logger> err_logger;
public:

    /**
     * @brief 构造函数,构造时自动搜索对应PID和VID的USB串口设备
     * @param PID 产品ID
     * @param VID 制造商ID
     */
    VCOMCOMM(uint16_t PID = 22336, uint16_t VID = 1155);


    /**
     * @brief 自动连接对应PID和VID的USB串口设备
     * @return 自动连接是否成功
     */
    bool auto_connect();

protected:
    void loggerFactory();

protected slots:
    void portReadyRead();
    void portErrorOccurred(QSerialPort::SerialPortError error);

public slots:
    /**
     * @brief 发送消息
     * @param fun_code 功能码
     * @param id 消息ID
     * @param data 数据
     */
    void Transmit(uint8_t fun_code, uint16_t id, const std::vector<uint8_t> &data);

signals:
    /**
     * @brief 收到数据包信号量
     * @param fun_code 数据包功能码
     * @param id 数据包ID
     * @param data 数据
     */
    void receiveData(uint8_t fun_code, uint16_t id, std::vector<uint8_t> data);

    /**
     * @brief 通过信号量实现跨线程发送
     * @param fun_code 功能码
     * @param id 消息ID
     * @param data 数据
     */
    void CrossThreadTransmitSignal(uint8_t fun_code, uint16_t id, const std::vector<uint8_t> &data);
};


#endif
