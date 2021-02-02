# wtf

This is the engine (and level editor used to create (Porcelanic War)[https://gabrielcnoble.itch.io/porcelanic-war]).

Currently not working due to incomplete refactors.

This is a fully functional, kind of general purpose, mostly data oriented, bsp + pvs based, game engine.

It features:
- clustered forward rendering
- shadow mapping
- normal mapping
- efficient front to back rendering (using the world bsp)
- efficient occlusion culling (using a precomputed pvs)
- rigid body physics (powered by Bullet)
- a flexible ECS
- scripting (powered by AngelScript)
- other normal stuff expected from a game engine
    - input
    - audio
    - asset loading

- a level editor
    - brushes, lights, entities, waypoints
    - csg for brushes (additive, subtractive)
    - world space tex coords, for easier texturing of walls/floors/etc

- an entity editor
    - quite crude, but does the job


The latest changes, that weren't completely realized, included raytraced shadows and a refactor of the whole brush system, which was particularly clunky.