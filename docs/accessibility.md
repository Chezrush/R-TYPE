# Accessibility

The game and this documentation must be usable by as many people as possible. For each family
of disability we list what exists today and what is planned. Items marked **todo** are not
implemented yet and must be tracked as issues. **Not in game** means the code and its tests
exist (settings, widgets) but the `r-type_client` game loop does not apply it yet. The settings
are read from `~/.config/r-type/settings.cfg`; the client starts straight in the game, so the
settings and help screens are not reachable from it.

## Physical and motor

| Measure | Status |
| --- | --- |
| Arrow keys **and** WASD both work, Space to shoot | done |
| Only 5 keys needed, no key combinations, no mouse | done |
| Remappable keys, stored as physical scancodes | done (read from the settings file, no in-game screen) |
| Auto-fire option (hold-free shooting) | done (settings file) |
| Hold or toggle for the beam | not in game (no beam yet) |
| Gamepad bindings and menu navigation | not in game (UI only, device not read) |
| Adjustable game speed / difficulty | todo |

## Audio and visual

| Measure | Status |
| --- | --- |
| Players differ by **colour** and the local player has an outline | done |
| Players also differ by **sprite** (one ship row per player) and by a **number in the score list** | done |
| Distinct marker **shapes** per player: circle, triangle, square, cross (slots 1 to 4) | done |
| Protanopia, deuteranopia and tritanopia palettes for the HUD and player tags | done (applied in game) |
| HUD indicator for sounds: a label on the side they come from | not in game (widget only) |
| Separate volumes for music and effects | not in game (setting stored, not applied) |
| High contrast: plates behind text, thick outlines | done (applied in game) |
| Scalable UI (100 to 150 %) | not in game (setting stored, not applied) |
| Resolution, fullscreen and vsync options | done (applied at start) |

### Colour-blind palettes

Each palette is checked with a simulation of the vision it targets (Machado, Oliveira and Fernandes,
2009, full severity). A unit test requires a colour distance of at least 25 (CIE76) between any two
players, and a text contrast of at least 4.5:1 on player tags. Players also keep their number and
shape, so colour is never the only clue.

Game sprites are not tinted by the palettes, so a ship can differ from the colour of its tag.

## Mental and cognitive

| Measure | Status |
| --- | --- |
| Very simple rules: move, shoot, avoid | done |
| Reduced motion option: stops the star-field and the full beam color change | not in game (setting stored, the star-field always scrolls) |
| Pause and difficulty settings | todo |
| "How to play" screen in short, plain sentences | screen done, not reachable from the client yet |

## Documentation

- Written in plain English, short paragraphs, headings for navigation and tables for
  comparisons, so it works with screen readers.
- Diagrams are Mermaid text sources: every diagram has a prose equivalent next to it.
- No information is conveyed by colour alone in the docs.
