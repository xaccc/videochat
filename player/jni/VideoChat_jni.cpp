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


jobject g_jObject = NULL;
VideoChat g_videochat;

/*
 * Class:     cn_videochat_VideoChat
 * Method:    Init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_Init(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_Init");
    g_jObject = env->NewGlobalRef(jobj);
    g_videochat.Init();
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    OpenPublisher
 * Signature: (Ljava/lang/String{})V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_OpenPublisher(JNIEnv *env, jobject jobj, jstring jRtmpUrl)
{
    LOGI("Java_cn_videochat_VideoChat_OpenPublisher");
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    ClosePublisher
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_ClosePublisher(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_ClosePublisher");
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    OpenPlayer
 * Signature: (Ljava/lang/String{})V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_OpenPlayer(JNIEnv *env, jobject jobj, jstring jRtmpUrl)
{
    LOGI("Java_cn_videochat_VideoChat_OpenPlayer");
    const char* rtmpUrl = env->GetStringUTFChars(jRtmpUrl, NULL);
    g_videochat.Play(rtmpUrl);
    env->ReleaseStringUTFChars(jRtmpUrl, rtmpUrl);
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    ClosePlayer
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_ClosePlayer(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_ClosePlayer");
    g_videochat.StopPlay();
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    Release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_Release(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_Release");
    env->DeleteGlobalRef(g_jObject);
    g_videochat.Release();
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    InitRender
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_InitRender(JNIEnv *, jobject, jint jWidth, jint jHeight)
{
    LOGI("Java_cn_videochat_VideoChat_InitRender");
    g_videochat.InitRender(jWidth,jHeight);
}


/*
 * Class:     cn_videochat_VideoChat
 * Method:    RenderFrame
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_RenderFrame(JNIEnv *, jobject)
{
    LOGI("Java_cn_videochat_VideoChat_RenderFrame");
    g_videochat.RenderFrame();
}


/*
 * Class:     cn_videochat_VideoChat
 * Method:    PausePlayer
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_PausePlayer
  (JNIEnv *, jobject, jboolean paused)
{
    g_videochat.PausePlayer(paused);
}




/*
 * Class:     cn_videochat_VideoChat
 * Method:    PausePublisher
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_PausePublisher
  (JNIEnv *, jobject, jboolean)
{
}