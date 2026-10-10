# Linux Kernel Driver Lab

[:kr: 한국어](#한국어) · [:us: English](#english)

## 한국어

이 프로젝트는 `/dev/securewatch` misc 문자 디바이스, 버전이 있는 고정 크기 UAPI, 사용자 공간 `swctl`을 구현한 커널 학습 실습입니다. 제작 의도는 `read`/`write`/`ioctl`/`poll` 경계, bounded ring buffer, wait queue와 spinlock 동기화, 사용자 포인터 검증, 모듈 자원 수명을 실제 작은 드라이버로 설명하는 것입니다. 파일 감시나 실서비스 보호 기능은 없습니다.

### 빌드 및 검증

```sh
make test
make module
```

`make test`는 사용자 공간 프로그램, native/가능 시 32비트 UAPI 레이아웃, `swctl listen` 입력 검증, JSON payload 인코딩, mock poll/read 기반 stale readiness와 signal 재시도·전체 timeout, VM 로드 가드의 기본 거부 동작을 확인합니다. 잘못된 UTF-8 바이트는 JSON 문자 U+00xx에 대응하여 표현됩니다. `make module`은 실행 중인 커널의 헤더로 모듈을 컴파일할 뿐 적재하지 않습니다. 현재 Linux `7.1.5+kali-amd64` 헤더를 이용한 GCC/Clang 모듈 빌드, GCC/Clang 및 Clang ASan/UBSan 사용자 공간 smoke가 통과했습니다. 모듈 runtime, 동시 reader/writer, overflow, KASAN/KCSAN, 두 번째 커널 매트릭스는 실행하지 않았습니다.

### 격리 및 안전

커널 모듈은 오류가 있으면 게스트를 멈출 수 있으므로 폐기 가능한 VM 스냅샷과 복구 콘솔에서만 로드하세요. `scripts/vm-test.sh`는 명시적 `SECUREWATCH_ALLOW_LOAD=YES`, snapshot 준비 확인, recovery console 준비 확인, VM 감지 조건을 요구하며 테스트 로드는 `allow_lab_injection=1`을 명시합니다. 두 준비 확인 변수는 운영자 확인값이며 snapshot이나 console 존재를 자동으로 검증하지는 않습니다. 모듈 파라미터 기본값은 주입 비활성입니다. 현재 호스트는 VM으로 감지되지 않아 모듈을 적재하지 않았습니다. 장치 기본 권한은 `0600`, synthetic 이벤트 쓰기와 통계 초기화는 `CAP_SYS_ADMIN`이 필요합니다.

현재 변경에서는 blocking `read`가 다른 reader가 이벤트를 먼저 소비한 경우 조건을 다시 확인하고, `swctl listen`은 stale poll 뒤 `EAGAIN`을 다시 기다리며 전체 요청에 단조 시계 기반 timeout을 적용합니다. signal의 `EINTR`에도 timeout을 재시작하지 않습니다. 숫자 옵션은 장치 접근 전에 엄격히 검증합니다. `listen` JSON encoder는 quote/control과 malformed UTF-8 payload를 안전하게 표현합니다. 실습 이벤트 주입은 명시적인 모듈 옵션을 지정해야만 활성화됩니다. 자세한 ABI와 검증 범위는 [`docs/UAPI_VERSIONING.md`](docs/UAPI_VERSIONING.md), [`docs/TEST_RESULTS.md`](docs/TEST_RESULTS.md)를 참고하세요.

추가 구현으로 `CONFIG_COMPAT` 커널에서 ioctl 포인터를 `compat_ptr_ioctl`로 전달하고, 사용자 공간 stats 응답의 ABI version/reserved 값을 검사합니다. 이 코드는 아직 빌드나 테스트로 재검증하지 않았습니다.

사용자 주입 이벤트에서 `sequence`와 `timestamp_ns`는 0이어야 하며 커널이 값을 부여합니다. smoke 테스트는 이 규칙을 포함한 UAPI 필드 배치 및 ioctl 인코딩을 64비트와 (컴파일러 지원 시) 32비트로 검사합니다.

### 학습 패키지

코드와 시스템 경계 학습 자료는 [`docs/ENGINEERING_STUDY_GUIDE.md`](docs/ENGINEERING_STUDY_GUIDE.md), [`docs/CODE_WALKTHROUGH.md`](docs/CODE_WALKTHROUGH.md), [`docs/LINUX_INTERNALS_EXPLAINED.md`](docs/LINUX_INTERNALS_EXPLAINED.md), [`docs/DEBUGGING_EXERCISES.md`](docs/DEBUGGING_EXERCISES.md), [`docs/TECHNICAL_INTERVIEW.md`](docs/TECHNICAL_INTERVIEW.md)에 있습니다. 세 독립 구현 연습은 완성 답안을 싣지 않으며, 기술 면접 질문 32개가 포함되어 있습니다. 저장소 자동 검증을 소유자 숙련도의 증거로 보지 않습니다.

## English

This educational project teaches Linux kernel/user-space boundaries through a versioned UAPI, an out-of-tree misc character device, and a C17 client for synthetic lab events. It exists to demonstrate bounded queues, safe user copies, synchronization, and module lifetime. The module has no filesystem hooks, process-memory access, hidden behavior, or production protection claims.

## Build userspace

    make userspace

`make test` compiles the userspace client, checks strict `swctl listen` numeric arguments, tests JSON control/UTF-8 payload encoding (invalid bytes map to their corresponding U+00xx code points), checks native and optionally 32-bit UAPI layout, mocks poll/read to check `EAGAIN` and `EINTR` retries plus the overall timeout, and verifies the VM script refuses loading without opt-in and recovery confirmations. GCC, Clang, and Clang ASan/UBSan smoke runs passed. `make module` compiles against the running kernel's headers without loading the module; GCC and Clang Kbuild succeeded for Linux `7.1.5+kali-amd64`.

## Build the module (compile only)

    make module

This requires headers matching the running kernel. Building does not load the module. Never run insmod/rmmod on a shared host; privileged lifecycle work belongs in a disposable Linux VM with a snapshot and recovery console.

## VM-only exercise

    SECUREWATCH_ALLOW_LOAD=YES \
    SECUREWATCH_VM_SNAPSHOT_READY=YES \
    SECUREWATCH_RECOVERY_CONSOLE_READY=YES \
    ./scripts/vm-test.sh

The guarded script first loads the module with injection disabled and confirms that a synthetic write is rejected, then unloads and reloads with `allow_lab_injection=1` for one event round trip. The device defaults to mode 0600. Injection plus counter clearing require CAP_SYS_ADMIN. Injected records must leave kernel-owned sequence and timestamp fields at zero. A bounded 256-record ring reports drops. The read interface returns one fixed-size event per successful read.

The current host is not detected as a VM, so the module was compile-checked but not loaded. `swctl listen` strictly validates decimal count and timeout values before opening the device. It retries stale readiness and interruptions against one monotonic deadline, and reports expiry as `ETIMEDOUT`. Module runtime tests are not run unless a disposable VM with a snapshot and recovery console is available. The readiness environment variables are operator confirmations, not independent proof. See docs/UAPI_VERSIONING.md and docs/TEST_RESULTS.md.

The latest source change adds `compat_ptr_ioctl` routing when `CONFIG_COMPAT` is enabled and validates stats ABI/reserved fields in userspace. This change has not been rebuilt or tested.

### Study package

The guide, walkthrough, Linux internals explanation, three unsolved exercises, and 32 repository-specific interview questions are in [`docs/ENGINEERING_STUDY_GUIDE.md`](docs/ENGINEERING_STUDY_GUIDE.md), [`docs/CODE_WALKTHROUGH.md`](docs/CODE_WALKTHROUGH.md), [`docs/LINUX_INTERNALS_EXPLAINED.md`](docs/LINUX_INTERNALS_EXPLAINED.md), [`docs/DEBUGGING_EXERCISES.md`](docs/DEBUGGING_EXERCISES.md), and [`docs/TECHNICAL_INTERVIEW.md`](docs/TECHNICAL_INTERVIEW.md). Exercise solutions are intentionally omitted until an attempt is submitted.
