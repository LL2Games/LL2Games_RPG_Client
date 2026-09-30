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

현재: 1~6절의 클라이언트 구현과 객체 연결 완료. Debug x64 솔루션 빌드 성공, 기존 패킷 검사 7개와 이동 검사 6개(총 13개 그룹) 통과. 실서버 연동 검증은 미완료다.

1. 6/16필드 파싱, 잘못된 패킷의 부분 갱신 방지, uint64 tick.
2. 첫 스냅샷/새 epoch 즉시 이동, 옛 epoch/tick 거부, 보간 정지.
3. 키 해제·점프 1회·100ms heartbeat·포커스 차단.
4. 사망/부활 스냅샷과 ok 순서, 같은 맵 이동, 생성 전 스냅샷.
5. Debug x64 빌드 및 패킷 테스트.
6. 실서버에 클라이언트 두 개를 연결해 발판·줄·모션·포탈·재접속 확인.

실서버 접속 정보와 배포 버전은 확인되지 않았다. 자동 검사와 실제 두 클라이언트 연동 결과는 구분해서 기록한다.

## 8. 실제 구현 코드 읽기

위 1~6절의 파일 순서로 소스를 읽으면 된다. 아래 변경본은 구현 전 상태(중간 커밋 0d2937f의 부모)와 현재 작업 트리를 비교한 코드다. `+`는 추가, `-`는 삭제이며, 실제 프로젝트에는 이미 적용되어 있다. 다시 복사해서 중복 적용하지 않는다.

접기 안에는 패킷·맵·입력·모션·생명주기·프로젝트 등록 변경을 수록했다. 읽기 편하도록 공백을 정리한 참고 자료이므로 패치 명령에 직접 넣지 않는다. 문서 작성 이후 수정 시에는 실제 소스 파일을 우선한다.

<!-- IMPLEMENTATION_REFERENCE -->

<details>
<summary>Source changes</summary>

```diff
diff --git a/LL2_Client_PacketTests/LL2_Client_PacketTests.vcxproj b/LL2_Client_PacketTests/LL2_Client_PacketTests.vcxproj
index 281a200..f84d064 100644
--- a/LL2_Client_PacketTests/LL2_Client_PacketTests.vcxproj
+++ b/LL2_Client_PacketTests/LL2_Client_PacketTests.vcxproj
@@ -102,7 +102,7 @@
       <SDLCheck>true</SDLCheck>
       <PreprocessorDefinitions>_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
       <ConformanceMode>true</ConformanceMode>
-      <AdditionalIncludeDirectories>$(SolutionDir)LL2_Client_Win_Source;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
+      <AdditionalIncludeDirectories>$(SolutionDir)LL2_Client_Win_Source;$(SolutionDir)include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
       <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>
       <LanguageStandard>stdcpp20</LanguageStandard>
       <PrecompiledHeader>NotUsing</PrecompiledHeader>
@@ -129,6 +129,7 @@
   </ItemDefinitionGroup>
   <ItemGroup>
     <ClCompile Include="PacketParserTest.cpp" />
+    <ClCompile Include="MovementTests.cpp" />
   </ItemGroup>
   <ItemGroup>
     <ProjectReference Include="..\LL2_Client_Win_lib\LL2_Client_Win_lib.vcxproj">
@@ -138,4 +139,4 @@
   <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
   <ImportGroup Label="ExtensionTargets">
   </ImportGroup>
-</Project>
\ No newline at end of file
+</Project>
diff --git a/LL2_Client_PacketTests/PacketParserTest.cpp b/LL2_Client_PacketTests/PacketParserTest.cpp
index 3fe1b65..a442ef4 100644
--- a/LL2_Client_PacketTests/PacketParserTest.cpp
+++ b/LL2_Client_PacketTests/PacketParserTest.cpp
@@ -9,6 +9,7 @@
 #include <vector>

 #include "PacketParser.h"
+int RunMovementTests();

 namespace
 {
@@ -292,7 +293,7 @@ int main()
         },
     };

-    int failureCount = 0;
+    int failureCount = RunMovementTests();

     for (const TestCase& test : tests)
     {
diff --git a/LL2_Client_Win/Data/Character/Warrior/One_Hand/player_warrior_onehand_sword.json.json b/LL2_Client_Win/Data/Character/Warrior/One_Hand/player_warrior_onehand_sword.json.json
index e0ba28e..0128b67 100644
--- a/LL2_Client_Win/Data/Character/Warrior/One_Hand/player_warrior_onehand_sword.json.json
+++ b/LL2_Client_Win/Data/Character/Warrior/One_Hand/player_warrior_onehand_sword.json.json
@@ -52,13 +52,13 @@
       "path": "Character/Warrior/onehand_sword/rope",
       "frame_prefix": "rope_",
       "frame_count": 3,
-      "delay_ms": 10
+      "delay_ms": 0.12
     },
     "ladder": {
       "path": "Character/Warrior/onehand_sword/ladder",
       "frame_prefix": "ladder_",
       "frame_count": 3,
-      "delay_ms": 10
+      "delay_ms": 0.12
     },
     "swingO3": {
       "path": "Character/Warrior/onehand_sword/swingO3",
@@ -95,6 +95,18 @@
       "frame_prefix": "dead_",
       "frame_count": 1,
       "delay_ms": 1.0
+    },
+    "fall": {
+      "path": "Character/Warrior/onehand_sword/jump",
+      "frame_prefix": "jump_",
+      "frame_count": 2,
+      "delay_ms": 0.1
+    },
+    "land": {
+      "path": "Character/Warrior/onehand_sword/stand",
+      "frame_prefix": "stand1_",
+      "frame_count": 1,
+      "delay_ms": 0.1
     }
   }
 }
diff --git a/LL2_Client_Win/Data/Maps/100000000.json b/LL2_Client_Win/Data/Maps/100000000.json
index 5410f7f..9d9b26a 100644
--- a/LL2_Client_Win/Data/Maps/100000000.json
+++ b/LL2_Client_Win/Data/Maps/100000000.json
@@ -3,59 +3,99 @@
   "name": "Forest Ground 1",
   "background": "Map/Forest/ground_1.png",
   "miniMap": "Map/Forest/ground_1_minimap.png",
-
   "portals": [
     {
       "id": "to_town_pending",
       "texture": "ForestPortal",
-
       "position": {
         "x": 100.0,
         "y": 60.0
       },
-
       "renderSize": {
         "x": 100.0,
         "y": 160.0
       },
-
       "halfSize": {
         "x": 60.0,
         "y": 100.0
       },
-
-      "destinationMapId": 0,
-
+      "destinationMapId": 100000001,
       "spawnPosition": {
         "x": 100.0,
         "y": 500.0
-      }
+      },
+      "interactionRange": 120.0
     },
     {
       "id": "to_map_100000001",
       "texture": "ForestPortal",
-
       "position": {
         "x": 1400.0,
-        "y": 760.0
+        "y": 770.0
       },
-
       "renderSize": {
         "x": 100.0,
         "y": 160.0
       },
-
       "halfSize": {
         "x": 60.0,
         "y": 100.0
       },
-
       "destinationMapId": 100000001,
-
       "spawnPosition": {
-        "x": 1400.0,
-        "y": 760.0
-      }
+        "x": 120.0,
+        "y": 730.0
+      },
+      "interactionRange": 120.0
     }
-  ]
-}
\ No newline at end of file
+  ],
+  "physics": {
+    "minX": 0,
+    "maxX": 1600,
+    "killY": 1200,
+    "safeFeet": {
+      "x": 100,
+      "y": 700
+    },
+    "platforms": [
+      {
+        "id": 1,
+        "left": 0,
+        "right": 1600,
+        "y": 700
+      },
+      {
+        "id": 2,
+        "left": 300,
+        "right": 650,
+        "y": 500
+      },
+      {
+        "id": 3,
+        "left": 850,
+        "right": 1200,
+        "y": 350
+      }
+    ],
+    "climbables": [
+      {
+        "id": 1,
+        "kind": "ladder",
+        "x": 450,
+        "top": 500,
+        "bottom": 700,
+        "grabRange": 16,
+        "topPlatformId": 2
+      },
+      {
+        "id": 2,
+        "kind": "rope",
+        "x": 1000,
+        "top": 350,
+        "bottom": 700,
+        "grabRange": 16,
+        "topPlatformId": 3
+      }
+    ]
+  }
+}
diff --git a/LL2_Client_Win/Data/Maps/100000001.json b/LL2_Client_Win/Data/Maps/100000001.json
index 231fec5..2b146aa 100644
--- a/LL2_Client_Win/Data/Maps/100000001.json
+++ b/LL2_Client_Win/Data/Maps/100000001.json
@@ -3,59 +3,99 @@
   "name": "Forest Ground 2",
   "background": "Map/Forest/ground_2.png",
   "miniMap": "Map/Forest/ground_2_minimap.png",
-
   "portals": [
     {
       "id": "to_map_100000000",
       "texture": "ForestPortal",
-
       "position": {
         "x": 120.0,
-        "y": 700.0
+        "y": 720.0
       },
-
       "renderSize": {
         "x": 100.0,
         "y": 100.0
       },
-
       "halfSize": {
         "x": 60.0,
         "y": 100.0
       },
-
       "destinationMapId": 100000000,
-
       "spawnPosition": {
         "x": 1400.0,
-        "y": 760.0
-      }
+        "y": 800.0
+      },
+      "interactionRange": 120.0
     },
     {
       "id": "to_map_100000002",
       "texture": "ForestPortal",
-
       "position": {
         "x": 1400.0,
         "y": 700.0
       },
-
       "renderSize": {
         "x": 100.0,
         "y": 100.0
       },
-
       "halfSize": {
         "x": 60.0,
         "y": 100.0
       },
-
       "destinationMapId": 100000002,
-
       "spawnPosition": {
-        "x": 1000.0,
-        "y": 800.0
-      }
+        "x": 100.0,
+        "y": 850.0
+      },
+      "interactionRange": 120.0
     }
-  ]
-}
\ No newline at end of file
+  ],
+  "physics": {
+    "minX": 0,
+    "maxX": 1600,
+    "killY": 1200,
+    "safeFeet": {
+      "x": 100,
+      "y": 700
+    },
+    "platforms": [
+      {
+        "id": 1,
+        "left": 0,
+        "right": 1600,
+        "y": 700
+      },
+      {
+        "id": 2,
+        "left": 300,
+        "right": 650,
+        "y": 500
+      },
+      {
+        "id": 3,
+        "left": 850,
+        "right": 1200,
+        "y": 350
+      }
+    ],
+    "climbables": [
+      {
+        "id": 1,
+        "kind": "ladder",
+        "x": 450,
+        "top": 500,
+        "bottom": 700,
+        "grabRange": 16,
+        "topPlatformId": 2
+      },
+      {
+        "id": 2,
+        "kind": "rope",
+        "x": 1000,
+        "top": 350,
+        "bottom": 700,
+        "grabRange": 16,
+        "topPlatformId": 3
+      }
+    ]
+  }
+}
diff --git a/LL2_Client_Win/Data/Maps/100000002.json b/LL2_Client_Win/Data/Maps/100000002.json
index 12f7125..460275c 100644
--- a/LL2_Client_Win/Data/Maps/100000002.json
+++ b/LL2_Client_Win/Data/Maps/100000002.json
@@ -3,59 +3,99 @@
   "name": "Forest Ground 3",
   "background": "Map/Forest/ground_3.png",
   "miniMap": "Map/Forest/ground_3_minimap.png",
-
   "portals": [
     {
       "id": "to_map_100000001",
       "texture": "ForestPortal",
-
       "position": {
         "x": 100.0,
         "y": 790.0
       },
-
       "renderSize": {
         "x": 100.0,
         "y": 160.0
       },
-
       "halfSize": {
         "x": 60.0,
         "y": 100.0
       },
-
       "destinationMapId": 100000001,
-
       "spawnPosition": {
         "x": 1400.0,
-        "y": 520.0
-      }
+        "y": 740.0
+      },
+      "interactionRange": 120.0
     },
     {
       "id": "to_map_100000003",
       "texture": "ForestPortal",
-
       "position": {
         "x": 1400.0,
-        "y": 770.0
+        "y": 700.0
       },
-
       "renderSize": {
         "x": 100.0,
         "y": 160.0
       },
-
       "halfSize": {
         "x": 60.0,
         "y": 100.0
       },
-
-      "destinationMapId": 100000003,
-
+      "destinationMapId": 100000002,
       "spawnPosition": {
         "x": 1000.0,
         "y": 520.0
-      }
+      },
+      "interactionRange": 120.0
     }
-  ]
-}
\ No newline at end of file
+  ],
+  "physics": {
+    "minX": 0,
+    "maxX": 1600,
+    "killY": 1200,
+    "safeFeet": {
+      "x": 100,
+      "y": 700
+    },
+    "platforms": [
+      {
+        "id": 1,
+        "left": 0,
+        "right": 1600,
+        "y": 700
+      },
+      {
+        "id": 2,
+        "left": 300,
+        "right": 650,
+        "y": 500
+      },
+      {
+        "id": 3,
+        "left": 850,
+        "right": 1200,
+        "y": 350
+      }
+    ],
+    "climbables": [
+      {
+        "id": 1,
+        "kind": "ladder",
+        "x": 450,
+        "top": 500,
+        "bottom": 700,
+        "grabRange": 16,
+        "topPlatformId": 2
+      },
+      {
+        "id": 2,
+        "kind": "rope",
+        "x": 1000,
+        "top": 350,
+        "bottom": 700,
+        "grabRange": 16,
+        "topPlatformId": 3
+      }
+    ]
+  }
+}
diff --git a/LL2_Client_Win/LL2_Client_Win.cpp b/LL2_Client_Win/LL2_Client_Win.cpp
index 3233f6d..c088e01 100644
--- a/LL2_Client_Win/LL2_Client_Win.cpp
+++ b/LL2_Client_Win/LL2_Client_Win.cpp
@@ -371,6 +371,14 @@ LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
     switch (message)
     {

+    case WM_ACTIVATEAPP:
+        if (!wParam)
+        {
+            if (auto* player = PlayerManager::getInstance()->GetLocalPlayer())
+                player->GetMovementScript()->StopMovementInput();
+        }
+        break;
+
     case WM_SHOW_REVIVE:
 {
     auto* player = PlayerManager::getInstance()->GetLocalPlayer();
diff --git a/LL2_Client_Win_Source/ChannelInitPacketHandler.cpp b/LL2_Client_Win_Source/ChannelInitPacketHandler.cpp
index 0ea7d6e..ed3a1fc 100644
--- a/LL2_Client_Win_Source/ChannelInitPacketHandler.cpp
+++ b/LL2_Client_Win_Source/ChannelInitPacketHandler.cpp
@@ -2,6 +2,7 @@
 #include "stbNetworkManager.h"
 #include "stbNetworkConfig.h"
 #include "Packet.h"
+#include "MovementPacketHandler.h"
 #include <sstream>
 #include <algorithm>
 // 태스트를 위해서 임시로 추가
@@ -54,6 +55,7 @@ void ChannelInitPacketHandler::Execute(const ParsedPacket& pkt)
             return;
         }

+        MovementPacketHandler::ResetConnection();
         stb::NetworkConfig::SetCharacterName(name);
         // 채널 인증 성공 후 맵 입장 패킷 전송
         OutputDebugStringA("채널 인증 완료! 맵 입장 패킷 전송...\n");
diff --git a/LL2_Client_Win_Source/CombatSystem.cpp b/LL2_Client_Win_Source/CombatSystem.cpp
index 1c38d71..8e83d9e 100644
--- a/LL2_Client_Win_Source/CombatSystem.cpp
+++ b/LL2_Client_Win_Source/CombatSystem.cpp
@@ -71,7 +71,7 @@ bool CombatSystem::CanUseSkill(int skillId)
         return false;
     }

-    if (m_player->IsDead())
+    if (m_player->IsDead() || !m_player->GetMovementScript()->CanAttack())
     {
         m_debugMsg = "player is Dead \n";
         OutputDebugStringA(m_debugMsg.c_str());
@@ -124,6 +124,8 @@ bool CombatSystem::CanBasicAttack()
 {
     if (m_player == nullptr)
         return false;
+    if (!m_player->GetMovementScript()->CanAttack())
+        return false;

     PlayerState state = m_player->GetState();

diff --git a/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems b/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems
index 2c8be24..d998c8c 100644
--- a/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems
+++ b/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems
@@ -1,4 +1,4 @@
-﻿<?xml version="1.0" encoding="utf-8"?>
+<?xml version="1.0" encoding="utf-8"?>
 <Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
   <PropertyGroup Label="Globals">
     <MSBuildAllProjects Condition="'$(MSBuildVersion)' == '' Or '$(MSBuildVersion)' &lt; '16.0'">$(MSBuildAllProjects);$(MSBuildThisFileFullPath)</MSBuildAllProjects>
@@ -219,4 +219,15 @@
     <ClCompile Include="$(MSBuildThisFileDirectory)UIManager.cpp" />
     <ClCompile Include="$(MSBuildThisFileDirectory)VFXDataManager.cpp" />
   </ItemGroup>
+  <ItemGroup>
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementTypes.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementProtocol.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementStream.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementMap.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementVisual.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementDebug.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementPacketHandler.h" />
+    <ClCompile Include="$(MSBuildThisFileDirectory)MovementProtocol.cpp" />
+    <ClCompile Include="$(MSBuildThisFileDirectory)MovementPacketHandler.cpp" />
+  </ItemGroup>
 </Project>
\ No newline at end of file
diff --git a/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems.filters b/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems.filters
index d2b1839..24ac9e1 100644
--- a/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems.filters
+++ b/LL2_Client_Win_Source/LL2_Client_Win_Source.vcxitems.filters
@@ -1,4 +1,4 @@
-﻿<?xml version="1.0" encoding="utf-8"?>
+<?xml version="1.0" encoding="utf-8"?>
 <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
   <ItemGroup>
     <ClCompile Include="$(MSBuildThisFileDirectory)MySocket.cpp" />
@@ -715,4 +715,15 @@
       <UniqueIdentifier>{425ca5cd-c3de-4dd9-839b-fe02ea6b07e3}</UniqueIdentifier>
     </Filter>
   </ItemGroup>
+  <ItemGroup>
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementTypes.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementProtocol.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementStream.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementMap.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementVisual.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementDebug.h" />
+    <ClInclude Include="$(MSBuildThisFileDirectory)MovementPacketHandler.h" />
+    <ClCompile Include="$(MSBuildThisFileDirectory)MovementProtocol.cpp" />
+    <ClCompile Include="$(MSBuildThisFileDirectory)MovementPacketHandler.cpp" />
+  </ItemGroup>
 </Project>
\ No newline at end of file
diff --git a/LL2_Client_Win_Source/Monster.cpp b/LL2_Client_Win_Source/Monster.cpp
index 0aef421..6de4f5b 100644
--- a/LL2_Client_Win_Source/Monster.cpp
+++ b/LL2_Client_Win_Source/Monster.cpp
@@ -1,6 +1,8 @@
-﻿#include "Monster.h"
+﻿#include "MovementDebug.h"
+#include "Monster.h"
 #include "stbResourceManager.h"
 #include "MonsterDataManager.h"
+#include "MapDataManager.h"
 #include "stbTexture.h"
 #include "Util.h"
 #include "stbLogger.h"
@@ -39,30 +41,58 @@ void Monster::InitFromSpawn(const MonsterSpawnInfo& info)

 void Monster::Update(float deltaTime)
 {
-   GameObject::Update();
-
-   if (m_state == MonsterState::E_Die)
-       return;
-
-   stb::math::Vector2 diff = m_targetPos - m_pos;
-   float dist = diff.length();
-
-   //DebugMsg = "dist : " + std::to_string(dist) + "\n";
-   //OutputDebugStringA(DebugMsg.c_str());
-
-   if (dist > 1.0f)
-   {
-       float correctionSpeed = static_cast<float>(m_moveSpeed);
-       //float correctionSpeed = m_moveSpeed * 2.0f;
-       float moveDist = correctionSpeed * deltaTime;
+    GameObject::Update();
+    movement::Snapshot displayed;
+    if (!m_movement.Update(deltaTime, displayed)) return;
+    m_pos = {displayed.position.x, displayed.position.y};
+    m_transform->SetPosition(m_pos);
+    m_dir = displayed.facing;
+    const auto* map = MapDataManager::getInstance()->FindMapData(displayed.mapId);
+    const auto* climb = map ? map->physics.FindClimbable(displayed.climbableId) : nullptr;
+    m_movementVisual.Update(m_animator, displayed, deltaTime, climb && climb->ladder,
+        m_state == MonsterState::E_Hit || m_state == MonsterState::E_Die);
+}

-       if (moveDist >= dist)
-           m_pos = m_targetPos;
-       else
-           m_pos += diff.normalize() * moveDist;
+void Monster::ApplyMovementSnapshot(const movement::Snapshot& s)
+{
+    const bool reset = !m_movement.HasSnapshot() || s.epoch != m_movement.Latest().epoch;
+    const int oldLife = m_movement.HasSnapshot() ? m_movement.Latest().lifeState : -1;
+    if (!m_movement.Push(s)) return;
+    m_curHp = s.hp; m_maxHp = s.maxHp;
+    if (reset)
+    {
+        m_movementVisual.Reset();
+        m_pos = m_targetPos = {s.position.x, s.position.y};
+        m_transform->SetPosition(m_pos);
+        m_isDead = false; m_isDeathAnimationFinished = false;
+        m_state = MonsterState::E_NONE;
+    }
+    if (s.lifeState == static_cast<int>(movement::MonsterLife::Dead))
+    {
+        SetState(MonsterState::E_Die);
+        m_isDead = true; m_isDeathAnimationFinished = true;
+    }
+    else if (movement::IsDead(s)) SetState(MonsterState::E_Die);
+    else if (reset || oldLife != s.lifeState)
+    {
+        switch (static_cast<movement::MonsterLife>(s.lifeState))
+        {
+        case movement::MonsterLife::Hit: SetState(MonsterState::E_Hit); break;
+        case movement::MonsterLife::Patrol: SetState(MonsterState::E_Patrol); break;
+        case movement::MonsterLife::Chase: SetState(MonsterState::E_Chase); break;
+        case movement::MonsterLife::Move: SetState(MonsterState::E_Move); break;
+        default: SetState(MonsterState::E_Idle); break;
+        }
+    }
+}

-       m_transform->SetPosition(m_pos);
-   }
+float Monster::GetFootOffset() const
+{
+    const auto* data = M_MONSTERDATAMANAGER->FindMonsterData(m_monsterId);
+    if (!data) return 0;
+    return data->colliderInfo.offset.y +
+        (data->colliderInfo.colliderType == stb::enums::eColliderType::Circle2D
+            ? data->colliderInfo.radius : data->colliderInfo.halfSize.y);
 }

 void Monster::Render(stbD2DRenderer& renderer)
@@ -72,6 +102,7 @@ void Monster::Render(stbD2DRenderer& renderer)


    GameObject::Render(renderer);
+        if (m_transform) movement::DrawOriginAndFeet(renderer, m_transform->GetPosition(), GetFootOffset());
    //m_collider->Render(renderer);
 }

@@ -81,6 +112,7 @@ void Monster::SetState(MonsterState state)
        return;

    m_state = state;
+    if (m_animator) m_animator->SetPaused(false);

    bool isLoop = true;

@@ -226,6 +258,7 @@ void Monster::OnMove(float /*x*/, float /*y*/, int /*dir*/)
 // 몬스터패킷 핸들러에서 바로 호출하는 함수
 void Monster::ApplyServerUpdate(const MonsterUpdateInfo& info)
 {
+    if (m_movement.HasSnapshot()) return;
    m_targetPos = info.pos;
    m_dir = info.dir;

@@ -261,6 +294,10 @@ void Monster::ApplyAttackResult(const AttackResult& result)

 void Monster::RespawnFromServer(const MonsterUpdateInfo& info)
 {
+    // A new live snapshot may arrive before the legacy respawn notification.
+    if (m_movement.HasSnapshot() && !movement::IsDead(m_movement.Latest())) return;
+    m_movement.Suspend();
+    m_movementVisual.Reset();
    OutputDebugStringA("[RESPAWN] RespawnFromServer called\n");

    m_isDeathAnimationFinished = false;
@@ -285,6 +322,7 @@ void Monster::RespawnFromServer(const MonsterUpdateInfo& info)

 void Monster::ResetFromSpawnInfo(const MonsterSpawnInfo& info)
 {
+    if (m_movement.HasSnapshot()) return;

    m_pos = info.pos;

diff --git a/LL2_Client_Win_Source/Monster.h b/LL2_Client_Win_Source/Monster.h
index 06f17ee..ce4ec60 100644
--- a/LL2_Client_Win_Source/Monster.h
+++ b/LL2_Client_Win_Source/Monster.h
@@ -9,6 +9,8 @@
 #include "stbCircleCollider2D.h"
 #include "MonsterScript.h"
 #include "CombatSystem_Info.h"
+#include "MovementStream.h"
+#include "MovementVisual.h"

 class stbD2DRenderer;

@@ -30,6 +32,10 @@ public:

     void ResetFromSpawnInfo(const MonsterSpawnInfo& info);
     void ApplyServerUpdate(const MonsterUpdateInfo& info);
+    void ApplyMovementSnapshot(const movement::Snapshot& snapshot);
+    bool HasMovementSnapshot() const { return m_movement.HasSnapshot(); }
+    void ResetMovementConnection() { m_movement.Reset(); m_movementVisual.Reset(); }
+    float GetFootOffset() const;
     void ApplyAttackResult(const AttackResult& result);
     void RespawnFromServer(const MonsterUpdateInfo& info);
 public:
@@ -45,6 +51,8 @@ private:

     stb::math::Vector2 m_pos{};
     stb::math::Vector2 m_targetPos{};     // 서버 이동 패킷 받은 위치
+    movement::Stream m_movement;
+    movement::Visual m_movementVisual;
     int m_dir = 1;

     int m_curHp = 0;
diff --git a/LL2_Client_Win_Source/MonsterDataManager.cpp b/LL2_Client_Win_Source/MonsterDataManager.cpp
index 37c4263..682c868 100644
--- a/LL2_Client_Win_Source/MonsterDataManager.cpp
+++ b/LL2_Client_Win_Source/MonsterDataManager.cpp
@@ -113,9 +113,18 @@ bool MonsterDataManager::LoadJsonFile(const std::string& path, MonsterData& mons
     monsterData.colliderInfo.offset.x = offset.value("x", 0.0f);
     monsterData.colliderInfo.offset.y = offset.value("y", 0.0f);

-    const auto& half = collider.at("half");
-    monsterData.colliderInfo.halfSize.x = half.value("w", 0.0f);
-    monsterData.colliderInfo.halfSize.y = half.value("h", 0.0f);
+    if (monsterData.colliderInfo.colliderType == stb::enums::eColliderType::Circle2D)
+    {
+        const float legacyRadius = collider.contains("half") ? collider.at("half").value("w", 0.0f) : 0.0f;
+        monsterData.colliderInfo.radius = collider.value("radius", legacyRadius);
+        monsterData.colliderInfo.halfSize = {monsterData.colliderInfo.radius, monsterData.colliderInfo.radius};
+    }
+    else
+    {
+        const auto& half = collider.at("half");
+        monsterData.colliderInfo.halfSize.x = half.value("w", 0.0f);
+        monsterData.colliderInfo.halfSize.y = half.value("h", 0.0f);
+    }

     const auto& ui = j.at("ui").at("hp_bar_offset");
     monsterData.UIPos.x = ui.value("x", 0.0f);
diff --git a/LL2_Client_Win_Source/MonsterManager.cpp b/LL2_Client_Win_Source/MonsterManager.cpp
index 990c32b..70b86e6 100644
--- a/LL2_Client_Win_Source/MonsterManager.cpp
+++ b/LL2_Client_Win_Source/MonsterManager.cpp
@@ -144,3 +144,8 @@ void MonsterManager::Clear()
     OutputDebugStringA("[MonsterManager] Clear\n");
 }

+
+void MonsterManager::ResetMovementConnections()
+{
+    for (auto& entry : m_monsters) entry.second->ResetMovementConnection();
+}
diff --git a/LL2_Client_Win_Source/MonsterManager.h b/LL2_Client_Win_Source/MonsterManager.h
index 308cf0f..9e7fd14 100644
--- a/LL2_Client_Win_Source/MonsterManager.h
+++ b/LL2_Client_Win_Source/MonsterManager.h
@@ -23,6 +23,7 @@ public:
     Monster* FindMonster(int instanceId);
     //void ApplyMonsterDamage(const MonsterHitInfo& info);
     void Clear();
+    void ResetMovementConnections();

 private:
     std::unordered_map<int, std::unique_ptr<Monster>> m_monsters;
diff --git a/LL2_Client_Win_Source/MovePacketHandler.cpp b/LL2_Client_Win_Source/MovePacketHandler.cpp
index 7370733..0dd051d 100644
--- a/LL2_Client_Win_Source/MovePacketHandler.cpp
+++ b/LL2_Client_Win_Source/MovePacketHandler.cpp
@@ -1,196 +1,8 @@
 ﻿#include "MovePacketHandler.h"
-#include "stbNetworkConfig.h"
-#include "stbOtherPlayerManager.h"
-#include "PacketParser.h"
-#include "StringConvert.h"
-#include "PacketData.h"
-#include "stbNetworkManager.h"
-#include "stbTransform.h"
-#include "stbLogger.h"
-#include "PlayerManager.h"
+#include "MovementPacketHandler.h"

-#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
-
-
-/*
-struct ParsedPacket
-{
-    uint16_t type;
-    std::string payload;
-};
-*/
-
-
-void MovePacketHandler::Execute(const ParsedPacket& pkt)
+// Legacy 0x0020 responses are diagnostic only; they must never move an entity.
+void MovePacketHandler::Execute(const ParsedPacket& packet)
 {
-    try
-    {
-        std::size_t offset = 0;
-        const std::size_t payloadSize = pkt.payload.size();
-        const char* data = pkt.payload.c_str();
-
-        std::string firstField;
-        std::string errMsg;
-
-        if (payloadSize < sizeof(std::uint16_t))
-        {
-            return;
-        }
-
-        // 첫 필드는 ok, nok 또는 이동한 플레이어 ID다.
-        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, firstField, errMsg))
-        {
-            M_LOGGER("이동 패킷 첫 필드 파싱 실패: %s",errMsg.c_str());
-            return;
-        }
-
-        std::string debugMessage ="[PKT_PLAYER_MOVE 수신] firstField=[" + firstField +"]\n";
-        OutputDebugStringA(debugMessage.c_str());
-
-        // 서버가 보낸 정상 이동 응답
-        if (firstField == "ok")
-        {
-            return;
-        }
-
-        // 서버가 이동을 거부한 경우 서버 좌표로 보정
-        if (firstField == "nok")
-        {
-            OutputDebugStringW(L"[이동] 서버 이동 거부 응답 처리 시작\n");
-            std::string reason;
-            float serverX = 0.0F;
-            float serverY = 0.0F;
-
-            if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, reason, errMsg))
-            {
-                OutputDebugStringA("[MOVE] failed to parse reason\n");
-                return;
-            }
-
-            if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, serverX, errMsg) || !PacketParser::ParseNextFloatField(data, payloadSize, offset, serverY, errMsg))
-            {
-                OutputDebugStringA("[MOVE] failed to parse server position\n");
-                return;
-            }
-
-            wchar_t positionLog[256]{};
-            swprintf_s(positionLog,L"[이동] 서버 좌표 파싱 완료 X=%.3f Y=%.3f\n",serverX,serverY);
-            OutputDebugStringW(positionLog);
-
-            stb::Player* localPlayer = M_PLAYERMANAGER->GetLocalPlayer();
-
-            if (localPlayer == nullptr)
-            {
-                OutputDebugStringA("[MOVE] local player is null\n");
-                return;
-            }
-
-            stb::Transform* transform =
-                localPlayer->GetComponent<stb::Transform>();
-
-            if (transform == nullptr)
-            {
-                OutputDebugStringA("[MOVE] local transform is null\n");
-                return;
-            }
-
-            const stb::math::Vector2 serverPosition{serverX,serverY};
-
-            transform->SetPosition(serverPosition);
-
-            if (localPlayer->GetPlayerLocation() != nullptr)
-            {
-                localPlayer->GetPlayerLocation()->pos = serverPosition;
-            }
-
-            OutputDebugStringW(L"[이동] 서버 기준 좌표로 위치 보정 완료\n");
-
-            return;
-        }
-
-        // ok/nok가 아니면 다른 플레이어의 ID다.
-        const std::string& playerId = firstField;
-
-        if (playerId == stb::NetworkConfig::GetCharacterId())
-        {
-            return;
-        }
-
-        int state = 0;
-        OtherPlayerMove otherPlayerMove{};
-
-        if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, otherPlayerMove.xPos, errMsg))
-        {
-            return;
-        }
-
-        if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, otherPlayerMove.yPos, errMsg))
-        {
-            return;
-        }
-
-        if (!PacketParser::ParseNextFloatField( data, payloadSize, offset, otherPlayerMove.speed, errMsg))
-        {
-            return;
-        }
-
-        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, otherPlayerMove.dir, errMsg))
-        {
-            return;
-        }
-
-        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, state, errMsg))
-        {
-            return;
-        }
-
-        otherPlayerMove.state = (state == 1) ? PlayerState::Walk : PlayerTypeUtil::IntToState(state);
-
-        otherPlayerMove.playerId = playerId;
-
-        auto otherPlayerManager = stb::OtherPlayerManager::getInstance();
-
-        if (otherPlayerManager == nullptr)
-        {
-            return;
-        }
-
-        otherPlayerManager->HandleMovePacket(otherPlayerMove);
-    }
-    catch (const std::exception& exception)
-    {
-        OutputDebugStringA(exception.what());
-
-    }
-    catch (...)
-    {
-        OutputDebugStringA("이동 패킷 처리 중 알 수 없는 예외 발생\n");
-    }
-}
-
-void MovePacketHandler::SendPlayerMove(stb::Player* player)
-{
-    if (player == nullptr)
-        return;
-
-    stb::Transform* tr = player->GetComponent<stb::Transform>();
-    if (tr == nullptr)
-        return;
-
-    stb::math::Vector2 pos = tr->GetPosition();
-    if (player->GetPlayerLocation() != nullptr)
-    {
-        player->GetPlayerLocation()->pos = pos;
-    }
-
-    std::vector<std::string> payload;
-
-    payload.push_back(std::to_string(pos.x));
-    payload.push_back(std::to_string(pos.y));
-    payload.push_back(std::to_string(player->GetPlayerMoveSpeed()));
-    payload.push_back(std::to_string(static_cast<int>(player->GetFacing())));
-    payload.push_back(std::to_string(static_cast<int>(player->GetState())));
-
-    stb::NetworkManager::getInstance()->SendPacket(PKT_PLAYER_MOVE, payload);
-    OutputDebugStringA("[PKT_PLAYER_MOVE 전송 완료]\n\n");
+    MovementPacketHandler::HandleInputResponse(packet);
 }
diff --git a/LL2_Client_Win_Source/MovePacketHandler.h b/LL2_Client_Win_Source/MovePacketHandler.h
index 5d0442c..5ec6b5b 100644
--- a/LL2_Client_Win_Source/MovePacketHandler.h
+++ b/LL2_Client_Win_Source/MovePacketHandler.h
@@ -1,14 +1,8 @@
 ﻿#pragma once
 #include "Packet.h"
-#include "CommonInclude.h"
-#include "..\\LL2_Client_Win_lib\\stbPlayer.h"

 class MovePacketHandler
 {
 public:
-   static void Execute(const ParsedPacket& pkt);
-   static void SendPlayerMove(stb::Player* player);
-
-private:
+    static void Execute(const ParsedPacket& packet);
 };
-
diff --git a/LL2_Client_Win_Source/MovementMap.h b/LL2_Client_Win_Source/MovementMap.h
new file mode 100644
index 0000000..37b6fdf
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementMap.h
@@ -0,0 +1,73 @@
+#pragma once
+#include "MovementTypes.h"
+#include <nlohmann/json.hpp>
+#include <cmath>
+#include <set>
+#include <stdexcept>
+#include <string>
+#include <vector>
+
+namespace movement
+{
+    struct Platform { int id = 0; float left = 0, right = 0, y = 0; };
+    struct Climbable
+    {
+        int id = 0;
+        bool ladder = false;
+        float x = 0, top = 0, bottom = 0, grabRange = 0;
+        int topPlatformId = 0;
+    };
+    struct MapGeometry
+    {
+        float minX = 0, maxX = 0, killY = 0;
+        Point safeFeet;
+        std::vector<Platform> platforms;
+        std::vector<Climbable> climbables;
+        const Climbable* FindClimbable(int id) const
+        {
+            for (const auto& c : climbables) if (c.id == id) return &c;
+            return nullptr;
+        }
+    };
+    inline MapGeometry ParseMapGeometry(const nlohmann::json& json)
+    {
+        MapGeometry result;
+        auto number = [](const nlohmann::json& object, const char* name)
+        {
+            float value = object.at(name).get<float>();
+            if (!std::isfinite(value)) throw std::runtime_error("non-finite map geometry");
+            return value;
+        };
+        result.minX = number(json, "minX"); result.maxX = number(json, "maxX");
+        result.killY = number(json, "killY");
+        result.safeFeet = {number(json.at("safeFeet"), "x"), number(json.at("safeFeet"), "y")};
+        if (result.minX >= result.maxX || result.safeFeet.x < result.minX ||
+            result.safeFeet.x > result.maxX || result.safeFeet.y >= result.killY)
+            throw std::runtime_error("invalid map bounds/safeFeet");
+        std::set<int> ids;
+        for (const auto& p : json.at("platforms"))
+        {
+            Platform platform{p.at("id").get<int>(), number(p, "left"), number(p, "right"), number(p, "y")};
+            if (platform.id <= 0 || !ids.insert(platform.id).second || platform.left >= platform.right)
+                throw std::runtime_error("invalid platform");
+            result.platforms.push_back(platform);
+        }
+        ids.clear();
+        for (const auto& c : json.at("climbables"))
+        {
+            const std::string kind = c.at("kind").get<std::string>();
+            Climbable climbable{c.at("id").get<int>(), kind == "ladder", number(c, "x"),
+                number(c, "top"), number(c, "bottom"), number(c, "grabRange"), c.at("topPlatformId").get<int>()};
+            if ((kind != "rope" && kind != "ladder") || climbable.id <= 0 ||
+                !ids.insert(climbable.id).second || climbable.top >= climbable.bottom || climbable.grabRange <= 0)
+                throw std::runtime_error("invalid climbable");
+            bool exitFound = false;
+            for (const auto& p : result.platforms)
+                if (p.id == climbable.topPlatformId && climbable.x >= p.left && climbable.x <= p.right &&
+                    std::abs(climbable.top - p.y) <= 0.05f) exitFound = true;
+            if (!exitFound) throw std::runtime_error("climbable top platform missing");
+            result.climbables.push_back(climbable); // Preserve server array order.
+        }
+        return result;
+    }
+}
diff --git a/LL2_Client_Win_Source/MovementPacketHandler.cpp b/LL2_Client_Win_Source/MovementPacketHandler.cpp
new file mode 100644
index 0000000..682cccb
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementPacketHandler.cpp
@@ -0,0 +1,160 @@
+#include "MovementPacketHandler.h"
+#include "MovementProtocol.h"
+#include "PacketParser.h"
+#include "stbNetworkManager.h"
+#include "PlayerManager.h"
+#include "stbPlayer.h"
+#include "stbOtherPlayerManager.h"
+#include "MonsterManager.h"
+#include <chrono>
+#include <map>
+#include <mutex>
+#include <tuple>
+
+namespace
+{
+    using Clock = std::chrono::steady_clock;
+    using Key = std::tuple<int, int, int>;
+    struct Pending { movement::Snapshot value; Clock::time_point received; };
+    std::mutex inboxMutex;
+    std::map<Key, Pending> inbox;
+    int activeMap = 0;
+    bool transitioning = false;
+    bool resetConnection = false;
+    constexpr std::size_t MaxPending = 1024;
+    void Queue(const Pending& pending)
+    {
+        const auto& s = pending.value;
+        std::lock_guard<std::mutex> lock(inboxMutex);
+        if (activeMap != 0 && !transitioning && s.mapId != activeMap) return;
+        const Key key{s.mapId, static_cast<int>(s.kind), s.entityId};
+        auto it = inbox.find(key);
+        if (it != inbox.end())
+        {
+            if (movement::IsNewer(s, it->second.value)) it->second = pending;
+        }
+        else if (inbox.size() < MaxPending) inbox.emplace(key, pending);
+    }
+}
+void MovementPacketHandler::Execute(const ParsedPacket& packet)
+{
+    movement::Snapshot snapshot;
+    std::string error;
+    if (!movement::ParseSnapshot(packet.payload, snapshot, error))
+    {
+        OutputDebugStringA(("[Movement rejected] " + error + "\n").c_str());
+        return;
+    }
+    Queue({snapshot, Clock::now()});
+}
+void MovementPacketHandler::HandleInputResponse(const ParsedPacket& packet)
+{
+    std::size_t offset = 0;
+    std::string status, reason, error;
+    if (!PacketParser::ParseLengthPrefixedString(packet.payload.data(), packet.payload.size(), offset, status, error)) return;
+    if (status == "nok")
+    {
+        PacketParser::ParseLengthPrefixedString(packet.payload.data(), packet.payload.size(), offset, reason, error);
+        OutputDebugStringA(("[Movement input rejected] " + reason + "\n").c_str());
+    }
+}
+bool MovementPacketHandler::SendInput(int mapId, int epoch, int sequence, const movement::Input& input)
+{
+    auto* network = stb::NetworkManager::getInstance();
+    std::vector<std::string> fields;
+    if (!network || !network->IsConnected() || !movement::BuildInputFields(mapId, epoch, sequence, input, fields)) return false;
+    network->SendPacket(PKT_MOVEMENT_INPUT, fields);
+#ifdef _DEBUG
+    OutputDebugStringA(("[Movement input] map=" + std::to_string(mapId) + " epoch=" + std::to_string(epoch) +
+        " sequence=" + std::to_string(sequence) + " axes=" + std::to_string(input.horizontal) + "," +
+        std::to_string(input.vertical) + " jump=" + (input.jump ? "1\n" : "0\n")).c_str());
+#endif
+    return true;
+}
+void MovementPacketHandler::Pump()
+{
+    std::map<Key, Pending> batch;
+    bool reset = false;
+    int mapId = 0;
+    {
+        std::lock_guard<std::mutex> lock(inboxMutex);
+        reset = resetConnection; resetConnection = false; mapId = activeMap;
+        if (!transitioning) batch.swap(inbox);
+    }
+    auto* player = PlayerManager::getInstance()->GetLocalPlayer();
+    if (reset && player) player->GetMovementScript()->ResetMovementConnection();
+    if (reset)
+    {
+        for (auto& entry : stb::OtherPlayerManager::getInstance()->GetPlayers())
+            if (entry.second) entry.second->ResetMovementConnection();
+        MonsterManager::getInstance()->ResetMovementConnections();
+    }
+    for (const auto& entry : batch)
+    {
+        const auto& pending = entry.second;
+        const auto& s = pending.value;
+        if (Clock::now() - pending.received > std::chrono::seconds(5)) continue;
+        if (mapId != 0 && s.mapId != mapId) continue;
+        if (!player || player->GetPlayerIdentity()->charId == 0 || mapId == 0)
+        {
+            Queue(pending); continue;
+        }
+        bool found = false;
+        if (s.kind == movement::Kind::Player)
+        {
+            if (s.entityId == player->GetPlayerIdentity()->charId)
+            {
+                player->GetMovementScript()->ApplyMovementSnapshot(s); found = true;
+            }
+            else
+            {
+                auto& players = stb::OtherPlayerManager::getInstance()->GetPlayers();
+                auto it = players.find(std::to_string(s.entityId));
+                if (it != players.end() && it->second)
+                {
+                    it->second->ApplyMovementSnapshot(s); found = true;
+                }
+            }
+        }
+        else if (auto* monster = MonsterManager::getInstance()->FindMonster(s.entityId))
+        {
+            monster->ApplyMovementSnapshot(s); found = true;
+        }
+        if (!found) Queue(pending);
+    }
+}
+void MovementPacketHandler::BeginMapTransition()
+{
+    if (auto* player = PlayerManager::getInstance()->GetLocalPlayer()) player->GetMovementScript()->SuspendMovement();
+    std::lock_guard<std::mutex> lock(inboxMutex);
+    inbox.clear(); transitioning = true;
+}
+void MovementPacketHandler::CommitMap(int mapId)
+{
+    std::lock_guard<std::mutex> lock(inboxMutex);
+    activeMap = mapId; transitioning = false;
+    for (auto it = inbox.begin(); it != inbox.end();)
+        if (it->second.value.mapId != mapId) it = inbox.erase(it);
+        else ++it;
+}
+void MovementPacketHandler::CancelMapTransition()
+{
+    if (auto* player = PlayerManager::getInstance()->GetLocalPlayer()) player->GetMovementScript()->CancelMovementSuspend();
+    std::lock_guard<std::mutex> lock(inboxMutex);
+    transitioning = false;
+    for (auto it = inbox.begin(); it != inbox.end();)
+        if (it->second.value.mapId != activeMap) it = inbox.erase(it);
+        else ++it;
+}
+void MovementPacketHandler::ResetConnection()
+{
+    std::lock_guard<std::mutex> lock(inboxMutex);
+    inbox.clear(); activeMap = 0; transitioning = false; resetConnection = true;
+}
+void MovementPacketHandler::Forget(movement::Kind kind, int entityId)
+{
+    std::lock_guard<std::mutex> lock(inboxMutex);
+    for (auto it = inbox.begin(); it != inbox.end();)
+        if (it->second.value.kind == kind && it->second.value.entityId == entityId) it = inbox.erase(it);
+        else ++it;
+}
diff --git a/LL2_Client_Win_Source/MovementPacketHandler.h b/LL2_Client_Win_Source/MovementPacketHandler.h
new file mode 100644
index 0000000..f0a0171
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementPacketHandler.h
@@ -0,0 +1,16 @@
+#pragma once
+#include "Packet.h"
+#include "MovementTypes.h"
+class MovementPacketHandler
+{
+public:
+    static void Execute(const ParsedPacket& packet);
+    static void HandleInputResponse(const ParsedPacket& packet);
+    static void Pump();
+    static bool SendInput(int mapId, int epoch, int sequence, const movement::Input& input);
+    static void BeginMapTransition();
+    static void CommitMap(int mapId);
+    static void CancelMapTransition();
+    static void ResetConnection();
+    static void Forget(movement::Kind kind, int entityId);
+};
diff --git a/LL2_Client_Win_Source/MovementProtocol.cpp b/LL2_Client_Win_Source/MovementProtocol.cpp
new file mode 100644
index 0000000..71cf2e8
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementProtocol.cpp
@@ -0,0 +1,92 @@
+#include "MovementProtocol.h"
+#include "PacketParser.h"
+#include <charconv>
+#include <cmath>
+#include <stdexcept>
+#include <string_view>
+
+namespace
+{
+    class Reader
+    {
+    public:
+        explicit Reader(const std::string& value) : payload(value) {}
+        std::string Field()
+        {
+            std::string field, error;
+            if (!PacketParser::ParseLengthPrefixedString(payload.data(), payload.size(), offset, field, error))
+                throw std::runtime_error(error);
+            return field;
+        }
+        template<class T> T Number()
+        {
+            const std::string field = Field();
+            T value{};
+            const auto result = std::from_chars(field.data(), field.data() + field.size(), value);
+            if (result.ec != std::errc{} || result.ptr != field.data() + field.size())
+                throw std::runtime_error("invalid or out-of-range movement number");
+            return value;
+        }
+        float Float()
+        {
+            float value = Number<float>();
+            if (!std::isfinite(value)) throw std::runtime_error("non-finite movement position/velocity");
+            return value;
+        }
+        bool Finished() const { return offset == payload.size(); }
+    private:
+        const std::string& payload;
+        std::size_t offset = 0;
+    };
+}
+
+bool movement::ParseSnapshot(const std::string& payload, Snapshot& result, std::string& error)
+{
+    try
+    {
+        Reader r(payload);
+        Snapshot s;
+        s.mapId = r.Number<int>();
+        const int kind = r.Number<int>();
+        s.entityId = r.Number<int>();
+        s.epoch = r.Number<int>();
+        s.tick = r.Number<std::uint64_t>();
+        s.sequence = r.Number<int>();
+        s.position = {r.Float(), r.Float()};
+        s.velocity = {r.Float(), r.Float()};
+        const int mode = r.Number<int>();
+        s.facing = r.Number<int>();
+        s.climbableId = r.Number<int>();
+        s.lifeState = r.Number<int>();
+        s.hp = r.Number<int>();
+        s.maxHp = r.Number<int>();
+        if (!r.Finished() || s.mapId <= 0 || kind < 0 || kind > 1 || s.entityId <= 0 ||
+            s.epoch < 0 || s.sequence < 0 || mode < 0 || mode > 3 ||
+            (s.facing != -1 && s.facing != 1) || s.climbableId < 0 ||
+            s.lifeState < 0 || s.lifeState > (kind == 0 ? 5 : 8) ||
+            s.hp < 0 || s.maxHp < 0 || s.hp > s.maxHp ||
+            (kind == 1 && s.sequence != 0) || (mode == 3 && s.climbableId == 0) ||
+            (mode != 3 && s.climbableId != 0))
+            throw std::runtime_error("invalid movement snapshot fields");
+        s.kind = static_cast<Kind>(kind);
+        s.mode = static_cast<Mode>(mode);
+        result = s; // All-or-nothing application.
+        error.clear();
+        return true;
+    }
+    catch (const std::exception& e)
+    {
+        error = e.what();
+        return false;
+    }
+}
+
+bool movement::BuildInputFields(int mapId, int epoch, int sequence, const Input& input,
+                                std::vector<std::string>& fields)
+{
+    if (mapId <= 0 || epoch < 0 || sequence <= 0 || input.horizontal < -1 ||
+        input.horizontal > 1 || input.vertical < -1 || input.vertical > 1) return false;
+    fields = {std::to_string(mapId), std::to_string(epoch), std::to_string(sequence),
+              std::to_string(input.horizontal), std::to_string(input.vertical), input.jump ? "1" : "0"};
+    return true;
+}
diff --git a/LL2_Client_Win_Source/MovementProtocol.h b/LL2_Client_Win_Source/MovementProtocol.h
new file mode 100644
index 0000000..df40772
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementProtocol.h
@@ -0,0 +1,11 @@
+#pragma once
+#include "MovementTypes.h"
+#include <string>
+#include <vector>
+
+namespace movement
+{
+    bool ParseSnapshot(const std::string& payload, Snapshot& result, std::string& error);
+    bool BuildInputFields(int mapId, int epoch, int sequence, const Input& input,
+                          std::vector<std::string>& fields);
+}
diff --git a/LL2_Client_Win_Source/MovementStream.h b/LL2_Client_Win_Source/MovementStream.h
new file mode 100644
index 0000000..5307b3b
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementStream.h
@@ -0,0 +1,94 @@
+#pragma once
+#include "MovementTypes.h"
+#include <algorithm>
+#include <limits>
+
+namespace movement
+{
+    // Server-authoritative rendering. No prediction, replay, or extrapolation.
+    class Stream
+    {
+    public:
+        bool Push(const Snapshot& value)
+        {
+            if (hasSnapshot && (!IsNewer(value, latest) ||
+                (waitNewEpoch && value.epoch <= latest.epoch && !(allowDead && IsDead(value))) ||
+                (IsDead(latest) && !IsDead(value) && value.epoch <= latest.epoch))) return false;
+            const bool snap = !hasSnapshot || value.epoch != latest.epoch || IsDead(value) != IsDead(latest);
+            const Snapshot previous = latest;
+            latest = value;
+            hasSnapshot = true;
+            waitNewEpoch = false;
+            elapsed = 0;
+            if (snap) displayed = value;
+            source = displayed;
+            if (!snap)
+            {
+                // Advance discrete states even when packets arrive just before the blend finishes.
+                source.mode = previous.mode;
+                source.climbableId = previous.climbableId;
+                source.velocity = previous.velocity;
+            }
+            return true;
+        }
+        bool Update(float dt, Snapshot& out)
+        {
+            if (!Ready()) return false;
+            elapsed = (std::min)(elapsed + (std::max)(dt, 0.0f), 0.05f);
+            const float alpha = elapsed / 0.05f;
+            displayed = latest;
+            displayed.position = {
+                static_cast<float>(double(source.position.x) + (double(latest.position.x) - source.position.x) * alpha),
+                static_cast<float>(double(source.position.y) + (double(latest.position.y) - source.position.y) * alpha)};
+            if (alpha < 1.0f && !IsDead(latest))
+            {
+                displayed.mode = source.mode;
+                displayed.climbableId = source.climbableId;
+                displayed.velocity = source.velocity;
+            }
+            out = displayed;
+            return true;
+        }
+        void Suspend(bool acceptDeath = false)
+        {
+            waitNewEpoch = true; allowDead = acceptDeath; displayed = source = latest; elapsed = 0;
+        }
+        void CancelSuspend() { waitNewEpoch = false; displayed = source = latest; elapsed = 0; }
+        void Reset() { *this = Stream{}; }
+        bool Ready() const { return hasSnapshot && !waitNewEpoch; }
+        bool HasSnapshot() const { return hasSnapshot; }
+        const Snapshot& Latest() const { return latest; }
+        const Snapshot& Displayed() const { return displayed; }
+    private:
+        Snapshot latest, source, displayed;
+        float elapsed = 0;
+        bool hasSnapshot = false;
+        bool waitNewEpoch = false;
+        bool allowDead = false;
+    };
+
+    class InputSchedule
+    {
+    public:
+        void Reset(int acceptedSequence = 0)
+        {
+            previous = {}; timer = 0; sent = false; sequence = acceptedSequence;
+        }
+        bool Poll(const Input& input, float dt, bool force, int& nextSequence)
+        {
+            timer += (std::max)(dt, 0.0f);
+            const bool changed = input.horizontal != previous.horizontal || input.vertical != previous.vertical;
+            if (sequence == (std::numeric_limits<int>::max)() ||
+                (!force && sent && !changed && !input.jump && timer < 0.1f)) return false;
+            previous = input; previous.jump = false;
+            timer = 0; sent = true;
+            nextSequence = ++sequence;
+            return true;
+        }
+    private:
+        Input previous;
+        float timer = 0;
+        int sequence = 0;
+        bool sent = false;
+    };
+}
diff --git a/LL2_Client_Win_Source/MovementTypes.h b/LL2_Client_Win_Source/MovementTypes.h
new file mode 100644
index 0000000..b41f8c1
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementTypes.h
@@ -0,0 +1,49 @@
+#pragma once
+#include <cstdint>
+
+namespace movement
+{
+    enum class Kind : int { Player = 0, Monster = 1 };
+    enum class Mode : int { Grounded = 0, Rising = 1, Falling = 2, Climbing = 3 };
+    enum class PlayerLife : int { Idle = 0, Move = 1, Jump = 2, Attack = 3, Stunned = 4, Dead = 5 };
+    enum class MonsterLife : int
+    {
+        Idle = 0, Patrol = 1, Chase = 2, Move = 3, Dying = 4,
+        Hit = 5, RangeAttack = 6, Dead = 7, None = 8
+    };
+
+    struct Point { float x = 0; float y = 0; };
+    struct Input { int horizontal = 0; int vertical = 0; bool jump = false; };
+    struct Snapshot
+    {
+        int mapId = 0;
+        Kind kind = Kind::Player;
+        int entityId = 0;
+        int epoch = 0;
+        std::uint64_t tick = 0;
+        int sequence = 0; // Last accepted input, NOT a physics acknowledgement.
+        Point position;
+        Point velocity;
+        Mode mode = Mode::Grounded;
+        int facing = -1;
+        int climbableId = 0;
+        int lifeState = 0;
+        int hp = 0;
+        int maxHp = 0;
+    };
+    inline bool IsDead(const Snapshot& s)
+    {
+        return s.kind == Kind::Player ? s.lifeState == static_cast<int>(PlayerLife::Dead)
+            : s.lifeState == static_cast<int>(MonsterLife::Dying) ||
+              s.lifeState == static_cast<int>(MonsterLife::Dead);
+    }
+    inline bool IsStunned(const Snapshot& s)
+    {
+        return s.kind == Kind::Player && s.lifeState == static_cast<int>(PlayerLife::Stunned);
+    }
+    inline bool IsNewer(const Snapshot& a, const Snapshot& b)
+    {
+        return a.epoch > b.epoch || (a.epoch == b.epoch && a.tick > b.tick);
+    }
+    inline constexpr float PlayerFootOffset = 10.0f;
+}
diff --git a/LL2_Client_Win_Source/MovementVisual.h b/LL2_Client_Win_Source/MovementVisual.h
new file mode 100644
index 0000000..e54b6fa
--- /dev/null
+++ b/LL2_Client_Win_Source/MovementVisual.h
@@ -0,0 +1,47 @@
+#pragma once
+#include "MovementTypes.h"
+#include "stbAnimator.h"
+#include <algorithm>
+#include <cmath>
+
+namespace movement
+{
+    class Visual
+    {
+    public:
+        void Reset() { previousMode = Mode::Grounded; landingTime = 0; }
+        void Update(stb::Animator* animator, const Snapshot& s, float dt, bool ladder, bool action)
+        {
+            if (!animator) return;
+            if ((previousMode == Mode::Rising || previousMode == Mode::Falling) && s.mode == Mode::Grounded)
+                landingTime = 0.1f;
+            previousMode = s.mode;
+            if (s.mode != Mode::Grounded) landingTime = 0;
+            const bool monster = s.kind == Kind::Monster;
+            animator->SetFlipX(s.facing > 0);
+            if (action && !IsDead(s))
+            {
+                animator->SetPaused(false); landingTime = 0; return;
+            }
+            std::wstring name;
+            bool loop = true;
+            if (IsDead(s)) { name = monster ? L"die" : L"dead"; loop = false; }
+            else if (IsStunned(s)) { name = L"hit"; loop = false; }
+            else if (s.mode == Mode::Rising) name = L"jump";
+            else if (s.mode == Mode::Falling) name = L"fall";
+            else if (s.mode == Mode::Climbing) name = ladder ? L"ladder" : L"rope";
+            else if (landingTime > 0) { name = L"land"; loop = false; }
+            else if (std::abs(s.velocity.x) > 0.01f) name = monster ? L"move" : L"walk";
+            else name = monster ? L"idle" : L"stand";
+            landingTime = (std::max)(0.0f, landingTime - dt);
+            if (!animator->FindAnimation(name))
+                name = (!monster && s.mode == Mode::Falling && animator->FindAnimation(L"jump"))
+                    ? L"jump" : (monster ? L"idle" : L"stand");
+            if (animator->FindAnimation(name) && !animator->IsPlaying(name)) animator->PlayAnimation(name, loop);
+            animator->SetPaused(!IsDead(s) && s.mode == Mode::Climbing && std::abs(s.velocity.y) < 0.01f);
+        }
+    private:
+        Mode previousMode = Mode::Grounded;
+        float landingTime = 0;
+    };
+}
diff --git a/LL2_Client_Win_Source/OtherPlayerPacketHandler.cpp b/LL2_Client_Win_Source/OtherPlayerPacketHandler.cpp
index b84446b..dfa57f9 100644
--- a/LL2_Client_Win_Source/OtherPlayerPacketHandler.cpp
+++ b/LL2_Client_Win_Source/OtherPlayerPacketHandler.cpp
@@ -1,5 +1,6 @@
 ﻿#include "OtherPlayerPacketHandler.h"
 #include "PacketParser.h"
+#include "MovementPacketHandler.h"
 #include "stbOtherPlayerManager.h"

 #define OTHERPLAYERMANAGER stb::singletonBase<stb::OtherPlayerManager>::getInstance()
@@ -62,7 +63,7 @@ void OtherPlayerPacketHandler::HandleOtherPlayerEnter(const ParsedPacket& pkt)
             throw std::runtime_error(errMsg);
         }

-        otherPlayerInfo.state = PlayerTypeUtil::IntToState(state);
+        otherPlayerInfo.state = PlayerTypeUtil::ServerLifeToState(state);

         std::string msg =
             "[OtherEnter] charId=" + std::to_string(otherPlayerInfo.char_id) +
@@ -116,6 +117,7 @@ void OtherPlayerPacketHandler::HandleOtherPlayerLeave(const ParsedPacket& pkt)
             throw std::runtime_error("OtherPlayerManager is not available");
         }

+        MovementPacketHandler::Forget(movement::Kind::Player, playerId);
         otherPlayerManager->RemovePlayer(std::to_string(playerId));

         const std::string message = "[OtherLeave] playerId=" +std::to_string(playerId) + "\n";
@@ -184,7 +186,7 @@ void OtherPlayerPacketHandler::HandleOtherPlayerSnapShot(const ParsedPacket& pkt
                 throw std::runtime_error(errMsg);
             }

-            otherPlayerInfo.state = PlayerTypeUtil::IntToState(state);
+            otherPlayerInfo.state = PlayerTypeUtil::ServerLifeToState(state);

             std::string msg =
                 "[SnapShot] charId=" + std::to_string(otherPlayerInfo.char_id) +
diff --git a/LL2_Client_Win_Source/Packet.h b/LL2_Client_Win_Source/Packet.h
index 60c0a3d..49b2fc5 100644
--- a/LL2_Client_Win_Source/Packet.h
+++ b/LL2_Client_Win_Source/Packet.h
@@ -63,6 +63,8 @@ enum PACKET_TYPE : uint16_t {
     PKT_PLAYER_PICKUP_ITEM  = 0x0029,
     PKT_PLAYER_DEAD         = 0x002A,
     PKT_PLAYER_REVIVE       = 0x002B,
+    PKT_MOVEMENT_INPUT      = 0x002C,
+    PKT_MOVEMENT_SNAPSHOT   = 0x002D,


     PKT_OTHERPLAYER_ENTER   = 0x0030,
diff --git a/LL2_Client_Win_Source/PacketManager.cpp b/LL2_Client_Win_Source/PacketManager.cpp
index 3759308..78d28e4 100644
--- a/LL2_Client_Win_Source/PacketManager.cpp
+++ b/LL2_Client_Win_Source/PacketManager.cpp
@@ -2,6 +2,7 @@
 #include "Packet.h"
 #include "stbNetworkManager.h"
 #include "MovePacketHandler.h"
+#include "MovementPacketHandler.h"
 #include "ChannelInitPacketHandler.h"
 #include "InventoryPacketHandler.h"
 #include "PlayerDataPacketHandler.h"
@@ -18,6 +19,8 @@
 bool PacketManager::RegisterAllHandlers()
 {
    auto networkManager = stb::NetworkManager::getInstance();
+   networkManager->RegisterHandler(PKT_MOVEMENT_INPUT, MovementPacketHandler::HandleInputResponse);
+   networkManager->RegisterHandler(PKT_MOVEMENT_SNAPSHOT, MovementPacketHandler::Execute);

    // 플레이어 접속 핸들러 등록
    networkManager->RegisterHandler(PKT_CHANNEL_AUTH,
@@ -86,7 +89,7 @@ bool PacketManager::RegisterAllHandlers()
    networkManager->RegisterHandler(PKT_PLAYER_MOVE,
        [](const ParsedPacket& pkt)
        {
-           MovePacketHandler::Execute(pkt);
+           MovementPacketHandler::HandleInputResponse(pkt);
        });

    // 플레이어 아이템 사용 핸들러 등록
diff --git a/LL2_Client_Win_Source/PlayerDataPacketHandler.cpp b/LL2_Client_Win_Source/PlayerDataPacketHandler.cpp
index bc17bc2..ad5de7d 100644
--- a/LL2_Client_Win_Source/PlayerDataPacketHandler.cpp
+++ b/LL2_Client_Win_Source/PlayerDataPacketHandler.cpp
@@ -5,6 +5,9 @@
 #include "PlayerManager.h"
 #include "ExpBarUI.h"
 #include "Stat.h"
+#include "stbPlayer.h"
+#include "stbTransform.h"
+#include <cmath>
 #include "UIManager.h"

 #include "stbApplication.h"
@@ -357,75 +360,22 @@ void PlayerDataPacketHandler::HandlePlayerOnDamaged(const ParsedPacket& pkt)

 void PlayerDataPacketHandler::HandlePlayerDead(const ParsedPacket& pkt)
 {
-   try
-   {
-       size_t offset = 0;
-       const char* data = pkt.payload.c_str();
-       size_t payloadSize = pkt.payload.size();
-       std::string errMsg;
-
-       auto playerManager = PlayerManager::getInstance();
-       if (!playerManager)
-       {
-           throw std::runtime_error("playerManager is nullptr");
-       }
-       auto localPlayer = playerManager->GetLocalPlayer();
-       if (!localPlayer)
-       {
-           throw std::runtime_error("localPlayer is nullptr");
-       }
-
-       std::vector<std::string> inputs;
-
-       /*
-       4개
-   payload.push_back(std::to_string(player->GetId()));
-    payload.push_back(player->GetName());
-    payload.push_back(std::to_string(player->GetPos().xPos));
-    payload.push_back(std::to_string(player->GetPos().yPos));
-       */
-       while (1)
-       {
-           std::string input;
-           if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, input, errMsg))
-           {
-               break;
-           }
-           inputs.push_back(input);
-       }
-
-       if (inputs.size() == 4) //4개 정상수신
-       {
-           //죽음 모달 띄우기 메시지 전송
-           const bool firstDeath = !localPlayer->IsDead();
-
-           localPlayer->SetState(PlayerState::Dead); //죽음으로 상태변경 -> 내부에서 상태변경에 따른 애니메이션 변경
-
-           if (firstDeath)
-           {
-               ::PostMessageW(
-                   stb::Application::getInstance()->GetHWND(),
-                   WM_SHOW_REVIVE,
-                   0,
-                   0
-               );
-           }
-       }
-       else
-           throw std::runtime_error("HandlePlayerDead input error");
-
-       OutputDebugStringA("HandlePlayerDead Success\n");
-   }
-   catch (const std::exception& e)
-   {
-       OutputDebugStringA("[HandlePlayerDead] ");
-       OutputDebugStringA(e.what());
-       OutputDebugStringA("\n");
-   }
-   catch (...)
-   {
-       OutputDebugStringA("예상치 못한 에러 발생\n");
-   }
+    size_t offset = 0;
+    std::string error, name;
+    int id = 0;
+    float x = 0, y = 0;
+    const char* data = pkt.payload.data();
+    const size_t size = pkt.payload.size();
+    if (!PacketParser::ParseNextIntField(data, size, offset, id, error) ||
+        !PacketParser::ParseLengthPrefixedString(data, size, offset, name, error) ||
+        !PacketParser::ParseNextFloatField(data, size, offset, x, error) ||
+        !PacketParser::ParseNextFloatField(data, size, offset, y, error) ||
+        offset != size || !std::isfinite(x) || !std::isfinite(y)) return;
+    auto* player = M_PLAYERMANAGER->GetLocalPlayer();
+    if (!player || player->GetPlayerIdentity()->charId != id) return;
+    player->GetMovementScript()->OnServerDeath();
+    if (auto* transform = player->GetComponent<stb::Transform>()) transform->SetPosition({x, y});
+    player->GetPlayerLocation()->pos = {x, y};
 }

 void PlayerDataPacketHandler::SendPlayerRevive()
@@ -438,49 +388,14 @@ void PlayerDataPacketHandler::SendPlayerRevive()

 void PlayerDataPacketHandler::HandlePlayerRevive(const ParsedPacket& pkt)
 {
-   try
-   {
-       size_t offset = 0;
-       const char* data = pkt.payload.c_str();
-       size_t payloadSize = pkt.payload.size();
-       std::string errMsg;
-
-       auto playerManager = PlayerManager::getInstance();
-       if (!playerManager)
-       {
-           throw std::runtime_error("playerManager is nullptr");
-       }
-       auto localPlayer = playerManager->GetLocalPlayer();
-       if (!localPlayer)
-       {
-           throw std::runtime_error("localPlayer is nullptr");
-       }
-
-       std::string input;
-       if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, input, errMsg))
-       {
-       }
-
-       if (input == "nok")
-           throw std::runtime_error("HandlePlayerRevive input error");
-
-
-       // 1. 플레이어 상태 갱신
-       localPlayer->SetState(PlayerState::Idle);
-
-       // 2. UI에게 부활 성공 알림
-       ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_REVIVE_SUCCESS, 0, 0);
-
-       OutputDebugStringA("HandlePlayerRevive Success\n");
-   }
-   catch (const std::exception& e)
-   {
-       OutputDebugStringA("[HandlePlayerRevive] ");
-       OutputDebugStringA(e.what());
-       OutputDebugStringA("\n");
-   }
-   catch (...)
-   {
-       OutputDebugStringA("예상치 못한 에러 발생\n");
-   }
+    size_t offset = 0;
+    std::string status, error;
+    if (!PacketParser::ParseLengthPrefixedString(pkt.payload.data(), pkt.payload.size(), offset, status, error)) return;
+    if (status != "ok")
+    {
+        OutputDebugStringA("[Revive] rejected\n");
+        return;
+    }
+    // A live new-epoch snapshot can precede this response. Never reset movement here.
+    OutputDebugStringA("[Revive] accepted; position/life comes from movement snapshot\n");
 }
diff --git a/LL2_Client_Win_Source/PortalPacketHandler.cpp b/LL2_Client_Win_Source/PortalPacketHandler.cpp
index 858aa1d..94bd978 100644
--- a/LL2_Client_Win_Source/PortalPacketHandler.cpp
+++ b/LL2_Client_Win_Source/PortalPacketHandler.cpp
@@ -7,6 +7,7 @@
 #include "stbTransform.h"
 #include "MapScene.h"
 #include "Portal.h"
+#include "MovementPacketHandler.h"
 #include "stbOtherPlayerManager.h"


@@ -51,6 +52,7 @@ void PortalPacketHandler::HandleMoveMap(const ParsedPacket& pkt)

        if (status != "ok")
        {
+            MovementPacketHandler::CancelMapTransition();
            std::string serverError;

            PacketParser::ParseLengthPrefixedString(
@@ -119,6 +121,8 @@ void PortalPacketHandler::HandleMoveMap(const ParsedPacket& pkt)
        player->GetPlayerLocation()->pos = spawnPostion;

        player->GetPlayerLocation()->mapId = destinationMapId;
+        player->GetMovementScript()->SuspendMovement();
+        MovementPacketHandler::CommitMap(destinationMapId);

        OutputDebugStringA("포탈 맵 이동 완료\n");
    }
@@ -136,6 +140,7 @@ void PortalPacketHandler::HandleMoveMap(const ParsedPacket& pkt)
 void PortalPacketHandler::SendPortalEnter(std::string portalId)
 {
    s_pendingPortalId = portalId;
+    MovementPacketHandler::BeginMapTransition();

    std::vector<std::string> data = { portalId };

diff --git a/LL2_Client_Win_Source/Stat.h b/LL2_Client_Win_Source/Stat.h
index 595381d..03489fb 100644
--- a/LL2_Client_Win_Source/Stat.h
+++ b/LL2_Client_Win_Source/Stat.h
@@ -17,6 +17,7 @@ public:
    void SetDerivedStat(const DerivedStat& derived);

    void SetCurHp(int cur_hp) { m_cur_hp = cur_hp; }
+   void SetHealth(int hp, int maxHp) { m_cur_hp = hp; m_derived.maxHp = maxHp; }
    void SetCurMp(int cur_mp) { m_cur_mp = cur_mp; }
    void SetRemainAp(int remainAp) { m_remain_ap = remainAp; }
    void SetExp(uint64_t exp) { m_expStat.exp = exp; }
diff --git a/LL2_Client_Win_Source/playerInfo.h b/LL2_Client_Win_Source/playerInfo.h
index 63561ff..7031e0d 100644
--- a/LL2_Client_Win_Source/playerInfo.h
+++ b/LL2_Client_Win_Source/playerInfo.h
@@ -89,6 +89,20 @@ enum class JobType

 namespace PlayerTypeUtil
 {
+    // Server movement/life enum is not the client's animation enum.
+    inline PlayerState ServerLifeToState(int state)
+    {
+        switch (state)
+        {
+        case 0: return PlayerState::Idle;
+        case 1: return PlayerState::Walk;
+        case 2: return PlayerState::Jump;
+        case 3: return PlayerState::Attack;
+        case 4: return PlayerState::Alert;
+        case 5: return PlayerState::Dead;
+        default: return PlayerState::Idle;
+        }
+    }
    inline JobType StringToJobType(const std::string& str)
    {
        if (str == "warrior")
diff --git a/LL2_Client_Win_Source/stbAnimator.cpp b/LL2_Client_Win_Source/stbAnimator.cpp
index 85f1f9d..b6fc585 100644
--- a/LL2_Client_Win_Source/stbAnimator.cpp
+++ b/LL2_Client_Win_Source/stbAnimator.cpp
@@ -34,7 +34,7 @@ namespace stb

    void Animator::Update()
    {
-       if (mActiveAnimation == nullptr)
+       if (mActiveAnimation == nullptr || m_paused)
            return;

        mActiveAnimation->Update();
@@ -276,6 +276,7 @@ namespace stb
        }

        mActiveAnimation = animation;
+       m_paused = false;
        mActiveAnimation->Reset();
        mbLoop = loop;
        mbCompleteEventCalled = false;
diff --git a/LL2_Client_Win_Source/stbAnimator.h b/LL2_Client_Win_Source/stbAnimator.h
index 33df3d4..6e4f9d3 100644
--- a/LL2_Client_Win_Source/stbAnimator.h
+++ b/LL2_Client_Win_Source/stbAnimator.h
@@ -111,6 +111,11 @@ namespace stb

    public:
        void SetFlipX(bool flipX) { m_flipX = flipX; }
+       void SetPaused(bool paused) { m_paused = paused; }
+       bool IsPlaying(const std::wstring& name) const
+       {
+           return mActiveAnimation && mActiveAnimation->GetName() == name;
+       }
    private:
        void InvokeEvent(const std::wstring& eventName);
    private:
@@ -122,6 +127,7 @@ namespace stb
        bool mbLoop;
        bool mbCompleteEventCalled;
        bool m_flipX = false;
+       bool m_paused = false;
        std::map<std::wstring, Events*> mEvents;


diff --git a/LL2_Client_Win_Source/stbApplication.cpp b/LL2_Client_Win_Source/stbApplication.cpp
index 8d64bc4..117a77e 100644
--- a/LL2_Client_Win_Source/stbApplication.cpp
+++ b/LL2_Client_Win_Source/stbApplication.cpp
@@ -1,5 +1,6 @@
 ﻿#include "stbApplication.h"
 #include "stbInput.h"
+#include "MovementPacketHandler.h"
 #include "stbTime.h"
 #include "stbSceneManager.h"
 #include "stbCollisionManager.h"
@@ -102,6 +103,7 @@ namespace stb
    {
        M_INPUT->Update();
        M_TIME->Update();
+       MovementPacketHandler::Pump();
        M_COLMANAGER->Update();
        M_SCENEMANAGER->Update();
    }
diff --git a/LL2_Client_Win_Source/stbNetworkManager.cpp b/LL2_Client_Win_Source/stbNetworkManager.cpp
index 80430c9..c3f4d61 100644
--- a/LL2_Client_Win_Source/stbNetworkManager.cpp
+++ b/LL2_Client_Win_Source/stbNetworkManager.cpp
@@ -1,4 +1,5 @@
-﻿#include "stbNetworkManager.h"
+﻿#include "MovementPacketHandler.h"
+#include "stbNetworkManager.h"
 #include <WinSock2.h>
 #include <WS2tcpip.h>
 #include <sstream>
@@ -88,6 +89,7 @@ namespace stb
         {
             m_socket.Close();
             m_bConnected = false;
+            MovementPacketHandler::ResetConnection();
         }
     }

diff --git a/LL2_Client_Win_Source/stbOtherPlayer.cpp b/LL2_Client_Win_Source/stbOtherPlayer.cpp
index 861e279..c7fbb00 100644
--- a/LL2_Client_Win_Source/stbOtherPlayer.cpp
+++ b/LL2_Client_Win_Source/stbOtherPlayer.cpp
@@ -1,9 +1,11 @@
-﻿#include "stbOtherPlayer.h"
+﻿#include "MovementDebug.h"
+#include "stbOtherPlayer.h"
 #include "stbTransform.h"
 #include "stbAnimator.h"
 #include "stbResourceManager.h"
 #include "stbTexture.h"
 #include "stbTime.h"
+#include "MapDataManager.h"
 #include "stbD2DRenderer.h"
 #include "stbRender.h"
 #include "stbCamera.h"
@@ -44,40 +46,34 @@ namespace stb
     void OtherPlayer::Update()
     {
         GameObject::Update();
-
-        // 목표 위치가 있으면 부드럽게 이동
-        if (mHasTarget)
+        movement::Snapshot displayed;
+        if (!m_movement.Update(M_Time->GetDeltaTime(), displayed)) return;
+        m_transform->SetPosition({displayed.position.x, displayed.position.y});
+        SyncFollowers({displayed.position.x, displayed.position.y});
+        const auto* map = MapDataManager::getInstance()->FindMapData(displayed.mapId);
+        const auto* climb = map ? map->physics.FindClimbable(displayed.climbableId) : nullptr;
+        const bool action = m_playerState >= PlayerState::Attack && m_playerState < PlayerState::Skill_End;
+        m_movementVisual.Update(m_animator, displayed, M_Time->GetDeltaTime(), climb && climb->ladder,
+            action && displayed.mode != movement::Mode::Climbing && !movement::IsStunned(displayed));
+    }
+
+    void OtherPlayer::ApplyMovementSnapshot(const movement::Snapshot& s)
+    {
+        const bool reset = !m_movement.HasSnapshot() || s.epoch != m_movement.Latest().epoch;
+        const int previousLife = m_movement.HasSnapshot() ? m_movement.Latest().lifeState : -1;
+        if (!m_movement.Push(s)) return;
+        if (reset)
         {
-            Transform* tr = GetComponent<Transform>();
-            if (tr)
-            {
-                Vector2 currentPos = tr->GetPosition();
-                Vector2 direction = mTargetPosition - currentPos;
-                float distance = direction.length();
-
-                if (distance > 0.5f) // 목표에 거의 도달하지 않았으면
-                {
-                    // Lerp 방식: 거리에 비례해서 부드럽게 이동
-                    float lerpFactor = 10.0f * M_Time->GetDeltaTime(); // 초당 10배 속도로 따라감
-                    if (lerpFactor > 1.0f) lerpFactor = 1.0f;
-
-                    Vector2 newPos = currentPos + direction * lerpFactor;
-                    tr->SetPosition(newPos);
-                    SyncFollowers(newPos);
-                }
-                else
-                {
-                    // 목표에 도달
-                    tr->SetPosition(mTargetPosition);
-                    SyncFollowers(mTargetPosition);
-                    mHasTarget = false;
-                    if (m_playerState == PlayerState::Walk)
-                    {
-                        SetState(PlayerState::Idle);
-                    }
-                }
-            }
+            m_movementVisual.Reset();
+            m_transform->SetPosition({s.position.x, s.position.y});
+            SyncFollowers({s.position.x, s.position.y});
+            SetState(movement::IsDead(s) ? PlayerState::Dead : PlayerState::Idle);
         }
+        if (movement::IsDead(s)) SetState(PlayerState::Dead);
+        else if (s.mode == movement::Mode::Climbing || movement::IsStunned(s)) SetState(PlayerState::Idle);
+        else if (s.lifeState == static_cast<int>(movement::PlayerLife::Attack) && previousLife != s.lifeState)
+            SetState(PlayerState::Attack);
+        mHasTarget = false;
     }

     void OtherPlayer::LateUpdate()
@@ -93,6 +89,7 @@ namespace stb
     void OtherPlayer::Render(stbD2DRenderer& renderer)
     {
         GameObject::Render(renderer);
+        if (m_transform) movement::DrawOriginAndFeet(renderer, m_transform->GetPosition(), movement::PlayerFootOffset);

         if (m_transform == nullptr ||
             m_nickName.empty())
@@ -144,6 +141,7 @@ namespace stb

     void OtherPlayer::UpdatePosition(float x, float y)
     {
+        if (m_movement.HasSnapshot()) return;
         Transform* tr = GetComponent<Transform>();
         if (tr)
         {
@@ -153,6 +151,7 @@ namespace stb

     void OtherPlayer::SetTargetPosition(float x, float y, float speed)
     {
+        if (m_movement.HasSnapshot()) return;
         mTargetPosition = Vector2(x, y);
         mTargetSpeed = speed;
         mHasTarget = true;
@@ -173,6 +172,7 @@ namespace stb
             return;

         m_playerState = state;
+        if (m_animator) m_animator->SetPaused(false);

         switch (state)
         {
@@ -184,6 +184,12 @@ namespace stb
             m_currentAnimation = L"walk";
             break;

+        case PlayerState::Dead:
+            m_currentAnimation = L"dead";
+            break;
+        case PlayerState::Skill_Slash:
+            m_currentAnimation = L"slash";
+            break;
         case PlayerState::Attack:
             m_currentAnimation = L"swingO3";
             break;
@@ -198,7 +204,7 @@ namespace stb
         {
             bool isLoop = true;

-            if (state == PlayerState::Attack)
+            if ((state >= PlayerState::Attack && state < PlayerState::Skill_End) || state == PlayerState::Dead)
                 isLoop = false;

             animator->PlayAnimation(m_currentAnimation, isLoop);
diff --git a/LL2_Client_Win_Source/stbOtherPlayer.h b/LL2_Client_Win_Source/stbOtherPlayer.h
index 619f55f..0a15872 100644
--- a/LL2_Client_Win_Source/stbOtherPlayer.h
+++ b/LL2_Client_Win_Source/stbOtherPlayer.h
@@ -7,6 +7,8 @@
 #include "stbDamageText.h"
 #include "BoxCollider2D.h"
 #include "EquipeTypes.h"
+#include "MovementStream.h"
+#include "MovementVisual.h"

 class stbD2DRenderer;

@@ -34,6 +36,9 @@ namespace stb
         void SetTargetPosition(float x, float y, float speed);
         void SetDirection(int dir);
         void SetState(PlayerState state);
+        void ApplyMovementSnapshot(const movement::Snapshot& snapshot);
+        bool HasMovementSnapshot() const { return m_movement.HasSnapshot(); }
+        void ResetMovementConnection() { m_movement.Reset(); m_movementVisual.Reset(); }

         PlayerState GetState() { return m_playerState; }
         void AddFollower(GameObject* obj, Vector2 offset)
@@ -53,6 +58,8 @@ namespace stb
         float mTargetSpeed;
         bool mHasTarget;
         float mInterpolationSpeed;
+        movement::Stream m_movement;
+        movement::Visual m_movementVisual;

         // 플레이어 상태
         PlayerState m_playerState;
diff --git a/LL2_Client_Win_Source/stbOtherPlayerManager.cpp b/LL2_Client_Win_Source/stbOtherPlayerManager.cpp
index ec25a87..e2b3349 100644
--- a/LL2_Client_Win_Source/stbOtherPlayerManager.cpp
+++ b/LL2_Client_Win_Source/stbOtherPlayerManager.cpp
@@ -61,6 +61,7 @@ namespace stb

         OtherPlayer* player = it->second;

+        if (player->GetState() == PlayerState::Dead) return false;
         player->SetState(PlayerState::Attack);

         return true;
@@ -77,7 +78,7 @@ namespace stb
         {
             // 이미 존재하면 위치/상태만 업데이트
             OtherPlayer* existing = it->second;
-            if (existing)
+            if (existing && !existing->HasMovementSnapshot())
             {
                 existing->UpdatePosition(playerInfo.xPos, playerInfo.yPos);
                 existing->SetTargetPosition(playerInfo.xPos, playerInfo.yPos, playerInfo.speed);
diff --git a/LL2_Client_Win_lib/MapDataManager.cpp b/LL2_Client_Win_lib/MapDataManager.cpp
index 9ce123a..66638e8 100644
--- a/LL2_Client_Win_lib/MapDataManager.cpp
+++ b/LL2_Client_Win_lib/MapDataManager.cpp
@@ -65,6 +65,16 @@ bool MapDataManager::LoadJsonFile(const std::string& path, MapData& mapData)
     }
     if (j.is_null()) return false;

+    try
+    {
+        mapData.physics = movement::ParseMapGeometry(j.at("physics"));
+    }
+    catch (const std::exception& error)
+    {
+        OutputDebugStringA(("[Map physics] " + path + ": " + error.what() + "\n").c_str());
+        return false;
+    }
+
     mapData.mapId = j.at("mapId").get<int>();
     mapData.name = j.at("name").get<std::string>();
     mapData.background = j.at("background").get<std::string>();
@@ -76,6 +86,7 @@ bool MapDataManager::LoadJsonFile(const std::string& path, MapData& mapData)
             PortalData portalData;

             portalData.id = portalJson.value("id", "");
+            portalData.interactionRange = portalJson.value("interactionRange", 120.0f);
             portalData.texture = portalJson.value("texture", "");
             const auto& position = portalJson.at("position");
             portalData.position.x = position.value("x", 0.0f);
diff --git a/LL2_Client_Win_lib/MapInfo.h b/LL2_Client_Win_lib/MapInfo.h
index 8e90fa9..4b9ac6a 100644
--- a/LL2_Client_Win_lib/MapInfo.h
+++ b/LL2_Client_Win_lib/MapInfo.h
@@ -1,11 +1,13 @@
 ﻿#pragma once

 #include "stbMath.h"
+#include "MovementMap.h"

 struct PortalData
 {
     std::string id;
     std::string texture;
+    float interactionRange = 120.0f;

     stb::math::Vector2 position;
     stb::math::Vector2 renderSize =
@@ -20,6 +22,7 @@ struct PortalData

 struct MapData
 {
+    movement::MapGeometry physics;
     int mapId = 0;

     std::string name;
diff --git a/LL2_Client_Win_lib/MapScene.cpp b/LL2_Client_Win_lib/MapScene.cpp
index 8731f0c..c03d668 100644
--- a/LL2_Client_Win_lib/MapScene.cpp
+++ b/LL2_Client_Win_lib/MapScene.cpp
@@ -8,6 +8,7 @@
 #include "stbResourceManager.h"
 #include "ProjectileManager.h"
 #include "SkillEffectManager.h"
+#include "MovementDebug.h"

 #define M_TIME SingletonBase<Time>::getInstance()
 #define M_MONSTERMANAGER SingletonBase<MonsterManager>::getInstance()
@@ -37,6 +38,7 @@ void MapScene::Update()
 void MapScene::Render(stbD2DRenderer& renderer)
 {
    RenderBackground(renderer);
+   RenderMovementGeometry(renderer);
    Scene::Render(renderer);

    M_MONSTERMANAGER->Render(renderer);
@@ -83,6 +85,7 @@ void MapScene::CreatePortals()

         portal->SetRenderSize(data.renderSize);
         portal->SetTriggerHalfSize(data.halfSize);
+        portal->SetInteractionRange(data.interactionRange);
         portal->SetPortalId(data.id);
         std::wstring destinationScene = L"Map_" +std::to_wstring(data.destinationMapId);

@@ -99,3 +102,38 @@ Portal* MapScene::FindPortal(const std::string& portalId) const

     return iter->second;
 }
+
+void MapScene::RenderMovementGeometry(stbD2DRenderer& renderer)
+{
+    const auto* map = M_MAPDATAMANAGER->FindMapData(GetMapId());
+    if (!map) return;
+    auto screen = [](float x, float y)
+    {
+        stb::math::Vector2 point{x, y};
+        return stb::render::mainCamera ? stb::render::mainCamera->CalculatePosition(point) : point;
+    };
+    for (const auto& c : map->physics.climbables)
+    {
+        auto top = screen(c.x, c.top), bottom = screen(c.x, c.bottom);
+        const D2D1::ColorF brown(D2D1::ColorF::SaddleBrown);
+        if (c.ladder)
+        {
+            renderer.DrawLine(top.x - 9, top.y, bottom.x - 9, bottom.y, brown, 3);
+            renderer.DrawLine(top.x + 9, top.y, bottom.x + 9, bottom.y, brown, 3);
+            for (float y = top.y; y <= bottom.y; y += 16)
+                renderer.DrawLine(top.x - 9, y, top.x + 9, y, brown, 2);
+        }
+        else renderer.DrawLine(top.x, top.y, bottom.x, bottom.y, brown, 3);
+#ifdef _DEBUG
+        renderer.DrawRect(top.x - c.grabRange, top.y, c.grabRange * 2, bottom.y - top.y,
+            D2D1::ColorF(D2D1::ColorF::Yellow));
+#endif
+    }
+#ifdef _DEBUG
+    for (const auto& platform : map->physics.platforms)
+    {
+        auto left = screen(platform.left, platform.y), right = screen(platform.right, platform.y);
+        renderer.DrawLine(left.x, left.y, right.x, right.y, D2D1::ColorF(D2D1::ColorF::Lime), 2);
+    }
+#endif
+}
diff --git a/LL2_Client_Win_lib/MapScene.h b/LL2_Client_Win_lib/MapScene.h
index b9bcd3f..7ca7b15 100644
--- a/LL2_Client_Win_lib/MapScene.h
+++ b/LL2_Client_Win_lib/MapScene.h
@@ -27,6 +27,7 @@ protected:

 private:
     void CreatePortals();
+    void RenderMovementGeometry(stbD2DRenderer& renderer);
     std::unordered_map<std::string, Portal*> m_portals;
 };

diff --git a/LL2_Client_Win_lib/Portal.cpp b/LL2_Client_Win_lib/Portal.cpp
index bcc4019..8b6f8ad 100644
--- a/LL2_Client_Win_lib/Portal.cpp
+++ b/LL2_Client_Win_lib/Portal.cpp
@@ -8,6 +8,8 @@
 #include "stbPlayer.h"
 #include "stbNetworkManager.h"
 #include "PortalPacketHandler.h"
+#include "UIManager.h"
+#include "stbApplication.h"
 #include <cmath>

 using namespace stb;
@@ -29,44 +31,18 @@ void Portal::Initialize()
 void Portal::Update()
 {
     GameObject::Update();
-
-    if (m_transitioning || m_destinationScene.empty())
-        return;
-
-    Player* player = M_PLAYERMANAGER->GetLocalPlayer();
-
-    if (player == nullptr || m_collider == nullptr)
-        return;
-
-    Transform* portalTransform = GetComponent<Transform>();
-    Transform* playerTransform = player->GetComponent<Transform>();
-    BoxCollider2D* playerCollider = player->GetComponent<BoxCollider2D>();
-
-    if (portalTransform == nullptr || playerTransform == nullptr || playerCollider == nullptr)
-    {
-        return;
-    }
-
-    math::Vector2 portalCenter = portalTransform->GetPosition() + m_collider->GetOffset();
-
-    math::Vector2 playerCenter = playerTransform->GetPosition() + playerCollider->GetOffset();
-
-    math::Vector2 portalHalfSize = m_collider->GetSize();
-
-    math::Vector2 playerHalfSize = playerCollider->GetSize();
-
-    bool isOverlapping = std::abs(playerCenter.x - portalCenter.x) <= playerHalfSize.x + portalHalfSize.x &&
-        std::abs(playerCenter.y - portalCenter.y) <= playerHalfSize.y + portalHalfSize.y;
-
-    if (!isOverlapping)
-        return;
-
-    if (!M_INPUT->GetActionDown(eActionCode::MoveUp))
-        return;
-
+    if (m_transitioning || m_destinationScene.empty()) return;
+    auto* player = M_PLAYERMANAGER->GetLocalPlayer();
+    if (!player || player->IsDead() || !player->GetMovementScript()->CanUsePortal() ||
+        UIManager::getInstance()->IsInputFocused() || UIManager::getInstance()->IsGameplayInputBlocked() ||
+        GetForegroundWindow() != GetAncestor(stb::Application::getInstance()->GetHWND(), GA_ROOT)) return;
+    auto* tr = GetComponent<Transform>();
+    auto* playerTransform = player->GetComponent<Transform>();
+    if (!tr || !playerTransform || !M_INPUT->GetActionDown(eActionCode::Interact)) return;
+    auto delta = playerTransform->GetPosition() - tr->GetPosition();
+    if (delta.x * delta.x + delta.y * delta.y > m_interactionRange * m_interactionRange) return;
     m_transitioning = true;
     PortalPacketHandler::SendPortalEnter(m_portalId);
-
 }

 void Portal::Render(stbD2DRenderer& renderer)
diff --git a/LL2_Client_Win_lib/Portal.h b/LL2_Client_Win_lib/Portal.h
index a8a63c8..580a362 100644
--- a/LL2_Client_Win_lib/Portal.h
+++ b/LL2_Client_Win_lib/Portal.h
@@ -20,12 +20,14 @@ public:
     void SetRenderSize(const stb::math::Vector2& size);
     void SetPortalId(const std::string& portalId) { m_portalId = portalId; }
     void ResetTransition() { m_transitioning = false; }
+    void SetInteractionRange(float range) { m_interactionRange = range; }
 private:
     std::string m_portalId;
     std::wstring m_destinationScene;
     stb::math::Vector2 m_spawnPosition;
     stb::BoxCollider2D* m_collider = nullptr;
     bool m_transitioning = false;
+    float m_interactionRange = 120.0f;

     stb::math::Vector2 m_triggerHalfSize = stb::math::Vector2(50.0f, 80.0f);

diff --git a/LL2_Client_Win_lib/stbPlayer.cpp b/LL2_Client_Win_lib/stbPlayer.cpp
index 0850983..f061d7e 100644
--- a/LL2_Client_Win_lib/stbPlayer.cpp
+++ b/LL2_Client_Win_lib/stbPlayer.cpp
@@ -1,4 +1,5 @@
-﻿#include "stbPlayer.h"
+﻿#include "MovementDebug.h"
+#include "stbPlayer.h"
 #include "InventoryManager.h"
 #include "stbPlayerScript.h"
 #include "..\\LL2_Client_Win_Source\\\PlayerAnimationManager.h"
@@ -59,6 +60,7 @@ namespace stb
    void Player::Render(stbD2DRenderer& renderer)
    {
        GameObject::Render(renderer);
+        if (m_transform) movement::DrawOriginAndFeet(renderer, m_transform->GetPosition(), movement::PlayerFootOffset);

        if (m_transform == nullptr ||
            m_playerProfile.name.empty())
@@ -157,6 +159,7 @@ namespace stb
            return;

        m_playerState = state;
+        if (m_animator) m_animator->SetPaused(false);

        switch (state)
        {
diff --git a/LL2_Client_Win_lib/stbPlayer.h b/LL2_Client_Win_lib/stbPlayer.h
index 03d7d13..9bfafde 100644
--- a/LL2_Client_Win_lib/stbPlayer.h
+++ b/LL2_Client_Win_lib/stbPlayer.h
@@ -75,6 +75,7 @@ namespace stb
        bool IsLocalPlayer() { return m_isLocalPlayer; }

        Animator* GetAnimator() { return m_animator; }
+       PlayerScript* GetMovementScript() { return m_script; }

        const float GetPlayerMoveSpeed() { return m_moveSpeed; }

diff --git a/LL2_Client_Win_lib/stbPlayerScript.cpp b/LL2_Client_Win_lib/stbPlayerScript.cpp
index 4fb23fe..b371c69 100644
--- a/LL2_Client_Win_lib/stbPlayerScript.cpp
+++ b/LL2_Client_Win_lib/stbPlayerScript.cpp
@@ -13,7 +13,9 @@
 #include "TradePacketHandler.h"
 #include "stbSceneManager.h"
 #include "stbPlayScene.h"
-#include "MovePacketHandler.h"
+#include "MovementPacketHandler.h"
+#include "MapDataManager.h"
+#include "GameUiMessages.h"
 #include "SkillDataManager.h"
 #include "SkillEffectManager.h"

@@ -37,75 +39,8 @@

 namespace stb
 {
-   Vector2 ClampPlayerPosition(Vector2 pos)
-    {
-        const auto* camera = render::mainCamera;
-        if (camera == nullptr)
-            return pos;
-
-        const Vector2 worldSize = camera->GetWorldSize();
-        const Vector2 resolution = camera->GetResolution();
-
-        if (worldSize.x <= 0.0f || worldSize.y <= 0.0f ||
-            resolution.y <= 0.0f)
-        {
-            return pos;
-        }
-
-        auto& renderer =
-            SingletonBase<Application>::getInstance()->GetRenderer();
-
-        float healthBarTop = renderer.GetRenderTargetSize().height;
-
-        if (auto* healthBar = M_UIMANAGER->GetHealthBarUI())
-        {
-            healthBarTop =
-                healthBar->CalculateBackgroundRect(renderer).y;
-        }
-
-        // 캐릭터 기준점에서 몸체 가장자리까지의 여백.
-        // 아래 값은 초기 조정값이며 실제 스프라이트에 맞춰 조정합니다.
-        constexpr float leftExtent   = 25.0f;
-        constexpr float rightExtent  = 25.0f;
-        constexpr float topExtent    = 60.0f;
-        constexpr float bottomExtent = 10.0f;
-        constexpr float uiGap        = 8.0f;
-
-        // 카메라가 맵 가장 아래까지 이동했을 때의 스크롤 값.
-        const float maxScrollY = (std::max)(
-            0.0f, worldSize.y - resolution.y
-        );
-
-        // 체력창 위쪽 선을 맵 하단의 월드 좌표로 변환.
-        const float bottomBoundary = (std::min)(
-            worldSize.y,
-            maxScrollY + healthBarTop - uiGap
-        );
-
-        // 작은 맵에서도 clamp의 최소값 <= 최대값을 보장.
-        const float minX = (std::min)(
-            leftExtent, worldSize.x * 0.5f
-        );
-        const float minY = (std::min)(
-            topExtent, worldSize.y * 0.5f
-        );
-
-        const float maxX = (std::max)(
-            minX, worldSize.x - rightExtent
-        );
-        const float maxY = (std::max)(
-            minY, bottomBoundary - bottomExtent
-        );
-
-        pos.x = std::clamp(pos.x, minX, maxX);
-        pos.y = std::clamp(pos.y, minY, maxY);
-
-        return pos;
-    }
-
    PlayerScript::PlayerScript()
-       : mNetworkSendTimer(0.0f)
-       , mHead(nullptr)
+       : mHead(nullptr)
        , mSword(nullptr)
        , m_player(nullptr)
        , mAttackTimer(0.0f)
@@ -127,33 +62,64 @@ namespace stb
    }

    void PlayerScript::Update()
-   {
-       if (m_player == nullptr) return;
-
-       // 사망 중 이동·공격 및 Idle 상태 전환 차단
-       if (m_player->IsDead())
-       {
-           mNetworkSendTimer = 0.0f;
-           mAttackTimer = 0.0f;
-           return;
-       }
-
-       if (M_UIMANAGER->IsInputFocused() || M_UIMANAGER->IsGameplayInputBlocked())
-           return;
-
-       HandleCombatInput();
-       HandleInput();
-
-       if (m_player->GetState() >= PlayerState::Attack && m_player->GetState() < PlayerState::Skill_End)
-       {
-           Idle(false);
-           return;
-       }
+    {
+        if (!m_player || !m_animator) return;
+        const float dt = M_TIME->GetDeltaTime();
+        auto* network = stb::NetworkManager::getInstance();
+        if (!network || !network->IsConnected())
+        {
+            ResetMovementConnection();
+            return;
+        }
+        movement::Snapshot displayed;
+        if (m_movement.Update(dt, displayed))
+        {
+            if (auto* tr = m_player->GetComponent<Transform>())
+                tr->SetPosition({displayed.position.x, displayed.position.y});
+            m_player->GetPlayerLocation()->pos = {displayed.position.x, displayed.position.y};
+            m_player->SetFacing(displayed.facing > 0 ? FacingDirection::Right : FacingDirection::Left);
+            SyncFollowers({displayed.position.x, displayed.position.y});
+            const auto* map = MapDataManager::getInstance()->FindMapData(displayed.mapId);
+            const auto* climb = map ? map->physics.FindClimbable(displayed.climbableId) : nullptr;
+            const bool attacking = m_player->GetState() >= PlayerState::Attack &&
+                                   m_player->GetState() < PlayerState::Skill_End;
+            m_movementVisual.Update(m_animator, displayed, dt, climb && climb->ladder,
+                attacking && displayed.mode != movement::Mode::Climbing && !movement::IsStunned(displayed));
+        }
+        const HWND window = stb::Application::getInstance()->GetHWND();
+        const auto held = [](eActionCode action)
+        {
+            return M_INPUT->GetAction(action) || M_INPUT->GetActionDown(action);
+        };
+        bool blocked = GetForegroundWindow() != GetAncestor(window, GA_ROOT) ||
+            M_UIMANAGER->IsInputFocused() || M_UIMANAGER->IsGameplayInputBlocked() || !CanMove();
+        if (blocked) { m_jumpPending = false; m_jumpNeedsRelease = true; }
+        else
+        {
+            if (!held(eActionCode::Jump)) m_jumpNeedsRelease = false;
+            if (CanAttack()) HandleCombatInput();
+            HandleInput();
+            blocked = M_UIMANAGER->IsInputFocused() || M_UIMANAGER->IsGameplayInputBlocked();
+            if (!m_jumpNeedsRelease && M_INPUT->GetActionDown(eActionCode::Jump)) m_jumpPending = true;
+        }
+        if (blocked) { m_jumpPending = false; m_jumpNeedsRelease = true; }
+        movement::Input input;
+        if (!blocked)
+        {
+            input.horizontal = int(held(eActionCode::MoveRight)) - int(held(eActionCode::MoveLeft));
+            input.vertical = int(held(eActionCode::MoveDown)) - int(held(eActionCode::MoveUp));
+            input.jump = m_jumpPending;
+        }
+        int sequence = 0;
+        if (CanMove() && m_inputSchedule.Poll(input, dt, blocked != m_inputBlocked, sequence))
+        {
+            const auto& latest = m_movement.Latest();
+            MovementPacketHandler::SendInput(latest.mapId, latest.epoch, sequence, input);
+        }
+        m_inputBlocked = blocked;
+        m_jumpPending = false;
+    }

-       Idle(true);
-
-   }
-
    void PlayerScript::LateUpdate()
    {

@@ -171,164 +137,9 @@ namespace stb
        m_animator = m_player->GetAnimator();
    }

-   void PlayerScript::UpdateAttackState()
-   {
-       stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();
-
-       if (player == nullptr)
-           return;
-
-       mAttackTimer += M_TIME->GetDeltaTime();
-
-       if (mAttackTimer < mAttackDuration)
-           return;
-
-       mAttackTimer = 0.0f;
-
-       if (IsMoveInputPressed())
-       {
-           player->SetState(PlayerState::Walk);
-           OutputDebugStringA("Attack End -> Move\n");
-       }
-       else
-       {
-           player->SetState(PlayerState::Idle);
-           OutputDebugStringA("Attack End -> Idle\n");
-       }
-   }
-
-   void PlayerScript::Idle(bool changeState)
-   {
-       if (M_UIMANAGER->IsGameplayInputBlocked())
-       {
-           return;
-       }
-
-       Transform* tr = GetOwner()->GetComponent<Transform>();
-
-       if (tr == nullptr)
-       {
-           return;
-       }
-
-       const Vector2 previousPos = tr->GetPosition();
-       Vector2 pos = previousPos;
-       bool moved = false;
-
-       const float moveSpeed = m_player->GetPlayerMoveSpeed();
-
-       if (m_player->GetState() != PlayerState::Attack)
-       {
-           float moveX = 0.0F;
-           float moveY = 0.0F;
-
-           if (M_INPUT->GetAction(eActionCode::MoveRight))
-           {
-               moveX += 1.0F;
-           }
-
-           if (M_INPUT->GetAction(eActionCode::MoveLeft))
-           {
-               moveX -= 1.0F;
-           }
-
-           if (M_INPUT->GetAction(eActionCode::MoveUp))
-           {
-               moveY -= 1.0F;
-           }
-
-           if (M_INPUT->GetAction(eActionCode::MoveDown))
-           {
-               moveY += 1.0F;
-           }
-
-           const float directionLength = std::sqrt(moveX * moveX +moveY * moveY);
-
-           if (directionLength > 0.0F)
-           {
-               // 대각선 방향을 포함해 방향 벡터의 길이를 1로 만든다.
-               moveX /= directionLength;
-               moveY /= directionLength;
-
-               const float movedDistance = moveSpeed * M_TIME->GetDeltaTime();
-
-               pos.x += moveX * movedDistance;
-               pos.y += moveY * movedDistance;
-
-               if (moveX > 0.0F)
-               {
-                   m_player->SetFacing(FacingDirection::Right);
-                   m_animator->SetFlipX(true);
-               }
-               else if (moveX < 0.0F)
-               {
-                   m_player->SetFacing(FacingDirection::Left);
-                   m_animator->SetFlipX(false);
-               }
-
-               pos = ClampPlayerPosition(pos);
-
-               // 키 입력 여부가 아닌 실제 위치 변화로 이동 상태 결정
-               moved = pos.x != previousPos.x || pos.y != previousPos.y;
-               tr->SetPosition(pos);
-               if (m_player->GetPlayerLocation() != nullptr)
-               {
-                   m_player->GetPlayerLocation()->pos = pos;
-               }
-           }
-       }
-
-       if (changeState)
-       {
-           stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();
-
-           if (player != nullptr)
-           {
-               if (moved)
-               {
-                   player->SetState(PlayerState::Walk);
-               }
-               else
-               {
-                   player->SetState(PlayerState::Idle);
-               }
-           }
-       }
-
-       SyncFollowers(pos);
-
-       if (moved)
-       {
-           mNetworkSendTimer += M_TIME->GetDeltaTime();
-
-           if (mNetworkSendTimer >= NETWORK_SEND_INTERVAL)
-           {
-               auto networkManager = stb::NetworkManager::getInstance();
-
-               if (networkManager != nullptr &&
-                   networkManager->IsConnected())
-               {
-                   MovePacketHandler::SendPlayerMove(
-                       m_player
-                   );
-               }
-
-               mNetworkSendTimer = 0.0F;
-           }
-       }
-       else
-       {
-           mNetworkSendTimer = 0.0F;
-       }
-   }
-
-   void PlayerScript::Move()
-   {
-
-   }
-
    void PlayerScript::Attack(const eSkillCode skillCode)
    {
+        if (!CanAttack()) return;
        stb::Player* player = m_player;

        if (player == nullptr)
@@ -381,20 +192,105 @@ namespace stb

    }

-   void PlayerScript::Jump()
-   {
-       if (m_player == nullptr)
-           return;
-
-       if (m_player->GetCombatSystem() == nullptr)
-           return;
-
-       if (m_player->GetState() == PlayerState::Jump)
-           return;
+   void PlayerScript::Jump()
+    {
+        if (CanMove() && !m_jumpNeedsRelease) m_jumpPending = true;
+    }

-       m_player->SetState(PlayerState::Jump);
-
-   }
+    bool PlayerScript::CanMove() const
+    {
+        return m_movement.Ready() && !movement::IsDead(m_movement.Latest()) &&
+            !movement::IsStunned(m_movement.Latest()) && m_player && !m_player->IsDead();
+    }
+    bool PlayerScript::CanAttack() const
+    {
+        return CanMove() && m_movement.Latest().mode != movement::Mode::Climbing;
+    }
+    bool PlayerScript::CanUsePortal() const
+    {
+        return CanMove() && m_movement.Latest().mode == movement::Mode::Grounded;
+    }
+    void PlayerScript::StopMovementInput()
+    {
+        if (CanMove())
+        {
+            int sequence = 0;
+            if (m_inputSchedule.Poll({}, 0, true, sequence))
+            {
+                const auto& s = m_movement.Latest();
+                MovementPacketHandler::SendInput(s.mapId, s.epoch, sequence, {});
+            }
+        }
+        m_inputBlocked = true; m_jumpPending = false; m_jumpNeedsRelease = true;
+    }
+    void PlayerScript::SuspendMovement()
+    {
+        StopMovementInput();
+        m_movement.Suspend(); m_movementVisual.Reset();
+        if (m_animator) m_animator->SetPaused(false);
+    }
+    void PlayerScript::CancelMovementSuspend()
+    {
+        m_movement.CancelSuspend(); m_inputBlocked = true;
+    }
+    void PlayerScript::ResetMovementConnection()
+    {
+        m_movement.Reset(); m_inputSchedule.Reset(); m_movementVisual.Reset();
+        m_inputBlocked = true; m_jumpPending = false; m_jumpNeedsRelease = true;
+        mAttackTimer = 0;
+        if (m_animator) m_animator->SetPaused(false);
+    }
+    void PlayerScript::OnServerDeath()
+    {
+        if (!m_player) return;
+        const bool firstDeath = !m_player->IsDead();
+        StopMovementInput();
+        m_movement.Suspend(true); m_movementVisual.Reset();
+        m_player->SetState(PlayerState::Dead);
+        if (firstDeath)
+            ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_SHOW_REVIVE, 0, 0);
+    }
+    void PlayerScript::ApplyMovementSnapshot(const movement::Snapshot& s)
+    {
+        if (!m_player || s.mapId != m_player->GetPlayerLocation()->mapId) return;
+        const bool newEpoch = !m_movement.HasSnapshot() || s.epoch != m_movement.Latest().epoch;
+        const bool wasDead = m_player->IsDead();
+        const int oldLife = m_movement.HasSnapshot() ? m_movement.Latest().lifeState : -1;
+        if (!m_movement.Push(s)) return;
+        if (newEpoch)
+        {
+            m_inputSchedule.Reset(s.sequence); m_movementVisual.Reset();
+            m_jumpPending = false; m_jumpNeedsRelease = true; m_inputBlocked = true;
+            m_player->SetState(movement::IsDead(s) ? PlayerState::Dead : PlayerState::Idle);
+        }
+        m_player->GetStat()->SetHealth(s.hp, s.maxHp);
+        if (!movement::IsDead(s) && (s.mode == movement::Mode::Climbing || movement::IsStunned(s)))
+            m_player->SetState(PlayerState::Idle);
+        if (movement::IsDead(s))
+        {
+            m_jumpPending = false; m_jumpNeedsRelease = true;
+            m_player->SetState(PlayerState::Dead);
+            if (!wasDead) ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_SHOW_REVIVE, 0, 0);
+        }
+        else if (wasDead && newEpoch)
+        {
+            m_player->SetState(PlayerState::Idle);
+            ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_REVIVE_SUCCESS, 0, 0);
+        }
+        else if (s.lifeState == static_cast<int>(movement::PlayerLife::Attack) && oldLife != s.lifeState &&
+                 !(m_player->GetState() >= PlayerState::Attack && m_player->GetState() < PlayerState::Skill_End))
+            m_player->SetState(PlayerState::Attack);
+        if (newEpoch || movement::IsDead(s))
+        {
+            if (auto* tr = m_player->GetComponent<Transform>()) tr->SetPosition({s.position.x, s.position.y});
+            m_player->GetPlayerLocation()->pos = {s.position.x, s.position.y};
+            SyncFollowers({s.position.x, s.position.y});
+        }
+#ifdef _DEBUG
+        OutputDebugStringA(("[Movement self] map=" + std::to_string(s.mapId) + " epoch=" + std::to_string(s.epoch) +
+            " sequence=" + std::to_string(s.sequence) + " tick=" + std::to_string(s.tick) + "\n").c_str());
+#endif
+    }

    void PlayerScript::PickUp()
    {
@@ -551,12 +447,4 @@ namespace stb
        }
    }

-   bool PlayerScript::IsMoveInputPressed() const
-   {
-       return M_INPUT->GetAction(eActionCode::MoveRight) ||
-           M_INPUT->GetAction(eActionCode::MoveLeft) ||
-           M_INPUT->GetAction(eActionCode::MoveUp) ||
-           M_INPUT->GetAction(eActionCode::MoveDown);
-   }
-
 }
diff --git a/LL2_Client_Win_lib/stbPlayerScript.h b/LL2_Client_Win_lib/stbPlayerScript.h
index 2dbc7c9..de6e351 100644
--- a/LL2_Client_Win_lib/stbPlayerScript.h
+++ b/LL2_Client_Win_lib/stbPlayerScript.h
@@ -4,6 +4,8 @@
 #include "..\\LL2_Client_Win_Source\\stbInput.h"
 #include "..\\LL2_Client_Win_Source\\stbAnimation.h"
 #include "..\\LL2_Client_Win_Source\\QuickSlotManager.h"
+#include "MovementStream.h"
+#include "MovementVisual.h"

 namespace stb { class Player; }
 class stbD2DRenderer;
@@ -34,10 +36,17 @@ namespace stb

        void SetPlayer(stb::Player* player) { m_player = player; }
        void SetAnimator();
+       void ApplyMovementSnapshot(const movement::Snapshot& snapshot);
+       void SuspendMovement();
+       void CancelMovementSuspend();
+       void ResetMovementConnection();
+       void OnServerDeath();
+       void StopMovementInput();
+       bool CanMove() const;
+       bool CanAttack() const;
+       bool CanUsePortal() const;
+       bool HasMovementSnapshot() const { return m_movement.HasSnapshot(); }
    private:
-       void UpdateAttackState();
-       void Idle(bool changeState = true);
-       void Move();
        void Attack(const eSkillCode skillCode = eSkillCode::None);
        void Jump();
        void PickUp();
@@ -47,11 +56,14 @@ namespace stb
        void ExecuteAction(eActionCode action);
        void SyncFollowers(Vector2 pos);

-       bool IsMoveInputPressed() const;

    private:
-       float mNetworkSendTimer;
-       const float NETWORK_SEND_INTERVAL = 0.03f;
+       movement::Stream m_movement;
+       movement::InputSchedule m_inputSchedule;
+       movement::Visual m_movementVisual;
+       bool m_inputBlocked = true;
+       bool m_jumpPending = false;
+       bool m_jumpNeedsRelease = true;
        float mAttackTimer;
        float mAttackDuration;
        GameObject* mHead;
```

</details>

<details>
<summary>LL2_Client_Win_Source/MovementDebug.h</summary>

```cpp
#pragma once
#include "stbD2DRenderer.h"
#include "stbRender.h"
#include "stbCamera.h"

namespace movement
{
    inline void DrawOriginAndFeet(stbD2DRenderer& renderer, stb::math::Vector2 origin, float footOffset)
    {
#ifdef _DEBUG
        auto feet = origin;
        feet.y += footOffset;
        if (stb::render::mainCamera)
        {
            origin = stb::render::mainCamera->CalculatePosition(origin);
            feet = stb::render::mainCamera->CalculatePosition(feet);
        }
        renderer.DrawLine(origin.x - 5, origin.y, origin.x + 5, origin.y, D2D1::ColorF(D2D1::ColorF::Magenta), 2);
        renderer.DrawLine(origin.x, origin.y - 5, origin.x, origin.y + 5, D2D1::ColorF(D2D1::ColorF::Magenta), 2);
        renderer.DrawLine(feet.x - 7, feet.y, feet.x + 7, feet.y, D2D1::ColorF(D2D1::ColorF::Cyan), 2);
#else
        (void)renderer; (void)origin; (void)footOffset;
#endif
    }
}
```

</details>

<details>
<summary>LL2_Client_PacketTests/MovementTests.cpp</summary>

```cpp
#include "MovementProtocol.h"
#include "MovementStream.h"
#include "MovementMap.h"
#include "PacketParser.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
    void Require(bool value, const char* reason)
    {
        if (!value) throw std::runtime_error(reason);
    }
    std::vector<std::string> Fields()
    {
        return {"100000000", "0", "42", "3", "18446744073709551615", "7",
            "100", "690", "0", "0", "0", "-1", "0", "0", "100", "100"};
    }
    movement::Snapshot Snapshot()
    {
        movement::Snapshot result;
        std::string error;
        Require(movement::ParseSnapshot(PacketParser::MakeBody(Fields()), result, error), "valid server snapshot rejected");
        return result;
    }
    void Protocol()
    {
        auto parsed = Snapshot();
        Require(parsed.tick == (std::numeric_limits<std::uint64_t>::max)(), "uint64 tick truncated");
        Require(parsed.position.y == 690 && parsed.sequence == 7, "origin or sequence changed");
        std::vector<std::string> input;
        Require(movement::BuildInputFields(100000000, 3, 1, {1, 0, true}, input), "valid input rejected");
        Require(input == std::vector<std::string>{"100000000", "3", "1", "1", "0", "1"}, "input not exact 6-field contract");
        Require(!movement::BuildInputFields(1, 3, 0, {}, input), "zero sequence accepted");
        Require(!movement::BuildInputFields(1, 3, 1, {2, 0, false}, input), "invalid horizontal accepted");
        auto fields = Fields();
        fields[1] = "1"; fields[5] = "0"; fields[13] = "7"; fields[14] = "0";
        std::string error;
        Require(movement::ParseSnapshot(PacketParser::MakeBody(fields), parsed, error) && movement::IsDead(parsed), "monster DEAD=7 not recognized");
        fields[13] = "4";
        Require(movement::ParseSnapshot(PacketParser::MakeBody(fields), parsed, error) && movement::IsDead(parsed), "monster DYING=4 not recognized");
        fields = Fields(); fields[13] = "5"; fields[14] = "0";
        Require(movement::ParseSnapshot(PacketParser::MakeBody(fields), parsed, error) && movement::IsDead(parsed), "player DEAD=5 not recognized");
    }
    void InvalidPackets()
    {
        const std::pair<std::size_t, const char*> invalid[] = {
            {0, "-1"}, {1, "2"}, {2, "0"}, {3, "2147483648"}, {4, "-1"},
            {4, "18446744073709551616"}, {5, "-1"}, {6, "nan"}, {7, "inf"},
            {8, "1e999"}, {9, "12garbage"}, {10, "4"}, {11, "0"}, {12, "-1"},
            {13, "6"}, {14, "101"}, {15, "-1"}, {6, ""}, {3, "1.5"}
        };
        for (const auto& [index, value] : invalid)
        {
            auto fields = Fields(); fields[index] = value;
            auto out = Snapshot(); out.entityId = 999;
            std::string error;
            Require(!movement::ParseSnapshot(PacketParser::MakeBody(fields), out, error), "invalid field accepted");
            Require(out.entityId == 999, "invalid packet partially applied");
        }
        const auto good = PacketParser::MakeBody(Fields());
        for (std::size_t n = 0; n < good.size(); ++n)
        {
            movement::Snapshot out;
            std::string error;
            Require(!movement::ParseSnapshot(good.substr(0, n), out, error), "truncated payload accepted");
        }
        auto fields = Fields(); fields.push_back("unexpected");
        movement::Snapshot out; std::string error;
        Require(!movement::ParseSnapshot(PacketParser::MakeBody(fields), out, error), "17th field accepted");
        fields = Fields(); fields[10] = "3";
        Require(!movement::ParseSnapshot(PacketParser::MakeBody(fields), out, error), "climbing without ID accepted");
    }
    void EpochAndInterpolation()
    {
        movement::Stream stream;
        auto s = Snapshot(); s.tick = 100;
        Require(stream.Push(s), "first snapshot rejected");
        movement::Snapshot view;
        Require(stream.Update(0, view) && view.position.x == 100, "first snapshot not immediate");
        s.tick = 103; s.position.x = 200;
        Require(stream.Push(s), "new tick rejected");
        stream.Update(0.025f, view);
        Require(std::abs(view.position.x - 150) < 0.01f, "50ms interpolation midpoint wrong");
        stream.Update(10, view);
        Require(view.position.x == 200, "extrapolation beyond final snapshot");
        Require(!stream.Push(s), "duplicate tick accepted");
        s.tick = 102; Require(!stream.Push(s), "old tick accepted");
        stream.Suspend();
        s.tick = 104; Require(!stream.Push(s), "old epoch resumed same-map transition");
        s.epoch = 4; s.tick = 1; s.position.x = 300;
        Require(stream.Push(s), "new epoch with smaller tick rejected");
        stream.Update(0, view);
        Require(view.position.x == 300, "new epoch blended across teleport");
        s.epoch = 3; s.tick = 999; Require(!stream.Push(s), "old epoch with newer tick accepted");
        stream.Reset(); Require(stream.Push(s), "new connection retained old epoch");
        s.tick = 1000; s.mode = movement::Mode::Falling;
        stream.Push(s); stream.Update(0.049f, view);
        s.tick = 1001;
        stream.Push(s); stream.Update(0.049f, view);
        Require(view.mode == movement::Mode::Falling, "frequent snapshots prevented mode transition");
        s.tick = 1002; s.mode = movement::Mode::Grounded;
        stream.Push(s); stream.Update(0.049f, view);
        s.tick = 1003;
        stream.Push(s); stream.Update(0.049f, view);
        Require(view.mode == movement::Mode::Grounded, "frequent snapshots prevented landing");
        stream.Reset();
        s.position.x = (std::numeric_limits<float>::max)();
        stream.Push(s);
        ++s.tick; s.position.x = -(std::numeric_limits<float>::max)();
        stream.Push(s); stream.Update(0.025f, view);
        Require(std::isfinite(view.position.x), "finite endpoints overflowed interpolation");
    }
    void DeathAndRevival()
    {
        movement::Stream stream;
        auto s = Snapshot(); s.tick = 1;
        stream.Push(s); stream.Suspend(true);
        s.tick = 2;
        Require(!stream.Push(s), "live old packet undid death event");
        s.lifeState = 5; s.hp = 0;
        Require(stream.Push(s), "same-epoch authoritative death rejected");
        movement::Snapshot displayed;
        stream.Update(0, displayed);
        Require(movement::IsDead(displayed), "death not applied immediately");
        s.tick = 3; s.lifeState = 0; s.hp = 100;
        Require(!stream.Push(s), "same-epoch live update resurrected player");
        s.epoch += 1; s.tick = 0;
        Require(stream.Push(s), "live new epoch rejected");
        Require(stream.Ready() && !movement::IsDead(stream.Latest()), "revival did not resume");
        // A later legacy 'ok' intentionally does not call Suspend/Reset.
    }
    void InputTiming()
    {
        movement::InputSchedule schedule;
        int sequence = 0;
        Require(schedule.Poll({}, 0, false, sequence) && sequence == 1, "first sequence not 1");
        Require(schedule.Poll({1, 0, true}, 0, false, sequence) && sequence == 2, "jump/change not immediate");
        Require(!schedule.Poll({1, 0, false}, 0.01f, false, sequence), "jump repeated on next frame");
        Require(schedule.Poll({1, 0, false}, 0.10f, false, sequence), "heartbeat missing");
        Require(schedule.Poll({}, 0, true, sequence), "focus/UI zero input not immediate");
        Require(!schedule.Poll({}, 0.01f, false, sequence), "neutral spammed every frame");
        schedule.Reset();
        Require(schedule.Poll({}, 0, false, sequence) && sequence == 1, "epoch did not reset sequence");
        schedule.Reset((std::numeric_limits<int>::max)());
        Require(!schedule.Poll({}, 1, true, sequence), "sequence overflowed");
    }
    void MapData()
    {
        std::ifstream file("docs/movement_data/maps.json");
        Require(file.is_open(), "run tests from repository root to load server map fixture");
        nlohmann::json exported; file >> exported;
        for (const auto& map : exported.at("maps"))
        {
            const auto geometry = movement::ParseMapGeometry(map.at("physics"));
            Require(geometry.platforms.size() == 3 && geometry.climbables.size() == 2, "server geometry incomplete");
            Require(geometry.FindClimbable(1)->ladder && !geometry.FindClimbable(2)->ladder, "rope/ladder swapped");
            std::ifstream clientFile("LL2_Client_Win/Data/Maps/" + std::to_string(map.at("mapId").get<int>()) + ".json");
            nlohmann::json client; clientFile >> client;
            Require(client.at("physics") == map.at("physics"), "client/server physics export mismatch");
            for (const auto& portal : map.at("portals"))
            {
                bool found = false;
                for (const auto& c : client.at("portals"))
                    if (c.at("id") == portal.at("id"))
                    {
                        for (auto it = portal.begin(); it != portal.end(); ++it)
                            Require(c.at(it.key()) == it.value(), "server portal field mismatch");
                        Require(c.contains("texture") && c.contains("renderSize"), "client portal art removed");
                        found = true;
                    }
                Require(found, "server portal absent");
            }
        }
        auto bad = exported.at("maps")[0].at("physics");
        bad["climbables"][1]["topPlatformId"] = 999;
        bool rejected = false;
        try { (void)movement::ParseMapGeometry(bad); } catch (const std::exception&) { rejected = true; }
        Require(rejected, "rope without exit platform accepted");
    }
}

int RunMovementTests()
{
    const std::pair<const char*, void(*)()> tests[] = {
        {"movement wire contract", Protocol}, {"movement invalid/truncated payloads", InvalidPackets},
        {"movement epoch/interpolation", EpochAndInterpolation}, {"movement death/revival", DeathAndRevival},
        {"movement input edges/heartbeat", InputTiming}, {"movement server map export", MapData}
    };
    int failed = 0;
    for (const auto& [name, run] : tests)
    {
        try { run(); std::cout << "[PASS] " << name << '\n'; }
        catch (const std::exception& e) { ++failed; std::cerr << "[FAIL] " << name << ": " << e.what() << '\n'; }
    }
    return failed;
}
```

</details>
