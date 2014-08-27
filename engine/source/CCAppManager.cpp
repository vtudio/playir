/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCAppManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCAppManager.h"

#ifdef QT
#include "CCMainWindow.h"
#endif

#include "CCFileManager.h"

#ifdef IOS
#include "TouchCapturingWindow.h"
#include "DMActivityInstagram.h"
#include <mach/mach.h>
#include <mach/mach_host.h>
#endif


CCGLView *gView = NULL;
CCAppEngine *gEngine = NULL;
#ifdef IOS
CCViewController *gViewController = NULL;
#endif


static bool AppStarted = false;

// Views
static CCGLView *GLView = NULL;
static CCWebView *WebView = NULL;
static CCWebJS *WebJS = NULL;

#if defined IOS || defined QT
static CCVideoView *VideoView = NULL;
#else
static void *VideoView = NULL;
#endif

#ifdef IOS
static CCARView *ARView = NULL;
static UIWindow *Window;
#else
static void *ARView = NULL;
#endif

static float DeviceOrientation = 0.0f;
static CCTarget<float> Orientation;
enum OrientationStateEnum
{
    Orientation_Set,
    Orientation_Updating,
    Orientation_Setting
};
static OrientationStateEnum OrientationState = Orientation_Set;

static bool OpaqueOpenGLRendering;
static bool CameraActive;
static int WebJSEvalCalls;
static float WebJSLastEvalTime;


bool CCAppManager::Startup()
{
	if( !AppStarted )
	{
        AppStarted = true;

        OpaqueOpenGLRendering = true;
        CameraActive = false;
        WebJSEvalCalls = 0;
        WebJSLastEvalTime = 0.0f;

#ifdef IOS

        // Create a full screen window
        CGRect rect = [[UIScreen mainScreen] bounds];

        CGRect statusBarRect = [[UIApplication sharedApplication] statusBarFrame];
        if( [[[UIDevice currentDevice] systemVersion] floatValue] >= 7 )
        {
            [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleBlackOpaque];
            rect.size.height -= statusBarRect.size.height;
            rect.origin.y += statusBarRect.size.height;
        }
        else
        {
            rect.size.height -= statusBarRect.size.height;
            rect.origin.y += statusBarRect.size.height * 0.5f;
        }

        Window = [[TouchCapturingWindow alloc] initWithFrame:rect];
        gViewController = [[CCViewController alloc] initWithNibName:NULL bundle:NULL];

        // Create OpenGL view and add to window
        GLView = [[CCGLView alloc] initWithFrame:rect];
//        [(TouchCapturingWindow*)Window addViewForTouchPriority:GLView];

        [Window setRootViewController:gViewController];

        // Initialise our status bar orientation so the keyboard launches in the correct orientation.
//        [[UIApplication sharedApplication] setStatusBarOrientation:[gViewController getInterfaceOrientation] animated:NO];
//        [[UIApplication sharedApplication] setStatusBarOrientation:UIInterfaceOrientationPortrait];

        gEngine = new CCAppEngine();
        [GLView setup];

#elif defined QT

        //QGraphicsScene scene;
        //scene.setSceneRect( QRectF( 0, 0, 300, 300 ) );

        //GLView = new CCGLView( NULL );
        //CCGraphicsView *graphicsView = new CCGraphicsView( &scene, CCMainWindow::Get() );
        //CCMainWindow::AddChild( graphicsView );
        //graphicsView->setViewport( glView );

        GLView = new CCGLView( CCMainWindow::Get() );
        gEngine = new CCAppEngine();
        CCMainWindow::AddChild( GLView );

#elif defined ANDROID
        
        // Create our game engine system
        GLView = new CCGLView();
        
        // Engine is set up after the render buffer size is sent to the GLView
        //gEngine = new CCAppEngine();
        
#endif
		return true;
	}
	return false;
}


void CCAppManager::LaunchWindow()
{
#ifdef IOS

    [Window makeKeyAndVisible];

#endif
}


void CCAppManager::Shutdown()
{
    if( AppStarted )
    {
        gEngine->running = false;

        // Qt isn't multi-threaded yet, on Android this get's called from the rendering thread.
#ifndef IOS
        gEngine->engineThreadRunning = false;
#endif

        while( gEngine->engineThreadRunning )
        {
            usleep( 0 );
        }

#ifdef IOS

        [GLView shutdown];
        
#elif defined QT
        
        GLView->shutdown();

#endif


    if( VideoView != NULL )
    {
        VideoViewStop();
    }

#ifdef IOS

        if( ARView != NULL )
        {
            StopARView();
        }

        CCExternalAccessoryManager::Stop();

        [GLView release];
        GLView = NULL;
        [gViewController release];
        gViewController = NULL;
        [Window release];
        Window = NULL;
        
#else
        
        delete GLView;

#endif

        GLView = NULL;
        
        AppStarted = false;
    }
}


static CCLambdaCallback *inAppPurchaseCallback = NULL;
static CCLambdaCallback *detectMusicCallback = NULL;
static CCLambdaCallback *selectImageFromGalleryCallback = NULL;

void CCAppManager::ResetSessionCallbacks()
{
    DELETE_POINTER( inAppPurchaseCallback );
    DELETE_POINTER( detectMusicCallback );
    DELETE_POINTER( selectImageFromGalleryCallback );
}


float CCAppManager::TotalMemory()
{
    static float totalMemory = -1.0f;

    if( totalMemory == -1.0f )
    {
#ifdef IOS

		mach_port_t host_port;
		mach_msg_type_number_t host_size;
		vm_size_t pagesize;

		host_port = mach_host_self();
		host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
		host_page_size(host_port, &pagesize);

		vm_statistics_data_t vm_stat;

		if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS)
		{
			NSLOG(@"Failed to fetch vm statistics");
			totalMemory = 128.0f;
			return totalMemory;
		}

		/* Stats in bytes */
		natural_t mem_used = (vm_stat.active_count +
							  vm_stat.inactive_count +
							  vm_stat.wire_count) * pagesize;
		natural_t mem_free = vm_stat.free_count * pagesize;
		natural_t mem_total = mem_used + mem_free;

		totalMemory = mem_total / 1024.0f / 1024.0f;

#elif defined ANDROID

    	totalMemory = CCJNI::TotalMemory();

#endif
	}

    return totalMemory;
}


bool CCAppManager::OSVersionLessThan(const char *version)
{
#ifdef IOS

#define SYSTEM_VERSION_LESS_THAN(v)                 ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedAscending)
#define SYSTEM_VERSION_LESS_THAN_OR_EQUAL_TO(v)     ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedDescending)

    if( SYSTEM_VERSION_LESS_THAN( @"7.0" ) )
    {
        return true;
    }

#endif

    return false;
}


void CCAppManager::Pause()
{
    if( gEngine != NULL )
    {
        gEngine->pause();
    }
}


void CCAppManager::Resume()
{
    if( gEngine != NULL )
    {
        gEngine->resume();
    }
}


float CCAppManager::WindowWidth()
{
    float width = gRenderer->frameBufferManager.getWidth( -1 );
#ifdef IOS
    width = gView.frame.size.width * gView.contentScaleFactor;
#endif
    return width;
}


float CCAppManager::WindowHeight()
{
    float height = gRenderer->frameBufferManager.getHeight( -1 );
#ifdef IOS
    height = gView.frame.size.height * gView.contentScaleFactor;
#endif
    return height;
}


bool CCAppManager::IsPortrait()
{
#ifdef ANDROID

	// Android uses OS for rotation, which recreates our frame buffer
    const float frameBufferWidth = gRenderer->frameBufferManager.getWidth( -1 );
    const float frameBufferHeight = gRenderer->frameBufferManager.getHeight( -1 );
    if( frameBufferHeight > frameBufferWidth )
    {
    	return true;
    }
    return false;

#else

    // Other platforms run in portrait mode and rotate manually
    return Orientation.target == 0.0f || Orientation.target == 180.0f;

#endif
}


bool CCAppManager::SetDeviceOrientation(const float deviceOrientation)
{
	DeviceOrientation = deviceOrientation;
	if( gEngine != NULL )
	{
		if( !gEngine->isOrientationSupported( deviceOrientation ) )
		{
            return false;
		}
	}
	return true;
}


float CCAppManager::GetDeviceOrientation()
{
    return DeviceOrientation;
}


void CCAppManager::SetIfNewOrientation(const float targetOrientation)
{
    if( Orientation.current != targetOrientation )
    {
        SetOrientation( targetOrientation, true );
    }
}


void CCAppManager::SetDefaultOrientation(const char *orientation)
{
	if( CCText::Equals( orientation, "landscape" ) )
	{
		if( CCAppManager::IsPortrait() )
		{
			CCAppManager::SetOrientation( 270.0f );
		}
	}
	else if( CCText::Equals( orientation, "portrait" ) )
	{
		if( !CCAppManager::IsPortrait() )
		{
			CCAppManager::SetOrientation( 0.0f );
		}
	}
	else
	{
		if( !CCAppManager::IsPortrait() )
		{
			CCAppManager::SetOrientation( 0.0f );
		}
	}
}


void CCAppManager::SetOrientation(const float targetOrientation, const bool interpolate)
{
#ifdef IOS

    [gViewController setOrientation:targetOrientation interpolate:interpolate];

#elif defined ANDROID

    CCJNI::SetAppOrientation( targetOrientation );
    // Android Java handels rotation and signals an update with recreation of render buffer.
    return;

#endif

    if( Orientation.target != targetOrientation )
    {
        Orientation.target = targetOrientation;
        //if( orientation.current != orientation.target )
        {
            OrientationState = interpolate ? Orientation_Updating : Orientation_Setting;

            // Update our controls
            CCLAMBDA_RUN_ENGINETHREAD({
                if( gEngine != NULL )
                {
                    gEngine->touchUpdateMovementThreasholds();
                }
            });
        }
    }
}


void CCAppManager::ProjectOrientation(float &x, float &y)
{
    if( Orientation.target == 270.0f )
    {
        CCFloatSwap( x, y );
        x = 1.0f - x;
        y = 1.0f - y;
    }
    else if( Orientation.target == 90.0f )
    {
        CCFloatSwap( x, y );
    }
    else if( Orientation.target == 180.0f )
    {
        x = 1.0f - x;
    }
    else
    {
        y = 1.0f - y;
    }
}


void CCAppManager::DetectOrientationNativeThread(const float delta)
{
#ifdef IOS
    [gViewController detectOrientationUpdate:delta];
#endif
}


void CCAppManager::UpdateOrientation(const float delta)
{
    if( OrientationState != Orientation_Set )
    {
        gEngine->resize();

        // Immediate update?
        if( OrientationState == Orientation_Setting )
        {
            Orientation.current = Orientation.target - CC_SMALLFLOAT;
        }
        OrientationState = Orientation_Set;
        return;
    }

#ifndef ANDROID
    if( Orientation.current != Orientation.target )
    {
        if( CCToRotation( Orientation.current, Orientation.target, delta * 360.0f ) )
        {
            gEngine->updatedOrientation();
        }
    }
#endif
}


const CCTarget<float>& CCAppManager::GetOrientation()
{
    return Orientation;
}


void CCAppManager::InAppPurchase(const char *itemCode, const bool consumable, CCLambdaCallback *callback)
{
#ifdef IOS


    CCLAMBDA_4( ThreadCallback, CCEngine, engine, CCText, itemCode, bool, consumable, CCLambdaCallback*, callback, {
        [gView->inAppPurchaseManager buyItem:itemCode.buffer consumable:consumable callback:callback];
    });
    gEngine->engineToNativeThread( new ThreadCallback( gEngine, itemCode, consumable, callback ) );

#elif defined ANDROID || defined WP8

	if( inAppPurchaseCallback != NULL )
	{
		delete inAppPurchaseCallback;
	}
	inAppPurchaseCallback = callback;

#ifdef ANDROID

    CCJNI::BillingRequestPurchase( itemCode, consumable );

#elif defined WP8

	// Consumable specified at store level
	csActionStack.add( new CSAction( "CCAppManager::InAppPurchase, ", itemCode ) );

#endif

#endif
}


void CCAppManager::InAppPurchaseSuccessful()
{
	if( inAppPurchaseCallback != NULL )
	{
		gEngine->nativeToEngineThread( inAppPurchaseCallback );
		inAppPurchaseCallback = NULL;
	}
}


void CCAppManager::AdvertsEnable(const char *advertID)
{
    class ThreadCallback : public CCLambdaCallback
    {
    public:
        ThreadCallback(const char *advertID)
        {
            this->advertID = advertID;
        }
    protected:
        void run()
        {
#ifdef IOS

            [gViewController advertsEnable:advertID.buffer];

#elif ANDROID

            CCJNI::AdvertsEnable( advertID.buffer );

#endif
        }
    private:
        CCText advertID;
    };
    gEngine->engineToNativeThread( new ThreadCallback( advertID ) );
}


void CCAppManager::AdvertsDisable()
{
    class ThreadCallback : public CCLambdaCallback
    {
    protected:
        void run()
        {
#ifdef IOS

            [gViewController advertsDisable];

#elif ANDROID

            CCJNI::AdvertsDisable();

#endif
        }
    };
    gEngine->engineToNativeThread( new ThreadCallback() );
}


float CCAppManager::AdvertsGetHeight()
{
#ifdef IOS

    return ( 50.0f / gRenderer->getScreenSize().height );

#elif ANDROID

    const float bannerWidth = 320.0f;
    const float bannerHeight = 50.0f;
    const float screenWidth = gRenderer->getScreenSize().width;
    const float screenHeight = gRenderer->getScreenSize().height;

    float scale = screenWidth / bannerWidth;
    float scaledHeight = bannerHeight * scale;
    return scaledHeight / screenHeight;

#endif

    return 0.1f;
}


void CCAppManager::VideoViewStart(const char *file)
{
    class ThreadCallback : public CCLambdaCallback
    {
    public:
        ThreadCallback(const char *file)
        {
            this->file = file;
        }
    protected:
        void run()
        {
            CCAppManager::VideoViewStartNativeThread( this->file.buffer );
        }
    private:
        CCText file;
    };
    gEngine->engineToNativeThread( new ThreadCallback( file ) );
}


void CCAppManager::VideoViewStartNativeThread(const char *file)
{
#ifdef QT
    return;
#endif

    CCText fullFilePath;
    if( CCFileManager::DoesFileExist( file, Resource_Packaged ) )
    {
        CCFileManager::GetFilePath( fullFilePath, file, Resource_Packaged );
    }
    else
    {
        CCFileManager::GetFilePath( fullFilePath, file, Resource_Cached );
    }

    CCASSERT( VideoView == NULL );

#ifdef IOS

    CGRect rect = [[UIScreen mainScreen] bounds];
    VideoView = [[CCVideoView alloc] initWithFrame:rect];
    [Window insertSubview:VideoView belowSubview:GLView];

    [VideoView playVideo:fullFilePath.buffer];

#elif defined ANDROID

    CCJNI::VideoViewStart( fullFilePath.buffer );
    VideoView = (void*)0x1;

#elif defined QT

    VideoView = new CCVideoView( CCMainWindow::Get() );
    CCMainWindow::AddChild( VideoView );
    VideoView->playVideo( fullFilePath.buffer );
    VideoView->lower();

#endif
}


void CCAppManager::VideoViewStop()
{
    CCLAMBDA_UNSAFE( ThreadCallback, CCAppManager::VideoViewStopNativeThread(); );
    gEngine->engineToNativeThread( new ThreadCallback() );
}


void CCAppManager::VideoViewStopNativeThread()
{
    CCASSERT( VideoView != NULL );

#ifdef IOS

    [VideoView stop];
    [VideoView remove];
    const int count = [VideoView retainCount];
    for( int i=0; i<count; ++i )
    {
        [VideoView release];
    }

#elif defined ANDROID

    CCJNI::VideoViewStop();

#endif

    VideoView = NULL;
}


float CCAppManager::VideoViewGetPlaybackTime()
{
#ifdef IOS

    if( VideoView != NULL )
    {
        return [VideoView getPlaybackTime];
    }

#elif defined ANDROID

    return CCJNI::VideoGetPlaybackTime();
    
#endif

    return 0.0f;
}


void CCAppManager::VideoViewSeek(const float time)
{
#ifdef IOS

    if( VideoView != NULL )
    {
        [VideoView seek:time];
    }

#elif defined ANDROID

    CCJNI::VideoViewSeek( time );
    
#endif
}


void CCAppManager::StartARView()
{
    CCLAMBDA_UNSAFE( ThreadCallback, CCAppManager::StartARViewNativeThread(); );
    gEngine->engineToNativeThread( new ThreadCallback() );
}


void CCAppManager::StartARViewNativeThread()
{
#ifdef IOS

    CCASSERT( ARView == NULL );

    CGRect rect = [[UIScreen mainScreen] bounds];
    ARView = [[CCARView alloc] initWithFrame:rect];
    [Window insertSubview:ARView belowSubview:GLView];

#endif
}


void CCAppManager::StopARView()
{
    CCLAMBDA_UNSAFE( ThreadCallback, CCAppManager::StopARViewNativeThread(); );
    gEngine->engineToNativeThread( new ThreadCallback() );
}


void CCAppManager::StopARViewNativeThread()
{
#ifdef IOS

    CCASSERT( ARView != NULL );
    [ARView remove];
    //const int count = [videoView retainCount];
    [ARView release];
    ARView = NULL;

#endif
}


void CCAppManager::SetCameraActive(bool toggle)
{
    CameraActive = toggle;
}


bool CCAppManager::IsCameraActive()
{
    return CameraActive;
}


void CCAppManager::WebBrowserOpen(const char *url)
{
    class ThreadCallback : public CCLambdaCallback
    {
    public:
        ThreadCallback(const char *url)
        {
            this->url = url;
        }
    protected:
        void run()
        {
#ifdef IOS

            NSString *urlString = [NSString stringWithFormat:@"%s", url.buffer];
            NSURL *url = [NSURL URLWithString:urlString];
            [[UIApplication sharedApplication] openURL:url];

#elif defined ANDROID

            CCJNI::WebBrowserOpen( url.buffer );

#elif defined QT

            QDesktopServices::openUrl( QUrl( url.buffer ) );

#elif defined WP8

			csActionStack.add( new CSAction( "CCAppManager::WebBrowserOpen, ", url.buffer ) );

#elif defined WIN8

			auto uri = ref new Windows::Foundation::Uri( GetString( url.buffer ) );

			// Set the option to show a warning
			auto launchOptions = ref new Windows::System::LauncherOptions();
			launchOptions->TreatAsUntrusted = true;

			// Launch the URI with a warning prompt
			Windows::System::Launcher::LaunchUriAsync( uri, launchOptions );

#endif
        }
    private:
        CCText url;
    };
    gEngine->engineToNativeThread( new ThreadCallback( url ) );
}


// WebView
void CCAppManager::WebViewOpen(const char *url, const bool fullScreen, const bool below)
{
    class ThreadCallback : public CCLambdaCallback
    {
    public:
        ThreadCallback(const char *url, const bool fullScreen, const bool below)
        {
            this->url = url;
            this->fullScreen = fullScreen;
            this->below = below;
        }
    protected:
        void run()
        {
            CCAppManager::WebViewOpenNativeThread( url.buffer, fullScreen, below );
        }
    private:
        CCText url;
        bool fullScreen;
        bool below;
    };
    gEngine->engineToNativeThread( new ThreadCallback( url, fullScreen, below ) );
}


void CCAppManager::WebViewOpenNativeThread(const char *url, const bool fullScreen, const bool below)
{
#ifdef IOS

    if( WebView != NULL )
    {
        if( WebView->fullScreen != fullScreen )
        {
            [WebView remove];
            WebView = NULL;
        }
    }

    if( WebView == NULL )
    {
        WebView = [[CCWebView alloc] initWithFrame:GLView.frame fullScreen:fullScreen];
        [WebView insert:Window view:GLView below:below];
    }
    [WebView openPage:url];

#elif defined ANDROID || defined WP8 || defined WIN8

    if( WebView == NULL )
    {
        WebView = new CCWebView( below );
    }
    WebView->openPage( url );

#elif defined QT

    if( WebView == NULL )
    {
        WebView = new CCWebView( CCMainWindow::Get() );
        CCMainWindow::AddChild( WebView );
    }

    WebView->openPage( url );

    // OpenGL widget's always render on top, so hide it until finished with this view
    gView->hideView();

#endif
}


void CCAppManager::WebViewLoadedNativeThread(const char *url, const char *data)
{
    class ThreadCallback : public CCLambdaCallback
    {
    public:
        ThreadCallback(const char *url, const char *data)
        {
            this->url = url;
            this->data = data;
        }
    protected:
        void run()
        {
            WebViewLoaded( this->url.buffer, this->data.buffer );
        }
    private:
        CCText url, data;
    };
    gEngine->nativeToEngineThread( new ThreadCallback( url, data ) );
}


void CCAppManager::WebViewLoaded(const char *url, const char *data)
{
    if( gEngine != NULL )
    {
        // Fire update
        for( int i=0; i<gEngine->onWebViewLoaded.length; ++i )
        {
            CCTextCallback *callback = gEngine->onWebViewLoaded.list[i];
            callback->add( url, 0 );
            callback->add( data, 1 );
            callback->safeRun();
        }
    }
}


void CCAppManager::WebViewClose(const bool nativeThread)
{
    if( nativeThread )
    {
        CCAppManager::WebViewCloseNativeThread();
    }
    else
    {
        CCLAMBDA_RUN_NATIVETHREAD( CCAppManager::WebViewCloseNativeThread(); );
    }
}


void CCAppManager::WebViewCloseNativeThread()
{
    if( WebView != NULL )
    {
#ifdef IOS

        [WebView remove];

#elif defined ANDROID || defined WP8 || defined WIN8

        delete WebView;

#elif defined QT

        WebView->shutdown();
        gView->showView();

#endif

        WebView = NULL;
    }
}


bool CCAppManager::WebViewIsLoaded()
{
#ifdef IOS

    if( WebView != NULL )
    {
        if( WebView->loaded )
        {
            return true;
        }
    }

#elif defined ANDROID || defined QT || defined WP8 || defined WIN8

    if( WebView != NULL )
    {
        if( WebView->isLoaded() )
        {
            return true;
        }
    }

#endif

    return false;
}


void CCAppManager::WebViewEval(const char *script, bool nativeThread)
{
#ifndef IOS
    if( !nativeThread )
    {
        // Only iOS manages both threads in C++
        nativeThread = true;
    }
#endif

    if( nativeThread )
    {
        WebViewEvalNativeThread( script );
    }
    else
    {
        class ThreadCallback : public CCLambdaCallback
        {
        public:
            ThreadCallback(const char *script)
            {
                this->script = script;
            }
        protected:
            void run()
            {
                WebViewEvalNativeThread( this->script.buffer );
            }
        private:
            CCText script;
        };
        gEngine->engineToNativeThread( new ThreadCallback( script ) );
    }
}


void CCAppManager::WebViewEvalNativeThread(const char *script)
{
    if( WebJSIsLoaded() )
    {

#ifdef IOS

        [WebView runJavaScript:script];

#elif defined ANDROID || defined WP8 || defined WIN8

        WebView->runJavaScript( script );

#elif defined QT

        WebView->runJavaScript( script );

#endif

    }
}


void CCAppManager::WebViewClearCache()
{
#ifdef IOS

    [CCWebView ClearCache];

#else

    CCWebView::ClearCache();

#endif
}



// WebJS
void CCAppManager::WebJSOpen(const char *url, const bool isFile, const char *htmlData, bool nativeThread)
{
#ifndef IOS
	if( !nativeThread )
	{
		// Only iOS manages both threads in C++
		nativeThread = true;
	}
#endif

    CCASSERT( url != NULL );
    if( nativeThread )
    {
        WebJSOpenNativeThread( url, isFile, htmlData );
    }
    else
    {
        class ThreadCallback : public CCLambdaCallback
        {
        public:
            ThreadCallback(const char *url, const bool isFile, const char *htmlData)
            {
                this->url = url;
                this->isFile = isFile;
                this->htmlData = htmlData;
            }
        protected:
            void run()
            {
                WebJSOpenNativeThread( url.buffer, isFile, htmlData.buffer );
            }
        private:
            CCText url;
            bool isFile;
            CCText htmlData;
            bool hidden;
        };
        gEngine->engineToNativeThread( new ThreadCallback( url, isFile, htmlData ) );
    }
}


void CCAppManager::WebJSOpenNativeThread(const char *url, const bool isFile, const char *htmlData)
{
#ifdef IOS

    if( WebJS == NULL )
    {
        WebJS = [[CCWebJS alloc] initWithFrame:GLView.frame];
        [WebJS setBackgroundColor:[UIColor blackColor]];
        //[Window insertSubview:WebJS belowSubview:GLView];
    }

    if( isFile )
    {
        [WebJS openFile:url];
    }
    else if( htmlData == NULL )
    {
        [WebJS openPage:url];
    }
    else
    {
        [WebJS openData:url data:htmlData];
    }

#elif defined ANDROID || defined WP8 || defined WIN8

    if( WebJS == NULL )
    {
        WebJS = new CCWebJS();
    }

    if( isFile )
    {
        WebJS->openFile( url );
    }
    else
    {
        WebJS->openPage( url, htmlData );
    }

#elif defined QT

    if( WebJS == NULL )
    {
        WebJS = new CCWebJS( CCMainWindow::Get() );
        CCMainWindow::AddChild( WebJS );
    }

    CCText fullURL = url;
    if( isFile )
    {
        fullURL = "file:///";
        fullURL += url;
    }

    WebJS->openPage( fullURL.buffer );
    WebJS->hide();

#endif
}


void CCAppManager::WebJSLoadedNativeThread(const char *url, const char *data)
{
    if( AppStarted )
    {
        DEBUGLOG( "CCAppManager::WebJSLoadedNativeThread WebJSEvalCalls:%i \n", WebJSEvalCalls );
        WebJSEvalCalls = 0;
        CCAppManager::WebJSSetLastEvalTime( gEngine->time.lifetime );

        // Fire update
        if( gEngine != NULL )
        {
            for( int i=0; i<gEngine->onWebJSLoaded.length; ++i )
            {
                CCTextCallback *callback = gEngine->onWebJSLoaded.list[i];
                callback->add( url, 0 );
                callback->add( data, 1 );
                callback->safeRun();
            }
        }
    }
}


void CCAppManager::WebJSClose(bool nativeThread)
{
#ifndef IOS
    if( !nativeThread )
    {
        // Only iOS manages both threads in C++
        nativeThread = true;
    }
#endif

    if( nativeThread )
    {
        WebJSCloseNativeThread();
    }
    else
    {
        CCLAMBDA_RUN_NATIVETHREAD( CCAppManager::WebJSCloseNativeThread(); );
    }
}


void CCAppManager::WebJSCloseNativeThread()
{
    if( WebJS != NULL )
    {
#ifdef IOS

        [WebJS remove];
        //const int count = [videoView retainCount];
        [WebJS release];

#elif defined ANDROID || defined WP8 || defined WIN8

        delete WebJS;

#elif defined QT

        WebJS->shutdown();

#endif

        WebJS = NULL;
    }
}


bool CCAppManager::WebJSIsLoaded()
{
    if( AppStarted && WebJS != NULL )
    {
#ifdef IOS

        if( WebJS->loaded )
        {
            return true;
        }

#elif defined ANDROID || defined QT || defined WP8 || defined WIN8

        if( WebJS->isLoaded() )
        {
            return true;
        }

#endif

    }

    return false;
}


void CCAppManager::WebJSEval(const char *script, const bool returnResult, bool nativeThread)
{
#ifndef IOS
	if( !nativeThread )
	{
		// Only iOS manages both threads in C++
		nativeThread = true;
	}
#endif

    WebJSEvalCalls++;
    //DEBUGLOG( "CCAppManager::WebJSEval() %i script:%s\n", CCAppManager::_THIS->WebJSEvalCalls, script );

    //CCASSERT( CCAppManager::THIS->webJS != NULL );
    //CCASSERT( CCAppManager::THIS->webJS->loaded );

    if( nativeThread )
    {
        WebJSEvalNativeThread( script, returnResult );
    }
    else
    {
        class ThreadCallback : public CCLambdaCallback
        {
        public:
            ThreadCallback(const char *script, const bool returnResult)
            {
                this->script = script;
                this->returnResult = returnResult;
            }
        protected:
            void run()
            {
                WebJSEvalNativeThread( this->script.buffer, this->returnResult );
            }
        private:
            CCText script;
            bool returnResult;
        };
        gEngine->engineToNativeThread( new ThreadCallback( script, returnResult ) );
    }
}


void CCAppManager::WebJSEvalNativeThread(const char *script, const bool returnResult)
{
    //DEBUGLOG( "CCAppManager::WebJSEvalNativeThread:%f, %s\n", gEngine->time.lifetime, script );

    CCData data;

    if( WebJSIsLoaded() )
    {

#ifdef IOS

        const CCData &result = [WebJS runJavaScript:script];
        data.set( result.buffer, result.length );
        WebJSEvalResultNativeThread( data, returnResult );

#elif defined ANDROID || defined WP8 || defined WIN8

        WebJS->runJavaScript( script, returnResult );
        // WebJSEvalResultNativeThread is called by CCWebJS once the javascript is run

#elif defined QT

        WebJS->runJavaScript( script, data );
        WebJSEvalResultNativeThread( data.buffer, returnResult );

#endif

    }
    else
    {
        WebJSEvalResultNativeThread( data, returnResult );
    }
}


void CCAppManager::WebJSEvalResultNativeThread(const CCData &data, const bool returnResult)
{
#if defined IOS || defined ANDROID || defined WP8 || defined WIN8 || defined QT

    // If the webJS isn't loaded, don't do anything
	if( WebJSIsLoaded() == false )
	{
		if( AppStarted )
		{
			DEBUGLOG( "CCAppManager::WebJSEvalResult (WebJSIsLoaded() == false) result:%i %s \n", WebJSEvalCalls, data.buffer );
			WebJSEvalCalls = 0;
		}
	}
	else if( WebJSEvalCalls <= 0 )
	{
		DEBUGLOG( "CCAppManager::WebJSEvalResult (WebJSEvalCalls <= 0) result:%i %s \n", WebJSEvalCalls, data.buffer );
		WebJSEvalCalls = 0;
	}
	else
	{
		WebJSEvalCalls--;

        //DEBUGLOG( "CCAppManager::WebJSEvalResultNativeThread WebJSEvalCalls:%i \n", WebJSEvalCalls );
		CCAppManager::WebJSSetLastEvalTime( gEngine->time.lifetime );

		// Fire update
        if( returnResult )
        {
            if( gEngine != NULL )
            {
                //DEBUGLOG( "CCAppManager::WebJSEvalResult:%f, %s\n", gEngine->time.lifetime, data );
                for( int i=0; i<gEngine->onWebJSEvalResult.length; ++i )
                {
                    CCDataCallback *callback = gEngine->onWebJSEvalResult.list[i];
                    callback->data = data;
                    callback->safeRun();
                }
            }
        }
	}

#endif
}


bool CCAppManager::WebJSIsEval()
{
    if( WebJSEvalCalls > 0 )
    {
        return true;
    }
    return false;
}


void CCAppManager::WebJSSetLastEvalTime(const float time)
{
    WebJSLastEvalTime = time;
}


float CCAppManager::WebJSGetLastEvalTime()
{
    return WebJSLastEvalTime;
}



void CCAppManager::SendEmail(const char *address, const char *subject, const char *body, const char *attachmentFilename, const CCData *attachmentData)
{
#ifdef IOS

    MailComposer *mail = [[MailComposer alloc] init];
    if( mail != NULL )
    {
        CCViewController *viewController = gViewController;
        mail.mailComposeDelegate = viewController;
        
        NSString *subjectLine = [NSString stringWithFormat:@"%s", subject];
        [mail setSubject:subjectLine];
        
        [mail setToRecipients:[NSArray arrayWithObject:[NSString stringWithFormat:@"%s", address]]];
        
        NSMutableString *nsBody = [[NSMutableString alloc] init];
        [nsBody appendString:[NSString stringWithFormat:@"%s", body]];
        [mail setMessageBody:nsBody isHTML:false];

        NSMutableData *nsAttachmentData = NULL;
        if( attachmentFilename != NULL && attachmentData != NULL )
        {
            nsAttachmentData = [[NSMutableData alloc] init];
            [nsAttachmentData appendBytes:attachmentData->buffer length:attachmentData->length];
            [mail addAttachmentData:nsAttachmentData mimeType:@"text/plain" fileName:[NSString stringWithFormat:@"%s", attachmentFilename]];
        }

        [viewController presentViewController:mail animated:YES completion:nil];
        [mail release];
        [nsBody release];

        if( nsAttachmentData != NULL )
        {
            [nsAttachmentData release];
        }
    }

#elif defined ANDROID

    const char *attachment = NULL;
    if( attachmentFilename != NULL && attachmentData != NULL )
    {
        CCFileManager::SaveCachedFile( attachmentFilename, attachmentData->buffer, attachmentData->length );
        attachment = attachmentFilename;
    }

    CCJNI::SendEmail( address, subject, body, attachment );

#endif
}



void CCAppManager::EnableControls(const bool toggle)
{
#ifdef IOS
    dispatch_async(dispatch_get_main_queue(), ^{
        if( GLView.userInteractionEnabled != toggle )
        {
            GLView.userInteractionEnabled = toggle;
        }
    });
#endif
}



void CCAppManager::KeyboardShow(const bool show)
{
    class ThreadCallback : public CCLambdaCallback
    {
    public:
        ThreadCallback(const bool show)
        {
            this->show = show;
        }
    protected:
        void run()
        {
            KeyboardShowNativeThread( show );
        }
    private:
        bool show;
    };
    gEngine->engineToNativeThread( new ThreadCallback( show ) );
}


void CCAppManager::KeyboardShowNativeThread(const bool show)
{
#ifdef IOS

    [gViewController keyboardShow:show];

#elif defined ANDROID

    CCJNI::KeyboardShow( show );

#elif defined WP8 || defined WIN8

	csActionStack.add( new CSAction( "CCAppManager::KeyboardShow, ", show ? "true" : "false" ) );

#endif
}


bool CCAppManager::DetectMusic(CCLambdaCallback *callback)
{
	if( detectMusicCallback != NULL )
	{
		delete detectMusicCallback;
	}
	detectMusicCallback = callback;

#ifdef IOS
    
    [GLView->musicPlayer detectMusic];
    
#elif defined ANDROID
    
    CCJNI::DetectMusic();
    return true;

#endif

    return false;
}


void CCAppManager::DetectMusicResult(const char *jsonString)
{
    if( detectMusicCallback != NULL )
    {
        CCLambdaCallback *callback = detectMusicCallback;

        callback->runParameters = (void*)jsonString;

        callback->safeRun();

        DELETE_POINTER( detectMusicCallback );
    }
}


void CCAppManager::SyncMusicEvent(const char *jsonString)
{
    if( gEngine != NULL )
    {
        gEngine->syncMusicEvent( jsonString );
    }
}


void CCAppManager::PlayMusic(const char *id, const float seconds)
{
#ifdef IOS
    
    [GLView->musicPlayer play:id time:seconds];
    
#elif defined ANDROID
    
    CCJNI::PlayMusic( id, seconds );

#endif
}


void CCAppManager::StopMusic()
{
#ifdef IOS
    
    [GLView->musicPlayer stop];
    
#elif defined ANDROID
    
    CCJNI::StopMusic();

#endif
}


void CCAppManager::SetMusicPosition(const float seconds)
{
#ifdef IOS
    
    [GLView->musicPlayer setPosition:seconds];
    
#elif defined ANDROID
    
    CCJNI::SetMusicPosition( seconds );

#endif
}


void CCAppManager::SetMusicVolume(const float volume)
{
#ifdef IOS
    
    [GLView->musicPlayer setVolume:volume];

#elif defined ANDROID
    
    CCJNI::SetMusicVolume( volume );

#endif
}


bool CCAppManager::DetectThinkGearDevice()
{
#ifdef IOS
#elif defined ANDROID
    
    return CCJNI::DetectThinkGearDevice();

#endif

    return false;
}


bool CCAppManager::SelectImageFromGallery(CCLambdaCallback *callback)
{
	if( selectImageFromGalleryCallback != NULL )
	{
		delete selectImageFromGalleryCallback;
	}
	selectImageFromGalleryCallback = callback;

#ifdef IOS

    CCLAMBDA_RUN_NATIVETHREAD({
        [gViewController selectImageFromGallery];
    });
    return true;

#elif defined ANDROID

    CCJNI::SelectImageFromGallery();
    return true;

#endif

    return false;
}


void CCAppManager::SelectImageFromGalleryResult(const char *filename)
{
    if( selectImageFromGalleryCallback != NULL )
    {
        CCLambdaCallback *callback = selectImageFromGalleryCallback;
        callback->runParameters = (void*)filename;
        callback->safeRun();
        DELETE_POINTER( selectImageFromGalleryCallback );
    }
}


void CCAppManager::AddImageToGallery(const char *filePath)
{
#ifdef ANDROID
    
    CCJNI::AddImageToGallery( filePath );

#elif defined IOS

    UIImage *image = [UIImage imageWithContentsOfFile:[NSString stringWithFormat:@"%s", filePath]];

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        UIImageWriteToSavedPhotosAlbum( image, nil, nil, nil );
    });

#else

    CCASSERT( false );

#endif
}


void CCAppManager::ShareImage(const char *filePath)
{
#ifdef ANDROID
    
    CCJNI::ShareImage( filePath );

#elif defined IOS

    static CCText path;
    path = filePath;
    dispatch_async(dispatch_get_main_queue(), ^{

        UIImage *image = [UIImage imageWithContentsOfFile:[NSString stringWithFormat:@"%s", path.buffer]];
        if( image != NULL )
        {
            NSString *text = @"Share...";
            NSURL *url = [NSURL URLWithString:@"http://playir.com/"];
            
            DMActivityInstagram *instagramActivity = [[DMActivityInstagram alloc] init];
            
            NSArray *activityItems = @[text, url, image];
            NSArray *applicationActivities = @[instagramActivity];

            UIActivityViewController *activityController = [[UIActivityViewController alloc] initWithActivityItems:activityItems applicationActivities:applicationActivities];

            // switch for iPhone and iPad.
            if( [[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad )
            {
                [gViewController presentPopOver:activityController];
            }
            else
            {
                [gViewController presentViewController:activityController animated:YES completion:nil];
            }
        }
    });

#else

    CCASSERT( false );

#endif
}


void CCAppManager::Exit()
{
#ifdef ANDROID
	CCJNI::Exit();
#endif
}
