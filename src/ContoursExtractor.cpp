#include "ContoursExtractor.hpp"

void contourfinder::ContoursExtractor::setup(contourfinder::Settings const & settings){
    bgGrayImg.allocate(settings.videoWidth, settings.videoHeight);
    srcGrayImg.allocate(settings.videoWidth, settings.videoHeight);
    diffGrayImg.allocate(settings.videoWidth, settings.videoHeight);
    
    contourFinder.setMaxArea(settings.videoWidth * settings.videoHeight * 0.5);
    contourFinder.setMinArea(settings.videoWidth * settings.videoHeight * 0.001);
    contourFinder.setFindHoles(false);
    contourFinder.setThreshold(threashold);
    
    shader.load("shaders/warpPerspective");
    shaderWarpMatrix = ofMatrix3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    fbo.allocate(settings.projectorWidth, settings.projectorHeight);
    plane.set(settings.projectorWidth, settings.projectorHeight);
    plane.setPosition(settings.projectorWidth/2, settings.projectorHeight/2, 0);
    learnBackground = true;
}

void contourfinder::ContoursExtractor::update(ofxCvGrayscaleImage & newFrame){
    srcGrayImg = newFrame;
    
    if(learnBackground){
        bgGrayImg = srcGrayImg;
        learnBackground = false;
    }
    
    diffGrayImg.absDiff(bgGrayImg, srcGrayImg);
    diffGrayImg.threshold(threashold);
    diffGrayImg.updateTexture();
    
    fbo.begin();
    shader.begin();
    diffGrayImg.getTexture().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    diffGrayImg.getTexture().bind();
    plane.mapTexCoordsFromTexture(diffGrayImg.getTexture());
    shader.setUniformMatrix3f("warpMatrix", shaderWarpMatrix);
    plane.draw();
    diffGrayImg.getTexture().unbind();
    shader.end();
    fbo.end();

    fbo.readToPixels(pixels);
    contourFinder.findContours(ofxCv::toCv(pixels));
    
    for(auto & p : polylines){
        p.clear();
    }
    
    for(int i = 0; i < MIN(maxBlobsCount, contourFinder.size()); ++i){
        polylines[i] = contourFinder.getPolyline(i);
        polylines[i].setClosed(false);
        polylines[i].simplify(polylinesSimplicity);
    }
}

void contourfinder::ContoursExtractor::toggleLearnBackground(){
    this->learnBackground = !(this->learnBackground);
}

void contourfinder::ContoursExtractor::setThreashold(int threashold){
    this->threashold = threashold;
}

void contourfinder::ContoursExtractor::setSimplicity(int polylinesSimplicity){
    this->polylinesSimplicity = polylinesSimplicity;
}

void contourfinder::ContoursExtractor::setWarpPerspectiveTransform(std::vector<ofVec2f> const & ofCaptureAreaCorners, std::vector<ofVec2f> const & ofVideoCorners, std::vector<ofVec2f>  const & ofWindowCorners){
    cv::Mat m = cv::getPerspectiveTransform(ofxCv::toCv(ofCaptureAreaCorners), ofxCv::toCv(ofVideoCorners));
    m = m.inv();
    ofMatrix3x3 ofMat;
    ofMat.set(m.at<double>(0,0), m.at<double>(1,0), m.at<double>(2,0),
                   m.at<double>(0,1), m.at<double>(1,1), m.at<double>(2,1),
                   m.at<double>(0,2), m.at<double>(1,2), m.at<double>(2,2));
    
    cv::Mat m2 = cv::getPerspectiveTransform(ofxCv::toCv(ofVideoCorners), ofxCv::toCv(ofWindowCorners));
    m2 = m2.inv();
    ofMatrix3x3 ofMat2;
    ofMat2.set(m2.at<double>(0,0), m2.at<double>(1,0), m2.at<double>(2,0),
                   m2.at<double>(0,1), m2.at<double>(1,1), m2.at<double>(2,1),
                   m2.at<double>(0,2), m2.at<double>(1,2), m2.at<double>(2,2));
    
    warpMatrix = ofMat * ofMat2;
}

void contourfinder::ContoursExtractor::applyWarpPerspectiveTransform(){
    shaderWarpMatrix = warpMatrix;
}

void contourfinder::ContoursExtractor::resetWarpPerspectiveTransform(){
    shaderWarpMatrix = ofMatrix3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
}

void contourfinder::ContoursExtractor::setMaxblobsCount(int maxBlobsCount){
    this->maxBlobsCount = maxBlobsCount;
    polylines.resize(maxBlobsCount);
}

std::vector<ofPolyline> const & contourfinder::ContoursExtractor::getPolylines(){
    return polylines;
}

int contourfinder::ContoursExtractor::contoursCount(){
    return contourFinder.size();
}

void contourfinder::ContoursExtractor::drawContours(){
    fbo.draw(0,0);
}

void contourfinder::ContoursExtractor::draw(){
    for(auto & p : polylines){
        p.draw();
    }
}
