# Accessibility

The game and this documentation must be usable by as many people as possible. For each family
of disability we list what exists today and what is planned. Items marked **todo** are not
implemented yet and must be tracked as issues. **Not in game** means the code and its tests
exist but the `r-type_client` does not apply it yet. The client starts on a main menu (Play,
Settings, Controls, How to play, Quit); Escape in the game opens the in-game menu. The settings
screen writes `~/.config/r-type/settings.cfg`, which is read at start.

## Physical and motor

| Measure | Status |
| --- | --- |
| Arrow keys **and** WASD both work, Space to shoot | done |
| Only 5 keys needed, no key combinations, no mouse | done |
| Remappable keys, stored as physical scancodes | done (Controls screen, from the main or the in-game menu) |
| Auto-fire option (hold-free shooting) | done (Controls screen) |
| Hold or toggle for the beam | not in game (no beam yet) |
| Menu navigation with the keyboard only (Tab, arrows, Enter, Escape) or the mouse | done |
| Gamepad bindings and menu navigation | not in game (UI only, device not read) |
| Adjustable game speed / difficulty | todo |

## Audio and visual

| Measure | Status |
| --- | --- |
| Players differ by **colour** and the local player has an outline | done |
| Players also differ by **sprite** (one ship row per player) and by a **number in the score list** | done |
| Distinct marker **shapes** per player: circle, triangle, square, cross (slots 1 to 4) | done |
| Protanopia, deuteranopia and tritanopia palettes for the HUD and player tags | done (applied in game) |
| HUD indicator for sounds: a label on the side they come from | done (shots, enemy fire and kills) |
| Volumes (master, effects) and mute | done (Settings screen, applied at once; there is no music yet) |
| High contrast: plates behind text, thick outlines | done (applied in game) |
| Scalable UI (100 to 150 %) | done for the menus, the HUD and the overlays (the game world is not scaled) |
| Resolution, fullscreen and vsync options | done (Settings screen, applied at once) |

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
| Reduced motion option: stops the star-field and the full beam color change | done |
| In-game menu with Resume (the game goes on behind it: the server is authoritative, there is no pause) | done |
| Difficulty settings | todo |
| "How to play" screen in short, plain sentences | done (main menu and in-game menu) |

## Documentation

- Written in plain English, short paragraphs, headings for navigation and tables for
  comparisons, so it works with screen readers.
- Diagrams are Mermaid text sources: every diagram has a prose equivalent next to it.
- No information is conveyed by colour alone in the docs.
