4/14/26: Copilot mentioned the hard-coded project_root. I intend to leave this in for now, though I admit it's fragile. I am the only person to interact with this codebase so I'm not particularly worried about this for now. It is not intended as a final implementation detail.

Error-path hardening
    Goal: make swapchain transitions predictable.
    Focus: unify acquire/present out-of-date handling into one state machine so “skip frame” and “recreate” are explicit and traceable.

Performance baseline pass
    Goal: establish objective “before/after” numbers.
    Focus: frame time, number of recreates during resize, and CPU time spent in sync/wait paths.