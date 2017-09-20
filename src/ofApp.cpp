#include "ofApp.h"


void ofApp::setup(){
    ofSetFrameRate(120);
    ofSetVerticalSync(true);
    
    currentStatus = Status::Setup;
    
    blackMagic.setup(srcVideoWidth, srcVideoHeight, srcVideoFPS);
    
    willLearnBg = true;
    isMaskedBlack = false;
    isMaskedWhite = false;
    thresh = 80;
    bgGrayImg.allocate(srcVideoWidth, srcVideoHeight);
    diffGrayImg.allocate(srcVideoWidth, srcVideoHeight);
    
    selectedCorner = -1;
    
    oFSrcPoints = {
        cv::Point2f(cornerPinRadius, cornerPinRadius),
        cv::Point2f(dstVideoWidth - cornerPinRadius, cornerPinRadius),
        cv::Point2f(dstVideoWidth - cornerPinRadius, dstVideoHeight - cornerPinRadius),
        cv::Point2f(cornerPinRadius, dstVideoHeight - cornerPinRadius)
    };
    
    oFDstPoints = {
        cv::Point2f(0, 0),
        cv::Point2f(dstVideoWidth, 0),
        cv::Point2f(dstVideoWidth, dstVideoHeight),
        cv::Point2f(0, dstVideoHeight)
    };
    
    unityPoints = {
        cv::Point2f(-unityScreenSize.x/2, unityScreenSize.y/2),
        cv::Point2f(unityScreenSize.x/2, unityScreenSize.y/2),
        cv::Point2f(unityScreenSize.x/2, -unityScreenSize.y/2),
        cv::Point2f(-unityScreenSize.x/2, -unityScreenSize.y/2)
    };
    
    cv::findHomography(oFDstPoints, unityPoints).convertTo(oF2UnityMat, CV_32FC1);
    
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
        contourFinder.findContours(diffGrayImg, 1000, 1280*720*0.5, 10, true);
        
        if (homographyMat.empty())
        {
            if (currentStatus == Status::Play)
                ofLogWarning() << "Homography Matrix is not set";
            return;
        }
        
        // Destroy all polygons
        for (int i = 0; i < maxBlobsCount; ++i)
        {
            polylines[i].clear();
        }
        
        // Create polygons
        for (int i = 0; i < contourFinder.blobs.size(); ++i)
        {
            for (int j = 0; j < contourFinder.blobs[i].pts.size(); ++j)
            {
                cv::Point3f point = ofxCv::toCv(ofVec3f(contourFinder.blobs[i].pts[j].x * 0.8, contourFinder.blobs[i].pts[j].y * 1.067, 1.0));
                cv::Mat srcMat = cv::Mat(point, CV_32FC1);
                cv::Mat dstMat = homographyMat * srcMat;
                dstMat /= dstMat.at<float>(0, 2);
                
                ofVec2f vertex = ofVec2f(dstMat.at<float>(0, 0), dstMat.at<float>(0, 1));
                polylines[i].addVertex(vertex);
                
            }
            polylines[i].setClosed(false);
            polylines[i].simplify(10);
        }
    }
    
    if (enableUDP) {
        sendVertices(polylines);
    }
}

void ofApp::draw(){
    ofPolyline polyline = ofxCv::toOf(oFSrcPoints);
    polyline.setClosed(true);
    
    switch (currentStatus) {
        case Status::Setup :
        {
            if (isMaskedBlack)
            {
                ofPushStyle();
                ofSetColor(0);
                ofDrawRectangle(0, 0, dstVideoWidth, dstVideoHeight);
                ofPopStyle();
            }
            
            else if (isMaskedWhite)
            {
                ofPushStyle();
                ofSetColor(255);
                ofDrawRectangle(0, 0, dstVideoWidth, dstVideoHeight);
                ofPopStyle();
            }
            
            else
            {
                ofPushStyle();
                ofSetColor(255);
                ofDrawRectangle(0, 0, dstVideoWidth, dstVideoHeight);
                ofPopStyle();
                
                diffGrayImg.draw(0, 0, dstVideoWidth, dstVideoHeight);
                contourFinder.draw(0, 0, dstVideoWidth, dstVideoHeight);
                
                ofPushStyle();
                ofSetLineWidth(3);
                ofSetColor(ofColor::bisque);
                
                polyline.draw();
                for (auto p : oFSrcPoints)
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
            
            for (int i = 0; i < contourFinder.blobs.size(); ++i)
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
            cv::findHomography(oFSrcPoints, oFDstPoints).convertTo(homographyMat, CV_32FC1);
            ofLog() << "homographyMatrix:\n" << homographyMat;
            
            for (int y = 0; y < homographyMat.rows; ++y)
            {
                for (int x = 0; x < homographyMat.cols; ++x)
                {
                    settings.setValue("settings:homographyMatrixValue" + ofToString(y) + "," + ofToString(x), homographyMat.at<float>(y, x));
                }
            }
            settings.saveFile("settings.xml");
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
        oFSrcPoints[selectedCorner].x = x;
        oFSrcPoints[selectedCorner].y = y;
    }
    
    ofPolyline polyline;
    for (auto p : oFSrcPoints)
    {
        polyline.addVertex(ofxCv::toOf(p));
    }
    
    oFSrcRect = polyline.getBoundingBox();
}

void ofApp::mousePressed(int x, int y, int button){
    for (int i = 0; i < oFSrcPoints.size(); ++i) {
        if (ofPoint(x,y).distance(ofPoint(oFSrcPoints[i].x, oFSrcPoints[i].y)) < cornerPinRadius)
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

void ofApp::sendVertices(std::array<ofPolyline, maxBlobsCount> vertices)
{
    string msg;
    
    for (int i = 0; i < maxBlobsCount; ++i)
    {
        msg = ofToString(i);
            
        for (int j = 0; j < polylines[i].getVertices().size(); ++j)
        {
            cv::Point3f point = ofxCv::toCv(ofVec3f(polylines[i].getVertices()[j].x, polylines[i].getVertices()[j].y, 1));
            cv::Mat srcMat = cv::Mat(point, CV_32FC1);
            cv::Mat dstMat = oF2UnityMat * srcMat;
            dstMat /= dstMat.at<float>(0, 2);
            ofVec2f pos = ofVec2f(dstMat.at<float>(0, 0), dstMat.at<float>(0, 1));
            msg += "|" + ofToString(pos.x, 3) + "," + ofToString(pos.y, 3);
        }
        udpConnection.Send(msg.c_str(), msg.length());
    }
}
