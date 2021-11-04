#include "StdAfx.h"
#include "DocManFileSync.h"
#include "REBProgressDialog.h"

#include "DocFile.h"

IMPLEMENT_DYNAMIC(CDocFile, CDocFileSync)

CDocFile::CDocFile(void)
{
	ResetProgressRoutine();
}

CDocFile::~CDocFile(void)
{
}

//BOOL CDocFile::OnOpenDocument(LPCTSTR lpszPathName)
//{
//	return OnOpenDocument( lpszPathName, NULL, NULL );
//}

BOOL CDocFile::OnOpenDocument( LPCTSTR lpszPathName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	TRACE2( "CDocFile::OnOpenDocument( %s ) %s\n", lpszPathName, 
		CString(GetRuntimeClass()->m_lpszClassName) );
#ifdef _DEBUG
	if (IsModified()) {
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");
	}
#endif

	if (lpszPathName != NULL && *lpszPathName == '\0' )
	{
		DeleteContents();
		m_strPathName.Empty();
		SetModifiedFlag(FALSE);     // make clean
		return TRUE;
	}

	ASSERT1( lpProgressRoutine != NULL, "test only: no progress supported?" );

	SetProgressRoutine( lpProgressRoutine, lpData );

	CFileException fe;
	CFile* pFile = GetFile(lpszPathName,
//		CFile::modeRead|CFile::shareDenyWrite, &fe);
		CFile::modeRead|CFile::shareDenyNone, &fe);							// PP 070703
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		// PP: prepare empty file - assuming pathname is valid
		DeleteContents();
		m_strPathName = lpszPathName;
		SetModifiedFlag(FALSE);     // make clean
		ResetProgressRoutine();
		return TRUE;
	}

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	CArchive loadArchive(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive.m_pDocument = this;
	loadArchive.m_bForceFlat = FALSE;
	TRY
	{
		CWaitCursor wait;
		if (pFile->GetLength() != 0)
			Serialize(loadArchive);     // load me
		loadArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);
		DeleteContents();   // remove failed contents

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		ResetProgressRoutine();
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // start off with unmodified

	ResetProgressRoutine();
	return TRUE;
}

BOOL CDocFile::Refresh( int nSide, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData )
{
	if ( !SaveModified( nSide ) )
		return FALSE;
	// protect for DeleteContents
	BOOL bReadOnly = m_bReadOnly;
	CDocDir *pParent = m_pParent;
	POSITION posParent = m_posParent;
	BOOL b = OnOpenDocument( m_strPathName, lpProgressRoutine, lpData );
	m_pParent = pParent;
	m_posParent = posParent;
	return b;
}

