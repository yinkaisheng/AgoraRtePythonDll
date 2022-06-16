#include "AgoraEventHandler.h"
#include <string>
#include <map>
#include <spdlog/spdlog.h>
#include <json.hpp>
#include "util.h"

#define STR(str) (str?str:"")

#define CALLBACK_BLOCK_BEGIN     \
    if (mCallback)\
    {\
        std::string thisFuncName{ __FUNCTION__ };\
        thisFuncName = thisFuncName.substr(thisFuncName.rfind(':') + 1);

#define CALLBACK_BLOCK_END      \
        mCallback(this, EpochMicroseconds(), thisFuncName.c_str(), jsonStr.c_str());\
    }

using namespace nlohmann;


void AgoraEventHandler::setEventCallback(AgoraEngineEventCallback callback)
{
    mCallback = callback;
}

void AgoraEventHandler::onError(int err, const char* msg)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["err"] = err;
	js["msg"] = msg;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onWarning(int warn, const char* msg)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["warn"] = warn;
	js["msg"] = msg;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onCameraReady()
{
    CALLBACK_BLOCK_BEGIN

    //json js;
    //std::string jsonStr = js.dump(4);
    std::string jsonStr;

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onJoinChannelSuccess(const char* channel, uid_t uid, int elapsed)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["channel"] = channel;
    js["uid"] = uid;
    js["elapsed"] = elapsed;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onUserJoined(uid_t uid, int elapsed)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["elapsed"] = elapsed;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onUserOffline(uid_t uid, USER_OFFLINE_REASON_TYPE reason)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["reason"] = reason;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state, LOCAL_AUDIO_STREAM_ERROR error)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["state"] = state;
	js["error"] = error;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state, LOCAL_VIDEO_STREAM_ERROR error)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["state"] = state;
	js["error"] = error;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onRemoteAudioStateChanged(uid_t uid, REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason, int elapsed)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["uid"] = uid;
	js["state"] = state;
	js["reason"] = reason;
	js["elapsed"] = elapsed;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onRemoteVideoStateChanged(uid_t uid, REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason, int elapsed)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["uid"] = uid;
	js["state"] = state;
	js["reason"] = reason;
	js["elapsed"] = elapsed;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onFirstLocalVideoFrame(int width, int height, int elapsed)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["width"] = width;
    js["height"] = height;
    js["elapsed"] = elapsed;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onFirstLocalVideoFramePublished(int elapsed)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["elapsed"] = elapsed;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["width"] = width;
    js["height"] = height;
    js["elapsed"] = elapsed;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

/*
void AgoraEventHandler::onFirstRemoteVideoFrame(uid_t uid, int width, int height, int elapsed)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["width"] = width;
    js["height"] = height;
    js["elapsed"] = elapsed;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}
*/

void AgoraEventHandler::onVideoSizeChanged(uid_t uid, int width, int height, int rotation)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["width"] = width;
    js["height"] = height;
    js["rotation"] = rotation;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onVideoStopped()
{
    CALLBACK_BLOCK_BEGIN

    std::string jsonStr = "{}";

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["deviceId"] = deviceId;
	js["deviceType"] = deviceType;
	js["deviceState"] = deviceState;
	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onVideoDeviceStateChanged(const char* deviceId, int deviceType, int deviceState)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["deviceId"] = deviceId;
	js["deviceType"] = deviceType;
    js["deviceState"] = deviceState;
	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}


#if AGORA_SDK_VERSION>=36200000
void AgoraEventHandler::onStreamMessage(uid_t uid, int streamId, const char* data, size_t length, uint64_t sentTs)
#else
void AgoraEventHandler::onStreamMessage(uid_t uid, int streamId, const char* data, size_t length)
#endif
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["streamId"] = streamId;
    js["data"] = std::string(data, length);
#if AGORA_SDK_VERSION==36200000
    js["sentTs"] = sentTs;
#endif
    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

void AgoraEventHandler::onStreamMessageError(uid_t uid, int streamId, int code, int missed, int cached)
{
    CALLBACK_BLOCK_BEGIN

    json js;
    js["uid"] = uid;
    js["streamId"] = streamId;
    js["code"] = code;
    js["missed"] = missed;
    js["cached"] = cached;

    std::string jsonStr = js.dump(4);

    CALLBACK_BLOCK_END
}

#if HAS_AUTO_CORRECT
#if AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109
void AgoraEventHandler::onTrapezoidAutoCorrectionFinished(uid_t uid, TrapezoidAutoCorrectionResult result, int costTime,
    const TrapezoidCorrectionOptions::Point* dragSrcPoints, int dragSrcPointsLen,
    const TrapezoidCorrectionOptions::Point* dragDstPoints, int dragDstPointsLen)
#else
void AgoraEventHandler::onTrapezoidAutoCorrectionFinished(uid_t uid, TRAPEZOID_AUTO_CORRECTION_RESULT result, int costTime,
	const TrapezoidCorrectionOptions::Point* dragSrcPoints, int dragSrcPointsLen,
	const TrapezoidCorrectionOptions::Point* dragDstPoints, int dragDstPointsLen)
#endif
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["uid"] = uid;
	js["result"] = result;
	js["costTime"] = costTime;
    json jsDragSrcPoints;
    for (int i = 0; i < dragSrcPointsLen; ++i)
    {
        jsDragSrcPoints.push_back(dragSrcPoints[i].x);
		jsDragSrcPoints.push_back(dragSrcPoints[i].y);
    }
    json jsDragDstPoints;
	for (int i = 0; i < dragDstPointsLen; ++i)
	{
        jsDragDstPoints.push_back(dragDstPoints[i].x);
        jsDragDstPoints.push_back(dragDstPoints[i].y);
	}
	js["dragSrcPoints"] = jsDragSrcPoints;
	js["dragDstPoints"] = jsDragDstPoints;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

#if AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109
void AgoraEventHandler::onTrapezoidAutoCorrectionFinished(const RtcConnection& connection, uid_t uid, TrapezoidAutoCorrectionResult result, int costTime,
	const TrapezoidCorrectionOptions::Point* dragSrcPoints, int dragSrcPointsLen,
	const TrapezoidCorrectionOptions::Point* dragDstPoints, int dragDstPointsLen)
#else
void AgoraEventHandler::onTrapezoidAutoCorrectionFinished(const RtcConnection& connection, uid_t uid, TRAPEZOID_AUTO_CORRECTION_RESULT result, int costTime,
	const TrapezoidCorrectionOptions::Point* dragSrcPoints, int dragSrcPointsLen,
	const TrapezoidCorrectionOptions::Point* dragDstPoints, int dragDstPointsLen)
#endif
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["channel"] = connection.channelId;
	js["localUid"] = connection.localUid;
	js["uid"] = uid;
	js["result"] = result;
	js["costTime"] = costTime;
	json jsDragSrcPoints;
	for (int i = 0; i < dragSrcPointsLen; ++i)
	{
		jsDragSrcPoints.push_back(dragSrcPoints[i].x);
		jsDragSrcPoints.push_back(dragSrcPoints[i].y);
	}
	json jsDragDstPoints;
	for (int i = 0; i < dragDstPointsLen; ++i)
	{
		jsDragDstPoints.push_back(dragDstPoints[i].x);
		jsDragDstPoints.push_back(dragDstPoints[i].y);
	}
	js["dragSrcPoints"] = jsDragSrcPoints;
	js["dragDstPoints"] = jsDragDstPoints;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}
#endif

#if (AGORA_SDK_VERSION >= 36200104 && AGORA_SDK_VERSION <= 36200109) || AGORA_SDK_VERSION>=38200000
void AgoraEventHandler::onSnapshotTaken(const char* channel, uid_t uid, const char* filePath, int width, int height, int errCode)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["channel"] = channel;
	js["uid"] = uid;
	js["filePath"] = filePath;
	js["width"] = width;
	js["height"] = height;
	js["errCode"] = errCode;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onSnapshotTaken(uid_t uid, const char* filePath, int width, int height, int errCode)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["uid"] = uid;
	js["filePath"] = filePath;
	js["width"] = width;
    js["height"] = height;
    js["errCode"] = errCode;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}

void AgoraEventHandler::onSnapshotTaken(const RtcConnection& connection, uid_t uid, const char* filePath, int width, int height, int errCode)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["channel"] = connection.channelId;
    js["localUid"] = connection.localUid;
	js["uid"] = uid;
	js["filePath"] = filePath;
	js["width"] = width;
	js["height"] = height;
	js["errCode"] = errCode;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}
#endif

#if AGORA_SDK_VERSION>=38200000
void AgoraEventHandler::onContentInspectResult(agora::media::CONTENT_INSPECT_RESULT result)
{
	CALLBACK_BLOCK_BEGIN

	json js;
	js["result"] = result;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}
#endif

#if (AGORA_SDK_VERSION >= 36200104 && AGORA_SDK_VERSION <= 36200109) && IS_DEV_36200104
void AgoraEventHandler::onServerSuperResolutionResult(int httpStatusCode, int errCode, const char* errReason,
	const char* imageData, int imageSize, int width, int height)
{
    CALLBACK_BLOCK_BEGIN

    std::string imagePath;
    if (imageData && imageSize > 0)
    {
        imagePath = "server-sr-image.jpg";
        FILE* pFile = fopen(imagePath.c_str(), "wb");
        if (pFile)
        {
            fwrite(imageData, imageSize, 1, pFile);
            fclose(pFile);
            pFile = nullptr;
        }
    }

	json js;
	js["httpStatusCode"] = httpStatusCode;
	js["errCode"] = errCode;
	js["errReason"] = errReason;
	js["imagePath"] = imagePath;
	js["width"] = width;
	js["height"] = height;

	std::string jsonStr = js.dump(4);

	CALLBACK_BLOCK_END
}
#endif

