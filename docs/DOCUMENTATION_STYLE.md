# Documentation style — What, Why, How

Use this pattern for **new or updated** docs in this repo so readers always get context, not only commands.

---

## What (mandatory)

- **What** is this doc, section, or change about? Name the artifact (file, directory, build step, product).
- One short paragraph or bullet list is enough.

---

## Why (mandatory)

- **Why** does it exist or why do we do it this way?
- Link to constraints: Soong, `repo`, bind mounts, multi-target sharing, security, or maintenance.
- If there is an alternative we rejected, say **why** briefly (e.g. symlinks vs bind mounts).

---

## How (mandatory for procedures)

- **How** do you apply it? Commands, file paths, order of steps, verification (`mount | grep`, `findmnt`, `lunch`, etc.).
- Prefer copy-pasteable blocks and “verify” steps so people know it worked.

---

## When to use full W/W/H vs light touch

| Doc type | What | Why | How |
|----------|------|-----|-----|
| Architecture / design | Yes | Yes | Short “how to use” or pointer to SETUP |
| Step-by-step (SETUP, flashing) | Yes | Yes | Yes, detailed |
| Small README (e.g. `projects/README.md`) | Yes | Yes | Yes, table + conventions |
| One-line fix in code | N/A | Optional in commit message | N/A |

---

## Section headings (optional but consistent)

For longer docs, you can use explicit subsections:

```markdown
### What
...

### Why
...

### How
...
```

Not every paragraph needs a heading—use them when a section mixes concepts.

---

## Changelog discipline

When you change behavior (build, mounts, products), update:

1. **What** changed (facts).
2. **Why** (reason or bug fixed).
3. **How** users must adjust (migration, new commands).

---

**Author:** Francis Lobo · **Project:** lobo-aosp-platform
