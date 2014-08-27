/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCExternalAccessoryManager.h
 * Description : Manages external accessories.
 *
 * Created     : 17/01/14
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCEXTERNALACCESSORYMANAGER_H_
#define __CCEXTERNALACCESSORYMANAGER_H_


#ifdef IOS
    #ifdef __OBJC__
        #include "CCDeviceExternalAccessoryManager.h"
    #else
        #define EAAccessory void
    #endif
#else
#define EAAccessory void
#endif


class CCAccessoryReader : public virtual CCActiveAllocation
{
public:
    virtual void readUpdate(const CCData &bytes) = 0;
};


class CCExternalAccessory
{
public:
    ~CCExternalAccessory()
    {
        protocolStrings.deleteObjects();
    }

    EAAccessory *accessory;
    uint connectionID;
    CCPtrList<CCText> protocolStrings;
};


class CCExternalAccessoryManager
{
protected:
    static CCPtrList<CCExternalAccessory> accessoryList;

public:
    static CCAccessoryReader *accessoryReader;


public:
    static void Reset();
    static void Stop();
    
    static const CCPtrList<CCExternalAccessory>& getAccessoryList()
    {
        return accessoryList;
    }

    static bool openSession();
    static void closeSession();

    static void setupControllerForAccessory(CCExternalAccessory *accessory, const char *protocol);

    static void addAccessory(EAAccessory *inAccessory);
    static void removeAccessory(EAAccessory *inAccessory);
};


#endif // __CCEXTERNALACCESSORYMANAGER_H_