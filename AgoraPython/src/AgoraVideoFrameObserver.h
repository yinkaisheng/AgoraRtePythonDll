#pragma once
#include <IAgoraMediaEngine.h>
#include <string_view>
#include <chrono>
#include <map>
#include <mutex>
using namespace agora::media;
using namespace agora::rtc;

class AgoraVideoFrameObserver : public agora::media::IVideoFrameObserver
{
public:

	virtual bool onCaptureVideoFrame(VideoFrame& videoFrame) override;

	virtual bool onPreEncodeVideoFrame(VideoFrame& videoFrame);

	virtual bool onSecondaryCameraCaptureVideoFrame(VideoFrame& videoFrame) override;

	virtual bool onSecondaryPreEncodeCameraVideoFrame(VideoFrame& videoFrame);

	virtual bool onScreenCaptureVideoFrame(VideoFrame& videoFrame) override;

	virtual bool onPreEncodeScreenVideoFrame(VideoFrame& videoFrame);

	virtual bool onMediaPlayerVideoFrame(VideoFrame& videoFrame, int mediaPlayerId) override;

	virtual bool onSecondaryScreenCaptureVideoFrame(VideoFrame& videoFrame) override;

	virtual bool onSecondaryPreEncodeScreenVideoFrame(VideoFrame& videoFrame);

#if AGORA_SDK_VERSION>=36200000
	virtual bool onRenderVideoFrame(const char* channelId, uid_t remoteUid, VideoFrame& videoFrame) override;
#else
	virtual bool onRenderVideoFrame(uid_t uid, conn_id_t connectionId, VideoFrame& videoFrame) override;
#endif

	virtual bool onTranscodedVideoFrame(VideoFrame& videoFrame) override;

	virtual VIDEO_FRAME_PROCESS_MODE getVideoFrameProcessMode() override;

	void saveCaptureVideoFrame(bool save, unsigned int maxFrames);
	void resetCaptureVideoFrame();
	void saveSecondaryCaptureVideoFrame(bool save, unsigned int maxFrames);
	void resetSecondaryCaptureVideoFrame();
	void saveRenderVideoFrame(bool save, unsigned int maxFrames);
	void resetRenderVideoFrame();

private:
	void saveYuv(VideoFrame& videoFrame, const char* fileName, const char* mode = "ab+");

	unsigned int mCaptureVideoFrameCount{};
	unsigned int mCaptureVideoFrameCountLast{};
	std::chrono::steady_clock::time_point mFirstCaptureVideoFrameTick{};
	unsigned int mFirstCaptureSeconds{};
	bool mSaveCaptureVideoFrame{};
	unsigned mSaveCaptureVideoFrameFileNo{};
	unsigned mSaveCaptureVideoFrameCount{ 100 };

	unsigned int mSecondaryCaptureVideoFrameCount{};
	unsigned int mSecondaryCaptureVideoFrameCountLast{};
	std::chrono::steady_clock::time_point mSecondaryCaptureVideoFrameTick{};
	unsigned int mSecondaryCaptureSeconds{};
	bool mSaveSecondaryCaptureVideoFrame{};
	unsigned mSaveSecondaryCaptureVideoFrameFileNo{};
	unsigned mSaveSecondaryCaptureVideoFrameCount{ 100 };

	//unsigned long mRenderVideoFrameCount{};
	//std::chrono::steady_clock::time_point mFirstCaptureVideoFrameTick{};
	//unsigned int mFirstCaptureSeconds{};
	bool mSaveRenderVideoFrame{};
	//unsigned mSaveRenderVideoFrameFileNo{};
	unsigned mSaveRenderVideoFrameCount{ 100 };

	std::map<uid_t, unsigned int> mRenderVideoFrameCount;
	std::map<uid_t, unsigned int> mRenderVideoFrameCountLast;
	std::map<uid_t, std::chrono::steady_clock::time_point> mRenderVideoFrameTick;
	std::map<uid_t, unsigned int> mRenderSeconds;
	std::map<uid_t, unsigned> mSaveRenderVideoFrameFileNo;
	//std::map<uid_t, unsigned> mSaveRenderVideoFrameCount;
	std::mutex mMutex;
};
