#ifndef STICKDISCERN_H
#define STICKDISCERN_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/core/types.hpp>
#include <opencv2/freetype.hpp>
#include <time.h>
#include <iostream>
#include <queue>

using namespace cv;
using namespace std;

struct StickDiscernResult
{
    int totalArea;
    int frameworkArea;
    int headArea;
    int tailArea;
    bool isValid;
    Point2f head;
    Point2f tail;
};

struct lineFunc
{
    float k;
    float b;
    bool hasSlope;
};

struct endPoints
{
    Point2f end1;
    Point2f end2;
};

class StickDiscern
{
protected:
    unsigned char m_BinarizationThreshold; // 二值化阈值
    double m_radio; // 识别比例
    // Mat m_thinImg;
    Mat m_binarizationImg;
    Mat m_borderImg;
    Mat m_resultImg1;
    Mat m_resultImg2;
    Mat m_resultImg;
    Mat m_frameworkImg;
    cv::Ptr<cv::freetype::FreeType2> ft2;
    int thickness;
    int linestyle;
    bool okngflag;
    int fd ;
    float RuleValue;
    endPoints edp;
    int totalArea;
    int frameworkArea;
    int headArea;
    int tailArea;
    int imgcount;

    int area(Mat& src);
    bool getLineRatioPoint(Mat& src, Point2f& begin, Point2f& output, Point2f& inAreaPoint, Point2f& frontPoint);
    bool getLineRatioPoint(Mat& src, Point2f& begin, double radio, Point2f& output, Point2f& inAreaPoint, Point2f& frontPoint);
    void getLineSlop(Point2f pb, Point2f pe, lineFunc& output);
    void getVerticalLine(Point2f linePoint, lineFunc& input, lineFunc& output);
    float getFuncResult(const lineFunc& func, const float& x, bool x2y=false);
    void drawResultToImg(Mat& src, Mat& dst, const StickDiscernResult& result);
    void DrawCrossLine(Mat& src, const lineFunc& func);
    bool init(Mat& src);
public:
    StickDiscern(unsigned char binarization_threshold, double radio);
    void setBinarizationThreshold(unsigned char binarization_threshold);
    void setRadio(double radio);
    endPoints thinImage(Mat& src, Mat& dst);
    StickDiscernResult result(Mat& src);
    Mat& result();
    void drawResultToImg(Mat& src, const StickDiscernResult& result);
    float getRuler(Mat& src_gray);
    void saveimage(Mat& imgsrc);
    Mat m_thinImg;
};

#endif // STICKDISCERN_H
