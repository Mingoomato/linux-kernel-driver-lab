# Portfolio review / 포트폴리오 검토 안내

## 문제와 제작 의도

고정 폭 userspace ABI와 커널 자원 수명을 설계하기. 이 저장소는 Linux 시스템 호출의 결과와 실패를 관측 가능한 계약으로 연결하는 교육용 구현입니다.

## 핵심 설계

character device에 bounded event queue와 read/poll/ioctl을 제공합니다. ABI version과 reserved 필드를 확인하며 32비트 compat dispatch를 분리합니다. injection은 기본 비활성이고 host에서 실험 모듈을 로드하지 않도록 실습 절차를 둡니다.

## 코드 검토 경로

1. [`kernel/securewatch_main.c`](../kernel/securewatch_main.c)의 CLI 진입점에서 자원 획득과 오류 반환을 찾습니다.
2. [코드 해설](CODE_WALKTHROUGH.md)에서 핵심 함수와 소유권을 따라갑니다.
3. [최신 검증 기록](VALIDATION_CHECKPOINT.md)과 [요구사항 추적표](REQUIREMENTS_TRACEABILITY.md)를 비교합니다.
4. [기술 질문](TECHNICAL_INTERVIEW.md)과 [정답 없는 실습](DEBUGGING_EXERCISES.md)으로 직접 설명·재현합니다.

검토 질문: copy_to_user 실패 시 queue 항목의 소유권은? poll readiness와 read 사이 경쟁은? compile 성공으로 kernel runtime을 주장할 수 없는 이유는?

## English

**Purpose:** Design a fixed-width userspace ABI and kernel resource lifetimes.

The character device exposes a bounded event queue and read/poll/ioctl. ABI versions and reserved fields are checked, with a separate compat dispatch. Injection is disabled by default, and the lab workflow keeps module loading off the host.

Start with the linked source entry point, follow `CODE_WALKTHROUGH.md`, and compare the dated checkpoint with requirement traceability. Review questions: Who owns a queue entry after copy_to_user fails? What can race between poll readiness and read? Why does a compiled module not establish runtime correctness?

The five study documents cover subsystem purpose, data/control flow, Linux interfaces, memory/resource lifetime, concurrency, security assumptions, failures and alternatives. Three reproduction exercises remain unsolved. Repository checks and the owner's demonstrated competence are tracked separately; neither AI assistance nor automated test completion establishes personal mastery.
