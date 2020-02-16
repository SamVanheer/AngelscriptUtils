#pragma once

#include <algorithm>
#include <stdexcept>
#include <type_traits>

#include <angelscript.h>

/**
*	@file
*
*	Smart pointer facilities
*/

namespace asutils
{
/**
*   @brief Checks if a type has a \code void AddRef() const \endcode method
*/
template<typename T, typename Ret>
struct HasAddRef
{
private:
    template<typename U>
    static constexpr auto Check(U*)
        -> typename
        std::is_same<
        decltype(std::declval<U>().AddRef()),
        Ret
        >::type
    {
    }

    template<typename>
    static constexpr std::false_type Check(...)
    {
    }

    typedef decltype(Check<T>(0)) type;

public:
    static constexpr bool value = type::value;
};

/**
*   @brief Checks if a type has a \code void Release() const \endcode method
*/
template<typename T, typename Ret>
struct HasRelease
{
private:
    template<typename U>
    static constexpr auto Check(U*)
        -> typename
        std::is_same<
        decltype(std::declval<U>().Release()),
        Ret
        >::type
    {
    }

    template<typename>
    static constexpr std::false_type Check(...)
    {
    }

    typedef decltype(Check<T>(0)) type;

public:
    static constexpr bool value = type::value;
};

/**
*   @brief Allows calling AddRef and Release on types that may not have such methods
*   If the type is missing either method no reference handling is performed
*   If a type has custom methods, overload this type to provide support for reference handling
*/
template<typename T>
struct ReferenceAdapter
{
    ReferenceAdapter() = delete;

    static void AddRef(const T& object)
    {
        object.AddRef();
    }

    static void Release(const T& object)
    {
        object.Release();
    }
};

/**
*   @brief Allows calling AddRef and Release on types that may not have such methods
*   If the type is missing either method no reference handling is performed
*/
template<typename T, typename C = void>
struct OptionalReferenceAdapter
{
    OptionalReferenceAdapter() = delete;

    static const bool HasReferenceCounting = false;

    using type = C;

    static void AddRef(const T&)
    {
        //Nothing
    }

    static void Release(const T&)
    {
        //Nothing
    }
};

template<typename T>
struct OptionalReferenceAdapter<T, typename std::enable_if<
    (HasAddRef<T, void>::value || HasAddRef<T, int>::value) &&
    (HasRelease<T, void>::value || HasRelease<T, int>::value)>::type>
{
    OptionalReferenceAdapter() = delete;

    static const bool HasReferenceCounting = true;

    static void AddRef(const T& object)
    {
        ReferenceAdapter<T>::AddRef(object);
    }

    static void Release(const T& object)
    {
        ReferenceAdapter<T>::Release(object);
    }
};

/**
*   @brief Sets a pointer to a reference counted object to the value of another pointer,
*   releasing the old reference and adding the new reference unless transferOwnership is true
*   @param destination Pointer to set
*   @param source Pointer to object to set destination to
*   @param transferOwnership If true, ownership of the source reference is transferred to the destination reference
*   @return Pointer to the new destination
*/
template<typename T>
T* SetPointer(T*& destination, T* source, bool transferOwnership = false)
{
    //Add reference to object before releasing old in case both are the same object
    auto oldDestination = destination;

    destination = source;

    if (destination && !transferOwnership)
    {
        ReferenceAdapter<T>::AddRef(*destination);
    }

    if (oldDestination)
    {
        ReferenceAdapter<T>::Release(*oldDestination);
    }

    return destination;
}

/**
*   @brief Sets a pointer to a reference counted script object to the value of another pointer,
*   releasing the old reference and adding the new reference unless transferOwnership is true
*   @param destination Pointer to set
*   @param source Pointer to object to set destination to
*   @param typeInfo Type info object of the underlying object
*   @param transferOwnership If true, ownership of the source reference is transferred to the destination reference
*   @return Pointer to the new destination
*/
inline void* SetPointer(void*& destination, void* source, const asITypeInfo& typeInfo, bool transferOwnership = false)
{
    //Add reference to object before releasing old in case both are the same object
    auto oldDestination = destination;

    auto& engine = *typeInfo.GetEngine();

    destination = source;

    if (destination && !transferOwnership)
    {
        engine.AddRefScriptObject(destination, &typeInfo);
    }

    if (oldDestination)
    {
        engine.ReleaseScriptObject(oldDestination, &typeInfo);
    }

    return destination;
}

/**
*   @brief Smart pointer to a reference counted object
*/
template<typename T>
class ReferencePointer final
{
public:
    using type = T;

    /**
    *   @brief Creates a reference that points to nothing
    */
    ReferencePointer() = default;

    /**
    *   @brief Creates a reference that points to nothing
    */
    ReferencePointer(std::nullptr_t)
    {
    }

    /**
    *   @brief Creates a reference that points to object, and transfers ownership of the given reference if transferOwnership is true
    */
    explicit ReferencePointer(T* object, bool transferOwnership = false)
    {
        Reset(object, transferOwnership);
    }

    /**
    *   @brief Creates a reference that points to the same object as other
    */
    ReferencePointer(const ReferencePointer& other)
    {
        Reset(other.m_Object, false);
    }

    /**
    *   @brief Creates a reference that points to the same object as other, transferring ownership from other to this reference in the process
    */
    ReferencePointer(ReferencePointer&& other)
    {
        Reset(other.m_Object, true);
        other.m_Object = nullptr;
    }

    ~ReferencePointer()
    {
        Reset();
    }

    /**
    *   @brief Changes this reference to point to the same object as other
    */
    ReferencePointer& operator=(const ReferencePointer& other)
    {
        if (this != &other)
        {
            Reset(other.m_Object, false);
        }

        return *this;
    }

    /**
    *   @brief Changes this reference to point to the same object as other, transferring ownership from other to this reference in the process
    */
    ReferencePointer& operator=(ReferencePointer&& other)
    {
        if (this != &other)
        {
            Reset(other.m_Object, true);
            other.m_Object = nullptr;
        }

        return *this;
    }

    /**
    *   @brief Resets this pointer to point to nothing
    */
    void Reset()
    {
        if (m_Object)
        {
            ReferenceAdapter<T>::Release(*m_Object);

            m_Object = nullptr;
        }
    }

    /**
    *   @brief Resets this pointer to point to object, transferring ownership if transferOwnership is true
    */
    void Reset(T* object, bool transferOwnership = false)
    {
        SetPointer(m_Object, object, transferOwnership);
    }

    void Swap(ReferencePointer& other)
    {
        std::swap(m_Object, other.m_Object);
    }

    T* Get() const { return m_Object; }

    T& operator*() const { return *m_Object; }

    T* operator->() const { return m_Object; }

    operator bool() const { return m_Object != nullptr; }

private:
    T* m_Object = nullptr;
};

/**
*   @brief Smart pointer to a reference counted script object
*/
class ObjectPointer final
{
public:
    /**
*   @brief Creates a reference that points to nothing
*/
    ObjectPointer() = default;

    /**
    *   @brief Creates a reference that points to nothing
    */
    ObjectPointer(std::nullptr_t)
    {
    }

    /**
    *   @brief Creates a reference that points to object, and transfers ownership of the given reference if transferOwnership is true
    */
    ObjectPointer(void* object, const ReferencePointer<asITypeInfo>& type, bool transferOwnership = false)
    {
        Reset(object, type, transferOwnership);
    }

    /**
    *   @brief Creates a reference that points to the same object as other
    */
    ObjectPointer(const ObjectPointer& other)
    {
        Reset(other.m_Object, other.m_Type, false);
    }

    /**
    *   @brief Creates a reference that points to the same object as other, transferring ownership from other to this reference in the process
    */
    ObjectPointer(ObjectPointer&& other)
    {
        Reset(other.m_Object, other.m_Type, true);
        other.m_Object = nullptr;
        other.m_Type.Reset();
    }

    ~ObjectPointer()
    {
        Reset();
    }

    /**
    *   @brief Changes this reference to point to the same object as other
    */
    ObjectPointer& operator=(const ObjectPointer& other)
    {
        if (this != &other)
        {
            Reset(other.m_Object, other.m_Type, false);
        }

        return *this;
    }

    /**
    *   @brief Changes this reference to point to the same object as other, transferring ownership from other to this reference in the process
    */
    ObjectPointer& operator=(ObjectPointer&& other)
    {
        if (this != &other)
        {
            Reset(other.m_Object, other.m_Type, true);
            other.m_Object = nullptr;
            other.m_Type.Reset();
        }

        return *this;
    }

    /**
    *   @brief Resets this pointer to point to nothing
    */
    void Reset()
    {
        if (m_Object)
        {
            m_Type->GetEngine()->ReleaseScriptObject(m_Object, m_Type.Get());

            m_Object = nullptr;
            m_Type.Reset();
        }
    }

    /**
    *   @brief Resets this pointer to point to object, transferring ownership if transferOwnership is true
    */
    void Reset(void* object, const ReferencePointer<asITypeInfo>& type, bool transferOwnership = false)
    {
        if (((object != nullptr) && !type))
        {
            throw std::invalid_argument("The type must be valid if object is non-null");
        }

        if (m_Object != object)
        {
            auto oldObject = m_Object;
            auto oldType = m_Type;

            m_Object = nullptr;

            if (object)
            {
                SetPointer(m_Object, object, *type, transferOwnership);

                m_Type = type;
            }
            else
            {
                //Don't assign the type unless it's a non-null pointer
                m_Type.Reset();
            }

            if (oldObject)
            {
                m_Type->GetEngine()->ReleaseScriptObject(oldObject, m_Type.Get());
            }
        }
    }

    void Swap(ObjectPointer& other)
    {
        std::swap(m_Object, other.m_Object);
        m_Type.Swap(other.m_Type);
    }

    void* Get() const { return m_Object; }

    const ReferencePointer<asITypeInfo>& GetTypeInfo() const { return m_Type; }

    explicit operator bool() const { return m_Object != nullptr; }

private:
    void* m_Object = nullptr;
    ReferencePointer<asITypeInfo> m_Type;
};
}
