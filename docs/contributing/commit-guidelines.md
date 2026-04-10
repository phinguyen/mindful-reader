# Commit Guidelines

## Purpose

This document defines a consistent way to write commit messages so they are easy to read, easy to review, and useful for tracking changes and generating releases.

This is a **general guideline** and is not tied to any specific language, framework, or project structure.

---

## Core Principles

1. **One commit should have one primary purpose**.
2. **Commit messages should describe the impact of the change**, not just the technical action.
3. **Be clear, concise, and consistent**.
4. **Do not use `chore` as a fallback when unsure**.
5. **If a commit contains multiple concerns, split it if possible**.

---

## Standard Format

```text
<type>(optional-scope): <short description>
```

Examples:

```text
feat(auth): add token refresh flow
fix(api): handle empty response safely
docs: update setup instructions
refactor: simplify validation logic
```

---

## Commit Message Components

### 1. `type`

The primary category of the change.

### 2. `scope` (optional)

Indicates the affected area (module, domain, or system part).

Examples:

* `auth`
* `api`
* `ui`
* `docs`
* `config`
* `build`

### 3. `short description`

A concise summary of the change.

Guidelines:

* use a single language consistently across the repository
* keep it short and direct
* start with a verb
* avoid capitalizing the first letter if following conventional commits style
* do not end with a period

Good examples:

```text
feat: add export action for reports
fix(ui): prevent dialog from closing on submit
refactor(api): extract shared request parser
```

Bad examples:

```text
update code
fix bug
misc changes
some improvements
```

---

## Recommended `type` Values

### `feat`

Use for **new features** or new capabilities.

```text
feat: add search by keyword
feat(user): support avatar upload
```

### `fix`

Use for **bug fixes** (from incorrect → correct behavior).

```text
fix: handle null data from API
fix(auth): prevent redirect loop after logout
```

### `refactor`

Use for **code restructuring** that does **not change expected behavior**.

```text
refactor: split validation logic into service layer
refactor(api): remove duplicated mapper code
```

### `perf`

Use for **performance improvements**.

```text
perf: reduce repeated database queries
perf(ui): lazy load large media assets
```

### `docs`

Use for **documentation changes**.

```text
docs: add local development guide
docs(api): clarify authentication flow
```

### `test`

Use for adding or updating **tests**.

```text
test: add unit tests for validator
test(api): cover pagination edge cases
```

### `build`

Use for changes to the **build system**, bundling, or packaging.

```text
build: update bundler configuration
build: optimize container image
```

### `ci`

Use for **CI/CD pipeline** changes.

```text
ci: add pull request workflow
ci: run tests before deploy
```

### `style`

Use for **formatting or stylistic changes** (no logic changes).

```text
style: format files with prettier
style: normalize import ordering
```

### `chore`

Use for **maintenance or housekeeping** tasks that do not affect system behavior.

```text
chore: update dependencies
chore: add editor configuration
chore(repo): clean unused scripts
```

### `revert`

Use to revert a previous commit.

```text
revert: remove unstable caching behavior
```

---

## When NOT to Use `chore`

Do not use `chore` when:

* user-facing behavior changes
* fixing a bug
* adding or modifying APIs
* improving performance
* refactoring core logic

Incorrect:

```text
chore: fix login issue
chore: update article layout
chore: change API response
```

Correct:

```text
fix(auth): handle login timeout
feat(ui): update article layout
feat(api): change response format for reports
```

---

## How to Choose the Correct `type`

Ask in order:

1. **Does this add new capability?** → `feat`
2. **Does this fix incorrect behavior?** → `fix`
3. **Does this improve performance?** → `perf`
4. **Does it change structure without changing behavior?** → `refactor`
5. **Is it documentation, tests, build, CI, style, or maintenance?** → use `docs`, `test`, `build`, `ci`, `style`, `chore`

---

## Using `scope`

`scope` is optional but recommended when it improves clarity.

Use when:

* the repository has multiple modules or domains
* the change is localized
* commit history should be readable by area

Avoid when:

* it does not add clarity

Examples:

```text
fix(auth): handle invalid token
feat(api): add filtering support
refactor(validation): simplify rule builder
```

---

## Commit Size Guidelines

A good commit should:

* be small enough to review quickly
* be meaningful on its own
* be safely revertible

Signs a commit is too large:

* mixes feature + fix + refactor
* touches UI, API, and config together
* uses vague descriptions like “misc” or “cleanup”

Split such commits when possible.

---

## Language Consistency

Use a single language consistently across the project:

* all English (recommended)
* or all Vietnamese

Avoid mixing languages across commits.

---

## Good Commit Examples

```text
feat: add export to CSV
feat(auth): support refresh token flow
fix(api): handle empty filter values
fix(ui): prevent modal close on submit
refactor: extract shared date parser
perf(search): reduce duplicate requests
docs: add deployment notes
test(validation): cover invalid payload cases
build: update production build config
ci: add workflow for release
chore: update lint rules
revert: remove unstable sorting logic
```

---

## Poor Commit Examples

```text
update
fix stuff
misc
code cleanup
final changes
test commit
abc
```

Why they are poor:

* unclear type
* unclear impact
* not useful for history or review

---

## Breaking Changes

If a commit introduces a **backward-incompatible change**, mark it clearly.

Option 1: use `!`

```text
feat!: change public API response format
refactor(core)!: replace legacy configuration model
```

Option 2: describe in the commit body.

---

## Commit Body (Optional)

Add a body when needed to explain:

* rationale
* trade-offs
* migration notes
* reviewer context

Example:

```text
fix(auth): handle expired refresh token

Prevent redirect loop when refresh token is invalid.
Also clears local session state before redirecting to login.
```

---

## Minimal Recommended Set

For most projects, the following types are sufficient:

* `feat`
* `fix`
* `refactor`
* `perf`
* `docs`
* `test`
* `build`
* `ci`
* `style`
* `chore`
* `revert`

---

## Pre-Commit Checklist

Before committing, verify:

* Does this commit have **one clear purpose**?
* Does the `type` reflect the **primary impact**?
* Is the description clear and concise?
* Are you avoiding misuse of `chore`?
* Can this commit be reviewed or reverted independently?

---

## Summary

A clean commit history enables:

* faster reviews
* easier debugging
* safer rollbacks
* clearer changelogs
* better collaboration

Focus on **clarity, correctness, and consistency** rather than just syntax.
