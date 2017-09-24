#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "Settings.hpp"

namespace contourfinder{
    
    class ContoursSender{
        
        public:
            void setup(contourfinder::Settings const & settings);
            void sendVertices(std::vector<ofPolyline> const & polylines, const int contoursCount);
        
        private:
            ofxUDPManager udpConnection;
            std::function<ofVec2f(float, float)> toUnity;
    };
}; // namespace contourfinder
