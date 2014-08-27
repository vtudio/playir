/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCAudioManager.h
 * Description : Manages audio playback.
 *
 * Created     : 11/07/13
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCAUDIOMANAGER_H__
#define __CCAUDIOMANAGER_H__


class CCAudioManager
{
public:
	static void Reset();

    static void Prepare(const char *id, const char *url);
    static void Play(const char *id, const char *url, const bool restart, const bool loop);
    static void Stop(const char *id);
    static void Pause(const char *id);
    static void Resume(const char *id);
    static void SetTime(const char *id, const float time);
    static void SetVolume(const char *id, const float volume);

    static void Ended(const char *id, const char *url);
};


#endif // __CCAUDIOMANAGER_H_