#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/core/types.hpp>

#include <ctime>
#include <string>
#include <sstream>
#include <fstream>

#include <stdio.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <linux/watchdog.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <net/if.h>

#include "StickDiscern.h"

#define TO_STRING(n) ( ((ostringstream&)(ostringstream() << n)).str() )
#define debug(...)     printf(__VA_ARGS__)

using namespace cv;
using namespace std;

//bounding box mouse callback
StickDiscern m_eelRecognition(60, 0.25);

int main(void) {

    Mat test = imread("./5.jpg");

    imshow("test", test);


    cout  << "test channels3=" << test.channels() << endl;
   
    // set init
    m_eelRecognition.setBinarizationThreshold(28);

    m_eelRecognition.drawResultToImg(test, m_eelRecognition.result(test));

    waitKey(0);

    return 0;
}
