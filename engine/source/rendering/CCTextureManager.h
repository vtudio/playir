/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureManager.h
 * Description : Manages the loading and setting of textures.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTEXTUREMANAGER_H__
#define __CCTEXTUREMANAGER_H__


#include "CCTextureBase.h"
class CCTextureFontPage;
struct CCTextureSprites;


struct CCTextureLoadOptions
{
    CCTextureLoadOptions(const bool asyncLoad=true, const bool alwaysResident=false)
    {
        filter = GL_LINEAR;
        disableMipMapping = false;
        this->asyncLoad = asyncLoad;
        this->alwaysResident = alwaysResident;
    }
    int filter;
    bool disableMipMapping;
    bool asyncLoad;
    bool alwaysResident;

    bool equals(const CCTextureLoadOptions &options) const
    {
        return filter == options.filter && 
            disableMipMapping == options.disableMipMapping;
    }
};

struct CCTextureHandle
{
    CCText filePath;
    CCResourceType resourceType;
	CCTextureBase *texture;

    bool loading;
    bool loadable;

	CCTextureLoadOptions options;

    float lastTimeUsed;
    CCLAMBDA_SIGNAL onLoad;

    CCTextureHandle(const char *inFilePath, const CCResourceType inResourceType)
    {
        filePath = inFilePath;
        resourceType = inResourceType;
		texture = NULL;
        loading = false;
        loadable = true;
        lastTimeUsed = 0.0f;
	}

	~CCTextureHandle();

	void deleteTexture(const bool reduceMemory=true);
};


class CCTextureManager : public virtual CCActiveAllocation
{
	friend class CCTextureHandle;

protected:
	const CCTextureName *currentGLTexture;
    int totalTexturesLoaded;
    int totalUsedTextureSpace;

	CCPtrList<CCTextureHandle> textureHandles;
    CCPtrList<CCTextureHandle> recreatingTextureHandles;



public:
	CCTextureManager();
	~CCTextureManager();

    void invalidateAllTextureHandles();		// Deletes OpenGL handles (usually done after a context reset)
protected:
    void recreatedTexture(CCTextureHandle *handle);
public:
    bool isReady();

    void loadFont(const char *name, const uint textureIndex, const char *csv);

    uint assignTextureIndex(const char *filePath, const CCResourceType resourceType,
                            const CCTextureLoadOptions options=CCTextureLoadOptions());

    CCTextureHandle* getTextureHandle(const char *filePath, const CCResourceType resourceType, const CCTextureLoadOptions options=CCTextureLoadOptions());
    CCTextureHandle* getTextureHandle(const int handleIndex);
    void deleteTextureHandle(const char *filePath);
    void invalidateTextureHandle(const char *filePath);

    void loadTextureAsync(CCTextureHandle &textureHandle, CCLambdaSafeCallback *callback=NULL);
    void loadTextureSync(CCTextureHandle &textureHandle, CCLambdaSafeCallback *callback=NULL);
    void loadTextureFailed(CCTextureHandle &textureHandle, CCTextureBase **texture);
    void loadedTexture(CCTextureHandle &textureHandle, CCTextureBase *texture);

    void trim();

    // Used for direct OpenGL access binding
	void bindTexture(const CCTextureName *texture);
    const CCTextureName* getCurrentGLTexture() { return currentGLTexture; }

    // Used for assignging textures
    bool setTextureIndex(const int textureIndex);

    CCTextureBase* getTexture(const int handleIndex, CCLambdaSafeCallback *callback, const bool async=true);

	CCPtrList<CCTextureFontPage> fontPages;
    CCTextureSprites *textureSprites;
};


#endif // __CCTEXTUREMANAGER_H__
