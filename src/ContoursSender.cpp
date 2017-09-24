#include "ContoursSender.hpp"

void contourfinder::ContoursSender::setup(contourfinder::Settings const & settings){
    udpConnection.Create();
    udpConnection.Connect(settings.udpIp, settings.udpPort);
    udpConnection.SetNonBlocking(true);
    toUnity = [&](float x, float y){return ofVec2f(ofMap(x, 0.0, settings.projectorWidth, -settings.unityWorldWidth/2, settings.unityWorldWidth/2), ofMap(y, 0.0, settings.projectorHeight, settings.unityWorldHeight/2, -settings.unityWorldHeight/2));};
}

void contourfinder::ContoursSender::sendVertices(std::vector<ofPolyline> const & polylines, const int contoursCount){
    std::string msg;
    for(int i = 0; i < contoursCount; ++i){
        msg = ofToString(i);
        for (auto const & vert : polylines[i].getVertices()){
            ofVec2f pos = toUnity(vert.x, vert.y);
            msg += "|" + ofToString(pos.x, 3) + "," + ofToString(pos.y, 3);
        }
        udpConnection.Send(msg.c_str(), msg.length());
    }
}
