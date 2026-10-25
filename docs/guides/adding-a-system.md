# How-to: add a component and a system

Example: a `Lifetime` component that kills an entity after a delay (useful for explosions
and missiles). The engine has no component of its own, so the component and the system belong
to the module of the game. The code below was compiled against the engine as written.

## 1. Define the component (plain data)

```cpp
struct Lifetime {
    double remaining; // seconds
};
```

A component is a struct with data only. The logic goes in a system.

## 2. Register it

```cpp
registry.registerComponent<Lifetime>();
```

Do it once, before the first use. Using an unregistered type throws `std::logic_error`.

## 3. Write the system

A system is registered in a phase of the `Scheduler`. List the components it needs: the
function is called once per entity that owns all of them.

```cpp
scheduler.addSystem<Lifetime>(
    "lifetime", Phase::UPDATE,
    [](Entity entity, Lifetime &lifetime, CommandBuffer &commands,
       double step) {
        lifetime.remaining -= step;
        if (lifetime.remaining <= 0.0)
            commands.kill(entity);
    });
```

- Write the component type as `const Position` for read access, plain `Position` for write
  access. The scheduler uses it to detect conflicts between systems of one phase
  (`scheduler.findConflicts()`).
- Never kill or spawn directly from a system. Record it in the `CommandBuffer`: the scheduler
  applies it after the phase, so the iteration stays valid.
- `step` is the fixed step in seconds, never the frame time.

Other commands: `commands.spawn(callback)`, `commands.add(entity, component)`,
`commands.emplace<C>(entity, args...)` and `commands.remove<C>(entity)`.

## 4. Create entities

```cpp
const Entity spark = registry.spawnEntity();
registry.addComponent(spark, Lifetime{0.5});
```

## 5. Run it

The scheduler is driven by the step callback of the game loop:

```cpp
GameLoop loop(
    FixedTimestep(1.0 / 60.0, 5), clock,
    [&](double step) { scheduler.run(registry, step); });
loop.run();
```

`clock` returns the time in seconds. In a test, pass a function returning a variable you
control and call `loop.tick()` yourself.

## 6. Test it

No window or socket is needed:

```cpp
double now = 0.0;
GameLoop loop(
    FixedTimestep(0.25, 5), [&now] { return now; },
    [&](double step) { scheduler.run(registry, step); });

now = 0.5;
loop.tick();
loop.tick();
REQUIRE_FALSE(registry.isAlive(spark));
```

Put the test in `engine/tests/` for engine code, or in the tests of the module that owns the
system, and register the file in the matching `CMakeLists.txt`.

## Several components, several phases

- Systems of the same phase run in registration order. Put input and network reads in
  `PRE_UPDATE`, gameplay in `UPDATE`, cleanup in `POST_UPDATE`.
- To iterate by hand inside a raw system, use `makeZipper` or `makeIndexedZipper` on
  `registry.getComponents<C>()`.

## Rules of thumb

- Components hold data only. Logic lives in systems.
- Do not create or kill entities while iterating a component array.
- A reference to a component is invalidated as soon as an entity is spawned. Copy the values
  you need.
- If the new entity must reach the clients, it also needs a change in
  [the protocol](../protocol/rtype-protocol.md) and in the client.
