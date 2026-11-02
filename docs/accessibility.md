# Accessibility

The game and this documentation must be usable by as many people as possible. For each family
of disability we list what exists today and what is planned. Items marked **todo** are not
implemented yet and must be tracked as issues.

## Physical and motor

| Measure | Status |
| --- | --- |
| Arrow keys **and** WASD both work, Space to shoot | done |
| Only 5 keys needed, no key combinations, no mouse | done |
| Remappable keys, stored as physical scancodes | done |
| Auto-fire option (hold-free shooting), hold or toggle for the beam | option done, gameplay input to come |
| Gamepad bindings and menu navigation | UI done, device reading to come |
| Adjustable game speed / difficulty | todo |

## Audio and visual

| Measure | Status |
| --- | --- |
| Players differ by **colour** and the local player has an outline | done |
| Players also differ by **number and shape**: circle, triangle, square, cross | done |
| Protanopia, deuteranopia and tritanopia palettes for the UI | done |
| HUD indicator for sounds: a label on the side they come from | indicator done, sound events to come |
| Separate volumes for music and effects | settings done, audio to come |
| Scalable UI (100 to 150 %), high contrast: plates behind text, thick outlines | done |

### Colour-blind palettes

Each palette is checked with a simulation of the vision it targets (Machado, Oliveira and Fernandes,
2009, full severity). A unit test requires a colour distance of at least 25 (CIE76) between any two
players, and a text contrast of at least 4.5:1 on player tags. Players also keep their number and
shape, so colour is never the only clue.

The images below show the HUD as seen through that simulation:

| Palette | Simulated vision |
| --- | --- |
| Protanopia | ![HUD, protanopia palette, simulated protanopia](images/accessibility/protanopia.png) |
| Deuteranopia | ![HUD, deuteranopia palette, simulated deuteranopia](images/accessibility/deuteranopia.png) |
| Tritanopia | ![HUD, tritanopia palette, simulated tritanopia](images/accessibility/tritanopia.png) |
| Normal, no simulation | ![HUD, normal palette](images/accessibility/normal.png) |

Game sprites are not tinted by the palettes, so a ship can differ from the colour of its tag.

## Mental and cognitive

| Measure | Status |
| --- | --- |
| Very simple rules: move, shoot, avoid | done |
| Reduced motion option: stops the star-field and the full beam color change | done |
| Pause and difficulty settings | todo |
| "How to play" screen in short, plain sentences | done |

## Documentation

- Written in plain English, short paragraphs, headings for navigation and tables for
  comparisons, so it works with screen readers.
- Diagrams are Mermaid text sources: every diagram has a prose equivalent next to it.
- No information is conveyed by colour alone in the docs.
