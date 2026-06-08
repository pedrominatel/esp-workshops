---
name: multinet-g2p
description: Converts English text (graphemes) to the ESP-SR MultiNet phoneme string without running any script. Use when the user asks to convert speech commands, words, or phrases to phonemes for ESP-SR MultiNet, asks for g2p conversion, or needs the phoneme representation of English text for an ESP-IDF wake word or command model.
---

# MultiNet G2P — Grapheme-to-Phoneme Conversion

Converts English text to the ESP-SR MultiNet phoneme alphabet by:
1. Mapping each word to its CMU ARPAbet phonemes
2. Translating each phoneme token through the MultiNet alphabet table

## Input format

- Multiple phrases within a command group are separated by `,`
- Multiple command groups are separated by `;`

Example: `turn on,switch on;turn off,switch off`

## MultiNet phoneme alphabet

Stress digits (0/1/2) on vowels are stripped before lookup — all stress variants map to the same letter.

| ARPAbet | Letter || ARPAbet | Letter |
|---------|--------||---------|--------|
| AE      | a      || DH      | j      |
| OW      | b      || ER      | k      |
| AH      | c      || NG      | l      |
| EY      | d      || IY      | m      |
| AO      | e      || AA      | n      |
| EH      | f      || UW      | o      |
| IH      | g      || CH      | p      |
| HH      | h      || JH      | q      |
| AY      | i      || ZH      | r      |
| AW      | t      || SH      | s      |
| OY      | u      || TH      | v      |
| (space) | (space)|| UH      | w      |
| B       | B      || K       | K      |
| D       | D      || L       | L      |
| F       | F      || M       | M      |
| G       | G      || N       | N      |
| P       | P      || R       | R      |
| S       | S      || T       | T      |
| V       | V      || W       | W      |
| Y       | Y      || Z       | Z      |

Phonemes not present in the table are silently skipped.

## Conversion steps

1. Split input on `;` → command groups
2. Split each group on `,` → individual phrases
3. For each phrase, derive ARPAbet phoneme sequence using standard English pronunciation rules (CMU dict knowledge)
4. Strip trailing stress digit from each phoneme token (e.g. `AE1` → `AE`, `ER0` → `ER`)
5. Look up each token in the table above; skip unknowns
6. Join mapped characters (no separator between phonemes within a phrase)
7. Re-join phrases with `,` and groups with `;`

## Examples

**Input:** `turn on`
**ARPAbet:** T ER0 N (space) AO1 N
- T→T, ER0→k, N→N, ` `→` `, AO1→e, N→N
**Output:** `TkN eN`

**Input:** `hello`
**ARPAbet:** `HH AH0 L OW1`
**Output:** H→h, AH0→c, L→L, OW1→b → `hcLb`

**Input:** `turn on,switch on;turn off`
**Output:** `TkN eN,sWgp eN;TkN eF`

## Notes

- The output is **case-sensitive**: uppercase letters are consonants that map 1-to-1, lowercase letters represent vowel classes.
- Spaces between words within a phrase are preserved as single space characters.
- Always preserve the `,` and `;` delimiters in the output at the same positions as the input.
- When in doubt about a phoneme sequence, prefer the CMU Pronouncing Dictionary transcription.
