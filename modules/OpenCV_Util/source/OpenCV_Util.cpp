#include "../include/OpenCV_Util.h"
#include "omp.h"

void drawRotatedRect(cv::InputOutputArray Img, cv::RotatedRect rect,
                     const cv::Scalar &color, int thickness, int lineType, int shift) {
    cv::Point2f points[4];
    rect.points(points);
    for (int k = 0; k < 4; ++k)
        cv::line(Img, points[k], points[(k + 1) % 4], color, thickness, lineType, shift);
}

void HChannleOffsetInRange(const cv::Mat &Input, int offset, const cv::Scalar &lowerb,
                           const cv::Scalar &upperb, cv::Mat &dst) {
    int nr = Input.rows;
    // 将3通道转换为1通道
    int chs = Input.channels();
    int nl = Input.cols * chs;
    dst.create(Input.rows, Input.cols, CV_8U);

    int16_t H_Upper = upperb[0] + offset, H_Lower = lowerb[0] + offset;
    int16_t S_Upper = upperb[1], S_Lower = lowerb[1];
    int16_t V_Upper = upperb[2], V_Lower = lowerb[2];
#pragma omp parallel for
    for (int k = 0; k < nr; k++) {
        // 每一行图像的指针
        const uchar *inData = Input.ptr<uchar>(k);
        uchar *outData = dst.ptr<uchar>(k);
#pragma omp parallel for
        for (int i = 0; i < nl; i += 3) {
            int16_t H = ((int16_t) inData[i] + offset) % 180;
            uint8_t S = inData[i + 1];
            uint8_t V = inData[i + 2];
            outData[i / 3] = (H_Lower <= H && H <= H_Upper &&
                              S_Lower <= S && S <= S_Upper &&
                              V_Lower <= V && V <= V_Upper) ? 0xff : 0x00;
        }
    }
}
