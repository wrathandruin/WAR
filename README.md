# WAR — Milestone 34 (Headless World Host / Dedicated Server Bootstrap)

> Current development milestone: M34 — Headless World Host / Dedicated Server Bootstrap

## Focus
Stand up the first separate host-owned runtime lane outside the client window loop.

M33 established shared simulation and fixed-step ownership.
M34 uses that same simulation runtime to prove that world ownership can boot in a separate process, publish startup diagnostics, and expose a controlled local host-presence lane ahead of the real client intent protocol in M35.

## What this milestone does
- adds `HeadlessHostMode` so `WAR.exe --headless-host` can boot a separate runtime process without creating the client window
- adds `HeadlessHostPresence` so the host writes a heartbeat/status file under `Runtime/Host` and the client can diagnose that host bootstrap lane
- keeps the client locally authoritative for gameplay truth for now, but makes the bootstrap boundary explicit so M35 can move action validation into the host
- adds host launch and smoke-test scripts for local authority bootstrap review
- fixes the Visual Studio project file and build definitions so the new shared/host files are included without broken path escaping
- updates runtime boundary rules so host bootstrap artifacts live under `Runtime/Host`

## Headless host bootstrap after M34
The repo should now read more clearly as:

- client: input, camera, rendering, diagnostics, local simulation presentation
- shared simulation runtime: reusable gameplay state/update ownership
- headless host bootstrap: separate process that can own and advertise a world runtime
- future protocol work: next milestone destination for client intents and authoritative validation

## Why this matters
M34 is where WAR stops only talking about authority and starts proving that authority can live outside the rendering process.

It is intentionally still a bootstrap milestone, not a full protocol milestone.
That discipline matters:

- M34 proves separate host boot and host diagnostics
- M35 moves movement and interaction truth into the host through a real intent lane
- M36 adds replication, latency, and divergence visibility

## Host launch commands
- `WAR.exe --headless-host`
- `WAR.exe --headless-host --host-tick-ms=50`
- `WAR.exe --headless-host --host-run-seconds=5`

## Demo controls
- `LMB`: move / set movement target
- `RMB`: interact
- `Shift + RMB`: inspect
- `MMB drag`: pan camera
- `Mouse wheel`: zoom
- `O`: toggle region boundary overlay
- `H`: toggle authored hotspot overlay
- `7 / 8 / 9`: Default / Muted / Vivid palette modes

## Requirements
The bgfx textured path expects compiled shader binaries at:

```text
assets/shaders/dx11/vs_color.bin
assets/shaders/dx11/fs_color.bin
assets/shaders/dx11/vs_texture.bin
assets/shaders/dx11/fs_texture.bin
```

And the shared sprite atlas at:

```text
assets/textures/world_atlas.bmp
```

## Next Milestone
### M35 — Client Intent Protocol / Authoritative Movement And Interaction
- move movement and interaction requests into the host lane
- validate and resolve client intents on the host instead of inside the client shell
- make authority visible in actual gameplay behavior rather than only startup/process diagnostics
