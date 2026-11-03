# Scenes and state machine

The client application transitions between multiple states (e.g., Boot, Main Menu, Lobby,
Game session, Settings) and displays modal overlays (Pause menu, Game Over).

To keep the game engine decoupled and reusable, the state machine is implemented in
`engine/core` via `IScene` and `SceneManager`. It knows nothing about R-Type specific
scenes, which live in `client/`.

## Scene lifecycle

Every screen or modal overlay implements the `IScene` interface:

```mermaid
stateDiagram-v2
    [*] --> onEnter: pushScene() / switchScene()
    onEnter --> Active: Ready
    Active --> onPause: Another scene pushed on top
    onPause --> onResume: Top scene popped
    onResume --> Active: Resumed
    Active --> onExit: popScene() / switchScene()
    onExit --> [*]: Destroyed
```

| Method | When called |
| --- | --- |
| `onEnter()` | When the scene becomes the active top scene for the first time. |
| `onExit()` | When the scene is removed from the stack and about to be destroyed. |
| `onPause()` | When a new scene or overlay is pushed on top of this scene. |
| `onResume()` | When the scene on top is popped and this scene resumes as active. |
| `update(dt)` | Ticked each frame for the top scene with delta time in seconds. |
| `render()` | Called to draw the scene. |
| `handleInput()` | Dispatches input events to the active top scene. |
| `isOverlay()` | Returns `true` if this is a transparent overlay (e.g. Pause menu). |

## Scene stack and overlays

`SceneManager` owns a stack of scenes (`std::vector<std::unique_ptr<IScene>>`):

- **Pushing a scene (`pushScene`)**: Pauses the current scene (`onPause()`) and pushes the
  new one (`onEnter()`).
- **Popping a scene (`popScene`)**: Exits the top scene (`onExit()`), destroys it, and
  resumes the underlying scene (`onResume()`).
- **Switching a scene (`switchScene`)**: Exits all existing scenes on the stack and installs
  the new scene as the single active base scene.

### Transparent overlay rendering

When the top scene is an overlay (`isOverlay() == true`), `SceneManager::render()` iterates
from the lowest visible base scene up to the top scene. For example, when `PauseOverlay`
is pushed on top of `GameScene`:

```mermaid
flowchart TD
    subgraph Stack ["Scene Stack"]
        Pause["PauseOverlay (isOverlay = true)"]
        Game["GameScene (isOverlay = false)"]
    end
    Render["SceneManager::render()"] --> DrawGame["1. Draw GameScene (paused world)"]
    DrawGame --> DrawPause["2. Draw PauseOverlay (semi-transparent backdrop & buttons)"]
```

While both scenes are rendered, **only the top scene receives `handleInput()`**, so player
keystrokes do not fire missiles while navigating a menu. By default only the top scene receives
`update()` too. An overlay whose `updatesSceneBelow()` returns `true` also keeps the scene below
it updating, first, as a server-authoritative game does behind its menu.

## UI scenes and navigation

Two generic helpers build screens for any game. They know nothing about R-Type:

- **`engine::ecs::UIScene`** (no SFML) is the base of a screen made only of UI components. It
  creates the widgets and listeners on `onEnter()` / `onResume()` (the `activate()` you write)
  and removes them on `onExit()` / `onPause()`. `handleInput()` applies the input the window code
  gave to the shared `UIInteractionSystem`. `render()` does nothing, the game's `UIRenderSystem`
  draws the registry. The look of the buttons comes from `UISceneContext::buttonStyle`.
- **`engine::core::SceneRouter<Route>`** changes screen. A scene publishes
  `NavigationRequested<Route>`, and `apply()` switches between two frames: switching at once would
  destroy the scene while its own listener runs. `Route` is chosen by the game, usually an enum. A
  route with no scene yet is ignored. `OverlayRequested<Route>` and `OverlayCloseRequested` push
  and close overlays the same way, and `OverlaysClearRequested` closes all of them. `QuitRequested` sets `quitRequested()`.
- **`engine::ecs::UIOverlay`** is a `UIScene` drawn above the others: its widgets get a higher
  z-index, `addBackdrop()` covers the window, and the scene below keeps updating unless told not to.

The main loop runs, in order:

```cpp
scenes.handleInput();
scenes.update(dt);
router.apply();
if (router.quitRequested())
    stop();
```

## R-Type menu scenes

`MainMenuScene` and `ConnectScene` live in `client/` and derive from `UIScene`.
`client::scenes::SceneRoute` lists the screens of the game, and `makeMenuContext()` gives the
scenes the R-Type button colors.

### Overlays above the game

`GameOverlays` decides what is above the game, from events only (`client/`). There is at most one
overlay. Game over wins over the in-game menu, which wins over the respawn countdown:

| Overlay | Opens on | Closes on |
| --- | --- | --- |
| `InGameMenuOverlay` | `Escape` (`UICancelEvent`) | `Escape`, "Resume"; "Disconnect" leaves the game |
| `RespawnOverlay` | our ship in `EntityDestroyed` with reason `KILLED` | our ship in `EntitySpawned` |
| `GameOverOverlay` | `GameStateChanged` with phase `GAME_OVER` | "Back to menu" leaves the game |

The menu publishes `InGameMenuOpened` / `InGameMenuClosed`; leaving publishes
`DisconnectRequested`. The countdown keeps running under the menu. The server is never paused.

### Settings

`SettingsScene` is an overlay opened from the main menu and from the in-game menu. It edits
`ClientSettings` and publishes the changes, without applying them: `VolumeChanged`, `MuteChanged`
and `VideoSettingsChanged` are for the audio and window code. Only the UI scale is applied by the
screen itself, on `UILayoutSystem`.

`SettingsStore` keeps the values in a `key=value` file (`settings.cfg` in the user data directory),
written when the screen exits. A missing or broken file gives usable settings: each invalid line is
ignored and the default stays. At startup the client calls `store.load()` and applies the values:

```cpp
layout.setScale(static_cast<float>(store.settings().uiScalePercent) / 100.0f);
```

### Controls and how to play

`KeyBindings` holds a key and a gamepad button per action, plus the auto-fire and beam charge
options. Keys are USB HID codes: they name a position on the keyboard, so they work on QWERTY and
AZERTY, and depend on no window library. The window code converts its own scancodes to them. The
bindings are saved with `ClientSettings`, and the "Reset to defaults" of each screen only touches its
own values.

`ControlsScene` changes them. Activating a row waits for a key or a button: meanwhile the UI input
is discarded, and the scene reads `RawKeyPressed` and `RawGamepadButtonPressed`. The window code must
publish those before calling `handleInput()`. A conflict shows a warning without refusing the change.
Every change publishes `BindingsChanged`. `HowToPlayScene` explains the game with the current
bindings.

### Accessibility options

The palette, high contrast and reduced motion are in the settings screen and in `ClientSettings`.
`makeTheme()` turns them into a `UITheme`, which the client owns: `SettingsScene` updates it, and
`UIRenderSystem` and the scenes read it, so a change shows at once. At startup the client builds it
from the loaded settings, gives it to `setTheme()` and `makeMenuContext()`, and publishes
`ReducedMotionChanged` for `HudController` and the rest of the game.

`VisualCueIndicator` shows `VisualCueRequested { icon, direction }` as a label on that side of the
screen. The audio and gameplay code publish those events.

### Developer console

`ConsoleOverlay` is only the screen: it sends lines and shows text, while the code that owns a
command registers and runs it. `ConsoleService` lives as long as the game, keeps the log and the
known command names while the console is closed, and opens it on the key above Tab (`KEY_GRAVE`,
from `RawKeyPressed`).

| Event | Direction | Meaning |
| --- | --- | --- |
| `ConsoleCommandSubmitted { line }` | console → game | Enter on a non-empty line |
| `ConsoleOutput { text, level }` | game → console | A line to show: info, warning or error |
| `ConsoleCommandsListed { names }` | game → console | The names used by Tab completion |
| `TextEntryStarted` / `TextEntryEnded` | console → input code | Typing must not move the ship |

Up / Down browse the history and Tab completes the command name; this logic is in `ConsoleModel`,
with no UI. Escape and the key above Tab close the console. The game keeps updating behind it and
does not receive the input meanwhile.

### Stats overlay

`StatsOverlay` shows performance and network values at the top right. It is not a scene, so it
never takes the input. F3 (`RawKeyPressed` with `KEY_F3`) shows or hides it. The render loop calls
`update(dt)` with the real frame time, also while it is hidden, so the graphs are already full
when it opens.

- FPS and frame time are measured by the overlay itself from `dt` (`StatsModel`, no UI).
- RTT, loss, traffic and dropped datagrams come from `NetStatsUpdated`, published by the network
  layer. Until the first one, the lines say "no data".
- Any code can add a line with `StatsLine { name, value }`, up to 8 lines.

The texts refresh 4 times per second and the graphs (`UIGraph`, 120 samples) every frame.
Measured cost per frame on a release build (AMD Radeon 610M laptop, shown, 8 extra lines, a network
event every frame): about 0.002 ms for `update()` and the layout, and 0.045 ms of CPU time for
`UIRenderSystem::render()` to draw it, about 0.05 ms in total against a budget of 0.2 ms. GPU time
was not isolated. The first figure is checked by a hidden test, run with
`rtype_client_tests "[benchmark]"`; the drawing time was measured by hand.

### Rooms, lobby and chat

These screens are for a multi-instance server (Track #2). Rooms and chat do not exist yet on the
server or in the protocol, so the screens only publish requests and show events
(`Client/Events/LobbyEvents.hpp`), and are tested with fake ones.

| Screen | Publishes | Shows |
| --- | --- | --- |
| `RoomBrowserScene` | `RoomListRequested`, `JoinRoomRequested`, `CreateRoomRequested`, `DisconnectRequested` | `RoomListReceived`, `JoinFailed`, then the lobby on `LobbyUpdated` |
| `LobbyScene` | `ReadyChanged`, `StartGameRequested` (host, everyone ready), `LeaveRoomRequested`, `ChatMessageSent` | `LobbyUpdated` (slots live), `ChatMessageReceived`, the game on `GameStateChanged` PLAYING |
| `ChatOverlay` | `ChatMessageSent`, `TextEntryStarted` / `TextEntryEnded` | `ChatMessageReceived` |

Each lobby slot shows the player's color, number and shape, as the HUD does. `ChatService` lives as
long as the client: it keeps the chat for the lobby and the game, stamps each line with the time
since the start of the client, and handles `/mute 2` and `/unmute 2` locally (the player numbers of
the HUD). In game, T opens `ChatOverlay` above the game, which keeps running; the game scene calls
`setGameRunning(true)` to allow it. Enter sends and closes, Escape closes. Our own messages show when
the server sends them back.

### Talking to the network layer

`ConnectScene` never calls network code. All the messages are `EventBus` events, declared in
`Client/Events/ConnectionEvents.hpp` and `GameEvents.hpp`:

| Event | Direction | Effect |
| --- | --- | --- |
| `ConnectRequested { host, port, nickname }` | scene → network | The form was valid; it is locked. |
| `ConnectCancelled` | scene → network | The player left during a connection. |
| `ConnectionStarted { host, port }` | network → scene | Shows "Connecting to host:port...". |
| `WelcomeReceived` | network → scene | Asks for the game scene. |
| `ConnectionFailed { reason }` | network → scene | Shows a message and unlocks the form. |

`ConnectionFailure` uses the protocol `reject` values, plus `TIMEOUT` and `INVALID_ADDRESS`
found by the client. An unknown reason still shows a generic message. The checks and the texts
are pure functions in `Client/Scenes/ConnectForm.hpp`.
