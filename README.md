# Jump Buffer — Geode / Geometry Dash 2.2081

A predictive jump buffer built from scratch around the 240 Hz physics tick.

## What it does

If you click at a point where an immediate jump would kill you, the mod predicts
the result at later physics ticks and uses the **earliest safe tick** within the
configured window.

Example with `Buffer Window = 4`:

- Tick 0: jump -> DEAD
- Tick 1: jump -> DEAD
- Tick 2: jump -> SAFE
- The real jump is therefore applied at tick 2.

It does **not** blindly delay every click by the configured number of ticks.

## Settings

- **Enabled** — master switch.
- **Buffer Window** — maximum number of 240 Hz physics ticks to move the input.
- **Prediction Lookahead** — how many ticks after a candidate input are simulated.
- **Practice Mode Only** — restricts the feature to practice mode.
- **Hold Modes** — optionally enable prediction for hold-based modes.

## Build

This is a Geode project targeting Geometry Dash 2.2081.

Install the Geode CLI and SDK, then:

```text
geode sdk install
geode sdk install-binaries
geode build
```

The resulting `.geode` package will be placed in the build directory.

The GitHub Actions workflow in `.github/workflows/build.yml` can also build the
Windows mod automatically using the official Geode build action.

## Important

The code intentionally hooks `GJBaseGameLayer::handleButton` and
`processCommands`, not `queueButton`. In the current 2.2081 bindings,
`queueButton` is inline and therefore cannot be hooked with the normal
`$modify` mechanism.

The predictor uses checkpoints and simulation. Test it in Practice Mode first.
