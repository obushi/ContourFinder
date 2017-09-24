#pragma once

#include "Settings.hpp"
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

namespace contourfinder{
    
    class ContoursExtractor{
        
        public:
            void setup(contourfinder::Settings const & settings);
            void update(ofPixels & newFrame);
            void toggleLearnBackground();
            void setThreashold(int threashold);
            void setSimplicity(int polylinesSimplicity);
            void setWarpPerspectiveTransform(std::vector<ofVec2f> const & ofCaptureAreaCorners, std::vector<ofVec2f>  const & ofWindowCorners);
            void setMaxblobsCount(int count);
            std::vector<ofPolyline> const & getPolylines();
            int  contoursCount();
            void drawContours();
            void draw();
        
        private:
            ofxCvGrayscaleImage     srcGrayImg;
            ofxCvGrayscaleImage     bgGrayImg;
            ofxCvGrayscaleImage     diffGrayImg;
            ofxCv::ContourFinder    contourFinder;
            ofShader                shader;
            ofFbo                   fbo;
            ofMatrix3x3             warpMatrix;
            ofPlanePrimitive        plane;
            ofPixels                pixels;
            std::vector<ofPolyline> polylines;
            int                     threashold;
            int                     polylinesSimplicity;
            int                     maxBlobsCount;
            bool                    learnBackground;
    };
}; // namespace contourfinder
