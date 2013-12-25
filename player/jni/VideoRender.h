#ifndef __VIDEORENDER_H__
#define __VIDEORENDER_H__


#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "VideoDecoder.h"

extern "C" {

#ifndef   UINT64_C
#define   UINT64_C(value) __CONCAT(value,ULL)
#endif

#if USEFFMPEG
#include "libavcodec/avcodec.h"
#include "libpostproc/postprocess.h"  
#include "libavutil/imgutils.h"
#endif

} // extern "C" 


#include "utils.h"

//
//
//
class VideoRender
{
public:
    VideoRender();
    ~VideoRender();

    void setViewport(int width, int height);
    void setFrame(AVFrame* frame, uint32_t width, uint32_t height);
    void clearFrame();
    void renderFrame();
    void pause(bool paused);
    
private:
    enum {
        ATTRIB_VERTEX,
        ATTRIB_TEXTURE,
    };

    GLuint bindTexture(GLuint texture, const char *buffer, GLuint w , GLuint h);
    GLuint buildShader(const char* source, GLenum shaderType);
    GLuint buildProgram(const char* vertexShaderSource, const char* fragmentShaderSource);

private:
    GLuint m_texYId;
    GLuint m_texUId;
    GLuint m_texVId;
    GLuint simpleProgram;
    
    Mutex      m_myPictureLock;
#if USEFFMPEG
    AVPicture* m_myPicture;
#else
    char*	   m_myPicture;
#endif
    int        m_myPictureSize;
    
    uint32_t m_width;
    uint32_t m_height;

    bool m_paused;
};


#endif  //__VIDEORENDER_H__
