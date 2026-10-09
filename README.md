# Linux Kernel Driver Lab

[:kr: 한국어](#한국어) · [:us: English](#english)

## 한국어

이 프로젝트는 `/dev/securewatch` misc 문자 디바이스, 버전이 있는 고정 크기 UAPI, 사용자 공간 `swctl`을 구현한 커널 학습 실습입니다. 제작 의도는 `read`/`write`/`ioctl`/`poll` 경계, bounded ring buffer, wait queue와 spinlock 동기화, 사용자 포인터 검증, 모듈 자원 수명을 실제 작은 드라이버로 설명하는 것입니다. 파일 감시나 실서비스 보호 기능은 없습니다.

### 빌드 및 검증

```sh
make test
make module
```

`make test`는 사용자 공간 프로그램, 64비트 UAPI 레이아웃, 가능할 경우 32비트 UAPI 컴파일, VM 로드 가드의 기본 거부 동작을 확인합니다. `make module`은 실행 중인 커널의 헤더로 모듈을 컴파일할 뿐 적재하지 않습니다. 현재 Linux `7.1.5+kali-amd64` 헤더 빌드와 `make test`가 통과했습니다. 모듈 runtime, 동시 reader/writer, overflow, KASAN/KCSAN, 두 번째 커널 매트릭스는 실행하지 않았습니다.

### 격리 및 안전

커널 모듈은 오류가 있으면 게스트를 멈출 수 있으므로 폐기 가능한 VM 스냅샷과 복구 콘솔에서만 로드하세요. `scripts/vm-test.sh`는 명시적 `SECUREWATCH_ALLOW_LOAD=YES`와 VM 감지 조건을 요구하며, 테스트 로드는 `allow_lab_injection=1`을 명시합니다. 모듈 파라미터 기본값은 주입 비활성입니다. 현재 호스트는 VM으로 감지되지 않아 모듈을 적재하지 않았습니다. 장치 기본 권한은 `0600`, synthetic 이벤트 쓰기와 통계 초기화는 `CAP_SYS_ADMIN`이 필요합니다.

현재 변경에서는 blocking `read`가 다른 reader가 이벤트를 먼저 소비한 경우 조건을 다시 확인하도록 보완했고, 실습 이벤트 주입은 명시적인 모듈 옵션을 지정해야만 활성화됩니다. 자세한 ABI와 검증 범위는 [`docs/UAPI_VERSIONING.md`](docs/UAPI_VERSIONING.md), [`docs/TEST_RESULTS.md`](docs/TEST_RESULTS.md)를 참고하세요.

사용자 주입 이벤트에서 `sequence`와 `timestamp_ns`는 0이어야 하며 커널이 값을 부여합니다. smoke 테스트는 이 규칙을 포함한 UAPI 필드 배치 및 ioctl 인코딩을 64비트와 (컴파일러 지원 시) 32비트로 검사합니다.

## English

This project contains a versioned UAPI, an out-of-tree misc character device, and a C17 client for synthetic lab events. The module has no filesystem hooks, process-memory access, hidden behavior, or production protection claims.

## Build userspace

    make userspace

`make test` compiles the userspace client, checks UAPI field/ioctl layout in native and optionally 32-bit compilation, and verifies the VM script refuses loading without explicit opt-in. `make module` compiles the module against the running kernel's headers without loading it.

## Build the module (compile only)

    make module

This requires headers matching the running kernel. Building does not load the module. Never run insmod/rmmod on a shared host; privileged lifecycle work belongs in a disposable Linux VM with a snapshot and recovery console.

## VM-only exercise

    sudo insmod kernel/securewatch.ko allow_lab_injection=1
    sudo ./userspace/swctl doctor
    sudo ./userspace/swctl inject-lab-event --message hello
    ./userspace/swctl listen --count 1 --timeout-ms 1000
    ./userspace/swctl stats
    sudo rmmod securewatch

The device defaults to mode 0600. Synthetic injection is disabled unless `allow_lab_injection=1` is supplied at module load, and injection plus counter clearing require CAP_SYS_ADMIN. Injected records must leave kernel-owned sequence and timestamp fields at zero. A bounded 256-record ring reports drops. The read interface returns one fixed-size event per successful read.

The current host is not detected as a VM, so the module was compile-checked but not loaded. Module runtime tests are deliberately not run unless a disposable VM with a snapshot and recovery console is available. See docs/UAPI_VERSIONING.md and docs/TEST_RESULTS.md.
