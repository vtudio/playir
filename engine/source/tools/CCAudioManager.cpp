/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCAudioManager.cpp
 *-----------------------------------------------------------
 */

#include "CCDefines.h"
#include "CCAppManager.h"
#include "CCDeviceAudioManager.h"


void CCAudioManager::Reset()
{
    CCDeviceAudioManager::Reset();
}


void CCAudioManager::Prepare(const char *id, const char *url)
{
    CCDeviceAudioManager::Prepare( id, url );
}


void CCAudioManager::Play(const char *id, const char *url, const bool restart, const bool loop)
{
    CCDeviceAudioManager::Play( id, url, restart, loop );
}


void CCAudioManager::Stop(const char *id)
{
    CCDeviceAudioManager::Stop( id );
}


void CCAudioManager::Pause(const char *id)
{
    CCDeviceAudioManager::Pause( id );
}


void CCAudioManager::Resume(const char *id)
{
    CCDeviceAudioManager::Resume( id );
}


void CCAudioManager::SetTime(const char *id, const float time)
{
    CCDeviceAudioManager::SetTime( id, time );
}


void CCAudioManager::SetVolume(const char *id, const float volume)
{
    CCDeviceAudioManager::SetVolume( id, volume );
}


void CCAudioManager::Ended(const char *id, const char *url)
{
    if( gEngine != NULL )
    {
		gEngine->audioEnded( id, url );
    }
}
