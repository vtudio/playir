/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCString.h
 * Description : Contains base structures.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCSTRING_H__
#define __CCSTRING_H__


struct CCText : CCData
{
    CCText() {}
    explicit CCText(const int inLength);
    CCText(const char *text);
    CCText(const CCText &other);

    bool operator==(const char *other) const;
    bool operator!=(const char *other) const;
    CCText& operator=(const char *text);
    CCText& operator=(const CCText &other);

    void set(const char *text);
    void set(const char *text, const uint inLength);
    void clear();
    void trimLength(const uint maxLength);

    inline static bool Equals(const CCText &text, const CCText &token)
    {
        return Equals( text.buffer, token.buffer );
    }

    inline static bool Equals(const CCText &text, const char *token)
    {
        return Equals( text.buffer, token );
    }
    
    inline static bool Equals(const char *buffer, const char *token)
    {
        if( buffer == NULL && token == NULL )
        {
            return true;
        }
        if( buffer != NULL && token != NULL )
        {
            return strcmp( buffer, token ) == 0;
        }
        return false;
    }

    inline static bool Contains(const CCText &text, const CCText &token)
    {
        return Contains( text.buffer, token.buffer );
    }

    inline static bool Contains(const CCText &text, const char *token)
    {
        return Contains( text.buffer, token);
    }

    inline static bool Contains(const char *buffer, const char *token)
    {
        if( buffer != NULL && token != NULL )
        {
            return strstr( buffer, token ) != NULL;
        }
        return false;
    }

    static bool StartsWith(const char *buffer, const char *token);
    void stripExtension();
    void stripFile();
    void stripDirectory(const bool windowsDirectories=false);
    void strip(const char *token);
    void toLowerCase();
    static void SetLastWord(const char *inBuffer, CCText &outText);

    const char* getExtension()
    {
        if( length > 4 )
        {
            return buffer+length-4;
        }
        return NULL;
    }

    void replaceChar(const char search, const char replace);
    void replaceChars(const char *token, const char *replace);
    void replaceChars(const char *token, const CCText &replace);
    void replaceChars(const char *token, const char replace);

    // Set the text to be the value between the split tokens
    void split(CCPtrList<char> &splitList, const char *token, const bool first=false);
    void splitBetween(CCText source, const char *from, const char *to);
    void splitBefore(CCText source, const char *before);
    void splitBeforeLast(CCText source, const char *before);
    void splitAfter(CCText source, const char *after);
    void splitAfterLast(CCText source, const char *after);
    void removeBetween(const char *from, const char *to);
    void removeBetweenIncluding(const char *from, const char *to);

    void encodeForWeb();
    void removeNewLines();
};


#endif // __CCSTRING_H__
