## Title

M34 - Headless World Host / Dedicated Server Bootstrap

## Description

Stand up the first host-owned runtime lane outside the client process.

This milestone proves that world ownership can boot separately from rendering, publish diagnosable startup/heartbeat state, and support a controlled local host bootstrap workflow.

It intentionally stops short of claiming full authoritative gameplay.
That belongs to M35.

## Download

Planned package name:

`WAR_M34_code_package.zip`

The milestone document should live at:

`Milestones/M34_Headless_World_Host_Dedicated_Server_Bootstrap.md`

## Included

Planned focus areas:

- headless world-host executable mode
- basic boot flow and heartbeat publication
- local host bootstrap launch scripts
- client-visible host presence diagnostics
- corrected build/project definitions for new shared/host files

## What should be seen visually

- a separate `WAR.exe --headless-host` process should boot without opening the client window
- the client overlay and bgfx status text should report whether a host heartbeat is present or stale
- failures should surface as diagnosable runtime issues instead of silent process ambiguity

## Why this is important

M34 is the milestone where the project stops being architecture theatre and begins proving authority in runtime form.

## What should be coming up in the next milestone

M35 - Client Intent Protocol / Authoritative Movement And Interaction
