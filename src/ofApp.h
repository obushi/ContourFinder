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
        enum Status {
            Setup, Play
        };
    
        Status currentStatus;
    
        // Constants
        static const int maxBlobsCount     = 10;
        static const int cameraWidth     = 1280;
        static const int cameraHeight    = 720;
        constexpr static float cameraFramerate = 59.94f;
        static const int projectorWidth     = 1024;
        static const int projectorHeight    = 768;
        static const int cornerPinRadius   = 12;
        const ofVec2f unityWorldSize = ofVec2f(20, 15);
        
        // TransformMatrix
        std::vector<ofVec2f> ofPinCoords;
        std::vector<ofVec2f> ofVideoCorners;
        std::vector<ofVec2f> ofWindowCorners;
        std::vector<ofVec2f> unityWorldCorners;
        ofxXmlSettings settings;
        int selectedCorner;
        
        // ImageProcessor
        bool                isMaskedBlack;
        bool                isMaskedWhite;
        bool                willLearnBg;
        int                 thresh;
        ofShader            shader;
        ofFbo               fbo;
        ofPixels            pixels;
        ofMatrix3x3         warpMatrix;
        ofMatrix3x3         resizeMatrix;
        ofPlanePrimitive    plane;
        ofxBlackMagic       blackMagic;
        ofxCvGrayscaleImage srcGrayImg, bgGrayImg, diffGrayImg;
        ofxCv::ContourFinder  contourFinder;
        std::array<ofPolyline, maxBlobsCount> polylines;
    
        // UDPSender
        ofxUDPManager udpConnection;
        void sendVertices(std::array<ofPolyline, maxBlobsCount> vertices);
        bool enableUDP;
};
