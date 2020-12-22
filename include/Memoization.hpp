#pragma once

#include <functional>
#include <map>
#include <optional>
#include <queue>
#include <tuple>
#include "CallableTraits.hpp"

namespace memoization
{
    template <auto CacheCapacity>
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

    template <typename Container, auto CacheCapacity>
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

    template <typename T, auto CacheCapacity, typename Key>
    auto findInCache(const LRUCacheImpl<T, CacheCapacity> &lru, const Key &key)
    {
        return findInCache(lru.cacheMap, key);
    }

    template <typename Cache, typename... Args>
    auto emplaceInCache(Cache &cache, Args &&... args)
    {
        return cache.try_emplace(std::forward<Args>(args)...).first;
    }

    template <typename T, auto CacheCapacity, typename... Args>
    auto emplaceInCache(LRUCacheImpl<T, CacheCapacity> &lru, Args &&... args)
    {
        if (lru.cacheQueue.size() == CacheCapacity)
        {
            lru.cacheMap.erase(lru.cacheQueue.front());
            lru.cacheQueue.pop();
        }
        auto iterator = emplaceInCache(lru.cacheMap, std::forward<Args>(args)...);
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
    struct create_cache
    {
        using type = OrderedMap<Callable>;
    };

    template <auto CacheCapacity, typename Callable>
    struct create_cache<LRUCache<CacheCapacity>, Callable>
    {
        static_assert(CacheCapacity > 0, "LRU cache capacity must be greater 0!");
        using type = LRUCacheImpl<OrderedMap<Callable>, CacheCapacity>;
    };

    template <typename Cache, typename Callable>
    using create_cache_t = typename create_cache<Cache, Callable>::type;

    template <typename CacheTag, typename Result>
    struct result_adaption : std::add_lvalue_reference<std::add_const_t<Result>>
    {
    };

    template <auto CacheCapacity, typename Result>
    struct result_adaption<LRUCache<CacheCapacity>, Result> : std::remove_cv<std::remove_reference_t<Result>>
    {
    };

    template <typename CacheTag, typename Result>
    using result_adaption_t = typename result_adaption<CacheTag, Result>::type;

} // namespace memoization::detail

namespace memoization
{
    using namespace memoization::detail;

    template <typename CacheTag = MapCache, typename Callable>
    [[nodiscard]] auto memoize(Callable &&callable) noexcept
    {
        using ResultType = callable_result_t<Callable>;
        static_assert(!std::is_void_v<ResultType>, "Callable with void return type cannot be memoized!");
        static_assert(
            !(std::is_reference_v<ResultType> && !std::is_const_v<std::remove_reference_t<ResultType>>),
            "Callable with non-const reference return type cannot be memoized!");
        static_assert(std::is_copy_constructible_v<ResultType>, "Callable return type must be copy constructible!");
        static_assert(
            std::tuple_size_v<callable_arguments_t<Callable>> > 0, "Callable with no arguments cannot be memoized!");

        using AdaptedResultType = result_adaption_t<CacheTag, ResultType>;
        using CacheType = create_cache_t<CacheTag, callable_to_function_t<Callable>>;

        return createMemoizer<AdaptedResultType>(std::forward<Callable>(callable), CacheType{});
    }

    template <auto CacheCapacity, typename Callable>
    [[nodiscard]] auto memoizeWithLRU(Callable &&callable) noexcept
    {
        return memoize<LRUCache<CacheCapacity>>(std::forward<Callable>(callable));
    }
} // namespace memoization
