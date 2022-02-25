#pragma once
#include <algorithm>
#include <list>

#include "pyexcept.hpp"
#include "pyobject.hpp"

namespace python {

#define ASSERT_IN_RANGE(i) do {                         \
    if (i < 0 || i > this->data()->size()) {            \
        auto const tmp = std::to_string(i);             \
        throw IndexError(tmp + " is out of range.");    \
    }                                                   \
} while (false)

class List : public Object {
public:
    template<typename... Args>
    explicit List(Args&&... args)
        : Object(construct<std::list<MetaObject>>(std::forward<Args>(args)...), (List*){}) {}

    // common pure interface

    Iterator begin() override {
        return this->data()->begin();
    }
    ConstIterator begin() const override {
        return this->data()->begin();
    }
    ConstIterator cbegin() const override {
        return this->data()->cbegin();
    }

    Iterator end() override {
        return this->data()->end();
    }
    ConstIterator end() const override {
        return this->data()->end();
    }
    ConstIterator cend() const override {
        return this->data()->cend();
    }

    std::ostream& stream_to(std::ostream& os) const override {
        // TODO
        return os;
    }

    // Python list access

    var& operator [](int idx) override {
        return const_cast<MetaObject&>(static_cast<List const&>(*this)[idx]);
    }

    var const& operator [](int idx) const override {
        this->adjust(idx);
        ASSERT_IN_RANGE(idx);
        return *this->at(idx);
    }

    bool iterable() const noexcept override {
        return true;
    }

    List* derived() noexcept override {
        return this;
    }

    List const* derived() const noexcept override {
        return this;
    }

    List* slice(int start, int stop, int step = 1) {
        this->adjust(start);
        this->adjust(stop);

        List* result = construct<List>();

        if (start >= stop) {
            return result;
        }
        ASSERT_IN_RANGE(start);
        ASSERT_IN_RANGE(stop);

        auto it_start = this->at(start);
        auto it_stop = this->at(stop);

        int i = 0;
        while (it_start != it_stop) {
            if (i == 0) {
                result->append(*it_start);
                i = step;
            }
            --i;
            ++it_start;
        }
        return result;
    }

    // Python list methods

    void append(var x) {
        this->data()->push_back(x);
    }

    void clear() {
        this->data()->clear();
    }

    List* copy() const {
        List* result = construct<List>();
        result->data()->assign(this->data()->cbegin(), this->data()->cend());
        return result;
    }

    int count(var x) const {
        return std::count(this->begin(), this->end(), x);
    }

    void extend(var x) {
        for (var elem : x) {
            this->append(elem);
        }
    }

    int index(var x) const {
        int i = 0;
        for (var elem : *this->data()) {
            if (x == elem) {
                return i;
            }
            ++i;
        }
        throw "ValueError";
    }

    void insert(int idx, var x) {
        this->adjust(idx);
        ASSERT_IN_RANGE(idx);
        this->data()->insert(this->at(idx), x);
    }

    var pop(int idx = -1) {
        this->adjust(idx);
        ASSERT_IN_RANGE(idx);
        var result = *this->at(idx);
        this->data()->erase(this->at(idx));
        return result;
    }

    var remove(var x) {
        return this->pop(this->index(x));
    }

    void reverse() {
        auto it_left = this->data()->begin();
        auto it_right = this->data()->end();
        --it_right;

        while (it_left != it_right) {
            using std::swap;
            swap(*it_left, *it_right);
        }
    }

    void sort() {
        // TODO
    }

    // Python magic functions
    // I'll not use double underscores since it's reserved by compilers

    List* _add_(Object* other) const override {
        List* other_list = dynamic_cast<List*>(other);
        if (other_list == nullptr) {
            throw TypeError(/* TODO */ " cannot be concatenated to a list.");
        }

        List* result = construct<List>();
        result->extend(m_data);
        result->extend(other_list->data);
        return result;
    }

    int _len_() const noexcept override {
        return this->data()->size();
    }

    std::type_info const* _type_() const noexcept override {
        return m_type;
    }

private:
    std::list<MetaObject>* data() {
        return reinterpret_cast<std::list<MetaObject>*>(m_data);
    }
    std::list<MetaObject> const* data() const {
        return reinterpret_cast<std::list<MetaObject>*>(m_data);
    }
    void adjust(int& idx) const noexcept {
        if (idx < 0) {
            idx += this->data()->size();
        }
    }
    Iterator at(int idx) {
        auto it = data()->begin();
        while (idx--) {
            ++it;
        }
        return it;
    }
    ConstIterator at(int idx) const {
        return const_cast<List*>(this)->at(idx);
    }
};


template<typename... Args>
Object* list(Args&&... args) {
    void* ptr = g_gc.alloc(sizeof(Object));
    return new(ptr) List(std::forward<decltype(args)>(args)...);
}

#define list list

#undef ASSERT_IN_RANGE

} // namespace python