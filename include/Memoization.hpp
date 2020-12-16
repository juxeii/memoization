#pragma once

#include <functional>
#include <map>
#include <optional>
#include <queue>
#include <tuple>
#include "CallableTraits.hpp"

namespace memoization
{
    template <auto cacheCapacity>
    struct LRUCache
    {
    };
} // namespace memoization

namespace memoization::detail
{
    struct MapCache
    {
    };

    template <typename Callable>
    using OrderedMap = std::map<callable_arguments_t<Callable>, callable_result_t<Callable>, std::less<>>;

    template <typename Container, auto cacheCapacity>
    struct LRUCacheImpl
    {
        Container cacheMap{};
        std::queue<typename Container::const_iterator> cacheQueue{};
    };

    template <typename Cache, typename Key>
    auto findInCache(const Cache &cache, const Key &key)
    {
        auto iterator{cache.find(key)};
        return iterator == cache.end() ? std::nullopt : std::optional{iterator};
    }

    template <typename T, auto cacheCapacity, typename Key>
    auto findInCache(const LRUCacheImpl<T, cacheCapacity> &lru, const Key &key)
    {
        return findInCache(lru.cacheMap, key);
    }

    template <typename Cache, typename... Args>
    auto emplaceInCache(Cache &cache, Args &&... args)
    {
        return cache.try_emplace(std::forward<decltype(args)>(args)...).first;
    }

    template <typename T, auto cacheCapacity, typename... Args>
    auto emplaceInCache(LRUCacheImpl<T, cacheCapacity> &lru, Args &&... args)
    {
        if (lru.cacheQueue.size() == cacheCapacity)
        {
            lru.cacheMap.erase(lru.cacheQueue.front());
            lru.cacheQueue.pop();
        }
        auto iterator = emplaceInCache(lru.cacheMap, std::forward<decltype(args)>(args)...);
        return lru.cacheQueue.emplace(iterator);
    }

    template <typename Callable, typename Arguments, typename Cache>
    auto applyFunction(const Callable &callable, const Arguments &arguments, Cache &cache)
    {
        auto result{std::apply(callable, arguments)};
        return emplaceInCache(cache, arguments, std::move(result));
    }

    template <typename CacheReturn, typename Callable, typename Cache>
    auto createMemoizer(Callable callable, Cache cache)
    {
        return [callable = std::move(callable), cache = std::move(cache)](auto &&... args) mutable -> CacheReturn {
            auto arguments{std::forward_as_tuple(std::forward<decltype(args)>(args)...)};
            if (auto maybeValue{findInCache(cache, arguments)})
            {
                return (*maybeValue)->second;
            }
            return applyFunction(callable, arguments, cache)->second;
        };
    }

    template <typename Cache, typename Callable>
    struct CreateCache
    {
        using type = OrderedMap<Callable>;
    };

    template <auto cacheCapacity, typename Callable>
    struct CreateCache<LRUCache<cacheCapacity>, Callable>
    {
        static_assert(cacheCapacity > 0, "LRU cache capacity must be greater 0!");
        static_assert(
            !std::is_reference_v<callable_result_t<Callable>>,
            "Callable return type for a LRU cache cannot be a reference!");
        using type = LRUCacheImpl<OrderedMap<Callable>, cacheCapacity>;
    };

    template <typename T, typename U>
    using CreateCache_t = typename CreateCache<T, U>::type;

    template <typename CacheTag, typename ReturnType>
    struct ReturnTypeDeduction : std::add_lvalue_reference<std::add_const_t<ReturnType>>
    {
    };

    template <auto cacheCapacity, typename ReturnType>
    struct ReturnTypeDeduction<LRUCache<cacheCapacity>, ReturnType> : remove_cvref<ReturnType>
    {
    };

    template <typename CacheTag, typename ReturnType>
    using ReturnTypeDeductionT = typename ReturnTypeDeduction<CacheTag, ReturnType>::type;

} // namespace memoization::detail

namespace memoization
{
    using namespace memoization::detail;

    template <typename CacheTag = MapCache, typename Callable>
    [[nodiscard]] auto memoize(Callable &&callable) noexcept
    {
        using ReturnType = callable_result_t<Callable>;
        static_assert(!std::is_void_v<ReturnType>, "Callable with void return type cannot be memoized!");
        static_assert(
            !(std::is_reference_v<ReturnType> && !std::is_const_v<std::remove_reference_t<ReturnType>>),
            "Callable with non-const reference return type cannot be memoized!");
        static_assert(std::is_copy_constructible_v<ReturnType>, "Callable return type must be copy constructible!");
        static_assert(
            std::tuple_size_v<callable_arguments_t<Callable>> > 0, "Callable with no arguments cannot be memoized!");

        using AdjustedReturnType = ReturnTypeDeductionT<CacheTag, ReturnType>;
        using CacheType = CreateCache_t<CacheTag, callable_to_function_t<Callable>>;

        return createMemoizer<AdjustedReturnType>(std::forward<Callable>(callable), CacheType{});
    }

    template <auto cacheCapacity, typename Callable>
    [[nodiscard]] auto memoizeWithLRU(Callable &&callable) noexcept
    {
        return memoize<LRUCache<cacheCapacity>>(std::forward<Callable>(callable));
    }
} // namespace memoization
