#ifndef KDROBOTCPPLIBS_OPENCV_UTIL_H
#define KDROBOTCPPLIBS_OPENCV_UTIL_H

#include <opencv2/opencv.hpp>

template<typename T>
float inline distenceP2P(const cv::Point_<T> &a, const cv::Point_<T> &b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

template<typename T>
float inline distenceP2L(const cv::Point_<T> &p, const cv::Point_<T> &a, const cv::Point_<T> &b) {
    float l = (b.y - a.y) / (b.x - a.x);
    return fabs(l * p.x - p.y + a.y - l * a.x) / sqrt(l * l + 1);
}

template<typename T>
float inline lineAngle(const cv::Point_<T> &a, const cv::Point_<T> &b) {
    cv::Point_<T> l = b - a;
    return std::atan2(l.y, l.x) * 360 / CV_2PI;
}

void drawRotatedRect(cv::InputOutputArray Img, cv::RotatedRect rect, const cv::Scalar &color,
              int thickness = 1, int lineType = cv::LINE_8, int shift = 0);

void HChannleOffsetInRange(const cv::Mat &Input, int offset, const cv::Scalar &lowerb,
                           const cv::Scalar &upperb, cv::Mat &dst);

#endif
