%YAML:1.0
Parameters:
   DetectMinSize: 0       #开始的跟踪最小的尺寸
   DetectMaxSize: 600     #开始的跟踪最小的尺寸
   threshValue: 10        #越小 移动速度越快, 值越大 跳动越不激烈
   allThreshValue: 10     #最后的滤波阀值,   有很明显的滤波作用
   trackernum: 1          #持最大跟踪的目标数量
   speedMin: 1            #跟踪框最小的速度
   speedMax: 20           #跟踪框最大的速度
   Meanedthresh: 2.9      #预测相机运动的阈值,倍数
   detectType: 3          #物体检测类型; 0:跟踪运动的框, 1:跟踪人脸和人头, 2:跟踪人脸和鸡, 3:跟踪人脸和手掌, 4:跟踪人脸和羊头
