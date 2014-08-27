/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCallbacks.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"


// CCActiveAllocation
CCPtrList<void> CCActiveAllocation::ActiveAllocations;
long CCActiveAllocation::NextAllocationID = 0;


CCActiveAllocation::CCActiveAllocation()
{
    CCNativeThreadLock();

#ifdef DEBUGON
    for( int i=0; i<ActiveAllocations.length; ++i )
    {
        if( ActiveAllocations.list[i] == this )
        {
            CCASSERT( false );
            break;
        }
    }
#endif

    lazyPointer = this;
    lazyID = NextAllocationID++;
    ActiveAllocations.add( lazyPointer );
    CCNativeThreadUnlock();
}


CCActiveAllocation::~CCActiveAllocation()
{
    CCNativeThreadLock();

    ActiveAllocations.remove( this );

#ifdef DEBUGON
    for( int i=0; i<ActiveAllocations.length; ++i )
    {
        if( ActiveAllocations.list[i] == this )
        {
            CCASSERT( false );
            break;
        }
    }
#endif

    CCNativeThreadUnlock();
}


bool CCActiveAllocation::IsCallbackActive(void *pendingPointer, const long pendingID)
{
    CCNativeThreadLock();
    for( int i=0; i<ActiveAllocations.length; ++i )
    {
        void *activeAllocation = ActiveAllocations.list[i];
        if( activeAllocation == pendingPointer )
        {
            // Perhaps the same pointer will be added later on
            // which will lead to an incorrect call?
            // Let's cross check with the callback's timestamp
            if( ((CCActiveAllocation*)activeAllocation)->lazyID == pendingID )
            {
                CCNativeThreadUnlock();
                return true;
            }

#ifdef DEBUGON

            else
            {
                //DEBUGLOG( "Detected that the callback has been deleted and the memory has been replaced by another callback\n" );
            }

#else

            break;

#endif
        }
    }
    CCNativeThreadUnlock();
    return false;
}

