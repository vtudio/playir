/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCArray.h
 * Description : Contains base structures.
 *
 * Created     : 01/03/10
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCARRAY_H__
#define __CCARRAY_H__


// A list containing pointers which aren't deleted on release
template <typename T> class CCPtrList
{
public:
    CCPtrList(const uint size=0)
    {
        list = NULL;
        length = 0;
        allocated = 0;

        if( size > 0 )
        {
            allocate( size );
        }
    }

    ~CCPtrList()
    {
        freeList();
    }

    void allocate(const int size)
    {
        allocated = size;

        if( list != NULL )
        {
            ::free( list );
        }

        const uint sizeOfPointer = sizeof( void* );
        list = (T**)malloc( sizeOfPointer * allocated );
        CCASSERT( list != NULL );
    }

    void add(T *object, const int index=-1)
    {
        //CCASSERT( find( object ) == -1 );
        if( length == allocated )
        {
            allocated += 16;
            const uint sizeOfPointer = sizeof( void* );
            T **newArray = (T**)malloc( sizeOfPointer * allocated );
            CCASSERT( newArray != NULL );

            if( list != NULL )
            {
                for( int i=0; i<length; ++i )
                {
                    newArray[i] = list[i];
                }

                ::free( list );
            }

            newArray[length++] = object;
            list = newArray;
        }
        else
        {
            list[length++] = object;
        }

        if( index >= 0 )
        {
            reinsert( object, index );
        }
    }

    void addOnce(T *object)
    {
        if( find( object ) == -1 )
        {
            add( object );
        }
    }

    void prioritiseIndex(const int index)
    {
        if( index > 0 && index < length)
        {
            T *temp = list[index-1];
            list[index-1] = list[index];
            list[index] = temp;
        }
    }

    bool removeIndex(const int index)
    {
        if( index >= 0 && index < length )
        {
            length--;
            for( int i=index; i<length; ++i )
            {
                list[i] = list[i+1];
            }
            return true;
        }

        return false;
    }

    bool remove(const void *object)
    {
        int index = find( object );
        if( index != -1 )
        {
            CCASSERT( length > 0 );
            return removeIndex( index );
        }

        return false;
    }

    void clear()
    {
        length = 0;
    }

    void freeList()
    {
        if( list != NULL )
        {
            free( list );
            list = NULL;
        }
        length = 0;
        allocated = 0;
    }

    void deleteObjects()
    {
        for( int i=0; i<length; ++i )
        {
            delete list[i];
        }
        length = 0;
    }

    void deleteObjectsAndList()
    {
        deleteObjects();
        freeList();
    }

    void resize(const int size)
    {
        // If it's too small
        while( length < size )
        {
            add( new T() );
        }

        // If it's too big
        for( int i=size; i<length; ++i )
        {
            delete list[i];
        }
        length = size;
    }

    void copy(CCPtrList<T> &other)
    {
        for( int i=0; i<other.length; ++i )
        {
            add( other.list[i] );
        }
    }

    void push(T *object)
    {
        add( object );
        reinsert( object, 0 );
    }

    T* pop()
    {
        CCASSERT( length > 0 );
        T *object = list[0];
        remove( object );
        return object;
    }

    // Re-insert from the back of the queue
    void reinsert(T *object, const int index)
    {
        CCASSERT( list[length-1] == object );
        for( int i=length-1; i>index; --i )
        {
            list[i] = list[i-1];
        }
        list[index] = object;
    }

    int find(const void *object)
    {
        for( int i=0; i<length; ++i )
        {
            if( list[i] == object )
            {
                return i;
            }
        }
        return -1;
    }

    T* first()
    {
        if( length > 0 )
        {
            return list[0];
        }
        return NULL;
    }

    T* last() const
    {
        CCASSERT( length > 0 );
        return list[length-1];
    }

    T **list;
    int length;

protected:
    int allocated;
};


// A list containing objects that deleted using the destruct call on release
template <typename T> class CCObjectPtrList : public CCPtrList<T>
{
public:
    void deleteObjects(const bool reversed=false)
    {
        if( reversed )
        {
            for( int i=this->length-1; i>=0; --i )
            {
                T *object = this->list[i];
                DELETE_OBJECT( object );
            }
        }
        else
        {
            for( int i=0; i<this->length; ++i )
            {
                T *object = this->list[i];
                DELETE_OBJECT( object );
            }
        }
        this->length = 0;
    }

	void deleteObjectsAndList(const bool reversed=false)
	{
		if( this->list != NULL )
		{
            deleteObjects( reversed );
            this->freeList();
		}
	}
};


// Always feed on destruct
template <typename T1, typename T2> class CCPairList
{
public:
    void add(T1 *name, T1 *value)
    {
        names.add( name );
        values.add( value );
    }

    int length() const { return names.length; }

    ~CCPairList()
    {
        names.deleteObjects();
        values.deleteObjects();
    }

    CCPtrList< T1 > names;
    CCPtrList< T2 > values;
};


#endif // __CCARRAY_H__
