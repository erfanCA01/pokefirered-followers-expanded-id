# Follower System Port — pokefirered

Porting merrp's stateless follower system from `pokeemerald-followers-expanded-id`
to this pret/pokefirered fork. Constraints:

- **Stateless**: no new fields in `SaveBlock1`/`SaveBlock2`/`PokemonStorage`; save-struct sizes must not change.
- **Multiplayer compatible**: followers are disabled in link rooms.

Build/verify: `make firered_rev1 -j$(nproc) COMPARE=0`

---

## Stage 1 — Chunk 1: Struct + macros (size-neutral) ✅ DONE

Widened the object-event `graphicsId` from `u8` to `u16` and added the follower
macro surface, without changing any save-structure size.

### Files changed

| File | Change |
|---|---|
| `include/global.fieldmap.h` | `ObjectEventTemplate`: `graphicsId` u8→u16 + `__attribute__((packed, aligned(4)))` (keeps size `0x18`). `ObjectEvent`: added `shiny:1`/`padding:3` bits (28–31), `graphicsId` u8→u16 at `0x04` (11-bit species + 5-bit form), moved `spriteId` to the former `0x23` pad byte (size stays `0x24`). |
| `include/constants/event_objects.h` | Follower gfx macros: `OBJ_EVENT_GFX_MON_BASE`, `OBJ_EVENT_GFX_SPECIES_BITS/MASK`, `OBJ_EVENT_GFX_SPECIES`, `OW_SPECIES`, `OW_FORM`, `IS_OW_MON_OBJ`; config knobs `OW_MON_*`; `TRACKS_SLITHER/SPOT/BUG`; `SHADOW_SIZE_NONE`; `LOCALID_FOLLOWING_POKEMON` (254) + `OBJ_EVENT_ID_FOLLOWER`. |
| `include/global.h` | `INCBIN_COMP`, `IS_POW_OF_TWO`, `OW_GFX_COMPRESS TRUE`. |
| `asm/macros/map.inc` | `object_event` / `clone_event` macros emit `.2byte \gfx` and drop the old pad byte after `kind`, matching the new template layout. |

### Key technique
Both `ObjectEvent` (0x24) and `ObjectEventTemplate` (0x18) contained the exact
padding merrp repurposed, so the `graphicsId` widening is size-neutral. The
`packed, aligned(4)` attribute on `ObjectEventTemplate` is required — without it
the `u16` field forces alignment padding that grows the struct and trips
`STATIC_ASSERT(sizeof(struct SaveBlock1) <= …)`.

### Verification
- Build: `make firered_rev1 -j$(nproc) COMPARE=0` → exit 0, ROM produced.
- Save-struct sizes confirmed unchanged via linker map:
  - `gSaveBlock1` = `0x3D68`
  - `gSaveBlock2` = `0xF24`
  - `gPokemonStorage` unchanged

### Deferred to later chunks
- `SPECIES_SHINY_TAG` (in `include/constants/species.h`) + `OBJ_EVENT_GFX_SPECIES_SHINY` macro (follower graphics/palette chunk).
- merrp's dead `extern u8 UpdateSpritePaletteWithTime(u8)` was intentionally skipped.

### ⚠️ Note
`graphicsId` is now `u16` and `spriteId` relocated, so a vanilla FireRed save's
`objectEvents` bytes will be interpreted differently. merrp's "Made follower
pokemon inactive on vanilla saves" logic must be ported in the `load_save.c` chunk.

---

## Stage 2 — Chunk 2: API widening u8→u16 ✅ DONE

Widened the object-event graphicsId API so the u16 `graphicsId` from Stage 1
flows through without truncation.

### Files changed

| File | Change |
|---|---|
| `include/event_object_movement.h` | `ObjectEventSetGraphicsId` `u8`→`u16` (both decls), `GetObjectEventGraphicsInfo` `u8`→`u16`. |
| `include/event_data.h` | `VarGetObjectEventGraphicsId` return `u8`→`u16`. |
| `src/event_object_movement.c` | `ObjectEventSetGraphicsId`, `ObjectEventSetGraphicsIdByLocalIdAndMap`, `GetObjectEventGraphicsInfo` params `u8`→`u16`. |
| `src/event_data.c` | `VarGetObjectEventGraphicsId` return `u8`→`u16`. |

### Verification
- Build: `make firered_rev1 -j$(nproc) COMPARE=0` → exit 0, ROM produced.
- EWRAM/IWRAM/ROM usage unchanged (size-neutral).

### Deferred to Stage 3 (asset-heavy)
The follower **graphics info table** and sprite assets are a separate, larger
stage: `graphics/object_events/pics/pokemon/*.png` (386+ sprites) + generated
tables (`sPicTable_*`, `sOamTables_*`, `sAnimTable_Following`, palettes) +
`src/data/object_events/object_event_graphics_info_followers.h` +
`SpeciesToGraphicsInfo` / `gPokemonObjectGraphics` / `gCastformObjectGraphics`
/ `gFollowerPalettes`. `GetObjectEventGraphicsInfo` will gain its
`OBJ_EVENT_GFX_MON_BASE` branch then.

---

## Progress summary (after Stage 2)

### Completed & pushed
- **Stage 1 (Chunk 1)** `3ea16338b` — widen `graphicsId` to u16 + follower macros (size-neutral).
- **Stage 2 (Chunk 2)** `afe556856` — widen object-event graphicsId API to u16.
- Both stages build clean (`make firered_rev1 -j$(nproc) COMPARE=0` → exit 0); save sizes unchanged.

### Next — Stage 3 (asset-heavy)
Follower graphics info table + sprite assets:
1. Copy 386+ OW sprites `graphics/object_events/pics/pokemon/*.png` from merrp.
2. Add `graphics_file_rules.mk` entries; generate `sPicTable_*` / `sOamTables_*` / `sAnimTable_Following` (gbagfx).
3. Port `src/data/object_events/object_event_graphics_info_followers.h` (`gPokemonObjectGraphics`, `gCastformObjectGraphics`, `gFollowerPalettes`).
4. Add `SpeciesToGraphicsInfo` + `OBJ_EVENT_GFX_MON_BASE` branch in `GetObjectEventGraphicsInfo` + `SPECIES_SHINY_TAG` in `species.h`.

### Later stages
- Core engine: `follower_helper.h/.c`, `event_object_movement.c` follower functions, `field_player_avatar.c` spawn/despawn, `overworld.c`/`load_save.c` respawn hooks.
- Scripts + maps: `follower.inc`, script-cmd table, per-map follower events.
- Field-move/battle hooks: `field_effect_helpers.c`, `scrcmd.c`, `script_movement.c`, battle return-to-field.
- Multiplayer link-room disable (honored throughout).

---

## Stage 3 — Part 1: follower sprite assets + build rules ✅ DONE

Imported the follower OW sprite assets and their build rules from merrp.

### What changed
- Copied 385 new follower OW sprites to `graphics/object_events/pics/pokemon/`
  (`cp -n`, so FireRed's existing 29 static-encounter sprites are preserved).
- Removed 16 stray `*_old.png` static-encounter backups (not needed — FireRed keeps
  its own static sprites).
- Appended 393 new `$(OBJEVENTGFXDIR)/pokemon/*.4bpp` rules to
  `spritesheet_rules.mk` (skipping `_old` and existing-species rules).

### Note (dormant assets)
The sprites are not yet referenced by any `INCBIN`/pic table, so the build
correctly reports nothing to do. They become active once the data files are added:
`object_event_graphics.h` (INCBIN `gObjectEventPic_*`), `object_event_pic_tables.h`
(`sPicTable_*`), `object_event_anims.h` (`sAnimTable_Following`),
`object_event_graphics_info_followers.h` (`gPokemonObjectGraphics` / `gCastformObjectGraphics`
/ `gFollowerPalettes`), plus the code wiring (`SpeciesToGraphicsInfo`,
`GetObjectEventGraphicsInfo` `OBJ_EVENT_GFX_MON_BASE` branch, `SPECIES_SHINY_TAG`,
local `OBJ_EVENT_PAL_TAG_DYNAMIC/SUBSTITUTE/NONE` palette tags).

---

## Stage 3 — Part 2a: palette tags + SPECIES_SHINY_TAG + followers table (dormant) ✅ DONE

- Added `SPECIES_SHINY_TAG 500` to `include/constants/species.h`.
- Added `OBJ_EVENT_PAL_TAG_DYNAMIC/SUBSTITUTE/WHITE` to the local palette-tag block in
  `src/event_object_movement.c`.
- Copied `src/data/object_events/object_event_graphics_info_followers.h` (511 lines;
  dormant until its `sPicTable_*`/`sAnimTable_Following` deps are ported).

Remaining (part 2b): follower anim commands (`sAnim_*2F`, `sAnim_Enter*`,
`sAnim_ExitPokeball*`, `sAnim_*_Asym`), `sAnimTable_Following`/`_Asym`,
`gObjectEventPic_*` INCBIN (385), `sPicTable_*` (385), `SpeciesToGraphicsInfo`,
`GetObjectEventGraphicsInfo` `OBJ_EVENT_GFX_MON_BASE` branch, `LoadDynamicFollowerPalette`.

---

## Stage 3 — Part 2b: follower graphics data + anims ✅ DONE (build green)

Ported the follower graphics data and animation commands. Build passes.

### What changed
- `object_event_anims.h`: ~29 follower anim commands (`sAnim_*2F`, `sAnim_Enter*`,
  `sAnim_ExitPokeball*`, `sAnim_*_Asym`) + `sAnimTable_Following`/`_Asym`.
- `object_event_graphics.h`: 385 `gObjectEventPic_*` INCBIN + ball/substitute/castform
  palettes (Emerald-only `emotes` + `_old`/`Old`/`RubySapphire` variants filtered out).
- `object_event_pic_tables.h`: 385 `sPicTable_*` (6-frame follower layout).
- `constants/event_object_movement.h`: `ANIM_EXIT_POKEBALL_FAST_*`.
- `global.h`: `OW_GFX_COMPRESS FALSE` (compression deferred; `INCBIN_COMP` needs a
  preprocessor change that is not yet ported).
- `spritesheet_rules.mk`: ball sprite pattern rule. Copied 28 ball sprites + species
  palettes (ho_oh, lugia, groudon, kyogre, …).
- 36 static-encounter sprites replaced by merrp's follower sprites (unified).

### ⚠️ Known follow-ups (next chunk)
- `SpeciesToGraphicsInfo` + `GetObjectEventGraphicsInfo` `OBJ_EVENT_GFX_MON_BASE`
  branch + `#include` of `object_event_graphics_info_followers.h` (follower table
  is still dormant).
- `LoadDynamicFollowerPalette` (dynamic species palettes).
- 36 static `gObjectEventGraphicsInfo_<Species>` entries still use `sAnimTable_Standard`
  (should be `sAnimTable_Following` to match the new 6-frame sprites).
- Re-enable `OW_GFX_COMPRESS` + port `INCBIN_COMP` preprocessor change later.

---

## Stage 3 — Part 2c: follower graphics table wired (live) ✅ DONE

- Added `SpeciesToGraphicsInfo` + `OBJ_EVENT_GFX_MON_BASE` branch in
  `GetObjectEventGraphicsInfo`; follower `graphicsId`s (0x200+) now resolve.
- Included `object_event_graphics_info_followers.h` (table is live/linked).
- Added `OBJ_EVENT_PAL_TAG_CASTFORM_*` + subsprite-table aliases
  (`sOamTables_*` → `gObjectEventSpriteOamTables_*`) + `gMonPalette_CircledQuestionMark`
  externs (FireRed naming differs from Emerald).
- Disabled `OW_MON_POKEBALLS` (ball feature deferred).

### ⚠️ Remaining
- 36 static `gObjectEventGraphicsInfo_<Species>` still use `sAnimTable_Standard`
  (should be `sAnimTable_Following`).
- `LoadDynamicFollowerPalette` (dynamic species palettes) — next chunk.
- Follower spawn/despawn engine (field_player_avatar, event_object_movement follower
  functions, follower_helper.c) — Stage 4.
