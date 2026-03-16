# V7 Merge Design Review -- Devil's Advocate

**Reviewer stance:** Adversarial. Every concern below is something that will waste your time if you discover it mid-implementation instead of now.

---

## 1. Font Compatibility: Cormorant Garamond on a Marshall Amp

### The core tension

Cormorant Garamond is a high-contrast display serif inspired by Claude Garamond's 16th-century cuts. It is *delicate, light, and scholarly*. Marshall amps are *heavy, industrial, and loud*. These are not friends.

### Specific problems to expect

**Hero brand text ("OneDial"):**
V5 uses `Instrument Serif` at italic, `clamp(2.5rem, 6vw, 4.5rem)`, weight 400. This font has thick slab-like serifs that read as silk-screened metal. Cormorant Garamond at the same size will look like a wine label pasted onto a guitar amp. Its hairline serifs will nearly disappear against `#0a0908` at low contrast.

**Section titles (italic, `clamp(2.5rem, 6vw, 4.5rem)`):**
V5's `font-style: italic` with Instrument Serif produces a confident brush-like swash. Cormorant Garamond italic is elegant and calligraphic -- it will feel *precious* rather than *powerful* next to grille cloth textures and gold piping.

**Small mono text will feel orphaned:**
V6 pairs Cormorant with JetBrains Mono; they share a refined sensibility. V5 pairs its display font with Space Mono (more brutalist, chunky). If you keep JetBrains Mono (the merge plan says Cormorant + JetBrains), the entire typographic palette skews elegant, which fights the amp-head visual language at every turn.

### Actionable fixes

1. **Test Cormorant Garamond at weight 600-700 for the brand text** -- its heavier weights have more presence. Weight 300 (used liberally in v6) will vanish on dark backgrounds.
2. **Increase letter-spacing for serif text on dark backgrounds.** V6 uses `-0.02em` on `hero-title` -- that works on cream (`#F5F0E6`) but will make characters bleed together on `#0a0908`.
3. **Consider keeping Instrument Serif exclusively for the amp brand text** and using Cormorant everywhere else. This gives the amp its own voice without forcing an elegant serif to pretend it's stamped metal.
4. **Drop weight 300 entirely for dark-on-dark use.** Cormorant 300 is beautiful on `--panna` and invisible on `--v-black`. Minimum weight on dark backgrounds should be 400, preferably 500.

---

## 2. Visual Consistency: V6 Architecture Section on Dark

### The v6 architecture section was explicitly designed for a dark background already

Good news: v6's `.architecture` section already uses `background: var(--warm-black)` (#1C1B18) with `color: var(--beige)`. So the signal chain boxes and blend detail box were *already* designed for dark. This is not the disaster it could have been.

### But the specific values will still clash with v5's palette

V6 dark: `#1C1B18` (warm-black) with borders at `rgba(196, 163, 90, 0.2)` and backgrounds at `rgba(196, 163, 90, 0.04)`.
V5 dark: `#0a0908` (v-black) with borders at `rgba(197, 164, 78, 0.06)` and `rgba(255,255,255,0.04)`.

The difference: v5 is *darker and lower contrast*. V6's architecture borders will feel too bright/visible if dropped into v5's environment without adjustment. The `0.2` border opacity on `.signal-node` will pop aggressively against `#0a0908`.

### Specific CSS values to adjust

| V6 Original | Needed for V7 |
|---|---|
| `.signal-node` border: `rgba(196, 163, 90, 0.2)` | Reduce to `0.08-0.12` to match v5 card borders |
| `.signal-node:hover` background: `rgba(196, 163, 90, 0.08)` | Fine, but hover border `var(--gold)` (solid) will be jarring -- add opacity |
| `.blend-detail` border: `rgba(196, 163, 90, 0.15)` | Reduce to `0.08` |
| `.blend-detail::before` background: `var(--warm-black)` | Must change to `var(--v-black)` or `#0a0908` to match the label chip background to the section background |
| `.blend-channel` background: `rgba(196, 163, 90, 0.02)` | Fine |
| `.tech-item` border-bottom: `1px solid var(--cream-dark)` (#DDD5C5) | This is a LIGHT color. Will be a blinding white line on dark. Replace with `rgba(197, 164, 78, 0.06)` |
| `.tech-name` color: `var(--warm-black)` (#1C1B18) | **Invisible on dark.** Must flip to `var(--v-cream)` |
| `.tech-desc` color: `var(--warm-gray)` (#6B6560) | Barely readable on dark. Use `var(--v-cream-dim)` |
| `.tech-version` background: `rgba(196, 163, 90, 0.1)` | Fine |

### The terminal block

V6's `.terminal` is already `background: var(--warm-black)` -- it was designed to be a dark island on a cream page. In v7, it's a dark block on a dark page. It will **disappear**. You need to either:
- Give it a lighter background (`var(--v-panel)` / `#1a1816`) so it has contrast against the section
- Add a visible border (`1px solid rgba(197, 164, 78, 0.08)`)
- Or: do both, like the v5 spec-cards with `rgba(18, 17, 16, 0.8)` + `border: 1px solid rgba(197, 164, 78, 0.06)`

### The `.github-cta` border

`border-top: 1px solid var(--cream-dark)` -- same problem. Cream-dark (#DDD5C5) is a light theme color. Pure white line on dark background.

---

## 3. Missing Transitions & Narrative Arc

### What the specs section actually did

V5's section order: Hero -> Zones -> Signal Chain -> Demos -> **Specs** -> CTA -> Footer

The specs section served as a **credibility bridge** between "look at the cool demos" and "download it now." It listed concrete technical details (macOS 12+, C++17, AU format, etc.) that reassured the user this is a real, engineered product. Without it, the flow becomes:

**Demos -> CTA**

That's going from "listen to these tones" straight to "download now." The build/source section from v6 partially fills this gap (terminal + tech stack), but it's developer-facing, not user-facing. A guitarist doesn't care about `cmake -B build`. They care about "macOS 12+, Logic Pro, GarageBand, free."

### The proposed order and its problems

`01-Zones -> 02-Signal -> 03-Demos -> 04-Build -> 05-CTA`

**Problem 1: Architecture before demos is backwards.** You're explaining *how* the sausage is made before letting people *taste* it. Most users don't care about the signal chain until they've heard the result. Suggested reorder:

`01-Zones -> 02-Demos -> 03-Signal -> 04-Build -> 05-CTA`

This follows the persuasion arc: **What is it? -> Does it sound good? -> How does it work? -> How do I get it? -> Go.**

**Problem 2: Build section before CTA kills momentum.** The CTA is "Stop tweaking. Start playing." with dramatic orbs and a VU meter. Putting a terminal with `cmake` commands right before it is like putting a tax form before a concert encore. The build section should be *after* the CTA or in a separate "Developers" area, or significantly compressed.

**Problem 3: No specs = no system requirements before download.** Add a small "Requirements" note inside the CTA section itself: "macOS 12+ | AU Plugin | Free & Open Source". Three items, one line, done.

---

## 4. Performance: Canvas Audit

V5 has **8 distinct canvas systems** running simultaneously. Here's the triage:

| Canvas | What it does | Lines of JS | Keep? | Rationale |
|---|---|---|---|---|
| `#bgCanvas` | Full-page particle system (floating gold dots) | ~50 | **CUT** | Runs continuously on every frame. Barely visible (opacity-level subtlety). The tolex texture + grain overlay already provide ambient texture. This is GPU waste. |
| `#grilleCanvas` | Oscilloscope waveform on hero grille | ~40 | **KEEP** | Visible, thematic, and only renders when hero is in view. Core to the amp-head metaphor. |
| 5x `.zone-oscilloscope` | Per-zone oscilloscope in zone cards | ~50 | **KEEP** | Only render on hover/visibility. Signature visual element. But: ensure they use `IntersectionObserver` to stop when offscreen. |
| `#circuitCanvas` | PCB trace pattern behind signal chain | ~50 | **CUT** | Renders at 6% opacity. The grid pattern from v6's `::before` pseudo-element achieves the same effect with zero JS and zero GPU cost. |
| 3x `.demo-canvas-bg canvas` | Waveform backgrounds behind vinyl records | ~55 | **CUT** | The vinyl records are already visually rich. These canvases run at 30% opacity behind them and are mostly occluded. Replace with a static SVG waveform pattern or CSS gradient. |
| `#ctaCanvas` | Ambient particle/constellation effect in CTA | ~45 | **CUT or simplify** | The CTA already has 3 animated orbs with `filter: blur(80px)`. Adding a canvas on top is overkill. The orbs alone provide sufficient atmosphere. |

**Net result:** Cut 4 of 8 canvases. This removes ~200 lines of JS, eliminates 4 `requestAnimationFrame` loops, and saves meaningful GPU/battery on laptops. The remaining canvases (grille + 5 zone oscilloscopes) are the ones users actually notice.

### Additional performance note

V5's `body::before` (grain) and `body::after` (scanlines) are both `position: fixed` with full-viewport SVG backgrounds. These trigger compositing layers. Combined with `backdrop-filter: blur(30px)` on the nav, you're asking the GPU to composite 3+ full-screen layers on every scroll frame. Test on a MacBook Air before shipping.

---

## 5. The Simplified Hero: Empty Amp Head Risk

### What's being removed

- 2 input jacks (`.amp-jack-group`)
- 1 pilot light + label (`.amp-pilot-group`)
- 1 zone indicator display (`.amp-zone-display`)

### What remains

- Brand text ("OneDial") in `.amp-logo-bar`
- Grille cloth section
- The dial (alone) in `.amp-control-strip`
- Bottom tagline bar

### The emptiness problem

The v5 control strip uses `gap: clamp(2rem, 5vw, 5rem)` with 5 elements (jack, pilot, dial, zone display, jack). Removing 4 of 5 leaves a single dial centered in a full-width strip. On a 1440px screen, that's a ~220px dial floating in ~1200px of empty dark space.

### What should fill the space

**Option A: Widen the dial.** Scale from `clamp(160px, 18vw, 220px)` to `clamp(200px, 25vw, 320px)`. A single, larger knob is a stronger statement than a small one surrounded by void.

**Option B: Add zone labels directly to the control strip.** Keep the 5 zone names (CLEAN / WARM / CRUNCH / DRIVE / LEAD) as subtle silk-screened labels flanking the dial, like the text printed on a real amp faceplate. No interactive elements, just typography. This fills the horizontal space while reinforcing the "five zones" concept.

**Option C: Add the zone indicator dots.** The `.zone-indicator-bar` with 5 small dots that light up based on dial position is subtle, informative, and doesn't add visual clutter. Move it into the control strip flanking the dial.

**Option D: Move the dial readout (zone label + value) from below the dial to beside it**, split left/right. Zone name on the left, numeric value on the right. This creates a "stereo" layout that fills the strip naturally.

**Recommendation:** Combine A + B. Larger dial, with faint zone labels silk-screened at equal intervals across the control strip background. This is what real amps do -- the faceplate is never blank; it always has labeling.

### The grille cloth section

With a simplified control strip, the grille cloth (which is just a dark rectangle with a dot pattern and a canvas oscilloscope) becomes the dominant visual. Make sure it has adequate `min-height` (at least 45-50vh). If it's too short, the amp looks like a toaster. If it's too tall, the user might not realize they need to scroll.

---

## 6. Section Ordering Deep Dive

### Proposed: 01-Zones -> 02-Signal -> 03-Demos -> 04-Build -> 05-CTA

### Recommended: 01-Zones -> 02-Demos -> 03-Signal -> 04-CTA -> 05-Build (or: drop Build entirely from main flow)

**Reasoning:**

| Position | What the user needs | Best section |
|---|---|---|
| After hero | "What does this thing do?" | Zones (establishes the 5-tone concept) |
| After zones | "Does it actually sound good?" | Demos (proof by audio) |
| After demos | "I'm interested, how does it work?" | Signal chain / architecture |
| After signal | "I want it" | CTA (emotional peak, download) |
| After CTA | "I want to build/contribute" | Build instructions (for the 2% who care) |

The v6 build section (terminal + tech stack) is GitHub README material. Putting it before the CTA creates a "developer detour" that kills conversion momentum for the 98% who just want the plugin. If you must include it, put it after the CTA as a quiet coda, or make it a collapsible/expandable section.

### The section dividers

V5 uses animated gold-piping dividers (`.section-divider` with a sweeping light animation) between sections. V6 uses `1px solid var(--cream-dark)` lines. The merge should use v5's dividers throughout for consistency, but consider making the sweep animation `prefers-reduced-motion`-aware (v5 already does this for most animations -- verify dividers are included).

---

## 7. Uncategorized Concerns

### Color variable collision

V5 uses `--m-gold: #c5a44e`. V6 uses `--gold: #C4A35A`. These are *slightly* different golds. Pick one. Having both in the merged stylesheet will create subtle but maddening inconsistencies.

### The v6 zones section is NOT the v5 zones section

V6 zones: simple cards in a horizontal flex row, minimal, no canvas, light background (`var(--ivory)`).
V5 zones: full-bleed immersive cards with grille cloth overlay, oscilloscope canvases, pilot LEDs, expanding on hover, gradient backgrounds per zone.

The merge plan says "exact v5 recreation." Good. But be aware that v6's zone CSS classes (`.zone-card`, `.zone-number`, `.zone-name`, `.zone-dot`, `.zone-desc`, `.zone-range`) share names with v5's zone CSS classes but have *completely different styling*. Do not attempt to merge the stylesheets by concatenation; you'll get specificity wars.

### The "Don't take our word for it" copy

V5's demos header uses this phrase. It works for audio demos. But if demos are placeholder (no actual audio files linked), this copy becomes ironic. Make sure the demo section either has real audio or changes the copy to something like "Built for every zone" with static descriptions.

### Theme morphing scope

V5's theme morphing (colors shift as dial rotates) uses `--theme-accent`, `--theme-glow`, and `--theme-morph` custom properties updated via JS. This affects the hero's grille glow and the background particle system. If you cut the background particle canvas (#bgCanvas), the morphing effect is reduced to just the grille glow -- which is fine, but verify it's still noticeable enough to justify the JS complexity.

### Mobile: The amp head at 375px

V5 already hides jacks and pilot lights at 768px (`display: none`). So the simplified hero actually matches what mobile users already see. This is good -- the merge's simplified hero is essentially "mobile-first" applied to desktop. Just verify the larger dial (if you go with Option A) doesn't overflow on small screens.

---

## Summary: Top 5 Risks, Ranked

1. **Cormorant Garamond at light weights on dark backgrounds will be illegible.** Use 500+ weight minimum, increase letter-spacing, or keep Instrument Serif for brand text.
2. **V6 terminal and tech-stack colors will be invisible on v5's dark palette.** Every light-theme color variable needs remapping.
3. **Build section before CTA breaks the persuasion flow.** Move it after CTA or merge system requirements into the CTA itself.
4. **4 unnecessary canvas systems will hurt performance** for zero perceptible benefit. Cut them.
5. **Empty control strip on desktop** needs visual compensation -- larger dial + silk-screened zone labels.
