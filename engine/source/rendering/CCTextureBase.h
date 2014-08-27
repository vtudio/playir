/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureBase.h
 * Description : Represents a texture.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTEXTUREBASE_H__
#define __CCTEXTUREBASE_H__


struct CCTextureLoadOptions;


class CCTextureName
{
public:
	CCTextureName()
	{
		glName = 0;
	}

	GLuint name() const { return glName; }

protected:
    GLuint glName;
};


class CCTextureBase : public CCTextureName, public virtual CCActiveAllocation
{
protected:
    uint32_t imageWidth, imageHeight;
    uint32_t allocatedWidth, allocatedHeight;
    uint32_t allocatedBytes;



public:
    CCTextureBase();
    virtual ~CCTextureBase();

    void loadAndCreateAsync(const char *path, const CCResourceType resourceType, const CCTextureLoadOptions options, CCLambdaSafeCallback *callback);

    bool loadAndCreateSync(const char *path, const CCResourceType resourceType, const CCTextureLoadOptions options);

#ifndef QT
protected:
#else
    virtual bool loadData(const char *data, const int length) = 0;
#endif
    virtual bool load(const char *path, const CCResourceType resourceType) = 0;
    virtual void createGLTexture(const CCTextureLoadOptions options) = 0;

public:
    float getImageWidth() const { return (float)imageWidth; }
    float getImageHeight() const { return (float)imageHeight; }

    virtual float getRawWidth() const { return (float)imageWidth; }
    virtual float getRawHeight() const { return (float)imageHeight; }

    float getAllocatedWidth() const { return (float)allocatedWidth; }
    float getAllocatedHeight() const { return (float)allocatedHeight; }
    int getBytes() { return allocatedBytes; }

    static bool ExtensionSupported(const char* extension);
};


#endif // __CCTEXTUREBASE_H__
