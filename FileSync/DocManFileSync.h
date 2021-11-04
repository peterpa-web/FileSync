#pragma once
//#include "afxwin.h"

class CDocFileSync;
class CViewFileSync;
class CDocTemplFileSync;

class CDocManFileSync :
	public CDocManager
{
	DECLARE_DYNAMIC(CDocManFileSync)

public:
	CDocManFileSync(void);

	virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
	LPITEMIDLIST BrowseForFolder( HWND hWnd, LPCTSTR lpszTitle, UINT nFlags, LPCTSTR lpszInitDir, LPITEMIDLIST pidlOld );
	virtual CString OnDirOpen(LPITEMIDLIST pidlOld);
	void OpenDirs(const CString &strDirLeft, const CString &strDirRight);
	void OpenDirs(CDocFileSync *pDocL, CDocFileSync *pDocR);
	static void ResolveShortcut( LPTSTR pszPath, LPCTSTR lpszFileName );
	CDocTemplFileSync* FindDirTemplate( LPTSTR lpszBasePath );	// and adj. base path
	CDocument* OpenDocumentDir(LPITEMIDLIST pidl);
	CDocument* OpenDocumentDir(LPCTSTR lpszDirName);
	CDocFileSync* CreateDocumentDir(LPCTSTR lpszDirName);
	virtual void OnFileOpen();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	CDocument* OpenDocumentFile(LPCTSTR lpszFileName, CDocTemplFileSync* pBestTemplate );
	CDocTemplFileSync* FindTemplate( UINT nIDResource );
	int GetMatchingIconNo(LPCTSTR lpszFileName);
	BOOL IsReadOnly() { return m_bReadOnly; }
	CViewFileSync* GetViewDir();
	void SetType( CDocTemplFileSync* pTemplate );

	CString m_strInitialDir;	// for DoPromptFileName(), OnDirOpen(), OpenDocumentDir()
	CString m_strType;			// from CFileDialogExt

protected:

	BOOL m_bReadOnly;			// result of DoPromptFileName()
};
