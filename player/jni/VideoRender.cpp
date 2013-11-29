#include "VideoRender.h"



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
    "varying lowp vec2 tc;                          \n"
    "uniform sampler2D SamplerY;                    \n"
    "uniform sampler2D SamplerU;                    \n"
    "uniform sampler2D SamplerV;                    \n"
    "void main(void)                                \n"
    "{                                              \n"
        "mediump vec3 yuv;                          \n"
        "lowp vec3 rgb;                             \n"
        "yuv.x = texture2D(SamplerY, tc).r;         \n"
        "yuv.y = texture2D(SamplerU, tc).r - 0.5;   \n"
        "yuv.z = texture2D(SamplerV, tc).r - 0.5;   \n"
        "rgb = mat3( 1, 1, 1,                       \n"
                    "0, -0.39465, 2.03211,          \n"
                    "1.13983, -0.58060, 0) * yuv;   \n"
        "gl_FragColor = vec4(rgb, 1);               \n"
    "}                                              \n";

//
// Shader.vert文件内容
//
static const char* VERTEX_SHADER =
      "attribute vec4 vPosition;    \n"
      "attribute vec2 a_texCoord;	\n"
      "varying vec2 tc;             \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "	  tc = a_texCoord;	        \n"
      "}                            \n";



VideoRender::VideoRender()
    : m_width(0), m_height(0), m_viewport_width(0), m_viewport_height(0)
    , m_image_buffer(NULL), m_image_buffer_size(0)
    , pp_c(NULL), pp_m(NULL)
{
    simpleProgram = buildProgram(VERTEX_SHADER, FRAG_SHADER);
    glUseProgram(simpleProgram);
    glGenTextures(1, &m_texYId);
    glGenTextures(1, &m_texUId);
    glGenTextures(1, &m_texVId);
}

VideoRender::~VideoRender()
{
    if(pp_m){  
        pp_free_mode( pp_m );  
        pp_m = 0;  
    }  
    if(pp_c){  
        pp_free_context(pp_c);  
        pp_c = 0;  
    }  
    
    if (m_image_buffer)
        delete[] m_image_buffer;
}

GLuint VideoRender::bindTexture(GLuint texture, const char *buffer, GLuint w , GLuint h)
{
//  GLuint texture;
//  glGenTextures ( 1, &texture );
//  checkGlError("glGenTextures");
    glBindTexture ( GL_TEXTURE_2D, texture );
    checkGlError("glBindTexture");
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    checkGlError("glTexImage2D");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    checkGlError("glTexParameteri");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    checkGlError("glTexParameteri");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    checkGlError("glTexParameteri");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    checkGlError("glTexParameteri");
    //glBindTexture(GL_TEXTURE_2D, 0);

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
        checkGlError("glAttachShader");
        glAttachShader(programHandle, fragmentShader);
        checkGlError("glAttachShader");
        glLinkProgram(programHandle);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
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

#if 1
// Galaxy Nexus 4.2.2
static GLfloat gl_squareVertices[] = {
    -1.0f, -1.0f, 1.0f, -1.0f,
    -1.0f,  1.0f, 1.0f,  1.0f
};

static GLfloat gl_coordVertices[] = {
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 0.0f
};
#else
// HUAWEIG510-0010 4.1.1
static GLfloat gl_squareVertices[] = {
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
};

static GLfloat gl_coordVertices[] = {
    -1.0f,  1.0f, 1.0f,  1.0f,
    -1.0f, -1.0f, 1.0f, -1.0f,
};
#endif

void VideoRender::render_frame()
{
    // render

    uint32_t& width = m_width;
    uint32_t& height = m_height;
    LOGI("[VideoRender::render_frame] size=%dx%d, viewport=%dx%d", width, height, m_viewport_width, m_viewport_height);
    if (width == 0 || height == 0)
        return;

    glViewport(0, 0, m_viewport_width, m_viewport_height);

    AutoLock lock_frame(*m_frame_lock);

#if 0

    // convert
    if (m_image_buffer_size < ((width*height) << 2)) {
        m_image_buffer_size = width*height*2;
        if (m_image_buffer) delete[] m_image_buffer;
        m_image_buffer = new uint8_t[m_image_buffer_size];
        memset(m_image_buffer, 0, m_image_buffer_size);
    }

    uint8_t* showImage[3];
    uint32_t showheight[3],showLx[3];

    showImage[0] = m_image_buffer;
    showImage[1] = showImage[0] + width*height;
    showImage[2] = showImage[1] + width*height/4;
    showLx[0] = width;
    showLx[1] = width >> 1;
    showLx[2] = width >> 1;
    
    showheight[0] = height;
    showheight[1] = height >> 1;
    showheight[2] = height >> 1;

    if (m_sem.tryWait()) {
        if (pp_c == NULL)
            pp_c = pp_get_context( width, height, NULL);
        if (pp_m == NULL)
            pp_m = pp_get_mode_by_name_and_quality( "ac", PP_QUALITY_MAX );
        
        int qstride, qp_type;
        int8_t *qp_table = av_frame_get_qp_table(m_frame, &qstride, &qp_type);
        
        pp_postprocess((const uint8_t**)m_frame->data, m_frame->linesize, showImage, (const int*)showLx, width, height,
                       qp_table, qstride, pp_m, pp_c, m_frame->pict_type);
    }
    // bindTexture(m_texYId, (const char*)showImage[0], showLx[0], showheight[0]);
    // bindTexture(m_texUId, (const char*)showImage[1], showLx[1], showheight[1]);
    // bindTexture(m_texVId, (const char*)showImage[2], showLx[2], showheight[2]);
#else
    bindTexture(m_texYId, (const char*)m_frame->data[0], m_frame->linesize[0], height);
    bindTexture(m_texUId, (const char*)m_frame->data[1], m_frame->linesize[1], height/2);
    bindTexture(m_texVId, (const char*)m_frame->data[2], m_frame->linesize[2], height/2);
#endif
    
    // clear background
    // glClearColor(0.5f, 0.5f, 0.5f, 1);
    // checkGlError("glClearColor");
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");


    //PRINTF("setsampler %d %d %d", g_texYId, g_texUId, g_texVId);
    GLint tex_y = glGetUniformLocation(simpleProgram, "SamplerY");
    checkGlError("glGetUniformLocation");
    GLint tex_u = glGetUniformLocation(simpleProgram, "SamplerU");
    checkGlError("glGetUniformLocation");
    GLint tex_v = glGetUniformLocation(simpleProgram, "SamplerV");
    checkGlError("glGetUniformLocation");


    glBindAttribLocation(simpleProgram, ATTRIB_VERTEX, "vPosition");
    checkGlError("glBindAttribLocation");
    glBindAttribLocation(simpleProgram, ATTRIB_TEXTURE, "a_texCoord");
    checkGlError("glBindAttribLocation");

    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, gl_squareVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, gl_coordVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    checkGlError("glEnableVertexAttribArray");

    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, m_texYId);
    checkGlError("glBindTexture");
    glUniform1i(tex_y, 0);
    checkGlError("glUniform1i");

    glActiveTexture(GL_TEXTURE1);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, m_texUId);
    checkGlError("glBindTexture");
    glUniform1i(tex_u, 1);
    checkGlError("glUniform1i");

    glActiveTexture(GL_TEXTURE2);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, m_texVId);
    checkGlError("glBindTexture");
    glUniform1i(tex_v, 2);
    checkGlError("glUniform1i");

    // glEnable(GL_TEXTURE_2D);
    // checkGlError("glEnable");
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    checkGlError("glDrawArrays");
}

