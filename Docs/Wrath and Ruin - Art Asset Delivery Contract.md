# Wrath and Ruin - Art Asset Delivery Contract

## Purpose

This document is the hard production contract for art deliverables.

It exists so the lead artist is not guessing:

- how many files each asset needs
- what each file is for
- which asset families require icons
- which files are source-of-truth versus runtime compatibility exports

If the art direction documents describe the look and feel, this document defines the exact file outputs.

## Relationship To Other Documents

Use these documents together:

1. `Docs/Wrath and Ruin - Lead Artist Onboarding Package.md`
2. `Docs/Wrath and Ruin - Art Asset Delivery Contract.md`
3. `Docs/Wrath and Ruin - Lead Artist Milestone Runway.md`
4. `Docs/Wrath and Ruin - Lore and Visual Theme Guide.md`

Direction comes from the onboarding package and lore guide.
File-output requirements come from this contract.

## Non-Negotiable Rule

No gameplay-facing art asset is considered production-ready unless it ships with the exact required file stack for its asset family.

Do not deliver:

- one flat PNG and call it complete
- one BMP and call it production-ready
- a style sheet without asset-level outputs
- icons only when someone remembers to ask

## Output Families

There are three output families:

1. world render pack
2. inventory icon pack
3. UI surface pack

Every asset belongs to one or more of these families.

## 1. World Render Pack

### Rule

Every gameplay-visible world asset must ship with exactly four PNG files.

This is mandatory for:

- floors
- walls
- doors and hatches
- props
- containers
- terminals and consoles
- equipment visible in the world
- dropped items
- characters
- drones
- hazards
- shuttle and orbital markers
- mission-critical world objects

If the asset is animated, this four-file contract applies to every frame or authored atlas frame group.

### Required files

1. `*_base.png`
2. `*_mask.png`
3. `*_fx.png`
4. `*_ui.png`

### 1.1 `*_base.png`

Format:

- PNG
- RGBA

Use:

- RGB = final authored base appearance
- A = cutout or opacity

Why it exists:

- this is the clean visible art layer
- it is the fallback if the runtime only supports a flat presentation
- it is the baseline for reviews, previews, contact sheets, and integration flattening

### 1.2 `*_mask.png`

Format:

- PNG
- RGBA

Channel use:

- R = primary recolor mask
- G = emissive or lit-surface mask
- B = lighting-response mask
- A = secondary recolor or overlay gate

Why it exists:

- supports faction recolor without repainting the asset
- supports lit panels, terminals, screens, and active machinery
- supports future lighting and material-response work
- avoids destructive repaint passes for every variant

### 1.3 `*_fx.png`

Format:

- PNG
- RGBA

Channel use:

- R = damage or wear blend
- G = dirt, dust, grime, or blood blend
- B = active, powered, hazard, or pulse blend
- A = state transition or alternate overlay gate

Why it exists:

- supports damaged and repaired states
- supports power-on versus power-off presentation
- supports hazard-coded or objective-active variants
- supports future state blending without repainting the base sheet

### 1.4 `*_ui.png`

Format:

- PNG
- RGBA

Channel use:

- R = selection outline source
- G = hover or interaction hotspot source
- B = silhouette or readability recovery source
- A = minimap, marker, or focus ping source

Why it exists:

- supports tactical readability in dense scenes
- supports hover, inspect, selection, and objective highlighting
- supports silhouette recovery when scenes are dark or visually noisy
- keeps readability support separate from the painted base art

### Blank channel rule

If a channel is not used yet, it must still be delivered as a black or zeroed channel.

Do not omit files because a channel is currently unused.

This project uses a fixed contract, not a “deliver only what you feel is needed” model.

## 2. Inventory Icon Pack

### Rule

Every inventory-bearing item must ship with a matching icon pack.

This is mandatory for:

- weapons
- tools
- suits and armor pieces
- consumables
- medical items
- mission key items
- loot items
- crafting or salvage items
- batteries, cells, chips, or similar utility items

If an item can appear in inventory, it must have an icon.
This is not optional.

### Locked icon size

Current mandatory production icon target:

- native export: `64 x 64`
- source size: `128 x 128`

Do not invent alternate icon sizes unless engineering locks a new target.

### Required files

1. `*_icon_base.png`
2. `*_icon_mask.png`
3. `*_icon_fx.png`
4. `*_icon_ui.png`

### 2.1 `*_icon_base.png`

Use:

- the readable inventory icon itself
- must work on dark and light inventory backgrounds

### 2.2 `*_icon_mask.png`

Use:

- rarity or quality tinting
- faction tinting
- emissive or powered item support
- lighting-response control if needed

### 2.3 `*_icon_fx.png`

Use:

- broken, damaged, depleted, charged, contaminated, mission-active, or special-state overlays

### 2.4 `*_icon_ui.png`

Use:

- hover treatment
- selected-slot treatment
- quickbar treatment
- stack emphasis
- important-item readability support

### Inventory world-rule

If an item exists both in the world and in inventory, both packs are required:

- world render pack
- inventory icon pack

Do not deliver one and assume engineering can derive the other.

## 3. UI Surface Pack

### Rule

UI-facing assets do not use the same contract as world renderables.

UI assets require exactly two PNG files by default:

1. `*_base.png`
2. `*_ui.png`

Optional extra files may be requested only when the surface needs material or state logic beyond that.

### This family includes

- room description panels
- prompt and vitals surfaces
- event-log framing
- journal and mission framing
- command-bar surfaces
- communication and portrait frames
- menu and onboarding panels
- cursor states
- objective markers

### UI rule

UI assets are not exempt from specification.
They still need:

- exact size
- exact state list
- exact role
- exact pivot or anchor behavior if applicable

## 4. Asset Class Matrix

### Floors and terrain

- required family: world render pack
- icon pack: no
- UI pack: no

### Walls, doors, hatches, barriers

- required family: world render pack
- icon pack: no
- UI pack: no

### Props and interactables

- required family: world render pack
- icon pack: only if inventory-bearing
- UI pack: no

### Containers

- required family: world render pack
- icon pack: only if the container becomes an inventory item
- UI pack: no

### Weapons, tools, consumables, suits, loot

- required family: inventory icon pack always
- required family: world render pack if dropped, equipped in-world, or placed in the scene
- UI pack: no

### Characters, drones, hostiles, NPCs

- required family: world render pack
- icon pack: no
- UI pack: no

### Hazards and mission-state world markers

- required family: world render pack
- icon pack: no unless also inventory-bearing
- UI pack: no

### UI panels and HUD surfaces

- required family: UI surface pack
- icon pack: no
- world render pack: no

### Objective, command, and log icons

- required family: UI surface pack
- exact size must be declared
- do not assume one generic icon size for all UI contexts

## 5. Naming Standard

Use this file pattern:

`WAR_[category]_[set]_[object]_[state]_[dir]_[frame]_[outputtype].png`

Examples:

- `WAR_prop_terminal_diagnostic_active_N_01_base.png`
- `WAR_prop_terminal_diagnostic_active_N_01_mask.png`
- `WAR_prop_terminal_diagnostic_active_N_01_fx.png`
- `WAR_prop_terminal_diagnostic_active_N_01_ui.png`
- `WAR_item_medkit_standard_default_icon_base.png`
- `WAR_item_medkit_standard_default_icon_mask.png`
- `WAR_item_medkit_standard_default_icon_fx.png`
- `WAR_item_medkit_standard_default_icon_ui.png`

## 6. Delivery Manifest Requirement

Every art batch must include a manifest.

Minimum required fields per asset:

- asset id
- asset family
- milestone or pack
- native export size
- source size
- gameplay footprint
- pivot
- state variants
- direction count if applicable
- frame count if applicable
- world render pack included: yes or no
- inventory icon pack included: yes or no
- UI surface pack included: yes or no

This can be delivered as:

- `asset_manifest.md`
- or `asset_manifest.csv`

Do not submit an art batch with no manifest.

## 7. Runtime Compatibility Rule

The current engine may still ingest a BMP compatibility export for immediate use.

That does not change the source-art contract.

The authoritative production deliverables are the PNG packs defined above.
The BMP export is only the current runtime compatibility layer.

## 8. Review Gates

No asset batch is approved unless it passes:

- top-down perspective check
- native gameplay-size readability check
- crowded-scene readability check
- dark-background readability check
- light-background readability check
- text-surface compatibility check where relevant
- icon readability check at native icon size

## 9. Immediate Locked Rule For The Lead Artist

Until a later engineering milestone changes the renderer and packaging path, assume this exact policy:

- world gameplay assets = four PNG files mandatory
- inventory items = four icon PNG files mandatory
- UI surfaces = two PNG files mandatory by default
- runtime BMP export = only when immediate integration is requested

This is now the production standard for Wrath and Ruin art delivery.
