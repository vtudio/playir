/*-----------------------------------------------------------
 * http://softwareispoetry.com
 *-----------------------------------------------------------
 * This software is distributed under the Apache 2.0 license.
 *-----------------------------------------------------------
 * File Name   : CCCallbacks.h
 * Description : Closure implementation using functors.
 *
 * Created     : 25/06/11
 * Author(s)   : Ashraf Samy Hegab
 *-----------------------------------------------------------
 */

#ifndef __CCCALLBACKS_H__
#define __CCCALLBACKS_H__


// Automatically keeps track of allocations to let you know if it's ok to callback this class
class CCActiveAllocation
{
public:
    CCActiveAllocation();
    virtual ~CCActiveAllocation();
    static bool IsCallbackActive(void *pendingPointer, const long pendingID);

    void *lazyPointer;
    long lazyID;
protected:
    static CCPtrList<void> ActiveAllocations;
    static long NextAllocationID;
};



class CCLambdaCallback
{
public:
    // Use this pointer to pass in parameters for the run function callback
    void *runParameters;


	CCLambdaCallback()
	{
		runParameters = NULL;
	}

    virtual ~CCLambdaCallback() {};

	virtual bool isActive()
	{
		return true;
	}

    virtual void safeRun()
    {
        run();
		finish();
    }

	virtual void safeRunOnly()
    {
        run();
    }

	virtual void safeFinishOnly()
    {
        finish();
    }

protected:
    virtual void run() = 0;
	virtual void finish() {};
};


class CCLambdaSafeCallback : public CCLambdaCallback
{
public:
	virtual bool isActive()
	{
		if( CCActiveAllocation::IsCallbackActive( lazyPointer, lazyID ) )
        {
			return true;
		}
		return false;
	}

    virtual void safeRun()
    {
        if( isActive() )
        {
            run();
			finish();
        }
    }

	virtual void safeRunOnly()
    {
        if( isActive() )
        {
            run();
		}
    }

	virtual void safeFinishOnly()
    {
        if( isActive() )
        {
            finish();
		}
    }

protected:
    void *lazyPointer;
    long lazyID;
};


class CCDataCallback : public CCLambdaCallback
{
public:
    CCData data;
};


class CCTextCallback : public CCLambdaCallback
{
public:
    virtual ~CCTextCallback()
    {
        text.deleteObjects();
    }

    void add(const char *text, const int index=-1)
    {
        if( index == -1 )
        {
            this->text.add( new CCText( text ) );
        }
        else
        {
            while( this->text.length <= index )
            {
                this->text.add( new CCText( text ) );
            }

            *this->text.list[index] = text;
        }
    }

    CCPtrList<CCText> text;
};


// Unsafe means no lazy callback checking to see if the pointer is still active or not
#define CCLAMBDA_UNSAFE(NAME, FUNCTION_CALL)						\
class NAME : public CCLambdaCallback                                \
{                                                                   \
protected:                                                          \
void run()                                                          \
{                                                                   \
FUNCTION_CALL                                                       \
}                                                                   \
};


#define CCLAMBDA_1_UNSAFE(NAME, TYPE1, NAME1, FUNCTION_CALL)		\
class NAME : public CCLambdaCallback                                \
{                                                                   \
public:                                                             \
    NAME(TYPE1 NAME1)                                               \
    {                                                               \
        this->NAME1 = NAME1;                                        \
    }                                                               \
protected:                                                          \
    void run()                                                      \
    {                                                               \
        FUNCTION_CALL                                               \
    }                                                               \
private:                                                            \
    TYPE1 NAME1;                                                    \
};


#define CCLAMBDA_1(NAME, TYPE1, NAME1, FUNCTION_CALL)						\
class NAME : public CCLambdaSafeCallback									\
{																			\
public:																		\
    NAME(TYPE1 *NAME1)														\
    {																		\
        this->NAME1 = NAME1;												\
        this->lazyPointer = NAME1->lazyPointer;								\
        this->lazyID = NAME1->lazyID;										\
    }																		\
protected:                                                                  \
    void run()																\
    {																		\
        FUNCTION_CALL                                                       \
    }																		\
private:																	\
    TYPE1 *NAME1;															\
};


#define CCLAMBDA_2(NAME, TYPE1, NAME1, TYPE2, NAME2, FUNCTION_CALL)			\
class NAME : public CCLambdaSafeCallback									\
{																			\
public:																		\
    NAME(TYPE1 *NAME1, TYPE2 NAME2)											\
    {																		\
        this->NAME1 = NAME1;												\
        this->lazyPointer = NAME1->lazyPointer;								\
        this->lazyID = NAME1->lazyID;										\
        this->NAME2 = NAME2;												\
    }																		\
protected:                                                                  \
    void run()																\
    {																		\
        FUNCTION_CALL                                                       \
    }																		\
private:																	\
    TYPE1 *NAME1;															\
    TYPE2 NAME2;															\
};

#define CCLAMBDA_2_UNSAFE(NAME, T1, N1, T2, N2, FUNCTION_CALL)              \
class NAME : public CCLambdaCallback                                        \
{                                                                           \
public:                                                                     \
    NAME(T1 N1, T2 N2)                                                      \
    {                                                                       \
        this->N1 = N1;                                                      \
        this->N2 = N2;                                                      \
    }                                                                       \
protected:                                                                  \
    void run()                                                              \
    {                                                                       \
        FUNCTION_CALL                                                       \
    }                                                                       \
private:                                                                    \
    T1 N1;                                                                  \
    T2 N2;                                                                  \
};

#define CCLAMBDA_FINISH_2(NAME, TYPE1, NAME1, TYPE2, NAME2, RUN_CALL, FINISH_CALL)	\
class NAME : public CCLambdaSafeCallback											\
{																					\
public:																				\
    NAME(TYPE1 *NAME1, TYPE2 NAME2)													\
    {																				\
        this->NAME1 = NAME1;														\
        this->lazyPointer = NAME1->lazyPointer;										\
        this->lazyID = NAME1->lazyID;												\
        this->NAME2 = NAME2;														\
    }																				\
protected:																			\
    void run()																		\
    {																				\
        RUN_CALL																	\
    }																				\
    void finish()																	\
    {																				\
        FINISH_CALL																	\
    }																				\
private:																			\
    TYPE1 *NAME1;																	\
    TYPE2 NAME2;																	\
};


#define CCLAMBDA_3(NAME, TYPE1, NAME1, TYPE2, NAME2, TYPE3, NAME3, FUNCTION_CALL)       \
class NAME : public CCLambdaSafeCallback												\
{																						\
public:																					\
    NAME(TYPE1 *NAME1, TYPE2 NAME2, TYPE3 NAME3)										\
    {                                                                   				\
        this->NAME1 = NAME1;                                            				\
        this->lazyPointer = NAME1->lazyPointer;                         				\
        this->lazyID = NAME1->lazyID;                                   				\
        this->NAME2 = NAME2;                                            				\
        this->NAME3 = NAME3;                                            				\
    }                                                                   				\
protected:                                                                              \
    void run()                                                          				\
    {                                                                   				\
        FUNCTION_CALL                                                                   \
    }                                                                   				\
private:                                                                				\
    TYPE1 *NAME1;                                                       				\
    TYPE2 NAME2;                                                       					\
    TYPE3 NAME3;                                                       					\
};


#define CCLAMBDA_FINISH_3(NAME, TYPE1, NAME1, TYPE2, NAME2, TYPE3, NAME3, RUN_CALL, FINISH_CALL)	\
class NAME : public CCLambdaSafeCallback															\
{																									\
public:																								\
    NAME(TYPE1 *NAME1, TYPE2 NAME2, TYPE3 NAME3)													\
    {																								\
        this->NAME1 = NAME1;																		\
        this->lazyPointer = NAME1->lazyPointer;														\
        this->lazyID = NAME1->lazyID;																\
        this->NAME2 = NAME2;																		\
        this->NAME3 = NAME3;                                            							\
    }																								\
protected:																							\
    void run()																						\
    {																								\
        RUN_CALL																					\
    }																								\
    void finish()																					\
    {																								\
        FINISH_CALL																					\
    }																								\
private:																							\
    TYPE1 *NAME1;																					\
    TYPE2 NAME2;																					\
    TYPE3 NAME3;                                                       								\
};


#define CCLAMBDA_4(CLASSNAME, T1, N1, T2, N2, T3, N3, T4, N4, FUNCTION_CALL)            \
class CLASSNAME : public CCLambdaSafeCallback											\
{																						\
public:																					\
    CLASSNAME(T1 *N1, T2 N2, T3 N3, T4 N4)                                              \
    {                                                                   				\
        this->N1 = N1;                                            						\
        this->lazyPointer = N1->lazyPointer;                         					\
        this->lazyID = N1->lazyID;                                   					\
        this->N2 = N2;                                            						\
        this->N3 = N3;                                            						\
        this->N4 = N4;                                            						\
    }                                                                   				\
protected:                                                                              \
    void run()                                                          				\
    {                                                                   				\
        FUNCTION_CALL                                                                   \
    }                                                                   				\
private:                                                                				\
    T1 *N1;                                                       						\
    T2 N2;                                                       						\
    T3 N3;                                                       						\
    T4 N4;                                                       						\
};


#define CCLAMBDA_4_UNSAFE(CLASSNAME, T1, N1, T2, N2, T3, N3, T4, N4, FUNCTION_CALL)     \
class CLASSNAME : public CCLambdaCallback                                               \
{																						\
public:																					\
    CLASSNAME(T1 N1, T2 N2, T3 N3, T4 N4)                                               \
    {                                                                   				\
        this->N1 = N1;                                            						\
        this->N2 = N2;                                            						\
        this->N3 = N3;                                            						\
        this->N4 = N4;                                            						\
    }                                                                   				\
protected:                                                                              \
    void run()                                                          				\
    {                                                                   				\
        FUNCTION_CALL                                                                   \
    }                                                                   				\
private:                                                                				\
    T1 N1;                                                       						\
    T2 N2;                                                       						\
    T3 N3;                                                       						\
    T4 N4;                                                       						\
};


#define CCLAMBDA_5(CLASSNAME, T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, FUNCTION_CALL)	\
class CLASSNAME : public CCLambdaSafeCallback											\
{																						\
public:																					\
    CLASSNAME(T1 *N1, T2 N2, T3 N3, T4 N4, T5 N5)										\
    {                                                                   				\
        this->N1 = N1;                                            						\
        this->lazyPointer = N1->lazyPointer;                         					\
        this->lazyID = N1->lazyID;                                   					\
        this->N2 = N2;                                            						\
        this->N3 = N3;                                            						\
        this->N4 = N4;                                            						\
        this->N5 = N5;                                            						\
    }                                                                   				\
protected:                                                                              \
    void run()                                                          				\
    {                                                                   				\
        FUNCTION_CALL                                                                   \
    }                                                                   				\
private:                                                                				\
    T1 *N1;                                                       						\
    T2 N2;                                                       						\
    T3 N3;                                                       						\
    T4 N4;                                                       						\
    T5 N5;                                                       						\
};


#define CCLAMBDA_6(CLASSNAME, T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, T6, N6, FUNCTION_CALL)    \
class CLASSNAME : public CCLambdaSafeCallback                                                   \
{                                                                                               \
public:                                                                                         \
    CLASSNAME(T1 *N1, T2 N2, T3 N3, T4 N4, T5 N5, T6 N6)                                        \
    {                                                                                           \
        this->N1 = N1;                                                                          \
        this->lazyPointer = N1->lazyPointer;                                                    \
        this->lazyID = N1->lazyID;                                                              \
        this->N2 = N2;                                                                          \
        this->N3 = N3;                                                                          \
        this->N4 = N4;                                                                          \
        this->N5 = N5;                                                                          \
        this->N6 = N6;                                                                          \
    }                                                                                           \
protected:                                                                                      \
    void run()                                                                                  \
    {                                                                                           \
        FUNCTION_CALL                                                                           \
    }                                                                                           \
private:                                                                                        \
    T1 *N1;                                                                                     \
    T2 N2;                                                                                      \
    T3 N3;                                                                                      \
    T4 N4;                                                                                      \
    T5 N5;                                                                                      \
    T6 N6;                                                                                      \
};


#define CCLAMBDA_FINISH_6(NAME, T1, N1, T2, N2, T3, N3, T4, N4, T5, N5, T6, N6, RUN_CALL, FINISH_CALL)  \
class NAME : public CCLambdaSafeCallback                                                                \
{                                                                                                       \
public:                                                                                                 \
    NAME(T1 *N1, T2 N2, T3 N3, T4 N4, T5 N5, T6 N6)                                                     \
    {                                                                                                   \
        this->N1 = N1;                                                                                  \
        this->lazyPointer = N1->lazyPointer;                                                            \
        this->lazyID = N1->lazyID;                                                                      \
        this->N2 = N2;                                                                                  \
        this->N3 = N3;                                                                                  \
        this->N4 = N4;                                                                                  \
        this->N5 = N5;                                                                                  \
        this->N6 = N6;                                                                                  \
    }                                                                                                   \
protected:                                                                                              \
    void run()                                                                                          \
    {                                                                                                   \
        RUN_CALL                                                                                        \
    }                                                                                                   \
    void finish()                                                                                       \
    {                                                                                                   \
        FINISH_CALL                                                                                     \
    }                                                                                                   \
private:                                                                                                \
    T1 *N1;                                                                                             \
    T2 N2;                                                                                              \
    T3 N3;                                                                                              \
    T4 N4;                                                                                              \
    T5 N5;                                                                                              \
    T6 N6;                                                                                              \
};



// Event stiching

// Defines a list of callbacks
#define CCLAMBDA_SIGNAL CCPtrList<CCLambdaCallback>


// Runs through the list running all the callbacks
#define CCLAMBDA_EMIT(LIST1)											\
if( LIST1.length > 0 )													\
{																		\
    for( int i=0; i<LIST1.length; ++i )									\
    {																	\
        LIST1.list[i]->safeRun();                                       \
    }                                                                   \
}


// Runs through the list running all the callbacks then clears them
#define CCLAMBDA_EMIT_ONCE(LIST1)										\
if( LIST1.length > 0 )													\
{																		\
    for( int i=0; i<LIST1.length; ++i )									\
    {																	\
        LIST1.list[i]->safeRun();                                       \
    }																	\
    LIST1.deleteObjectsAndList();										\
}


// Connects an event list with a function call of a class
#define CCLAMBDA_CONNECT_CLASS(LIST1, TYPE1, POINTER1, FUNCTION_CALL)	\
{																		\
    CCLAMBDA_1( EventCallback, TYPE1, NAME1,							\
              NAME1->FUNCTION_CALL;										\
            )															\
    LIST1.add( new EventCallback( POINTER1 ) );							\
}


// Connects an event list with a function call to this class
#define CCLAMBDA_CONNECT_THIS(LIST1, TYPE1, FUNCTION_CALL)				\
{																		\
    CCLAMBDA_1( EventCallback, TYPE1, NAME1,							\
        NAME1->FUNCTION_CALL;											\
    )																	\
    LIST1.add( new EventCallback( this ) );								\
}


#define CCLAMBDA_CONNECT_UNSAFE(LIST1, FUNCTION_CALL)					\
{																		\
    CCLAMBDA_UNSAFE( EventCallback,										\
              FUNCTION_CALL;											\
            )															\
    LIST1.add( new EventCallback() );									\
}


// Connects a custom event list with a function call to another class
#define CCLAMBDA_CONNECT_OTHER(LISTTYPE, LIST1, TYPE1, OBJECT, FUNCTION_CALL)   \
{                                                                               \
    class EventCallback : public LISTTYPE                                       \
    {                                                                           \
    public:                                                                     \
        EventCallback(TYPE1 *OBJECTPOINTER)                                     \
        {                                                                       \
            this->OBJECTPOINTER = OBJECTPOINTER;                                \
        }                                                                       \
    protected:                                                                  \
        void run()                                                              \
        {                                                                       \
            OBJECTPOINTER->FUNCTION_CALL;                                       \
        }                                                                       \
    private:                                                                    \
        TYPE1 *OBJECTPOINTER;                                                   \
    };                                                                          \
    LIST1.add( new EventCallback( OBJECT ) );                                   \
}

#define CCLAMBDA_NEW_TYPE(TARGET, LISTTYPE, TYPE1, OBJECT, FUNCTION_CALL)       \
{                                                                               \
    class EventCallback : public LISTTYPE                                       \
    {                                                                           \
    public:                                                                     \
        EventCallback(TYPE1 *OBJECTPOINTER)                                     \
        {                                                                       \
            this->OBJECTPOINTER = OBJECTPOINTER;                                \
        }                                                                       \
    protected:                                                                  \
        void run()                                                              \
        {                                                                       \
            OBJECTPOINTER->FUNCTION_CALL;                                       \
        }                                                                       \
    private:                                                                    \
        TYPE1 *OBJECTPOINTER;                                                   \
    };                                                                          \
    TARGET = new EventCallback( OBJECT );                                       \
}


#define LAMBDA_CONNECT_OTHER_UNSAFE(LISTTYPE, LIST1, FUNCTION_CALL)             \
{                                                                               \
    class EventCallback : public LISTTYPE                                       \
    {                                                                           \
    public:                                                                     \
        EventCallback()                                                         \
        {                                                                       \
        }                                                                       \
    protected:                                                                  \
        void run()                                                              \
        {                                                                       \
            FUNCTION_CALL;                                                      \
        }                                                                       \
    };                                                                          \
    LIST1.add( new EventCallback() );                                           \
}


#endif // __CCCALLBACKS_H__
