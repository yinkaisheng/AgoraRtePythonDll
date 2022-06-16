#include "AgoraVideoFrameObserver.h"
#include <spdlog/spdlog.h>

void AgoraVideoFrameObserver::saveCaptureVideoFrame(bool save, unsigned int maxFrames)
{
	mCapture1.saveFrame = save;
	mCapture1.saveFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetCaptureVideoFrame()
{
	mCapture1.frameCount = 0;
	mCapture1.saveFrameFileNo = 0;
}

void AgoraVideoFrameObserver::saveSecondaryCaptureVideoFrame(bool save, unsigned int maxFrames)
{
	mCapture2.saveFrame = save;
	mCapture2.saveFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetSecondaryCaptureVideoFrame()
{
	mCapture2.frameCount = 0;
	mCapture2.saveFrameFileNo = 0;
}

void AgoraVideoFrameObserver::saveScreenVideoFrame(bool save, unsigned int maxFrames)
{
	mScreen1.saveFrame = save;
	mScreen1.saveFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetScreenVideoFrame()
{
	mScreen1.frameCount = 0;
	mScreen1.saveFrameFileNo = 0;
}

void AgoraVideoFrameObserver::saveSecondaryScreenVideoFrame(bool save, unsigned int maxFrames)
{
	mScreen2.saveFrame = save;
	mScreen2.saveFrameCount = maxFrames;
}

void AgoraVideoFrameObserver::resetSecondaryScreenVideoFrame()
{
	mScreen2.frameCount = 0;
	mScreen2.saveFrameFileNo = 0;
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

void AgoraVideoFrameObserver::resetAllStats()
{
	mCapture1.reset();
	mCapture2.reset();
	mScreen1.reset();
	mScreen2.reset();
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
	++mCapture1.frameCount;
	if (mCapture1.frameCount == 1)
	{
		mCapture1.firstFrameTick = std::chrono::steady_clock::now();
		mCapture1.elapsedSeconds = 0;
		mCapture1.frameCountLast = mCapture1.frameCount;
		spdlog::info("CPP onCaptureVideoFrame {} * {} format {} frame count {}",
			videoFrame.width, videoFrame.height, videoFrame.type, mCapture1.frameCount);
	}
	else
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - mCapture1.firstFrameTick;
		double seconds = elapsed.count();
		if (seconds >= mCapture1.elapsedSeconds + 1.0)
		{
			mCapture1.elapsedSeconds = (int)seconds;
			unsigned fps = mCapture1.frameCount - mCapture1.frameCountLast;
			mCapture1.frameCountLast = mCapture1.frameCount;
			spdlog::info("CPP onCaptureVideoFrame {} * {} format {} frame count {} fps {}",
				videoFrame.width, videoFrame.height, videoFrame.type, mCapture1.frameCount, fps);
		}
	}

	if (mCapture1.saveFrame)
	{
		std::string fileMode{ "ab+" };
		if (mCapture1.frameCount % mCapture1.saveFrameCount == 1)
		{
			fileMode = "wb";
			mCapture1.saveFrameFileNo = (mCapture1.saveFrameFileNo + 1) % 2;
		}
		char szFile[128]{};
		std::snprintf(szFile, std::size(szFile), "onCaptureVideoFrame_%u_%dx%d_r%d.yuv",
			mCapture1.saveFrameFileNo, videoFrame.width, videoFrame.height, videoFrame.rotation);
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
	++mCapture2.frameCount;
	if (mCapture2.frameCount == 1)
	{
		mCapture2.firstFrameTick = std::chrono::steady_clock::now();
		mCapture2.elapsedSeconds = 0;
		mCapture2.frameCountLast = mCapture2.frameCount;
		spdlog::info("CPP onSecondaryCameraCaptureVideoFrame {} * {} format {} frame count {}",
			videoFrame.width, videoFrame.height, videoFrame.type, mCapture2.frameCount);
	}
	else
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - mCapture2.firstFrameTick;
		double seconds = elapsed.count();
		if (seconds >= mCapture2.elapsedSeconds + 1.0)
		{
			mCapture2.elapsedSeconds = (int)seconds;
			unsigned fps = mCapture2.frameCount - mCapture2.frameCountLast;
			mCapture2.frameCountLast = mCapture2.frameCount;
			spdlog::info("CPP onSecondaryCameraCaptureVideoFrame {} * {} format {} frame count {} fps {}",
				videoFrame.width, videoFrame.height, videoFrame.type, mCapture2.frameCount, fps);
		}
	}

	if (mCapture2.saveFrame)
	{
		std::string fileMode{ "ab+" };
		if (mCapture2.frameCount % mCapture2.saveFrameCount == 1)
		{
			fileMode = "wb";
			mCapture2.saveFrameFileNo = (mCapture2.saveFrameFileNo + 1) % 2;
		}
		char szFile[128]{};
		std::snprintf(szFile, std::size(szFile), "onSecondaryCameraCaptureVideoFrame_%u_%dx%d_r%d.yuv",
			mCapture2.saveFrameFileNo, videoFrame.width, videoFrame.height, videoFrame.rotation);
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
	++mScreen1.frameCount;
	if (mScreen1.frameCount == 1)
	{
		mScreen1.firstFrameTick = std::chrono::steady_clock::now();
		mScreen1.elapsedSeconds = 0;
		mScreen1.frameCountLast = mScreen1.frameCount;
		spdlog::info("CPP onScreenCaptureVideoFrame {} * {} format {} frame count {}",
			videoFrame.width, videoFrame.height, videoFrame.type, mScreen1.frameCount);
	}
	else
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - mScreen1.firstFrameTick;
		double seconds = elapsed.count();
		if (seconds >= mScreen1.elapsedSeconds + 1.0)
		{
			mScreen1.elapsedSeconds = (int)seconds;
			unsigned fps = mScreen1.frameCount - mScreen1.frameCountLast;
			mScreen1.frameCountLast = mScreen1.frameCount;
			spdlog::info("CPP onScreenCaptureVideoFrame {} * {} format {} frame count {} fps {}",
				videoFrame.width, videoFrame.height, videoFrame.type, mScreen1.frameCount, fps);
		}
	}

	if (mScreen1.saveFrame)
	{
		std::string fileMode{ "ab+" };
		if (mScreen1.frameCount % mScreen1.saveFrameCount == 1)
		{
			fileMode = "wb";
			mScreen1.saveFrameFileNo = (mScreen1.saveFrameFileNo + 1) % 2;
		}
		char szFile[128]{};
		std::snprintf(szFile, std::size(szFile), "onScreenCaptureVideoFrame_%u_%dx%d_r%d.yuv",
			mScreen1.saveFrameFileNo, videoFrame.width, videoFrame.height, videoFrame.rotation);
		saveYuv(videoFrame, szFile, fileMode.c_str());
	}
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
	++mScreen2.frameCount;
	if (mScreen2.frameCount == 1)
	{
		mScreen2.firstFrameTick = std::chrono::steady_clock::now();
		mScreen2.elapsedSeconds = 0;
		mScreen2.frameCountLast = mScreen2.frameCount;
		spdlog::info("CPP onScreenCaptureVideoFrame {} * {} format {} frame count {}",
			videoFrame.width, videoFrame.height, videoFrame.type, mScreen2.frameCount);
	}
	else
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - mScreen2.firstFrameTick;
		double seconds = elapsed.count();
		if (seconds >= mScreen2.elapsedSeconds + 1.0)
		{
			mScreen2.elapsedSeconds = (int)seconds;
			unsigned fps = mScreen2.frameCount - mScreen2.frameCountLast;
			mScreen2.frameCountLast = mScreen2.frameCount;
			spdlog::info("CPP onScreenCaptureVideoFrame {} * {} format {} frame count {} fps {}",
				videoFrame.width, videoFrame.height, videoFrame.type, mScreen2.frameCount, fps);
		}
	}

	if (mScreen2.saveFrame)
	{
		std::string fileMode{ "ab+" };
		if (mScreen2.frameCount % mScreen2.saveFrameCount == 1)
		{
			fileMode = "wb";
			mScreen2.saveFrameFileNo = (mScreen2.saveFrameFileNo + 1) % 2;
		}
		char szFile[128]{};
		std::snprintf(szFile, std::size(szFile), "onScreenCaptureVideoFrame_%u_%dx%d_r%d.yuv",
			mScreen2.saveFrameFileNo, videoFrame.width, videoFrame.height, videoFrame.rotation);
		saveYuv(videoFrame, szFile, fileMode.c_str());
	}
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
