# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_ARM_MODE  := arm

#LOCAL_ARM_NEON  := true
#LOCAL_CFLAGS    := -DHAVE_NEON=1

#LOCAL_CFLAGS    := -DUSEPOSTPROC
#LOCAL_LDFLAGS    +=  -lpostproc

LOCAL_MODULE    :=  vc
LOCAL_SRC_FILES :=  VideoChat.cpp \
                    VideoChat_jni.cpp \
                    AudioDecoder.cpp \
                    AudioRender.cpp \
                    VideoDecoder.cpp \
                    VideoRender.cpp \
                    json.c


LOCAL_LDFLAGS	:= $(LOCAL_LDFLAGS) -L$(LOCAL_PATH)/$(TARGET_ARCH_ABI)/lib 
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/$(TARGET_ARCH_ABI)/include

APP_CPPFLAGS := -S -frtti -fexceptions


# for native audio
LOCAL_LDFLAGS    += -lOpenSLES
LOCAL_LDFLAGS	+= -lGLESv2
# for logging
LOCAL_LDFLAGS    += -llog
# for native asset manager
LOCAL_LDFLAGS    += -landroid
# for Speex
LOCAL_LDFLAGS    += -lspeex -lspeexdsp
# for rtmpdump
LOCAL_LDFLAGS    += -lrtmp
# for x264
#LOCAL_LDLIBS    += -lx264

# for ffmpeg
LOCAL_CFLAGS     += -DUSEFFMPEG
LOCAL_LDFLAGS    += -lavcodec -lavutil
#LOCAL_SRC_FILES :=  h264dec/cabac.cpp \
#                    h264dec/common.cpp \
#                    h264dec/dsputil.cpp \
#                    h264dec/golomb.cpp \
#                    h264dec/h264.cpp \
#                    h264dec/h264utils.cpp \
#                    h264dec/mpegvideo.cpp \
#                    h264dec/H264Android.cpp

include $(BUILD_SHARED_LIBRARY)

