#pragma once

namespace contourfinder{
    
    struct Settings{
        const int         videoWidth       = 1280;
        const int         videoHeight      = 720;
        const float       videoFramerate   = 59.94f;
        const int         projectorWidth   = 1024;
        const int         projectorHeight  = 768;
        const float       unityWorldWidth  = 20.f;
        const float       unityWorldHeight = 15.f;
        const char *      udpIp            = "127.0.0.1";
        const int         udpPort          = 11999;
    };
}; // namespace contourfinder
