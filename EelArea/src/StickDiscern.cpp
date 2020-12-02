#include "StickDiscern.h"
// #include "wiringSerial.h"
#include <unistd.h>
#include <fcntl.h>
#include "dirent.h"

StickDiscern::StickDiscern(unsigned char binarization_threshold, double radio): m_BinarizationThreshold(binarization_threshold), m_radio(radio)
{
    okngflag = true;

    imgcount = 0;

 /*   string text="温度:20 湿度:29 PM2.5:60 空气良好";
    int fontHeight = 40;
    thickness  = -1;
    linestyle  = 8;
    int baseline   = 0;

    ft2=cv::freetype::createFreeType2();
    ft2->loadFontData("./simkai.ttf",0);

    cv::Size textSize=ft2->getTextSize(text,fontHeight,thickness,&baseline);
    if (thickness>0) baseline+=thickness;
*/

    printf("ttyS4\n");

/*   if ((fd = serialOpen ("/dev/ttyS4", 115200)) < 0){
      fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    }
    // test send
    char Cmdbegin[7] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x09};
    //char Cmdbegin[30] = "StickDiscern begin run !!!";
    // serialPutsLen(fd, Cmdbegin, 7);
    serialPuts(fd, Cmdbegin);
*/

    printf("ttyS4_2\n");

    //save to file
    if(access("./rulefile.txt", F_OK) != -1){
        FILE * ruleFile2 = fopen("./rulefile.txt", "r");
        int id = fread(&RuleValue, sizeof(float), 1, ruleFile2);
        printf("=================>>read Rule = %f,  id=%d\n", RuleValue, id);
        fclose(ruleFile2);
    }
}

void StickDiscern::setBinarizationThreshold(unsigned char binarization_threshold)
{
    m_BinarizationThreshold = binarization_threshold;
}

void StickDiscern::setRadio(double radio)
{
    m_radio = radio;
}

int StickDiscern::area(Mat& src)
{
    int area = 0;
    int width = src.cols;
    int height = src.rows;
    for (int i = 0; i < height; i++)
    {
        uchar *p = src.ptr<uchar>(i);
        for (int j = 0; j < width; j++)
        {
            if(*p++ == 1)
            {
                area++;
            }
        }
    }

    return area;
}

void StickDiscern::getLineSlop(Point2f pb, Point2f pe, lineFunc& output)
{
    if(pe.x != pb.x)
    {
        output.k = (pe.y - pb.y) / (pe.x - pb.x);
        output.hasSlope = true;
        output.b = pb.y - output.k * pb.x;      
    }
    else
    {
        output.k = 0;
        output.hasSlope = false;
        output.b = pe.x;
    }
}

void StickDiscern::getVerticalLine(Point2f linePoint, lineFunc& input, lineFunc& output)
{
    if(input.hasSlope)
    {
        if(input.k == 0)
        {
            output.b = linePoint.x;
            output.hasSlope = false;
        }
        else
        {
            output.k = 1.0 / input.k * -1;
            output.b = linePoint.y - output.k * linePoint.x;
            output.hasSlope = true;
        }
    }
    else
    {
        output.k = 0;
        output.b = linePoint.y;
        output.hasSlope = true;
    }
}

float StickDiscern::getFuncResult(const lineFunc& func, const float& x, bool x2y)
{
    if(x2y)
    {
        if(func.hasSlope)
        {
            return (x - func.b) / func.k;
        }
        else
        {
            return func.b;
        }
    }
    else
    {
        if(func.hasSlope)
        {
            return func.k * x + func.b;
        }
        else
        {
            return 900000000;
        }
    }
}

void StickDiscern::DrawCrossLine(Mat& src, const lineFunc& func)
{
    int width = src.cols;
    int height = src.rows;

    int pointAarray[5] = {0};
    cout << func.hasSlope << endl;
    if(func.hasSlope)
    {
        if(func.k == 0)
        {
            pointAarray[0] = 0;
            pointAarray[1] = func.b;
            pointAarray[2] = width;
            pointAarray[3] = func.b;
        }
        else
        {
            int x_0 = getFuncResult(func, 0, true), \
                y_0 = getFuncResult(func, 0), \
                x_max = getFuncResult(func, height, true), \
                y_max = getFuncResult(func, width);
			
			cout << "x_0=" << x_0 << endl;
			cout << "y_0=" << y_0 << endl;
			cout << "x_max=" << x_max << endl;
			cout << "y_max=" << y_max << endl;
            
			if(0 <= x_0 && x_0 <= width)
            {
                pointAarray[pointAarray[4]] = static_cast<int>(x_0);
                pointAarray[pointAarray[4] + 1] = 0;
                pointAarray[4] += 2;
            }

            if(0 <= y_0 && y_0 <= height)
            {
                pointAarray[pointAarray[4] + 1] = static_cast<int>(y_0);
                pointAarray[pointAarray[4]] = 0;
                pointAarray[4] += 2;
            }

            if(0 <= x_max && x_max <= width)
            {
                if(pointAarray[4] < 4)
                {
                    pointAarray[pointAarray[4]] = static_cast<int>(x_max);
                    pointAarray[pointAarray[4] + 1] = height;
                    pointAarray[4] += 2;
                }
            }

            if(0 <= y_max && y_max <= height)
            {
                if(pointAarray[4] < 4)
                {
                    pointAarray[pointAarray[4] + 1] = static_cast<int>(y_max);
                    pointAarray[pointAarray[4]] = width;
                    pointAarray[4] += 2;
                }
            }
        }
    }
    else
    {
        pointAarray[0] = func.b;
        pointAarray[1] = 0;
        pointAarray[2] = func.b;
        pointAarray[3] = height;
    }

    for(int i = 0; i<4; i++)
    {
        cout << pointAarray[i] << endl;
    }

    line(src, Point(pointAarray[0], pointAarray[1]), Point(pointAarray[2], pointAarray[3]), Scalar(1), 1);
}

bool StickDiscern::getLineRatioPoint(Mat& src, Point2f& begin, double radio, Point2f& output, Point2f& inAreaPoint, Point2f& frontPoint)
{
    int linearea = area(src);
    int count = 0;
    int direction = 0;
    int step = 15;
    queue<Point2f> surroundingPoints;

    Mat dst;
    src.copyTo(dst);
    int newPoint[10] = {0};
    int width = dst.cols;
    int height = dst.rows;
    uchar * pcurrentPoint = dst.ptr<uchar>(begin.y);
    int currentX = begin.x;
    int currentY = begin.y;
    cout << "=======linearea:" << linearea << endl;
    cout << "=======radio:" << radio << endl;
    printf("===========>>>>>>>>>>>>>>>>>>>>>>radio=%f\n", radio);

    // putText(src, "Begin", Point(Endpointx1, Endpointy1), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,255,255), 1, LINE_AA);
    while(count < (static_cast<int>(linearea * radio) + step))
    {
        newPoint[2] = (currentY == 0) ? 0 : *(pcurrentPoint - dst.step + currentX);
        newPoint[3] = (currentY == 0 || currentX == width - 1) ? 0 : *(pcurrentPoint - dst.step + currentX + 1);
        newPoint[4] = (currentX == width - 1) ? 0 : *(pcurrentPoint + currentX + 1);
        newPoint[5] = (currentY == height - 1 || currentX == width - 1) ? 0 : *(pcurrentPoint + dst.step + currentX + 1);
        newPoint[6] = (currentY == height - 1) ? 0 : *(pcurrentPoint + dst.step + currentX);
        newPoint[7] = (currentY == height - 1 || currentX == 0) ? 0 : *(pcurrentPoint + dst.step + currentX - 1);
        newPoint[8] = (currentX == 0) ? 0 : *(pcurrentPoint + currentX - 1);
        newPoint[9] = (currentY == 0 || currentX == 0) ? 0 : *(pcurrentPoint - dst.step + currentX - 1);

        // cout << newPoint[9] << newPoint[2] << newPoint[3] << endl;
        // cout << newPoint[8] << "1" << newPoint[4] << endl;
        // cout << newPoint[7] << newPoint[6] << newPoint[5] << endl;

        // cout << endl;

        for(int i=1; i<=4; i++)
        {
            if(newPoint[i*2] == 1)
            {
                direction = i*2;
                break;
            }
            else
            {
                direction = -1;
            }
        }

      //  printf("===========1>>>direction=%d\n", direction);
        if(direction < 0)
        {
            for(int i=2; i<=5; i++)
            {
                if(newPoint[i*2-1] == 1)
                {
                    direction = i*2-1;
                    break;
                }
                else
                {
                    direction = -1;
                }
            }
        }

     //   printf("===========2>>>direction=%d\n", direction);

        if(direction > 0)
        {
            dst.ptr<uchar>(currentY)[currentX] = 0;
            //cout << "direction: " << direction << endl;
            if(direction == 2)
            {
                currentY = currentY - 1;
                pcurrentPoint = dst.ptr<uchar>(currentY);
            }
            else if(direction == 3)
            {
                currentY = currentY - 1;
                pcurrentPoint = dst.ptr<uchar>(currentY);
                currentX = currentX + 1;
            }
            else if(direction == 4)
            {
                currentX = currentX + 1;
            }
            else if(direction == 5)
            {
                currentY = currentY + 1;
                pcurrentPoint = dst.ptr<uchar>(currentY);
                currentX = currentX + 1;
            }
            else if(direction == 6)
            {
                currentY = currentY + 1;
                pcurrentPoint = dst.ptr<uchar>(currentY);
            }
            else if(direction == 7)
            {
                currentY = currentY + 1;
                pcurrentPoint = dst.ptr<uchar>(currentY);
                currentX = currentX - 1;
            }
            else if(direction == 8)
            {
                currentX = currentX - 1;
            }
            else if(direction == 9)
            {
                currentY = currentY - 1;
                pcurrentPoint = dst.ptr<uchar>(currentY);
                currentX = currentX - 1;
            }

            if(count == static_cast<int>(linearea * radio))
            {
                output.x = currentX;
                output.y = currentY;
            }

            if(count <= static_cast<int>(linearea * radio))
            {
                if(surroundingPoints.size() < step + 1)
                {
                    surroundingPoints.push(Point2f(currentX, currentY));
                }
                else
                {
                    surroundingPoints.pop();
                    surroundingPoints.push(Point2f(currentX, currentY));
                }
            }
        }
        else
        {
            cout << "count:" << count << endl;
            cout << "line is discontinuous" << endl;
            cout << "currentX: " << currentX << endl;
            cout << "currentY: " << currentY << endl;
// test 
//            putText(src, "iEnd1", Point(currentX, currentY), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);
            return false;
        }

        count++;
    }

    inAreaPoint.x = currentX;
    inAreaPoint.y = currentY;

    frontPoint.x = surroundingPoints.front().x;
    frontPoint.y = surroundingPoints.front().y;
    cout << "success to get radio point" << endl;
    // cout << "RatioPointx: " << RatioPointx << endl;
    // cout << "RatioPointy: " << RatioPointy << endl;
    cout << "inAreaPointx:" << inAreaPoint.x << endl;
    cout << "inAreaPointy:" << inAreaPoint.y << endl;
	cout << "radioPointx" << output.x << endl;
	cout << "radioPointy" << output.y << endl;
    cout << "frontx:" << frontPoint.x << endl;
    cout << "fronty:" << frontPoint.y << endl;
  //  putText(src, "iEnd2", Point(output.x, output.y), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);
  //  putText(src, "ifront2", Point(frontPoint.x, frontPoint.y), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);

    return true;
}

bool StickDiscern::getLineRatioPoint(Mat& src, Point2f& begin, Point2f& output, Point2f& inAreaPoint, Point2f& frontPoint)
{
    return getLineRatioPoint(src, begin, m_radio, output, inAreaPoint, frontPoint);
}

/*
* @brief 对输入图像进行细化,骨骼化
* @param src为输入图像,用cvThreshold函数处理过的8位灰度图像格式，元素中只有0与1,1代表有元素，0代表为空白
* @param dst为对src细化后的输出图像,格式与src格式相同，元素中只有0与1,1代表有元素，0代表为空白
*/
endPoints StickDiscern::thinImage(Mat & src, Mat & dst)
{
    int width  = src.cols;
    int height = src.rows;
    src.copyTo(dst);
    vector<uchar *> mFlag; //用于标记需要删除的点

    int pointArray[5] = {0};

    // int Endpointx1=0, Endpointy1=0;
    // int Endpointx2=0, Endpointy2=0;
    int deletecount = 0;

    endPoints endps;

    while (true)
    {
        // 步骤一
        for (int i = 0; i < height; ++i)
        {
            uchar * p = dst.ptr<uchar>(i);
            for (int j = 0; j < width; ++j)
            {
                //获得九个点对象，注意边界问题
                uchar p1 = p[j];
                if (p1 != 1) continue;
                uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
                uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
                uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
                uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
                uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
                uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
                uchar p8 = (j == 0) ? 0 : *(p + j - 1);
                uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
                if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)//条件1判断
                {
                    //条件2计算
                    int ap = 0;
                    if (p2 == 0 && p3 == 1) ++ap;
                    if (p3 == 0 && p4 == 1) ++ap;
                    if (p4 == 0 && p5 == 1) ++ap;
                    if (p5 == 0 && p6 == 1) ++ap;
                    if (p6 == 0 && p7 == 1) ++ap;
                    if (p7 == 0 && p8 == 1) ++ap;
                    if (p8 == 0 && p9 == 1) ++ap;
                    if (p9 == 0 && p2 == 1) ++ap;
                    //条件2、3、4判断
                    if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
                    {
                        //标记
                        mFlag.push_back(p + j);
                    }
                }

                //记录端点的位置
                // if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) == 1)
                // {
                    // if(pointArray[4] < 4)
                    // {
                        // pointArray[pointArray[4]] = j;
                        // pointArray[pointArray[4] + 1] = i;
                        // pointArray[4] += 2;
                    // }
                // }
            }
        }
        // Endpointx1 = pointArray[0];
        // Endpointy1 = pointArray[1];
        // Endpointx2 = pointArray[2];
        // Endpointy2 = pointArray[3];
        // pointArray[4] = 0;
        //将标记的点删除
        for (vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
        {
            **i = 0;
            deletecount ++;
        }
        //直到没有点满足，算法结束
        if (mFlag.empty())
        {
            break;
        }
        else
        {
            mFlag.clear();//将mFlag清空
        }

        //步骤二，根据情况该步骤可以和步骤一封装在一起成为一个函数
        for (int i = 0; i < height; ++i)
        {
            uchar * p  = dst.ptr<uchar>(i);
            for (int j = 0; j < width; ++j)
            {
                //  如果满足四个条件，进行标记
                //  p9 p2 p3
                //  p8 p1 p4
                //  p7 p6 p5
                uchar p1 = p[j];
                if (p1 != 1) continue;
                uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
                uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
                uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
                uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
                uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
                uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
                uchar p8 = (j == 0) ? 0 : *(p + j - 1);
                uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
                if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
                {
                    int ap = 0;
                    if (p2 == 0 && p3 == 1) ++ap;
                    if (p3 == 0 && p4 == 1) ++ap;
                    if (p4 == 0 && p5 == 1) ++ap;
                    if (p5 == 0 && p6 == 1) ++ap;
                    if (p6 == 0 && p7 == 1) ++ap;
                    if (p7 == 0 && p8 == 1) ++ap;
                    if (p8 == 0 && p9 == 1) ++ap;
                    if (p9 == 0 && p2 == 1) ++ap;
                    if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
                    {
                        //标记
                        mFlag.push_back(p + j);
                    }
                }

                // 记录端点的位置
                if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) == 1)
                {
                 //   cout << "x:" << j << " y:" << i << endl;
                    if(pointArray[4] < 4)
                    {
                        if(pointArray[4] == 2)
                        {
                            if(pointArray[0] != j && pointArray[1] != i)
                            {
                                pointArray[pointArray[4]] = j;
                                pointArray[pointArray[4] + 1] = i;
                                pointArray[4] += 2;
                            }
                        }
                        else
                        {
                            pointArray[pointArray[4]] = j;
                            pointArray[pointArray[4] + 1] = i;
                            pointArray[4] += 2;
                        }
                    }
                }
            }
        }
        endps.end1.x = pointArray[0];
        endps.end1.y = pointArray[1];
        endps.end2.x = pointArray[2];
        endps.end2.y = pointArray[3];
        //将标记的点删除
        for (vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
        {
            **i = 0;
        }
        //直到没有点满足，算法结束
        if (mFlag.empty()){
            break;
        }
        else
        {
            mFlag.clear();//将mFlag清空
        }
    }

    return endps;
}

bool StickDiscern::init(Mat& src)
{
    bool status = true;
    Mat src_gray;
    // for test pic
    if(src.channels() > 1){
        cvtColor(src, src_gray, CV_BGR2GRAY);
    }else{
        src_gray = src.clone();
    }

    GaussianBlur(src_gray, src_gray, Size(7, 7), 0, 0);  //高斯滤波
    threshold(src_gray, src_gray, m_BinarizationThreshold, 1, cv::THRESH_BINARY_INV);//二值化，前景为1，背景为0

    cv::Mat eroded;
    Mat structureElement1 = getStructuringElement(MORPH_RECT, Size(42, 42), Point(-1, -1));
    Mat structureElement2 = getStructuringElement(MORPH_RECT, Size(24, 24), Point(-1, -1));

    Mat structureElement = getStructuringElement(MORPH_RECT, Size(6, 6), Point(-1, -1));

    dilate(src_gray, eroded, structureElement1, Point(-1, -1), 1);
    // imwrite("./saveimgdir/dilate01.jpg",  eroded*255);
    erode(eroded, eroded, structureElement2, Point(-1, -1), 1);

    erode(eroded, eroded, structureElement, Point(-1, -1), 1);

    m_binarizationImg = eroded;

    Canny(eroded, m_borderImg, 0, 1);
    threshold(m_borderImg, m_borderImg, m_BinarizationThreshold, 1, cv::THRESH_BINARY);

    edp = thinImage(eroded, m_thinImg);  //图像细化（骨骼化）

    bitwise_or(m_thinImg*255, m_borderImg*200, m_frameworkImg);

  // dilate(m_thinImg, m_thinImg, structureElement, Point(-1, -1), 1);
  // erode(m_thinImg, m_thinImg, structureElement, Point(-1, -1), 1);

    cout << "m_BinarizationThreshold:" << (int)m_BinarizationThreshold << endl;
    cout << "m_radio:" << m_radio << endl;

    cout << "6Endpointx1:" << edp.end1.x << endl;
    cout << "Endpointy1:" << edp.end1.y << endl;
    cout << "6Endpointx2:" << edp.end2.x << endl;
    cout << "Endpointy2:" << edp.end2.y << endl;

    Point2f radioPoint1, radioPoint2, inAreaPoint1, inAreaPoint2, frontPoint1, frontPoint2;

    status &= getLineRatioPoint(m_thinImg, edp.end1, radioPoint1, inAreaPoint1, frontPoint1);
    status &= getLineRatioPoint(m_thinImg, edp.end2, radioPoint2, inAreaPoint2, frontPoint2);
    cout << "status=" << status << endl;

    putText(m_thinImg, "end1", edp.end1, FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);
    putText(m_thinImg, "end2", edp.end2, FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);

    putText(m_thinImg, "frontPoint1", frontPoint1, FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);
    putText(m_thinImg, "frontPoint2", frontPoint2, FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);
    putText(m_thinImg, "radioPoint1", radioPoint1, FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);
    putText(m_thinImg, "radioPoint2", radioPoint2, FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,255,255), 1, LINE_AA);

    //test
    // resize(eroded, eroded, Size(eroded.cols/2, eroded.rows/2), 0, 0, cv::INTER_AREA);
    // resize(m_thinImg, m_thinImg, Size(m_thinImg.cols/2, m_thinImg.rows/2), 0, 0, cv::INTER_AREA);
    // resize(m_borderImg, m_borderImg, Size(m_borderImg.cols/2, m_borderImg.rows/2), 0, 0, cv::INTER_AREA);
    imshow("eroded", eroded*255);
    imshow("m_thinImg", m_thinImg*255);
    imshow("m_borderImg", m_borderImg*255);

 //   waitKey(0);

 // imwrite("./saveimgdir/eroded01.jpg",  eroded*255);
 // imwrite("./saveimgdir/m_thinImg01.jpg",  m_thinImg*255);
 // imwrite("./saveimgdir/m_borderImg01.jpg",  m_borderImg*255);

    if(status)
    {
        lineFunc line1, line2, line3, line4;
        getLineSlop(radioPoint1, frontPoint1, line1);
        getVerticalLine(radioPoint1, line1, line2);
        getLineSlop(radioPoint2, frontPoint2, line3);
        getVerticalLine(radioPoint2, line3, line4);

        m_borderImg.copyTo(m_resultImg1);
        m_borderImg.copyTo(m_resultImg2);

        DrawCrossLine(m_resultImg1, line2);
        cout << "line2:k=" << line2.k << " b=" << line2.b << endl;
         // for test
		resize(m_resultImg1, m_resultImg1, Size(m_resultImg1.cols/2, m_resultImg1.rows/2), 0, 0, cv::INTER_AREA);
        imshow("m_resultImg1_01_1", m_resultImg1*255);
//      imwrite("./saveimgdir/m_resultImg1_01_1.jpg",  m_resultImg1*255);

        DrawCrossLine(m_resultImg2, line4);
        cout << "line4:k=" << line4.k << " b=" << line4.b << endl;
        // for test
		resize(m_resultImg2, m_resultImg2, Size(m_resultImg2.cols/2, m_resultImg2.rows/2), 0, 0, cv::INTER_AREA);
        imshow("m_resultImg1_02", m_resultImg2*255);

   	waitKey(0);
//         imwrite("./saveimgdir/m_resultImg1.jpg",  m_resultImg1*255);
//         imwrite("./saveimgdir/m_resultImg2.jpg",  m_resultImg2*255);

        floodFill(m_resultImg1, frontPoint1, Scalar(1));
        floodFill(m_resultImg2, frontPoint2, Scalar(1));

        erode(m_resultImg1, m_resultImg1, structureElement, Point(-1, -1), 1);
        dilate(m_resultImg1, m_resultImg1, structureElement, Point(-1, -1), 1);
        erode(m_resultImg2, m_resultImg2, structureElement, Point(-1, -1), 1);
        dilate(m_resultImg2, m_resultImg2, structureElement, Point(-1, -1), 1);
        bitwise_or(m_frameworkImg, m_resultImg1*255, m_frameworkImg);
        bitwise_or(m_frameworkImg, m_resultImg2*255, m_frameworkImg);
    }
    else
    {
        m_frameworkImg.copyTo(m_resultImg);
    }

    // test
 //   imshow("m_resultImg1_01_2", m_resultImg1*255);
 //   imshow("m_resultImg1_02_2", m_resultImg2*255);
 //   imwrite("./saveimgdir/m_resultImg1_2.jpg",  m_resultImg1*255);
 //   imwrite("./saveimgdir/m_resultImg2_2.jpg",  m_resultImg2*255);
   // resize(m_resultImg1, m_resultImg1, Size(m_resultImg1.cols/2, m_resultImg1.rows/2), 0, 0, cv::INTER_AREA);
   // resize(m_resultImg2, m_resultImg2, Size(m_resultImg2.cols/2, m_resultImg2.rows/2), 0, 0, cv::INTER_AREA);
   // resize(m_frameworkImg, m_frameworkImg, Size(m_frameworkImg.cols/2, m_frameworkImg.rows/2), 0, 0, cv::INTER_AREA);
   // imshow("m_resultImg1", m_resultImg1*255);
   //  imshow("m_resultImg2", m_resultImg2*255);
    // imshow("m_frameworkImg", m_frameworkImg*255);

   // waitKey(0);
    return status;
}

StickDiscernResult StickDiscern::result(Mat& src)
{
    StickDiscernResult ret;

    ret.isValid = init(src);

    ret.totalArea = area(m_binarizationImg)  * RuleValue * RuleValue;
    ret.frameworkArea = area(m_thinImg) * RuleValue;

    cout << "========RuleValue:" << RuleValue << endl;

    if(ret.isValid)
    {
        int temp1 = area(m_resultImg1) * RuleValue * RuleValue;
        int temp2 = area(m_resultImg2) * RuleValue * RuleValue;
        ret.headArea = temp1 > temp2 ? temp1 : temp2;
        ret.tailArea = temp1 > temp2 ? temp2 : temp1;
        if(temp1 > temp2){
            ret.head = edp.end1;
            ret.tail = edp.end2;
        }else{
            ret.head = edp.end2;
            ret.tail = edp.end1;
        }
    }

    return ret;
}

Mat& StickDiscern::result()
{
    return m_resultImg;
}

void StickDiscern::drawResultToImg(Mat& src, Mat& dst, const StickDiscernResult& result)
{
    char str[100];

    src.copyTo(dst);

    cout << "totalArea:" << result.totalArea << endl;
    cout << "frameworkImg:" << result.frameworkArea << endl;
    cout << "headArea:" << result.headArea << endl;
    cout << "tailArea:" << result.tailArea << endl;

     totalArea = result.totalArea ;
     frameworkArea = result.frameworkArea ;
     headArea = result.headArea ;
     tailArea = result.tailArea ;

    if(result.isValid){
        cout << "all is OK" << endl;

        sprintf(str, "总面积:%d 骨架长度:%d(mm)", result.totalArea, result.frameworkArea);
        //then put the text itself
        ft2->putText(dst, str, Point(10, 50), 32, cv::Scalar(255, 0, 255),  thickness, linestyle, true);
        sprintf(str, "头部面积:%d 尾部面积:%d(mm2)", result.headArea, result.tailArea);
        ft2->putText(dst, str, Point(10, 100), 32, cv::Scalar(255, 0, 255),  thickness, linestyle, true);

        sprintf(str, "头部");
        ft2->putText(dst, str, result.head, 32, cv::Scalar(0, 255, 0), thickness, linestyle, true);
        sprintf(str, "尾部");
        ft2->putText(dst, str, result.tail, 32, cv::Scalar(0, 255, 0), thickness, linestyle, true);
    }else{
        cout << "all is NG" << endl;
        headArea = 0;
        tailArea = 0;

/*        sprintf(str, "总面积:%d 骨架长度:%d(mm)", result.totalArea, result.frameworkArea);
        //then put the text itself
        ft2->putText(dst, str, Point(10, 120), 120, cv::Scalar(255, 0, 255),  thickness, linestyle, true);
        sprintf(str, "NG");
        ft2->putText(dst, str, Point(10, 400), 180, cv::Scalar(255, 0, 255),  thickness, linestyle, true);
*/
    }

    // test

    // serila send  cmd
    // char Cmdbegin[7] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x09};
    // serialPuts(fd, Cmdbegin);

}

void StickDiscern::saveimage(Mat& imgsrc){

    char str[100];
    imgcount++;
    sprintf(str, "./saveimgdir/img%d_%d_%d_%d_%d.jpg", totalArea, frameworkArea, headArea, tailArea, imgcount);
    imwrite(str, imgsrc);
}

void StickDiscern::drawResultToImg(Mat& src, const StickDiscernResult& result)
{
    // drawResultToImg(src, m_resultImg, result);
    src = m_frameworkImg;

    if(src.channels() == 1){
        cvtColor(src, src, CV_GRAY2BGR);
    }

    drawResultToImg(src, m_resultImg, result);
}

float StickDiscern::getRuler(Mat& src_gray)
{
    // Mat src_gray;
    // cvtColor(src, src_gray, CV_BGR2GRAY);
    GaussianBlur(src_gray, src_gray, Size(7, 7), 0, 0);  //高斯滤波
    //imshow("src_gray1", src_gray);

    threshold(src_gray, src_gray, m_BinarizationThreshold, 1, cv::THRESH_BINARY_INV); //二值化，前景为1，背景为0

    cv::Mat eroded;
    Mat structureElement = getStructuringElement(MORPH_RECT, Size(11, 11), Point(-1, -1));

    dilate(src_gray, eroded, structureElement, Point(-1, -1), 1);
    erode(eroded, eroded, structureElement, Point(-1, -1), 1);

    // Canny(eroded, m_borderImg, 0, 1);
    // threshold(m_borderImg, m_borderImg, m_BinarizationThreshold, 1, cv::THRESH_BINARY);

    thinImage(eroded, m_thinImg);           //图像细化（骨骼化）
    int frameworkArea = area(m_thinImg);
    // 150mm  de baid  chi
    RuleValue = 150.0f / frameworkArea;

    // for test
    // resize(m_thinImg, m_thinImg, Size(1280, 720), 0, 0, cv::INTER_AREA);
    char str[100];
    sprintf(str, "frameworkArea:%d(mm)", frameworkArea);
    cv::putText(m_thinImg, str, Point(10, 200), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 0, 255),  2, 2);
    imshow("m_thinImg", m_thinImg*255);
    // imshow("eroded", eroded*255);
    // imshow("src_gray2", src_gray*255);

    //save to file
    FILE * ruleFile = fopen("./rulefile.txt", "w");
    float RuleArry[1];
    RuleArry[0] = RuleValue;
    fwrite(RuleArry, sizeof(float), 1, ruleFile);
    printf("=================>>Write Rule = %f, frameworkArea=%d\n", RuleArry[0], frameworkArea);
    fclose(ruleFile);

    //save
    char str2[100];
    sprintf(str2, "./saveimgdir/imgRRR%d.jpg", RuleValue*1000);
    imwrite(str2, m_thinImg*255);

    return RuleValue;
}
