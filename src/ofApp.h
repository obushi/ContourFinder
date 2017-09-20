#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxBlackMagic.h"
#include <random>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void windowResized(int w, int h);
        void exit(ofEventArgs &args);
    
    private:
    
        // Constants
        static const int maxBlobsCount     = 10;
        static const int srcVideoWidth     = 1280;
        static const int srcVideoHeight    = 720;
        constexpr static float srcVideoFPS = 59.94f;
        static const int dstVideoWidth     = 1024;
        static const int dstVideoHeight    = 768;
        static const int cornerPinRadius   = 12;
        const ofVec2f unityScreenSize = ofVec2f(20, 15);
    
        enum Status {
            Setup, Play
        };
        
        Status currentStatus;
        
        // TransformMatrix
        cv::Mat homographyMat;
        cv::Mat oF2UnityMat;
        std::vector<cv::Point2f> oFSrcPoints;
        std::vector<cv::Point2f> oFDstPoints;
        std::vector<cv::Point2f> unityPoints;
        ofRectangle oFSrcRect;
        ofxXmlSettings settings;
        int selectedCorner;
        
        // ImageProcessor
        bool                isMaskedBlack;
        bool                isMaskedWhite;
        bool                willLearnBg;
        int                 thresh;
        ofxBlackMagic       blackMagic;
        ofxCvGrayscaleImage srcGrayImg, bgGrayImg, diffGrayImg;
        ofxCvContourFinder  contourFinder;
        std::array<ofPolyline, maxBlobsCount> polylines;
    
        // UDPSender
        ofxUDPManager udpConnection;
        void sendVertices(std::array<ofPolyline, maxBlobsCount> vertices);
        bool enableUDP;
};
