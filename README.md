# bounc

A simple ball physics simulation built with Raylib in C. Features interactive red balls that bounce around the screen with real-time controls.

## Controls

- **Left Click**: Add a new ball at cursor position
- **SPACE**: Pause/Resume simulation
- **UP/DOWN Arrows**: Increase/Decrease ball speed
- **M**: Toggle between Collision and Absorption modes
- **ESC** or **Close Window**: Exit application

## Modes

- **Collision**: Balls bounce off each other with realistic physics
- **Absorption**: Colliding balls merge (one ball is removed)

## Prerequisites

- C compiler (GCC, Clang)
- [Raylib](https://www.raylib.com/)

## Compilation

```console
make run
```

## Demo

![bounc](./bounc.gif)
