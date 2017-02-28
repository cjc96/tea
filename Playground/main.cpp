//
//  main.cpp
//  Playground
//
//  Created by 曹景辰 on 16/8/9.
//  Copyright © 2016年 曹景辰. All rights reserved.
//

#include "common.h"
#include "kinect.hpp"

extern cv::Mat terminal;

int main()
{
    terminal = cv::Mat(800, 1300, CV_8UC3, cv::Scalar(255, 255, 255));
    std::cout << terminal.rows << " " << terminal.cols << std::endl;
    main_loop();
    
    return 0;
}