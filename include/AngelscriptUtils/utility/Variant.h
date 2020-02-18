#pragma once

#include <cstdint>
#include <type_traits>
#include <variant>

#include <angelscript.h>

#include "AngelscriptUtils/utility/SmartPointers.h"
#include "AngelscriptUtils/utility/TypeInfo.h"

namespace asutils
{
/**
*	@brief Stores a reference to any Angelscript object
*/
class Variant final
{
private:
	//Type id value used to indicate that no object is being referred to
	//Use void since there can't be any void instances
	static const int NO_OBJECT_TYPEID = asTYPEID_VOID;

public:
	Variant() = default;

	template<typename T>
	Variant(T value)
	{
		Reset(value);
	}

	Variant(const ObjectPointer& object)
	{
		Reset(object);
	}

	Variant(ObjectPointer&& object)
	{
		Reset(std::move(object));
	}

	Variant(asIScriptEngine& engine, int typeId, void* value)
	{
		Set(engine, typeId, value);
	}

	~Variant()
	{
		Reset();
	}

	Variant(const Variant& other)
	{
		m_TypeId = other.m_TypeId;
		m_PrimitiveValue = other.m_PrimitiveValue;
		m_Object = other.m_Object;
	}

	Variant(Variant&& other)
	{
		m_TypeId = other.m_TypeId;
		m_PrimitiveValue = other.m_PrimitiveValue;
		m_Object = std::move(other.m_Object);

		other.m_TypeId = NO_OBJECT_TYPEID;
		other.m_PrimitiveValue = {};
	}

	Variant& operator=(const Variant& other)
	{
		if (this != &other)
		{
			Reset();

			if (other.GetTypeId() != NO_OBJECT_TYPEID)
			{
				if (IsPrimitive(other.GetTypeId()))
				{
					SetPrimitiveValue(other);
				}
				else
				{
					SetObjectValue(ObjectPointer{other.m_Object});
				}
			}
		}

		return *this;
	}

	Variant& operator=(Variant&& other)
	{
		if (this != &other)
		{
			Reset();

			if (other.GetTypeId() != NO_OBJECT_TYPEID)
			{
				if (IsPrimitive(other.GetTypeId()))
				{
					SetPrimitiveValue(other);

					other.m_PrimitiveValue = {};
				}
				else
				{
					SetObjectValue(std::move(other.m_Object));
					//If this was an object its object pointer will have been reset by the move operation
				}

				other.m_TypeId = NO_OBJECT_TYPEID;
			}
		}

		return *this;
	}

	bool IsEmpty() const
	{
		return m_TypeId == NO_OBJECT_TYPEID;
	}

	void Reset()
	{
		if (!IsEmpty())
		{
			m_TypeId = NO_OBJECT_TYPEID;
			m_PrimitiveValue = {};
			m_Object.Reset();
		}
	}

	template<typename T, typename std::enable_if_t<IsPrimitiveType<T>::value, int> = 0>
	void Reset(T value)
	{
		SetPrimitiveValue(PrimitiveTypeToId<T>::TypeId, value);
	}

	void Reset(const ObjectPointer& object)
	{
		Reset();
		SetObjectValue(ObjectPointer{object});
	}

	void Reset(ObjectPointer&& object)
	{
		Reset();
		SetObjectValue(std::move(object));
	}

	bool Set(asIScriptEngine& engine, int typeId, void* value)
	{
		if (IsPrimitive(typeId))
		{
			Reset();

#define PRIMITIVE_CASE(typeId, type)							\
case typeId:													\
	SetPrimitiveValue(typeId, *reinterpret_cast<type*>(value));	\
	break

			switch (typeId)
			{
			PRIMITIVE_CASE(asTYPEID_BOOL, bool);

			PRIMITIVE_CASE(asTYPEID_INT8, int8_t);
			PRIMITIVE_CASE(asTYPEID_INT16, int16_t);
			PRIMITIVE_CASE(asTYPEID_INT32, int32_t);
			PRIMITIVE_CASE(asTYPEID_INT64, int64_t);

			PRIMITIVE_CASE(asTYPEID_UINT8, uint8_t);
			PRIMITIVE_CASE(asTYPEID_UINT16, uint16_t);
			PRIMITIVE_CASE(asTYPEID_UINT32, uint32_t);
			PRIMITIVE_CASE(asTYPEID_UINT64, uint64_t);

			PRIMITIVE_CASE(asTYPEID_FLOAT, float);
			PRIMITIVE_CASE(asTYPEID_DOUBLE, double);

			default: return false;
			}

#undef PRIMITIVE_CASE

			return true;
		}
		else if (IsEnum(typeId))
		{
			//Enums are 32 bit signed integers
			m_TypeId = typeId;
			m_PrimitiveValue.emplace<int>(*reinterpret_cast<int*>(value));
			m_Object = {};

			return true;
		}
		else if (IsObject(typeId))
		{
			ObjectPointer object{value, ReferencePointer<asITypeInfo>{engine.GetTypeInfoById(typeId)}};

			Reset(std::move(object));

			return true;
		}

		return false;
	}

	int GetTypeId() const { return m_TypeId; }

	template<typename T, std::enable_if_t<IsPrimitiveType<T>::value, int> = 0>
	T GetPrimitiveValue() const
	{
		return std::get<T>(m_PrimitiveValue);
	}

	const ObjectPointer& GetObject() const { return m_Object; }

	operator bool() const { return !IsEmpty(); }

private:
	template<typename T>
	void SetPrimitiveValue(int typeId, T value)
	{
		Reset();
		m_TypeId = typeId;
		m_PrimitiveValue.emplace<T>(value);
	}

	void SetPrimitiveValue(const Variant& other)
	{
		Reset();
		m_TypeId = other.GetTypeId();
		m_PrimitiveValue = other.m_PrimitiveValue;
	}

	void SetObjectValue(ObjectPointer&& object)
	{
		m_Object = std::move(object);

		m_TypeId = m_Object ? m_Object.GetTypeInfo()->GetTypeId() : NO_OBJECT_TYPEID;
	}

private:
	int m_TypeId = NO_OBJECT_TYPEID;

	//Value if this is any kind of primitive type (bool, int, float, enum)
	std::variant<
		bool,
		int8_t,
		int16_t,
		int32_t,
		int64_t,
		uint8_t,
		uint16_t,
		uint32_t,
		uint64_t,
		float,
		double> m_PrimitiveValue;

	//Pointer to object & object type if this is any kind of object (ref or value type, including functions)
	ObjectPointer m_Object;
};
}
