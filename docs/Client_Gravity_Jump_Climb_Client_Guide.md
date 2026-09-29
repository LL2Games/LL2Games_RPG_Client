# 클라이언트 중력·점프·로프·사다리 구현 가이드

기준: 2026-09-29 서버 개발 전달서와 함께 받은 movement_data/maps.json.
이전 가이드의 예측·입력 재실행·RESYNC 제안은 현재 서버와 호환되지 않아 제거했다.

## 1. 패킷 연결

- 0x002C PKT_MOVEMENT_INPUT: mapId, epoch, sequence, horizontal, vertical, jump — 정확히 6필드.
- 0x002D PKT_MOVEMENT_SNAPSHOT: mapId, kind, entityId, epoch, tick, sequence, x, y, vx, vy, mode, facing, climbableId, lifeState, hp, maxHp — 정확히 16필드.
- 기존 TCP 프레임과 uint16 big-endian 길이 접두사 문자열 직렬화를 재사용한다.
- tick은 uint64, epoch/sequence는 int. sequence는 수락 번호이며 물리 처리 완료 ack가 아니다.
- 구 PKT_PLAYER_MOVE 좌표 송신 및 0x002E 송신은 하지 않는다. 입력 opcode의 nok는 로그로 처리한다.
- 전체 파싱 후 검증한다. 잘린 패킷·추가 필드·NaN·범위 밖 상태는 부분 갱신 없이 거부한다.

구현: MovementTypes.h, MovementProtocol.h/.cpp, MovementPacketHandler.h/.cpp, Packet.h, PacketManager.cpp.

## 2. 맵 데이터

서버 전달본은 [movement_data/maps.json](movement_data/maps.json)에 보관한다. sourceSha256은 전달받은 서버 원본 파일의 해시이며 이 축약 JSON의 해시가 아니다.

클라이언트 Data/Maps/100000000~100000002.json의 배경·텍스처·렌더 크기는 보존하고 physics 및 포탈의 위치/거리/목적지만 병합한다.

- minX/maxX/killY/safeFeet/platforms/climbables를 읽는다.
- 발판은 발의 X 중앙점을 기준으로 하는 수평 단방향 발판이다.
- 로프와 사다리 모두 topPlatformId의 발판으로 올라선다. 배열 순서를 유지한다.
- climbableId로 맵의 kind를 조회해 줄 종류를 구분한다.
- 좌표는 객체 원점이다. 플레이어 발점은 (x, y+10)이며 Transform에는 수신 y를 그대로 적용한다.
- 중력 1800, 점프 vy -650, 낙하 상한 1000, 탑승 속도 180, 재탑승 제한 0.2초는 서버 상수다. 클라이언트에서 재계산하지 않는다.

구현: MovementMap.h, MapInfo.h, MapDataManager.cpp, MapScene.cpp.

전달 좌표 주의: to_town_pending의 y=60은 기본 바닥에서 접근하기 어렵고, 일부 spawnPosition은 바닥 y=700 아래다. 서버 데이터 그대로 유지하며 서버/배경과 함께 검증한다. to_map_100000003의 실제 목적지는 100000002(같은 맵)이다.

## 3. 입력 송신

- 왼쪽 Alt=점프, 좌우=이동, 상하=탑승, Space(Interact)=포탈.
- 축이 바뀌거나 점프 Down이면 즉시 송신, 그 외 100ms heartbeat.
- 키 해제·채팅/UI 차단·포커스 상실에는 즉시 0 입력. 점프는 heartbeat에 반복하지 않는다.
- 첫 내 스냅샷 전에는 송신하지 않는다. 새 epoch마다 sequence를 다시 시작한다.
- 사망/기절/맵 전환 대기 중 입력을 차단한다. 수신과 보간은 계속한다.

구현: MovementStream.h의 InputSchedule, stbPlayerScript.cpp.

## 4. 스냅샷과 보간

- 내 플레이어/다른 플레이어/몬스터 모두 서버 좌표를 약 50ms에 걸쳐 표시한다.
- 첫 수신 또는 새 epoch는 즉시 적용한다. 새 epoch에서는 낮아진 tick도 유효하다.
- 같은 epoch에서 이전/중복 tick은 버린다. 패킷이 끊기면 마지막 위치에 정지한다.
- 위치 예측/입력 재실행은 하지 않는다.
- 수신 콜백은 임시 구조체를 큐에 보관하고 Application::Update가 게임 스레드에서 전달한다.
- 외형 생성 전 스냅샷은 객체당 최신 하나, 전체 최대 1024개, 최대 5초 보관한다.
- 맵 전환/퇴장/연결 종료 시 캐시를 정리하고 구 이동 코드의 좌표 덮어쓰기를 막는다.

구현: MovementStream.h, MovementPacketHandler.cpp, stbPlayerScript.cpp, stbOtherPlayer.cpp, Monster.cpp.

## 5. 모션

| 상태 | 표시 |
|---|---|
| mode 0 Grounded | vx에 따라 대기/걷기, 낙하에서 바뀌면 착지 |
| mode 1 Rising | jump |
| mode 2 Falling | fall |
| mode 3 Climbing | 맵 kind에 따라 rope/ladder, vy=0이면 애니메이션 정지 |
| 플레이어 lifeState 5 | 죽음, 입력 차단 |
| 몬스터 lifeState 4/7 | 죽는 중/죽음 |

플레이어와 몬스터 lifeState는 별도 enum으로 해석하며 기존 PlayerState로 숫자를 캐스팅하지 않는다. 죽음이 최우선이다. 탑승 중 공격을 차단하고 공중 공격/채팅 중에도 위치 갱신을 유지한다.

구현: MovementVisual.h, stbAnimator.h/.cpp, 캐릭터 애니메이션 JSON. 기존 jump/rope/ladder PNG를 재사용한다. 전용 fall/land 아트가 없으면 기존 프레임을 대체 사용하며 별도 아트 제작이 필요하다.

## 6. 생명주기

- 기존 사망 이벤트는 캐릭터 ID를 확인한다. 스냅샷으로만 죽음이 와도 입력 차단/모달을 처리한다.
- 부활은 새 epoch의 살아 있는 스냅샷으로 재개한다. 부활 ok가 뒤늦게 와도 다시 대기로 돌리지 않는다.
- 같은 맵 포탈도 이전 epoch를 기억하고 새 epoch만 기다린다. 전환 실패는 기존 이동을 복원한다.
- 맵 준비 전에 온 목적지 스냅샷은 제한된 큐에 보관하고 준비 후 적용한다.
- 추락 복귀는 서버의 새 epoch 위치를 따른다. 자체 순간이동하지 않는다.
- 재접속 시 epoch/tick/입력/보간 기록을 모두 버린다.

## 7. 검증과 작업 기록

현재: 패킷/맵/보간 기반 작성, 객체 연결 진행 중. 중간 커밋이며 전체 빌드 완료를 의미하지 않는다. 완료 후 실제 검증 결과를 이 절에 기록한다.

1. 6/16필드 파싱, 잘못된 패킷의 부분 갱신 방지, uint64 tick.
2. 첫 스냅샷/새 epoch 즉시 이동, 옛 epoch/tick 거부, 보간 정지.
3. 키 해제·점프 1회·100ms heartbeat·포커스 차단.
4. 사망/부활 스냅샷과 ok 순서, 같은 맵 이동, 생성 전 스냅샷.
5. Debug x64 빌드 및 패킷 테스트.
6. 실서버에 클라이언트 두 개를 연결해 발판·줄·모션·포탈·재접속 확인.

실서버 접속 정보와 배포 버전은 확인되지 않았다. 자동 검사와 실제 두 클라이언트 연동 결과는 구분해서 기록한다.
