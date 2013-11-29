#ifndef __VIDEORENDER_H__
#define __VIDEORENDER_H__


#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


extern "C" {

#ifndef   UINT64_C
#define   UINT64_C(value) __CONCAT(value,ULL)
#endif

#include "libavcodec/avcodec.h"
#include "libpostproc/postprocess.h"  

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

    void set_view(int width, int height) { m_viewport_width = width; m_viewport_height = height; }
    void set_size(uint32_t width, uint32_t height) { m_width = width; m_height = height; }
    void set_frame(AVFrame* frame, Mutex* frame_lock) { m_frame = frame; m_frame_lock = frame_lock; };
    void render_frame();
    void pause(bool paused);
    
    void ready2render(){m_sem.Post();}

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
    
    uint8_t* m_image_buffer;
    uint32_t m_image_buffer_size;
    
    AVFrame* m_frame;
    Mutex*   m_frame_lock;
    
    Semaphore m_sem;
    
    pp_context* pp_c;
    pp_mode*    pp_m;

    uint32_t m_width;
    uint32_t m_height;

    int m_viewport_width;
    int m_viewport_height;
    bool m_paused;
};


#endif  //__VIDEORENDER_H__