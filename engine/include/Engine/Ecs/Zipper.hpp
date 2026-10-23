/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Zipper.hpp
*/

#ifndef ZIPPER_HPP_
#define ZIPPER_HPP_

#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>

namespace engine::ecs {

/** @brief Reference to the component held by a SparseArray, const-aware. */
template <typename Array>
using ComponentRef =
    std::conditional_t<std::is_const_v<Array>,
                       const typename Array::Slot::value_type &,
                       typename Array::Slot::value_type &>;

/**
 * @brief Input iterator over the indices owning a component in every array.
 *
 * Dereferencing yields a tuple of references, one per array. The iterator
 * keeps pointers to the arrays: it is invalidated when one of them grows.
 *
 * @tparam Arrays SparseArray types, const-qualified for read-only access.
 */
template <typename... Arrays>
class ZipperIterator {
public:
    using value_type = std::tuple<ComponentRef<Arrays>...>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    ZipperIterator() = default;

    /**
     * @param arrays The arrays to walk in lockstep.
     * @param index First index to consider, moved to the next full match.
     * @param end One past the last index to visit.
     */
    ZipperIterator(std::tuple<Arrays *...> arrays, std::size_t index,
                   std::size_t end);

    [[nodiscard]] reference operator*() const;
    ZipperIterator &operator++();
    ZipperIterator operator++(int);
    [[nodiscard]] bool operator==(const ZipperIterator &other) const;

private:
    [[nodiscard]] bool isComplete() const;
    void seek();

    std::tuple<Arrays *...> _arrays{};
    std::size_t _index = 0;
    std::size_t _end = 0;
};

/**
 * @brief View over the entities owning every component of several arrays.
 *
 * Iteration stops at the shortest array, so arrays of different sizes are
 * safe. The view does not own the arrays and must not outlive them.
 *
 * @tparam Arrays SparseArray types, const-qualified for read-only access.
 */
template <typename... Arrays>
class Zipper {
    static_assert(sizeof...(Arrays) > 0, "a zipper needs at least one array");

public:
    using Iterator = ZipperIterator<Arrays...>;

    explicit Zipper(Arrays &...arrays);

    [[nodiscard]] Iterator begin() const;
    [[nodiscard]] Iterator end() const;

private:
    std::tuple<Arrays *...> _arrays;
    std::size_t _size;
};

template <typename... Arrays>
Zipper(Arrays &...) -> Zipper<Arrays...>;

} // namespace engine::ecs

#include "Engine/Ecs/Zipper.tpp"

#endif /* !ZIPPER_HPP_ */
