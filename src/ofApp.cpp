#include "ofApp.h"

void ofApp::setup(){
    ofSetFrameRate(120);
    ofSetVerticalSync(false);
    
    threashold.addListener(this, & ofApp::threasholdChanged);
    maxBlobsCount.addListener(this, & ofApp::maxBlobsCountChanged);
    polylinesSimplicity.addListener(this, & ofApp::polylinesSimplicityChanged);
    
    gui.setup();
    gui.add(fps.set("fps", ""));
    gui.add(threashold.set("threashold", 80, 0, 150));
    gui.add(maxBlobsCount.set("maxBlobsCount", 10, 0, 50));
    gui.add(polylinesSimplicity.set("polylinesSimplicity", 10, 0, 20));
    gui.add(cornerPinRadius.set("cornerPinRadius", 12, 1, 50));
    
    initializeCorners();
    
    currentStatus = Status::Setup;
    selectedCorner = -1;
    isMaskedWhite = false;
    drawGui = true;
    
    videoSource.setup(settings);
    contoursExtractor.setup(settings);
    contoursExtractor.setWarpPerspectiveTransform(ofCaptureAreaCorners, ofWindowCorners);
    contoursSender.setup(settings);
}

void ofApp::update(){
    fps = ofToString(ofGetFrameRate());
    
    if(videoSource.update()){
        contoursExtractor.update(videoSource.getGrayscalePixels());
    }
    
    if(udpEnabled) {
        contoursSender.sendVertices(contoursExtractor.getPolylines(), maxBlobsCount);
    }
}

void ofApp::draw(){
    ofPolyline polyline;
    polyline.resize(ofCaptureAreaCorners.size());
    for(int i = 0; i < ofCaptureAreaCorners.size(); ++i){
        polyline[i].x = ofCaptureAreaCorners[i].x;
        polyline[i].y = ofCaptureAreaCorners[i].y;
    }
    polyline.close();
    polyline.setClosed(true);
    
    switch(currentStatus){
        case Status::Setup :
            if(isMaskedWhite){
                ofBackground(ofColor::white);
            }
            else{
                contoursExtractor.drawContours();
            }
            break;
            
        case Status::Play :
            ofPushStyle();
            ofSetColor(ofColor::gray);
            contoursExtractor.draw();
            ofPopStyle();
            break;
            
        default:
            break;
    }
    
    if(drawGui){
        ofPushStyle();
        ofNoFill();
        ofSetColor(ofColor::gray);
        ofDrawRectangle(ofRectangle(ofPoint(1.0, 1.0), ofPoint(settings.projectorWidth - 1.0, settings.projectorHeight - 1.0)));
        polyline.draw();
        for(auto & p : ofCaptureAreaCorners){
            ofDrawCircle(p.x, p.y, cornerPinRadius);
        }
        ofPopStyle();
        gui.draw();
    }
}

void ofApp::keyPressed(int key){
    switch(key){
        case ' ':
            contoursExtractor.toggleLearnBackground();
            break;
            
        case 'g':
            drawGui = !drawGui;
            break;
            
        case 'p':
            currentStatus = Status::Play;
            break;
            
        case 's':
            currentStatus = Status::Setup;
            break;
            
        case OF_KEY_LEFT:
            threashold--;
            ofLog() << "Threshold: " << ofToString(threashold) << std::endl;
            break;
            
        case OF_KEY_RIGHT:
            threashold++;
            ofLog() << "Threshold: " << ofToString(threashold) << std::endl;
            break;
            
        case 'w':
            isMaskedWhite = !isMaskedWhite;
            break;
            
        case 'u':
            udpEnabled = !udpEnabled;
            ofLog() << "UDP: " << ofToString(udpEnabled) << std::endl;
            break;
            
        default:
            break;
    }
}

void ofApp::mouseDragged(int x, int y, int button){
    if(selectedCorner >= 0 && selectedCorner < 4){
        ofVec2f p = ofVec2f(x, y);
        ofCaptureAreaCorners[selectedCorner] = p;
    }
    
    ofPolyline polyline;
    for(auto & p : ofCaptureAreaCorners){
        polyline.addVertex(p);
    }
    
    contoursExtractor.setWarpPerspectiveTransform(ofCaptureAreaCorners, ofWindowCorners);
}

void ofApp::mousePressed(int x, int y, int button){
    for(const auto & p : ofCaptureAreaCorners){
        if(ofPoint(x,y).distance(ofPoint(p.x, p.y)) < cornerPinRadius){
            std::vector<ofVec2f>::iterator it = std::find(ofCaptureAreaCorners.begin(), ofCaptureAreaCorners.end(), p);
            selectedCorner = std::distance(ofCaptureAreaCorners.begin(), it);
            break;
        }
    }
}

void ofApp::exit(ofEventArgs &args){
    videoSource.close();
}

void ofApp::threasholdChanged(int & threashold){
    contoursExtractor.setThreashold(threashold);
}

void ofApp::maxBlobsCountChanged(int & maxBlobsCount){
    contoursExtractor.setMaxblobsCount(maxBlobsCount);
}

void ofApp::polylinesSimplicityChanged(int & polylinesSimplicity){
    contoursExtractor.setSimplicity(polylinesSimplicity);
}

void ofApp::initializeCorners(){
    std::function<std::vector<ofVec2f>(std::vector<ofPoint>)> toVec2f = [](std::vector<ofPoint> vec3fPoints){
        std::vector<ofVec2f> vec2fPoints;
        for(auto p : vec3fPoints){
            vec2fPoints.push_back(ofVec2f(p.x, p.y));
        }
        return vec2fPoints;
    };
    
    ofVideoCorners = toVec2f(ofPolyline::fromRectangle(ofRectangle(ofPoint(0.0, 0.0), ofPoint(settings.videoWidth, settings.videoHeight))).getVertices());
    ofWindowCorners = toVec2f(ofPolyline::fromRectangle(ofRectangle(ofPoint(0.0, 0.0), ofPoint(settings.projectorWidth, settings.projectorHeight))).getVertices());
    unityWorldCorners = toVec2f(ofPolyline::fromRectangle(ofRectangle(ofPoint(-settings.unityWorldWidth/2, settings.unityWorldHeight/2), ofPoint(settings.unityWorldWidth/2, -settings.unityWorldHeight/2))).getVertices());
    ofCaptureAreaCorners = toVec2f(ofPolyline::fromRectangle(ofRectangle(ofPoint(cornerPinRadius, cornerPinRadius), ofPoint(settings.projectorWidth - cornerPinRadius, settings.projectorHeight - cornerPinRadius))).getVertices());
}
