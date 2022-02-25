#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <typeinfo>
#include <iostream>
#include <variant>

#include "gc.hpp"
#include "pytypes.hpp"

namespace python {

class Object;
class MetaObject;

using Iterator = std::list<MetaObject>::iterator;
using ConstIterator = std::list<MetaObject>::const_iterator;

template<typename ObjectT>
ObjectT* construct(auto&&... args) {
    void* ptr = g_gc.alloc(sizeof(ObjectT));
    ObjectT* data = new(ptr) ObjectT{ std::forward<decltype(args)>(args)... };
    return data;
}

template<typename ObjectT>
void destroy(ObjectT* obj) {
    g_gc.dealloc(obj, sizeof(ObjectT));
}

/**
 * @brief The basic object type.
 * It's the base class of all non-primitive types in Pyish.
 */
class Object {
public:
    Object() = default;

    explicit Object(void* data, std::type_info const* type, std::size_t size)
        : m_data(data), m_type(type), m_size(size) {}

    template<typename ObjectT>
    explicit Object(auto* data, ObjectT* dull)
        : m_data(data), m_type(&typeid(ObjectT)), m_size(sizeof(std::remove_pointer_t<decltype(data)>)) {}

    // common pure interface

    virtual Iterator begin() = 0;
    virtual ConstIterator begin() const = 0;
    virtual ConstIterator cbegin() const = 0;

    virtual Iterator end() = 0;
    virtual ConstIterator end() const = 0;
    virtual ConstIterator cend() const = 0;

    virtual MetaObject& operator [](int idx) = 0;
    virtual MetaObject const& operator [](int idx) const = 0;

    virtual bool iterable() const noexcept = 0;

    virtual Object* derived() noexcept = 0;
    virtual Object const* derived() const noexcept = 0;

    virtual std::ostream& stream_to(std::ostream& os) const = 0;

    // default-bahavior interface

    virtual Object* clone() const {
        void* obj = g_gc.alloc(sizeof(Object));
        void* data = g_gc.alloc(this->m_size);
        std::memcpy(data, m_data, m_size);
        
        Object* result = reinterpret_cast<Object*>(obj);
        result->m_data = data;
        result->m_type = m_type;
        result->m_size = m_size;
        return result;
    }

    virtual void swap(Object* other) {
        using std::swap;
        swap(m_data, other->m_data);
        swap(m_type, other->m_type);
        swap(m_size, other->m_size);
    }

    friend std::ostream& operator <<(std::ostream& os, Object const* obj) {
        return obj->stream_to(os);
    }

    virtual ~Object() {
        g_gc.dealloc(m_data, m_size);
    }

    // Python magic functions

    virtual bool _eq_(Object* other) const noexcept = 0;
    virtual int _len_() const noexcept = 0;
    virtual std::type_info const* _type_() const noexcept = 0;

protected:
    void* m_data;
    std::type_info const* m_type;
    std::size_t m_size;
};

#define var MetaObject

/**
 * @brief An object pointer wrapper for easy use.
 * This class dispatches all methods to specific implementations.
 * Consequently, this is an overstuffed class, but I haven't figured out a better way.
 */
class MetaObject {
public:
    /**
     * @brief Possible types of any object in Pyish.
     * See pytypes.hpp for details.
     */
    using DataType = std::variant<bool, /* bool */
                                  int64_t, /* int64 */
                                  double, /* float64 */
                                  std::complex<double>, /* complex */
                                  std::string,  /* string */
                                  std::nullptr_t, /* NoneType */
                                  Object*>;

    enum struct Type {
        k_bool, k_int64, k_float64, k_complex128, k_string, k_null, k_object
    };

// constructors and assignments

    template<typename Arg>
    MetaObject(Arg&& arg) 
        requires (!std::is_same_v<std::decay_t<Arg>, MetaObject> && !std::is_same_v<std::decay_t<Arg>, DataType>)
        : m_data(std::make_shared<DataType>(arg)), m_type(this->type()) {}

    MetaObject(DateType const& data, std::type_info const* type)
        : m_data(data), m_type(type) {}

    MetaObject(MetaObject const& other) = default;

    template<typename Arg>
    MetaObject& operator =(Arg&& arg) requires (!std::is_same_v<std::decay_t<Arg>, MetaObject>) {
        m_data = std::make_shared<DataType>(arg);
        return *this;
    }

    MetaObject& operator =(MetaObject const& other) = default;

// overloaded operators

    MetaObject& operator [](int idx) {
        return const_cast<MetaObject&>(static_cast<MetaObject&>(*this)[idx]);
    }

    MetaObject const& operator [](int idx) const {
        if (m_type == gk_string) {
            return std::get<std::string>(*m_data)[idx];
        }
        else if (!is_object(m_type) || !this->as_object()->iterable()) {
            throw TypeError(/* TODO */" is not iterable");
        }
        return (*this->as_object())[idx];
    }

    friend std::ostream& operator <<(std::ostream& os, MetaObject const& mo) {
        std::visit([&os, &mo](auto&& obj) { os << obj; }, *mo.m_data);
        return os;
    }

    friend bool operator ==(MetaObject mo_1, MetaObject mo_2) {
        if (mo_1.m_type != mo_2.m_type) {
            return false;
        }
        if (!is_object(mo_1.m_type) && !is_object(mo_2.m_type)) {
            mo_1.m_data == mo_2.m_data;
        }
        return mo_1.as_object()->_eq_(mo_2.as_object());
    }

// Python methods

    void append(var obj) {
        if (!is_object(m_type)) {
            throw TypeError(/* TODO */ " has no attribute 'append'.");
        }
        return this->as_object()->append(obj);
    }

    void clear() {
        if (m_type == gk_string) {
            this->as_string().clear();
        }
        else if (!is_object(m_type)) {
            throw TypeError(/* TODO */ " has no attribute 'clear'.");
        }
        return this->as_object()->clear();
    }

    var copy() const {
        if (!is_object(m_type)) {
            
        }
    }

    var slice(int start, int stop, int step = 1) const {
        if (m_type == gk_string) {
            auto const str = this->as_string();
            // TODO
        }
        else if (!is_object(m_type) || !this->as_object()->iterable()) {
            throw TypeError(/* TODO */ " is not sliceable.");
        }
        return this->as_object()->slice(start, stop, step);
    }

// C++ utilities

    friend auto begin(MetaObject& mo) {
        if (std::holds_alternative<Object*>(*mo.m_data)) {
            return std::get<Object*>(*mo.m_data)->begin();
        }
        throw nullptr;
    }

    friend auto end(MetaObject& mo) {
        if (std::holds_alternative<Object*>(*mo.m_data)) {
            return std::get<Object*>(*mo.m_data)->end();
        }
        throw nullptr;
    }
    
private:
    /**
     * @brief Get the type of current object.
     */
    std::type_info const* type() const noexcept {
        using enum Type;
        auto const obj_type = static_cast<Type>(m_data->index());

        switch (obj_type) {
        case k_bool:        return gk_bool;
        case k_int64:       return gk_int64;
        case k_float64:     return gk_float64;
        case k_complex128:  return gk_complex128;
        case k_string:      return gk_string;
        case k_null:        return gk_null;
        case k_object:      return this->as_object()->_type_();
        default:            return nullptr;
        }
    }

    std::string& as_string() noexcept {
        return std::get<std::string>(*m_data);
    }

    Object* as_object() const noexcept {
        return std::get<Object*>(*m_data);
    }

    std::shared_ptr<DataType> m_data;
    /**
     * @brief Type of the current object.
     * It's actually redundant since we already keep the type information in m_data;
     * however the type information is so frequently accessed (almost every time we
     * call a method), I'd prefer to make it a member. 
     */
    std::type_info const* m_type;
};

}