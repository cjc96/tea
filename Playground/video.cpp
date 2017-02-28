//
//  video.cpp
//  Playground
//
//  Created by 曹景辰 on 8/16/16.
//  Copyright © 2016 曹景辰. All rights reserved.
//

#include "common.h"

cv::Mat terminal;
cv::Mat wallpaper;
cv::VideoCapture water("/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/water.mp4");
cv::VideoCapture mountain("/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/fps15.mp4");
cv::VideoCapture *videoArray[] = {&mountain, &water};
videoInfo now;
movInfo bkgrd;
bool bkgrdOn = false;
int waveNum;

int dist(cv::Point a, cv::Point b)
{
    float temp = sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    
    return temp;
}

void process(cv::Mat pic, int x, int y, bool cover)
{
    for (int i = 0; i < pic.rows; i++)
    {
        for (int j = 0; j < pic.cols; j++)
        {
            if (j + x < 1300 && i + y < 800)
            {
                if (!cover)
                {
                    if (pic.at<cv::Vec3b>(i, j)[0] != 255 || pic.at<cv::Vec3b>(i, j)[2] != 255)
                    {
                        terminal.at<cv::Vec3b>(800 - (i + y), j + x) = pic.at<cv::Vec3b>(i, j);
                    }
                }
                else
                {
                    terminal.at<cv::Vec3b>(i, j) = pic.at<cv::Vec3b>(i, j);
                }
            }
        }
    }
}

void playVideo(packStruct pack, bool wave)
{
    if (wave)
    {
        std::string pwd;
        waveNum++;
        if (waveNum <= 4)
        {
            if (waveNum == 1)
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper1.png";
            else if (waveNum == 2)
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper2.png";
            else if (waveNum == 3)
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper3.png";
            else
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper4.png";
            wallpaper = cv::imread(pwd);
            process(wallpaper, 0, 0, 1);
        }
        else
        {
            if (waveNum == 5 && (!bkgrdOn))
            {
                bkgrd.loc = cv::Point(0, 0);
                bkgrd.type = BACKGROUND;
                bkgrd.frame = 0;
                bkgrdOn = true;
            }
        }
    }
    else
    {
        if (waveNum <= 4 && waveNum > 0)
        {
            std::string pwd;
            if (waveNum == 1)
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper1.png";
            else if (waveNum == 2)
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper2.png";
            else if (waveNum == 3)
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper3.png";
            else
                pwd = "/Users/caojingchen/Fudan/My projects/cpp/tea/Playground/wallpaper4.png";
            wallpaper = cv::imread(pwd);
            process(wallpaper, 0, 0, 1);
        }
    }
    if (bkgrdOn)
    {
        terminal = cv::Mat(800, 1300, CV_8UC3, cv::Scalar(255, 255, 255));
        cv::Mat temp;
        mountain.read(temp);
        
        process(temp, 0, 0, 1);
    }
    for (packStruct::iterator i = pack.begin(); i != pack.end(); i++)
    {
        bool cover = false;
        for (videoInfo::iterator j = now.begin(); j != now.end(); j++)
        {
            if (dist(i->first, j->loc) < 100)
            {
                cover = true;
                break;
            }
        }
        if (!cover)
        {
            movInfo temp;
            temp.loc = i->first;
            temp.type = i->second;
            temp.frame = 0;
            
            now.push_back(temp);
        }
    }
    for (videoInfo::iterator j = now.begin(); j != now.end(); j++)
    {
        cv::Mat temp;
        videoArray[j->type]->set(CV_CAP_PROP_POS_FRAMES,j->frame++);
        if (j->frame < 60)
        {
            videoArray[j->type]->read(temp);
            float offset_x =(j->loc).x * 1300 / 700, offset_y = (j->loc).y * 800 / 350;
            process(temp, (int)offset_x, (int)offset_y, 0);
        }
        else
        {
            now.erase(j);
            if (!now.size())
                break;
        }
    }
    
    cv::imshow("RGB Image", terminal);
    cv::waitKey(10);
}