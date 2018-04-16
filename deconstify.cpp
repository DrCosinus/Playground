#include <iostream>

template<typename OBJECT_TYPE, typename RETURN_TYPE, typename... ARG_TYPES>
RETURN_TYPE& deconstify(OBJECT_TYPE* _object, const RETURN_TYPE& (OBJECT_TYPE::*_method)(ARG_TYPES...)const, ARG_TYPES&&... _args)
{
    return const_cast<RETURN_TYPE&>(((static_cast<const OBJECT_TYPE*>(_object))->*(_method))(std::forward<ARG_TYPES>(_args)...));
}
template<typename OBJECT_TYPE, typename RETURN_TYPE, typename... ARG_TYPES>
RETURN_TYPE* deconstify(OBJECT_TYPE* _object, const RETURN_TYPE* (OBJECT_TYPE::*_method)(ARG_TYPES...)const, ARG_TYPES&&... _args)
{
    return const_cast<RETURN_TYPE*>(((static_cast<const OBJECT_TYPE*>(_object))->*(_method))(std::forward<ARG_TYPES>(_args)...));
}

struct FooBar
{
    const int& get() const { return value_; }
          int& get()       { return deconstify(this, &FooBar::get); }
    const int* get_ptr() const { return &value_; }
          int* get_ptr()       { return deconstify(this, &FooBar::get_ptr); }
private:
    int value_ = 42;
};

int main()
{
    using namespace std;

    FooBar foobar;
    const FooBar& const_ref_to_foobar = foobar;
    foobar.get() = 78;
    std::cout << "FooBar A: " << const_ref_to_foobar.get() << std::endl;
    *foobar.get_ptr() = 509;
    std::cout << "FooBar B: " << *const_ref_to_foobar.get_ptr() << std::endl;
}