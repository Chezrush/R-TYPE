/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Zipper.tpp
*/

#ifndef ZIPPER_TPP_
#define ZIPPER_TPP_

#include <algorithm>
#include <utility>

#include "Engine/Ecs/Zipper.hpp"

namespace engine::ecs {

template <typename... Arrays>
ZipperIterator<Arrays...>::ZipperIterator(std::tuple<Arrays *...> arrays,
                                          std::size_t index, std::size_t end)
    : _arrays(arrays),
      _index(index),
      _end(end)
{
    seek();
}

template <typename... Arrays>
typename ZipperIterator<Arrays...>::reference
ZipperIterator<Arrays...>::operator*() const
{
    return std::apply(
        [this](auto *...arrays) {
            return reference{*arrays->begin()[_index]...};
        },
        _arrays);
}

template <typename... Arrays>
ZipperIterator<Arrays...> &ZipperIterator<Arrays...>::operator++()
{
    ++_index;
    seek();
    return *this;
}

template <typename... Arrays>
ZipperIterator<Arrays...> ZipperIterator<Arrays...>::operator++(int)
{
    ZipperIterator previous = *this;

    ++*this;
    return previous;
}

template <typename... Arrays>
bool ZipperIterator<Arrays...>::operator==(const ZipperIterator &other) const
{
    return _index == other._index;
}

template <typename... Arrays>
std::size_t ZipperIterator<Arrays...>::index() const noexcept
{
    return _index;
}

template <typename... Arrays>
bool ZipperIterator<Arrays...>::isComplete() const
{
    return std::apply(
        [this](auto *...arrays) {
            return (arrays->begin()[_index].has_value() && ...);
        },
        _arrays);
}

template <typename... Arrays>
void ZipperIterator<Arrays...>::seek()
{
    while (_index < _end && !isComplete())
        ++_index;
}

template <typename... Arrays>
Zipper<Arrays...>::Zipper(Arrays &...arrays)
    : _arrays(&arrays...),
      _size(std::min({arrays.size()...}))
{}

template <typename... Arrays>
typename Zipper<Arrays...>::Iterator Zipper<Arrays...>::begin() const
{
    return Iterator(_arrays, 0, _size);
}

template <typename... Arrays>
typename Zipper<Arrays...>::Iterator Zipper<Arrays...>::end() const
{
    return Iterator(_arrays, _size, _size);
}

template <typename... Arrays>
Zipper<Arrays...> makeZipper(Arrays &...arrays)
{
    return Zipper<Arrays...>(arrays...);
}

} // namespace engine::ecs

#endif /* !ZIPPER_TPP_ */
