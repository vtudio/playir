/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCFileManager.h
 * Description : Handles loading files between platforms.
 *
 * Created     : 11/05/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCFILEMANAGER_H__
#define __CCFILEMANAGER_H__


#include "CCTypes.h"
#include "CCCallbacks.h"
#include <sys/stat.h>

class CCIOCallback : public CCLambdaCallback
{
public:
    CCIOCallback(const int inPriority)
    {
        priority = inPriority;
        exists = false;
    }

    // Run this before IO is performed to verify the lambda of the call is still active
    virtual bool isCallbackActive() = 0;

    CCText filePath;
    int priority;
    bool exists;
};


class CCFileManager
{
public:
    virtual ~CCFileManager() {}

    static CCFileManager* File(CCResourceType resourceType);
    virtual bool open(const char *filePath) = 0;
    virtual void close() = 0;

    virtual uint read(void *dest, const uint size) = 0;

    virtual void seek(const uint size) = 0;
    virtual bool endOfFile() = 0;
    virtual uint size() = 0;

protected:
    // data pointer must be freed with a call to FREE_POINTER
    static int GetFileData(const char *fullFilePath, char **data, CCResourceType resourceType, const bool assertOnFail=true);
    static int GetFileSize(const char *fullFilePath, CCResourceType resourceType, const bool assertOnFail=true);

public:
    static void GetFilePath(CCText &fullFilePath, const char *filePath, CCResourceType resourceType);

    static int GetFile(const char *filePath, CCData &fileData, CCResourceType resourceType=Resource_Unknown, const bool assertOnFail=true, struct stat *info=NULL);

    static int GetFileInfo(const char *filePath, CCResourceType resourceType=Resource_Unknown, const bool assertOnFail=true, struct stat *info=NULL);

    static bool SaveCachedFile(const char *filePath, const char *data, const int length);
    static bool DeleteCachedFile(const char *filePath, const bool checkIfExists=true);
    static bool RenameCachedFile(const char *oldFile, CCResourceType resourceType, const char *newFile);

    static bool DeleteFile(const char *filePath, CCResourceType resourceType, const bool checkIfExists=true);

    static void ReadyIO();
    static void DoesCachedFileExistAsync(const char *filePath, CCIOCallback *inCallback);
    static bool DoesFileExist(const char *filePath, CCResourceType resourceType=Resource_Cached);

    static CCResourceType FindFile(const char *filePath);

    static const char* GetAppStorageFolder();
};


#endif // __CCFILEMANAGER_H__
