#pragma once
#include <complex>
#include <cstdint>
#include <string>
#include <typeinfo>

namespace python {

inline auto const* gk_bool = &typeid(bool);
inline auto const* gk_int64 = &typeid(int64_t);
inline auto const* gk_float64 = &typeid(double);
inline auto const* gk_complex128 = &typeid(std::complex<double>);
inline auto const* gk_string = &typeid(std::string);
inline auto const* gk_null = &typeid(std::nullptr_t);

template<typename... TypeInfo>
bool any_of(std::type_info const* t, TypeInfo... args) {
    return ((t == args) || ...);
}

template<typename... TypeInfo>
bool none_of(std::type_info const* t, TypeInfo... args) {
    return ((t != args) && ...);
}

bool is_primitive(std::type_info const* ti) {
    return any_of(ti, gk_bool, gk_int64, gk_float64, gk_complex128, gk_string, gk_null);
}

bool is_object(std::type_info const* ti) {
    return none_of(ti, gk_bool, gk_int64, gk_float64, gk_complex128, gk_string, gk_null);
}

}