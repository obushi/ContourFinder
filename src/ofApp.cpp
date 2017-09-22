#include "ofApp.h"
using namespace ofxCv;
using namespace cv;

void ofApp::setup(){
    
    ofSetFrameRate(120);
    ofSetVerticalSync(true);
    
    gui.setup();
    gui.add(fps.set("fps", ""));
    gui.add(threashold.set("threashold", 80, 0, 150));
    gui.add(maxBlobsCount.set("maxBlobsCount", 10, 0, 50));
    gui.add(polylinesSimplicity.set("polylinesSimplicity", 10, 0, 20));
    gui.add(cornerPinRadius.set("cornerPinRadius", 12, 1, 50));
    gui.add(captureAreaBorderWidth.set("captureAreaBorderWidth", 3, 1, 50));
    
    maxBlobsCount.addListener(this, &ofApp::maxBlobsCountChanged);
    polylines.resize(maxBlobsCount);
    
    settings.cameraWidth      = 1280;
    settings.cameraHeight     = 720;
    settings.cameraFramerate  = 59.94f;
    settings.projectorWidth   = 1024;
    settings.projectorHeight  = 768;
    settings.unityWorldWidth  = 20.0f;
    settings.unityWorldHeight = 15.0f;
    
    currentStatus = Status::Setup;
    
    blackMagic.setup(cameraWidth, cameraHeight, cameraFramerate);
    
    willLearnBg = true;
    isMaskedWhite = false;
    bgGrayImg.allocate(cameraWidth, cameraHeight);
    srcGrayImg.allocate(cameraWidth, cameraHeight);
    diffGrayImg.allocate(cameraWidth, cameraHeight);
    
    selectedCorner = -1;
    
    ofPinCoords = {
        ofVec2f(cornerPinRadius, cornerPinRadius),
        ofVec2f(projectorWidth - cornerPinRadius, cornerPinRadius),
        ofVec2f(projectorWidth - cornerPinRadius, projectorHeight - cornerPinRadius),
        ofVec2f(cornerPinRadius, projectorHeight - cornerPinRadius)
    };
    
    ofVideoCorners = {
        ofVec2f(0, 0),
        ofVec2f(cameraWidth, 0),
        ofVec2f(cameraWidth, cameraHeight),
        ofVec2f(0, cameraHeight)
    };
    
    ofWindowCorners = {
        ofVec2f(0, 0),
        ofVec2f(projectorWidth, 0),
        ofVec2f(projectorWidth, projectorHeight),
        ofVec2f(0, projectorHeight)
    };
    
    unityWorldCorners = {
        ofVec2f(-unityWorldSize.x/2, unityWorldSize.y/2),
        ofVec2f(unityWorldSize.x/2, unityWorldSize.y/2),
        ofVec2f(unityWorldSize.x/2, -unityWorldSize.y/2),
        ofVec2f(-unityWorldSize.x/2, -unityWorldSize.y/2)
    };
    
    contourFinder.setMaxArea(cameraWidth * cameraHeight * 0.5);
    contourFinder.setMinArea(cameraWidth * cameraHeight * 0.01);
    contourFinder.setFindHoles(false);
    contourFinder.setThreshold(thresh);
    
    shader.load("shaders/warpPerspective");
    fbo.allocate(projectorWidth, projectorHeight);
    
    cv::Mat m = cv::getPerspectiveTransform(ofxCv::toCv(ofPinCoords), ofxCv::toCv(ofWindowCorners));
    m = m.inv();
    warpMatrix.set(m.at<double>(0,0), m.at<double>(1,0), m.at<double>(2,0),
                   m.at<double>(0,1), m.at<double>(1,1), m.at<double>(2,1),
                   m.at<double>(0,2), m.at<double>(1,2), m.at<double>(2,2));
    
    plane.set(projectorWidth, projectorHeight);
    plane.setPosition(projectorWidth/2, projectorHeight/2, 0);
    
    udpConnection.Create();
    udpConnection.Connect("127.0.0.1", 11999);
    udpConnection.SetNonBlocking(true);
    enableUDP = false;
}

void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    if (blackMagic.update())
    {
        srcGrayImg.setFromPixels(blackMagic.getGrayPixels());
        if (willLearnBg)
        {
            bgGrayImg = srcGrayImg;
            willLearnBg = false;
        }
        
        diffGrayImg.absDiff(bgGrayImg, srcGrayImg);
        diffGrayImg.threshold(thresh);
        diffGrayImg.updateTexture();
        
        fbo.begin();
        shader.begin();
        diffGrayImg.getTexture().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
        diffGrayImg.getTexture().bind();
        plane.mapTexCoordsFromTexture(diffGrayImg.getTexture());
        shader.setUniformMatrix3f("warpMatrix", warpMatrix);
        plane.draw();
        diffGrayImg.getTexture().unbind();
        shader.end();
        fbo.end();
        
        fbo.readToPixels(pixels);
        contourFinder.findContours(ofxCv::toCv(pixels));
        
        for (int i = 0; i < maxBlobsCount; ++i)
        {
            polylines[i].clear();
        }
        
        for (int i = 0; i < contourFinder.size(); ++i) {
            polylines[i] = contourFinder.getPolyline(i);
            polylines[i].setClosed(false);
            polylines[i].simplify(10);
        }
    }
    
    if (enableUDP) {
        sendVertices(polylines);
    }
}

void ofApp::draw(){
    
    ofPolyline polyline;
    polyline.resize(ofPinCoords.size());
    for (int i = 0; i < ofPinCoords.size(); ++i)
    {
        polyline[i].x = ofPinCoords[i].x;
        polyline[i].y = ofPinCoords[i].y;
    }
    polyline.close();
    polyline.setClosed(true);
    
    switch (currentStatus) {
        case Status::Setup :
        {
            if (isMaskedWhite)
            {
                ofPushStyle();
                ofSetColor(255);
                ofDrawRectangle(0, 0, projectorWidth, projectorHeight);
                ofPopStyle();
            }
            else
            {
                fbo.draw(0, 0);
                
                ofPushStyle();
                ofSetLineWidth(3);
                ofSetColor(ofColor::gray);
                
                polyline.draw();
                for (auto p : ofPinCoords)
                {
                    ofDrawCircle(p.x, p.y, cornerPinRadius);
                }
                ofPopStyle();
            }
            break;
        }
            
        case Status::Play :
            ofPushStyle();
            ofSetLineWidth(5);
            ofSetColor(ofColor::gray);
            
            for (int i = 0; i < MIN(maxBlobsCount, contourFinder.size()); ++i)
            {
                polylines[i].draw();
            }
            
            ofPopStyle();
            break;
            
        default:
            break;
    }
}

void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
            willLearnBg = true;
            break;
            
        case 'p':
            currentStatus = Status::Play;
            break;
            
        case 's':
            currentStatus = Status::Setup;
            break;
            
        case OF_KEY_RETURN:
        {
            cv::Mat m = cv::getPerspectiveTransform(ofxCv::toCv(ofPinCoords), ofxCv::toCv(ofWindowCorners));
            m = m.inv();
            
            warpMatrix.set(m.at<double>(0,0), m.at<double>(1,0), m.at<double>(2,0),
                           m.at<double>(0,1), m.at<double>(1,1), m.at<double>(2,1),
                           m.at<double>(0,2), m.at<double>(1,2), m.at<double>(2,2));

            xmlSettings.setValue("a", warpMatrix.a);
            xmlSettings.setValue("b", warpMatrix.b);
            xmlSettings.setValue("c", warpMatrix.c);
            xmlSettings.setValue("d", warpMatrix.d);
            xmlSettings.setValue("e", warpMatrix.e);
            xmlSettings.setValue("f", warpMatrix.f);
            xmlSettings.setValue("g", warpMatrix.g);
            xmlSettings.setValue("h", warpMatrix.h);
            xmlSettings.setValue("i", warpMatrix.i);
            xmlSettings.saveFile("settings.xml");
            break;
        }
            
        case OF_KEY_LEFT:
            thresh--;
            std::cout << "Thresh: " << ofToString(thresh) << std::endl;
            break;
            
        case OF_KEY_RIGHT:
            thresh++;
            std::cout << "Thresh: " << ofToString(thresh) << std::endl;
            break;
            
        case 'b':
            isMaskedBlack = !isMaskedBlack;
            break;
            
        case 'w':
            isMaskedWhite = !isMaskedWhite;
            break;
            
        case 'u':
            enableUDP = !enableUDP;
            std::cout << "UDP: " << ofToString(enableUDP) << std::endl;
            break;
            
        default:
            break;
    }
}

void ofApp::mouseDragged(int x, int y, int button){
    if (selectedCorner >= 0 && selectedCorner < 4)
    {
        ofPinCoords[selectedCorner].x = x;
        ofPinCoords[selectedCorner].y = y;
    }
    
    ofPolyline polyline;
    for (auto p : ofPinCoords)
    {
        polyline.addVertex(p);
    }
    
    cv::Mat m = cv::getPerspectiveTransform(ofxCv::toCv(ofPinCoords), ofxCv::toCv(ofWindowCorners));
    m = m.inv();
    
    warpMatrix.set(m.at<double>(0,0), m.at<double>(1,0), m.at<double>(2,0),
                   m.at<double>(0,1), m.at<double>(1,1), m.at<double>(2,1),
                   m.at<double>(0,2), m.at<double>(1,2), m.at<double>(2,2));
}

void ofApp::mousePressed(int x, int y, int button){
    for (int i = 0; i < ofPinCoords.size(); ++i) {
        if (ofPoint(x,y).distance(ofPoint(ofPinCoords[i].x, ofPinCoords[i].y)) < cornerPinRadius)
        {
            selectedCorner = i;
            break;
        }
    }
}

void ofApp::windowResized(int w, int h){

}

void ofApp::exit(ofEventArgs &args)
{
    blackMagic.close();
}

void ofApp::sendVertices(std::vector<ofPolyline> vertices)
{
    string msg;
    for (int i = 0; i < maxBlobsCount; ++i)
    {
        msg = ofToString(i);
            
        for (int j = 0; j < polylines[i].getVertices().size(); ++j)
        {
            ofVec2f p(polylines[i].getVertices()[j].x, polylines[i].getVertices()[j].y);
            ofVec2f pos = ofVec3f(ofMap(p.x, 0.0, projectorWidth, -unityWorldSize.x/2, unityWorldSize.x/2), ofMap(p.y, 0.0, projectorHeight, unityWorldSize.y/2, -unityWorldSize.y/2));
            msg += "|" + ofToString(pos.x, 3) + "," + ofToString(pos.y, 3);
        }
        udpConnection.Send(msg.c_str(), msg.length());
    }
}

void ofApp::maxBlobsCountChanged(int &maxBlobsCount)
{
    polylines.resize(maxBlobsCount);
}
