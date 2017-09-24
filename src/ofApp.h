#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Settings.hpp"
#include "VideoSource.hpp"
#include "ContoursExtractor.hpp"
#include "ContoursSender.hpp"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
        void exit(ofEventArgs &args);
    
    private:
        void initializeCorners();
    
        enum Status{
            Setup, Play
        };
        Status currentStatus;
    
        ofxPanel gui;
        ofParameter<std::string> fps;
        ofParameter<bool> udpEnabled;
        ofParameter<int>  threashold;
        ofParameter<int>  maxBlobsCount;
        ofParameter<int>  polylinesSimplicity;
        ofParameter<int>  cornerPinRadius;
        void threasholdChanged(int & threashold);
        void maxBlobsCountChanged(int & maxBlobsCount);
        void polylinesSimplicityChanged(int & polylinesSimplicity);
    
        contourfinder::Settings          settings;
        contourfinder::VideoSource       videoSource;
        contourfinder::ContoursExtractor contoursExtractor;
        contourfinder::ContoursSender    contoursSender;

        std::vector<ofVec2f> ofCaptureAreaCorners;
        std::vector<ofVec2f> ofVideoCorners;
        std::vector<ofVec2f> ofWindowCorners;
        std::vector<ofVec2f> unityWorldCorners;
        int  selectedCorner;
        bool isMaskedWhite;
        bool drawGui;
};
