/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCameraRecorder.h
 * Description : Manages camera input.
 *
 * Created     : 27/09/13
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCCAMERAMANAGER_H__
#define __CCCAMERAMANAGER_H__


class CCCameraRecorder : public virtual CCActiveAllocation
{
public:
    struct CameraRecording
    {
        CameraRecording()
        {
            name = NULL;
            textureIndex = 0;
            vertices = NULL;
            uvs = NULL;
            vertexCount = 0;
        }

        const char *name;
        uint textureIndex;
        float *vertices;
        float *uvs;
        uint vertexCount;

        CCPtrList<class CCJSPrimitiveOBJ> linked;
    };

protected:
    CameraRecording current;
    CameraRecording loading;



public:
    CCCameraRecorder();

    void updateJobsThread();

    CameraRecording& getCurrent()
    {
        return current;
    }
    
    void unlink(void *pointer);
};


#endif // __CCCAMERAMANAGER_H__
