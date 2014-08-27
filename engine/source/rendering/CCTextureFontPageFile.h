/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCTextureFontPageFile.h
 * Description : Handles loadng font description files.
 *
 * Created     : 20/04/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCTEXTUREFONTPAGEFILE_H__
#define __CCTEXTUREFONTPAGEFILE_H__


#include "CCTextureFontPage.h"

class CCTextureFontPageFile : public CCTextureFontPage
{
public:
	typedef CCTextureFontPage super;

	CCTextureFontPageFile(const char *name);
	virtual ~CCTextureFontPageFile();

    virtual bool load(const int textureIndex, const char *csv);

protected:
	virtual void bindTexturePage() const;
    
public:
    const uint getTextureIndex() { return textureIndex; }
    const char* getCSV() { return csv.buffer; }

protected:
	uint textureIndex;
    CCText csv;
};


#endif // __CCTEXTUREFONTPAGEFILE_H__
