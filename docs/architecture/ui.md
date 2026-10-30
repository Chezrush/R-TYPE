# UI and layout system

In contrast to gameplay entities (spaceships, bullets, Bydos monsters) that exist in
world coordinates and scroll horizontally with the starfield, user interface elements
(HUD, menus, dialogues, buttons) live in viewport screen-space.

To remain responsive across different window resolutions, aspect ratios, and resizing
events, the UI framework provides the `UITransform` component and `UILayoutSystem`.
Interactive widgets are handled via `UIButton` and `UIInteractionSystem`.

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

## Interactive buttons & EventBus decoupling

Per subject decoupling requirements, the UI interaction layer never directly invokes
gameplay rules or network calls. Instead, widget clicks emit typed events on the engine's
`EventBus`.

```mermaid
flowchart LR
    User["Mouse Click / Keyboard (Enter/Space)"] --> Sys["UIInteractionSystem"]
    Sys -- emits --> Event["UIButtonClickedEvent { entityIndex, action }"]
    Event --> Bus["EventBus"]
    Bus --> Sub["Subscribers (e.g. GameScene, MenuScene)"]
```

### Button states

A `UIButton` cycles through 4 states:
- **`NORMAL`**: Idle state.
- **`HOVERED`**: Mouse is currently over the button's `actualBounds`.
- **`PRESSED`**: Mouse button is currently held down over the button.
- **`DISABLED`**: Non-interactive; ignores mouse clicks and keyboard focus.

### Accessibility: Keyboard and Gamepad navigation

For accessibility compliance, menus can be fully navigated without a mouse:
- `Tab` / `Down` / `Right`: Advances focus (`isFocused = true`) to the next enabled button.
- `Up` / `Left`: Moves focus to the previous enabled button.
- `Enter` / `Space` / Gamepad A: Activates the focused button, emitting `UIButtonClickedEvent`.

Focus rules: at most one button is focused at a time, and clicking a button gives it the focus.
A `DISABLED` button is never focused: a button disabled by game code loses its focus at the
next call to `UIInteractionSystem`, and a button disabled while it is held down does not emit a
click when it is released. While the pointer leaves a held button, the button shows `NORMAL`; it
shows `PRESSED` again when the pointer comes back.

### Event listeners and scenes

A UI scene that subscribes to the `EventBus` keeps the `ListenerId` returned by `subscribe()` and
calls `EventBus::unsubscribe()` for each of them in `onExit()` (or in its destructor). Otherwise a
popped scene stays registered and the next event calls into a destroyed object. The bus itself
must outlive every object that subscribed to it. `HudController` is an example: it stores its
listener ids and unsubscribes them in its destructor.

## Rendering

UI drawing lives in a separate library, `rtype::engine_render`, because it needs SFML. It
is only built with the client (`RTYPE_BUILD_CLIENT`), so the server keeps linking the
SFML-free `rtype::engine`. The components themselves stay plain data in `rtype::engine`.

| Component | Draws |
| --- | --- |
| `UIPanel` | A plain colored rectangle, or a texture (`textureId`, optional `textureRect`) stretched (`STRETCH`) or 9-sliced (`NINE_SLICE`, unscaled `borders`). |
| `UIProgressBar` | A background, then a fill of `(value - min) / (max - min)`, clamped to [0, 1], left to right (`HORIZONTAL`) or bottom to top (`VERTICAL`). |
| `UIText` | A UTF-8 string in the font `fontId`, aligned `LEFT`, `CENTER` or `RIGHT`, vertically centered. |

`UIRenderSystem::render()` draws every entity with a `UITransform` and at least one of these
components, inside its `actualBounds`, sorted by `zIndex` (ties keep the entity order). An
entity with several of them draws panel, then bar, then text.

### Fixed screen-space view

Before drawing, the system replaces the target view with `screenView()`, one unit per pixel
of the target, and puts the previous view back afterwards. The world camera can scroll
freely: the HUD never drifts. Keep `UILayoutSystem` at the window size
(`setResolution()` on resize) so `actualBounds` match the pixels.

```cpp
engine::resources::ResourceManager resources("assets");
engine::render::UIRenderSystem uiRenderer(resources);

window.setView(worldCamera);
// ... draw the starfield and the entities ...
uiRenderer.render(window, transforms, panels, bars, texts);
window.display();
```

### Resources

`ResourceManager` resolves ids against its asset root (`"fonts/main.ttf"`) and loads each one
at most once, through the generic `ResourceCache<T>`. A missing file returns `nullptr` and is
remembered, so it is not read again every frame. The renderer then skips the text, or
falls back to the panel color for a missing texture: a missing asset never stops a frame.

## In-game HUD

The arcade HUD lives in `client/` (`Client/Hud/`), in the SFML-free `rtype_client_core`
library:

| Class | Role |
| --- | --- |
| `HudController` | Listens to game events on the `EventBus` and keeps a plain `HudState` (local player, lives, score, stage, beam charge, connected players). |
| `BeamGauge` | Beam charge: empty when idle, fills while shoot is held, empty again on release. |
| `HudView` | Builds the widgets from `UITransform`, `UIPanel`, `UIText` and `UIProgressBar`, and copies a `HudState` into them each frame. |

Bottom bar, left to right: player tags `P1`-`P4` (player colors, grey when the slot is empty,
the local player outlined in white), ship icon and lives, stage, `BEAM` gauge (white when full),
score on 7 digits (`0079500`). Every ship also gets its colored tag above it
(`HudView::setMarker`), outlined in white for the local player.

### Where the data comes from

The HUD never reads the network. The network layer publishes one event per decoded message
(`Client/Events/GameEvents.hpp`), and the controller reacts:

| Event | Protocol message (section 6) | HUD effect |
| --- | --- | --- |
| `WelcomeReceived` | `welcome` | local player index, its slot is connected |
| `EntitySpawned` | `spawn` | a player ship marks its owner connected; the one owned by the local player is our ship (on join and on every respawn) |
| `EntityDestroyed` | `destroy` | our ship: not alive, beam emptied, one life lost when `reason` is `KILLED` |
| `PlayerJoined` / `PlayerLeft` | `player_joined` / `player_left` | player slot connected / free |
| `ScoreChanged` | `score` | score of the local player |
| `GameStateChanged` | `game_state` | level shown as `STAGE n`, game phase kept for the overlays |

Player indexes outside the 4 HUD slots (and `owner = 0xFF`) are ignored. A `net_id` can be reused
after its `destroy`, so the controller forgets our ship's id as soon as it is destroyed.

The protocol has no lives field yet: lives are counted on the client (3 to start). The
controller unsubscribes its listeners when destroyed, so the `EventBus` must outlive it.
