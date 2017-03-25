#include "ArrayRef.h"

#include <array>
#include <algorithm>
#include <vector>

static void func(cxx::array_ref<int>) {}
static void func_const(cxx::array_ref<const int>) {}

int main()
{
    {
        constexpr int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        using AV = cxx::array_ref<int const>;

        static_assert(AV{arr}.end() - AV{arr}.begin() == 10);
        static_assert(AV{arr}.begin() == AV{arr}.begin());
        static_assert(AV{arr}.begin() != AV{arr}.end());
        static_assert(AV{arr}.begin() <  AV{arr}.end());
        static_assert(AV{arr}.begin() <= AV{arr}.begin());
        static_assert(AV{arr}.end()   >  AV{arr}.begin());
        static_assert(AV{arr}.begin() >= AV{arr}.begin());
        static_assert(!(AV{arr}.begin() == AV{arr}.end()));
        static_assert(!(AV{arr}.begin() != AV{arr}.begin()));
        static_assert(!(AV{arr}.end()   <  AV{arr}.begin()));
        static_assert(!(AV{arr}.end()   <= AV{arr}.begin()));
        static_assert(!(AV{arr}.begin() >  AV{arr}.begin()));
        static_assert(!(AV{arr}.begin() >= AV{arr}.end()));
        static_assert(AV{arr}.data() == arr);
        static_assert(AV{arr}.size() == 10);
        static_assert(AV{arr}[0] == 1);
        static_assert(AV{arr}[9] == 10);
        static_assert(*AV{arr}.begin() == 1);
        static_assert(*(AV{arr}.begin() + 2) == 3);
        static_assert((AV{arr}.begin() + 1) - 1 == AV{arr}.begin());
        static_assert(AV{arr}.begin()++ == AV{arr}.begin());
        static_assert(AV{arr}.end()-- == AV{arr}.end());
        static_assert(++AV{arr}.begin() == AV{arr}.begin() + 1);
        static_assert(--AV{arr}.end() == AV{arr}.end() - 1);
        static_assert((AV{arr}.begin() + 2)[2] == 5);
        //static_assert(AV{arr} == AV{ AV{arr}.begin(), AV{arr}.end() });
        //func(AV{arr}); // ERROR
        func_const(AV{arr});

        //static_assert(AV{arr} == AV{arr});
        //static_assert(!(AV{arr} != AV{arr}));
        AV av = arr;
        bool const b1 = std::equal(av.begin(), av.end(), av.begin(), av.end());

        static_assert(std::is_nothrow_default_constructible<AV>::value);
        static_assert(std::is_nothrow_copy_assignable<AV>::value);
        static_assert(std::is_nothrow_copy_constructible<AV>::value);
        static_assert(std::is_nothrow_move_assignable<AV>::value);
        static_assert(std::is_nothrow_move_constructible<AV>::value);
        static_assert(std::is_nothrow_destructible<AV>::value);
        static_assert(std::is_nothrow_default_constructible<AV::iterator>::value);
        static_assert(std::is_copy_assignable<AV::iterator>::value);
        static_assert(std::is_nothrow_copy_assignable<AV::iterator>::value);
        static_assert(std::is_nothrow_copy_constructible<AV::iterator>::value);
        static_assert(std::is_nothrow_move_assignable<AV::iterator>::value);
        static_assert(std::is_nothrow_move_constructible<AV::iterator>::value);
        static_assert(std::is_nothrow_destructible<AV::iterator>::value);
    }

    struct Base {
        Base() = default;
        Base(int) {}
    };
    struct Derived : Base {
        Derived() = default;
        Derived(int) {}
    };

    {
        cxx::array_ref<Derived> ard;
        cxx::array_ref<const Derived> arcd = ard;
        auto it = arcd.begin();
        it = ard.begin();
        auto i2 = ard.begin();
        //cxx::array_ref<Derived> ard_ = arcd; // ERROR
        //cxx::array_ref<Base> arb = ard; // ERROR
        cxx::array_ref<Base> arb2;
        //cxx::array_ref<const Derived> ard3 = arb2; // ERROR

        auto i3 = arb2.begin();
        //i3 = ard.begin(); // ERROR
        //i2 = i3; // ERROR
        cxx::array_ref<const Base>::iterator i4;
        i4 = i3;
    }

    {
        std::vector<Base> vb;
        cxx::array_ref<Base> arb = vb;
        arb = vb;
        cxx::array_ref<const Base> arb2 = vb;
        arb2 = vb;
    }

    {
        int arr[4];
        func(arr);
        func_const(arr);
        std::vector<int> v;
        func(v);
        func_const(v);
        const std::vector<int> cv;
        //func(cv); // ERROR
        func_const(cv);
        //func(std::vector<int>{}); // ERROR
        func_const(std::vector<int>{});
    }

    {
        Derived arrd[5] = {1,2,3,4,5};
        Base arrb[5] = {1,2,3,4,5};
        cxx::array_ref<const Derived> ard = arrd;
        cxx::array_ref<const Base> arb = arrb;
        //cxx::array_ref<const Base> arb2 = arrd; // ERROR
    }

    {
        int arr[10];
        using AV = cxx::array_ref<int>;
        using AVC = cxx::array_ref<const int>;
        AV av1 = arr;
        AVC av2 = arr;
    }
    {
        const std::array<const int, 10> arr = {0,1,2,3,4,5,6,7,8,9};
        using AV = cxx::array_ref<int>;
        using AVC = cxx::array_ref<const int>;
        //AV av1 = arr; // ERROR
        AVC av2 = arr;
    }
    {
        const std::array<int, 10> arr = {0,1,2,3,4,5,6,7,8,9};
        using AV = cxx::array_ref<int>;
        using AVC = cxx::array_ref<const int>;
        //AV av1 = arr; // ERROR
        AVC av2 = arr;
    }
}

