// CRegister.cpp: 구현 파일
//

//#include "pch.h"
//#include "chatClient.h"
#include "afxdialogex.h"
#include "CRegister.h"
#include "resource.h"
#include "MySocket.h"


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

int CRegister::Register(const CString &strID, const CString &strPW)
{
	int rc = EXIT_FAILURE;
	char* buff = NULL;
	int nBuffLen = 0;
	int nSendLen = 0;

	const CStringA idA(strID);
	const CStringA pwA(strPW);

	nBuffLen = idA.GetLength() + pwA.GetLength() + 20;
	buff = (char*)calloc(nBuffLen, sizeof(char));
	if (buff == NULL)
	{
		AfxMessageBox(_T("메모리 할당 실패"));
		goto err;
	}

	nSendLen = sprintf_s(buff, nBuffLen, "REGISTER$%s$%s$", idA.GetString(), pwA.GetString());

	//m_pSock->Send(buff, nSendLen);

	rc = EXIT_SUCCESS;
err:
	free(buff);
	buff = NULL;

	return rc;
}

int CRegister::OnRegister(const char* /*pID*/, const size_t /*nIDLen*/)
{
	int i;
	char* context = NULL;
	char* pLine = NULL;
	char recvBuff[2048] = { 0 };
	int rc = EXIT_FAILURE;


	m_pSock->Receive(recvBuff, 2048);
	{ char szTmp[2058]; sprintf_s(szTmp, sizeof(szTmp), "gunoo22_TEST recvBuff[%s]", recvBuff); OutputDebugStringA(szTmp); }

	if (strlen(recvBuff) == 0)
		goto err;

	for (pLine = strtok_s(recvBuff, "$", &context), i = 0; pLine; pLine = strtok_s(NULL, "$", &context), i++)
	{
		switch (i)
		{
		case 0:
			if (!strcmp(pLine, "NOK"))
			{
				CString strTmp;
				pLine = strtok_s(NULL, "$", &context);
				strTmp.Format(_T("회원가입 실패: %s"), CString(pLine));
				AfxMessageBox(strTmp);
				goto err;
			}
			break;
		}
	}

	rc = EXIT_SUCCESS;
err:

	return rc;
}
void CRegister::OnBnClickedButtonRegister()
{
	CString strID;
	CString strPw;
	CString strPwChecked;

	m_editID.GetWindowText(strID);
	m_editPasswd.GetWindowText(strPw);
	m_editPasswdChecked.GetWindowText(strPwChecked);

#if 1 //예외처리
	//ID 예외처리
	if (strID.GetLength() == 0)
	{
		AfxMessageBox(_T("ID를 입력하세요"));
		return;
	}

	//비밀번호 입력값 예외처리
	if (strPw.GetLength() == 0)
	{
		AfxMessageBox(_T("비밀번호를 입력하세요"));
		return;
	}

	if (strPw != strPwChecked)
	{
		AfxMessageBox(_T("비밀번호 입력값이 다릅니다."));
		return;
	}
#endif

	Register(strID, strPw);

	EndDialog(IDOK);
}
