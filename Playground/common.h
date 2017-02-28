//
//  header.h
//  Playground
//
//  Created by 曹景辰 on 8/10/16.
//  Copyright © 2016 曹景辰. All rights reserved.
//

#ifndef header_h
#define header_h

#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <stack>
#include <list> 

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv/cvaux.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>

#define AREA_NUM 100

#define BACKGROUND 0
#define CUP 1
#define HAND 1

typedef std::vector<std::pair<cv::Point, int> > packStruct;

typedef struct
{
    cv::Point loc;
    int type;
    int frame;
} movInfo;

typedef std::vector<movInfo> videoInfo;

#endif /* header_h */
