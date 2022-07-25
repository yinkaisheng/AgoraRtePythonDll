//#include <windows.h>
//#include <gdiplus.h>    //do not define WIN32_LEAN_AND_MEAN
//using namespace Gdiplus;
//#pragma comment(lib, "gdiplus.lib")

#include <IAgoraRtcEngineEx.h>
#include <IAgoraMediaEngine.h>
#include <IAgoraParameter.h>
#include "AgoraEventHandler.h"
#include "AgoraVideoFrameObserver.h"
#include "util.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>
#include <json.hpp>


using namespace nlohmann;
using namespace agora;
using namespace agora::base;
using namespace agora::commons;
using namespace agora::rtc;
using namespace agora::media;
using namespace agora::media::base;

#define AGORA_CAPI extern "C" __declspec(dllexport)

static IAgoraParameter* sAgoraParameter{};
static IAudioDeviceManager* sAudioDeviceManager{};
static IVideoDeviceManager* sVideoDeviceManager{};
static IMediaEngine* sMediaEngine{};
static AgoraVideoFrameObserver sVideoFrameObserver;

class AgoraAudioFrameObserver : public agora::media::IAudioFrameObserver
{
public:
	virtual bool onRecordAudioFrame(AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onRecordAudioFrame(const char* channelId, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onPlaybackAudioFrame(AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onPlaybackAudioFrame(const char* channelId, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onMixedAudioFrame(AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onMixedAudioFrame(const char* channelId, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onEarMonitoringAudioFrame(AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, sCount {}", __FUNCTION__, sCount);
		}
		return true;
	}

	virtual bool onPlaybackAudioFrameBeforeMixing(agora::media:: base::user_id_t userId, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, userId {}, sCount {}", __FUNCTION__, userId, sCount);
		}
		return true;
	}

	virtual bool onPlaybackAudioFrameBeforeMixing(const char* channelId, agora::media::base::user_id_t userId, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, c userId {}, sCount {}", __FUNCTION__, userId, sCount);
		}
		return true;
	}

	virtual bool onPlaybackAudioFrameBeforeMixing(rtc::uid_t uid, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, uid {}, sCount {}", __FUNCTION__, uid, sCount);
		}
		return true;
	}

	virtual bool onPlaybackAudioFrameBeforeMixing(const char* channelId, rtc::uid_t uid, AudioFrame& audioFrame)
	{
		static unsigned int sCount = 0;
		++sCount;
		if (sCount % mLogFrameInterval == 1)
		{
			spdlog::info("CPP {}, c uid {}, sCount {}", __FUNCTION__, uid, sCount);
		}
		return true;
	}

#if AGORA_SDK_VERSION >= 50000000
	virtual int getObservedAudioFramePosition()
	{
		return agora::media::IAudioFrameObserverBase::AUDIO_FRAME_POSITION::AUDIO_FRAME_POSITION_BEFORE_MIXING;
	}

	virtual AudioParams getPlaybackAudioParams()
	{
		static AudioParams params;
		params.sample_rate = 48000;
		params.channels = 2;
		params.samples_per_call = 480;
		return params;
	}

	virtual AudioParams getRecordAudioParams()
	{
		static AudioParams params;
		params.sample_rate = 48000;
		params.channels = 2;
		params.samples_per_call = 480;
		return params;
	}

	virtual AudioParams getMixedAudioParams()
	{
		static AudioParams params;
		params.sample_rate = 48000;
		params.channels = 2;
		params.samples_per_call = 480;
		return params;
	}

	virtual AudioParams getEarMonitoringAudioParams()
	{
		static AudioParams params;
		params.sample_rate = 48000;
		params.channels = 2;
		params.samples_per_call = 480;
		return params;
	}
#endif

private:
	unsigned int mLogFrameInterval{ 100 };
};

static AgoraAudioFrameObserver sAudioFrameObserver;


/*
static ULONG_PTR g_nGdiPlusToken = 0;
AGORA_CAPI void initializeGdiPlus()
{
	if (!g_nGdiPlusToken)
	{
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&g_nGdiPlusToken, &gdiplusStartupInput, nullptr);
	}
}

//format = L"image/png" or jpg, bmp
int GetImageEncoderClsid(const wchar_t* format, CLSID* pClsid)
{
	UINT num = 0;          // number of image encoders
	UINT size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = nullptr;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == nullptr)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		//wprintf(L"%s\n", pImageCodecInfo[j].MimeType);
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void SaveBitmap(Bitmap& bmp, const wchar_t* path)
{
    CLSID clsid;
    if (GetImageEncoderClsid(L"image/png", &clsid) >= 0)
    {
        bmp.Save(path, &clsid, nullptr);
    }
}

void drawBitmap(Bitmap& bmp)
{
	Status sret = Status::Ok;
	//Bitmap bmp(width, height, PixelFormat32bppARGB);
	Graphics graphics{ &bmp };
    SolidBrush lightBlueBrush(Color(204, 232, 207));
	FontFamily fontFamily(L"Times New Roman");
	Font font(&fontFamily, 24, FontStyleBold, UnitPixel);
	SolidBrush textBrush(Color(128, 0, 64));

	sret = graphics.FillRectangle(&lightBlueBrush, 0, 0, bmp.GetWidth(), bmp.GetHeight());

	SYSTEMTIME sysTime = { 0 };
	GetLocalTime(&sysTime);
	std::string timeText = String::format("%d-%02d-%02d %02d:%02d:%02d.%03d",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	std::wstring wTimeText = Ansi2Wide(timeText);
    PointF pt;
    RectF rect;
    graphics.MeasureString(wTimeText.c_str(), (int)wTimeText.size(), &font, pt, &rect);
    pt.X = (bmp.GetWidth() - rect.Width) / 2.0F;
    pt.Y = 20;
    graphics.DrawString(wTimeText.c_str(), (int)wTimeText.size(), &font, pt, &textBrush);
    //SaveBitmap(bmp, L"test.png");
}
*/

#define CmoSetFrom(field, defaultValue)	\
if (js.contains(#field))				\
{										\
	chMediaOptions.field = js.value(#field, defaultValue);	\
}										\

#define CmoSetStringFrom(field, defaultValue)	\
if (js.contains(#field))				\
{										\
	static std::string field = js.value(#field, defaultValue);	\
	chMediaOptions.field = field.c_str();						\
}										\

#define CmoCastFrom(field, type, defaultValue)	\
if (js.contains(#field))				\
{										\
	chMediaOptions.field = static_cast<type>(js.value(#field, defaultValue));	\
}										\

void json2ChannelMediaOptions(const char* jsonOptions, agora::rtc::ChannelMediaOptions& chMediaOptions)
{
	json js = json::parse(jsonOptions);

	CmoSetFrom(publishCameraTrack, false);
	CmoSetFrom(publishSecondaryCameraTrack, false);

#if AGORA_SDK_VERSION >= 40000000
	if (js.contains("publishAudioTrack"))
	{
		chMediaOptions.publishMicrophoneTrack = js.value("publishAudioTrack", false);
	}
#else
	CmoSetFrom(publishAudioTrack, false);
#endif
	CmoSetFrom(publishScreenTrack, false);
	CmoSetFrom(publishSecondaryScreenTrack, false);
	CmoSetFrom(publishCustomAudioTrack, false);
	CmoSetFrom(publishCustomAudioTrackEnableAec, false);
	CmoSetFrom(publishCustomVideoTrack, false);
	CmoSetFrom(publishEncodedVideoTrack, false);
	CmoSetFrom(publishMediaPlayerAudioTrack, false);
	CmoSetFrom(publishMediaPlayerVideoTrack, false);
	CmoSetFrom(publishTrancodedVideoTrack, false);
	CmoSetFrom(autoSubscribeAudio, false);
	CmoSetFrom(autoSubscribeVideo, false);
	CmoSetFrom(enableAudioRecordingOrPlayout, false);
#if AGORA_SDK_VERSION >= 38202000
	CmoSetFrom(publishCustomAudioSourceId, false);
	CmoSetFrom(publishDirectCustomAudioTrack, false);
	CmoSetFrom(enableBuiltInMediaEncryption, false);
	CmoSetFrom(publishRhythmPlayerTrack, false);
	CmoSetFrom(isInteractiveAudience, false);
#endif

#if AGORA_SDK_VERSION >= 38202000 && AGORA_SDK_VERSION <= 38209000
	CmoSetFrom(startPreview, false);
#endif

	CmoSetFrom(publishMediaPlayerId, 0);
	CmoSetFrom(audioDelayMs, 0);
#if AGORA_SDK_VERSION >= 38202000
	CmoSetFrom(mediaPlayerAudioDelayMs, 0);
	CmoSetStringFrom(token, "");
#endif

	CmoCastFrom(channelProfile, agora::CHANNEL_PROFILE_TYPE, 0);
	CmoCastFrom(clientRoleType, agora::rtc::CLIENT_ROLE_TYPE, 0);
	CmoCastFrom(audienceLatencyLevel, agora::rtc::AUDIENCE_LATENCY_LEVEL_TYPE, 0);
	CmoCastFrom(defaultVideoStreamType, agora::rtc::VIDEO_STREAM_TYPE, 0);
}

AGORA_CAPI const char* getSdkErrorDescription(int error)
{
    return getAgoraSdkErrorDescription(error);
}

AGORA_CAPI IRtcEngine* createRtcEngine()
{
	//auto new_logger = spdlog::basic_logger_mt("file_logger", "agsdklog/cpp.log", true);
	//spdlog::set_default_logger(new_logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e P%P T%t %l: %v");

    auto rtcEngine = createAgoraRtcEngine();
    spdlog::info("CPP {} createAgoraRtcEngine {}", __FUNCTION__, fmt::ptr(rtcEngine));
    return rtcEngine;
}

AGORA_CAPI void release(IRtcEngine* rtcEngine, int sync)
{
    if (rtcEngine == nullptr)
    {
        return;
    }

    if (sAgoraParameter)
    {
        sAgoraParameter->release();
        sAgoraParameter = nullptr;
    }

    if (sMediaEngine)
    {
        sMediaEngine->release();
        sMediaEngine = nullptr;
    }

	if (sAudioDeviceManager)
	{
		sAudioDeviceManager->release();
		sAudioDeviceManager = nullptr;
	}

    if (sVideoDeviceManager)
    {
        sVideoDeviceManager->release();
        sVideoDeviceManager = nullptr;
    }

    rtcEngine->release(sync);
    spdlog::info("CPP {} {} released", __FUNCTION__, fmt::ptr(rtcEngine));

	sVideoFrameObserver.resetAllStats();
}

AGORA_CAPI AgoraEventHandler* createRtcEngineEventHandler()
{
    AgoraEventHandler* eventHandler = new AgoraEventHandler();
    spdlog::info("CPP {} create AgoraEventHandler {}", __FUNCTION__, fmt::ptr(eventHandler));
    return eventHandler;
}

AGORA_CAPI void releaseRtcEngineEventHandler(AgoraEventHandler* eventHandler)
{
    delete eventHandler;
}

AGORA_CAPI void setRtcEngineEventCallback(AgoraEventHandler* eventHandler, AgoraEngineEventCallback callback)
{
    if (eventHandler)
    {
        eventHandler->setEventCallback(callback);
    }
}

AGORA_CAPI const char* getVersion(IRtcEngine* rtcEngine, int* build)
{
    if (rtcEngine == nullptr)
    {
        return nullptr;
    }
    return rtcEngine->getVersion(build);
}

AGORA_CAPI const char* getErrorDescription(IRtcEngine* rtcEngine, int code)
{
    if (rtcEngine == nullptr)
    {
        return nullptr;
    }
    return rtcEngine->getErrorDescription(code);
}

AGORA_CAPI int initialize(IRtcEngine* rtcEngine, const char* appId, int areaCode, AUDIO_SCENARIO_TYPE audioScenario,
    CHANNEL_PROFILE_TYPE channelProfile, int enableAudioDevice, AgoraEventHandler* eventHandler,
    const char* logPath, int logSizeInKB, LOG_LEVEL logLevel)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

	sVideoFrameObserver.resetAllStats();

    RtcEngineContext context;
    context.appId = appId;
    context.areaCode = areaCode;
    context.audioScenario = audioScenario;
    context.channelProfile = channelProfile;
#if AGORA_SDK_VERSION < 50000000
    context.enableAudioDevice = (bool)enableAudioDevice;
#endif
    context.eventHandler = eventHandler;
    context.logConfig.filePath = logPath;
    context.logConfig.fileSizeInKB = logSizeInKB;
    context.logConfig.level = logLevel;
    int ret = rtcEngine->initialize(context);
    return ret;
}

AGORA_CAPI int setLogFileSize(IRtcEngine* rtcEngine, unsigned int fileSizeInKB)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->setLogFileSize(fileSizeInKB);
}

#if AGORA_SDK_VERSION >= 36200000
AGORA_CAPI int loadExtensionProvider(IRtcEngine* rtcEngine, const char* libPath)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->loadExtensionProvider(libPath);
}

AGORA_CAPI int enableExtension(IRtcEngine* rtcEngine, const char* providerName, const char* extensionName, int enabled,
	agora::media::MEDIA_SOURCE_TYPE type)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableExtension(providerName, extensionName, bool(enabled), type);
}

AGORA_CAPI int setBeautyEffectOptions(IRtcEngine* rtcEngine, int enabled, BeautyOptions::LIGHTENING_CONTRAST_LEVEL lighteningContrastLevel,
    float lighteningLevel, float rednessLevel, float sharpnessLevel, float smoothnessLevel, agora::media::MEDIA_SOURCE_TYPE type)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    BeautyOptions options;
    if (enabled)
    {
        options.lighteningContrastLevel = lighteningContrastLevel;
        options.lighteningLevel = lighteningLevel;
        options.rednessLevel = rednessLevel;
        options.sharpnessLevel = sharpnessLevel;
        options.smoothnessLevel = smoothnessLevel;
    }
    return rtcEngine->setBeautyEffectOptions((bool)enabled, options, type);
}

AGORA_CAPI int setExtensionProperty(IRtcEngine* rtcEngine, const char* provider_name, const char* extension_name,
    const char* key, const char* json_value, agora::media::MEDIA_SOURCE_TYPE type)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->setExtensionProperty(provider_name, extension_name, key, json_value, type);
}
#endif

#if AGORA_SDK_VERSION == 37200000 && IS_DEV_37200100
AGORA_CAPI int enableVirtualBackground(IRtcEngine* rtcEngine, int enabled, VirtualBackgroundSource::BACKGROUND_SOURCE_TYPE backgroundSourceType, int color, const char* source,
	VirtualBackgroundSource::BACKGROUND_BLUR_DEGREE blurDegree, VirtualBackgroundSource::SEG_MODEL_TYPE modelType, int prefervelocity, float greencapacity,
	SegmentationProperty::SEG_MODEL_TYPE segModelType, int segPrefervelocity, float segGreencapacity, agora::media::MEDIA_SOURCE_TYPE sourceType)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	VirtualBackgroundSource vbgSrc;
	vbgSrc.background_source_type = backgroundSourceType;
	vbgSrc.color = color;
	vbgSrc.source = source;
	vbgSrc.blur_degree = blurDegree;
	vbgSrc.modeltype = modelType;
	vbgSrc.prefervelocity = prefervelocity;
	vbgSrc.greencapacity = greencapacity;

	SegmentationProperty segPty;
	segPty.modeltype = segModelType;
	segPty.prefervelocity = segPrefervelocity;
	segPty.greencapacity = segGreencapacity;

	return rtcEngine->enableVirtualBackground((bool)enabled, vbgSrc, segPty, sourceType);
}
#endif

#if (AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109) || (AGORA_SDK_VERSION == 37204000 && IS_DEV_37204000)
AGORA_CAPI int enableBrightnessCorrection(IRtcEngine* rtcEngine, int enable, BRIGHTNESS_CORRECTION_MODE mode, agora::rtc::VIDEO_SOURCE_TYPE sourceType)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->enableBrightnessCorrection((bool)enable, (BRIGHTNESS_CORRECTION_MODE)mode, sourceType);
}

AGORA_CAPI int applyBrightnessCorrectionToRemote(IRtcEngine* rtcEngine, uid_t uid, int enable, BRIGHTNESS_CORRECTION_MODE mode)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->applyBrightnessCorrectionToRemote(uid, (bool)enable, mode);
}

AGORA_CAPI int applyBrightnessCorrectionToRemoteEx(IRtcEngine* rtcEngine, uid_t uid, int enable, BRIGHTNESS_CORRECTION_MODE mode, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	return rtcEngineEx->applyBrightnessCorrectionToRemoteEx(uid, (bool)enable, mode, connt);
}

void trapezoidJsonToOptions(const char* jsonOptions, TrapezoidCorrectionOptions& options)
{
    json js = json::parse(jsonOptions);

    if (js.contains("resetDragPoints"))
    {
		options.setResetDragPoints(js.value("resetDragPoints", 0));
    }
	if (js.contains("mirror"))
	{
		options.setMirror(js.value("mirror", 0));
	}
    if (js.contains("autoCorrect"))
    {
		options.setAutoCorrect(js.value("autoCorrect", 0));
    }
    if (js.contains("assistLine"))
    {
		options.showAssistLine(js.value("assistLine", 0));
    }
    if (js.contains("setDragPoints"))
    {
        if (js["setDragPoints"].contains("dragSrcPoints"))
        {
			int n = 0;
#if IS_DEV_37204000
            int arraySize = sizeof(TrapezoidCorrectionOptions::TrapezoidPoints) / sizeof(float);
            TrapezoidCorrectionOptions::TrapezoidPoints points;
            float* pFloat = reinterpret_cast<float*>(&points);
			for (auto it = js["setDragPoints"]["dragSrcPoints"].begin(); it != js["setDragPoints"]["dragSrcPoints"].end(); ++it)
			{
                pFloat[n++] = it.value();
				if (n >= arraySize)
				{
					break;
				}
			}
            options.dragSrcPoints = points;
#else
			for (auto it = js["setDragPoints"]["dragSrcPoints"].begin(); it != js["setDragPoints"]["dragSrcPoints"].end(); ++it)
			{
				options.dragSrcPoints[n++] = it.value();
				if (n >= TrapezoidCorrectionOptions::POINT_ARRAY_LEN)
				{
					break;
				}
			}
			options.hasMultiPoints = true;
#endif
        }
        if (js["setDragPoints"].contains("dragDstPoints"))
        {
            int n = 0;
#if IS_DEV_37204000
			int arraySize = sizeof(TrapezoidCorrectionOptions::TrapezoidPoints) / sizeof(float);
			TrapezoidCorrectionOptions::TrapezoidPoints points;
			float* pFloat = reinterpret_cast<float*>(&points);
			for (auto it = js["setDragPoints"]["dragDstPoints"].begin(); it != js["setDragPoints"]["dragDstPoints"].end(); ++it)
			{
				pFloat[n++] = it.value();
				if (n >= arraySize)
				{
					break;
				}
			}
			options.dragDstPoints = points;
#else
			for (auto it = js["setDragPoints"]["dragDstPoints"].begin(); it != js["setDragPoints"]["dragDstPoints"].end(); ++it)
			{
				options.dragDstPoints[n++] = it.value();
				if (n >= TrapezoidCorrectionOptions::POINT_ARRAY_LEN)
				{
					break;
				}
			}
			options.hasMultiPoints = true;
#endif
        }
    }
    if (js.contains("setDragPoint"))
    {
        if (js["setDragPoint"].contains("dragSrcPoint"))
        {
            TrapezoidCorrectionOptions::Point pt;
            pt.x = (float)js["setDragPoint"]["dragSrcPoint"].value("x", 0.0);
            pt.y = (float)js["setDragPoint"]["dragSrcPoint"].value("y", 0.0);
            options.dragSrcPoint = pt;
        }
        if (js["setDragPoint"].contains("dragDstPoint"))
        {
            TrapezoidCorrectionOptions::Point pt;
            pt.x = (float)js["setDragPoint"]["dragDstPoint"].value("x", 0.0);
            pt.y = (float)js["setDragPoint"]["dragDstPoint"].value("y", 0.0);
            options.dragDstPoint = pt;
        }
        if (js["setDragPoint"].contains("dragFinished"))
        {
            options.dragFinished = js["setDragPoint"]["dragFinished"];
        }
        else
        {
            options.dragFinished = 0;
        }
    }
}

void trapezoidOptionsToJson(const TrapezoidCorrectionOptions& options, std::string& jsonOptions)
{
    json js;

	if (options.resetDragPoints.has_value())
	{
		js["resetDragPoints"] = options.resetDragPoints.value();
	}
	if (options.mirror.has_value())
	{
		js["mirror"] = options.mirror.value();
	}
	if (options.autoCorrect.has_value())
	{
		js["autoCorrect"] = options.autoCorrect.value();
	}
    if (options.assistLine.has_value())
    {
        js["assistLine"] = options.assistLine.value();
    }
#if IS_DEV_37204000
	if (options.dragSrcPoints.has_value() && options.dragDstPoints.has_value())
	{
		json dragSetPoints;
		json dragSrcPoints = json::array();
		const float* pFloat = reinterpret_cast<const float*>(&options.dragSrcPoints.value());
		for (int n = 0; n < sizeof(TrapezoidCorrectionOptions::TrapezoidPoints) / sizeof(float); ++n)
		{
			dragSrcPoints.push_back(pFloat[n]);
		}
		dragSetPoints["dragSrcPoints"] = dragSrcPoints;
		json dragDstPoints = json::array();
		pFloat = reinterpret_cast<const float*>(&options.dragDstPoints.value());
		for (int n = 0; n < sizeof(TrapezoidCorrectionOptions::TrapezoidPoints) / sizeof(float); ++n)
		{
			dragDstPoints.push_back(pFloat[n]);
		}
		dragSetPoints["dragDstPoints"] = dragDstPoints;
		js["dragSetPoints"] = dragSetPoints;
	}
#else
    if (options.hasMultiPoints)
    {
        json dragSetPoints;
        json dragSrcPoints = json::array();
        for (int n = 0; n < TrapezoidCorrectionOptions::POINT_ARRAY_LEN; ++n)
        {
            dragSrcPoints.push_back(options.dragSrcPoints[n]);
        }
        dragSetPoints["dragSrcPoints"] = dragSrcPoints;
		json dragDstPoints = json::array();
		for (int n = 0; n < TrapezoidCorrectionOptions::POINT_ARRAY_LEN; ++n)
		{
            dragDstPoints.push_back(options.dragDstPoints[n]);
		}
		dragSetPoints["dragDstPoints"] = dragDstPoints;
        js["dragSetPoints"] = dragSetPoints;
    }
#endif
    if (options.dragDstPoint.has_value())
    {
        //when get option, does not have dragDstPoint
    }

    jsonOptions = js.dump(4);
}

AGORA_CAPI int enableLocalTrapezoidCorrection(IRtcEngine* rtcEngine, int enabled, agora::rtc::VIDEO_SOURCE_TYPE sourceType)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableLocalTrapezoidCorrection((bool)enabled, sourceType);
}

AGORA_CAPI int setLocalTrapezoidCorrectionOptions(IRtcEngine* rtcEngine, const char* jsonOptions, agora::rtc::VIDEO_SOURCE_TYPE sourceType)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    TrapezoidCorrectionOptions options;
    trapezoidJsonToOptions(jsonOptions, options);

    return rtcEngine->setLocalTrapezoidCorrectionOptions(options, sourceType);
}

AGORA_CAPI int getLocalTrapezoidCorrectionOptions(IRtcEngine* rtcEngine, char* jsonOptions, int size, agora::rtc::VIDEO_SOURCE_TYPE sourceType)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    TrapezoidCorrectionOptions options;
    int ret = rtcEngine->getLocalTrapezoidCorrectionOptions(options, sourceType);
    std::string jsonValue;
    trapezoidOptionsToJson(options, jsonValue);
    std::strncpy(jsonOptions, jsonValue.c_str(), size - 1);
    jsonOptions[size - 1] = 0;
    return ret;
}

AGORA_CAPI int enableRemoteTrapezoidCorrection(IRtcEngine* rtcEngine, uid_t uid, int enabled)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    return rtcEngine->enableRemoteTrapezoidCorrection(uid, (bool)enabled);
}

AGORA_CAPI int enableRemoteTrapezoidCorrectionEx(IRtcEngine* rtcEngine, uid_t uid, int enabled, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	return rtcEngineEx->enableRemoteTrapezoidCorrectionEx(uid, (bool)enabled, connt);
}

AGORA_CAPI int setRemoteTrapezoidCorrectionOptions(IRtcEngine* rtcEngine, uid_t uid, const char* jsonOptions)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    TrapezoidCorrectionOptions options;
    trapezoidJsonToOptions(jsonOptions, options);

    return rtcEngine->setRemoteTrapezoidCorrectionOptions(uid, options);
}

AGORA_CAPI int setRemoteTrapezoidCorrectionOptionsEx(IRtcEngine* rtcEngine, uid_t uid, const char* jsonOptions, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	TrapezoidCorrectionOptions options;
	trapezoidJsonToOptions(jsonOptions, options);

	return rtcEngineEx->setRemoteTrapezoidCorrectionOptionsEx(uid, options, connt);
}

AGORA_CAPI int getRemoteTrapezoidCorrectionOptions(IRtcEngine* rtcEngine, uid_t uid, char* jsonOptions, int size)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    TrapezoidCorrectionOptions options;
    int ret = rtcEngine->getRemoteTrapezoidCorrectionOptions(uid, options);
    std::string jsonValue;
    trapezoidOptionsToJson(options, jsonValue);
    std::strncpy(jsonOptions, jsonValue.c_str(), size - 1);
    jsonOptions[size - 1] = 0;
    return ret;
}

AGORA_CAPI int getRemoteTrapezoidCorrectionOptionsEx(IRtcEngine* rtcEngine, uid_t uid, char* jsonOptions, int size, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	TrapezoidCorrectionOptions options;
	int ret = rtcEngineEx->getRemoteTrapezoidCorrectionOptionsEx(uid, options, connt);
	std::string jsonValue;
	trapezoidOptionsToJson(options, jsonValue);
	std::strncpy(jsonOptions, jsonValue.c_str(), size - 1);
	jsonOptions[size - 1] = 0;
	return ret;
}

AGORA_CAPI int applyTrapezoidCorrectionToRemote(IRtcEngine* rtcEngine, uid_t uid, int enabled)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->applyTrapezoidCorrectionToRemote(uid, (bool)enabled);
}

AGORA_CAPI int applyTrapezoidCorrectionToRemoteEx(IRtcEngine* rtcEngine, uid_t uid, int enabled, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	return rtcEngineEx->applyTrapezoidCorrectionToRemoteEx(uid, (bool)enabled, connt);
}

AGORA_CAPI int applyVideoEncoderMirrorToRemote(IRtcEngine* rtcEngine, uid_t uid, VIDEO_MIRROR_MODE_TYPE mirrorMode)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->applyVideoEncoderMirrorToRemote(uid, mirrorMode);
}

AGORA_CAPI int applyVideoEncoderMirrorToRemoteEx(IRtcEngine* rtcEngine, uid_t uid, int mirrorMode, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	return rtcEngineEx->applyVideoEncoderMirrorToRemoteEx(uid, (VIDEO_MIRROR_MODE_TYPE)mirrorMode, connt);
}

AGORA_CAPI int applyVideoOrientationToRemote(IRtcEngine* rtcEngine, uid_t uid, VIDEO_ORIENTATION orientation)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->applyVideoOrientationToRemote(uid, orientation);
}

AGORA_CAPI int applyVideoOrientationToRemoteEx(IRtcEngine* rtcEngine, uid_t uid, VIDEO_ORIENTATION orientation, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	return rtcEngineEx->applyVideoOrientationToRemoteEx(uid, orientation, connt);
}
#endif

AGORA_CAPI int setChannelProfile(IRtcEngine* rtcEngine, CHANNEL_PROFILE_TYPE profile)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->setChannelProfile(profile);
}

AGORA_CAPI int setClientRole(IRtcEngine* rtcEngine, CLIENT_ROLE_TYPE role)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->setClientRole(role);
}

AGORA_CAPI int enableAudio(IRtcEngine* rtcEngine)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableAudio();
}

AGORA_CAPI int disableAudio(IRtcEngine* rtcEngine)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->disableAudio();
}

AGORA_CAPI int enableVideo(IRtcEngine* rtcEngine)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableVideo();
}

AGORA_CAPI int disableVideo(IRtcEngine* rtcEngine)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->disableVideo();
}

AGORA_CAPI int enableLocalAudio(IRtcEngine* rtcEngine, int enabled)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableLocalAudio((bool)enabled);
}

AGORA_CAPI int enableLocalVideo(IRtcEngine* rtcEngine, int enabled)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableLocalVideo((bool)enabled);
}

#if AGORA_SDK_VERSION >= 36200000
AGORA_CAPI int setCameraCapturerConfiguration(IRtcEngine* rtcEngine, const char* deviceId, int width, int height, int fps)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    CameraCapturerConfiguration config;
    memset(config.deviceId, 0, std::size(config.deviceId));
    std::strncpy(config.deviceId, deviceId, std::size(config.deviceId) - 1);
    config.format.width = width;
    config.format.height = height;
    config.format.fps = fps;
    return rtcEngine->setCameraCapturerConfiguration(config);
}
#endif

AGORA_CAPI int startPrimaryCameraCapture(IRtcEngine* rtcEngine, const char* deviceId, int width, int height, int fps)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	CameraCapturerConfiguration config;
	memset(config.deviceId, 0, std::size(config.deviceId));
	std::strncpy(config.deviceId, deviceId, std::size(config.deviceId) - 1);
	config.format.width = width;
	config.format.height = height;
	config.format.fps = fps;
	return rtcEngine->startPrimaryCameraCapture(config);
}

AGORA_CAPI int stopPrimaryCameraCapture(IRtcEngine* rtcEngine)
{
	return rtcEngine->stopPrimaryCameraCapture();
}

AGORA_CAPI int startSecondaryCameraCapture(IRtcEngine* rtcEngine, const char* deviceId, int width, int height, int fps)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	CameraCapturerConfiguration config;
	memset(config.deviceId, 0, std::size(config.deviceId));
	std::strncpy(config.deviceId, deviceId, std::size(config.deviceId) - 1);
	config.format.width = width;
	config.format.height = height;
	config.format.fps = fps;
	return rtcEngine->startSecondaryCameraCapture(config);
}

AGORA_CAPI int stopSecondaryCameraCapture(IRtcEngine* rtcEngine)
{
	return rtcEngine->stopSecondaryCameraCapture();
}

AGORA_CAPI int setCameraDeviceOrientation(IRtcEngine* rtcEngine, agora::rtc::VIDEO_SOURCE_TYPE sourceType, VIDEO_ORIENTATION orientation)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->setCameraDeviceOrientation(sourceType, orientation);
}

AGORA_CAPI int setVideoEncoderConfiguration(IRtcEngine* rtcEngine, unsigned connnectionId, int width = 640, int height = 360, int frameRate = 15,
    int bitrate = 0, VIDEO_CODEC_TYPE codecType = VIDEO_CODEC_H264, DEGRADATION_PREFERENCE degradationPreference = MAINTAIN_QUALITY,
    int minBitrate = -1, VIDEO_MIRROR_MODE_TYPE mirrorMode = VIDEO_MIRROR_MODE_DISABLED, ORIENTATION_MODE orientationMode = ORIENTATION_MODE_ADAPTIVE)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    VideoEncoderConfiguration config;
    config.bitrate = bitrate;
    config.codecType = codecType;
    config.degradationPreference = degradationPreference;
    config.dimensions.width = width;
    config.dimensions.height = height;
    config.frameRate = frameRate;
    config.minBitrate = minBitrate;
    config.mirrorMode = mirrorMode;
    config.orientationMode = orientationMode;

#if AGORA_SDK_VERSION>=36200000
    return rtcEngine->setVideoEncoderConfiguration(config);
#else
    return rtcEngine->setVideoEncoderConfiguration(config, connnectionId);
#endif
}

#if AGORA_SDK_VERSION>=36200000
AGORA_CAPI int setVideoEncoderConfigurationEx(IRtcEngine* rtcEngine, const char* channelId, uid_t uid, int width = 640, int height = 360, int frameRate = 15,
	int bitrate = 0, VIDEO_CODEC_TYPE codecType = VIDEO_CODEC_H264, DEGRADATION_PREFERENCE degradationPreference = MAINTAIN_QUALITY,
	int minBitrate = -1, VIDEO_MIRROR_MODE_TYPE mirrorMode = VIDEO_MIRROR_MODE_DISABLED, ORIENTATION_MODE orientationMode = ORIENTATION_MODE_ADAPTIVE)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
    RtcConnection connt(channelId, uid);

	VideoEncoderConfiguration config;
	config.bitrate = bitrate;
	config.codecType = codecType;
	config.degradationPreference = degradationPreference;
	config.dimensions.width = width;
	config.dimensions.height = height;
	config.frameRate = frameRate;
	config.minBitrate = minBitrate;
	config.mirrorMode = mirrorMode;
	config.orientationMode = orientationMode;

	return rtcEngineEx->setVideoEncoderConfigurationEx(config, connt);
}
#endif

AGORA_CAPI int setupLocalVideo(IRtcEngine* rtcEngine, uid_t uid, void* view, VIDEO_MIRROR_MODE_TYPE mirrorMode, RENDER_MODE_TYPE renderMode,
	agora::rtc::VIDEO_SOURCE_TYPE sourceType, int isScreenView, int setupMode)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	spdlog::info("CPP {} view {} setupMode {}", __FUNCTION__, fmt::ptr(view), setupMode);
	VideoCanvas canvas;
	canvas.isScreenView = (bool)isScreenView;
	canvas.mirrorMode = mirrorMode;
	canvas.renderMode = renderMode;
	canvas.uid = uid;
	canvas.sourceType = sourceType;
	canvas.view = view;

#if AGORA_SDK_VERSION >= 38200000
	if (setupMode >= 0)
	{
		canvas.setupMode = (VIDEO_VIEW_SETUP_MODE)setupMode;	//VIDEO_VIEW_SETUP_MODE
	}
	return rtcEngine->setupLocalVideo(canvas);
#elif AGORA_SDK_VERSION >= 36200104 && AGORA_SDK_VERSION <= 36200109
	if (setupMode == -1) //-1 indicates not using setupMode
	{
		return rtcEngine->setupLocalVideo(canvas);
	}
	else
	{
		return rtcEngine->setupLocalVideo(canvas, (VIDEO_VIEW_SETUP_MODE)setupMode);
	}
#elif AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200103
	if (setupMode == -1)
	{
		return rtcEngine->setupLocalVideo(canvas);
	}
	else
	{
		IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
		return rtcEngineEx->setupLocalVideoEx(canvas, (VIDEO_VIEW_SETUP_MODE)setupMode);
	}
#else
	return rtcEngine->setupLocalVideo(canvas);
#endif
}

AGORA_CAPI int setLocalVideoMirrorMode(IRtcEngine* rtcEngine, VIDEO_MIRROR_MODE_TYPE mirrorMode)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
    return rtcEngine->setLocalVideoMirrorMode(mirrorMode);
}

#if (AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109)
AGORA_CAPI int setLocalRenderMode(IRtcEngine* rtcEngine, RENDER_MODE_TYPE renderMode, VIDEO_MIRROR_MODE_TYPE mirrorMode, agora::rtc::VIDEO_SOURCE_TYPE sourceType)
#else
AGORA_CAPI int setLocalRenderMode(IRtcEngine* rtcEngine, RENDER_MODE_TYPE renderMode, VIDEO_MIRROR_MODE_TYPE mirrorMode)
#endif
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
#if (AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109)
	return rtcEngine->setLocalRenderMode(renderMode, mirrorMode, sourceType);
#else
	return rtcEngine->setLocalRenderMode(renderMode, mirrorMode);
#endif
}

AGORA_CAPI int setRemoteRenderMode(IRtcEngine* rtcEngine, uid_t uid, RENDER_MODE_TYPE renderMode, VIDEO_MIRROR_MODE_TYPE mirrorMode)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->setRemoteRenderMode(uid, renderMode, mirrorMode);
}

AGORA_CAPI int setupRemoteVideo(IRtcEngine* rtcEngine, uid_t uid, void* view, VIDEO_MIRROR_MODE_TYPE mirrorMode,
    RENDER_MODE_TYPE renderMode, agora::rtc::VIDEO_SOURCE_TYPE sourceType, int setupMode, int isScreenView, unsigned int connectionId)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    spdlog::info("CPP {} connectionId {} uid {} view {}", __FUNCTION__, connectionId, uid, fmt::ptr(view));
    VideoCanvas canvas;
    canvas.isScreenView = (bool)isScreenView;
    canvas.mirrorMode = mirrorMode;
    canvas.renderMode = renderMode;
    canvas.uid = uid;
    canvas.sourceType = sourceType;
    canvas.view = view;

#if AGORA_SDK_VERSION>=36200000
    return rtcEngine->setupRemoteVideo(canvas);
#else
    return rtcEngine->setupRemoteVideo(canvas, connectionId);
#endif
}

AGORA_CAPI int startPreview(IRtcEngine* rtcEngine)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->startPreview();
}

AGORA_CAPI int stopPreview(IRtcEngine* rtcEngine)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->stopPreview();
}

#if AGORA_SDK_VERSION >= 36200000
AGORA_CAPI int startPreview2(IRtcEngine* rtcEngine, agora::rtc::VIDEO_SOURCE_TYPE type)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->startPreview(type);
}

AGORA_CAPI int stopPreview2(IRtcEngine* rtcEngine, agora::rtc::VIDEO_SOURCE_TYPE type)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->stopPreview(type);
}
#endif

#if AGORA_SDK_VERSION >= 37200000 && AGORA_SDK_VERSION != 37201100
AGORA_CAPI int getScreenCaptureSources(IRtcEngine* rtcEngine, int thumbWidth, int thumbHeight, int iconWidth, int iconHeight, int includeScreen, char* outBuf, int size)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	SIZE thumbSize(thumbWidth, thumbHeight);
	SIZE iconSize(iconWidth, iconHeight);
	agora::rtc::IScreenCaptureSourceList* csList = rtcEngine->getScreenCaptureSources(thumbSize, iconSize, (bool)includeScreen);
	if (csList)
	{
		json js;
		FILE* pFile = fopen("agora_screen_sources.data", "wb");
		unsigned int count = csList->getCount();
		for (unsigned int n = 0; n < count; ++n)
		{
			json jsInfo;
			agora::rtc::ScreenCaptureSourceInfo csInfo = csList->getSourceInfo(n);
			jsInfo["type"] = csInfo.type;
			jsInfo["primaryMonitor"] = csInfo.primaryMonitor;
			jsInfo["isOccluded"] = csInfo.isOccluded;
			jsInfo["processPath"] = csInfo.processPath;
			jsInfo["sourceId"] = (size_t)csInfo.sourceId;
			jsInfo["sourceName"] = csInfo.sourceName;
			jsInfo["sourceTitle"] = csInfo.sourceTitle;
			json jsThumb;
			jsThumb["length"] = csInfo.thumbImage.length;
			jsThumb["width"] = csInfo.thumbImage.width;
			jsThumb["height"] = csInfo.thumbImage.height;
			jsInfo["thumbImage"] = jsThumb;
			json jsIcon;
			jsIcon["length"] = csInfo.iconImage.length;
			jsIcon["width"] = csInfo.iconImage.width;
			jsIcon["height"] = csInfo.iconImage.height;
			jsInfo["iconImage"] = jsIcon;

			js["sourceList"].push_back(jsInfo);

			if (pFile)
			{
				fwrite(&n, sizeof(n), 1, pFile);
				fwrite(&csInfo.thumbImage.length, sizeof(csInfo.thumbImage.length), 1, pFile);
				fwrite(&csInfo.thumbImage.width, sizeof(csInfo.thumbImage.width), 1, pFile);
				fwrite(&csInfo.thumbImage.height, sizeof(csInfo.thumbImage.height), 1, pFile);
				if (csInfo.thumbImage.length > 0)
				{
					fwrite(csInfo.thumbImage.buffer, csInfo.thumbImage.length, 1, pFile);
				}
				fwrite(&csInfo.iconImage.length, sizeof(csInfo.iconImage.length), 1, pFile);
				fwrite(&csInfo.iconImage.width, sizeof(csInfo.iconImage.width), 1, pFile);
				fwrite(&csInfo.iconImage.height, sizeof(csInfo.iconImage.height), 1, pFile);
				if (csInfo.iconImage.length > 0)
				{
					fwrite(csInfo.iconImage.buffer, csInfo.iconImage.length, 1, pFile);
				}
			}
		}

		if (pFile)
		{
			fclose(pFile);
			pFile = nullptr;
		}

		std::string jsonStr = js.dump();
		std::strncpy(outBuf, jsonStr.c_str(), size);
		outBuf[size - 1] = '\0';

		csList->release();
		csList = nullptr;
		return 0;
	}


	return -1;
}
#endif

AGORA_CAPI int startScreenCaptureByScreenRect(IRtcEngine* rtcEngine, int sx, int sy, int sWidth, int sHeight,
	int rx, int ry, int rWidth, int rHeight, int dWidth, int dHeight, int fps, int bitrate, agora::view_t* excludeWindows, int windowSize)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	agora::rtc::Rectangle sRect{ sx, sy, sWidth, sHeight };
	agora::rtc::Rectangle rRect{ rx, ry, rWidth, rHeight };
	ScreenCaptureParameters scParam;
	scParam.dimensions.width = dWidth;
	scParam.dimensions.height = dHeight;
	scParam.frameRate = fps;
	scParam.bitrate = bitrate;
	scParam.excludeWindowList = excludeWindows;
	scParam.excludeWindowCount = windowSize;
	return rtcEngine->startScreenCaptureByScreenRect(sRect, rRect, scParam);
}

AGORA_CAPI int startPrimaryScreenCapture(IRtcEngine* rtcEngine, int sx, int sy, int sWidth, int sHeight,
	int rx, int ry, int rWidth, int rHeight, int dWidth, int dHeight, int fps, int bitrate, agora::view_t* excludeWindows, int windowSize)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	agora::rtc::Rectangle sRect{ sx, sy, sWidth, sHeight };
	agora::rtc::Rectangle rRect{ rx, ry, rWidth, rHeight };
	ScreenCaptureParameters scParam;
	scParam.dimensions.width = dWidth;
	scParam.dimensions.height = dHeight;
	scParam.frameRate = fps;
	scParam.bitrate = bitrate;
	scParam.excludeWindowList = excludeWindows;
	scParam.excludeWindowCount = windowSize;

	ScreenCaptureConfiguration config;
	config.isCaptureWindow = false;
	config.screenRect = sRect;
	config.regionRect = rRect;
	config.params = scParam;
	return rtcEngine->startPrimaryScreenCapture(config);
}

AGORA_CAPI int startSecondaryScreenCapture(IRtcEngine* rtcEngine, int sx, int sy, int sWidth, int sHeight,
	int rx, int ry, int rWidth, int rHeight, int dWidth, int dHeight, int fps, int bitrate, agora::view_t* excludeWindows, int windowSize)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	agora::rtc::Rectangle sRect{ sx, sy, sWidth, sHeight };
	agora::rtc::Rectangle rRect{ rx, ry, rWidth, rHeight };
	ScreenCaptureParameters scParam;
	scParam.dimensions.width = dWidth;
	scParam.dimensions.height = dHeight;
	scParam.frameRate = fps;
	scParam.bitrate = bitrate;
	scParam.excludeWindowList = excludeWindows;
	scParam.excludeWindowCount = windowSize;

	ScreenCaptureConfiguration config;
	config.isCaptureWindow = false;
	config.screenRect = sRect;
	config.regionRect = rRect;
	config.params = scParam;
	return rtcEngine->startSecondaryScreenCapture(config);
}

AGORA_CAPI int startScreenCaptureByWindowId(IRtcEngine* rtcEngine, view_t view, int x, int y, int width, int height,
	int dWidth, int dHeight, int fps, int bitrate)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	agora::rtc::Rectangle rect{ x, y, width, height };
	ScreenCaptureParameters scParam;
	scParam.dimensions.width = dWidth;
	scParam.dimensions.height = dHeight;
	scParam.frameRate = fps;
	scParam.bitrate = bitrate;
	return rtcEngine->startScreenCaptureByWindowId(view, rect, scParam);
}

AGORA_CAPI int stopScreenCapture(IRtcEngine* rtcEngine)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->stopScreenCapture();
}

AGORA_CAPI int registerLocalUserAccount(IRtcEngine* rtcEngine, const char* appId, const char* userAccount)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->registerLocalUserAccount(appId, userAccount);
}

AGORA_CAPI int joinChannel(IRtcEngine* rtcEngine, const char* channelId, uid_t uid, const char* token, const char* info)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->joinChannel(token, channelId, info, uid);
}

AGORA_CAPI int joinChannelWithOptions(IRtcEngine* rtcEngine, const char* channelId, uid_t uid, const char* token,
    const char* optionsJson)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

    ChannelMediaOptions options;
	json2ChannelMediaOptions(optionsJson, options);

	return rtcEngine->joinChannel(token, channelId, uid, options);
}

AGORA_CAPI int updateChannelMediaOptions(IRtcEngine* rtcEngine, const char* optionsJson)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	ChannelMediaOptions options;
	json2ChannelMediaOptions(optionsJson, options);

	return rtcEngine->updateChannelMediaOptions(options);
}

AGORA_CAPI int joinChannelWithUserAccount(IRtcEngine* rtcEngine, const char* channelId,
	const char* userAccount, const char* token)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->joinChannelWithUserAccount(token, channelId, userAccount);
}

AGORA_CAPI int joinChannelWithUserAccount2(IRtcEngine* rtcEngine, const char* channelId,
	const char* userAccount, const char* token, const char* optionsJson)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	ChannelMediaOptions options;
	json2ChannelMediaOptions(optionsJson, options);

	return rtcEngine->joinChannelWithUserAccount(token, channelId, userAccount, options);
}

AGORA_CAPI int leaveChannel(IRtcEngine* rtcEngine)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->leaveChannel();
}

#if AGORA_SDK_VERSION>=36200000

AGORA_CAPI int setupRemoteVideoEx(IRtcEngine* rtcEngine, uid_t uid, void* view, VIDEO_MIRROR_MODE_TYPE mirrorMode,
	RENDER_MODE_TYPE renderMode, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	spdlog::info("CPP {} uid {} view {}", __FUNCTION__, uid, fmt::ptr(view));
	VideoCanvas canvas;
	canvas.mirrorMode = mirrorMode;
	canvas.renderMode = renderMode;
	canvas.uid = uid;
	canvas.view = view;

	return rtcEngineEx->setupRemoteVideoEx(canvas, connt);
}


AGORA_CAPI int joinChannelEx(IRtcEngine* rtcEngine, const char* channelId, uid_t uid,
	const char* token, const char* optionsJson, AgoraEventHandler* eventHandler)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

    IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, uid);

	ChannelMediaOptions options;
	json2ChannelMediaOptions(optionsJson, options);

	return rtcEngineEx->joinChannelEx(token, connt, options, eventHandler);
}

AGORA_CAPI int updateChannelMediaOptionsEx(IRtcEngine* rtcEngine, const char* channelId, uid_t uid, const char* optionsJson)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, uid);

	ChannelMediaOptions options;
	json2ChannelMediaOptions(optionsJson, options);

	return rtcEngineEx->updateChannelMediaOptionsEx(options, connt);
}


AGORA_CAPI int joinChannelWithUserAccountEx(IRtcEngine* rtcEngine, const char* channelId, const char* userAccount,
	const char* token, const char* optionsJson, AgoraEventHandler* eventHandler)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	ChannelMediaOptions options;
	json2ChannelMediaOptions(optionsJson, options);

	return rtcEngineEx->joinChannelWithUserAccountEx(token, channelId, userAccount, options, eventHandler);
}


AGORA_CAPI int leaveChannelEx(IRtcEngine* rtcEngine, const char* channelId, uid_t uid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
    RtcConnection connt(channelId, uid);

	return rtcEngineEx->leaveChannelEx(connt);
}
#endif

AGORA_CAPI int muteLocalAudioStream(IRtcEngine* rtcEngine, int mute)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->muteLocalAudioStream((bool)mute);
}

AGORA_CAPI int muteLocalVideoStream(IRtcEngine* rtcEngine, int mute)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->muteLocalVideoStream((bool)mute);
}

AGORA_CAPI int muteRemoteAudioStream(IRtcEngine* rtcEngine, uid_t uid, int mute, unsigned int connectionId)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

#if AGORA_SDK_VERSION>=36200000
	return rtcEngine->muteRemoteAudioStream(uid, (bool)mute);
#else
	return rtcEngine->muteRemoteAudioStream(uid, (bool)mute, connectionId);
#endif
}

AGORA_CAPI int muteRemoteVideoStream(IRtcEngine* rtcEngine, uid_t uid, int mute, unsigned int connectionId)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

#if AGORA_SDK_VERSION>=36200000
	return rtcEngine->muteRemoteVideoStream(uid, (bool)mute);
#else
	return rtcEngine->muteRemoteVideoStream(uid, (bool)mute, connectionId);
#endif
}

#if (AGORA_SDK_VERSION >= 36200105 && AGORA_SDK_VERSION <= 36200109)
AGORA_CAPI int takeSnapshot(IRtcEngine* rtcEngine, uid_t uid, const char* filePath,
	float left, float top, float right, float bottom)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->takeSnapshot(uid, filePath, left, top, right, bottom);
}

AGORA_CAPI int takeSnapshotEx(IRtcEngine* rtcEngine, uid_t uid, const char* filePath,
	float left, float top, float right, float bottom, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, localUid);

	return rtcEngineEx->takeSnapshotEx(uid, filePath, left, top, right, bottom, connt);
}

AGORA_CAPI int startServerSuperResolution(IRtcEngine* rtcEngine, const char* token, const char* srcImagePath, const char* dstImagePath, float scale, int timeoutSeconds)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->startServerSuperResolution(token, srcImagePath, dstImagePath, scale, timeoutSeconds);
}

#endif

#if AGORA_SDK_VERSION>=38200000
AGORA_CAPI int takeSnapshot(IRtcEngine* rtcEngine, uid_t uid, const char* filePath)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->takeSnapshot(uid, filePath);
}
#endif

AGORA_CAPI int createDataStream(IRtcEngine* rtcEngine, int* streamId, int reliable, int ordered, unsigned int connectionId)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

#if AGORA_SDK_VERSION>=36200000
    return rtcEngine->createDataStream(streamId, reliable, ordered);
#else
    return rtcEngine->createDataStream(streamId, reliable, ordered, connectionId);
#endif
}

AGORA_CAPI int sendStreamMessage(IRtcEngine* rtcEngine, int streamId, const char* data, int len, unsigned int connectionId)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

#if AGORA_SDK_VERSION>=36200000
    return rtcEngine->sendStreamMessage(streamId, data, len);
#else
    return rtcEngine->sendStreamMessage(streamId, data, len, connectionId);
#endif
}

#define INIT_VIDEO_DEVICE_MANAGER		\
	if (rtcEngine == nullptr)			\
	{									\
		return -1;						\
	}									\
	if (sVideoDeviceManager == nullptr)																				\
	{																												\
		INTERFACE_ID_TYPE iid = rtc::AGORA_IID_VIDEO_DEVICE_MANAGER;												\
		if (rtcEngine->queryInterface(iid, (void**)&sVideoDeviceManager) != 0)										\
		{																											\
			spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);	\
		}																											\
	}																												\
	if (sVideoDeviceManager == nullptr)	\
	{									\
		return -1;						\
	}									\

AGORA_CAPI int enumerateVideoDevices(IRtcEngine* rtcEngine, char* szDeviceList, int size)
{
	INIT_VIDEO_DEVICE_MANAGER

    std::string deviceInfo;
    deviceInfo.reserve(size);
    IVideoDeviceCollection* deviceCollection = sVideoDeviceManager->enumerateVideoDevices();
    if (deviceCollection)
    {
        int count = deviceCollection->getCount();
        for (int n = 0; n < count; ++n)
        {
            char szDeviceName[MAX_DEVICE_ID_LENGTH]{};
            char szDeviceId[MAX_DEVICE_ID_LENGTH]{};
            int ret = deviceCollection->getDevice(n, szDeviceName, szDeviceId);
            if (ret != 0)
            {
                spdlog::info("CPP {} IVideoDeviceCollection {} getDevice returns {}", __FUNCTION__, fmt::ptr(deviceCollection), ret);
            }
            deviceInfo += szDeviceName;
            deviceInfo += "%%";
            deviceInfo += szDeviceId;
			if (n < count - 1)
			{
				deviceInfo += "||";
			}
        }
        deviceCollection->release();
        deviceCollection = nullptr;
    }
    else
    {
        spdlog::info("CPP {} IVideoDeviceManager {} enumerateVideoDevices returns null", __FUNCTION__, fmt::ptr(sVideoDeviceManager));
    }
    std::strncpy(szDeviceList, deviceInfo.c_str(), size);
	szDeviceList[size - 1] = '\0';
    return 0;
}

AGORA_CAPI int getVideoDevice(IRtcEngine* rtcEngine, char* szDeviceId)
{
	INIT_VIDEO_DEVICE_MANAGER

	return sVideoDeviceManager->getDevice(szDeviceId);
}

AGORA_CAPI int setVideoDevice(IRtcEngine* rtcEngine, const char* deviceId)
{
	INIT_VIDEO_DEVICE_MANAGER

	return sVideoDeviceManager->setDevice(deviceId);
}

AGORA_CAPI int startVideoDeviceTest(IRtcEngine* rtcEngine, void* hwnd)
{
	INIT_VIDEO_DEVICE_MANAGER

	return sVideoDeviceManager->startDeviceTest(hwnd);
}

AGORA_CAPI int stopVideoDeviceTest(IRtcEngine* rtcEngine)
{
	INIT_VIDEO_DEVICE_MANAGER

	return sVideoDeviceManager->stopDeviceTest();
}

AGORA_CAPI int getVideoDeviceNumberOfCapabilities(IRtcEngine* rtcEngine, const char* szDeviceId)
{
	INIT_VIDEO_DEVICE_MANAGER

#if AGORA_SDK_VERSION == 37200000 || AGORA_SDK_VERSION == 37201100 || AGORA_SDK_VERSION >= 38202000
	return sVideoDeviceManager->numberOfCapabilities(szDeviceId);
#else
	return 0;
#endif
}

AGORA_CAPI int getVideoDeviceCapabilities(IRtcEngine* rtcEngine, const char* szDeviceId, char* szCapabilities, int size)
{
	INIT_VIDEO_DEVICE_MANAGER

#if AGORA_SDK_VERSION == 37200000 || AGORA_SDK_VERSION == 37201100 || AGORA_SDK_VERSION >= 38202000
	std::string strOutput;
	char szFormat[64];
	int count = sVideoDeviceManager->numberOfCapabilities(szDeviceId);
	for (int n = 0; n < count; ++n)
	{
		VideoFormat vformat;
		sVideoDeviceManager->getCapability(szDeviceId, n, vformat);
		std::snprintf(szFormat, sizeof(szFormat), "%d|%d|%d", vformat.width, vformat.height, vformat.fps);
		strOutput += szFormat;
		if (n < count - 1)
		{
			strOutput += "||";
		}
	}
	strncpy(szCapabilities, strOutput.c_str(), size);
	szCapabilities[size - 1] = '\0';
#else
#endif
	return 0;
}

AGORA_CAPI int setExternalVideoSource(IRtcEngine* rtcEngine, int enabled)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sMediaEngine == nullptr)
	{
		if (rtcEngine->queryInterface(rtc::AGORA_IID_MEDIA_ENGINE, (void**)&sMediaEngine) != 0)
		{
			//failed
		}
	}

	if (sMediaEngine)
	{
		return sMediaEngine->setExternalVideoSource((bool)enabled, false);
	}

    return -1;
}

AGORA_CAPI int pushVideoFrame(IRtcEngine* rtcEngine, void* rawData, agora::media::base::VIDEO_PIXEL_FORMAT format, int width, int height)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sMediaEngine == nullptr)
	{
		if (rtcEngine->queryInterface(rtc::AGORA_IID_MEDIA_ENGINE, (void**)&sMediaEngine) != 0)
		{
			//failed
		}
	}

	if (sMediaEngine)
	{
		std::unique_ptr<char> uBuf;

		if (rawData == nullptr)
		{
			/*
			Bitmap bmp(width, height, PixelFormat32bppARGB);
			drawBitmap(bmp);
			//SaveBitmap(bmp, L"pushVideoFrame.bmp");
			//return 0;
			Gdiplus::Rect rect{ 0, 0, width, height };

			uBuf.reset(new char[width * height * 4]);
			Gdiplus::BitmapData bmpData{};
			bmpData.Width = rect.Width;
			bmpData.Height = rect.Height;
			bmpData.PixelFormat = PixelFormat32bppARGB;
			bmpData.Stride = rect.Width * 4;
			bmpData.Scan0 = uBuf.get();
			bmp.LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeUserInputBuf | Gdiplus::ImageLockMode::ImageLockModeRead,
				PixelFormat32bppARGB, &bmpData);
			bmp.UnlockBits(&bmpData);
			rawData = uBuf.get();
			format = agora::media::base::VIDEO_PIXEL_RGBA;
			*/
		}

		agora::media::base::ExternalVideoFrame frame;
        frame.type = agora::media::base::ExternalVideoFrame::VIDEO_BUFFER_RAW_DATA;
		frame.format = format;
		frame.buffer = rawData;
        frame.stride = width;
        frame.height = height;
        frame.timestamp = GetTickCount64();
		return sMediaEngine->pushVideoFrame(&frame);
	}

	return -1;
}

#if AGORA_SDK_VERSION>=36200000
AGORA_CAPI int pushVideoFrameEx(IRtcEngine* rtcEngine, void* rawData, agora::media::base::VIDEO_PIXEL_FORMAT format, int width, int height, const char* channelId, uid_t localUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sMediaEngine == nullptr)
	{
		if (rtcEngine->queryInterface(rtc::AGORA_IID_MEDIA_ENGINE, (void**)&sMediaEngine) != 0)
		{
			//failed
		}
	}

	if (sMediaEngine)
	{
		std::unique_ptr<char> uBuf;

		if (rawData == nullptr)
		{
			/*
			Bitmap bmp(width, height, PixelFormat32bppARGB);
			drawBitmap(bmp);
			//SaveBitmap(bmp, L"pushVideoFrame.bmp");
			//return 0;
			Gdiplus::Rect rect{ 0, 0, width, height };

			uBuf.reset(new char[width * height * 4]);
			Gdiplus::BitmapData bmpData{};
			bmpData.Width = rect.Width;
			bmpData.Height = rect.Height;
			bmpData.PixelFormat = PixelFormat32bppARGB;
			bmpData.Stride = rect.Width * 4;
			bmpData.Scan0 = uBuf.get();
			bmp.LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeUserInputBuf | Gdiplus::ImageLockMode::ImageLockModeRead,
				PixelFormat32bppARGB, &bmpData);
			bmp.UnlockBits(&bmpData);
			rawData = uBuf.get();
			format = agora::media::base::VIDEO_PIXEL_RGBA;
			*/
		}

		agora::media::base::ExternalVideoFrame frame;
		frame.type = agora::media::base::ExternalVideoFrame::VIDEO_BUFFER_RAW_DATA;
		frame.format = format;
		frame.buffer = rawData;
		frame.stride = width;
		frame.height = height;
		frame.timestamp = GetTickCount64();

#if AGORA_SDK_VERSION >= 40000000
		return sMediaEngine->pushVideoFrame(&frame, 0);
#else
		RtcConnection connt(channelId, localUid);
		return sMediaEngine->pushVideoFrame(&frame, connt);
#endif
	}

	return -1;
}
#endif

AGORA_CAPI int registerAudioFrameObserver(IRtcEngine* rtcEngine)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sMediaEngine == nullptr)
	{
		if (rtcEngine->queryInterface(rtc::AGORA_IID_MEDIA_ENGINE, (void**)&sMediaEngine) != 0)
		{
			//failed
		}
	}

	if (sMediaEngine)
	{
		rtcEngine->setPlaybackAudioFrameParameters(48000, 2, RAW_AUDIO_FRAME_OP_MODE_READ_ONLY, 1024);
		rtcEngine->setPlaybackAudioFrameBeforeMixingParameters(2, 48000);
		rtcEngine->setMixedAudioFrameParameters(48000, 2, 1024);
		return sMediaEngine->registerAudioFrameObserver(&sAudioFrameObserver);
	}

	return -1;
}

AGORA_CAPI int registerVideoFrameObserver(IRtcEngine* rtcEngine)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sMediaEngine == nullptr)
	{
		if (rtcEngine->queryInterface(rtc::AGORA_IID_MEDIA_ENGINE, (void**)&sMediaEngine) != 0)
		{
			//failed
		}
	}

    if (sMediaEngine)
    {
        return sMediaEngine->registerVideoFrameObserver(&sVideoFrameObserver);
    }

    return -1;
}

AGORA_CAPI void saveCaptureVideoFrame(int save, unsigned int count)
{
    sVideoFrameObserver.saveCaptureVideoFrame(save, count);
}

AGORA_CAPI void resetCaptureVideoFrame()
{
    sVideoFrameObserver.resetCaptureVideoFrame();
}

AGORA_CAPI void saveSecondaryCaptureVideoFrame(int save, unsigned int count)
{
	sVideoFrameObserver.saveSecondaryCaptureVideoFrame(save, count);
}

AGORA_CAPI void resetSecondaryCaptureVideoFrame()
{
	sVideoFrameObserver.resetSecondaryCaptureVideoFrame();
}

AGORA_CAPI void saveRenderVideoFrame(int save, unsigned int count)
{
	sVideoFrameObserver.saveRenderVideoFrame(save, count);
}

AGORA_CAPI void resetRenderVideoFrame()
{
	sVideoFrameObserver.resetRenderVideoFrame();
}

AGORA_CAPI int setParameters(IRtcEngine* rtcEngine, const char* parameters)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sAgoraParameter == nullptr)
    {
        if (rtcEngine->queryInterface(rtc::AGORA_IID_PARAMETER_ENGINE, (void**)&sAgoraParameter) != 0)
        {
            //failed
        }
    }

    if (sAgoraParameter)
    {
        return sAgoraParameter->setParameters(parameters);
    }

    return -1;
}

AGORA_CAPI int getStringParameter(IRtcEngine* rtcEngine, const char* parameters, char* result, int size)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sAgoraParameter == nullptr)
    {
        if (rtcEngine->queryInterface(rtc::AGORA_IID_PARAMETER_ENGINE, (void**)&sAgoraParameter) != 0)
        {
            //failed
        }
    }

    if (sAgoraParameter)
    {
        agora::util::AString strResult;
        int ret = sAgoraParameter->getString(parameters, strResult);
        if (ret == 0)
        {
            strncpy_s(result, size, strResult->c_str(), size - 1);
        }
        return ret;
    }

    return -1;
}

AGORA_CAPI int getObjectParameter(IRtcEngine* rtcEngine, const char* parameters, char* result, int size)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sAgoraParameter == nullptr)
    {
        if (rtcEngine->queryInterface(rtc::AGORA_IID_PARAMETER_ENGINE, (void**)&sAgoraParameter) != 0)
        {
            //failed
        }
    }

    if (sAgoraParameter)
    {
        agora::util::AString strResult;
        int ret = sAgoraParameter->getObject(parameters, strResult);
        if (ret == 0)
        {
            strncpy_s(result, size, strResult->c_str(), size - 1);
        }
        return ret;
    }

    return -1;
}

AGORA_CAPI int getBoolParameter(IRtcEngine* rtcEngine, const char* parameters, int* value)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sAgoraParameter == nullptr)
    {
        if (rtcEngine->queryInterface(rtc::AGORA_IID_PARAMETER_ENGINE, (void**)&sAgoraParameter) != 0)
        {
            //failed
        }
    }

    if (sAgoraParameter)
    {
        bool result = false;
        int ret = sAgoraParameter->getBool(parameters, result);
        if (ret == 0)
        {
            *value = (int)result;
        }
        return ret;
    }

    return -1;
}

AGORA_CAPI int getIntParameter(IRtcEngine* rtcEngine, const char* parameters, int* value)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sAgoraParameter == nullptr)
    {
        if (rtcEngine->queryInterface(rtc::AGORA_IID_PARAMETER_ENGINE, (void**)&sAgoraParameter) != 0)
        {
            //failed
        }
    }

    if (sAgoraParameter)
    {
        return sAgoraParameter->getInt(parameters, *value);
    }

    return -1;
}

AGORA_CAPI int getNumberParameter(IRtcEngine* rtcEngine, const char* parameters, double* value)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sAgoraParameter == nullptr)
    {
        if (rtcEngine->queryInterface(rtc::AGORA_IID_PARAMETER_ENGINE, (void**)&sAgoraParameter) != 0)
        {
            //failed
        }
    }

    if (sAgoraParameter)
    {
        return sAgoraParameter->getNumber(parameters, *value);
    }

    return -1;
}

#define INIT_AUDIO_DEVICE_MANAGER		\
	if (rtcEngine == nullptr)			\
	{									\
		return -1;						\
	}									\
	if (sAudioDeviceManager == nullptr)																				\
	{																												\
		INTERFACE_ID_TYPE iid = rtc::AGORA_IID_AUDIO_DEVICE_MANAGER;												\
		if (rtcEngine->queryInterface(iid, (void**)&sAudioDeviceManager) != 0)										\
		{																											\
			spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);	\
		}																											\
	}																												\
	if (sAudioDeviceManager == nullptr)	\
	{									\
		return -1;						\
	}									\


AGORA_CAPI int enumeratePlaybackDevices(IRtcEngine* rtcEngine, char* szDeviceList, int size)
{
	INIT_AUDIO_DEVICE_MANAGER

	std::string deviceInfo;
	deviceInfo.reserve(size);
	IAudioDeviceCollection* deviceCollection = sAudioDeviceManager->enumeratePlaybackDevices();
	if (deviceCollection)
	{
		int count = deviceCollection->getCount();
		for (int n = 0; n < count; ++n)
		{
			char szDeviceName[MAX_DEVICE_ID_LENGTH]{};
			char szDeviceId[MAX_DEVICE_ID_LENGTH]{};
			int ret = deviceCollection->getDevice(n, szDeviceName, szDeviceId);
			if (ret != 0)
			{
				spdlog::info("CPP {} IAudioDeviceCollection {} getDevice returns {}", __FUNCTION__, fmt::ptr(deviceCollection), ret);
			}
			deviceInfo += szDeviceName;
			deviceInfo += "%%";
			deviceInfo += szDeviceId;
			if (n < count - 1)
			{
				deviceInfo += "||";
			}
		}
		deviceCollection->release();
		deviceCollection = nullptr;
	}
	else
	{
		spdlog::info("CPP {} IAudioDeviceManager {} enumeratePlaybackDevices returns null", __FUNCTION__, fmt::ptr(sAudioDeviceManager));
	}
	std::strncpy(szDeviceList, deviceInfo.c_str(), size);
	szDeviceList[size - 1] = '\0';
	return 0;
}

AGORA_CAPI int enumerateRecordingDevices(IRtcEngine* rtcEngine, char* szDeviceList, int size)
{
	INIT_AUDIO_DEVICE_MANAGER

	std::string deviceInfo;
	deviceInfo.reserve(size);
	IAudioDeviceCollection* deviceCollection = sAudioDeviceManager->enumerateRecordingDevices();
	if (deviceCollection)
	{
		int count = deviceCollection->getCount();
		for (int n = 0; n < count; ++n)
		{
			char szDeviceName[MAX_DEVICE_ID_LENGTH]{};
			char szDeviceId[MAX_DEVICE_ID_LENGTH]{};
			int ret = deviceCollection->getDevice(n, szDeviceName, szDeviceId);
			if (ret != 0)
			{
				spdlog::info("CPP {} IAudioDeviceCollection {} getDevice returns {}", __FUNCTION__, fmt::ptr(deviceCollection), ret);
			}
			deviceInfo += szDeviceName;
			deviceInfo += "%%";
			deviceInfo += szDeviceId;
			if (n < count - 1)
			{
				deviceInfo += "||";
			}
		}
		deviceCollection->release();
		deviceCollection = nullptr;
	}
	else
	{
		spdlog::info("CPP {} IAudioDeviceManager {} enumerateRecordingDevices returns null", __FUNCTION__, fmt::ptr(sAudioDeviceManager));
	}
	std::strncpy(szDeviceList, deviceInfo.c_str(), size);
	szDeviceList[size - 1] = '\0';
	return 0;
}

AGORA_CAPI int setPlaybackDevice(IRtcEngine* rtcEngine, const char* deviceId)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->setPlaybackDevice(deviceId);
}

AGORA_CAPI int getPlaybackDevice(IRtcEngine* rtcEngine, char* deviceId)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->getPlaybackDevice(deviceId);
}

AGORA_CAPI int setRecordingDevice(IRtcEngine* rtcEngine, const char* deviceId)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->setRecordingDevice(deviceId);
}

AGORA_CAPI int getRecordingDevice(IRtcEngine* rtcEngine, char* deviceId)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->getRecordingDevice(deviceId);
}

AGORA_CAPI int startPlaybackDeviceTest(IRtcEngine* rtcEngine, const char* testAudioFilePath)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->startPlaybackDeviceTest(testAudioFilePath);
}

AGORA_CAPI int stopPlaybackDeviceTest(IRtcEngine* rtcEngine)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->stopPlaybackDeviceTest();
}

AGORA_CAPI int startRecordingDeviceTest(IRtcEngine* rtcEngine, int indicationInterval)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->startRecordingDeviceTest(indicationInterval);
}

AGORA_CAPI int stopRecordingDeviceTest(IRtcEngine* rtcEngine)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->stopRecordingDeviceTest();
}

AGORA_CAPI int startAudioDeviceLoopbackTest(IRtcEngine* rtcEngine, int indicationInterval)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->startAudioDeviceLoopbackTest(indicationInterval);
}

AGORA_CAPI int stopAudioDeviceLoopbackTest(IRtcEngine* rtcEngine)
{
	INIT_AUDIO_DEVICE_MANAGER

	return sAudioDeviceManager->stopAudioDeviceLoopbackTest();
}

