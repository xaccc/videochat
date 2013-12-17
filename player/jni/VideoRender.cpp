#include "VideoRender.h"



#undef LOGI
#define LOGI(...)

///////////////////////////////////////////////////////////////////////////////
//
// class VideoRender 实现
//
///////////////////////////////////////////////////////////////////////////////


static void checkGlError(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
    {
        LOGI("[OpenGL ERROR]::after %s() glError (0x%x)\n", op, error);
    }
}


//
//Shader.frag文件内容
//
static const char* FRAG_SHADER =
        "varying lowp vec2 tc;                              \n"
        "uniform sampler2D SamplerY;                        \n"
        "uniform sampler2D SamplerU;                        \n"
        "uniform sampler2D SamplerV;                        \n"
        "void main(void)                                    \n"
        "{                                                  \n"
        "   mediump vec3 yuv;                               \n"
        "   lowp vec3 rgb;                                  \n"
        "   yuv.x = texture2D(SamplerY, tc).r;              \n"
        "   yuv.y = texture2D(SamplerU, tc).r - 0.5;        \n"
        "   yuv.z = texture2D(SamplerV, tc).r - 0.5;        \n"
        "   rgb = mat3(       1,        1,       1,         \n"
        "                     0, -0.39465, 2.03211,         \n"
        "               1.13983, -0.58060,       0) * yuv; 	\n"
        "   gl_FragColor = 1.1 * vec4(rgb, 1);              \n"
        "}                                                  \n";

//
// Shader.vert文件内容
//
static const char* VERTEX_SHADER =
        "attribute vec4 ppp;    \n"
        "attribute vec2 xxx;    \n"
        "varying vec2 tc;       \n"
        "void main()            \n"
        "{                      \n"
        "   gl_Position = ppp;  \n"
        "   tc = xxx;           \n"
        "}                      \n";



VideoRender::VideoRender()
    : m_width(0), m_height(0), m_paused(false)
{
    simpleProgram = buildProgram(VERTEX_SHADER, FRAG_SHADER);
    glUseProgram(simpleProgram);

    glBindAttribLocation(simpleProgram, ATTRIB_VERTEX, "ppp");
    glBindAttribLocation(simpleProgram, ATTRIB_TEXTURE, "xxx");

    glGenTextures(1, &m_texYId);
    glGenTextures(1, &m_texUId);
    glGenTextures(1, &m_texVId);
    
    m_myPicture = (AVPicture*)av_malloc(sizeof(AVPicture));
    memset(m_myPicture, 0, sizeof(AVPicture));
    m_myPictureSize = 0;
}

VideoRender::~VideoRender()
{
    if (m_myPicture->data[0])
        av_free(m_myPicture->data[0]);
    av_free(m_myPicture);
}

void VideoRender::pause(bool paused)
{
    m_paused = paused;
}

GLuint VideoRender::bindTexture(GLuint texture, const char *buffer, GLuint w , GLuint h)
{
    glBindTexture ( GL_TEXTURE_2D, texture );
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    return texture;
}

GLuint VideoRender::buildShader(const char* source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);

    if (shaderHandle)
    {
        glShaderSource(shaderHandle, 1, &source, 0);
        glCompileShader(shaderHandle);

        GLint compiled = 0;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);
                    LOGI("[OpenGL ERROR]::Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shaderHandle);
                shaderHandle = 0;
            }
        }else{
            LOGI("[VideoRender::buildShader] shader compiled!\n");
        }
    }

    return shaderHandle;
}

GLuint VideoRender::buildProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint vertexShader = buildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = buildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint programHandle = glCreateProgram();

    if (programHandle)
    {
        glAttachShader(programHandle, vertexShader);
        glAttachShader(programHandle, fragmentShader);
        glLinkProgram(programHandle);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char* buf = (char*) malloc(bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(programHandle, bufLength, NULL, buf);
                    LOGI("[OpenGL ERROR]::Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(programHandle);
            programHandle = 0;
        }else{
            LOGI("[VideoRender::buildProgram] program linked!\n");
        }
    }

    return programHandle;
}

// Galaxy Nexus 4.2.2
static GLfloat gl_squareVertices[] = {
		-1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f
};
static GLfloat gl_coordVertices[] =  {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f
};


void VideoRender::setViewport(int width, int height) 
{
    glViewport(0, 0, width, height);
}

void VideoRender::clearFrame()
{
    AutoLock lock_frame(m_myPictureLock);
    if (m_myPicture->data[0])
        av_free(m_myPicture->data[0]);
    
    m_myPicture->data[0] = NULL;
    m_myPictureSize = 0;
    m_width = m_height = 0;
}

void VideoRender::setFrame(AVFrame* frame, uint32_t width, uint32_t height)
{
    AutoLock lock_frame(m_myPictureLock);
    
    if ( m_width != width || m_height != height ) {
        // realloc picture
        if (m_myPicture->data[0])
            av_free(m_myPicture->data[0]);
        
        m_myPicture->data[0] = NULL;
        m_myPictureSize = av_image_alloc(
            m_myPicture->data, m_myPicture->linesize,
            width, height, AV_PIX_FMT_YUV420P, 1);
    }
    
    if (m_myPictureSize > 0 && m_myPicture->data[0]) {
        m_width = width;
        m_height = height;
        av_image_copy(m_myPicture->data, m_myPicture->linesize,
        		(const uint8_t**)frame->data, frame->linesize,
        		AV_PIX_FMT_YUV420P, width, height);
    } else {
        LOGE("[VideoRender::set_frame] alloc picture memory failed!");
    }
}

void VideoRender::renderFrame()
{
    AutoLock lock_frame(m_myPictureLock);

    // render
    if (m_myPicture->data[0] == NULL || m_paused)
    {
        // clear background
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }
    
    bindTexture(m_texYId, (const char*)m_myPicture->data[0], m_myPicture->linesize[0], m_height);
    bindTexture(m_texUId, (const char*)m_myPicture->data[1], m_myPicture->linesize[1], m_height/2);
    bindTexture(m_texVId, (const char*)m_myPicture->data[2], m_myPicture->linesize[2], m_height/2);
    
    glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, gl_squareVertices);
    glEnableVertexAttribArray(ATTRIB_VERTEX);

    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, gl_coordVertices);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    GLint tex_y = glGetUniformLocation(simpleProgram, "SamplerY");
    GLint tex_u = glGetUniformLocation(simpleProgram, "SamplerU");
    GLint tex_v = glGetUniformLocation(simpleProgram, "SamplerV");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texYId);
    glUniform1i(tex_y, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texUId);
    glUniform1i(tex_u, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_texVId);
    glUniform1i(tex_v, 2);

    glEnable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}

