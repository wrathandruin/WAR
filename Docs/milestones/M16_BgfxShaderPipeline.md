# Milestone 16 — bgfx Shader Pipeline

## Goal
Introduce a repo-owned shader pipeline for the bgfx render path and make runtime failure visible.

## Delivered
- shader source files in `assets/shaders/src`
- runtime shader loading in `BgfxWorldRenderer`
- debug-text fallback when shader binaries are missing

## Technical Notes
- shader binaries are expected under `assets/shaders/dx11`
- this milestone keeps setup explicit rather than hiding missing assets behind a black screen
