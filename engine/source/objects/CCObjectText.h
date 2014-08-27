/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCModelText.h
 * Description : Represents a 3d text primitive.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

// Here's a model that contains the text primitive to make things easier
class CCObjectText : public CCObject
{
    typedef CCObject super;

protected:
	struct CCText text;

    CCCollideable *parent;

    CCTextureFontPage *fontPage;
    float height;
    bool centered;
    bool endMarker;

public:
    const char *shader;



public:
    CCObjectText(CCCollideable *inParent=NULL);
    CCObjectText(const long jsID);
    virtual void destruct();

	virtual void renderObject(const CCCameraBase *camera, const bool alpha);

    float getWidth();
    float getHeight();
    void setHeight(const float height);

    const CCText& getText() { return text; }
    void setText(const char *text, const float height=-1.0f, const char *font=NULL);
    
    void setCentered(const bool centered);

    void setFont(const char *font);

    void setEndMarker(const bool toggle) { endMarker = toggle; }
    bool getEndMarker() { return endMarker; }
};
