/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIComponents.hpp
*/

#ifndef UICOMPONENTS_HPP_
#define UICOMPONENTS_HPP_

#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIProgressBar.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Registry.hpp"

namespace engine::ecs {

/**
 * @brief Registers every UI component type in the registry.
 *
 * Call it once before using the UI systems. Registering twice keeps the
 * existing components.
 */
void registerUIComponents(Registry &registry);

} // namespace engine::ecs

#endif /* !UICOMPONENTS_HPP_ */
