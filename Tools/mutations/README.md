# Mutation patches

Each patch breaks one new assertion once, as `docs/40_DEV_GUIDE.md` asks of every new test: apply it (`git apply <patch>`),
build, run the named test and see it fail, then revert (`git apply -R <patch>`). The patch name says which test it breaks.
Kept from milestone B on (B1); the earlier milestones ran their mutations from scripts and recorded them in the commit
messages and in `docs/60_DECISIONS/DECISIONS_LOG.md`.

| Folder | Patch | Breaks |
|---|---|---|
| `B1/` | `b1a_walk_from_one_place.patch` — the look starts from the first place of the eye only | `Cosmic_Ascension.Sim.SightWalkMoved` (both ruler cases) |
