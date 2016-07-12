#ifndef ANGELSCRIPT_CASBASECLASS_H
#define ANGELSCRIPT_CASBASECLASS_H

/**
*	Base class for Angelscript classes that require one.
*/
class CASBaseClass
{
public:
	CASBaseClass() = default;
	CASBaseClass( const CASBaseClass& other ) = default;
	CASBaseClass& operator=( const CASBaseClass& other ) = default;
	~CASBaseClass() = default;
};

/**
*	Base class for all Angelscript classes that are reference counted
*/
class CASRefCountedBaseClass : public CASBaseClass
{
public:
	/**
	*	Constructor.
	*/
	CASRefCountedBaseClass()
		: CASBaseClass()
		, m_iRefCount( 1 )
	{
	}

	/**
	*	Copy constructor.
	*/
	CASRefCountedBaseClass( const CASRefCountedBaseClass& other )
		: CASBaseClass( other )
		, m_iRefCount( 1 )
	{
	}

	CASRefCountedBaseClass& operator=( const CASRefCountedBaseClass& other ) = default;

	~CASRefCountedBaseClass() = default;

	/**
	*	@return The reference count.
	*/
	int GetRefCount() const { return m_iRefCount; }

	/**
	*	Adds a reference.
	*/
	void AddRef() const;

protected:
	/**
	*	Releases a reference.
	*	@return true if the reference count has become 0
	*/
	bool InternalRelease() const;

protected:
	mutable int m_iRefCount;
};

/**
*	Base class for all Angelscript classes that are reference counted and are used acrosss threads
*/
class CASAtomicRefCountedBaseClass : public CASRefCountedBaseClass
{
public:
	CASAtomicRefCountedBaseClass() = default;
	CASAtomicRefCountedBaseClass( const CASAtomicRefCountedBaseClass& other ) = default;
	CASAtomicRefCountedBaseClass& operator=( const CASAtomicRefCountedBaseClass& other ) = default;
	~CASAtomicRefCountedBaseClass() = default;

	/**
	*	@copydoc CASRefCountedBaseClass::AddRef() const
	*	Thread-safe.
	*/
	void AddRef() const;

protected:
	/**
	*	@copydoc CASRefCountedBaseClass::InternalRelease() const
	*	Thread-safe.
	*/
	bool InternalRelease() const;
};

/**
*	Garbage collected base class
*	Pass in either CASRefCountedBaseClass or CASAtomicRefCountedBaseClass as the base class
*/
template<typename BASECLASS>
class CASGCBaseClass : public BASECLASS
{
public:
	CASGCBaseClass() = default;
	CASGCBaseClass( const CASGCBaseClass& other ) = default;
	CASGCBaseClass& operator=( const CASGCBaseClass& other ) = default;
	~CASGCBaseClass() = default;

	/**
	*	@return The garbage collector flag.
	*/
	bool GetGCFlag() const { return m_fGCFlag; }

	/**
	*	Sets the garbage collector flag.
	*/
	void SetGCFlag() const { m_fGCFlag = true; }

	/**
	*	Adds a reference and clears the garbage collector flag.
	*/
	void AddRef() const
	{
		m_fGCFlag = false;

		BASECLASS::AddRef();
	}

protected:
	/**
	*	Releases a reference and clears the garbage collector flag.
	*/
	bool InternalRelease() const
	{
		m_fGCFlag = false;

		return BASECLASS::InternalRelease();
	}

private:
	mutable bool m_fGCFlag = false;
};

/**
*	Base class for garbage collected classes.
*/
typedef CASGCBaseClass<CASRefCountedBaseClass> CASGCRefCountedBaseClass;

/**
*	Base class for thread-safe garbage collected classes.
*/
typedef CASGCBaseClass<CASAtomicRefCountedBaseClass> CASGCAtomicRefCountedBaseClass;

#endif //ANGELSCRIPT_CASBASECLASS_H
