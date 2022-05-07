﻿#pragma once
#include <IAgoraRtcEngine.h>
#include <list>

#define  HAS_AUTO_CORRECT 1

using namespace agora::rtc;

class AgoraEventHandler;

typedef void (* AgoraEngineEventCallback)(void* pThis, long long callbackTimeSinceEpoch, const char* funcName, const char* json);

class agora::rtc::IRtcEngine;


#if (AGORA_SDK_VERSION==36200100 && IS_DEV_36200100) || AGORA_SDK_VERSION>=50000000
class SnapshotHandler
	: public agora::media::ISnapshotCallback
{
public:
	void setEventCallback(AgoraEngineEventCallback callback);
private:
	void onSnapshotTaken(const char* channel, uid_t uid, const char* filePath, int width, int height, int errCode) override;

private:
	AgoraEngineEventCallback mCallback{};

};
#endif


class AgoraEventHandler : public IRtcEngineEventHandler
{
public:
    void setEventCallback(AgoraEngineEventCallback callback);

private:

	void onError(int err, const char* msg) override;
	void onWarning(int warn, const char* msg) override;
	void onCameraReady() override;
	void onJoinChannelSuccess(const char* channel, uid_t uid, int elapsed) override;
	void onUserJoined(uid_t uid, int elapsed) override;
	void onUserOffline(uid_t uid, USER_OFFLINE_REASON_TYPE reason) override;
	void onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state, LOCAL_AUDIO_STREAM_ERROR error) override;
	void onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state, LOCAL_VIDEO_STREAM_ERROR error) override;
	void onFirstLocalVideoFrame(int width, int height, int elapsed) override;
	void onFirstLocalVideoFramePublished(int elapsed) override;
	void onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed) override;
    //void onFirstRemoteVideoFrame(uid_t uid, int width, int height, int elapsed) override;//not suggested
	void onRemoteAudioStateChanged(uid_t uid, REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason, int elapsed) override;
	void onRemoteVideoStateChanged(uid_t uid, REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason, int elapsed) override;
	void onVideoSizeChanged(uid_t uid, int width, int height, int rotation) override;
	void onVideoStopped() override;
	void onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) override;
	void onVideoDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) override;
#if AGORA_SDK_VERSION>=36200000
    void onStreamMessage(uid_t userId, int streamId, const char* data, size_t length, uint64_t sentTs) override;
#else
    void onStreamMessage(uid_t uid, int streamId, const char* data, size_t length) override;
#endif
    void onStreamMessageError(uid_t uid, int streamId, int code, int missed, int cached) override;

#if AGORA_SDK_VERSION == 36200100 && HAS_AUTO_CORRECT
	void onTrapezoidAutoCorrectionFinished(uid_t uid, TrapezoidAutoCorrectionResult result, int costTime,
		const TrapezoidCorrectionOptions::Point* dragSrcPoints, int dragSrcPointsLen,
		const TrapezoidCorrectionOptions::Point* dragDstPoints, int dragDstPointsLen);
#endif

#if (AGORA_SDK_VERSION==36200100 && IS_DEV_36200100) || AGORA_SDK_VERSION>=50000000
	void onSnapshotTaken(const char* channel, uid_t uid, const char* filePath, int width, int height, int errCode) override;
#endif

#if AGORA_SDK_VERSION>=50000000
	void onContentInspectResult(agora::media::CONTENT_INSPECT_RESULT result) override;
#endif

#if AGORA_SDK_VERSION==36200100 && IS_DEV_36200100
	void onServerSuperResolutionResult(int httpStatusCode, const char* imageData, int imageSize, int width, int height, const char* reason) override;
#endif

private:
	AgoraEngineEventCallback mCallback{};

};
