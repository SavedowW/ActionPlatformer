# TODOs

### 1. Mandatory

1. [ ] Restore all remaining states
    1. [x] Restore states in basic form
    2. [ ] Restore attack hitboxes
        - Needs 1.8
    3. [ ] Restore hurtboxes
        - Needs 1.8
    4. [ ] Restore state particle
        - Needs 1.5
    5. [ ] Add a proper way to prevent attacks from going through enemies due to movement
    6. [x] Add a screen shake effect
        1. [x] On state entry (for hard landing recovery)
        2. [x] On state update (for attack chain)
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
9. [ ] Move timeline property iteration and caching logic into the iterator
10. [x] Fix bug when the neutral jump after chain attack faces the opposite direction


### 2. Maybe later

1. [ ] Fix infinite loop when moving non-obstacle collider pushes moving character into the ground (should be an impossible situation in-game due to a 1.3)
