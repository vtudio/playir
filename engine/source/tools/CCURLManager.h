/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCURLManager.h
 * Description : Manager for HTTP requests.
 *
 * Created     : 02/06/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCURLMANAGER_H__
#define __CCURLMANAGER_H__


class CCURLCallback : public CCLambdaCallback
{
public:
    CCURLCallback()
    {
        reply = NULL;
    }
    struct CCURLRequest *reply;
};

#define CCPOST_BOUNDARY "---------------------------14737809831466499882746641449"
struct CCURLRequest
{
	enum RequestState
    {
		not_started = 1,
        in_flight = 2,
		failed = 3,
        timed_out = 4,
        data_error = 5,
        succeeded = 6,              // Fresh download from internet
        used_cache = 7,             // Found and used cached results
        failed_but_used_cache = 8   // Failed but returned the cache
	};

	CCURLRequest()
	{
        state = CCURLRequest::not_started;
        priority = 0;
        checkCache = true;
        cacheChecked = false;
        cacheFileTimeoutInSeconds = -1;
        timeRequestable = 0.0f;
        timeRequested = -1.0f;
        downloadLength = 0;
	}

    ~CCURLRequest()
    {
        onComplete.deleteObjectsAndList();
    }

    CCText url;						// The URL to request
    CCPairList<CCText, CCData> postData;
    CCData postBody;
    int priority;
    CCLAMBDA_SIGNAL onComplete;		// The on complete callback
    RequestState state;				// The state of the request
    CCText cacheFile;				// The filename to cache the result
    bool checkCache;				// Should we check the cache?
    int cacheFileTimeoutInSeconds;	// How long before our cache file becomes invalid
    bool cacheChecked;				// Have we checked the cache?
    float timeRequestable;			// The time we can process this request
    float timeRequested;			// The time when the request was made
    CCText downloadFile;
    int downloadLength;

    CCPairList<CCText, CCText> header;
};


class CCDeviceURLManager;

class CCURLManager
{
public:
    CCURLManager();
    ~CCURLManager();

    void update();

    void flushPendingRequests();

    void requestURL(const char *url,
                    CCURLCallback *inCallback=NULL,
                    const int priority=0);

    void requestURLAfterTimeout(const char *url,
                                CCURLCallback *inCallback,
                                const int priority,
                                const float timeout);

    void requestURLAndCache(const char *url,
                            CCURLCallback *inCallback,
                            const int priority,
                            const char *cacheFile,
                            const int cacheFileTimeoutInSeconds=-1);

    CCURLRequest* findUnprocessedRequest(const char *url, const char *cacheFile);
    
    void requestURLAndCacheAfterTimeout(const char *url,						// The URL to request
                                        CCURLCallback *inCallback=NULL,			// The on complete callback
                                        const int priority=0,					// The priority
                                        const char *cacheFile=NULL,				// The file to save the result to
                                        const int cacheFileTimeoutInSeconds=-1,	// How long before our cache file becomes invalid
                                        const float timeout=0.0f);				// Length of time to wait before requesting

    void requestPostURL(const char *url,
                        const CCPairList<CCText, CCData> &postData,
                        CCURLCallback *inCallback=NULL,
                        const int priority=0,
                        const float timeout=0.0f);

    void updateRequestPriority(CCURLRequest *urlRequest, const int priority);

protected:
    bool useCacheFile(CCURLRequest *urlRequest, bool ignoreTimeout=false);
    void finishURL(CCURLRequest *request);

public:
    void setDomainTimeOut(const char *domain, float timeout);
    bool processingHighPriority() { return highPriorityRequestsPending; }
    int getRequestQueueLength() { return requestQueue.length; }

protected:
    bool isReadyToRequest();

public:
    CCDeviceURLManager *deviceURLManager;

protected:
    CCPtrList<CCURLRequest> currentRequests;
    CCPtrList<CCURLRequest> requestQueue;
    bool highPriorityRequestsPending;

    struct DomainTimeOut
    {
        DomainTimeOut()
        {
            lastRequested = 0.0f;
        }
        CCText name;
        float timeout;
        float lastRequested;
    };
    CCPtrList<DomainTimeOut> domainTimeOuts;
};


#endif // __CCURLMANAGER_H__
