/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCDefines.h
 * Description : Includes the generic library headers.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCDEFINES_H__
#define __CCDEFINES_H__


#include "CCPlatform.h"

#define MAX_OBJECTS 2048

#include "CCVectors.h"
#include "CCMathTools.h"
#include "CCTools.h"
#include "CCTypes.h"
#include "CCCallbacks.h"
#include "CCRenderTools.h"
#include "CCCollisionTools.h"
#include "CCInterpolators.h"

#include "CCApp.h"
extern CCAppEngine *gEngine;

#ifdef WP8
// Used to communicate with JS
struct JSAction
{
	CCText action;
	CCLambdaSafeCallback *callback;

	JSAction(const char *key, const char *value, CCLambdaSafeCallback *callback)
	{
		action = key;
		if( value != NULL )
		{
			action += value;
		}

		this->callback = callback;
	}

	~JSAction()
	{
		DELETE_POINTER( callback );
	}
};
extern CCPtrList<JSAction> jsActionStack;

// Used to communicate with C#
struct CSAction
{
	CCText action;

	CSAction(const char *key, const char *value=NULL)
	{
		action = key;
		if( value != NULL )
		{
			action += value;
		}
	}
};
extern CCPtrList<CSAction> csActionStack;
#endif

#if defined WIN8 || defined WP8
extern const char* GetChars(Platform::String^ string);
extern Platform::String^ GetString(const char *c);
#endif


#endif // __CCDEFINES_H__
