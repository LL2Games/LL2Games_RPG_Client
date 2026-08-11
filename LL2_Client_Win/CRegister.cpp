// CRegister.cpp: 구현 파일
//

//#include "pch.h"
//#include "chatClient.h"
#include "afxdialogex.h"
#include "CRegister.h"
#include "resource.h"
#include "MySocket.h"

#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"
#include "..\\LL2_Client_Win_Source\\Util.h"

static BOOL CheckIdValidate(const CString& strName, CString& strErrMsg)
{
	if (strName.GetLength() == 0)
	{
		strErrMsg = _T("ID를 입력하세요");
		return FALSE;
	}

	//글자수 제한
	if (strName.GetLength() < 5 || strName.GetLength() > 20)
	{
		strErrMsg = _T("ID는 최소 5자리 이상 20자리 이하여야 합니다.");
		return FALSE;
	}

	CStringW wName(strName);  // 유니코드 변환
	for (int i = 0; i < wName.GetLength(); ++i)
	{
		WCHAR ch = wName[i];
		if (!(
			(ch >= L'A' && ch <= L'Z') ||
			(ch >= L'a' && ch <= L'z') ||
			(ch >= L'0' && ch <= L'9')
			))
		{
			strErrMsg = _T("아이디는 영어, 숫자로만 이루어져야 합니다");
			return FALSE;
		}
	}

	return TRUE;
}

static BOOL CheckPwdValidate(const CString& strPwd, CString& strErrMsg, BOOL bComplexity = FALSE)
{
	if (strPwd.IsEmpty())
	{
		strErrMsg = _T("비밀번호를 입력하세요.");
		return FALSE;
	}

	// bComplexity == TRUE이면 복잡도 검사 생략
	if (bComplexity)
	{
		return TRUE;
	}

	const int nPassLen = strPwd.GetLength();

	// 길이 제한
	if (nPassLen < 9 || nPassLen > 50)
	{
		strErrMsg = _T("비밀번호는 최소 9자리 이상 50자리 이하여야 합니다.");
		return FALSE;
	}

	BOOL bAlphaUP = FALSE;	// 영문 대문자
	BOOL bAlphaLO = FALSE;	// 영문 소문자
	BOOL bDigit = FALSE;	// 숫자
	BOOL bSpecial = FALSE;	// 특수문자

	int nSame = 0;			// 동일 문자 연속 횟수
	int nASC = 0;			// 연속 문자 오름차순
	int nDESC = 0;			// 연속 문자 내림차순

	for (int i = 0; i < nPassLen; ++i)
	{
		TCHAR ch = strPwd[i];

		// 문자 종류 확인
		if (ch >= _T('A') && ch <= _T('Z'))
		{
			bAlphaUP = TRUE;
		}
		else if (ch >= _T('a') && ch <= _T('z'))
		{
			bAlphaLO = TRUE;
		}
		else if (ch >= _T('0') && ch <= _T('9'))
		{
			bDigit = TRUE;
		}
		else
		{
			bSpecial = TRUE;
		}

		// 같은 문자 3번 이상 연속 사용 금지
		if (i > 0 && strPwd[i] == strPwd[i - 1])
		{
			nSame++;

			if (nSame > 1)
			{
				strErrMsg = _T("같은 문자를 3자리 이상 연속으로 사용할 수 없습니다.");
				return FALSE;
			}
		}
		else
		{
			nSame = 0;
		}

		// 연속 문자 3자리 이상 금지
		if (i > 0)
		{
			int nDiff = strPwd[i] - strPwd[i - 1];

			if (nDiff == 1)
			{
				nASC++;
				nDESC = 0;
			}
			else if (nDiff == -1)
			{
				nDESC++;
				nASC = 0;
			}
			else
			{
				nASC = 0;
				nDESC = 0;
			}

			if (nASC > 1 || nDESC > 1)
			{
				strErrMsg = _T("연속된 문자를 3자리 이상 사용할 수 없습니다.");
				return FALSE;
			}
		}
	}

	int nVariable = 0;

	if (bAlphaUP)
		nVariable++;

	if (bAlphaLO)
		nVariable++;

	if (bDigit)
		nVariable++;

	if (bSpecial)
		nVariable++;

	// 특수문자 필수
	if (!bSpecial)
	{
		strErrMsg = _T("비밀번호에는 특수문자가 1개 이상 포함되어야 합니다.");
		return FALSE;
	}

	// 대문자 / 소문자 / 숫자 / 특수문자 중 3종류 이상
	if (nVariable < 3)
	{
		strErrMsg = _T("비밀번호는 영문 대문자, 영문 소문자, 숫자, 특수문자 중 3종류 이상을 조합해야 합니다.");
		return FALSE;
	}

	return TRUE;
}


// CRegister 대화 상자

IMPLEMENT_DYNAMIC(CRegister, CDialogEx)

CRegister::CRegister(CMySocket* sock, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGISTER, pParent), m_pSock(sock)
{

}

CRegister::~CRegister()
{
}

void CRegister::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ID, m_editID);
	DDX_Control(pDX, IDC_EDIT_PASSWD, m_editPasswd);
	DDX_Control(pDX, IDC_EDIT_PASSWD_CHECK, m_editPasswdChecked);
}


BEGIN_MESSAGE_MAP(CRegister, CDialogEx)
	ON_BN_CLICKED(ID_BUTTON_REGISTER, &CRegister::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// CRegister 메시지 처리기

int CRegister::Register(const CString &strID, const CString &strPW, const CString& strPWCheck)
{
	int rc = EXIT_FAILURE;

	const CStringA idA(strID);
	const CStringA pwA(strPW);
	const CStringA pwCheckA(strPWCheck);

	std::vector<std::string> datas;
	std::string body, pkt;

	datas.push_back(std::string(idA));
	datas.push_back(std::string(pwA));
	datas.push_back(std::string(pwCheckA));

	body = PacketParser::MakeBody(datas);
	pkt = PacketParser::MakePacket(PKT_REGISTER, body);

	m_pSock->m_status = E_REGISTER; //OnRegister를 받기 위함
	m_pSock->SendPacket(pkt);

	rc = EXIT_SUCCESS;

	return rc;
}

int CRegister::OnRegister(const char* recvBuff, const size_t recvLen)
{
	size_t offset = 0;
	std::string errMsg;
	CString strCharList;

	std::string sBuff;
	std::vector<char> vBuff;
	CString wideValue;

	sBuff.append(recvBuff, recvLen);
	vBuff.insert(vBuff.end(), sBuff.begin(), sBuff.end());
	auto pkt = PacketParser::Parse(vBuff);
	if (!pkt.has_value())
	{
		return -1;
	}

	std::string status;

	if (!PacketParser::ParseLengthPrefixedString(
		pkt->payload.c_str(),
		pkt->payload.size(),
		offset,
		status,
		errMsg))
	{
		//더이상 없으면 중단
		//K_slog_trace(K_SLOG_DEBUG, "[%s][%d]gunoo22_TEST", __FUNCTION__, __LINE__);
		return -1;
	}

	if (status == "nok")
	{
		AfxMessageBox(_T("회원가입 실패"));
	}
	else
	{
		AfxMessageBox(_T("회원가입 성공"));
		
	}


	return 0;
}
void CRegister::OnBnClickedButtonRegister()
{
	CString strErrMsg;
	CString strID;
	CString strPw;
	CString strPwChecked;

	m_editID.GetWindowText(strID);
	m_editPasswd.GetWindowText(strPw);
	m_editPasswdChecked.GetWindowText(strPwChecked);

#if 1 //예외처리
	//ID 예외처리
	if (!CheckIdValidate(strID, strErrMsg))
	{
		AfxMessageBox(strErrMsg);
		return;
	}

	//비밀번호 입력값 예외처리
	if (strPw != strPwChecked)
	{
		AfxMessageBox(_T("비밀번호 입력값이 다릅니다."));
		return;
	}

	if (!CheckPwdValidate(strPw, strErrMsg))
	{
		AfxMessageBox(strErrMsg);
		return;
	}

#endif

	Register(strID, strPw, strPwChecked);

	EndDialog(IDOK);
}
