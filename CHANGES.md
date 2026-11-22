# CHANGES — enhancements over GrymRpr/b0rk3d-V2 (Terminus V2.2)

Scope: single-player trainer features only. No changes were made to the
network, spoofing, kick/toxic, or anticheat-hook code paths.

## 1. Trainer Essentials — one-toggle survival preset

- New file: `src/game/features/self/TrainerEssentials.cpp`
- Menu: **Self → Main → Globals** (first item in the group)
- Command name: `traineressentials`
- What it does: enabling it turns on, in one click:
  - `godmode` (God Mode)
  - `keepbarsfilled` (Health / Stamina / Deadeye bars filled)
  - `keepcoresfilled` (cores kept filled)
  - `infiniteammo` and `infiniteclip`
- Disabling it turns the same five back off.
- Implementation: a `BoolCommand` whose `OnEnable()`/`OnDisable()` resolve
  the member commands through the command registry
  (`Commands::GetCommand<BoolCommand>`) and call `SetState()`, which runs
  each member's enable/disable logic through the fiber pool exactly like a
  manual click. All lookups are null-checked — if an upstream rename ever
  removes a member command, it is skipped instead of crashing.

## 2. Keep Max Bonding — horse bonding, including horse swaps

- New file: `src/game/features/mount/KeepMaxBonding.cpp`
- Menu: **Self → Horse → Globals** (after "Keep Horse Cores Filled")
- Command name: `keepmaxbonding`
- What it does: re-asserts bonding level 4 (the max) on your current mount
  every tick while enabled. The header has no getter native for the current
  bonding level, so (same approach as Godmode's invincibility) it writes
  level 4 unconditionally; writes while already maxed are harmless.
  Because it re-asserts per tick, a newly mounted horse is bonded
  automatically — no re-toggling needed.
- Implementation: a `LoopedCommand` following the existing
  `KeepHorseBarsFilled` pattern (`Self::GetMount()`, null/dead guards),
  calling `PED::_SET_MOUNT_BONDING_LEVEL(mount.GetHandle(), 4)` — a native
  that already exists in this repo's `Natives.hpp` (hash 3619).

## 3. Menu wiring

- `src/game/frontend/submenus/Self.cpp`: two `BoolCommandItem` additions —
  `traineressentials` in the Globals group (Self → Main) and
  `keepmaxbonding` in the Horse group (Self → Horse). Both follow the
  existing hash-based registration idiom (`"name"_J`).

## 4. Crash protections against hostile mod-menu events

Defensive only: everything here **blocks incoming** events so other
players' mod menus cannot crash or manipulate your client. Nothing in
this section sends anything to anyone.

### 4a. Per-player script-event flood gate

- `src/game/backend/PlayerData.hpp`: new `RateLimiter
  m_ScriptEventFloodLimit{10s, 120}` alongside the existing vehicle/
  ticker limiters.
- `src/game/hooks/Protections/HandleScriptedGameEvent.cpp`: at the top of
  the handler, when `blockscripteventflood` is on (default), a player
  sending more than 120 scripted game events in 10 seconds is cut off
  (logged once, not notification-spammed — same idiom as the ticker-spam
  limiter). Legit remote scripts never approach that rate from a single
  sender; crash exploits that spam malformed events do.
- Note: the upstream `src/game/hooks/Protections/AntiCrash.cpp` rate
  limiter is **unreferenced** — nothing in the tree calls it — and is now
  functionally superseded by this gate. It was left untouched.

### 4b. Six newly covered script events

The existing dispatcher handled 7 of the 14 `ScriptEvent` members defined
in `src/game/rdr/Enums.hpp`. The following now have block toggles (event
IDs taken from that enum — nothing invented):

| Toggle (Settings → Protections → Script Events) | Default | Event |
|---|---|---|
| Block Fetch Manager | on | `SCRIPT_EVENT_FETCH_MANAGER` (178) |
| Block ACE Launcher | on | `SCRIPT_EVENT_ACE_LAUNCHER` (5) |
| Block Stable Events (All) | on | `SCRIPT_EVENT_NET_STABLE` (46) — the mount variant (47) was already covered |
| Block VS Mission Intro | off | `SCRIPT_EVENT_NET_VS_MISSION_INTRO` (49) |
| Block Net HUD | off | `SCRIPT_EVENT_NET_HUD` (186) |
| Block Player Camp | off | `SCRIPT_EVENT_PLAYER_CAMP` (201) |

Defaults: the known-abuse families block immediately; the gameplay-visible
ones (VS intro, HUD, camp) start off so legitimate missions keep working —
flip them on if a lobby is attacking you. Each block shows the usual
"Protections" notification naming the sender.

`SCRIPT_EVENT_AMBIENT_CONTENT_EVALUATOR` (4) remains unhandled — no
evidence of abuse, and leaving it alone avoids breaking ambient event
traffic.

### 4c. Menu wiring

- `src/game/frontend/submenus/Settings.cpp`: seven new toggles in
  **Settings → Protections → Script Events** (after "Block Stable
  Events").

## Files added / modified

| File | Change |
|---|---|
| `src/game/features/self/TrainerEssentials.cpp` | added |
| `src/game/features/mount/KeepMaxBonding.cpp` | added |
| `src/game/frontend/submenus/Self.cpp` | 2 menu items added |
| `src/game/hooks/Protections/HandleScriptedGameEvent.cpp` | flood gate, 6 event cases, 7 toggles |
| `src/game/backend/PlayerData.hpp` | `m_ScriptEventFloodLimit` added |
| `src/game/frontend/submenus/Settings.cpp` | 7 protection toggles wired |

## Verification status

The additions were written against the repo's own idioms and use only
natives/wrappers already present in `src/game/rdr/Natives.hpp` and event
IDs already present in `src/game/rdr/Enums.hpp`, but they have **not** been
compiled here (this workspace is Linux; the DLL needs the MSVC toolchain —
see BUILD.md). Build once on Windows, then check in an online-free
setting: the new toggles appear under Settings → Protections → Script
Events, the trainer preset and bonding toggles behave as described, and
the log shows blocked-event lines when they fire.
