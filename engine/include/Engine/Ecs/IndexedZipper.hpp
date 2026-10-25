/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IndexedZipper.hpp
*/

#ifndef INDEXEDZIPPER_HPP_
#define INDEXEDZIPPER_HPP_

#include <cstddef>
#include <iterator>
#include <tuple>

#include "Engine/Ecs/Zipper.hpp"

namespace engine::ecs {

/**
 * @brief Input iterator of Zipper that also yields the entity index.
 *
 * Dereferencing gives a tuple whose first element is the index, followed by
 * one reference per array. The entities visited are those of ZipperIterator.
 *
 * @tparam Arrays SparseArray types, const-qualified for read-only access.
 */
template <typename... Arrays>
class IndexedZipperIterator {
public:
    using value_type = std::tuple<std::size_t, ComponentRef<Arrays>...>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    IndexedZipperIterator() = default;
    explicit IndexedZipperIterator(ZipperIterator<Arrays...> inner);

    [[nodiscard]] reference operator*() const;
    IndexedZipperIterator &operator++();
    IndexedZipperIterator operator++(int);
    [[nodiscard]] bool operator==(const IndexedZipperIterator &other) const;

private:
    ZipperIterator<Arrays...> _inner;
};

/**
 * @brief View like Zipper whose elements start with the entity index.
 *
 * The view does not own the arrays and must not outlive them.
 *
 * @tparam Arrays SparseArray types, const-qualified for read-only access.
 */
template <typename... Arrays>
class IndexedZipper {
public:
    using Iterator = IndexedZipperIterator<Arrays...>;

    /** @param arrays The arrays to walk in lockstep, at least one. */
    explicit IndexedZipper(Arrays &...arrays);

    /** @return An iterator on the first entity owning every component. */
    [[nodiscard]] Iterator begin() const;

    /** @return The past-the-end iterator. */
    [[nodiscard]] Iterator end() const;

private:
    Zipper<Arrays...> _zipper;
};

template <typename... Arrays>
IndexedZipper(Arrays &...) -> IndexedZipper<Arrays...>;

/** @brief Builds an IndexedZipper, deducing the array types. */
template <typename... Arrays>
[[nodiscard]] IndexedZipper<Arrays...> makeIndexedZipper(Arrays &...arrays);

} // namespace engine::ecs

#include "Engine/Ecs/IndexedZipper.tpp"

#endif /* !INDEXEDZIPPER_HPP_ */
