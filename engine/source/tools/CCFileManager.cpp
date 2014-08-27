/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCFileManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCDeviceFileManager.h"
#include "CCTexture2D.h"

#ifdef DEBUGON
#include <errno.h>
#endif


CCFileManager* CCFileManager::File(CCResourceType resourceType)
{
    return new CCDeviceFileManager( resourceType );
}


// Must provide full path
int CCFileManager::GetFileData(const char *fullFilePath, char **data, CCResourceType resourceType, const bool assertOnFail)
{
    CCDeviceFileManager *file = new CCDeviceFileManager( resourceType );
    if( file->open( fullFilePath ) == false )
    {
        if( assertOnFail )
        {
            DEBUGLOG( "CCFileManager::GetFileData( %s )\n", fullFilePath );
#ifdef DEBUGON
			file->open( fullFilePath );
#endif
            CCASSERT( false );
        }
        delete file;
        return -1;
    }

    const uint fileSize = file->size();
    *data = (char*)malloc( fileSize+1 );
    file->read( *data, fileSize );
    file->close();
    delete file;

    // Add null terminate character to the end
    (*data)[fileSize] = 0;

    return fileSize;
}


int CCFileManager::GetFileSize(const char *fullFilePath, CCResourceType resourceType, const bool assertOnFail)
{
    CCDeviceFileManager *file = new CCDeviceFileManager( resourceType );
    if( file->open( fullFilePath ) == false )
    {
        if( assertOnFail )
        {
            DEBUGLOG( "CCFileManager::GetFileSize( %s )\n", fullFilePath );
#ifdef DEBUGON
			file->open( fullFilePath );
#endif
            CCASSERT( false );
        }
        delete file;
        return -1;
    }

    const uint fileSize = file->size();
    delete file;

    return fileSize;
}


void CCFileManager::GetFilePath(CCText &fullFilePath, const char *filePath, CCResourceType resourceType)
{
    CCText filename = filePath;
    
#ifdef QT

    fullFilePath = QtRootPath().toUtf8().constData();

    if( resourceType == Resource_Packaged )
    {
        filename.stripDirectory();
        
        fullFilePath += "packaged/";
    }
    else
    {
        fullFilePath += "cache/";
    }
    
    fullFilePath += filename;

#elif defined( IOS )

    if( resourceType == Resource_Packaged )
    {
        filename.stripDirectory();
        
        CCText directory = CCDeviceFileManager::GetAppFolder();
        fullFilePath = directory.buffer;
    }
    else if( resourceType == Resource_Temp )
    {
        CCText directory = CCDeviceFileManager::GetTempFolder();
        fullFilePath = directory.buffer;
    }
    else
    {
        CCText directory = GetAppStorageFolder();
        fullFilePath = directory.buffer;
    }
    
    fullFilePath += filename;

#elif defined( ANDROID )

	if( resourceType == Resource_Packaged )
	{
		filename.toLowerCase();
		filename.stripDirectory();
		
		fullFilePath = filename;
	}
	else
	{
		fullFilePath = GetAppStorageFolder();
		fullFilePath += filename.buffer;
	}

#elif defined( WP8 ) || defined WIN8

    if( resourceType == Resource_Packaged )
    {
		filename.stripDirectory();
		
		Platform::String ^location = Platform::String::Concat(Windows::ApplicationModel::Package::Current->InstalledLocation->Path, "\\");
		fullFilePath = GetChars( location );

		fullFilePath += "Resources\\Packaged\\";
    }
    else
    {
		fullFilePath = GetAppStorageFolder();
    }
    
    fullFilePath += filename;

#endif

    // No extra stuff
    if( CCText::Contains( fullFilePath, "?" ) )
    {
        fullFilePath.splitBefore( fullFilePath, "?" );
    }
}


// Must provide relative path for files generated/downloaded by the app
int CCFileManager::GetFile(const char *filePath, CCData &fileData, CCResourceType resourceType, const bool assertOnFail, struct stat *info)
{
    char *data = NULL;
    int fileSize = -1;

    if( resourceType == Resource_Unknown )
    {
        resourceType = FindFile( filePath );
    }

    if( resourceType != Resource_Unknown )
    {
        CCText fullFilePath;
        GetFilePath( fullFilePath, filePath, resourceType );

        if( info != NULL )
        {
            stat( fullFilePath.buffer, info );
        }

        fileSize = GetFileData( fullFilePath.buffer, &data, resourceType, assertOnFail );

        if( fileSize > 0 )
        {
            fileData.set( data, fileSize );
            FREE_POINTER( data );
        }
    }

    return fileSize;
}


// Must provide relative path for files generated/downloaded by the app
int CCFileManager::GetFileInfo(const char *filePath, CCResourceType resourceType, const bool assertOnFail, struct stat *info)
{
    int fileSize;

    if( resourceType == Resource_Unknown )
    {
        resourceType = FindFile( filePath );
    }

    CCText fullFilePath;
    GetFilePath( fullFilePath, filePath, resourceType );

    if( info != NULL )
    {
        stat( fullFilePath.buffer, info );
    }

    fileSize = GetFileSize( fullFilePath.buffer, resourceType, assertOnFail );
    return fileSize;
}


bool CCFileManager::SaveCachedFile(const char *filePath, const char *data, const int length)
{
    CCASSERT( data != NULL );
	if( data == NULL )
	{
		return false;
    }
    
    // Ensure folder exists
    if( CCText::Contains( filePath, "/" ) )
    {
        CCText directorySplit = filePath;
        CCPtrList<char> splitList;
        directorySplit.split( splitList, "/" );
        
        CCText directory = GetAppStorageFolder();
        for( int i=0; i<splitList.length-1; ++i )
        {
            directory += splitList.list[i];
            const int success = mkdir( directory.buffer, 0744 );
#ifdef DEBUGON
            DEBUGLOG( "Created directory %s %i", directory.buffer, success );
#endif
            directory += "/";
        }
    }
    
    CCText fullFilePath;
    GetFilePath( fullFilePath, filePath, Resource_Cached );

#ifdef QT

    QFile file( fullFilePath.buffer );
    if( file.open( QIODevice::WriteOnly ) )
    {
        file.write( data, length );
        file.close();

        return true;
    }

#elif defined( IOS ) || defined( ANDROID )

    //DEBUGLOG( "CCFileManager::Saving %s \n", fullFilePath.buffer );
    FILE *pFile = fopen( fullFilePath.buffer, "w" );
    CCASSERT( pFile != NULL );
    if( pFile != NULL )
    {
        fwrite( data, sizeof( char ), length, pFile );
        fclose( pFile );

#ifdef IOS
        CCDeviceFileManager::DoNotBackupFile( fullFilePath.buffer );
#endif

        return true;
    }

#elif defined WP8 || defined WIN8

	CCDeviceFileManager *file = new CCDeviceFileManager( Resource_Cached );

    CCASSERT( data != NULL );
	bool result = file->save( fullFilePath.buffer, data, length );

	delete file;

	return result;

#endif

    return false;
}


bool CCFileManager::DeleteCachedFile(const char *filePath, const bool checkIfExists)
{
    return CCFileManager::DeleteFile( filePath, Resource_Cached, checkIfExists );
}


bool CCFileManager::RenameCachedFile(const char *oldFile, CCResourceType resourceType, const char *newFile)
{
#ifdef QT

    CCText fullPath;

    fullPath = QtRootPath().toUtf8().constData();
    fullPath += "cache/";

    CCText oldFilename = oldFile;
    CCText oldPath = fullPath;
    oldPath += oldFilename.buffer;

    CCText newFilename = newFile;
    CCText newPath = fullPath;
    newPath += newFilename.buffer;

    CCASSERT( false );

#elif defined( IOS ) || defined( ANDROID )

    CCText oldFilename = oldFile;
    CCText oldPath;
    GetFilePath( oldPath, oldFilename.buffer, resourceType );

    CCText newFilename = newFile;
    CCText newPath;
    GetFilePath( newPath, newFilename.buffer, Resource_Cached );

    //DEBUGLOG( "CCFileManager::Saving %s \n", fullFilePath.buffer );
    const int result = rename( oldPath.buffer, newPath.buffer );
    if( result != 0 )
    {
#ifdef DEBUGON
    	DEBUGLOG( "CCFileManager::RenameCachedFile error\n%s\n%s\n%s\n", strerror( errno ), oldPath.buffer, newPath.buffer );
        CCASSERT( result == 0 );
#endif
    }
    else
    {
#ifdef IOS
        CCDeviceFileManager::DoNotBackupFile( newPath.buffer );
#endif
        return true;
    }

#elif defined WP8 || defined WIN8

	CCASSERT( false );
    
#endif
    
    return false;
}


class IOPriorityCallback : public CCLambdaCallback
{
public:
    IOPriorityCallback(const int inPriority)
    {
        priority = inPriority;
    }
    int priority;
};


bool CCFileManager::DeleteFile(const char *filePath, CCResourceType resourceType, const bool checkIfExists)
{
    if( checkIfExists )
    {
        if( !CCFileManager::DoesFileExist( filePath, resourceType ) )
        {
            return false;
        }
    }

    CCText fullFilePath;
    GetFilePath( fullFilePath, filePath, resourceType );

#ifdef QT

    if( QFile::remove( fullFilePath.buffer ) == false )
    {
        CCASSERT( false );
        return false;
    }

#elif defined( IOS ) || defined( ANDROID )

    //DEBUGLOG( "CCFileManager::deleteCachedFile %s \n", fullFilePath.buffer );
    if( remove( fullFilePath.buffer ) != 0 )
    {
        CCASSERT( false );
        return false;
    }

#elif defined WP8 || defined WIN8

	CCDeviceFileManager *file = new CCDeviceFileManager( resourceType );
	bool result = file->deleteFile( fullFilePath.buffer );

	delete file;

	return result;

#else

    CCASSERT( false );
    return false;
    
#endif
    
    return true;
}


static CCPtrList<IOPriorityCallback> pendingIO;
static int numberOfIORequests = 0;
#define MAX_IO_PER_FRAME 3
void CCFileManager::ReadyIO()
{
    numberOfIORequests = 0;

    while( pendingIO.length > 0 )
    {
        if( numberOfIORequests < MAX_IO_PER_FRAME )
        {
            IOPriorityCallback *ioCallback = pendingIO.list[0];
            pendingIO.remove( ioCallback );
            ioCallback->safeRun();
            delete ioCallback;
        }
        else
        {
            break;
        }
    }

#if 0 && defined DEBUGON
    static int currentHighPriorityIO = 0;
    static int maxHighPrirityIO = 0;
    for( int i=0; i<pendingIO.length; ++i )
    {
        IOPriorityCallback *ioCallback = pendingIO.list[i];
        if( ioCallback->priority > 0 )
        {
            currentHighPriorityIO = i;
        }
    }
    LOG_NEWMAX( "Highest Priority IO remaining", maxHighPrirityIO, currentHighPriorityIO );
#endif
}


void CCFileManager::DoesCachedFileExistAsync(const char *filePath, CCIOCallback *inCallback)
{
    inCallback->filePath = filePath;

    class FileExistsCallback : public IOPriorityCallback
    {
    public:
        FileExistsCallback(CCIOCallback *inCallback) :
            IOPriorityCallback( inCallback->priority )
        {
            this->ioCallback = inCallback;
        }

        void run()
        {
            if( ioCallback->isCallbackActive() == false )
            {
                return;
            }

            CCASSERT( priority == ioCallback->priority );

            CCText fullFilePath;
            GetFilePath( fullFilePath, ioCallback->filePath.buffer, Resource_Cached );

            numberOfIORequests++;
#ifdef WIN8
            const int result = _access( fullFilePath.buffer, F_OK );
#else
            const int result = access( fullFilePath.buffer, F_OK );
#endif
            ioCallback->exists = result == 0;

            CCASSERT( priority == ioCallback->priority );

            ioCallback->safeRun();
        }

    private:
        CCIOCallback *ioCallback;
    };


    FileExistsCallback *fileExistsCallback = new FileExistsCallback( inCallback );
    if( fileExistsCallback->priority > 0 && numberOfIORequests < MAX_IO_PER_FRAME )
    {
        fileExistsCallback->run();
        delete fileExistsCallback;
    }
    else
    {
        pendingIO.add( fileExistsCallback );

        // Can't do this as it'll mess up the order they we're requested
//        for( int i=0; i<pendingIO.length; ++i )
//        {
//            IOPriorityCallback *ioRequest = pendingIO.list[i];
//            if( ioRequest->priority < fileExistsCallback->priority )
//            {
//                pendingIO.reinsert( fileExistsCallback, i );
//                break;
//            }
//        }
    }
}


bool CCFileManager::DoesFileExist(const char *filePath, CCResourceType resourceType)
{
    //DEBUGLOG( "CCFileManager::DoesFileExist() %s\n", filePath );

    CCText fullFilePath;
    GetFilePath( fullFilePath, filePath, resourceType );

#ifdef ANDROID

    if( CCText::Contains( filePath, ".png" ) || CCText::Contains( filePath, ".jpg" ) )
    {
        return CCTexture2D::DoesTextureExist( filePath, resourceType );
    }
    else
    {
        CCDeviceFileManager *file = new CCDeviceFileManager( resourceType );
        const bool exists = file->open( fullFilePath.buffer );
        if( exists )
        {
            file->close();
        }
        delete file;

        return exists;
    }

#elif defined WP8 || defined WIN8

    CCDeviceFileManager *file = new CCDeviceFileManager( resourceType );
	const bool exists = file->exists( fullFilePath.buffer );
    if( exists )
    {
        file->close();
    }
    delete file;

    return exists;

#else

    const int result = access( fullFilePath.buffer, F_OK );
    return result == 0;

#endif
}



CCResourceType CCFileManager::FindFile(const char *filePath)
{
	if( DoesFileExist( filePath, Resource_Cached ) )
    {
        return Resource_Cached;
    }
    else if( DoesFileExist( filePath, Resource_Packaged ) )
    {
        return Resource_Packaged;
    }
#ifdef IOS
    else if( DoesFileExist( filePath, Resource_Temp ) )
    {
        return Resource_Temp;
    }
#endif

    return Resource_Unknown;
}



const char* CCFileManager::GetAppStorageFolder()
{
#ifdef ANDROID
    return CCDeviceFileManager::dataPath.buffer;
#elif defined IOS
    return CCDeviceFileManager::GetDocsFolder();
#elif defined( WP8 ) || defined WIN8
    Platform::String ^location = Platform::String::Concat(Windows::Storage::ApplicationData::Current->LocalFolder->Path, "\\");
    return GetChars( location );
#else
    CCASSERT( false );
#endif
}
