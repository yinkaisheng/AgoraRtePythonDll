#include <windows.h>
#include <gdiplus.h>    //do not define WIN32_LEAN_AND_MEAN
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

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
static IVideoDeviceManager* sVideoDeviceManager{};
static IMediaEngine* sMediaEngine{};
static AgoraVideoFrameObserver sVideoFrameObserver;
#if ((AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109) && IS_DEV_36200100) || AGORA_SDK_VERSION>=50000000
static SnapshotHandler sSnapshotHandler;
#endif

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

    if (sVideoDeviceManager)
    {
        sVideoDeviceManager->release();
        sVideoDeviceManager = nullptr;
    }

    rtcEngine->release(sync);
    spdlog::info("CPP {} {} released", __FUNCTION__, fmt::ptr(rtcEngine));
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
#if ((AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109) && IS_DEV_36200100) || AGORA_SDK_VERSION>=50000000
	sSnapshotHandler.setEventCallback(callback);
#endif
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
AGORA_CAPI int enableBrightnessCorrection(IRtcEngine* rtcEngine, int enable, BRIGHTNESS_CORRECTION_MODE mode, VIDEO_SOURCE_TYPE sourceType)
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

AGORA_CAPI int enableLocalTrapezoidCorrection(IRtcEngine* rtcEngine, int enabled, VIDEO_SOURCE_TYPE sourceType)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }
    return rtcEngine->enableLocalTrapezoidCorrection((bool)enabled, sourceType);
}

AGORA_CAPI int setLocalTrapezoidCorrectionOptions(IRtcEngine* rtcEngine, const char* jsonOptions, VIDEO_SOURCE_TYPE sourceType)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    TrapezoidCorrectionOptions options;
    trapezoidJsonToOptions(jsonOptions, options);

    return rtcEngine->setLocalTrapezoidCorrectionOptions(options, sourceType);
}

AGORA_CAPI int getLocalTrapezoidCorrectionOptions(IRtcEngine* rtcEngine, char* jsonOptions, int size, VIDEO_SOURCE_TYPE sourceType)
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

AGORA_CAPI int setCameraDeviceOrientation(IRtcEngine* rtcEngine, VIDEO_SOURCE_TYPE sourceType, VIDEO_ORIENTATION orientation)
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
    VIDEO_SOURCE_TYPE sourceType, int isScreenView, int setupMode)
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
	canvas.setupMode = (VIDEO_VIEW_SETUP_MODE)setupMode;	//VIDEO_VIEW_SETUP_MODE
	return rtcEngine->setupLocalVideo(canvas);
#elif (AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109)
	if (setupMode >= 0)
	{
		IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
		return rtcEngineEx->setupLocalVideoEx(canvas, (VIDEO_VIEW_SETUP_MODE)setupMode);
	}
	return rtcEngine->setupLocalVideo(canvas);
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
AGORA_CAPI int setLocalRenderMode(IRtcEngine* rtcEngine, RENDER_MODE_TYPE renderMode, VIDEO_MIRROR_MODE_TYPE mirrorMode, VIDEO_SOURCE_TYPE sourceType)
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
    RENDER_MODE_TYPE renderMode, VIDEO_SOURCE_TYPE sourceType, int setupMode, int isScreenView, unsigned int connectionId)
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
AGORA_CAPI int startPreview2(IRtcEngine* rtcEngine, VIDEO_SOURCE_TYPE type)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->startPreview(type);
}

AGORA_CAPI int stopPreview2(IRtcEngine* rtcEngine, VIDEO_SOURCE_TYPE type)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}
	return rtcEngine->stopPreview(type);
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
    CHANNEL_PROFILE_TYPE channelProfile,
    CLIENT_ROLE_TYPE clientRole,
    int autoSubscribeAudio,
	int autoSubscribeVideo,
	int publishAudioTrack,
    int publishCameraTrack,
	int publishSecondaryCameraTrack,
	int publishScreenTrack,
	int publishSecondaryScreenTrack,
	int publishCustomAudioTrack,
    int publishCustomVideoTrack)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

    ChannelMediaOptions options;
	options.channelProfile = channelProfile;
    options.clientRoleType = clientRole;

    //callee use -1 if not set
	if (autoSubscribeAudio == 0 || autoSubscribeAudio == 1)
	{
		options.autoSubscribeAudio = (bool)autoSubscribeAudio;
	}
	if (autoSubscribeVideo == 0 || autoSubscribeVideo == 1)
	{
		options.autoSubscribeVideo = (bool)autoSubscribeVideo;
	}
	if (publishAudioTrack == 0 || publishAudioTrack == 1)
	{
#if AGORA_SDK_VERSION >= 40000000
		options.publishMicrophoneTrack = (bool)publishAudioTrack;
#else
		options.publishAudioTrack = (bool)publishAudioTrack;
#endif
	}
    if (publishCameraTrack == 0 || publishCameraTrack == 1)
    {
        options.publishCameraTrack = (bool)publishCameraTrack;
    }
	if (publishSecondaryCameraTrack == 0 || publishSecondaryCameraTrack == 1)
	{
		options.publishSecondaryCameraTrack = (bool)publishSecondaryCameraTrack;
	}
	if (publishScreenTrack == 0 || publishScreenTrack == 1)
	{
		options.publishScreenTrack = (bool)publishScreenTrack;
	}
	if (publishSecondaryScreenTrack == 0 || publishSecondaryScreenTrack == 1)
	{
		options.publishSecondaryScreenTrack = (bool)publishSecondaryScreenTrack;
	}
	if (publishCustomAudioTrack == 0 || publishCustomAudioTrack == 1)
	{
		options.publishCustomAudioTrack = (bool)publishCustomAudioTrack;
	}
    if (publishCustomVideoTrack == 0 || publishCustomVideoTrack == 1)
    {
        options.publishCustomVideoTrack = (bool)publishCustomVideoTrack;
    }

	return rtcEngine->joinChannel(token, channelId, uid, options);
}

AGORA_CAPI int updateChannelMediaOptions(IRtcEngine* rtcEngine,
	CHANNEL_PROFILE_TYPE channelProfile,
	CLIENT_ROLE_TYPE clientRole,
	int autoSubscribeAudio,
	int autoSubscribeVideo,
	int publishAudioTrack,
	int publishCameraTrack,
	int publishSecondaryCameraTrack,
	int publishScreenTrack,
	int publishSecondaryScreenTrack,
	int publishCustomAudioTrack,
	int publishCustomVideoTrack)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	ChannelMediaOptions options;
	options.channelProfile = channelProfile;
	options.clientRoleType = clientRole;

	//callee use -1 if not set
	if (autoSubscribeAudio == 0 || autoSubscribeAudio == 1)
	{
		options.autoSubscribeAudio = (bool)autoSubscribeAudio;
	}
	if (autoSubscribeVideo == 0 || autoSubscribeVideo == 1)
	{
		options.autoSubscribeVideo = (bool)autoSubscribeVideo;
	}
	if (publishAudioTrack == 0 || publishAudioTrack == 1)
	{
#if AGORA_SDK_VERSION >= 40000000
		options.publishMicrophoneTrack = (bool)publishAudioTrack;
#else
		options.publishAudioTrack = (bool)publishAudioTrack;
#endif
	}
	if (publishCameraTrack == 0 || publishCameraTrack == 1)
	{
		options.publishCameraTrack = (bool)publishCameraTrack;
	}
	if (publishSecondaryCameraTrack == 0 || publishSecondaryCameraTrack == 1)
	{
		options.publishSecondaryCameraTrack = (bool)publishSecondaryCameraTrack;
	}
	if (publishScreenTrack == 0 || publishScreenTrack == 1)
	{
		options.publishScreenTrack = (bool)publishScreenTrack;
	}
	if (publishSecondaryScreenTrack == 0 || publishSecondaryScreenTrack == 1)
	{
		options.publishSecondaryScreenTrack = (bool)publishSecondaryScreenTrack;
	}
	if (publishCustomAudioTrack == 0 || publishCustomAudioTrack == 1)
	{
		options.publishCustomAudioTrack = (bool)publishCustomAudioTrack;
	}
	if (publishCustomVideoTrack == 0 || publishCustomVideoTrack == 1)
	{
		options.publishCustomVideoTrack = (bool)publishCustomVideoTrack;
	}

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
	const char* userAccount, const char* token, 
	CHANNEL_PROFILE_TYPE channelProfile,
	CLIENT_ROLE_TYPE clientRole,
	int autoSubscribeAudio,
	int autoSubscribeVideo,
	int publishAudioTrack,
	int publishCameraTrack,
	int publishSecondaryCameraTrack,
	int publishScreenTrack,
	int publishSecondaryScreenTrack,
	int publishCustomAudioTrack,
	int publishCustomVideoTrack)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	ChannelMediaOptions options;
	options.channelProfile = channelProfile;
	options.clientRoleType = clientRole;
	if (autoSubscribeAudio == 0 || autoSubscribeAudio == 1)
	{
		options.autoSubscribeAudio = (bool)autoSubscribeAudio;
	}
	if (autoSubscribeVideo == 0 || autoSubscribeVideo == 1)
	{
		options.autoSubscribeVideo = (bool)autoSubscribeVideo;
	}
	if (publishAudioTrack == 0 || publishAudioTrack == 1)
	{
#if AGORA_SDK_VERSION >= 40000000
		options.publishMicrophoneTrack = (bool)publishAudioTrack;
#else
		options.publishAudioTrack = (bool)publishAudioTrack;
#endif
	}
	if (publishCameraTrack == 0 || publishCameraTrack == 1)
	{
		options.publishCameraTrack = (bool)publishCameraTrack;
	}
	if (publishSecondaryCameraTrack == 0 || publishSecondaryCameraTrack == 1)
	{
		options.publishSecondaryCameraTrack = (bool)publishSecondaryCameraTrack;
	}
	if (publishScreenTrack == 0 || publishScreenTrack == 1)
	{
		options.publishScreenTrack = (bool)publishScreenTrack;
	}
	if (publishSecondaryScreenTrack == 0 || publishSecondaryScreenTrack == 1)
	{
		options.publishSecondaryScreenTrack = (bool)publishSecondaryScreenTrack;
	}
	if (publishCustomAudioTrack == 0 || publishCustomAudioTrack == 1)
	{
		options.publishCustomAudioTrack = (bool)publishCustomAudioTrack;
	}
	if (publishCustomVideoTrack == 0 || publishCustomVideoTrack == 1)
	{
		options.publishCustomVideoTrack = (bool)publishCustomVideoTrack;
	}

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
	RENDER_MODE_TYPE renderMode, const char* channelId, uid_t locaUid)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, locaUid);

	spdlog::info("CPP {} uid {} view {}", __FUNCTION__, uid, fmt::ptr(view));
	VideoCanvas canvas;
	canvas.mirrorMode = mirrorMode;
	canvas.renderMode = renderMode;
	canvas.uid = uid;
	canvas.view = view;

	return rtcEngineEx->setupRemoteVideoEx(canvas, connt);
}


AGORA_CAPI int joinChannelEx(IRtcEngine* rtcEngine, const char* channelId, uid_t uid, const char* token, AgoraEventHandler* eventHandler,
	CHANNEL_PROFILE_TYPE channelProfile,
	CLIENT_ROLE_TYPE clientRole,
	int autoSubscribeAudio,
	int autoSubscribeVideo,
	int publishAudioTrack,
	int publishCameraTrack,
	int publishSecondaryCameraTrack,
	int publishScreenTrack,
	int publishSecondaryScreenTrack,
	int publishCustomAudioTrack,
	int publishCustomVideoTrack)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

    IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, uid);

	ChannelMediaOptions options;
	options.channelProfile = channelProfile;
	options.clientRoleType = clientRole;
	if (autoSubscribeAudio == 0 || autoSubscribeAudio == 1)
	{
		options.autoSubscribeAudio = (bool)autoSubscribeAudio;
	}
	if (autoSubscribeVideo == 0 || autoSubscribeVideo == 1)
	{
		options.autoSubscribeVideo = (bool)autoSubscribeVideo;
	}
	if (publishAudioTrack == 0 || publishAudioTrack == 1)
	{
#if AGORA_SDK_VERSION >= 40000000
		options.publishMicrophoneTrack = (bool)publishAudioTrack;
#else
		options.publishAudioTrack = (bool)publishAudioTrack;
#endif
	}
	if (publishCameraTrack == 0 || publishCameraTrack == 1)
	{
		options.publishCameraTrack = (bool)publishCameraTrack;
	}
	if (publishSecondaryCameraTrack == 0 || publishSecondaryCameraTrack == 1)
	{
		options.publishSecondaryCameraTrack = (bool)publishSecondaryCameraTrack;
	}
	if (publishScreenTrack == 0 || publishScreenTrack == 1)
	{
		options.publishScreenTrack = (bool)publishScreenTrack;
	}
	if (publishSecondaryScreenTrack == 0 || publishSecondaryScreenTrack == 1)
	{
		options.publishSecondaryScreenTrack = (bool)publishSecondaryScreenTrack;
	}
	if (publishCustomAudioTrack == 0 || publishCustomAudioTrack == 1)
	{
		options.publishCustomAudioTrack = (bool)publishCustomAudioTrack;
	}
	if (publishCustomVideoTrack == 0 || publishCustomVideoTrack == 1)
	{
		options.publishCustomVideoTrack = (bool)publishCustomVideoTrack;
	}

	return rtcEngineEx->joinChannelEx(token, connt, options, eventHandler);
}

AGORA_CAPI int updateChannelMediaOptionsEx(IRtcEngine* rtcEngine, const char* channelId, uid_t uid,
	CHANNEL_PROFILE_TYPE channelProfile,
	CLIENT_ROLE_TYPE clientRole,
	int autoSubscribeAudio,
	int autoSubscribeVideo,
	int publishAudioTrack,
	int publishCameraTrack,
	int publishSecondaryCameraTrack,
	int publishScreenTrack,
	int publishSecondaryScreenTrack,
	int publishCustomAudioTrack,
	int publishCustomVideoTrack)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	RtcConnection connt(channelId, uid);

	ChannelMediaOptions options;
	options.channelProfile = channelProfile;
	options.clientRoleType = clientRole;
	if (autoSubscribeAudio == 0 || autoSubscribeAudio == 1)
	{
		options.autoSubscribeAudio = (bool)autoSubscribeAudio;
	}
	if (autoSubscribeVideo == 0 || autoSubscribeVideo == 1)
	{
		options.autoSubscribeVideo = (bool)autoSubscribeVideo;
	}
	if (publishAudioTrack == 0 || publishAudioTrack == 1)
	{
#if AGORA_SDK_VERSION >= 40000000
		options.publishMicrophoneTrack = (bool)publishAudioTrack;
#else
		options.publishAudioTrack = (bool)publishAudioTrack;
#endif
	}
	if (publishCameraTrack == 0 || publishCameraTrack == 1)
	{
		options.publishCameraTrack = (bool)publishCameraTrack;
	}
	if (publishSecondaryCameraTrack == 0 || publishSecondaryCameraTrack == 1)
	{
		options.publishSecondaryCameraTrack = (bool)publishSecondaryCameraTrack;
	}
	if (publishScreenTrack == 0 || publishScreenTrack == 1)
	{
		options.publishScreenTrack = (bool)publishScreenTrack;
	}
	if (publishSecondaryScreenTrack == 0 || publishSecondaryScreenTrack == 1)
	{
		options.publishSecondaryScreenTrack = (bool)publishSecondaryScreenTrack;
	}
	if (publishCustomAudioTrack == 0 || publishCustomAudioTrack == 1)
	{
		options.publishCustomAudioTrack = (bool)publishCustomAudioTrack;
	}
	if (publishCustomVideoTrack == 0 || publishCustomVideoTrack == 1)
	{
		options.publishCustomVideoTrack = (bool)publishCustomVideoTrack;
	}

	return rtcEngineEx->updateChannelMediaOptionsEx(options, connt);
}


AGORA_CAPI int joinChannelWithUserAccountEx(IRtcEngine* rtcEngine, const char* channelId, const char* userAccount, const char* token, AgoraEventHandler* eventHandler,
	CHANNEL_PROFILE_TYPE channelProfile,
	CLIENT_ROLE_TYPE clientRole,
	int autoSubscribeAudio,
	int autoSubscribeVideo,
	int publishAudioTrack,
	int publishCameraTrack,
	int publishSecondaryCameraTrack,
	int publishScreenTrack,
	int publishSecondaryScreenTrack,
	int publishCustomVideoTrack)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	IRtcEngineEx* rtcEngineEx = static_cast<IRtcEngineEx*>(rtcEngine);
	ChannelMediaOptions options;
	options.channelProfile = channelProfile;
	options.clientRoleType = clientRole;
	if (autoSubscribeAudio == 0 || autoSubscribeAudio == 1)
	{
		options.autoSubscribeAudio = (bool)autoSubscribeAudio;
	}
	if (autoSubscribeVideo == 0 || autoSubscribeVideo == 1)
	{
		options.autoSubscribeVideo = (bool)autoSubscribeVideo;
	}
	if (publishAudioTrack == 0 || publishAudioTrack == 1)
	{
#if AGORA_SDK_VERSION >= 40000000
		options.publishMicrophoneTrack = (bool)publishAudioTrack;
#else
		options.publishAudioTrack = (bool)publishAudioTrack;
#endif
	}
	if (publishCameraTrack == 0 || publishCameraTrack == 1)
	{
		options.publishCameraTrack = (bool)publishCameraTrack;
	}
	if (publishSecondaryCameraTrack == 0 || publishSecondaryCameraTrack == 1)
	{
		options.publishSecondaryCameraTrack = (bool)publishSecondaryCameraTrack;
	}
	if (publishScreenTrack == 0 || publishScreenTrack == 1)
	{
		options.publishScreenTrack = (bool)publishScreenTrack;
	}
	if (publishSecondaryScreenTrack == 0 || publishSecondaryScreenTrack == 1)
	{
		options.publishSecondaryScreenTrack = (bool)publishSecondaryScreenTrack;
	}
	if (publishCustomVideoTrack == 0 || publishCustomVideoTrack == 1)
	{
		options.publishCustomVideoTrack = (bool)publishCustomVideoTrack;
	}

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

#if (AGORA_SDK_VERSION >= 36200100 && AGORA_SDK_VERSION <= 36200109) && IS_DEV_36200100
AGORA_CAPI int takeSnapshot(IRtcEngine* rtcEngine, const char* channel, uid_t uid, const char* filePath,
	float left, float top, float right, float bottom, AgoraEventHandler* eventHandler)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	media::SnapShotConfig ssConfig;
	ssConfig.channel = channel;
	ssConfig.uid = uid;
	ssConfig.filePath = filePath;
	ssConfig.left = left;
	ssConfig.top = top;
	ssConfig.right = right;
	ssConfig.bottom = bottom;
	media::ISnapshotCallback* callback = &sSnapshotHandler;
	callback = nullptr;
	return rtcEngine->takeSnapshot(ssConfig, callback);
}

AGORA_CAPI int startServerSuperResolution(IRtcEngine* rtcEngine, const char* token, const char* imagePath, float scale, int timeoutSeconds)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	return rtcEngine->startServerSuperResolution(token, imagePath, scale, timeoutSeconds);
}

#endif

#if AGORA_SDK_VERSION>=50000000
AGORA_CAPI int takeSnapshot(IRtcEngine* rtcEngine, const char* channel, uid_t uid, const char* filePath, AgoraEventHandler* eventHandler)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	media::SnapShotConfig ssConfig;
	ssConfig.channel = channel;
	ssConfig.uid = uid;
	ssConfig.filePath = filePath;
	media::ISnapshotCallback* callback = &sSnapshotHandler;
	callback = nullptr;
	return rtcEngine->takeSnapshot(ssConfig, callback);
}

AGORA_CAPI int setContentInspect(IRtcEngine* rtcEngine, int enable, int cloudWork)
{
	// agora::media::CONTENT_INSPECT_DEVICE_TYPE deviceWorkType
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	media::ContentInspectConfig config;
	config.enable = enable;
	config.CloudWork = cloudWork;
	config.DeviceWork = !config.CloudWork;
	//config.DeviceworkType = deviceWorkType;
	config.moduleCount = 1;
	config.modules[0].frequency = 2;
	config.modules[0].type = agora::media::CONTENT_INSPECT_TYPE::CONTENT_INSPECT_MODERATION;
	return rtcEngine->SetContentInspect(config);
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

AGORA_CAPI int getVideoDevices(IRtcEngine* rtcEngine, char* szDeviceList, int size)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sVideoDeviceManager == nullptr)
    {
        INTERFACE_ID_TYPE iid = rtc::AGORA_IID_VIDEO_DEVICE_MANAGER;
        if (rtcEngine->queryInterface(rtc::AGORA_IID_VIDEO_DEVICE_MANAGER, (void**)&sVideoDeviceManager) != 0)
        {
            //failed
            spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);
        }
    }

    if (sVideoDeviceManager == nullptr)
    {
        spdlog::info("CPP {} rtcEngine {} VideoDeviceManager is null", __FUNCTION__, fmt::ptr(rtcEngine));
        return -1;
    }

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

AGORA_CAPI int getVideoDeviceId(IRtcEngine* rtcEngine, char* szDeviceId, int size)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sVideoDeviceManager == nullptr)
    {
        INTERFACE_ID_TYPE iid = rtc::AGORA_IID_VIDEO_DEVICE_MANAGER;
        if (rtcEngine->queryInterface(rtc::AGORA_IID_VIDEO_DEVICE_MANAGER, (void**)&sVideoDeviceManager) != 0)
        {
            //failed
            spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);
        }
    }

    if (sVideoDeviceManager)
    {
        char szDeviceId[MAX_DEVICE_ID_LENGTH]{};
        int ret = sVideoDeviceManager->getDevice(szDeviceId);
        if (ret == 0)
        {
            std::strncpy(szDeviceId, szDeviceId, size - 1);
        }
        else
        {
            spdlog::info("CPP {} IVideoDeviceManager {} getDevice returns {}", __FUNCTION__, fmt::ptr(sVideoDeviceManager), ret);
        }
        return ret;
    }

    return -1;
}

AGORA_CAPI int setVideoDeviceId(IRtcEngine* rtcEngine, const char* deviceId)
{
    if (rtcEngine == nullptr)
    {
        return -1;
    }

    if (sVideoDeviceManager == nullptr)
    {
        INTERFACE_ID_TYPE iid = rtc::AGORA_IID_VIDEO_DEVICE_MANAGER;
        if (rtcEngine->queryInterface(rtc::AGORA_IID_VIDEO_DEVICE_MANAGER, (void**)&sVideoDeviceManager) != 0)
        {
            //failed
            spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);
        }
    }

    if (sVideoDeviceManager)
    {
        return sVideoDeviceManager->setDevice(deviceId);
    }

    return -1;
}

AGORA_CAPI int getVideoDeviceNumberOfCapabilities(IRtcEngine* rtcEngine, const char* szDeviceId)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sVideoDeviceManager == nullptr)
	{
		INTERFACE_ID_TYPE iid = rtc::AGORA_IID_VIDEO_DEVICE_MANAGER;
		if (rtcEngine->queryInterface(rtc::AGORA_IID_VIDEO_DEVICE_MANAGER, (void**)&sVideoDeviceManager) != 0)
		{
			//failed
			spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);
		}
	}

	if (sVideoDeviceManager == nullptr)
	{
		spdlog::info("CPP {} rtcEngine {} VideoDeviceManager is null", __FUNCTION__, fmt::ptr(rtcEngine));
		return -1;
	}

#if AGORA_SDK_VERSION == 37200100 || AGORA_SDK_VERSION >= 40000000
	return sVideoDeviceManager->numberOfCapabilities(szDeviceId);
#else
	return 0;
#endif
}

AGORA_CAPI int getVideoDeviceCapabilities(IRtcEngine* rtcEngine, const char* szDeviceId, char* szCapabilities, int size)
{
	if (rtcEngine == nullptr)
	{
		return -1;
	}

	if (sVideoDeviceManager == nullptr)
	{
		INTERFACE_ID_TYPE iid = rtc::AGORA_IID_VIDEO_DEVICE_MANAGER;
		if (rtcEngine->queryInterface(rtc::AGORA_IID_VIDEO_DEVICE_MANAGER, (void**)&sVideoDeviceManager) != 0)
		{
			//failed
			spdlog::info("CPP {} rtcEngine {} queryInterface {} failed", __FUNCTION__, fmt::ptr(rtcEngine), iid);
		}
	}

	if (sVideoDeviceManager == nullptr)
	{
		spdlog::info("CPP {} rtcEngine {} VideoDeviceManager is null", __FUNCTION__, fmt::ptr(rtcEngine));
		return -1;
	}

#if AGORA_SDK_VERSION == 37201100 || AGORA_SDK_VERSION >= 40000000
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



