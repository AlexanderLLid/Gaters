#!/usr/bin/env node
// Deterministic checks for the Gateers wiki. No LLM. Run: node scripts/lint.mjs
// Checks: frontmatter type present and known, unresolved hard contradictions,
// orphan pages (no inbound wikilinks), and dangling wikilinks (no target page).
import { readdirSync, readFileSync, statSync } from "node:fs";
import { join, relative, basename, extname } from "node:path";

const ROOT = process.cwd();
const SCAN = ["docs/lore", "docs/systems"];
const TYPES = new Set([
  "character", "location", "organization", "species", "culture", "religion",
  "myth", "item", "language", "event", "overview", "timeline",
  "system", "resource", "recipe", "station", "structure", "creature", "biome",
  "status-effect", "tech", "formula",
]);

function walk(dir) {
  let out = [];
  let names = [];
  try { names = readdirSync(dir); } catch { return out; }
  for (const name of names) {
    if (name.startsWith(".") || name === "_templates") continue;
    const p = join(dir, name);
    if (statSync(p).isDirectory()) out = out.concat(walk(p));
    else if (extname(p) === ".md") out.push(p);
  }
  return out;
}

function frontmatter(text) {
  const lines = text.split(/\r?\n/);
  if (lines[0] !== "---") return "";
  const end = lines.indexOf("---", 1);
  return end === -1 ? "" : lines.slice(1, end).join("\n");
}

function field(fm, key) {
  for (const line of fm.split("\n")) {
    const t = line.trim();
    if (t.startsWith(key + ":")) return t.slice(key.length + 1).trim();
  }
  return "";
}

function aliases(fm) {
  let v = field(fm, "aliases");
  if (v.startsWith("[")) v = v.slice(1, -1);
  return v.split(",").map((s) => s.trim().replace(/^["']|["']$/g, "")).filter(Boolean);
}

function wikilinks(text) {
  const out = new Set();
  let i = 0;
  for (;;) {
    const a = text.indexOf("[[", i);
    if (a === -1) break;
    const b = text.indexOf("]]", a + 2);
    if (b === -1) break;
    let t = text.slice(a + 2, b);
    for (const sep of ["#", "|"]) {
      const k = t.indexOf(sep);
      if (k !== -1) t = t.slice(0, k);
    }
    t = t.trim();
    if (t) out.add(t.toLowerCase());
    i = b + 2;
  }
  return out;
}

const files = SCAN.flatMap((d) => walk(join(ROOT, d)));
const pages = new Map();
const hub = new Map();
const linkSets = new Map();
const problems = [];

for (const f of files) {
  const text = readFileSync(f, "utf8");
  const fm = frontmatter(text);
  pages.set(basename(f, ".md").toLowerCase(), f);
  for (const a of aliases(fm)) pages.set(a.toLowerCase(), f);
  hub.set(f, field(fm, "tags").includes("hub"));
  linkSets.set(f, wikilinks(text));

  const type = field(fm, "type");
  if (!type) problems.push(relative(ROOT, f) + ": missing frontmatter type");
  else if (!TYPES.has(type)) problems.push(relative(ROOT, f) + ": unknown type " + type);

  const low = text.toLowerCase();
  if (low.includes("severity: hard") && low.includes("status: unresolved"))
    problems.push(relative(ROOT, f) + ": unresolved hard contradiction");
}

const inbound = new Map();
for (const set of linkSets.values())
  for (const t of set) inbound.set(t, (inbound.get(t) || 0) + 1);

// Valid link targets = the whole vault (docs/), not just the lore/systems pages
// we quality-check. Root design docs (world-spine, open-questions, pillars…) are
// real Obsidian targets too; resolving only against lore/systems false-flags them.
const targets = new Set();
for (const f of walk(join(ROOT, "docs"))) {
  const fm = frontmatter(readFileSync(f, "utf8"));
  targets.add(basename(f, ".md").toLowerCase());
  for (const a of aliases(fm)) targets.add(a.toLowerCase());
}

for (const [f, set] of linkSets)
  for (const t of set)
    if (!targets.has(t))
      problems.push("dangling link in " + relative(ROOT, f) + ": [[" + t + "]]");

for (const [id, f] of pages) {
  if (hub.get(f)) continue;
  if (basename(f, ".md").toLowerCase() !== id) continue;
  if (!inbound.get(id)) problems.push(relative(ROOT, f) + ": orphan (no inbound wikilinks)");
}

if (problems.length === 0) {
  console.log("wiki lint: clean");
  process.exit(0);
}
console.log("wiki lint: " + problems.length + " issue(s)");
for (const p of problems) console.log("  - " + p);
process.exit(1);
