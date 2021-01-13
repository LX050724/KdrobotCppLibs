/**
 * @file OpenCV_Util.h
 * @author yao
 * @date 2021年1月13日
 * @brief OpenCV常用算法整理
 */

#ifndef KDROBOTCPPLIBS_OPENCV_UTIL_H
#define KDROBOTCPPLIBS_OPENCV_UTIL_H

#include <opencv2/opencv.hpp>

/**
 * 点到点距离
 * @tparam T 点数据类型
 * @param a 点A
 * @param b 点B
 * @return 两点间距离
 */
template<typename T>
auto inline distenceP2P(const cv::Point_<T> &a, const cv::Point_<T> &b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

/**
 * 点到线距离
 * @tparam T 点数据类型
 * @param p 点
 * @param a 线端点A
 * @param b 线端点B
 * @return 点到线距离
 */
template<typename T>
auto inline distenceP2L(const cv::Point_<T> &p, const cv::Point_<T> &a, const cv::Point_<T> &b) {
    float l = (b.y - a.y) / (b.x - a.x);
    return fabs(l * p.x - p.y + a.y - l * a.x) / sqrt(l * l + 1);
}

/**
 * 线角度
 * @tparam T 点数据类型
 * @param a 线端点A
 * @param b 线端点B
 * @return 线角度
 */
template<typename T>
auto inline lineAngle(const cv::Point_<T> &a, const cv::Point_<T> &b) {
    cv::Point_<T> l = b - a;
    return std::atan2(l.y, l.x) * 360 / CV_2PI;
}

/**
 * 画旋转矩形
 * @param Img 图像
 * @param rect 旋转矩形
 * @param color 颜色
 * @param thickness 线粗
 * @param lineType 线类型
 * @param shift 坐标点的小数点位数
 */
void drawRotatedRect(cv::InputOutputArray Img, cv::RotatedRect rect, const cv::Scalar &color,
              int thickness = 1, int lineType = cv::LINE_8, int shift = 0);

/**
 * 通道一带有偏置的InRange，带有OpenMP多线程加速
 * @note 针对解决HSV颜色空间中红色在0附近不好判断的问题，HSV的H通道范围[0-180)
 * @param Input 输入图像
 * @param offset 通道一偏置
 * @param lowerb 下限
 * @param upperb 上线
 * @param dst 输出图像
 */
void HChannleOffsetInRange(const cv::Mat &Input, int offset, const cv::Scalar &lowerb,
                           const cv::Scalar &upperb, cv::Mat &dst);

#endif
