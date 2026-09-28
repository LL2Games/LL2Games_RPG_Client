# 잎새마을 리소스

기존 게임 C++ 코드와 활성 맵 JSON은 수정하지 않았습니다. 이 폴더의 배치 정보와 NPC의 idle.json은 적용 시 참고할 메타데이터이며 현재 클라이언트에 자동 등록되지 않습니다.

## 이미지

- 마을 배경: `../../Background/village/leafhaven.png` (1536×1024)
- NPC: `../../NPC/{innkeeper,herbalist,elder,blacksmith,ranger,merchant}/`
- NPC마다 `idle_0.png`~`idle_5.png`: 투명 96×96, 동일 배율, 발 기준점 (48,92)
- NPC마다 `HD/idle_0.png`~`HD/idle_5.png`: 투명 512×512 원본 셀
- `idle-strip.png`: 왼쪽부터 0~5번 프레임을 배열한 576×96 시트
- `portrait.png`: NPC 기본 원화
- 원본 애니메이션 시트: 이 폴더의 `*-idle-atlas.png`
- 한눈에 확인: `village-preview.png` (마을에 NPC 배치), `idle-overview.png` (전체 IDLE 프레임)

## IDLE 재생

0 → 1 → 2 → 3 → 4 → 5 → 0 순서로 반복합니다. 권장 프레임 시간은 600, 180, 220, 100, 120, 350ms입니다. 숨쉬기, 눈 깜빡임, 작은 옷·소품 움직임을 위한 생성 프레임입니다. 프레임마다 크기를 따로 맞추지 않고 NPC별 공통 배율과 발 기준점을 적용했습니다. 생성형 원화 특성상 세부 선과 소품에 프레임 차이가 있을 수 있습니다.

| 폴더 | 이름 | 역할 | 배치 X |
|---|---|---|---:|
| innkeeper | 로제 | 여관주인 | 210 |
| herbalist | 리프 | 약초상 | 505 |
| elder | 오르 | 촌장 | 785 |
| blacksmith | 브람 | 대장장이 | 1140 |
| ranger | 세린 | 숲지기 | 1380 |
| merchant | 토비 | 잡화상 | 940 |

마을 바닥의 캐릭터 발 기준 Y는 630입니다. 생성 요청의 Y=820과 달리 실제 이미지의 길은 Y≈630에 있으므로 실제 이미지에 맞춰 배치했습니다. `placement.json`은 마을 ID 100000003, NPC 이름·대사·좌표·포탈의 제안 데이터입니다. 서버 등록과 충돌 지형, 상점 기능, NPC 상호작용은 구현하지 않았습니다.

이미지는 built-in ImageGen으로 생성했습니다. 정확한 요청문은 `PROMPTS.md`, `idle-prompts.json`에 보관했습니다. 원화 출처는 이 리소스 팩의 새 생성 이미지입니다.
