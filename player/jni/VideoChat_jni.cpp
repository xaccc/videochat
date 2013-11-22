/**
 *
 * Copyright (C) 2013 ALiang (jjyyis@qq.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
 
#include "VideoChat.h"


/*
 * Class:     cn_videochat_VideoChat
 * Method:    Init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_Init(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_Init");
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
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    ClosePlayer
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_cloudstep_sayhi_SayHi_ClosePlayer(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_ClosePublisher");
}

/*
 * Class:     cn_videochat_VideoChat
 * Method:    Release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_videochat_VideoChat_Release(JNIEnv *env, jobject jobj)
{
    LOGI("Java_cn_videochat_VideoChat_Release");
}
