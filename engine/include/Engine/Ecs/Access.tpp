/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Access.tpp
*/

#ifndef ACCESS_TPP_
#define ACCESS_TPP_

#include "Engine/Ecs/Access.hpp"

namespace engine::ecs {

template <typename Component>
Access &Access::read()
{
    _reads.insert(std::type_index(typeid(Component)));
    return *this;
}

template <typename Component>
Access &Access::write()
{
    _writes.insert(std::type_index(typeid(Component)));
    return *this;
}

} // namespace engine::ecs

#endif /* !ACCESS_TPP_ */
