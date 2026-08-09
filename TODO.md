# TODOs

### 1. Mandatory

1. [ ] Restore all remaining states
    1. [ ] Restore states in basic form
    2. [ ] Restore attack properties
        - Needs 1.8
    3. [ ] Restore state particle
        - Needs 1.5
    4. [ ] Add a proper way to prevent attacks from going through enemies due to movement
    5. [ ] Add a screen shake effect on state update / entry (hard landing recovery, attack chain)
2. [ ] Add collider traits to allow / disallow clinging
3. [ ] Add obstacle traits to enable wall-only behavior
4. [ ] Refactor platform movement logic
    1. [ ] Reuse proper object movement logic
    2. [ ] Make so that all moving platforms are clingable obstacles by default
        - Needs 1.3
5. [ ] Restore the particle platform attachment
6. [ ] Finish AI system
7. [ ] Restore enemy system
8. [ ] Restore attack system


### 2. Maybe later

1. [ ] Fix infinite loop when moving non-obstacle collider pushes moving character into the ground (should be an impossible situation in-game due to a 1.3)
