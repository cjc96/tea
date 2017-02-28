//
//  detection.cpp
//  Playground
//
//  Created by 曹景辰 on 8/11/16.
//  Copyright © 2016 曹景辰. All rights reserved.
//

#include "common.h"
#include "video.hpp"

cv::BackgroundSubtractorMOG bg_model, bg_model_depth;
cv::Mat fgmask, fgimg, labelImg, fgmaskDepth, fgimgDepth;
packStruct pack;
int waveFrame = 0;

/* 对标定label的区域换色显示中心 */
void icvprLabelColor(const cv::Mat& _labelImg, cv::Mat& _colorLabelImg)
{
    
    if (_labelImg.empty() ||
        _labelImg.type() != CV_32SC1)
    {
        return ;
    }
    
    std::map<int, cv::Scalar> colors;
    std::map<int, int> labelMap;
    std::vector<int> labelVec;
    int maxx[AREA_NUM], maxy[AREA_NUM], minx[AREA_NUM], miny[AREA_NUM];
    
    memset(maxx, 0, sizeof(maxx));
    memset(maxy, 0, sizeof(maxy));
    memset(minx, 0, sizeof(minx));
    memset(miny, 0, sizeof(miny));
    
    int rows = _labelImg.rows ;
    int cols = _labelImg.cols;
    int cupNum = 0;
    
    _colorLabelImg.release();
    _colorLabelImg.create(rows, cols, CV_8UC3) ;
    _colorLabelImg = cv::Scalar::all(0);
    labelMap.clear();
    labelVec.clear();
    
    for (int i = 0; i < rows; i++)
    {
        const int* data_src = (int*)_labelImg.ptr<int>(i) ;
        uchar* data_dst = _colorLabelImg.ptr<uchar>(i) ;
        for (int j = 0; j < cols; j++)
        {
            int pixelValue = data_src[j] ;
            if (pixelValue > 1)
            {
                if (colors.count(pixelValue) <= 0)
                {
                    labelVec.push_back(pixelValue);
                    cupNum++;
                    labelMap[pixelValue] = cupNum;
                }
                cv::Scalar color = colors[pixelValue];
                // Find max and min coordinate of each labeled area
                if (i < minx[labelMap[pixelValue]] || minx[labelMap[pixelValue]] == 0)
                    minx[labelMap[pixelValue]] = i;
                if (i > maxx[labelMap[pixelValue]] || maxx[labelMap[pixelValue]] == 0)
                    maxx[labelMap[pixelValue]] = i;
                if (j < miny[labelMap[pixelValue]] || miny[labelMap[pixelValue]] == 0)
                    miny[labelMap[pixelValue]] = j;
                if (j > maxy[labelMap[pixelValue]] || maxy[labelMap[pixelValue]] == 0)
                    maxy[labelMap[pixelValue]] = j;
            }
            else
            {
                data_dst++ ;
                data_dst++ ;
                data_dst++ ;
            }
        }
    }
    pack.clear();
    for (std::vector<int>::iterator i = labelVec.begin(); i != labelVec.end(); i++)
    {
        int _minx = minx[labelMap[*i]], _maxx = maxx[labelMap[*i]], _miny = miny[labelMap[*i]], _maxy = maxy[labelMap[*i]];
        int tmpy = (_minx + _maxx) / 2, tmpx = (_miny + _maxy) / 2;
        cv::Point tmp(tmpx, tmpy);

        if (std::fabs((_maxx - _minx) - (_maxy - _miny)) > 0.15 * (float)((_maxx - _minx) + (_maxy - _miny)))
        {
            pack.push_back(std::make_pair(tmp, CUP));
        }
        else
        {
            pack.push_back(std::make_pair(tmp, HAND));
        }
    }
    for (packStruct::iterator i = pack.begin(); i != pack.end(); i++)
    {
        cv::circle(_colorLabelImg, i->first, 20, (i->second == CUP ? cv::Scalar(255, 0, 0): cv::Scalar(0, 255, 0)),-1);
    }
}

/* 联通域检测 */
void icvprCcaByTwoPass(const cv::Mat& _binImg, cv::Mat& _lableImg)
{
    // connected component analysis (4-component)
    // use two-pass algorithm
    // 1. first pass: label each foreground pixel with a label
    // 2. second pass: visit each labeled pixel and merge neighbor labels
    //
    // foreground pixel: _binImg(x,y) = 1
    // background pixel: _binImg(x,y) = 0
    
    
    if (_binImg.empty() ||
        _binImg.type() != CV_8UC1)
    {
        return ;
    }
    
    // 1. first pass
    
    _lableImg.release() ;
    _binImg.convertTo(_lableImg, CV_32SC1) ;
    
    int label = 1 ;  // start by 2
    std::vector<int> labelSet ;
    labelSet.push_back(0) ;   // background: 0
    labelSet.push_back(1) ;   // foreground: 1
    
    int rows = _binImg.rows - 1 ;
    int cols = _binImg.cols - 1 ;
    for (int i = 1; i < rows; i++)
    {
        int* data_preRow = _lableImg.ptr<int>(i-1) ;
        int* data_curRow = _lableImg.ptr<int>(i) ;
        for (int j = 1; j < cols; j++)
        {
            if (data_curRow[j] == 1)
            {
                std::vector<int> neighborLabels ;
                neighborLabels.reserve(2) ;
                int leftPixel = data_curRow[j-1] ;
                int upPixel = data_preRow[j] ;
                if ( leftPixel > 1)
                {
                    neighborLabels.push_back(leftPixel) ;
                }
                if (upPixel > 1)
                {
                    neighborLabels.push_back(upPixel) ;
                }
                
                if (neighborLabels.empty())
                {
                    labelSet.push_back(++label) ;  // assign to a new label
                    data_curRow[j] = label ;
                    labelSet[label] = label ;
                }
                else
                {
                    std::sort(neighborLabels.begin(), neighborLabels.end()) ;
                    int smallestLabel = neighborLabels[0] ;
                    data_curRow[j] = smallestLabel ;
                    
                    // save equivalence
                    for (size_t k = 1; k < neighborLabels.size(); k++)
                    {
                        int tempLabel = neighborLabels[k] ;
                        int& oldSmallestLabel = labelSet[tempLabel] ;
                        if (oldSmallestLabel > smallestLabel)
                        {
                            labelSet[oldSmallestLabel] = smallestLabel ;
                            oldSmallestLabel = smallestLabel ;
                        }
                        else if (oldSmallestLabel < smallestLabel)
                        {
                            labelSet[smallestLabel] = oldSmallestLabel ;
                        }
                    }
                }
            }
        }
    }
    
    // update equivalent labels
    // assigned with the smallest label in each equivalent label set
    for (size_t i = 2; i < labelSet.size(); i++)
    {
        int curLabel = labelSet[i] ;
        int preLabel = labelSet[curLabel] ;
        while (preLabel != curLabel)
        {
            curLabel = preLabel ;
            preLabel = labelSet[preLabel] ;
        }
        labelSet[i] = curLabel ;
    }
    
    
    // 2. second pass
    for (int i = 0; i < rows; i++)
    {
        int* data = _lableImg.ptr<int>(i) ;
        for (int j = 0; j < cols; j++)
        {
            int& pixelLabel = data[j] ;
            pixelLabel = labelSet[pixelLabel] ;
        }  
    }  
}

/* 前景检测 */
void refineSegments(const cv::Mat& img, cv::Mat& mask, cv::Mat& dst)
{
    int niters = 3;
    
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::Mat temp;
    
    cv::dilate(mask, temp, cv::Mat(), cv::Point(-1,-1), niters);//膨胀，3*3的element，迭代次数为niters
    cv::erode(temp, temp, cv::Mat(), cv::Point(-1,-1), niters * 2);//腐蚀
    cv::dilate(temp, temp, cv::Mat(), cv::Point(-1,-1), niters);
    
    cv::findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );//找轮廓
    
    dst = cv::Mat::zeros(img.size(), CV_8UC3);
    
    if( contours.size() == 0 )
        return;
    
    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int idx = 0, comp = 0;
    double minArea = 2000;
    //轮廓模式为CV_RETR_CCOMP，第一层为连通域的外围边界，因为我们前景颜色值为255，背景为0
    //所以得到的轮廓为外围边界，所以我们只需要第一层，hierarchy[idx][0]表示为下一个轮廓的索引
    //如果到达最后一个轮廓，则hierarchy[idx][0]=-1；
    cv::Scalar color( 255, 255, 255);
    for( ; idx >= 0; idx = hierarchy[idx][0] )
    {
        const std::vector<cv::Point>& c = contours[idx];
        double area = fabs(cv::contourArea(cv::Mat(c)));
        if( area > minArea )
        {
            comp = idx;
            cv::drawContours( dst, contours, comp, color, CV_FILLED, 8, hierarchy);
        }
    }
}

void refineDepthSegments(const cv::Mat& img, cv::Mat& mask, cv::Mat& dst)
{
    int niters = 3;
    
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::Mat temp;
    
    cv::dilate(mask, temp, cv::Mat(), cv::Point(-1,-1), niters);//膨胀，3*3的element，迭代次数为niters
    cv::erode(temp, temp, cv::Mat(), cv::Point(-1,-1), niters * 2);//腐蚀
    cv::dilate(temp, temp, cv::Mat(), cv::Point(-1,-1), niters);
    
    cv::findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );//找轮廓
    
    dst = cv::Mat::zeros(img.size(), CV_8UC3);
    
    if( contours.size() == 0 )
        return;
    
    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int idx = 0, comp = 0;
    double minArea = 100;
    cv::Scalar color( 255, 255, 255);
    bool active = true;
    for( ; idx >= 0; idx = hierarchy[idx][0] )
    {
        const std::vector<cv::Point>& c = contours[idx];
        double area = fabs(cv::contourArea(cv::Mat(c)));
        if( area > minArea )
        {
            active = false;
            comp = idx;
            cv::drawContours(dst, contours, comp, color, CV_FILLED, 8, hierarchy);
        }
    }
    if (active)
        waveFrame = 0;
    else
        waveFrame++;
}

cv::Mat detectPot(cv::Mat inputMat, cv::Mat assisMat)
{
    bg_model(inputMat, fgmask, 0);
    refineSegments(inputMat, fgmask, fgimg);
    cv::cvtColor(fgimg, fgimg, CV_BGR2GRAY);
    
    cv::Mat outputMat;
//    cv::resize(fgimg, outputMat, cv::Size(), 0.5, 0.5);
//    cv::imshow("Foreground Image", outputMat);
    
    cv::threshold(fgimg, fgimg, 50, 1, CV_THRESH_BINARY_INV);
    fgimg = 1 - fgimg;
    icvprCcaByTwoPass(fgimg, labelImg);
    cv::Mat colorLabelImg;
    icvprLabelColor(labelImg, colorLabelImg);
    
    bg_model_depth(assisMat, fgmaskDepth, 0);
    refineDepthSegments(assisMat, fgmaskDepth, fgimgDepth);
//    cv::imshow("Depth Image", fgimgDepth);
    bool waveJudge = false;
    if (waveFrame > 5)
    {
        waveJudge = true;
        waveFrame = 0;
    }
    playVideo(pack, waveJudge);
    
//    int x =100;
//    cv::line(colorLabelImg, cv::Point(x, 0), cv::Point(x, 700), cv::Scalar(0, 0, 255));
    
    return colorLabelImg;
}