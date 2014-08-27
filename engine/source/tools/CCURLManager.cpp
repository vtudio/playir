/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCURLManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCDeviceURLManager.h"
#include "CCFileManager.h"
#include <time.h>


CCURLManager::CCURLManager()
{
    deviceURLManager = new CCDeviceURLManager();

    highPriorityRequestsPending = false;
}


CCURLManager::~CCURLManager()
{
    flushPendingRequests();

    DELETE_POINTER( deviceURLManager );

    domainTimeOuts.deleteObjectsAndList();
}


void CCURLManager::update()
{
	CCNativeThreadLock();
	/////////////////////

    // Have any requests finished?
    if( currentRequests.length > 0 )
    {
        for( int i=0; i<currentRequests.length; ++i )
        {
            CCURLRequest *currentRequest = currentRequests.list[i];
            if( currentRequest->state >= CCURLRequest::failed )
            {
                finishURL( currentRequest );
            }
        }
    }

    // Should start processing new requests
    if( isReadyToRequest() )
	{
        // Start a request
        // There's three streams going, one for anything, two for higher priority requests
        while( currentRequests.length < 3 && requestQueue.length > 0 )
		{
            CCURLRequest *pendingRequest = requestQueue.list[0];
            CCASSERT( pendingRequest->state == CCURLRequest::not_started );

            // Don't process any low priority requests if we already have one in progress
            if( currentRequests.length == 1 )
            {
                if( pendingRequest->priority == 0 )
                {
                    break;
                }
            }

            const bool removed = requestQueue.remove( pendingRequest );
            CCASSERT( removed );
            currentRequests.add( pendingRequest );
        }
	}

    // Either cache or kick off our download if we haven't started our current requests
    for( int i=0; i<currentRequests.length; ++i )
    {
        CCURLRequest *currentRequest = currentRequests.list[i];
        if( currentRequest->state == CCURLRequest::not_started )
        {
            if( gEngine->time.lifetime < currentRequest->timeRequestable )
            {
                continue;
            }

            if( currentRequest->checkCache )
            {
                if( currentRequest->cacheChecked == false )
                {
                    currentRequest->cacheChecked = true;

                    // See if the data has been cached
                    if( useCacheFile( currentRequest ) )
                    {
                        currentRequest = NULL;
                    }
                }
            }

            if( currentRequest != NULL )
            {
                // Check to see if the url needs to wait for the domain to be ready
                bool wait = false;
                for( int i=0; i<domainTimeOuts.length; ++i )
                {
                    DomainTimeOut *domainTimeOut = domainTimeOuts.list[i];
                    if( CCText::Contains( currentRequest->url, domainTimeOut->name ) )
                    {
                        const float nextRequestTime = domainTimeOut->lastRequested + domainTimeOut->timeout;
                        if( gEngine->time.lifetime < nextRequestTime )
                        {
                            wait = true;
                        }
                        break;
                    }
                }

                if( wait == false )
                {
                    currentRequest->timeRequested = gEngine->time.lifetime;
                    currentRequest->downloadFile = "download_";
                    if( currentRequest->cacheFile.length > 0 )
                    {
                        CCText cacheFilename = currentRequest->cacheFile;
                        cacheFilename.stripDirectory();
                        currentRequest->downloadFile += cacheFilename.buffer;
                    }
                    else
                    {
                        CCText name = currentRequest->url;
                        name.stripDirectory();
                        name.replaceChars( "?", "_" );
                        name.replaceChars( "#", "_" );
                        name.replaceChars( "=", "_" );
                        name.replaceChars( ":", "_" );
                        name.replaceChars( "@", "_" );
                        name.replaceChars( "/", "_" );
                        name.replaceChars( "\\", "_" );
                        currentRequest->downloadFile += name;
                    }
                    if( currentRequest->downloadFile.length > 64 )
                    {
                        currentRequest->downloadFile.trimLength( 64 );
                    }
                    deviceURLManager->processRequest( currentRequest );

                    // Record the last request of this domain
                    for( int i=0; i<domainTimeOuts.length; ++i )
                    {
                        DomainTimeOut *domainTimeOut = domainTimeOuts.list[i];
                        if( CCText::Contains( currentRequest->url, domainTimeOut->name ) )
                        {
                            domainTimeOut->lastRequested = gEngine->time.lifetime;
                            break;
                        }
                    }
                }
            }
        }
    }

    /////////////////////
	CCNativeThreadUnlock();
}


void CCURLManager::flushPendingRequests()
{
    deviceURLManager->clear();

    // Clean up our request object
    currentRequests.deleteObjects();
    requestQueue.deleteObjects();
}


void CCURLManager::requestURL(const char *url,
                              CCURLCallback *inCallback,
                              const int priority)
{
    requestURLAndCacheAfterTimeout( url, inCallback, priority, NULL, 0, 0.0f );
}

void CCURLManager::requestURLAfterTimeout(const char *url,
                                          CCURLCallback *inCallback,
                                          const int priority,
                                          const float timeout)
{
    requestURLAndCacheAfterTimeout( url, inCallback, priority, NULL, 0, timeout );
}

void CCURLManager::requestURLAndCache(const char *url,
                                      CCURLCallback *inCallback,
                                      const int priority,
                                      const char *cacheFile,
                                      const int cacheFileTimeoutInSeconds)
{
    requestURLAndCacheAfterTimeout( url, inCallback, priority, cacheFile, cacheFileTimeoutInSeconds, 0.0f );
}


CCURLRequest* CCURLManager::findUnprocessedRequest(const char *url, const char *cacheFile)
{
    for( int i=0; i<requestQueue.length; ++i )
    {
        CCURLRequest *request = requestQueue.list[i];
        CCASSERT( request != NULL );
        CCASSERT( request->url.length < 1000 );
        CCASSERT( request->state == CCURLRequest::not_started );
        if( CCText::Equals( request->url.buffer, url ) )
        {
            if( request->cacheFileTimeoutInSeconds == -1 )
            {
                bool matchingCacheState = false;
                if( cacheFile == NULL )
                {
                    if( request->cacheFile.length == 0 )
                    {
                        matchingCacheState = true;
                    }
                }
                else
                {
                    if( CCText::Equals( request->cacheFile, cacheFile ) )
                    {
                        matchingCacheState = true;
                    }
                }

                if( matchingCacheState )
                {
                    return request;
                    break;
                }
            }
        }
    }
    return NULL;
}


void CCURLManager::requestURLAndCacheAfterTimeout(const char *url,
                                                  CCURLCallback *inCallback,
                                                  const int priority,
                                                  const char *cacheFile,
                                                  const int cacheFileTimeoutInSeconds,
                                                  const float timeout)
{
    CCASSERT( priority >= 0 && priority <= 10 );
    if( priority > 0 )
    {
        highPriorityRequestsPending = true;
    }

    CCURLRequest *urlRequest = NULL;

    // Ignore advanced requests
    if( timeout == 0.0f && cacheFileTimeoutInSeconds == -1 )
    {
        urlRequest = findUnprocessedRequest( url, cacheFile );
    }

    if( urlRequest == NULL )
    {
        urlRequest = new CCURLRequest();
        urlRequest->url.set( url );
        urlRequest->cacheFileTimeoutInSeconds = cacheFileTimeoutInSeconds;
        urlRequest->timeRequestable = gEngine->time.lifetime + timeout;

        if( cacheFile != NULL )
        {
            urlRequest->cacheFile = cacheFile;
        }

        // If our priority is 0 push it to the back
        requestQueue.add( urlRequest );
    }
    updateRequestPriority( urlRequest, priority );

    if( inCallback != NULL )
    {
        inCallback->reply = urlRequest;
        urlRequest->onComplete.add( inCallback );
    }

    // Is this request already cached?
    if( urlRequest->checkCache )
    {
        if( gEngine->time.lifetime >= urlRequest->timeRequestable )
        {
            if( urlRequest->cacheChecked == false )
            {
                urlRequest->cacheChecked = true;

                // See if the data has been cached
                if( useCacheFile( urlRequest ) )
                {
                    const bool removed = requestQueue.remove( urlRequest );
                    CCASSERT( removed );
                    currentRequests.add( urlRequest );
                    finishURL( urlRequest );
                    return;
                }

#ifdef DEBUGON
				// For stepping through any cache misses
				else
				{
					useCacheFile( urlRequest );
				}
#endif
            }
        }
    }

//    // TEMP Disable downloads
//    urlRequest->state = CCURLRequest::failed;
//    const bool removed = requestQueue.remove( urlRequest );
//    CCASSERT( removed );
//    currentRequests.add( urlRequest );
//    finishURL( urlRequest );
}


void CCURLManager::requestPostURL(const char *url,
                                  const CCPairList<CCText, CCData> &postData,
                                  CCURLCallback *inCallback,
                                  const int priority,
                                  const float timeout)
{
    CCASSERT( priority >= 0 && priority <= 4 );
    if( priority > 0 )
    {
        highPriorityRequestsPending = true;
    }

    CCURLRequest *urlRequest = new CCURLRequest();
    urlRequest->url.set( url );

    for( int i=0; i<postData.length(); ++i )
    {
        urlRequest->postData.names.add( new CCText( postData.names.list[i]->buffer ) );
        CCData *value = new CCData();
        value->set( postData.values.list[i]->buffer, postData.values.list[i]->length );
        urlRequest->postData.values.add( value );
    }

    CCData &postBody = urlRequest->postBody;
    postBody += "--";
    postBody += CCPOST_BOUNDARY;

    for( int i=0; i<postData.length(); ++i )
    {
        CCText &name = *postData.names.list[i];
        CCData &value = *postData.values.list[i];

        if( CCText::Equals( name, "file" ) )
        {
            postBody += "\r\nContent-Disposition: form-data; name=\"";
            postBody += name.buffer;
            postBody += "\"; filename=\"blob\"\r\n";
            postBody += "Content-Type: application/octet-stream\r\n\r\n";
        }
        else
        {
            postBody += "\r\nContent-Disposition: form-data; name=\"";
            postBody += name.buffer;
            postBody += "\"\r\n\r\n";
        }
        postBody.append( value.buffer, value.length );

        postBody += "\r\n--";
        postBody += CCPOST_BOUNDARY;
    }

    postBody += "--\r\n";

    urlRequest->timeRequestable = gEngine->time.lifetime + timeout;
    requestQueue.add( urlRequest );
    updateRequestPriority( urlRequest, priority );

    if( inCallback != NULL )
    {
        inCallback->reply = urlRequest;
        urlRequest->onComplete.add( inCallback );
    }
}


void CCURLManager::updateRequestPriority(CCURLRequest *urlRequest, const int priority)
{
    if( urlRequest->priority != priority )
    {
        if( urlRequest->priority > priority )
        {
            urlRequest->priority = priority;
        }

        urlRequest->priority = priority;

        // If our new priority is 0, push it to the back of the queue
        if( priority == 0 )
        {
            const bool removed = requestQueue.remove( urlRequest );
            CCASSERT( removed );
            requestQueue.add( urlRequest );
        }

        // If our new priority is greater, make sure it's positioned appropriately
        else
        {
            const int urlIndex = requestQueue.find( urlRequest );
            const int previousIndex = urlIndex - 1;
            const int nextIndex = urlIndex + 1;
            bool correctSlot = true;

            // Ensure the previous request has a greater or equal priority
            if( previousIndex >= 0 )
            {
                CCURLRequest *request = requestQueue.list[previousIndex];
                if( request->priority < priority )
                {
                    correctSlot = false;
                }
            }

            // Ensure the next request has a less than or equal priority
            if( nextIndex < requestQueue.length )
            {
                CCURLRequest *request = requestQueue.list[nextIndex];
                if( request->priority > priority )
                {
                    correctSlot = false;
                }
            }

            // Otherwise reinsert it in the correct priority
            if( correctSlot == false )
            {
                const bool removed = requestQueue.remove( urlRequest );
                CCASSERT( removed );
                requestQueue.add( urlRequest );
                for( int i=0; i<requestQueue.length; ++i )
                {
                    CCURLRequest *request = requestQueue.list[i];
                    if( request->priority < priority )
                    {
                        requestQueue.reinsert( urlRequest, i );
                        break;
                    }
                }
            }
        }
    }
}


bool CCURLManager::useCacheFile(CCURLRequest *urlRequest, bool ignoreTimeout)
{
    if( ignoreTimeout || urlRequest->cacheFileTimeoutInSeconds != 0 )
    {
        // See if the data has been cached
        if( urlRequest->cacheFile.length > 0 )
        {
            struct stat fileInfo;
            int fileSize = CCFileManager::GetFileInfo( urlRequest->cacheFile.buffer, Resource_Unknown, false, &fileInfo );

            if( /*false*/ fileSize > 0 )
            {
                // In seconds
                time_t timeNow = time( NULL );
#if defined( Q_OS_WIN ) || defined( ANDROID ) || defined( Q_OS_LINUX ) || defined( WP8 ) || defined( WIN8 )
                time_t timeSince = timeNow - fileInfo.st_mtime;
#else
                time_t timeSince = timeNow - fileInfo.st_mtimespec.tv_sec;
#endif
                if( ignoreTimeout ||
                    urlRequest->cacheFileTimeoutInSeconds == -1 ||
                    urlRequest->cacheFileTimeoutInSeconds > timeSince )
                {
                    urlRequest->state = CCURLRequest::used_cache;
                    urlRequest->downloadFile = urlRequest->cacheFile;
                    return true;
                }
            }
        }
    }
    return false;
}


void CCURLManager::finishURL(CCURLRequest *request)
{
    // Validate data
    if( request->state == CCURLRequest::succeeded )
    {
        if( request->downloadLength == 0 )
        {
            request->state = CCURLRequest::failed;
        }
    }

    // Save out our result?
    bool keepingDownload = false;
    if( request->cacheFile.length > 0 )
    {
        if( CCText::Equals( request->downloadFile, request->cacheFile ) )
        {
            if( request->state >= CCURLRequest::succeeded )
            {
                keepingDownload = true;
            }
        }
        else
        {
            if( request->state == CCURLRequest::succeeded )
            {
                if( CCFileManager::DoesFileExist( request->downloadFile.buffer, Resource_Temp ) )
                {
                    CCFileManager::RenameCachedFile( request->downloadFile.buffer, Resource_Temp, request->cacheFile.buffer );
                }
                if( CCFileManager::DoesFileExist( request->cacheFile.buffer, Resource_Cached ) )
                {
                    request->downloadFile = request->cacheFile;
                }
                keepingDownload = true;
            }
        }
    }

    // Handle failed
    if( request->state == CCURLRequest::failed )
    {
        if( useCacheFile( request, true ) )
        {
            request->state = CCURLRequest::failed_but_used_cache;
        }
    }

    //LAMBDA_EMIT_ONCE( request->onComplete );
    for( int i=0; i<request->onComplete.length; ++i )
    {
        CCLambdaCallback *callback = request->onComplete.list[i];
        callback->safeRun();
    }
    request->onComplete.deleteObjectsAndList();

    // Clean up our request object
    const bool removed = currentRequests.remove( request );
    CCASSERT( removed );
    if( !keepingDownload )
    {
        if( CCFileManager::DoesFileExist( request->downloadFile.buffer, Resource_Temp ) )
        {
            CCFileManager::DeleteFile( request->downloadFile.buffer, Resource_Temp, false );
        }
    }
    delete request;

    // Reset our high priority marker
    if( highPriorityRequestsPending )
    {
        highPriorityRequestsPending = false;
        for( int i=0; i<requestQueue.length; ++i )
        {
            CCURLRequest *request = requestQueue.list[i];
            if( request->priority > 0 )
            {
                highPriorityRequestsPending = true;
                break;
            }
        }
        if( highPriorityRequestsPending == false )
        {
            for( int i=0; i<currentRequests.length; ++i )
            {
                CCURLRequest *request = currentRequests.list[i];
                if( request->priority > 0 )
                {
                    highPriorityRequestsPending = true;
                    break;
                }
            }
        }
    }
}


void CCURLManager::setDomainTimeOut(const char *domain, float timeout)
{
    for( int i=0; i<domainTimeOuts.length; ++i )
    {
        DomainTimeOut *domainTimeOut = domainTimeOuts.list[i];
        if( CCText::Equals( domain, domainTimeOut->name ) )
        {
            return;
        }
    }

    DomainTimeOut *domainTimeOut = new DomainTimeOut();
    domainTimeOut->name = domain;
    domainTimeOut->timeout = timeout;
    domainTimeOuts.add( domainTimeOut );
}


bool CCURLManager::isReadyToRequest()
{
    if( requestQueue.length > 0 )
    {
        if( deviceURLManager->isReadyToRequest() )
        {
            return true;
        }
    }
    return false;
}
