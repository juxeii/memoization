#include <catch2/catch_all.hpp>
#include "Memoization.hpp"
#include <iostream>

namespace
{
    struct MemoizeFixture
    {
        using Arguments = std::pair<std::int8_t, std::int32_t>;

        bool isCacheUsed{false};
        const Arguments firstArguments{5, 1};
        const Arguments secondArguments{42, 1};

        auto createTestFunction()
        {
            return [this](std::int8_t x, std::int32_t y) {
                isCacheUsed = false;
                return x + y;
            };
        }

        auto createTestFunctionForLRU()
        {
            return [this](const std::int8_t x, std::int32_t y) -> std::int32_t {
                isCacheUsed = false;
                int r = x + y;
                return 32;
            };
        }

        template <typename Callable>
        void assertCacheUsed(Callable &&c, const Arguments &arguments, bool wasCacheUsed)
        {
            isCacheUsed = true;
            c(arguments.first, arguments.second);
            REQUIRE(isCacheUsed == wasCacheUsed);
        }

        template <typename Callable>
        void assertCacheIsUsed(Callable &&c, const Arguments &arguments)
        {
            assertCacheUsed(std::forward<Callable>(c), arguments, true);
        }

        template <typename Callable>
        void assertCacheIsNotUsed(Callable &&c, const Arguments &arguments)
        {
            assertCacheUsed(std::forward<Callable>(c), arguments, false);
        }
    };

    TEST_CASE_METHOD(MemoizeFixture, "Function calls are cached with the default map cache")
    {
        auto memoized = memoization::memoize(createTestFunction());

        assertCacheIsNotUsed(memoized, firstArguments);
        assertCacheIsUsed(memoized, firstArguments);

        assertCacheIsNotUsed(memoized, secondArguments);
        assertCacheIsUsed(memoized, secondArguments);

        assertCacheIsUsed(memoized, firstArguments);
        assertCacheIsUsed(memoized, secondArguments);
    }

    TEST_CASE_METHOD(MemoizeFixture, "Generic cache returns ref-to-const type")
    {
        auto m = memoization::memoize([](int) { return 42; });
        static_assert(std::is_same_v<decltype(m(42)), int const &>);
    }

    TEST_CASE_METHOD(MemoizeFixture, "Function calls are cached with a single LRU cache capacity")
    {
        auto memoized = memoization::memoizeWithLRU<1>(createTestFunction());

        assertCacheIsNotUsed(memoized, firstArguments);
        assertCacheIsUsed(memoized, firstArguments);

        assertCacheIsNotUsed(memoized, secondArguments);
        assertCacheIsUsed(memoized, secondArguments);

        assertCacheIsNotUsed(memoized, firstArguments);
        assertCacheIsNotUsed(memoized, secondArguments);
    }

    TEST_CASE_METHOD(MemoizeFixture, "LRU cache returns non-ref-type")
    {
        auto m = memoization::memoizeWithLRU<1>([](int) { return 42; });
        static_assert(std::is_same_v<decltype(m(42)), int>);
    }
} // namespace
