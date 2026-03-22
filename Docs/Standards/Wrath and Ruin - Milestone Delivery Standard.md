# Wrath and Ruin - Milestone Delivery Standard

## Purpose

This document defines the required delivery contract for all future Wrath and Ruin milestones.

The goal is simple:

- milestones must be professional
- milestones must be self-contained
- milestones must be reviewable
- milestones must be buildable
- milestones must be demoable
- milestones must not depend on the reader reconstructing missing code from snippets or patch hunks

This is the operating standard for the next lead developer.

## Non-Negotiable Rules

1. Deliver complete file contents, not patches.
2. Never deliver partial snippets for changed files.
3. Never use ellipses to skip code, XML, JSON, batch files, shader files, or docs.
4. If a file changes, the delivery must include the entire updated file.
5. If a build file or project file changes, the entire build file or project file must be delivered.
6. If a milestone changes scope, architecture, or user-facing behavior, the milestone document and relevant roadmap docs must be updated in the same delivery.
7. Every milestone must leave the project in a state that is buildable, runnable, and diagnosable.
8. Every milestone must state what should be visibly or operationally proven after rebuild and run.

## Package Contract

Every milestone package should use this naming pattern:

```text
WAR_M##_code_package.zip
```

The milestone document inside the package should use this path pattern:

```text
Milestones/M##_milestone_name.md
```

The package should contain:

- complete updated source files
- complete updated build and project files
- complete updated docs
- the milestone document
- any new supporting assets or generated files that are intentionally versioned

The package should not require the receiver to reconstruct code manually.

## Required Milestone Writeup Format

Every milestone handoff must use this exact section order:

```text
Title

M## - Milestone Name

Description

[clear professional description]

Download

WAR_M##_code_package.zip

The milestone document is included inside the package at:

Milestones/M##_milestone_name.md

Included

[full list of updated and new files]

What should be seen visually

[visual proof, or operational proof if the milestone is mostly backend]

Why this is important

[strategic importance]

What should be coming up in the next milestone

M## - Next Milestone Name
```

## Clarification For Backend And Ops Milestones

The heading `What should be seen visually` must still be present even when the milestone is mostly backend, deployment, persistence, or tooling work.

In those cases, that section should describe the visible proof of success such as:

- a successful hosted boot flow
- a working diagnostic overlay
- a passing smoke-test sequence
- a visible admin tool state
- a stable reconnect or persistence behavior
- a packaged build that installs and launches correctly

## File Delivery Standard

For all code and content deliveries:

- provide complete source files
- provide complete headers
- provide complete project files
- provide complete CMake files
- provide complete batch or shell scripts
- provide complete milestone docs
- provide complete README updates

Do not deliver:

- patch hunks
- diff-only answers
- "change these lines" instructions
- abbreviated snippets
- pseudocode in place of real files
- omitted middle sections

## Verification Standard

Every milestone should report:

- what was built
- what was run
- what was visually or operationally confirmed
- what remains unverified
- what the next milestone depends on

If something could not be verified locally, that limitation must be stated clearly.

## Production Standard

Milestones are not just code drops.

They are production deliverables and should demonstrate:

- scope discipline
- build integrity
- runtime integrity
- documentation integrity
- milestone-to-milestone continuity
- clear next-step ownership

## Final Rule

If a delivery is not complete enough for another developer to apply it safely without reconstructing missing context, it is not ready to ship as a milestone.
