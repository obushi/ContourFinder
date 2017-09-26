#pragma once

#include "Settings.hpp"
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxBlackMagic.h"
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/usb/BaslerUsbConfigurationEventHandler.h>
#include <pylon/usb/BaslerUsbImageEventHandler.h>
#include <pylon/usb/BaslerUsbCameraEventHandler.h>

namespace contourfinder{
    
    class BaslerCamera{
        
        public:
            Pylon::CBaslerUsbInstantCamera * instantCamera;
            static cv::Mat result;
            static ofxCvGrayscaleImage image;
    };

    class VideoSource{
        
        public:
            void setup(contourfinder::Settings const & settings);
            bool update();
            ofxCvGrayscaleImage & getGrayscalePixels();
            void close();
            cv::Mat result;
        
        private:
            ofxBlackMagic blackmagic;
            BaslerCamera camera;
        
    };
    
    class BaslerConfigEventHandler : public Pylon::CBaslerUsbConfigurationEventHandler{
        
        public:
            void OnOpened(Pylon::CBaslerUsbInstantCamera & camera);
    };
    
    class BaslerImageEventHandler : public Pylon::CBaslerUsbImageEventHandler{
        
        public:
            void OnImageGrabbed(Pylon::CBaslerUsbInstantCamera & camera, const Pylon::CBaslerUsbGrabResultPtr & grabResult);
    };
    
//    class BaslerCameraEventHandler : public Pylon::CBaslerUsbCameraEventHandler{
//
//    };
    
}; // namespace contourfinder
