    template <char... bits>
        struct to_binary;

    template <char high_bit, char... bits>
        struct to_binary<high_bit, bits...>
        {
            static_assert(high_bit == '0' || high_bit == '1', "Not a binary value!");
            static const unsigned long long value =
                (high_bit - '0') << (sizeof...(bits)) | to_binary<bits...>::value;
        };

    template <char high_bit>
        struct to_binary<high_bit>
        {
            static_assert(high_bit == '0' || high_bit == '1', "Not a binary value!");
            static const unsigned long long value = (high_bit - '0');
        };

    template <char... bits>
        constexpr unsigned long long operator "" _b()
        {
            return to_binary<bits...>::value;
        }

    // ...

    int arr[1010_b]; // значение вычисляется во время компиляции
    std::cout << 101100_b << std::endl; // выведет 44
