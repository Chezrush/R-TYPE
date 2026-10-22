# UI and layout system

In contrast to gameplay entities (spaceships, bullets, Bydos monsters) that exist in
world coordinates and scroll horizontally with the starfield, user interface elements
(HUD, menus, dialogues, buttons) live in viewport screen-space.

To remain responsive across different window resolutions, aspect ratios, and resizing
events, the UI framework provides the `UITransform` component and `UILayoutSystem`.

## Anchoring model

`UIAnchor` provides 9 attachment points across the screen viewport:

```
TOP_LEFT (0,0) ----------- TOP_CENTER (W/2,0) ----------- TOP_RIGHT (W,0)
     |                           |                              |
CENTER_LEFT (0,H/2) ------    CENTER (W/2,H/2)    ------ CENTER_RIGHT (W,H/2)
     |                           |                              |
BOTTOM_LEFT (0,H) -------- BOTTOM_CENTER (W/2,H) -------- BOTTOM_RIGHT (W,H)
```

- **`offset`**: 2D pixel offset relative to the chosen anchor.
- **`size`**: Dimensions of the element (in absolute pixels or normalized relative to
  viewport dimensions via `normalizedSize`).
- **`zIndex`**: Layer ordering (higher z-index values draw on top of lower ones).
- **`actualBounds`**: The resolved 2D rectangle (`x, y, width, height`) computed by
  `UILayoutSystem`.

## UILayoutSystem

`UILayoutSystem` resolves the final screen bounds for all entities with a `UITransform`:

```cpp
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

// Setup layout system with initial window resolution:
engine::ecs::UILayoutSystem layoutSystem(1280.0f, 720.0f);

// Create a HUD element anchored at the bottom-center of the screen:
engine::ecs::UITransform hudTransform;
hudTransform.anchor = engine::ecs::UIAnchor::BOTTOM_CENTER;
hudTransform.size = {800.0f, 60.0f};
hudTransform.offset = {0.0f, -20.0f}; // 20 pixels above the bottom edge

// When the window is resized to 1920x1080:
layoutSystem.setResolution(1920.0f, 1080.0f);
layoutSystem.update(transforms);

// hudTransform.actualBounds is now: {560.0f, 1000.0f, 800.0f, 60.0f}
// The element remains centered and 20px above the bottom without drift.
```

