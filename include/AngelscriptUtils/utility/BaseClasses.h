#pragma once

#include <angelscript.h>

/**
*	@defgroup BaseClasses Angelscript Base classes
*
*	@{
*/

/*
*	Note: Don't rely on default implementations of copy and move constructors and operators because
*	this will copy the reference count from the other object
*/

namespace asutils
{
/**
*	@brief Base class for Angelscript classes that require one
*/
class EmptyClass
{
public:
	EmptyClass() = default;
	EmptyClass(const EmptyClass& other) = default;
	EmptyClass& operator=(const EmptyClass& other) = default;
	~EmptyClass() = default;
};

/**
*	@brief Base class for all Angelscript classes that are reference counted
*	The pattern used is to provide only a protected InternalRelease() const method
*	Deriving classes must implement their own Release() const method that calls delete when InternalRelease returns true
*	This avoid imposing a vtable on deriving classes when it is not necessary
*/
class ReferenceCountedClass : public EmptyClass
{
public:
	ReferenceCountedClass()
		: EmptyClass()
	{
	}

	ReferenceCountedClass(const ReferenceCountedClass& other)
		: EmptyClass(other)
	{
	}

	ReferenceCountedClass& operator=(const ReferenceCountedClass&)
	{
		return *this;
	}

	~ReferenceCountedClass() = default;

	int GetRefCount() const { return m_RefCount; }

	void AddRef() const
	{
		++m_RefCount;
	}

protected:
	/**
	*	@brief Releases a reference, returns true if the reference count has become 0
	*/
	bool InternalRelease() const
	{
		return !--m_RefCount;
	}

protected:
	mutable int m_RefCount = 1;
};

/**
*	@brief Base class for all Angelscript classes that are reference counted and are used acrosss threads
*	@see ReferenceCountedClass for details on how to implement release behavior
*/
class AtomicReferenceCountedClass : public ReferenceCountedClass
{
public:
	AtomicReferenceCountedClass() = default;
	AtomicReferenceCountedClass(const AtomicReferenceCountedClass& other)
		: ReferenceCountedClass(other)
	{
	}

	AtomicReferenceCountedClass& operator=(const AtomicReferenceCountedClass& other)
	{
		ReferenceCountedClass::operator=(other);
		return *this;
	}

	~AtomicReferenceCountedClass() = default;

	/**
	*	@copydoc ReferenceCountedClass::AddRef() const
	*	Thread-safe
	*/
	void AddRef() const
	{
		asAtomicInc(m_RefCount);
	}

protected:
	/**
	*	@copydoc ReferenceCountedClass::InternalRelease() const
	*	Thread-safe
	*/
	bool InternalRelease() const
	{
		return !asAtomicDec(m_RefCount);
	}
};

/**
*	@brief Garbage collected base class
*	Pass in either @see ReferenceCountedClass or @see AtomicReferenceCountedClass as the base class
*/
template<typename BASECLASS>
class GarbageCollectedClass : public BASECLASS
{
public:
	GarbageCollectedClass() = default;
	GarbageCollectedClass(const GarbageCollectedClass& other)
		: BASECLASS(other)
	{
	}

	GarbageCollectedClass& operator=(const GarbageCollectedClass& other)
	{
		BASECLASS::operator=(other);
		return *this;
	}

	~GarbageCollectedClass() = default;

	bool GetGCFlag() const { return m_GCFlag; }

	void SetGCFlag() const { m_GCFlag = true; }

	void AddRef() const
	{
		m_GCFlag = false;

		BASECLASS::AddRef();
	}

protected:
	bool InternalRelease() const
	{
		m_GCFlag = false;

		return BASECLASS::InternalRelease();
	}

private:
	mutable bool m_GCFlag = false;
};

/**
*	@brief Base class for garbage collected classes
*/
typedef GarbageCollectedClass<ReferenceCountedClass> GarbageCollectedReferenceCountedClass;

/**
*	@brief Base class for thread-safe garbage collected classes
*/
typedef GarbageCollectedClass<AtomicReferenceCountedClass> GarbageCollectedAtomicReferenceCountedClass;

/**
*	@brief Registers a ref counted class's ref counting behaviors
*	@param className Name of the script class being registered
*/
template<typename CLASS>
void RegisterReferenceCountedClass(asIScriptEngine& engine, const char* className)
{
	engine.RegisterObjectBehaviour(
		className, asBEHAVE_ADDREF, "void AddRef()",
		asMETHODPR(CLASS, AddRef, () const, void), asCALL_THISCALL);

	engine.RegisterObjectBehaviour(
		className, asBEHAVE_RELEASE, "void Release()",
		asMETHODPR(CLASS, Release, () const, void), asCALL_THISCALL);
}

/**
*	@brief Registers a garbage collected, ref counted class's ref counting and gc behaviors
*	@param className Name of the script class being registered
*/
template<typename CLASS>
void RegisterGarbageCollectedReferenceCountedClass(asIScriptEngine& engine, const char* className)
{
	RegisterReferenceCountedClass<CLASS>(pEngine, className);

	engine.RegisterObjectBehaviour(
		className, asBEHAVE_GETREFCOUNT, "int GetRefCount() const",
		asMETHODPR(CLASS, GetRefCount, () const, int), asCALL_THISCALL);

	engine.RegisterObjectBehaviour(
		className, asBEHAVE_GETGCFLAG, "bool GetGCFlag() const",
		asMETHODPR(CLASS, GetGCFlag, () const, bool), asCALL_THISCALL);

	engine.RegisterObjectBehaviour(
		className, asBEHAVE_SETGCFLAG, "void SetGCFlag()",
		asMETHODPR(CLASS, SetGCFlag, () const, void), asCALL_THISCALL);

	engine.RegisterObjectBehaviour(
		className, asBEHAVE_ENUMREFS, "void EnumReferences(int& in)",
		asMETHOD(CLASS, EnumReferences), asCALL_THISCALL);

	engine.RegisterObjectBehaviour(
		className, asBEHAVE_RELEASEREFS, "void ReleaseReferences(int& in)",
		asMETHOD(CLASS, ReleaseReferences), asCALL_THISCALL);
}
}

/** @} */
