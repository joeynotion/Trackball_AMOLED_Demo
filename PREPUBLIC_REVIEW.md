# Pre-publication Review

This document summarizes a pre-publication review of the repository with concrete recommendations.

## Scope reviewed

- Repository structure and top-level docs.
- Build configuration (`platformio.ini`, partition layout, build patch script).
- Core firmware files (`src/main.cpp`, display/input/UI drivers).
- Public-readiness items (license, security policy, maintenance signals).

## What is already strong

1. **Clear hardware focus**: The project is explicit about target board/peripherals and pin usage.
2. **Practical power-management implementation**: The wake/sleep state machine is understandable and well logged.
3. **Usable demonstration app**: The 3x3 trackball-driven LED demo is a good minimal reference.
4. **Helpful troubleshooting notes**: README includes concrete recovery tips for common issues.

## Risks before public release

### High priority

1. **No explicit OSS license file**
   - Without a `LICENSE`, reuse rights are ambiguous for contributors and users.
   - Recommendation: add a license file before publishing.

2. **No security reporting policy**
   - Researchers/users have no disclosure path for vulnerabilities.
   - Recommendation: add a minimal `SECURITY.md` with contact and SLA expectations.

### Medium priority

3. **Dependency drift risk**
   - Current setup relies on external package resolution over time.
   - Recommendation: pin key dependency versions in `platformio.ini`.

4. **Operational assumptions not fully formalized**
   - Light-sleep + reinit behavior is described, but no explicit "known limitations" section.
   - Recommendation: add expected USB CDC behavior and wake latency notes in docs.

### Low priority

5. **Minor documentation mismatches**
   - Some wording could imply file names/behavior that differ from source.
   - Recommendation: keep README wording tightly aligned to current code paths.

## Suggested go-public criteria

Ship publicly once the following are complete:

- [ ] `LICENSE` added.
- [ ] `SECURITY.md` added.
- [ ] Dependency versions pinned or intentionally documented as floating.
- [ ] README includes one hardware screenshot or gif.
- [ ] Fresh-clone build test documented (exact command + board/env).

## Verification commands used in this review

```bash
rg --files
sed -n '1,220p' README.md
sed -n '1,220p' platformio.ini
nl -ba src/main.cpp | sed -n '1,320p'
sed -n '1,260p' src/input.cpp
sed -n '1,300p' src/ui.cpp
sed -n '1,260p' src/trackball.h
sed -n '1,260p' src/qspi_display.h
sed -n '1,260p' src/lv_conf.h
```
