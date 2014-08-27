/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCBaseTypes.h
 * Description : Contains base functions.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCJSON_H__
#define __CCJSON_H__


#include "jansson.h"


class CCJSONCallback : public CCLambdaCallback
{
public:
    json_t *jsonData;
};


static bool json_object_string(CCText &result, json_t *jsonObject, const char *tag=NULL, const bool assertOnFail=true)
{
    json_t *jsonValue = tag != NULL ? json_object_get( jsonObject, tag ) : jsonObject;
    if( jsonValue != NULL )
    {
        if( jsonValue->type == JSON_STRING )
        {
            const char *jsonStringValue = json_string_value( jsonValue );
            result = jsonStringValue;
            return true;
        }
        else if( jsonValue->type == JSON_INTEGER )
        {
            const json_int_t jsonIntValue = json_integer_value( jsonValue );
            result += jsonIntValue;
            return true;
        }
        else if( jsonValue->type == JSON_REAL )
        {
            const float jsonFloatValue = (float)json_real_value( jsonValue );
            result += jsonFloatValue;
            return true;
        }
        else if( jsonValue->type == JSON_TRUE )
        {
            result += "true";
            return true;
        }
        else if( jsonValue->type == JSON_FALSE )
        {
            result += "false";
            return true;
        }
    }

    if( assertOnFail )
    {
        CCASSERT( false );
    }

    return false;
}


static json_int_t json_object_int64(json_t *jsonObject, const char *tag, const bool assertOnFail=true)
{
    json_t *jsonValue = tag != NULL ? json_object_get( jsonObject, tag ) : jsonObject;
    if( jsonValue != NULL )
    {
        if( jsonValue->type == JSON_INTEGER )
        {
            const json_int_t jsonIntValue = json_integer_value( jsonValue );
            return jsonIntValue;
        }
        else if( jsonValue->type == JSON_STRING )
        {
            const char *jsonStringValue = json_string_value( jsonValue );
#if JSON_INTEGER_IS_LONG_LONG
            const json_int_t jsonIntValue = atoll( jsonStringValue );
#else
            const json_int_t jsonIntValue = atol( jsonStringValue );
#endif
            return jsonIntValue;
        }
    }

    if( assertOnFail )
    {
        CCASSERT( false );
    }

    return -1;
}


static int json_object_int(json_t *jsonObject, const char *tag=NULL, const bool assertOnFail=true)
{
    return (int)json_object_int64( jsonObject, tag, assertOnFail );
}


static float json_object_float(json_t *jsonObject, const char *tag=NULL, const bool assertOnFail=true)
{
    json_t *jsonValue = tag != NULL ? json_object_get( jsonObject, tag ) : jsonObject;
    if( jsonValue != NULL )
    {
        if( jsonValue->type == JSON_STRING )
        {
            const char *jsonStringValue = json_string_value( jsonValue );
            if( jsonStringValue != NULL )
            {
                const float jsonFloatValue = (float)atof( jsonStringValue );
                return jsonFloatValue;
            }
        }
        else
        {
            if( jsonValue->type == JSON_INTEGER )
            {
                const json_int_t jsonIntValue = json_integer_value( jsonValue );
                return (float)jsonIntValue;
            }
            else if( jsonValue->type == JSON_REAL )
            {
                const float jsonFloatValue = (float)json_real_value( jsonValue );
                return jsonFloatValue;
            }
            else if( jsonValue->type == JSON_NULL )
            {
                return 0.0f;
            }
        }
    }

    if( assertOnFail )
    {
        CCASSERT( false );
    }

    return -1.0f;
}


static bool json_object_bool(json_t *jsonObject, const char *tag)
{
    json_t *jsonValue = json_object_get( jsonObject, tag );
    if( jsonValue != NULL )
    {
        if( jsonValue->type == JSON_TRUE )
        {
            return true;
        }
    }
    return false;
}


#endif // __CCJSON_H__
