# Validation checkpoint — 2026-10-11

한국어: 최신 코드의 기본 빌드와 아래 한정된 기능 시험을 수행했습니다. 이 기록이 이전의 “현재 변경 미검증” 기록보다 최신이며, SRS 전체 완료나 소유자 숙련도를 뜻하지 않습니다.

English: This checkpoint covers the current source hashes in [validation-checkpoint.json](validation-checkpoint.json). It supersedes earlier unbuilt/unverified statements only for the checks explicitly listed here. It does not certify all SRS gates or owner mastery.

## Environment and commands

Linux 7.1.5+kali-amd64, x86_64; GCC 16.2.0; CMake Release; Python 3.14.7.

```sh
make userspace
make test
make module
```

## Result and scope

make userspace and make test passed; make module passed.

Native/32-bit UAPI layout, JSON output, listener retry/deadline and module compile against Linux 7.1.5+kali-amd64 headers.

## Remaining limits

No kernel module was loaded. systemd-detect-virt returned none; disposable-VM runtime, KASAN/KCSAN and compat runtime remain blocked here.

## CI and source provenance

[Build and test workflow](../.github/workflows/ci.yml) runs GCC/Clang jobs on Ubuntu 24.04. The action is pinned to the [official checkout v4.2.2 commit](https://github.com/actions/checkout/releases/tag/v4.2.2). Kernel loading and privileged experiments are excluded from hosted runners. A workflow definition alone is not remote CI evidence; consult the actual run in GitHub Actions and the portfolio report.

[Source hashes and machine-readable scope](validation-checkpoint.json) identify the files exercised. Documentation and CI additions do not imply that pending runtime/fault matrices passed.

### Remote run outcome

[Run 38107490096](https://github.com/Mingoomato/linux-kernel-driver-lab/actions/runs/38107490096) was created after the source commit, but GitHub refused to start every job because of an account billing/payment or spending-limit restriction. This is **BLOCKED before execution**, not a passing remote check and not an observed compiler/test failure. No account payment or visibility settings were changed.

## Current-source local verification follow-up — 2026-10-11

The GitHub Actions build/test commands were run locally on Kali Linux 7.1.5+kali-amd64, x86_64, GCC 16.2.0, Clang 21.1.8 and CMake 4.4.3. This host is not the Ubuntu 24.04 runner. No kernel module was loaded.

- GCC and Clang userspace/UAPI smoke: **passed**.
- Clang AddressSanitizer + UBSan userspace/UAPI smoke: **passed**.
- Kernel module compile against Linux 7.1.5 headers: **passed**; no load.

GCC/Clang userspace/UAPI tests, Clang ASan/UBSan userspace test and module compile against Linux 7.1.5 headers pass; module runtime not attempted.

The original GitHub Actions runs remain blocked before execution by account billing/spending-limit state. Current-source local results do not close the remaining full-SRS runtime, fault, scale or OS gates. Owner mastery remains unassessed.
