/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCAppManager.h
 * Description : Interface between the different app specific views and features.
 *
 * Created     : 30/08/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCAPPMANAGER_H__
#define __CCAPPMANAGER_H__


#ifdef IOS
    #ifdef __OBJC__
        #include "CCGLView.h"
        #include "CCVideoView.h"
        #import "CCViewController.h"
        #import "CCARView.h"
        #import "CCWebView.h"
        #import "CCWebJS.h"
    #else
        #define CCGLView void
        #define CCVideoView void
        #define CCViewController void
        #define UIWindow void
        #define CCARView void
        #define CCWebView void
        #define CCWebJS void
    #endif
#else
    #include "CCGLView.h"
    #include "CCWebView.h"
    #include "CCWebJS.h"
    #ifdef QT
        #include "CCVideoView.h"
    #endif
#endif

class CCAppManager
{
public:
	CCAppManager();
    ~CCAppManager();

    static bool Startup();
    static void LaunchWindow();
    
    static void Shutdown();

    static void ResetSessionCallbacks();

public:
    static float TotalMemory();
    static bool OSVersionLessThan(const char *version);

    static void Pause();
    static void Resume();

    static float WindowWidth();
    static float WindowHeight();
    static bool IsPortrait();
    static bool SetDeviceOrientation(const float deviceOrientation);
    static float GetDeviceOrientation();
    static void SetIfNewOrientation(const float targetOrientation);
    static void SetDefaultOrientation(const char *orientation);
    static void SetOrientation(const float targetOrientation, const bool interpolate=true);
    static void ProjectOrientation(float &x, float &y);
    static void DetectOrientationNativeThread(const float delta);
    static void UpdateOrientation(const float delta);
    static const CCTarget<float>& GetOrientation();

    static void InAppPurchase(const char *code, const bool consumable, CCLambdaCallback *callback);
    static void InAppPurchaseSuccessful();

public:     static void AdvertsEnable(const char *adMobID);
public:     static void AdvertsDisable();
public:     static float AdvertsGetHeight();

public:     static void VideoViewStart(const char *file);
protected:  static void VideoViewStartNativeThread(const char *file);
public:     static void VideoViewStop();
protected:  static void VideoViewStopNativeThread();
public:     static float VideoViewGetPlaybackTime();
            static void VideoViewSeek(const float time);

public:     static void StartARView();
protected:  static void StartARViewNativeThread();
public:     static void StopARView();
protected:  static void StopARViewNativeThread();

public:     static void SetCameraActive(bool toggle);
    static bool IsCameraActive();

public:     static void WebBrowserOpen(const char *url);

public:     static void WebViewOpen(const char *url, const bool fullScreen, const bool below=false);
protected:  static void WebViewOpenNativeThread(const char *url, const bool fullScreen, const bool below);

public:     static void WebViewLoadedNativeThread(const char *url, const char *data);
protected:  static void WebViewLoaded(const char *url, const char *data);

public:     static void WebViewClose(const bool nativeThread=false);
protected:  static void WebViewCloseNativeThread();

public:     static bool WebViewIsLoaded();

public:     static void WebViewEval(const char *script, bool nativeThread=false);
protected:  static void WebViewEvalNativeThread(const char *script);
public:     static void WebViewClearCache();

public:     static void WebJSOpen(const char *url, const bool isFile=false, const char *htmlData=NULL, bool nativeThread=false);
protected:  static void WebJSOpenNativeThread(const char *url, const bool isFile, const char *htmlData);
public:     static void WebJSLoadedNativeThread(const char *url, const char *data);

public:     static void WebJSClose(bool nativeThread=false);
protected:  static void WebJSCloseNativeThread();

public:     static bool WebJSIsLoaded();

public:     static void WebJSEval(const char *script, const bool returnResult, bool nativeThread);
protected:  static void WebJSEvalNativeThread(const char *script, const bool returnResult);

public:     static void WebJSEvalResultNativeThread(const CCData &data, const bool returnResult);
public:     static bool WebJSIsEval();
            static void WebJSSetLastEvalTime(const float time);
            static float WebJSGetLastEvalTime();

public:     static void SendEmail(const char *address, const char *subject, const char *body, const char *attachmentFilename=NULL, const CCData *attachmentData=NULL);

public:     static void EnableControls(const bool toggle);
public:     static void KeyboardShow(const bool show);
protected:  static void KeyboardShowNativeThread(const bool show);

public:     static bool DetectMusic(CCLambdaCallback *callback);
public:     static void DetectMusicResult(const char *jsonString);
public:     static void SyncMusicEvent(const char *jsonString);
public:     static void PlayMusic(const char *id, const float seconds);
public:     static void StopMusic();
public:     static void SetMusicPosition(const float position);
public:     static void SetMusicVolume(const float volume);

public:     static bool DetectThinkGearDevice();

public:     static bool SelectImageFromGallery(CCLambdaCallback *callback);
public:     static void SelectImageFromGalleryResult(const char *filename);
public:     static void AddImageToGallery(const char *filePath);
public:     static void ShareImage(const char *filePath);

public:		static void Exit();
};


#endif // __CCAPPMANAGER_H__

