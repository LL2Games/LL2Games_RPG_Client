#pragma once
#include "afxdialogex.h"

class CReviveDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CReviveDlg)

public:
    explicit CReviveDlg(CWnd* pParent = nullptr);

    void ResetMessage();

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;
    void OnCancel() override;

    DECLARE_MESSAGE_MAP()
};
