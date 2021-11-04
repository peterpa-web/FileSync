#pragma once
#include "ViewText.h"

class CViewXml :
	public CViewText
{
	DECLARE_DYNCREATE(CViewXml)
public:
	CViewXml(void);
	virtual ~CViewXml(void);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateViewXml(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewText(CCmdUI *pCmdUI);
};
