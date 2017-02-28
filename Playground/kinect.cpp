//
//  kinect.cpp
//  Playground
//
//  Created by 曹景辰 on 8/11/16.
//  Copyright © 2016 曹景辰. All rights reserved.
//

#include "common.h"
#include "detection.hpp"

void deviceEnd(libfreenect2::Freenect2Device *dev)
{
    dev->stop();
    dev->close();
}

int main_loop()
{
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;
    
    std::string serial;
    const bool enable_rgb = true, enable_depth = true; // some constant switch
    
    // initialize kinect
    if(freenect2.enumerateDevices() == 0)
    {
        std::cout << "no device connected!" << std::endl;
        return -1;
    }
    if (serial == "")
    {
        serial = freenect2.getDefaultDeviceSerialNumber();
    }
    pipeline = new libfreenect2::OpenGLPacketPipeline();
    
    // configure kinect
    dev = freenect2.openDevice(serial, pipeline);
    int types = 0;
    if (enable_rgb)
        types |= libfreenect2::Frame::Color;
    if (enable_depth)
        types |= libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
    libfreenect2::SyncMultiFrameListener listener(types);
    libfreenect2::FrameMap frames;
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);
    
    // start kinect
    if (enable_rgb && enable_depth)
    {
        if (!dev->start())
            return -1;
    }
    else
    {
        if (!dev->startStreams(enable_rgb, enable_depth))
            return -1;
    }
    std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
    std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
    libfreenect2::Registration* registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());
    libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);
    
    // Receive images frames
    bool protonect_shutdown = false;
    int framemax = -1, framecount = 0;
    cv::namedWindow("Depth Image", CV_WINDOW_AUTOSIZE);
    cv::namedWindow("Color Image", CV_WINDOW_AUTOSIZE);
    cv::namedWindow("RGB Image", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("Color IMage", 0, 150);
    cv::moveWindow("RGB IMage", 0, 1000);
    while(!protonect_shutdown && (framemax == (size_t)-1 || framecount < framemax))
    {
        if (!listener.waitForNewFrame(frames, 1 * 1000)) // 1 seconds
        {
            std::cout << "timeout!" << std::endl;
            return -1;
        }
        // Take each frame from frameMap
        libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
        libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
        
        // Feed Libfreenect2 frame to Opencv Mat
        cv::Mat rgbMat, depthMat, outputMat;
        cv::Mat(rgb->height, rgb->width, CV_8UC4, rgb->data).copyTo(rgbMat);
        cv::Mat(depth->height, depth->width, CV_32FC1, depth->data).copyTo(depthMat);
        cv::cvtColor(rgbMat, rgbMat, CV_BGRA2BGR);
        
        cv::normalize(depthMat,depthMat,1.0,0.0,cv::NORM_MINMAX);
        depthMat.convertTo(depthMat, CV_8UC1, 255, 0);
        
        // Set ROI
        int zoom = rgbMat.rows/depthMat.rows;
        cv::Rect ROI(0, 70, 400, 200);
        
        cv::Rect ROI_rgb(60 * zoom, 30 * zoom, 350 * zoom * 2, 350 * zoom);
        
        rgbMat = rgbMat(ROI_rgb);
        depthMat = depthMat(ROI);
//        cv::rectangle(depthMat, ROI, cv::Scalar(255, 255, 0), 10);
//        cv::imshow("Depth Image", depthMat);
        
        // Detect object(Opencv GMM algorithm, background substraction)
        cv::Mat resultMat;
        cv::medianBlur(rgbMat, rgbMat, 5);
        cv::medianBlur(depthMat, depthMat, 5);
        cv::resize(rgbMat, rgbMat, cv::Size(), 0.5, 0.5);
        resultMat = detectPot(rgbMat, depthMat);
        cv::resize(resultMat, outputMat, cv::Size(), 0.5, 0.5);
        cv::imshow("Color Image", outputMat);
        
        // Quit if you press any key
        int key = cv::waitKey(20);
        if (key == 32)
        {
            cv::waitKey();
        }
        else if (key >= 0)
        {
            deviceEnd(dev);
            return 0;
        }
        
        //! [registration]
        registration->apply(rgb, depth, &undistorted, &registered);
        //! [registration]
        
        // prepare for next loop
        listener.release(frames);
        framecount++;
    }
    
    deviceEnd(dev);
    return 0;
}
