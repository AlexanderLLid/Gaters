---
name: ingest
description: Ingest a raw source into the Gaters wiki. Use when the user adds a file to raw/ and asks to process, ingest, or file it, or says update the wiki from this source.
---

# ingest

Input: a source in raw/ (or pasted text the human wants filed).

Steps:
1. Read the source fully.
2. Summarize the key takeaways back to the human.
3. Decide which wiki(s) and which pages it touches (lore, systems, or both).
4. Create or update those pages from the templates in _templates/. Follow the
   conventions in AGENTS.md: frontmatter type, wikilinks, and anchor claims to
   the source as (src: raw/file).
5. For systems pages, document the model and point to where data and code live;
   never freeze balance numbers in prose.
6. Update index.md (add new pages, refresh one-line summaries).
7. Append an entry to log.md with the [date] ingest | source prefix.
8. Run the contradiction-check discipline on the touched pages.

A single source may touch several pages across both wikis.
