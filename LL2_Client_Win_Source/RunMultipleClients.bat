@echo off
echo ========================================
echo 여러 클라이언트 실행 스크립트
echo ========================================
echo.

REM exe 파일 경로 (빌드 후 생성되는 위치)
set EXE_PATH=..\x64\Debug\WinAPITest_Client.exe

REM exe 파일이 존재하는지 확인
if not exist "%EXE_PATH%" (
    echo 오류: %EXE_PATH% 파일을 찾을 수 없습니다.
    echo 먼저 프로젝트를 빌드하세요!
    pause
    exit /b
)

echo 클라이언트 3개를 실행합니다...
echo.

REM 클라이언트 1 실행 (캐릭터 ID: 1)
echo [1] 캐릭터 ID: 1 실행 중...
start "" "%EXE_PATH%" 1

REM 1초 대기
timeout /t 1 /nobreak >nul

REM 클라이언트 2 실행 (캐릭터 ID: 2)
echo [2] 캐릭터 ID: 2 실행 중...
start "" "%EXE_PATH%" 2

REM 1초 대기
timeout /t 1 /nobreak >nul

REM 클라이언트 3 실행 (캐릭터 ID: 3)
echo [3] 캐릭터 ID: 3 실행 중...
start "" "%EXE_PATH%" 3

REM 클라이언트 4 실행 (캐릭터 ID: 4)
echo [4] 캐릭터 ID: 4 실행 중...
start "" "%EXE_PATH%" 4

REM 클라이언트 5 실행 (캐릭터 ID: 5)
echo [5] 캐릭터 ID: 5 실행 중...
start "" "%EXE_PATH%" 5

REM 클라이언트 6 실행 (캐릭터 ID: 6)
echo [6] 캐릭터 ID: 6 실행 중...
start "" "%EXE_PATH%" 6

REM 클라이언트 7 실행 (캐릭터 ID: 7)
echo [7] 캐릭터 ID: 7 실행 중...
start "" "%EXE_PATH%" 7

REM 클라이언트 8 실행 (캐릭터 ID: 8)
echo [8] 캐릭터 ID: 8 실행 중...
start "" "%EXE_PATH%" 8

REM 클라이언트 9 실행 (캐릭터 ID: 9)
echo [9] 캐릭터 ID: 9 실행 중...
start "" "%EXE_PATH%" 9

REM 클라이언트 10 실행 (캐릭터 ID: 10)
echo [10] 캐릭터 ID: 10 실행 중...
start "" "%EXE_PATH%" 10

REM 클라이언트 11 실행 (캐릭터 ID: 11)
echo [11] 캐릭터 ID: 11 실행 중...
start "" "%EXE_PATH%" 11

REM 클라이언트 12 실행 (캐릭터 ID: 12)
echo [12] 캐릭터 ID: 12 실행 중...
start "" "%EXE_PATH%" 12

REM 클라이언트 13 실행 (캐릭터 ID: 13)
echo [13] 캐릭터 ID: 13 실행 중...
start "" "%EXE_PATH%" 13



echo.
echo ========================================
echo 13개의 클라이언트가 실행되었습니다!
echo 각 클라이언트는 다른 캐릭터 ID로 접속합니다.
echo ========================================
pause
