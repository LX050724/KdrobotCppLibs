#ifndef KDROBOTCPPLIBS_REALSENSE_H
#define KDROBOTCPPLIBS_REALSENSE_H

#include <QThread>
#include <QMutex>
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include "spdlogger.h"
#include "JsonConfig.h"
#include "RealsenseFrame.h"

class Realsense : public QThread {
Q_OBJECT
    QMutex mut;
    QMutex ready;
    spdlogger logger;
    JsonConfig config;
    volatile bool running = true;
    RealsenseFrame buffer;
    rs2_intrinsics depth_intrinsics, color_intrinsics;

public:
    Realsense(JsonConfig _config, QObject *parent = nullptr) :
            QThread(parent), logger(__FUNCTION__), config(_config) {
        (void) ready.try_lock();
        this->start();
    }

    ~Realsense();

    const RealsenseFrame &getFrame();

    //TODO 后期可能会修改
    static cv::Point3f deproject_pixel_to_point(const rs2_intrinsics &intrin, const cv::Point2f &pixel, float depth);

    inline const rs2_intrinsics &getDepthIntrinsics() const {
        return depth_intrinsics;
    }

    inline const rs2_intrinsics &getColorIntrinsics() const {
        return color_intrinsics;
    }

protected:

    void run() override;

    std::string get_device_name(const rs2::device &dev);

    std::string get_sensor_name(const rs2::sensor &sensor);
};


#endif //KDROBOTCPPLIBS_REALSENSE_H
