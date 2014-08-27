/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCExternalAccessoryManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCAppManager.h"

#ifdef IOS
static CCDeviceExternalAccessoryManager *gEAManager = NULL;
#endif

CCPtrList<CCExternalAccessory> CCExternalAccessoryManager::accessoryList;
CCAccessoryReader *CCExternalAccessoryManager::accessoryReader;


void CCExternalAccessoryManager::Reset()
{
    CCNativeThreadLock();
    
    accessoryList.deleteObjects();

#ifdef IOS
    if( gEAManager == NULL )
    {
        gEAManager = [[CCDeviceExternalAccessoryManager alloc] init];
    }
    [gEAManager resetAccessories];
#endif

    accessoryReader = NULL;
    
    CCNativeThreadUnlock();
}


void CCExternalAccessoryManager::Stop()
{
#ifdef IOS
    if( gEAManager != NULL )
    {
        [gEAManager release];
        gEAManager = NULL;
    }
#endif
}


bool CCExternalAccessoryManager::openSession()
{
#ifdef IOS
    return [gEAManager openSession];
#endif

	return false;
}


void CCExternalAccessoryManager::closeSession()
{
#ifdef IOS
    [gEAManager closeSession];
#endif
}


void CCExternalAccessoryManager::setupControllerForAccessory(CCExternalAccessory *accessory, const char *protocol)
{
#ifdef IOS
    [gEAManager setupControllerForAccessory:accessory->accessory withProtocolString:protocol];
#endif
}


void CCExternalAccessoryManager::addAccessory(EAAccessory *inAccessory)
{
#ifdef IOS
    CCNativeThreadLock();
    
    EAAccessory *eaAccessory = inAccessory;
    const uint connectionID = [eaAccessory connectionID];

    for( int i=0; i<accessoryList.length; ++i )
    {
        CCExternalAccessory *accessory = accessoryList.list[i];
        if( accessory->connectionID == connectionID )
        {
            CCNativeThreadUnlock();
            return;
        }
    }

    CCExternalAccessory *accessory = new CCExternalAccessory();
    accessory->accessory = inAccessory;
    accessory->connectionID = connectionID;

    NSArray *protocols = [eaAccessory protocolStrings];
    const int protocolsLength = [protocols count];
    for( int i=0; i<protocolsLength; ++i )
    {
        NSString *protocol = [protocols objectAtIndex:i];
        accessory->protocolStrings.add( new CCText( [protocol UTF8String] ) );
    }

    accessoryList.add( accessory );
    
    CCNativeThreadUnlock();
#endif
}


void CCExternalAccessoryManager::removeAccessory(EAAccessory *inAccessory)
{
    CCNativeThreadLock();
    
    uint connectionID = 0;

#ifdef IOS
    EAAccessory *eaAccessory = inAccessory;
    connectionID = [eaAccessory connectionID];
#endif

    for( int i=0; i<accessoryList.length; ++i )
    {
        CCExternalAccessory *accessory = accessoryList.list[i];
        if( accessory->connectionID == connectionID )
        {
            accessoryList.remove( accessory );
            delete accessory;
            break;
        }
    }
    
    CCNativeThreadUnlock();
}
