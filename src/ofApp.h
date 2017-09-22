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
    
        struct Settings {
            int cameraWidth;
            int cameraHeight;
            float cameraFramerate;
            int projectorWidth;
            int projectorHeight;
            float unityWorldWidth;
            float unityWorldHeight;
        };
    
        Status currentStatus;
        Settings settings;
    
        // GUI
        ofxPanel gui;
        ofParameter<std::string> fps;
        ofParameter<int> threashold;
        ofParameter<int> maxBlobsCount;
        ofParameter<int> polylinesSimplicity;
        ofParameter<int> cornerPinRadius;
        ofParameter<int> captureAreaBorderWidth;
        void maxBlobsCountChanged(int &maxBlobsCount);
    
        // TransformMatrix
        std::vector<ofVec2f> ofPinCoords;
        std::vector<ofVec2f> ofVideoCorners;
        std::vector<ofVec2f> ofWindowCorners;
        std::vector<ofVec2f> unityWorldCorners;
        ofxXmlSettings xmlSettings;
        int selectedCorner;
        
        // ImageProcessor
        bool                isMaskedWhite;
        bool                willLearnBg;
        ofShader            shader;
        ofFbo               fbo;
        ofPixels            pixels;
        ofMatrix3x3         warpMatrix;
        ofPlanePrimitive    plane;
        ofxBlackMagic       blackMagic;
        ofxCvGrayscaleImage srcGrayImg, bgGrayImg, diffGrayImg;
        ofxCv::ContourFinder  contourFinder;
        std::vector<ofPolyline> polylines;
    
        // UDPSender
        ofxUDPManager udpConnection;
        void sendVertices(std::vector<ofPolyline> vertices);
        bool enableUDP;
};
