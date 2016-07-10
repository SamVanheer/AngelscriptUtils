#include <angelscript.h>

#include "CASBaseClass.h"

CASBaseClass::CASBaseClass()
{
}

CASBaseClass::~CASBaseClass()
{
}

void CASRefCountedBaseClass::AddRef() const
{
	++m_iRefCount;
}

bool CASRefCountedBaseClass::InternalRelease() const
{
	return !--m_iRefCount;
}

void CASAtomicRefCountedBaseClass::AddRef() const
{
	asAtomicInc( m_iRefCount );
}

bool CASAtomicRefCountedBaseClass::InternalRelease() const
{
	return !asAtomicDec( m_iRefCount );
}