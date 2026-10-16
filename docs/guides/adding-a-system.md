# How-to: add a component and a system

Example: a `lifetime` component that removes entities after a delay (useful for explosions).

## 1. Define the component (plain data)

`game/include/rtype/components.hpp`:

```cpp
struct lifetime {
    float remaining; // seconds
};
```

## 2. Register it

`game/src/factory.cpp`, in `register_components`:

```cpp
reg.register_component<lifetime>();
```

## 3. Write the system

`game/include/rtype/systems.hpp` / `game/src/systems.cpp`. A system receives the registry and
the arrays it asked for:

```cpp
struct lifetime_system {
    float const& dt;
    std::vector<std::size_t>& dead;
    void operator()(registry&, sparse_array<components::lifetime>& lifetimes) const {
        for (auto&& [i, life] : engine::ecs::indexed_zipper(lifetimes)) {
            life.remaining -= dt;
            if (life.remaining <= 0.f) {
                dead.push_back(i); // never kill while iterating
            }
        }
    }
};
```

## 4. Plug it in

On the server, `server::server()` registers systems in the order they must run:

```cpp
_reg.add_system<lifetime>(systems::lifetime_system{_dt, _dead});
```

Anything that must also reach the clients (a new kind of entity, for example) needs a new
value of `protocol::entity_kind`, a documented change in
[the protocol](../protocol/rtype-protocol.md), and handling in the client's renderer.

## 5. Test it

Add a case to `game/tests/test_game.cpp`: build a `registry`, call
`register_components`, spawn entities, run the system, check the result. Systems depend only
on components, so no window or socket is needed.

## Rules of thumb

- Components hold data only. Logic lives in systems.
- Do not create or kill entities while iterating a component array: collect first, act after
  (arrays can be reallocated).
- A reference to a component (`auto& c = array[i]`) is invalidated as soon as an entity is
  spawned. Copy the values you need.
