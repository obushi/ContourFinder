#include "VideoSource.hpp"

cv::Mat contourfinder::BaslerCamera::result;
ofxCvGrayscaleImage contourfinder::BaslerCamera::image;

void contourfinder::VideoSource::setup(contourfinder::Settings const & settings)
{
    blackmagic.setup(settings.videoWidth, settings.videoHeight, settings.videoFramerate);
    
    contourfinder::BaslerCamera::image.allocate(settings.videoWidth, settings.videoHeight);
//    Pylon::PylonAutoInitTerm autoInitTerm;
    Pylon::PylonInitialize();
    camera.instantCamera = new Pylon::CBaslerUsbInstantCamera(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
    
    camera.instantCamera->RegisterConfiguration(new contourfinder::BaslerConfigEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);
    camera.instantCamera->RegisterImageEventHandler(new contourfinder::BaslerImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);
    std::cout << "Using device: " << camera.instantCamera->GetDeviceInfo().GetModelName() << std::endl;
    camera.instantCamera->StartGrabbing(Pylon::GrabStrategy_OneByOne, Pylon::GrabLoop_ProvidedByInstantCamera);
}

bool contourfinder::VideoSource::update()
{
//    return blackmagic.update();
    return BaslerCamera::result.ptr() != nullptr;
}

ofxCvGrayscaleImage & contourfinder::VideoSource::getGrayscalePixels(){
    BaslerCamera::image.setFromPixels(BaslerCamera::result.ptr(), BaslerCamera::result.cols, BaslerCamera::result.rows);
    return BaslerCamera::image;
}

void contourfinder::VideoSource::close(){
    blackmagic.close();
    camera.instantCamera->StopGrabbing();
}

void contourfinder::BaslerConfigEventHandler::OnOpened(Pylon::CBaslerUsbInstantCamera &camera){
    try{
        std::cout << "(OnOpened) Using device: " << camera.GetDeviceInfo().GetModelName() << std::endl;
        
        GenApi::INodeMap & control = camera.GetNodeMap();
        const GenApi::CIntegerPtr width = control.GetNode("Width");
        const GenApi::CIntegerPtr height = control.GetNode("Height");
        const GenApi::CIntegerPtr offsetX = control.GetNode("OffsetX");
        const GenApi::CIntegerPtr offsetY = control.GetNode("OffsetY");
        
        if(GenApi::IsWritable(width)){
            width->SetValue(1920);
        }
        if(GenApi::IsWritable(height)){
            height->SetValue(1080);
        }
        if(GenApi::IsWritable(offsetX)){
            offsetX->SetValue(0);
        }
        if(GenApi::IsWritable(offsetY)){
            offsetY->SetValue(0);
        }
        
        GenApi::CEnumerationPtr(control.GetNode("PixelFormat"))->FromString("Mono8");
    }
    catch(const GenericException & e){
        throw RUNTIME_EXCEPTION("Could not apply configuration. Messsage: ", e.what());
    }
}

void contourfinder::BaslerImageEventHandler::OnImageGrabbed(Pylon::CBaslerUsbInstantCamera & camera, const Pylon::CBaslerUsbGrabResultPtr & grabResult){
    try{
        Pylon::CImageFormatConverter fc;
        Pylon::CPylonImage image;
        fc.OutputPixelFormat = Pylon::PixelType_RGB8packed;
        cv::Mat mat;
        
        if(grabResult->GrabSucceeded()){
            fc.Convert(image, grabResult);
            mat = cv::Mat(grabResult->GetHeight(), grabResult->GetWidth(), CV_8UC3, (uint8_t *)image.GetBuffer());
            cv::cvtColor(mat, BaslerCamera::result, CV_RGB2GRAY);
        }
    }
    catch(const GenericException & e){
        std::cout << "Exception: " << e.what() << std::endl;
    }
}
