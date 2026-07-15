# 여러 클라이언트 동시 테스트 방법

## 방법 1: 같은 PC에서 여러 클라이언트 실행

### 1단계: 프로젝트 복사
```
WinAPITest-master 폴더를 복사해서 3개 만들기:
- WinAPITest-Client1
- WinAPITest-Client2
- WinAPITest-Client3
```

### 2단계: 각 클라이언트의 캐릭터 ID 설정

**Client1**: `WinAPITest-Client1/LL2_Client_Win_Source/stbNetworkConfig.h`
```cpp
static constexpr const char* CHARACTER_ID = "1";
```

**Client2**: `WinAPITest-Client2/LL2_Client_Win_Source/stbNetworkConfig.h`
```cpp
static constexpr const char* CHARACTER_ID = "2";
```

**Client3**: `WinAPITest-Client3/LL2_Client_Win_Source/stbNetworkConfig.h`
```cpp
static constexpr const char* CHARACTER_ID = "3";
```

### 3단계: 각각 빌드 및 실행
1. Client1 폴더 열기 → 빌드 → 실행
2. Client2 폴더 열기 → 빌드 → 실행
3. Client3 폴더 열기 → 빌드 → 실행

### 4단계: 테스트
- 각 클라이언트에서 방향키로 이동
- 다른 클라이언트의 출력 창에서 이동 로그 확인
- 서버에서 모든 클라이언트의 이동 패킷 확인

---

## 방법 2: 빌드된 exe 파일로 실행

### 1단계: exe 파일 찾기
```
WinAPITest-master/x64/Debug/LL2_Client_Win.exe
```

### 2단계: exe 파일 3개 복사
```
LL2_Client_Win1.exe
LL2_Client_Win2.exe
LL2_Client_Win3.exe
```

### 3단계: 각 exe 실행
- 3개의 exe를 동시에 실행
- 모두 같은 캐릭터 ID "1"로 접속됨 (구분 안 됨)

**주의**: 이 방법은 모두 같은 설정을 사용하므로 제대로 된 테스트가 안 됩니다.
**방법 1을 추천합니다!**

---

## 방법 3: 명령줄 인자로 캐릭터 ID 전달 (고급)

### 코드 수정 필요
`LL2_Client_Win.cpp`의 `wWinMain`에서:
```cpp
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    // 명령줄 인자로 캐릭터 ID 받기
    if (wcslen(lpCmdLine) > 0)
    {
        // lpCmdLine을 파싱해서 CHARACTER_ID 설정
    }
    
    // ... 기존 코드
}
```

실행:
```
LL2_Client_Win.exe 1
LL2_Client_Win.exe 2
LL2_Client_Win.exe 3
```

---

## 테스트 시나리오

### 시나리오 1: 기본 이동 동기화
1. Client1 실행 → 캐릭터 ID "1"로 접속
2. Client2 실행 → 캐릭터 ID "2"로 접속
3. Client1에서 방향키로 이동
4. Client2의 출력 창에서 "Char 1 -> (x, y)" 로그 확인

### 시나리오 2: 동시 이동
1. 3개 클라이언트 모두 실행
2. 각각 다른 방향으로 동시에 이동
3. 모든 클라이언트에서 다른 플레이어들의 이동 로그 확인

### 시나리오 3: 순차 접속
1. Client1 접속 → 이동
2. Client2 접속 → Client1의 위치 확인
3. Client3 접속 → Client1, 2의 위치 확인

---

## 출력 창에서 확인할 내용

### 자신의 이동
```
(로그 없음 - 서버로만 전송)
```

### 다른 플레이어 이동
```
[이동] Char 2 -> (150.5, 200.3) speed: 20.0
[이동] Char 3 -> (300.0, 100.0) speed: 20.0
```

---

## 현재 설정

- **서버 IP**: 100.99.220.45
- **포트**: 1234
- **기본 캐릭터 ID**: "1"
- **맵 ID**: "100000000"

**여러 클라이언트 테스트 시 각 클라이언트의 CHARACTER_ID를 다르게 설정하세요!**
