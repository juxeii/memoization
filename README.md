[![Build status][ci-image]][ci-link]
# memoization
A tiny [memoization] library written in C++17.
# Usage
Normal caching 
```
auto m = memoization::memoize([](int) { return 42; });
```
With a **l**east **r**ecently **u**sed cache type(LRU)
```
auto m = memoization::memoizeWithLRU<3>([](int) { return 42; });
```

[ci-image]: https://github.com/juxeii/memoization/workflows/build/badge.svg
[ci-link]: https://github.com/juxeii/memoization/actions?query=workflow%3Abuild
[memoization]: https://en.wikipedia.org/wiki/Memoization