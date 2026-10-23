/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IndexedZipper.tpp
*/

#ifndef INDEXEDZIPPER_TPP_
#define INDEXEDZIPPER_TPP_

#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"

namespace engine::ecs {

template <typename... Arrays>
IndexedZipperIterator<Arrays...>::IndexedZipperIterator(
    ZipperIterator<Arrays...> inner)
    : _inner(inner)
{}

template <typename... Arrays>
typename IndexedZipperIterator<Arrays...>::reference
IndexedZipperIterator<Arrays...>::operator*() const
{
    return std::tuple_cat(std::tuple<std::size_t>(_inner.index()), *_inner);
}

template <typename... Arrays>
IndexedZipperIterator<Arrays...> &IndexedZipperIterator<Arrays...>::operator++()
{
    ++_inner;
    return *this;
}

template <typename... Arrays>
IndexedZipperIterator<Arrays...>
IndexedZipperIterator<Arrays...>::operator++(int)
{
    IndexedZipperIterator previous = *this;

    ++*this;
    return previous;
}

template <typename... Arrays>
bool IndexedZipperIterator<Arrays...>::operator==(
    const IndexedZipperIterator &other) const
{
    return _inner == other._inner;
}

template <typename... Arrays>
IndexedZipper<Arrays...>::IndexedZipper(Arrays &...arrays)
    : _zipper(arrays...)
{}

template <typename... Arrays>
typename IndexedZipper<Arrays...>::Iterator
IndexedZipper<Arrays...>::begin() const
{
    return Iterator(_zipper.begin());
}

template <typename... Arrays>
typename IndexedZipper<Arrays...>::Iterator
IndexedZipper<Arrays...>::end() const
{
    return Iterator(_zipper.end());
}

template <typename... Arrays>
IndexedZipper<Arrays...> makeIndexedZipper(Arrays &...arrays)
{
    return IndexedZipper<Arrays...>(arrays...);
}

} // namespace engine::ecs

#endif /* !INDEXEDZIPPER_TPP_ */
