#include "VideoSource.hpp"

void contourfinder::VideoSource::setup(contourfinder::Settings const & settings)
{
    blackmagic.setup(settings.videoWidth, settings.videoHeight, settings.videoFramerate);
}

bool contourfinder::VideoSource::update()
{
    return blackmagic.update();
}

ofPixels & contourfinder::VideoSource::getGrayscalePixels()
{
    return blackmagic.getGrayPixels();
}

void contourfinder::VideoSource::close()
{
    blackmagic.close();
}
