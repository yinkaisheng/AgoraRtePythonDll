#pragma once
#include <IAgoraMediaEngine.h>
#include <string_view>
#include <chrono>
#include <map>
#include <mutex>
using namespace agora::media;
using namespace agora::rtc;

struct FrameStats
{
	unsigned int frameCount{};
	unsigned int frameCountLast{};
	std::chrono::steady_clock::time_point firstFrameTick{};
	unsigned int elapsedSeconds{};
	bool saveFrame{};
	unsigned saveFrameFileNo{};
	unsigned saveFrameCount{ 100 };

	void reset()
	{
		frameCount = 0;
		frameCountLast = 0;
		//firstFrameTick ;
		elapsedSeconds = 0;
		//saveFrame = false;
		saveFrameFileNo = 0;
		saveFrameCount = 100;
	}
};

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
	void saveScreenVideoFrame(bool save, unsigned int maxFrames);
	void resetScreenVideoFrame();
	void saveSecondaryScreenVideoFrame(bool save, unsigned int maxFrames);
	void resetSecondaryScreenVideoFrame();
	void saveRenderVideoFrame(bool save, unsigned int maxFrames);
	void resetRenderVideoFrame();
	void resetAllStats();

private:
	void saveYuv(VideoFrame& videoFrame, const char* fileName, const char* mode = "ab+");

	FrameStats mCapture1;
	FrameStats mCapture2;
	FrameStats mScreen1;
	FrameStats mScreen2;

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
