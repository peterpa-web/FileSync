#include "StdAfx.h"
#include "Storage.h"

#ifdef _DEBUG
void StackWalkerToTrace::OnOutput(LPCSTR szText)
{
    afxDump << szText;
}

void AssertX(BOOL b)
{
	AssertX1( b, NULL );
}

void AssertX1(BOOL b, LPCSTR szText)
{
	if ( b )
		return;
	if ( szText != NULL )
		afxDump << "ASSERT1 " << szText << "\n";
	StackWalkerToTrace sw;
	sw.ShowCallstack();
	if (AfxAssertFailedLine(__FILE__, __LINE__))
		AfxDebugBreak();
}
#endif

//CStorageObj::CStorageObj()
//{
//	m_posStorage = NULL;
//}


//CStorageObj::~CStorageObj()
//{
//    ASSERT( m_posStorage == NULL );
//}
