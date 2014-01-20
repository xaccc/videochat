/**
 *
 * Copyright (C) 2013 ALiang (jjyyis@qq.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
 
#include "VideoChat.h"
#include "VideoChat_jni.h"
#include "VideoChat_OnEventCallback_jni.h"


//
//
//

JNIEXPORT jlong JNICALL Java_cn_videochat_VideoChat_Init(JNIEnv *env, jclass jobj)
{
    VideoChat* pVideoChat = new VideoChat();
    if (pVideoChat) {
        pVideoChat->Init();
        LOGI("pVideoChat->Init");
        pVideoChat->m_jObject = env->NewGlobalRef(jobj);
        LOGI("env->NewGlobalRef");
        env->GetJavaVM((JavaVM**)&pVideoChat->m_jVM);
        LOGI("env->GetJavaVM");
    }
    
    return (jlong)pVideoChat;
}

static void* shutdownObject(void* fuck){

	JNIEnv *env;
	JavaVM *jVM;
	VideoChat* pVideoChat = (VideoChat*)fuck;

	jVM = (JavaVM*)pVideoChat->m_jVM;

	jVM->AttachCurrentThread(&env, NULL);

    pVideoChat->Release();
    env->DeleteGlobalRef(pVideoChat->m_jObject);
    //((JavaVM*)pVideoChat->m_jVM)->DestroyJavaVM();
    delete pVideoChat;

    jVM->DetachCurrentThread();
	return 0;
}

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_Release(JNIEnv *env, jclass jobj, jlong handler)
{
    VideoChat* pVideoChat = (VideoChat*)handler;
    
    if (pVideoChat) {
    	pthread_t thread_shutdown;
    	pthread_create(&thread_shutdown, NULL, &shutdownObject, (void*)pVideoChat);
    }
}

//
// recoder
//

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_OpenPublisher(JNIEnv *, jobject, jlong, jstring) {}
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_PausePublisher(JNIEnv *, jobject, jlong, jboolean) {}
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_ClosePublisher(JNIEnv *, jobject, jlong) {}

//
// player
//

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_SetVideoRender(JNIEnv *env, jclass jobj, jlong handler, jlong renderHandler)
{
    VideoChat* pVideoChat = (VideoChat*)handler;
    
    if (pVideoChat) {
        pVideoChat->setVideoRender((VideoRender*)renderHandler);
    }
}

  
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_OpenPlayer(JNIEnv *env, jclass jobj, jlong handler, jstring jRtmpUrl)
{
    VideoChat* pVideoChat = (VideoChat*)handler;
    
    if (pVideoChat) {
        const char* rtmpUrl = env->GetStringUTFChars(jRtmpUrl, NULL);
        pVideoChat->Play(rtmpUrl);
        env->ReleaseStringUTFChars(jRtmpUrl, rtmpUrl);
    }
}

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_PausePlayer(JNIEnv *env, jclass jobj, jlong handler, jboolean paused)
{
    VideoChat* pVideoChat = (VideoChat*)handler;
    
    if (pVideoChat) {
        pVideoChat->PausePlayer(paused);
    }
}

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_ClosePlayer(JNIEnv *env, jclass jobj, jlong handler)
{
    VideoChat* pVideoChat = (VideoChat*)handler;
    
    if (pVideoChat) {
        pVideoChat->StopPlay();
    }
}


//
// Video Render Inferface
//


JNIEXPORT jlong JNICALL Java_cn_videochat_VideoChat_CreateRender(JNIEnv *env, jclass jobj)
{
    return (jlong)(new VideoRender());
}

JNIEXPORT jlong JNICALL Java_cn_videochat_VideoChat_SetRenderViewPort(JNIEnv *env, jclass jobj, jlong handler, jint jWidth, jint jHeight)
{
    VideoRender* pVideoRender = (VideoRender*)handler;
    
    if (pVideoRender) {
        pVideoRender->setViewport(jWidth,jHeight);
    }

    return 0;
}

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_RenderFrame(JNIEnv *env, jclass jobj, jlong handler)
{
    VideoRender* pVideoRender = (VideoRender*)handler;
    
    if (pVideoRender) {
        pVideoRender->renderFrame();
    }
}

JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_RenderRelease(JNIEnv *env, jclass jobj, jlong handler)
{
    VideoRender* pVideoRender = (VideoRender*)handler;
    
    if (pVideoRender) {
        delete pVideoRender;
        pVideoRender = NULL;
    }
}
