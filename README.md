# LL2Games Tester

LL2Games RPG 서버와 통신하기 위한 MFC 기반 클라이언트 테스터입니다.
IP, 포트, 패킷 타입, 페이로드를 직접 입력해서 서버로 요청을 보내고, 수신한 패킷을 파싱해 화면에 표시합니다.

## 주요 기능

- 서버 IP / Port를 입력하고 연결 및 연결 해제를 수행합니다.
- 패킷 타입을 16진수로 입력해 요청을 전송합니다.
- 페이로드를 구분자 `$` 기준으로 여러 필드로 입력할 수 있습니다.
- 수신 패킷을 파싱해 TYPE과 FIELD 목록을 리스트 박스에 표시합니다.
- 연결 상태를 LED 색상으로 표시합니다.
- 마지막으로 사용한 IP / Port를 레지스트리에 저장해 다음 실행 시 복원합니다.

## 화면 구성

- `IP`: 서버 주소 입력
- `Port`: 서버 포트 입력
- `Type`: 전송할 패킷 타입 입력(16진수)
- `Payload`: 전송할 페이로드 입력(`$` 구분자 사용)
- `Connect`: 서버 연결
- `Disconnect`: 서버 연결 해제
- `Request`: 패킷 생성 후 전송
- `Response List`: 수신 데이터 표시

## 동작 방식

1. `Connect` 버튼을 누르면 입력한 IP / Port로 소켓을 생성하고 연결합니다.
2. `Request` 버튼을 누르면 입력값을 바탕으로 패킷을 생성해 서버로 전송합니다.
3. 서버로부터 데이터가 도착하면 수신 버퍼를 읽고 패킷 파서를 통해 해석합니다.
4. 파싱된 결과는 `TYPE`과 `PAYLOAD` 필드로 분리되어 화면에 표시됩니다.
5. 연결이 끊기면 상태 LED가 빨간색으로 바뀌고 상태 텍스트가 갱신됩니다.

## 패킷 형식

현재 테스터는 다음 구조를 기준으로 동작합니다.

```text
[PacketHeader]
- length : 전체 패킷 길이
- type   : 패킷 타입

[Body]
- payload 데이터
```

페이로드는 여러 필드의 묶음으로 다룰 수 있으며, 각 필드는 길이 정보와 값으로 해석됩니다.

## 입력 규칙

- `Type`은 16진수로 입력합니다. 예: `01`, `0001`, `0x01` 형식 중 환경에 맞게 사용
- `Payload`는 `$`로 필드를 구분합니다.
- 각 필드는 UTF-8 문자열로 처리되어 수신 시 한글 등도 표시할 수 있습니다.

예시:

```text
Type: 0001
Payload: user01$password123$Seoul
```

## 빌드 환경

- Visual Studio
- MFC
- Windows / Winsock

## 빌드 방법

1. `LL2Games_Tester.sln`을 Visual Studio로 엽니다.
2. 구성(Configuration)과 플랫폼(Platform)을 선택합니다.
3. `Build` 또는 `Rebuild`를 실행합니다.

## 사용 예시

1. 서버 IP와 Port를 입력합니다.
2. `Connect`를 눌러 접속합니다.
3. `Type`과 `Payload`를 입력합니다.
4. `Request`를 눌러 서버로 데이터를 전송합니다.
5. 수신 응답은 리스트 박스에서 확인합니다.

## 관련 파일

- [LL2Games_TesterDlg.cpp](LL2Games_TesterDlg.cpp)
- [MySocket.cpp](MySocket.cpp)
- [PacketParser.h](PacketParser.h)
- [Packet.h](Packet.h)
- [UTIL.cpp](UTIL.cpp)

## 참고

이 프로젝트는 서버 테스트용 도구이므로, 실제 게임 클라이언트와는 별개의 용도로 사용하는 것이 좋습니다.
