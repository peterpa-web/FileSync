#include "StdAfx.h"
#include "FileSync.h"

#include "ViewXml.h"

IMPLEMENT_DYNCREATE(CViewXml, CViewText)

BEGIN_MESSAGE_MAP(CViewXml, CViewText)
	ON_UPDATE_COMMAND_UI(ID_VIEW_XML, &CViewXml::OnUpdateViewXml)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXT, &CViewXml::OnUpdateViewText)
END_MESSAGE_MAP()

CViewXml::CViewXml(void)
{
	m_nTitleID = IDR_VIEWXML;
}

CViewXml::~CViewXml(void)
{
}

void CViewXml::OnUpdateViewXml(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( FALSE );
}


void CViewXml::OnUpdateViewText(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( TRUE );
}
