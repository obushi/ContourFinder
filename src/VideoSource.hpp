#pragma once

#include "Settings.hpp"
#include "ofMain.h"
#include "ofxBlackMagic.h"
#include "ofxOpenCv.h"

namespace contourfinder{
    
    class VideoSource{
        
        public:
            void setup(contourfinder::Settings const & settings);
            bool update();
            ofPixels & getGrayscalePixels();
            void close();
        
        private:
            ofxBlackMagic blackmagic;
    };
}; // namespace contourfinder
