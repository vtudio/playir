/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCEngine.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCTextureFontPage.h"
#include "CCObjects.h"
#include "CCFileManager.h"

#include "CCDeviceControls.h"
#include "CCDeviceRenderer.h"
#include "CCAppManager.h"

#ifdef IOS
#include <OpenGLES/ES1/gl.h>
#endif

#if defined WP8 || defined WIN8
#include <wrl.h>
#endif


#ifdef IOS
CCText CCEngine::DeviceType = "iOS";
CCText CCEngine::DeviceOS = "iOS";
#elif defined ANDROID
CCText CCEngine::DeviceType = "Android";
#ifdef GOOGLE
CCText CCEngine::DeviceOS = "AndroidGoogle";
#elif defined AMAZON
CCText CCEngine::DeviceOS = "AndroidAmazon";
#elif defined SAMSUNG
CCText CCEngine::DeviceOS = "AndroidSamsung";
#elif defined NOKIA
CCText CCEngine::DeviceOS = "AndroidNokia";
#endif
#elif defined WIN8
CCText CCEngine::DeviceType = "Windows8";
CCText CCEngine::DeviceOS = "Windows8";
#elif defined WP8
CCText CCEngine::DeviceType = "WindowsPhone";
CCText CCEngine::DeviceOS = "WindowsPhone";
#elif defined Q_OS_WIN
CCText CCEngine::DeviceType = "Windows";
CCText CCEngine::DeviceOS = "Windows";
#elif defined Q_OS_MACX
CCText CCEngine::DeviceType = "Mac";
CCText CCEngine::DeviceOS = "Mac";
#elif defined Q_OS_LINUX
CCText CCEngine::DeviceType = "Linux";
CCText CCEngine::DeviceOS = "Linux";
#else
CCText CCEngine::DeviceType = "Unknown";
CCText CCEngine::DeviceOS = "Unknown";
#endif


double CCEngine::GetSystemTime()
{
#ifdef IOS

    const NSTimeInterval currentTime = [NSDate timeIntervalSinceReferenceDate];

#elif defined( QT )

    const double currentTime = gView->timer.elapsed() * 0.001;

#elif defined( ANDROID )

    struct timespec res;
	clock_gettime( CLOCK_REALTIME, &res );
	const double currentTime = res.tv_sec + (double)( ( res.tv_nsec / 1e6 ) * 0.001 );

#elif defined( WP8 ) || defined WIN8

	static LARGE_INTEGER m_frequency, m_currentTime;
	if (!QueryPerformanceFrequency(&m_frequency))
	{
		throw ref new Platform::FailureException();
	}

	if (!QueryPerformanceCounter(&m_currentTime))
	{
		throw ref new Platform::FailureException();
	}

	const double currentTime = static_cast<double>(m_currentTime.QuadPart) / static_cast<double>(m_frequency.QuadPart);

#endif

    return currentTime;
}



CCEngine::CCEngine() :
    collisionManager( 250000.0f )
{
    running = true;
    engineThreadRunning = false;
    paused = false;

    renderer = NULL;
    textureManager = NULL;

	fpsLimit = 1/60.0f;

    // Initialise our time lastUpdate value;
    time.lastUpdate = CCEngine::GetSystemTime();

    backButtonActionPending = false;
}


CCEngine::~CCEngine()
{
    onWebViewLoaded.deleteObjects();
    onWebJSLoaded.deleteObjects();
    onWebJSEvalResult.deleteObjects();
    onKeyboardUpdate.deleteObjects();
    onPause.deleteObjects();
    onResume.deleteObjects();

    // Run remaining callbacks
    while( nativeThreadCallbacks.length > 0 || engineThreadCallbacks.length > 0 )
    {
        while( nativeThreadCallbacks.length > 0 )
        {
        	CCLambdaCallback *callback = nativeThreadCallbacks.list[0];
        	nativeThreadCallbacks.remove( callback );
        	callback->safeRun();
            delete callback;
        }

        while( engineThreadCallbacks.length > 0 )
        {
        	CCLambdaCallback *callback = engineThreadCallbacks.list[0];
        	engineThreadCallbacks.remove( callback );
        	callback->safeRun();
            delete callback;
        }
    }

    delete cameraRecorder;
    delete urlManager;
	delete textureManager;
	delete controls;
	delete renderer;

	gEngine = NULL;

    CCNativeThreadUnlock();
}


void CCEngine::setupNativeThread()
{
    CCAudioManager::Reset();
    CCExternalAccessoryManager::Reset();
}


static int ZCompare(const void *a, const void *b)
{
    const CCCollideable *objectA = CCOctreeGetVisibleCollideables( *(int*)a );
    const CCCollideable *objectB = CCOctreeGetVisibleCollideables( *(int*)b );
    const int drawOrderA = objectA->getDrawOrder();
    const int drawOrderB = objectB->getDrawOrder();

    if( drawOrderA == 200 || drawOrderB == 200 )
    {
		if( CCCameraBase::CurrentCamera != NULL )
		{
			if( drawOrderA == 200 && drawOrderB == 200 )
			{
				const CCVector3 &cameraPosition = CCCameraBase::CurrentCamera->getRotatedPosition();
				const CCVector3 &positionA = objectA->getConstPosition();
				const CCVector3 &positionB = objectB->getConstPosition();
				const float distanceA = CCVector3Distance( positionA, cameraPosition, true );
				const float distanceB = CCVector3Distance( positionB, cameraPosition, true );

				// If A is smaller than B, swap
				return (int)( distanceB - distanceA );
			}
        }

        // Largest to the back to be drawn last
        return drawOrderA - drawOrderB;
    }

    // Largest to the back to be drawn last
    return drawOrderA - drawOrderB;
}


bool CCEngine::setupEngineThread()
{
    urlManager = new CCURLManager();
    CCCameraBase::SetVisibleSortFunction( &ZCompare );
    const bool rendererSetup = setupRenderer();
    controls = new CCDeviceControls();
    cameraRecorder = new CCCameraRecorder();

    if( rendererSetup )
    {
        start();
    }

    engineThreadRunning = true;
    return rendererSetup;
}


void CCEngine::createRenderer()
{
    if( renderer != NULL )
    {
        delete renderer;
    }

    renderer = new CCDeviceRenderer();
}


bool CCEngine::setupRenderer()
{
    if( renderer->setup() )
    {
    	DEBUG_OPENGL();

    	if( textureManager == NULL )
    	{
    		textureManager = new CCTextureManager();
    	}
        textureManager->invalidateAllTextureHandles();
    	DEBUG_OPENGL();

        renderer->setupOpenGL();
        DEBUG_OPENGL();

        return true;
    }

    return false;
}


void CCEngine::updateTime()
{
    double currentTime = CCEngine::GetSystemTime();
    double realTime = ( currentTime - time.lastUpdate );

    // If we're too fast, sleep
	if( realTime < fpsLimit )
	{
        // Round down, as we can't sleep for half a microsecond
        const uint difference = uint( ( fpsLimit - realTime ) * 1000.0f );
        if( difference > 0 )
        {
            //DEBUGLOG( "CCEngine::updateTime sleep %f %i\n", currentTime, difference );
            usleep( difference );
        }

		currentTime = CCEngine::GetSystemTime();
		realTime = ( currentTime - time.lastUpdate );
	}
    time.real = (float)realTime;

	// Fake 25 fps
    static const float minFPS = 1/15.0f;
	time.delta = MIN( time.real, minFPS );

    time.lastUpdate = currentTime;
}


bool CCEngine::updateNativeThread()
{
    // Run callbacks
	if( nativeThreadCallbacks.length > 0 )
    {
        int jobsProcessed = 0;
        
        const double startTime = CCEngine::GetSystemTime();
        const double finishTime = startTime + 0.002f;   // Spend a max of 2ms on this task
        double currentTime = startTime;
        while( nativeThreadCallbacks.length > 0 )
        {
            CCNativeThreadLock();
            CCLambdaCallback *callback = nativeThreadCallbacks.pop();
			if( callback != NULL )
			{
				callback->safeRun();
            }
            CCNativeThreadUnlock();
            
            if( callback != NULL )
            {
				delete callback;
			}
            
            jobsProcessed++;
            if( textureManager != NULL && textureManager->isReady() )
            {
				currentTime = CCEngine::GetSystemTime();
				if( currentTime > finishTime )
				{
					//DEBUGLOG( "Max nativeThreadCallbacks processed in time %i, %i\n", jobsProcessed, engineThreadCallbacks.length );
					break;
				}
            }
        }
    }

	return false;
}


void CCEngine::updateEngineThread()
{
    // Update our system time
    updateTime();

	time.lifetime += time.real;

#if LOG_FPS
    static uint loggedUpdates = 0;
    static float loggedDelta = 0.0f;
    loggedUpdates++;
    loggedDelta += time.real;
    if( loggedDelta > 1.0f )
    {
#if !defined WP8 && !defined WIN8
        const float averageFPS = 1.0f / ( loggedDelta / loggedUpdates );
        DEBUGLOG( "Average FPS: %f \n", averageFPS );
#endif
        loggedUpdates = 0;
        loggedDelta = 0.0f;
    }
#endif

    if( backButtonActionPending )
    {
    	backButtonActionPending = false;
    	handleBackButton();
    }

    // Run callbacks
    if( engineThreadCallbacks.length > 0 )
    {
        int jobsProcessed = 0;
        
        const double startTime = CCEngine::GetSystemTime();
        const double finishTime = startTime + 0.002f;   // Spend a max of 2ms on this task
        double currentTime = startTime;
        while( engineThreadCallbacks.length > 0 )
        {
            CCNativeThreadLock();
            CCJobsThreadLock();
            CCLambdaCallback *callback = engineThreadCallbacks.pop();
			if( callback != NULL )
			{
				callback->safeRun();
            }
            CCNativeThreadUnlock();
            CCJobsThreadUnlock();
            
            if( callback != NULL )
            {
				delete callback;
			}
            
            jobsProcessed++;
            if( textureManager != NULL && textureManager->isReady() )
            {
				currentTime = CCEngine::GetSystemTime();
				if( currentTime > finishTime )
				{
					DEBUGLOG( "Max engineThreadCallbacks processed in time %i, %i\n", jobsProcessed, engineThreadCallbacks.length );
					break;
				}
            }
        }
    }

    finishJobs();
	updateLoop();

    if( paused == false )
    {
        CCAppManager::UpdateOrientation( time.delta );
    }
    renderLoop();

#if defined DEBUGON && TARGET_IPHONE_SIMULATOR
	// 66 frames a second in debug
	//usleep( 15000 );
	usleep( 0 );
#endif
}


#ifdef WP8
#include <ppl.h>
#include <ppltasks.h>

bool CCEngine::updateJobsThread()
{
    // Run callbacks
    CCJobsThreadLock();

	static bool RUNNING_JOB = false;
	if( jobsThreadCallbacks.length > 0 && !RUNNING_JOB )
    {
		RUNNING_JOB = true;

        CCLambdaCallback *callback = jobsThreadCallbacks.pop();
        CCJobsThreadUnlock();

		auto currentThread = Concurrency::task_continuation_context::use_current();
		Concurrency::create_task([this, callback] {

			// Runs on a random thread
			if( callback->isActive() )
			{
				callback->safeRunOnly();
			}

		}).then([this, callback]() {

			// Finishes on current thread
			if( callback->isActive() )
			{
				callback->safeFinishOnly();
			}
			delete callback;

			RUNNING_JOB = false;

		}, currentThread ); // schedule this continuation to run in the current context.
    }

	CCJobsThreadUnlock();
    return false;
}

#else

bool CCEngine::updateJobsThread(const bool multicore)
{
    // If we have more than one CPU we can process more jobs
    if( multicore || textureManager == NULL || !textureManager->isReady() )
    {
        int jobsProcessed = 0;
        
        const double startTime = CCEngine::GetSystemTime();
        const double finishTime = startTime + 0.002f;   // Spend a max of 2ms on this task
        double currentTime = startTime;

        bool updated = false;
        while( jobsThreadCallbacks.length > 0 )
        {
            CCJobsThreadLock();
            CCLambdaCallback *callback = jobsThreadCallbacks.pop();
            CCJobsThreadUnlock();
            callback->safeRun();
            delete callback;
            updated = true;

            jobsProcessed++;
            if( textureManager != NULL && textureManager->isReady() )
            {
				currentTime = CCEngine::GetSystemTime();
				if( currentTime > finishTime )
				{
					//DEBUGLOG( "Max jobsThreadCallbacks processed in time %i, %i\n", jobsProcessed, engineThreadCallbacks.length );
					break;
				}
            }
        }
        return updated;
    }
    
    // One job per call on single cores
	if( jobsThreadCallbacks.length > 0 )
    {
        CCJobsThreadLock();
        CCLambdaCallback *callback = jobsThreadCallbacks.pop();
        CCJobsThreadUnlock();
        callback->safeRun();
        delete callback;
        return true;
    }
    return false;
}

#endif


void CCEngine::renderFrameBuffer(const int frameBufferID)
{
    if( renderer->openGL2() == false )
    {
        return;
    }

    if( frameBufferID == -1 )
    {
        return;
    }

    const bool currentBlendState = CCRenderer::CCGetBlendState();
    const bool currentDepthWriteState = CCRenderer::CCGetDepthWriteState();

    // Draw frame buffer

    CCRenderer::CCSetBlend( currentBlendState );
    CCRenderer::CCSetDepthWrite( currentDepthWriteState );
}


void CCEngine::finishJobs()
{
#if defined PROFILEON
    CCProfiler profile( "CCEngine::finishJobs()" );
#endif

    CCFileManager::ReadyIO();

    urlManager->update();

	// Prune the octree
	if( collisionManager.pruneOctreeTimer > 0.0f )
	{
		collisionManager.pruneOctreeTimer -= time.real;
		if( collisionManager.pruneOctreeTimer <= 0.0f )
		{
            //DEBUGLOG( "Octree - prune" );
			CCOctreePruneTree( collisionManager.octree );
		}
	}
}


void CCEngine::restart()
{
    urlManager->flushPendingRequests();

    start();
}


void CCEngine::touchUpdateMovementThreasholds()
{
    CCControls::RefreshTouchMovementThreashold();
}


void CCEngine::nextEngineUpdate(CCLambdaCallback *lambdaCallback, const int index)
{
    engineThreadCallbacks.add( lambdaCallback );

    if( index >= 0 )
    {
        engineThreadCallbacks.reinsert( lambdaCallback, index );
    }
}


void CCEngine::engineToNativeThread(CCLambdaCallback *lambdaCallback)
{
    CCNativeThreadLock();
    nativeThreadCallbacks.add( lambdaCallback );
    CCNativeThreadUnlock();
}


void CCEngine::nativeToEngineThread(CCLambdaCallback *lambdaCallback, const bool pushToFront)
{
    CCNativeThreadLock();
    nextEngineUpdate( lambdaCallback, pushToFront ? 0 : -1 );
    CCNativeThreadUnlock();
}


void CCEngine::engineToJobsThread(CCLambdaCallback *lambdaCallback, const bool pushToFront)
{
    CCJobsThreadLock();
    jobsThreadCallbacks.add( lambdaCallback );
    if( pushToFront )
    {
        jobsThreadCallbacks.reinsert( lambdaCallback, 0 );
    }
    CCJobsThreadUnlock();
}


void CCEngine::jobsToEngineThread(CCLambdaCallback *lambdaCallback)
{
    CCJobsThreadLock();
    engineThreadCallbacks.add( lambdaCallback );
    CCJobsThreadUnlock();
}


void CCEngine::pause()
{
    paused = true;
    CCLAMBDA_EMIT( onPause );
}


void CCEngine::resume()
{
    paused = false;
    CCLAMBDA_EMIT( onResume );
}


void CCEngine::touchBegin(const int index, const float x, const float y)
{
}


void CCEngine::touchMove(const int index, const float x, const float y)
{
}


void CCEngine::touchEnd(const int index)
{
}


bool CCEngine::shouldHandleBackButton()
{
    return backButtonActionPending;
}
