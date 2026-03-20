# Wrath and Ruin - Legacy Migration Assessment

## Purpose

This document explains how the previous `Wrath-and-Ruin` MUD project should influence the new `WAR` roguelike migration.

The legacy project is not the new game.
It is the reference source for:

- production lessons
- server-authoritative thinking
- deployment discipline
- identity and launcher flow
- persistence and content migration lessons

---

## 1. What The Legacy Project Already Proved

Based on the old repository documentation, the previous project already established real production patterns in several areas.

### 1.1 Operations

The old project had:

- local, staging, and live environment models
- deployment scripts
- healthchecks
- backup and restore flows
- host-service and hardening runbooks
- runtime drift reporting

This is highly valuable.
The new repo should not wait until late beta or launch to reintroduce these habits.

### 1.2 Runtime Boundaries

The old project documented the difference between:

- deployable source and canonical content
- mutable runtime state

That distinction is essential for the new roguelike project as soon as persistence, saves, and authored content begin to grow.

### 1.3 Identity And Entry Flow

The old project already understood that:

- account identity should be separate from the raw game transport
- website and desktop clients should share the same identity flow
- ticket or session handoff models reduce login fragmentation

Even if the new repo does not immediately recreate the full website stack, it should preserve that product thinking.

### 1.4 Release Discipline

The old desktop release runbook proved mature thinking around:

- installers
- update feeds
- packaging
- publishing
- validation before release

That is directly relevant to future market readiness for the new product.

### 1.5 Data Migration Awareness

The old project already had strong instincts around:

- content manifests as source of truth
- migration without gameplay drift
- structured persistence evolution
- clear rules about which formats are canonical

Those instincts should guide the new content pipeline from the start.

---

## 2. What The New Repo Currently Proves

The new `WAR` repo proves a better starting point for the new game’s spatial and visual identity:

- cleaner top-down presentation direction
- renderer abstraction
- bgfx-based client runtime
- more obvious path toward a roguelike-style playable surface

It is a better fit for the target product than trying to mutate the old monorepo directly.

However, it does not yet carry forward enough of the old repo’s production maturity.

---

## 3. What Must Be Inherited Immediately

These should be treated as production requirements for the new repo, not optional future polish.

### 3.1 Server Authority

The new game must be planned around:

- authoritative server ownership
- client intent rather than client truth
- deterministic or at least strongly controlled simulation ownership

### 3.2 Runtime Hygiene

The new repo must establish early:

- what is canonical and versioned
- what is runtime-only
- where saves, logs, generated data, and live mutable state belong

### 3.3 Deployment Thinking

Even before public release, the new repo should define:

- local demo flow
- internal deploy flow
- staging expectations
- rollback expectations
- crash and health visibility

### 3.4 Account And Session Direction

The product should eventually preserve the old insight that:

- the game client should not invent its own identity universe
- session and account flow should be intentional and product-quality

### 3.5 Regression Culture

The new project should inherit the old mindset of:

- smoke tests
- acceptance checks
- deploy verification
- boring operations

---

## 4. What Should Not Be Ported Blindly

These are the parts of the old project that should inform design but not be copied as-is.

- room-based navigation as the world primitive
- monolithic subsystem structure
- telnet-first UX assumptions
- legacy runtime/data mixing
- broad historical feature sprawl without slice discipline

---

## 5. Production Implications For The New Roadmap

### For Alpha

Alpha must inherit:

- server authority
- persistence discipline
- runtime boundaries
- deploy and packaging direction
- diagnostics and crash awareness

### For Beta

Beta must inherit:

- onboarding discipline
- external supportability
- broader content validation
- operational monitoring

### For Market

Market readiness must inherit:

- installer and update thinking
- release runbooks
- rollback discipline
- live operations support
- product identity flow

---

## 6. Recommended Carryover Work

The new repo should explicitly recreate or adapt the following concepts from the old project:

- runtime drift and mutable-data boundary rules
- environment model for local, staging, and hosted internal builds
- deploy and rollback runbooks
- smoke-test and acceptance gate culture
- packaging and release candidate checklist
- account/session architecture direction
- content manifest and migration rules

---

## 7. Final Assessment

The old project should be treated as a **production reference**, not as dead history.

The new repo should be treated as the **cleaner product foundation** for the roguelike migration.

The winning strategy is:

- use the new repo for the new game’s actual architecture and presentation
- use the old repo to avoid relearning expensive production lessons

That is the shortest path to a product that is not only interesting, but truly shippable.
