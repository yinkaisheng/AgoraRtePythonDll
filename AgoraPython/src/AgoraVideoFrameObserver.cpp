#include "AgoraVideoFrameObserver.h"
#include <spdlog/spdlog.h>

void AgoraVideoFrameObserver::saveCaptureVideoFrame(bool save, unsigned int maxFrames)
{
	mSaveCaptureVideoFrame = save;
	mSaveCaptureVideoFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetCaptureVideoFrame()
{
	mCaptureVideoFrameCount = 0;
	mSaveCaptureVideoFrameFileNo = 0;
}

void AgoraVideoFrameObserver::saveSecondaryCaptureVideoFrame(bool save, unsigned int maxFrames)
{
	mSaveSecondaryCaptureVideoFrame = save;
	mSaveSecondaryCaptureVideoFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetSecondaryCaptureVideoFrame()
{
	mSecondaryCaptureVideoFrameCount = 0;
	mSaveSecondaryCaptureVideoFrameFileNo = 0;
}

void AgoraVideoFrameObserver::saveRenderVideoFrame(bool save, unsigned int maxFrames)
{
	mSaveRenderVideoFrame = save;
	mSaveRenderVideoFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetRenderVideoFrame()
{
	mRenderVideoFrameCount.clear();
	mSaveRenderVideoFrameFileNo.clear();
}

void AgoraVideoFrameObserver::saveYuv(VideoFrame& videoFrame, const char* fileName, const char* mode)
{
	FILE* pFile = fopen(fileName, mode);

	if (pFile)
	{
		std::unique_ptr<char[]> yuvBuf;
		char* pNewYuvBuf = (char*)videoFrame.yBuffer;
		char* pNewUBuf = (char*)videoFrame.uBuffer;
		char* pNewVBuf = (char*)videoFrame.vBuffer;

		//copy per line
		if (videoFrame.yStride > videoFrame.width)
		{
			yuvBuf.reset(new(std::nothrow) char[videoFrame.width * videoFrame.height * 3 / 2]);
			pNewYuvBuf = yuvBuf.get();
			pNewUBuf = pNewYuvBuf + videoFrame.width * videoFrame.height;
			pNewVBuf = pNewUBuf + videoFrame.width * videoFrame.height / 4;

			char* yBuf1 = (char*)pNewYuvBuf;
			char* yBuf2 = (char*)videoFrame.yBuffer;

			for (int h = 0; h < videoFrame.height; ++h)
			{
				memcpy(yBuf1, yBuf2, videoFrame.width);
				yBuf1 += videoFrame.width;
				yBuf2 += videoFrame.yStride;
			}

			char* uBuf1 = (char*)pNewUBuf;
			char* uBuf2 = (char*)videoFrame.uBuffer;

			for (int h = 0; h < videoFrame.height / 2; ++h)
			{
				memcpy(uBuf1, uBuf2, videoFrame.width / 2);
				uBuf1 += videoFrame.width / 2;
				uBuf2 += videoFrame.uStride;
			}

			char* vBuf1 = (char*)pNewVBuf;
			char* vBuf2 = (char*)videoFrame.vBuffer;

			for (int h = 0; h < videoFrame.height / 2; ++h)
			{
				memcpy(vBuf1, vBuf2, videoFrame.width / 2);
				vBuf1 += videoFrame.width / 2;
				vBuf2 += videoFrame.vStride;
			}
		}

		fwrite(pNewYuvBuf, videoFrame.width * videoFrame.height, 1, pFile);
		fwrite(pNewUBuf, videoFrame.width * videoFrame.height / 4, 1, pFile);
		fwrite(pNewVBuf, videoFrame.width * videoFrame.height / 4, 1, pFile);
		fclose(pFile);
	}
}

bool AgoraVideoFrameObserver::onCaptureVideoFrame(VideoFrame& videoFrame)
{
	++mCaptureVideoFrameCount;
	if (mCaptureVideoFrameCount == 1)
	{
		mFirstCaptureVideoFrameTick = std::chrono::steady_clock::now();
		mFirstCaptureSeconds = 0;
		mCaptureVideoFrameCountLast = mCaptureVideoFrameCount;
		spdlog::info("CPP onCaptureVideoFrame {} * {} format {} frame count {}",
			videoFrame.width, videoFrame.height, videoFrame.type, mCaptureVideoFrameCount);
	}
	else
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - mFirstCaptureVideoFrameTick;
		double seconds = elapsed.count();
		if (seconds >= mFirstCaptureSeconds + 1.0)
		{
			mFirstCaptureSeconds = (int)seconds;
			unsigned fps = mCaptureVideoFrameCount - mCaptureVideoFrameCountLast;
			mCaptureVideoFrameCountLast = mCaptureVideoFrameCount;
			spdlog::info("CPP onCaptureVideoFrame {} * {} format {} frame count {} fps {}",
				videoFrame.width, videoFrame.height, videoFrame.type, mCaptureVideoFrameCount, fps);
		}
	}

	if (mSaveCaptureVideoFrame)
	{
		std::string fileMode{ "ab+" };
		if (mCaptureVideoFrameCount % mSaveCaptureVideoFrameCount == 1)
		{
			fileMode = "wb";
			mSaveCaptureVideoFrameFileNo = (mSaveCaptureVideoFrameFileNo + 1) % 2;
		}
		char szFile[128]{};
		std::snprintf(szFile, std::size(szFile), "onCaptureVideoFrame_%u_%dx%d_r%d.yuv",
			mSaveCaptureVideoFrameFileNo, videoFrame.width, videoFrame.height, videoFrame.rotation);
		saveYuv(videoFrame, szFile, fileMode.c_str());
	}
	return true;
}

bool AgoraVideoFrameObserver::onPreEncodeVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

bool AgoraVideoFrameObserver::onSecondaryCameraCaptureVideoFrame(VideoFrame& videoFrame)
{
	++mSecondaryCaptureVideoFrameCount;
	if (mSecondaryCaptureVideoFrameCount == 1)
	{
		mSecondaryCaptureVideoFrameTick = std::chrono::steady_clock::now();
		mSecondaryCaptureSeconds = 0;
		mSecondaryCaptureVideoFrameCountLast = mSecondaryCaptureVideoFrameCount;
		spdlog::info("CPP onSecondaryCameraCaptureVideoFrame {} * {} format {} frame count {}",
			videoFrame.width, videoFrame.height, videoFrame.type, mCaptureVideoFrameCount);
	}
	else
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - mSecondaryCaptureVideoFrameTick;
		double seconds = elapsed.count();
		if (seconds >= mSecondaryCaptureSeconds + 1.0)
		{
			mSecondaryCaptureSeconds = (int)seconds;
			unsigned fps = mSecondaryCaptureVideoFrameCount - mSecondaryCaptureVideoFrameCountLast;
			mSecondaryCaptureVideoFrameCountLast = mSecondaryCaptureVideoFrameCount;
			spdlog::info("CPP onSecondaryCameraCaptureVideoFrame {} * {} format {} frame count {} fps {}",
				videoFrame.width, videoFrame.height, videoFrame.type, mCaptureVideoFrameCount, fps);
		}
	}

	if (mSaveSecondaryCaptureVideoFrame)
	{
		std::string fileMode{ "ab+" };
		if (mSecondaryCaptureVideoFrameCount % mSaveSecondaryCaptureVideoFrameCount == 1)
		{
			fileMode = "wb";
			mSaveSecondaryCaptureVideoFrameFileNo = (mSaveSecondaryCaptureVideoFrameFileNo + 1) % 2;
		}
		char szFile[128]{};
		std::snprintf(szFile, std::size(szFile), "onSecondaryCaptureVideoFrame_%u_%dx%d_r%d.yuv",
			mSaveSecondaryCaptureVideoFrameFileNo, videoFrame.width, videoFrame.height, videoFrame.rotation);
		saveYuv(videoFrame, szFile, fileMode.c_str());
	}
	return true;
}

bool AgoraVideoFrameObserver::onSecondaryPreEncodeCameraVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

bool AgoraVideoFrameObserver::onScreenCaptureVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

bool AgoraVideoFrameObserver::onPreEncodeScreenVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

bool AgoraVideoFrameObserver::onMediaPlayerVideoFrame(VideoFrame& videoFrame, int mediaPlayerId)
{
	return true;
}

bool AgoraVideoFrameObserver::onSecondaryScreenCaptureVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

bool AgoraVideoFrameObserver::onSecondaryPreEncodeScreenVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

#if AGORA_SDK_VERSION>=36200000
bool AgoraVideoFrameObserver::onRenderVideoFrame(const char* channelId, uid_t uid, VideoFrame& videoFrame)
#else
bool AgoraVideoFrameObserver::onRenderVideoFrame(uid_t uid, conn_id_t connectionId, VideoFrame& videoFrame)
#endif
{
	std::string filePath;
	std::string fileMode{ "ab+" };
	unsigned long frameCount = 0;
	{
		std::lock_guard lock(mMutex);
		++mRenderVideoFrameCount[uid];
		frameCount = mRenderVideoFrameCount[uid];
		if (frameCount == 1)
		{
			mRenderVideoFrameTick[uid] = std::chrono::steady_clock::now();
			mRenderSeconds[uid] = 0;
			mRenderVideoFrameCountLast[uid] = frameCount;
			spdlog::info("CPP onRenderVideoFrame uid {}, {} * {} format {} frame count {}",
				uid, videoFrame.width, videoFrame.height, videoFrame.type, frameCount);
		}
		else
		{
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed = now - mRenderVideoFrameTick[uid];
			double seconds = elapsed.count();
			if (seconds >= mRenderSeconds[uid] + 1.0)
			{
				mRenderSeconds[uid] = (int)seconds;
				unsigned int renderFps = frameCount - mRenderVideoFrameCountLast[uid];
				mRenderVideoFrameCountLast[uid] = frameCount;
				spdlog::info("CPP onRenderVideoFrame uid {} {} * {} format {} frame count {}, fps {}",
					uid, videoFrame.width, videoFrame.height, videoFrame.type, frameCount, renderFps);
			}
		}

		if (mSaveRenderVideoFrame)
		{
			if (mRenderVideoFrameCount[uid] % mSaveRenderVideoFrameCount == 1)
			{
				fileMode = "wb";
				mSaveRenderVideoFrameFileNo[uid] = (mSaveRenderVideoFrameFileNo[uid] + 1) % 2;
			}
			char szFile[128]{};
			std::snprintf(szFile, std::size(szFile), "onRenderVideoFrame_%u_%u_%dx%d_r%d.yuv",
				uid, mSaveRenderVideoFrameFileNo[uid], videoFrame.width, videoFrame.height, videoFrame.rotation);
			filePath = szFile;
		}
	}

	if (!filePath.empty())
	{
		saveYuv(videoFrame, filePath.c_str(), fileMode.c_str());
	}

	return true;
}

bool AgoraVideoFrameObserver::onTranscodedVideoFrame(VideoFrame& videoFrame)
{
	return true;
}

IVideoFrameObserver::VIDEO_FRAME_PROCESS_MODE AgoraVideoFrameObserver::getVideoFrameProcessMode()
{
	return PROCESS_MODE_READ_ONLY;
}
