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
