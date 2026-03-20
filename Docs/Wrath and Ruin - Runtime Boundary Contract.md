# Wrath and Ruin - Runtime Boundary Contract

## Purpose

This document defines the canonical content boundary and runtime-only mutable-data boundary for the `WAR` repo.

It exists to prevent source-versus-runtime drift before persistence, packaging, deployment, and authority work become more expensive.

This contract takes direct inspiration from the legacy Wrath-and-Ruin project's runtime hygiene and mutable-data separation.

---

## 1. Canonical Source-Controlled Truth

These paths are treated as versioned source of truth unless explicitly documented otherwise:

- `src/`
- `shared/`
- `desktop/`
- `server/`
- `contracts/`
- `assets/shaders/`
- `Docs/`
- `Milestones/`
- `scripts/`
- `CMakeLists.txt`
- Visual Studio and solution definitions that intentionally describe the workspace
- `.gitignore`
- other intentionally versioned build and packaging definitions

### Canonical content rules

1. Reviewable game code belongs in owned source trees, not mutable runtime folders.
2. Versioned render-pipeline assets belong in `assets/shaders/`.
3. Local development textures and images may exist under `assets/textures/`, but they are machine-local and ignored by Git unless policy changes explicitly.
4. Production policy, architecture, and milestone handoff docs belong in `Docs/` or `Milestones/`.
5. Source-controlled defaults and templates may be versioned, but live mutable data must not replace them.
6. Generated outputs are never promoted into canonical source without an explicit pipeline decision.

---

## 2. Runtime-Only Mutable State

The repo runtime root is:

```text
Runtime/
```

Within that root, the current approved mutable subdirectories are:

- `Runtime/Config/`
- `Runtime/Logs/`
- `Runtime/Saves/`
- `Runtime/CrashDumps/`
- `Runtime/Host/`

### Runtime rules

1. Logs, saves, crash dumps, machine-local mutable config, and host status files belong under `Runtime/`.
2. Runtime data must not be written into versioned source, doc, or canonical asset paths.
3. Local development should preserve the same conceptual separation that packaged builds will use later.
4. Runtime directories may contain placeholder `.gitkeep` files only so the layout remains visible in source control.
5. Live mutable files inside `Runtime/` are ignored by Git.

### Persistence rules after M37

1. The authoritative save slot currently lives at:

```text
Runtime/Saves/authoritative_world_primary.txt
```

2. This file is runtime-only mutable state and must never be committed.
3. Save publication must be atomic so partial writes do not become accepted state.
4. Save format versioning is required from the first persistence milestone onward.
5. Migration handling must happen in shared/server ownership, not in desktop-only presentation code.

---

## 3. Generated And Non-Versioned Outputs

These areas are not canonical content and should remain ignored:

- `bin/`
- `obj/`
- `build/`
- `out/`
- Visual Studio and CMake transient files
- packaging outputs
- mutable files under `Runtime/`

### Generated-output rules

1. Build artifacts are disposable and reproducible.
2. Packaging outputs must be created from versioned source and scripts, not hand-edited after generation.
3. Runtime files produced during demos, tests, authority runs, or acceptance runs are not reviewable source assets.
4. If a generated file must become canonical later, that decision must be documented and the pipeline updated intentionally.

---

## 4. Local Development Versus Packaged Layout

### Local source-tree execution

When the runtime can resolve a repo root containing canonical source markers, it should treat that as source-tree mode.

In source-tree mode:

- canonical content resolves from the repo root
- mutable runtime state resolves from `Runtime/`

### Packaged execution

When the repo root cannot be resolved, the build should assume a packaged layout.

In packaged mode:

- canonical assets resolve relative to the executable package
- mutable runtime state resolves under an executable-local `runtime/` root

This preserves the separation even when the game is no longer being run from the repo checkout.

---

## 5. Operational Expectations

Startup diagnostics should make the following visible:

- whether execution is source-tree or packaged
- what asset root was resolved
- what runtime root is active
- whether runtime directories were created successfully
- whether required asset directories are missing
- whether the authoritative persistence slot is present
- whether persistence last saved or loaded successfully
- what schema version the runtime believes it owns

This is not polish.
It is production hygiene that makes packaging, persistence, and hosted runtime work safer.

---

## 6. Repository Hygiene Rules

Contributors should follow these rules unless a later documented milestone changes the contract:

1. Do not add live saves or logs to versioned directories.
2. Do not use `assets/` as a miscellaneous runtime dump.
3. Do not commit local-only textures or images from `assets/textures/`.
4. Do not commit machine-local mutable config unless it is an explicit versioned template.
5. Do not place generated package outputs in reviewable source paths.
6. If a new runtime class of data appears, add it to this contract and `.gitignore` in the same delivery.

---

## 7. Why This Matters Before Alpha

Alpha requires:

- persistence discipline
- packaging discipline
- diagnostics discipline
- eventual server/client runtime separation

Those become fragile and expensive if the repo does not first establish what is canonical and what is mutable.

The M31 boundary contract created the base.
M37 extends it by making versioned authoritative save files a first-class runtime concern without weakening the public-safe repo boundary.
