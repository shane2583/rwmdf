
// FileInfo4.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "MDF4Viewer.h"
#include "FileInfo4.h"
#include "FindDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


typedef struct fi4type {
	int type;
	int nummer; // 0-relativ
	union {
		mdfFileId *pId;  // type = 1
		M4HDBlock *pHdr; // type = 2
		M4DGBlock *pDG;  // type = 3 or 6 (Data)
		M4CNBlock *pCN;  // type = 5
		M4CCBlock *pCC;  // type = 7
		M4EVBlock *pEV;  // type = 8
		M4FHBlock *pFH;  // type = 9
		TCHAR    *pszXML;// type = 10;
		M4SRBlock *pSR;  // type = 11
		M4DTBlock *pDT;  // type = 12
		M4RDBlock *pRD;  // type = 13
		M4DLBlock *pDL;  // type = 14
		M4CABlock *pCA;  // type = 15
		M4HLBlock *pHL;  // type = 101
		M4DZBlock *pDZ;  // type = 102
	} u;
	M4CGBlock *pCG;  // type = 4 or 6 (Data)
	CMdf4Calc *pCalc;// type = 6 (Data)
	struct fi4type* pChans; // Pointers auf *FI4Type der Kanäle
	int nChans;
	HTREEITEM hItem;
	BOOL bNotYetInitialized;
} FI4Type;

/////////////////////////////////////////////////////////////////////////////
// CInfo4Tree

CInfo4Tree::CInfo4Tree()
{
  m_bCase = FALSE;
  m_bWords = FALSE;
  m_nFindIdx = 0;
  m_nLastFindIdx = 0;
	m_bStreamAllocated = FALSE;
	m_pScan = NULL;
}

CInfo4Tree::~CInfo4Tree()
{
	if (m_pScan)
		free(m_pScan);
}

BEGIN_MESSAGE_MAP(CInfo4Tree, CTreeCtrl)
	//{{AFX_MSG_MAP(CInfo4Tree)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfo4Tree message handlers

void CInfo4Tree::OnRClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  CFileInfo4 *pDlg = (CFileInfo4 *)GetParent();
  HTREEITEM hItem = GetSelectedItem( );

	
	*pResult = 0;
}

void CInfo4Tree::CleanUp(HTREEITEM hParent)
{
	HTREEITEM hNode = GetChildItem(hParent);
	while (hNode)
	{
    FI4Type *pEntry = (FI4Type*)GetItemData(hNode);
		if (pEntry)
		{
			switch (pEntry->type)
			{
			case 1: /* pEntry->u.pId; */ break;
			case 2: /* pEntry->u.pHdr; */ break;
			case 3: delete pEntry->u.pDG; break;
			case 4: delete pEntry->pCG; break;
			case 5: delete pEntry->u.pCN; break;
			case 6:	if (pEntry->pChans)
								free(pEntry->pChans);
				      break;
			case 7: delete pEntry->u.pCC; break;
			case 8: delete pEntry->u.pEV; break;
			case 9: delete pEntry->u.pFH; break;
			case 10: free(pEntry->u.pszXML); break;
			case 11: delete pEntry->u.pSR; break;
			case 12: delete pEntry->u.pDT; break;
			case 13: delete pEntry->u.pRD; break;
			case 14: delete pEntry->u.pDL; break;
			case 15: delete pEntry->u.pCA; break;
			case 101: delete pEntry->u.pHL; break;
			case 102: delete pEntry->u.pDZ; break;
			}
			if (pEntry->pCalc)
				delete pEntry->pCalc;
			free(pEntry);
			SetItemData(hNode,0);
		}
		
		if (ItemHasChildren(hNode))
		{
			HTREEITEM hChildNode = GetChildItem(hNode);
			CleanUp(hNode);
		}
		hNode = GetNextSiblingItem(hNode);
	}
}

CMdf4DataGroup* CInfo4Tree::FindDataGroup(std::vector<CMdf4DataGroup*> &vDataGroups, int ithGroup)
{
  int i, n;
  if(vDataGroups.size()==0)
    return NULL;
  CFileInfo4 *pParent = (CFileInfo4 *)GetParent();
  if(pParent)
    pParent->m_pCurDataGroup=NULL;
  CMdf4DataGroup* pGroup = vDataGroups[0];
  n = vDataGroups.size();
  for(i=0; i<n; i++)
  {
    CMdf4DataGroup* pGroup = vDataGroups[i];
    if(pGroup->m_iAllGrp == ithGroup)
    {
      if(pParent)
        pParent->m_pCurDataGroup=pGroup;
      return pGroup;
    }
  }
  return NULL;
}

void CInfo4Tree::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

  CFileInfo4 *pDlg = (CFileInfo4 *)GetParent();

  if (pNMTreeView->itemNew.hItem)
  {
    FI4Type *pEntry = (FI4Type*)GetItemData(pNMTreeView->itemNew.hItem);
		if (pEntry)
		{
			RECT r;
			if (pEntry->type == 6) // Data
			{
				int change = 0;
				if (pDlg->m_pCurCtrl == &pDlg->m_List || pDlg->m_pCurCtrl == &pDlg->m_XMLTree)
					change = 1;
				if (m_pGroupEntryPtr != (void*)pEntry)
					change |= 2;
				if (change)
				{
					pDlg->GetDlgItem(IDC_EXPORT)->EnableWindow(TRUE);
				}
				if (change & 1)
				{
					pDlg->m_pCurCtrl->ShowWindow( SW_HIDE );
					pDlg->m_pCurCtrl->GetWindowRect(&r);
					pDlg->m_pCurCtrl = &pDlg->m_DataList;
					pDlg->ScreenToClient(&r);
					pDlg->m_pCurCtrl->MoveWindow(&r);
					pDlg->m_pCurCtrl->ShowWindow( SW_NORMAL );
				}
				if (change & 2)
				{
					pDlg->m_bOneCol = FALSE;
					if (pDlg->m_Cache)
					{
						TCHAR **p = pDlg->m_Cache;
						for (DWORD i=0; i<pDlg->m_dwInCache; i++,p++)
							free(*p);
						free(pDlg->m_Cache);
						pDlg->m_Cache = NULL;
					}
					pDlg->m_nChan = pEntry->nChans;
					pDlg->m_dwInCache = pEntry->pCG->cg_cycle_count * pEntry->nChans;
					pDlg->m_Cache = (TCHAR**)calloc(pDlg->m_dwInCache,sizeof(char**));
					pDlg->m_bShowingData = TRUE;
					pDlg->m_DataList.SetItemCountEx( pEntry->pCG->cg_cycle_count);
					pDlg->GetDlgItem(IDC_EXPORT)->EnableWindow(TRUE);
					m_pGroupEntryPtr = (void*)pEntry;
					PrepareData(pDlg->m_DataList, pDlg->m_Tree, m_pGroupEntryPtr);
				}
			}
			else if (pEntry->type == 9 || pEntry->type == 10) // FH or XML
			{
				pDlg->m_pCurCtrl->ShowWindow( SW_HIDE );
				pDlg->m_pCurCtrl->GetWindowRect(&r);
				pDlg->m_pCurCtrl = &pDlg->m_XMLTree;
				pDlg->ScreenToClient(&r);
				pDlg->m_pCurCtrl->MoveWindow(&r);
				pDlg->m_pCurCtrl->ShowWindow( SW_NORMAL );
			}
			else if (pDlg->m_pCurCtrl != &pDlg->m_List)
			{
				if (m_bStreamAllocated && m_pGroupEntryPtr)
				{
					FI4Type *pEntry = (FI4Type *)m_pGroupEntryPtr; 
					int iGroup = pEntry->nummer;
          CMdf4DataGroup *pDatagroup = FindDataGroup(pDlg->m_pMDF->m_vDataGroups, iGroup);
					delete pDatagroup->m_pDataStream;
					pDatagroup->m_pDataStream = NULL;
				}
        pDlg->m_bOneCol = FALSE;
        if (pDlg->m_Cache)
        {
          TCHAR **p = pDlg->m_Cache;
          for (DWORD i=0; i<pDlg->m_dwInCache; i++,p++)
            free(*p);
          free(pDlg->m_Cache);
          pDlg->m_Cache = NULL;
        }

				pDlg->m_pCurCtrl->ShowWindow( SW_HIDE );
				pDlg->m_pCurCtrl->GetWindowRect(&r);
				pDlg->m_pCurCtrl = &pDlg->m_List;
				pDlg->ScreenToClient(&r);
				pDlg->m_pCurCtrl->MoveWindow(&r);
				pDlg->m_pCurCtrl->ShowWindow( SW_NORMAL );
			}
			if (pEntry->type != 6) // Data
			{
				pDlg->GetDlgItem(IDC_EXPORT)->EnableWindow(FALSE);
			}
			switch (pEntry->type)
			{
			case 1: DisplayIDBlock(pDlg, pEntry->u.pId); break;
			case 2: DisplayHdrBlock(pDlg, pEntry->u.pHdr); break;
			case 3: DisplayDGBlock(pDlg, pEntry->u.pDG); break;
			case 4: DisplayCGBlock(pDlg, pEntry->pCG); break;
			case 5: DisplayCNBlock(pDlg, pEntry->u.pCN); break;
			case 6: DisplayData(pDlg, pEntry->u.pDG); break;
			case 7: DisplayCCBlock(pDlg, pEntry->u.pCC); break;
			case 8: DisplayEVBlock(pDlg, pEntry->u.pEV); break;
			case 9: DisplayFHBlock(pDlg, pEntry->u.pFH); break;
			case 10: DisplayXML(pDlg, pEntry->u.pszXML); break;
			case 11: DisplaySRBlock(pDlg, pEntry->u.pSR); break;
			case 12: DisplayDTBlock(pDlg, pEntry->u.pDT); break;
			case 13: DisplayRDBlock(pDlg, pEntry->u.pRD); break;
			case 14: DisplayDLBlock(pDlg, pEntry->u.pDL); break;
			case 15: DisplayCABlock(pDlg, pEntry->u.pCA); break;
			case 101: DisplayHLBlock(pDlg, pEntry->u.pHL); break;
			case 102: DisplayDZBlock(pDlg, pEntry->u.pDZ); break;
			}
		}
  }
	*pResult = 0;
}

void CInfo4Tree::DisplayData(CFileInfo4 *pDlg, M4DGBlock *pDG)
{
	pDlg->m_DataList.LockWindowUpdate();
	pDlg->m_DataList.UnlockWindowUpdate();
}

void CInfo4Tree::PrepareData(CListCtrl& List, CTreeCtrl& Tree, void *ptr)
{
  HDITEM Item;
  int i;
  CString str;
  CWaitCursor w;
	FI4Type *pEntry = (FI4Type *)ptr; 
  List.DeleteAllItems();
  List.InsertItem(0,_T("nix"));
  CHeaderCtrl *pHdr = List.GetHeaderCtrl();
  while ((i=pHdr->GetItemCount()) > 0)
  {
    List.DeleteColumn(i-1);
  }
	List.InsertColumn(0, _T("Index"));
  memset(&Item, 0, sizeof(Item));
  Item.mask = HDI_TEXT;
  Item.cchTextMax = 6;
	Item.pszText = _T("Index");
  pHdr->SetItem( 0, &Item);
  List.SetColumnWidth(0, 50);
  List.LockWindowUpdate();
	for (i = 1; i <= pEntry->nChans; i++)
  {
		str = Tree.GetItemText(pEntry->pChans[i-1].hItem);
    Item.pszText = str.LockBuffer();
    if (i >= pHdr->GetItemCount())
      List.InsertColumn( i, str, LVCFMT_RIGHT, 80);
    else
      pHdr->SetItem( i, &Item);
    str.UnlockBuffer();
    List.SetColumnWidth(i, 80);
  }
	List.SetItemCountEx( pEntry->pCG->cg_cycle_count);
  List.UnlockWindowUpdate();

	m_iScanSize = pEntry->pCG->cg_data_bytes + pEntry->pCG->cg_inval_bytes + pEntry->u.pDG->dg_rec_id_size;
  m_i64LastScanIndex = 0;
  m_pScan = (BYTE*)calloc(1,m_iScanSize);

  GetScan(0, ptr);
}

BOOL CInfo4Tree::GetScan(unsigned __int64 i64Index, void *ptr)
{
	BOOL bResult = FALSE;
  CFileInfo4 *pDlg = (CFileInfo4 *)GetParent();
	FI4Type *pEntry = (FI4Type *)ptr; 
	int iGroup = pEntry->nummer;
  CMdf4DataGroup *pDatagroup = FindDataGroup(pDlg->m_pMDF->m_vDataGroups, iGroup);
  if(pDatagroup)
  {
    bResult = pDatagroup->GetRecord(pEntry->pCG, m_pScan, i64Index, i64Index);
    if(bResult)
      m_i64LastScanIndex = i64Index;
    else
      memset(m_pScan, 0, m_iScanSize);
  }
	return bResult;
}

CString CInfo4Tree::Show(int iItem, int iSubItem, BOOL bPhysVal)
{
  CFileInfo4 *pDlg = (CFileInfo4 *)GetParent();
	FI4Type *pEntry = (FI4Type *)m_pGroupEntryPtr; 
	BOOL bNoval;
	CString str;
	BOOL bReadOK = TRUE;
	double value;
	FI4Type *pChan = &pEntry->pChans[iSubItem-1];
  CMdf4DataGroup *pDatagroup = FindDataGroup(pDlg->m_pMDF->m_vDataGroups, pEntry->nummer);
  if(pDatagroup->m_pDataStream == NULL)
	{
		pDatagroup->m_pDataStream = pEntry->u.pDG->ReadStream();
		m_bStreamAllocated = TRUE;
	}
	else
		m_bStreamAllocated = FALSE;
  if (iItem != m_i64LastScanIndex)
    bReadOK = GetScan(iItem,m_pGroupEntryPtr);
	if (bReadOK)
	{
		bNoval = FALSE;
		M4CNBlock *cn = pChan->u.pCN;
		if (cn->cn_type == CN_T_VMASTER)
		{
			if (pChan->pCalc)
			{
				value = pChan->pCalc->MdfCalc(iItem);
			}
			else
			{
				value = 1E308;
				bNoval = TRUE;
			}
		}
		else if (cn->cn_flags & CN_F_VIRTUAL)
		{
			value = 1E308;
			bNoval = TRUE;
		}
		else
		  bNoval = pDatagroup->GetRawValueFromRecord(pEntry->pCG, pChan->u.pCN, m_pScan, &value);
		if (bNoval)
			str.Format(_T("! %.15lg"), value);
		else
		{
			if (bPhysVal && pChan->pCalc)
				value = pChan->pCalc->MdfCalc(value);
			str.Format(_T("%.15lg"), value);
		}
	}
	else
		str = _T("read err");
	return str;
}

void CInfo4Tree::Export()
{
  void yield(int i);
  int i; UINT j;
  CString str;
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("CSV-Datei (*.csv)||"));
  if (dlg.DoModal() == IDOK)
  {
    TCHAR buf[80];
    TCHAR dez,list;
		GetProfileString(_T("intl"), _T("sDecimal"), _T("."), buf, 40);
    dez = buf[0];
		GetProfileString(_T("intl"), _T("sList"), _T("."), buf, 40);
    list = buf[0];
    yield(0);
    {
      CWaitCursor cw;
			FILE *f = _tfopen(dlg.GetPathName(), _T("wt"));
      if (f==NULL)
      {
				AfxMessageBox(_T("Kann die Textdatei nicht erzeugen!"));
        return;
			}

		  CFileInfo4 *pDlg = (CFileInfo4 *)GetParent();
			FI4Type *pEntry = (FI4Type *)m_pGroupEntryPtr; 
			DWORD dwN = pEntry->pCG->cg_cycle_count;
 			for (i = 0; i < pEntry->nChans; i++)
			{
				str = GetItemText(pEntry->pChans[i].hItem);
        if (i)
          fprintf(f,"%c",list);
        fprintf(f,"%s",str);
      }
      fprintf(f,"\n");
 			for (i = 0; i < pEntry->nChans; i++)
			{
				M4CNBlock *pCN = pEntry->pChans[i].u.pCN;
				M4TXBlock *tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCN, M4CNBlock::cn_md_unit);
				if (tx)
				{
					str = pDlg->m_pMDF->GetString(tx);
					delete tx;
				}
				else str = _T(" ");
        if (i)
          fprintf(f,"%c",list);
        fprintf(f,"%s",str);
      }
      fprintf(f,"\n");
      for (j=0; j<dwN; j++)
      {
				for (i = 0; i < pEntry->nChans; i++)
				{
          if (i)
            fprintf(f,"%c",list);
          str = Show(j, i+1, pDlg->m_bPhysVal); // i=0 -> Index!
          str.Replace('.',dez);
          fprintf(f,"%s",str);
        }
        fprintf(f,"\n");
      }
      fclose(f);
    }
		str.Format(_T("Die Datei %s wurde erfolgreich exportiert!"), dlg.GetPathName());
    AfxMessageBox(str);
  }
}

CString StrID(M_UINT16 hdrID)
{
	TCHAR szBuffer[8];
	for (int i=0; i<4; i++)
	{
		szBuffer[i] = hdrID & 0xFF;
		hdrID >>= 8;
	}
	return szBuffer;
}

void CInfo4Tree::DisplayDGBlock(CFileInfo4 *pDlg, M4DGBlock *pDG)
{
	int i;
	TCHAR buf[40];
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("Next Data Group"));
	_stprintf(buf, _T("0x%I64X"), pDG->getLink(M4DGBlock::dg_dg_next));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("1st Channel Group"));
	_stprintf(buf, _T("0x%I64X"), pDG->getLink(M4DGBlock::dg_cg_first));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("1st Data Block"));
	m4Block *b = NULL;
	M_LINK l = pDG->getLink(M4DGBlock::dg_data);
	if (l)
		b = pDlg->m_pMDF->m_m4.LoadLink( *pDG, M4DGBlock::dg_data);
	if (b)
		_stprintf(buf, _T("0x%I64X ID = %s"), l, StrID(b->hdrID()));
	else
		_stprintf(buf, _T("0x%I64X"), l);
	
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Record ID Size"));
	_stprintf(buf, _T("%u"), (UINT)pDG->dg_rec_id_size);
	pList->SetItemText( i++, 1, buf);

	DisplayM4Block( pDlg, *pDG, i);
}

void CInfo4Tree::DisplayCGBlock(CFileInfo4 *pDlg, M4CGBlock *pCG)
{
	int i;
	TCHAR buf[40];
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("Next Channel Group"));
	_stprintf(buf, _T("0x%I64X"), pCG->getLink(M4CGBlock::cg_cg_next));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("1st Channel"));
	_stprintf(buf, _T("0x%I64X"), pCG->getLink(M4CGBlock::cg_cn_first));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Acquisition Name"));
	M4TXBlock *tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCG, M4CGBlock::cg_tx_acq_name);
	if (tx)
	{
		str = pDlg->m_pMDF->GetString(tx);
		delete tx;
	}
	else str = _T(" ");
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Acquisition Source"));
	M4SIBlock *si = (M4SIBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCG, M4CGBlock::cg_si_acq_source);
	if (si)
	{
		tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *si, M4SIBlock::si_tx_name);
		if (tx)
		{
			str = pDlg->m_pMDF->GetString(tx);
			delete tx;
		}
		delete si;
	}
	else str = _T(" ");
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Record ID"));
	_stprintf(buf, _T("%I64u"), pCG->cg_record_id);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Cycle Count"));
	_stprintf(buf, _T("%I64u"), pCG->cg_cycle_count);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Data Bytes"));
	_stprintf(buf, _T("%lu"), pCG->cg_data_bytes);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Invalid Bytes"));
	_stprintf(buf, _T("%lu"), pCG->cg_inval_bytes);
	pList->SetItemText( i++, 1, buf);

	DisplayM4Block( pDlg, *pCG, i);
}

void CInfo4Tree::DisplaySRBlock(CFileInfo4 *pDlg, M4SRBlock *pSR)
{
	int i;
	TCHAR buf[40];
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("Next SR block"));
	_stprintf(buf,_T("0x%I64X"),  pSR->getLink(M4SRBlock::sr_sr_next));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Data Block"));
	_stprintf(buf,_T("0x%I64X"),  pSR->getLink(M4SRBlock::sr_data));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Cycle Count"));
	str.Format(_T("%I64u"),pSR->sr_cycle_count);
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Interval [s]"));
	_stprintf(buf,_T("%lg"),  pSR->sr_interval);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Sync type"));
	switch (pSR->sr_sync_type)
	{
	case SR_S_TIME: str = _T("1 = TIME"); break;
	case SR_S_ANGLE: str = _T("2 = ANGLE"); break;
	case SR_S_DISTANCE: str = _T("3 = DISTANCE"); break;
	case SR_S_INDEX: str = _T("4 = INDEX"); break;
	}
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Flags"));
	if (pSR->sr_flags & SR_F_INVALIDATION)
		str = _T("Invalidation bytes");
	else
		str = _T("No invalidation bytes");
	pList->SetItemText( i++, 1, str);

	DisplayM4Block( pDlg, *pSR, i);
}

void CInfo4Tree::DisplayCNBlock(CFileInfo4 *pDlg, M4CNBlock *pCN)
{
	int i,j;
	TCHAR buf[40], *p;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("Next Channel"));
	_stprintf(buf, _T("0x%I64X"), pCN->getLink(M4CNBlock::cn_cn_next));
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Name"));
	M4TXBlock *tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCN, M4CNBlock::cn_tx_name);
	if (tx)
	{
		str = pDlg->m_pMDF->GetString(tx);
		delete tx;
	}
	else str = _T(" ");
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Source"));
	M4SIBlock *si = (M4SIBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCN, M4CNBlock::cn_si_source);
	if (si)
	{
		tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *si, M4SIBlock::si_tx_name);
		if (tx)
		{
			str = pDlg->m_pMDF->GetString(tx);
			delete tx;
		}
		delete si;
	}
	else str = _T(" ");
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Unit"));
	tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCN, M4CNBlock::cn_md_unit);
	if (tx)
	{
		str = pDlg->m_pMDF->GetString(tx);
		delete tx;
	}
	else str = _T(" ");
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Channel Type"));
	switch (pCN->cn_type)
	{
	case CN_T_FIXEDLEN: p = _T("CN_T_FIXEDLEN"); break;
	case CN_T_VLSD:     p = _T("CN_T_VLSD"); break;
	case CN_T_MASTER:   p = _T("CN_T_MASTER"); break;
	case CN_T_VMASTER:  p = _T("CN_T_VMASTER"); break;
	case CN_T_SYNC:     p = _T("CN_T_SYNC"); break;
	default:            p = _T("?? undefined value");
	}
	pList->SetItemText( i++, 1, p);


	i = pList->InsertItem(i, _T("Data Type"));
	switch (pCN->cn_data_type)
	{
	case CN_D_UINT_LE : p = _T("CN_D_UINT_LE"); break;
	case CN_D_UINT_BE : p = _T("CN_D_UINT_BE");  break;
	case CN_D_SINT_LE : p = _T("CN_D_SINT_LE"); break;
	case CN_D_SINT_BE : p = _T("CN_D_SINT_BE");  break;
	case CN_D_FLOAT_LE: p = _T("CN_D_FLOAT_LE"); break;
	case CN_D_FLOAT_BE: p = _T("CN_D_FLOAT_BE"); break;
	case CN_D_STRING  : p = _T("CN_D_STRING");   break;
	case CN_D_UTF8    : p = _T("CN_D_UTF8");     break;
	case CN_D_UTF16_LE: p = _T("CN_D_UTF16_LE"); break;
	case CN_D_UTF16_BE: p = _T("CN_D_UTF16_BE"); break;
	case CN_D_BYTES   : p = _T("CN_D_BYTES");    break;
	case CN_D_SAMPLE  : p = _T("CN_D_SAMPLE");   break;
	case CN_D_STREAM  : p = _T("CN_D_STREAM");   break;
	case CN_D_CODATE  : p = _T("CN_D_CODATE");   break;
	case CN_D_COTIME  : p = _T("CN_D_COTIME");   break;
	default:            p = _T("?? undefined value");
	}
	pList->SetItemText( i++, 1, p);


	i = pList->InsertItem(i, _T("Bit Offset"));
	_stprintf(buf, _T("%lu"), (UINT)pCN->cn_bit_offset);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Byte Offset"));
	_stprintf(buf, _T("%lu"), (UINT)pCN->cn_byte_offset);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Bit Count"));
	_stprintf(buf, _T("%lu"), (UINT)pCN->cn_bit_count);
	pList->SetItemText( i++, 1, buf);

  const TCHAR *szFlagText[] = {_T("CN_F_ALLINVALID"), 
															 _T("CN_F_INVALID"),    
															 _T("CN_F_PRECISION"),  
															 _T("CN_F_RANGE"),      
															 _T("CN_F_LIMIT"),      
															 _T("CN_F_EXTLIMIT"),   
															 _T("CN_F_DISCRETE"),   
															 _T("CN_F_CALIBRATION"),
															 _T("CN_F_CALCULATED"), 
															 _T("CN_F_VIRTUAL")};    

	i = pList->InsertItem(i, _T("Flags"));
	str.Empty();
	for (j=0; j<10; j++)
	{
		int m = 1<<j;
		if (pCN->cn_flags & m)
		{
			if (!str.IsEmpty())
				str += " | ";
			str += szFlagText[j];
		}
	}
	if (str.IsEmpty())
		str = _T("no flags set");
	pList->SetItemText( i++, 1, str);



	i = pList->InsertItem(i, _T("Inval. Bit Pos"));
	_stprintf(buf, _T("%lu"), pCN->cn_inval_bit_pos);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Min Range Raw"));
	_stprintf(buf, _T("%lf"), pCN->cn_val_range_min);
	pList->SetItemText( i++, 1, buf);

	i = pList->InsertItem(i, _T("Max Range Raw"));
	_stprintf(buf, _T("%lf"), pCN->cn_val_range_max);
	pList->SetItemText( i++, 1, buf);

	DisplayM4Block( pDlg, *pCN, i);
}

void CInfo4Tree::DisplayCCBlock(CFileInfo4 *pDlg, M4CCBlock *pCC)
{
	int i,j;
	TCHAR *p;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("Type"));
	switch (pCC->cc_type)
	{
	case CC_T_IDENTITY:  p = _T("1:1 conversion"); break;
	case CC_T_LINEAR:  p = _T("linear conversion"); break;
	case CC_T_RATIONAL:  p = _T("rational conversion"); break;
	case CC_T_FORMULA:  p = _T("algebraic conversion (MCD-2 MC text formula)"); break;
	case CC_T_N2N_INTERPOL:  p = _T("value to value tabular look-up with interpolation"); break;
	case CC_T_N2N:  p = _T("value to value tabular look-up without interpolation"); break;
	case CC_T_R2N:  p = _T("value range to value tabular look-up"); break;
	case CC_T_N2T					: 	p = _T("value to text/scale conversion tabular look-up"); break;
  case CC_T_R2T         :  p = _T("value range to text/scale conversion tabular look-up"); break; 
  case CC_T_T2N         :  p = _T("text to value tabular look-up"); break; 
  case CC_T_T2T         :  p = _T("text to text tabular look-up (translation)"); break;
	default               :  p = _T("unknown"); break;
	}
	pList->SetItemText( i++, 1, p);
	if (pCC->cc_type == CC_T_LINEAR)
	{
		double f = *pCC->m_var.get(1);
		double o = *pCC->m_var.get(0);
		i = pList->InsertItem(i, _T("Factor"));
		str.Format(_T("%.15lg"),f);
		pList->SetItemText( i++, 1, str);
		i = pList->InsertItem(i, _T("Offset"));
		str.Format(_T("%.15lg"),o);
		pList->SetItemText( i++, 1, str);
	}
	else
	{
		for (j=0; j<pCC->cc_val_count; j++)
		{
			str.Format(_T("cc_val[%d]"),j);
			i = pList->InsertItem(i, str);
			str.Format(_T("%.15lg"),*pCC->m_var.get(j));
			pList->SetItemText( i++, 1, str);
		}
	}


	DisplayM4Block( pDlg, *pCC, i);
}

void CInfo4Tree::DisplayEVBlock(CFileInfo4 *pDlg, M4EVBlock *pEV)
{
	int i=0;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(i, _T("Event type"));
	switch (pEV->ev_type)
	{
	case EV_T_RECORDING: 
					{ 
						pList->SetItemText( i++, 1, _T("EV_T_RECORDING"));
						if (pEV->ev_sync_type == EV_S_TIME)
						{
							double t = (double)pEV->ev_sync_base_value * pEV->ev_sync_factor;
							i = pList->InsertItem(i, _T("Time"));
							str.Format(_T("%.15lg"),t);
							pList->SetItemText( i++, 1, str);
							
							i = pList->InsertItem(i, _T("Range type"));
							if (pEV->ev_range_type == EV_R_POINT)
								pList->SetItemText( i++, 1, _T("EV_R_POINT"));
							else if (pEV->ev_range_type == EV_R_START)
								pList->SetItemText( i++, 1, _T("EV_R_START"));
							else if (pEV->ev_range_type == EV_R_END)
								pList->SetItemText( i++, 1, _T("EV_R_END"));
						}
					}
				  break;
	case EV_T_RECINT:
					pList->SetItemText( i++, 1, _T("EV_T_RECINT"));
				  break;
	case EV_T_ACQINT: 
					pList->SetItemText( i++, 1, _T("EV_T_ACQINT"));
				  break;
	case EV_T_STARTTRG:
					pList->SetItemText( i++, 1, _T("EV_T_STARTTRG"));
				  break;
	case EV_T_STOPTRG:
					pList->SetItemText( i++, 1, _T("EV_T_STOPTRG"));
				  break;
	case EV_T_TRIGGER:
					pList->SetItemText( i++, 1, _T("EV_T_TRIGGER"));
				  break;
	case EV_T_MARKER:
					pList->SetItemText( i++, 1, _T("EV_T_MARKER"));
				  break;
	}
	if (pEV->ev_scope_count == 0)
	{
		if (pEV->hasLink(pEV->ev_ev_parent))
		{
			i = pList->InsertItem( i, _T("Scope"));
			pList->SetItemText( i++, 1, _T("cf. parent event"));
		}
		else if (pEV->hasLink(pEV->ev_ev_range))
		{
			M_LINK l = pEV->getLink(pEV->ev_ev_range);
			str.Format(_T("cf. start of range 0x%I64X"),l);
			i = pList->InsertItem( i, _T("Scope"));
			pList->SetItemText( i++, 1, str);
		}
		else
		{
			i = pList->InsertItem( i, _T("Scope"));
			pList->SetItemText( i++, 1, _T("none defined - whole file"));
		}
	}
	else
	{
		m4Block *b = pDlg->m_pMDF->m_m4.LoadLink( *pEV, m4EVRecord::ev_scope);
		for (UINT j=0; j<pEV->ev_scope_count && b; j++)
		{
			str.Format(_T("Scope[%d]"),j);
			i = pList->InsertItem( i, str);
			if (b)
			{
				str = L"";
				if (b->hdrID()==M4ID_CG)
				{
					str = _T("Channel Group ");
					M4CGBlock *pCG = (M4CGBlock *)b;
					M4TXBlock *tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCG, M4CGBlock::cg_tx_acq_name, M4ID_TX);
					if (tx)
					{
						str += pDlg->m_pMDF->GetString(tx);
						delete tx;
					}
					else 
					{
						M4SIBlock *si = (M4SIBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCG, M4CGBlock::cg_si_acq_source, M4ID_SI);
						if (si)
						{
							tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *si, M4SIBlock::si_tx_name, M4ID_TX);
							if (tx)
							{
								str = pDlg->m_pMDF->GetString(tx);
								delete tx;
							}
							delete si;
						}
					}
				}
				else if (b->hdrID()==M4ID_CN)
				{
					str = _T("Channel ");
					M4CNBlock *pCN = (M4CNBlock *)b;
					M4TXBlock *tx = (M4TXBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pCN, M4CNBlock::cn_tx_name, M4ID_TX);
					if (tx)
					{
						str = pDlg->m_pMDF->GetString(tx);
						delete tx;
					}
				}
				pList->SetItemText( i++, 1, str);
			}
			b = pDlg->m_pMDF->m_m4.LoadLink( *pEV, m4EVRecord::ev_scope+j);
		}
	}

	i = pList->InsertItem(i, _T("Synch base value"));
	str.Format(_T("%I64d [0x%I64X]"),pEV->ev_sync_base_value,pEV->ev_sync_base_value);
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem(i, _T("Synch factor "));
	str.Format(_T("%.15lg"),pEV->ev_sync_factor);
	pList->SetItemText( i++, 1, str);

	DisplayM4Block( pDlg, *pEV, i);
}
void CInfo4Tree::InsertXMLNode(CTreeCtrl &Tree, HTREEITEM hParent, MSXML2::IXMLDOMNodePtr pParentNode)
{
	HTREEITEM hItem;
	BOOL bFlag = TRUE;
	CString str, str1, strNode;
	MSXML2::IXMLDOMNodeListPtr pnl, pList;
	MSXML2::IXMLDOMNodePtr pNode = NULL;
	strNode = (LPCTSTR)pParentNode->baseName;
	if (_tcscmp(strNode, _T("common_properties")) == 0)
	{
		hItem = hParent;
	}
	else if (_tcscmp(strNode, _T("e")) == 0)
	{
		CString strName, strDesc, strUnit, strUnitRef, strType;
		strName.Empty(); strDesc.Empty(); strUnit.Empty(); strUnitRef.Empty(); strType.Empty();
		MSXML2::IXMLDOMNamedNodeMapPtr pAttr = pParentNode->attributes;
		if (pAttr)
		{
			for (int k=0; k<pAttr->length; k++)
			{ 
				MSXML2::IXMLDOMNodePtr pNa = pAttr->item[k];
				if (pNa)
				{
					str = (LPCTSTR)pNa->baseName;
					if (_tcscmp(str,_T("name"))==0)
						strName = (LPCTSTR)pNa->text;
					else if (_tcscmp(str,_T("desc"))==0)
						strDesc = (LPCTSTR)pNa->text;
					else if (_tcscmp(str,_T("unit"))==0)
						strUnit = (LPCTSTR)pNa->text;
					else if (_tcscmp(str,_T("unit_ref"))==0)
						strUnitRef = (LPCTSTR)pNa->text;
					else if (_tcscmp(str,_T("type"))==0)
						strType = (LPCTSTR)pNa->text;
				}
			}
			str.Format(_T("%s = %s"), strName, (TCHAR*)pParentNode->text);
			if (!strUnit.IsEmpty())
			{
				str1.Format(_T(" [%s]"), strUnit);
				str += str1;
			}
			if (!strUnitRef.IsEmpty())
			{
				str1.Format(_T(" [%s]"), strUnitRef);
				str += str1;
			}
			if (!strDesc.IsEmpty())
			{
				str1.Format(_T(" (%s)"), strDesc);
				str += str1;
			}
			if (!strType.IsEmpty())
			{
				str1.Format(_T(" (%s)"), strType);
				str += str1;
			}
			hItem = Tree.InsertItem( str, hParent);
			Tree.Expand( hItem, TVE_EXPAND); 
			return;
		}
	}
	else if (_tcscmp(strNode, _T("tree")) == 0)
	{
		CString strName;
		strName.Empty();
		MSXML2::IXMLDOMNamedNodeMapPtr pAttr = pParentNode->attributes;
		if (pAttr)
		{
			for (int k=0; k<pAttr->length; k++)
			{ 
				MSXML2::IXMLDOMNodePtr pNa = pAttr->item[k];
				if (pNa)
				{
					str = (LPCTSTR)pNa->baseName;
					if (_tcscmp(str,_T("name"))==0)
					{
						strName = (LPCTSTR)pNa->text;
						break;
					}
				}
			}
			hItem = Tree.InsertItem( strName, hParent);
			bFlag = FALSE;
		}
	}
	else
	{
		str.Format(_T("%s = %s"), strNode, (TCHAR*)pParentNode->text);
		hItem = Tree.InsertItem( str, hParent);
	}
	MSXML2::IXMLDOMNamedNodeMapPtr pAttr = pParentNode->attributes;
	if (bFlag && pAttr)
	{
		for (int k=0; k<pAttr->length; k++)
		{ 
			MSXML2::IXMLDOMNodePtr pNa = pAttr->item[k];
			if (pNa)
			{
				str.Format(_T("%s = %s"), (TCHAR*)pNa->baseName, (TCHAR*)pNa->text);
				Tree.InsertItem( str, hItem);
			}
		}
	}
	pnl = pParentNode->childNodes;
	if (pnl)
	{
		for (int i=0; i<pnl->length; i++)
		{
			pNode = pnl->item[i];
			str = (LPCTSTR)pNode->baseName;
			if (str.IsEmpty())
			{
				MSXML2::IXMLDOMNamedNodeMapPtr pAttr = pNode->attributes;
				if (pAttr)
				{
					for (int k=0; k<pAttr->length; k++)
					{ 
						MSXML2::IXMLDOMNodePtr pNa = pAttr->item[k];
						if (pNa)
						{
							str.Format(_T("%s = %s"), (TCHAR*)pNa->baseName, (TCHAR*)pNa->text);
							Tree.InsertItem( str, hItem);
						}
					}
				}
			}
			if (pNode->baseName.length() > 0)
				InsertXMLNode( Tree, hItem, pNode);
		}
		pnl.Release();
	}
	Tree.Expand( hItem, TVE_EXPAND); 
}

void CInfo4Tree::FillXMLTree(CTreeCtrl &Tree, LPCTSTR pszXML)
{
	CString str;
	HTREEITEM hItem;
  MSXML2::IXMLDOMDocument3Ptr doc;
	MSXML2::IXMLDOMNodeListPtr pnl;
	MSXML2::IXMLDOMNodePtr pNode = NULL;
	HRESULT hr = S_FALSE;
	hr = (hr == S_OK) ? hr : doc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	//hr = (hr == S_OK) ? hr : doc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	//hr = (hr == S_OK) ? hr : doc.CreateInstance(__uuidof(MSXML2::DOMDocument40));
	//hr = (hr == S_OK) ? hr : doc.CreateInstance(__uuidof(MSXML2::DOMDocument26));
	//hr = (hr == S_OK) ? hr : doc.CreateInstance( __uuidof(MSXML2::DOMDocument) );
	if (hr != S_OK)
	{
		ATLASSERT(FALSE); // did u CoInitialize ?
		return;
	}
	try
	{
		bstr_t bstrXML(pszXML);
		doc->loadXML(bstrXML);
		MSXML2::IXMLDOMElementPtr pElement = doc->documentElement;
		Tree.DeleteAllItems();
		hItem = Tree.InsertItem( pElement->baseName, Tree.GetRootItem());
		pnl = pElement->childNodes;
		if (pnl)
		{
			for (int i=0; i<pnl->length; i++)
			{
				pNode = pnl->item[i];
				InsertXMLNode(Tree, hItem, pNode);
			}
			pnl.Release();
		}
		Tree.Expand( hItem, TVE_EXPAND); 
	}
	catch (...)
	{
	}
	doc.Release();
}

void CInfo4Tree::DisplayFHBlock(CFileInfo4 *pDlg, M4FHBlock *pFH)
{
	int i=0;
	CString str;
	CTreeCtrl *pTree = &pDlg->m_XMLTree;
	str.Empty();
	M4MDBlock *md = (M4MDBlock *)pDlg->m_pMDF->m_m4.LoadLink( *pFH, m4FHRecord::fh_md_comment, M4ID_MD);
	if (md)
	{
		utf8in in(md->m_utf8.data());
		str = in.data();
		// ...
		delete md;
	}
	if (!str.IsEmpty())
		FillXMLTree( pDlg->m_XMLTree, str);
}

void CInfo4Tree::DisplayXML(CFileInfo4 *pDlg, LPCTSTR pszXML)
{
	FillXMLTree( pDlg->m_XMLTree, pszXML);
}

void CInfo4Tree::DisplayHLBlock(CFileInfo4 *pDlg, M4HLBlock *pHL)
{
	int i=0;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	
	i = pList->InsertItem(i, _T("Flags"));
	if (pHL->hl_flags == 0)
		pList->SetItemText(i++, 1, _T("0"));
	else
		pList->SetItemText(i++, 1, _T("HL_F_EQUALLENGTH"));

	i = pList->InsertItem(i, _T("ZIP Type"));
	if (pHL->hl_zip_type == 0)
		pList->SetItemText(i++, 1, _T("Deflate"));
	else
		pList->SetItemText(i++, 1, _T("Transpose & Deflate"));

	DisplayM4Block( pDlg, *pHL,i);
}

void CInfo4Tree::DisplayCABlock(CFileInfo4 *pDlg, M4CABlock *pCA)
{
	int i=0;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();

	i = pList->InsertItem( i, _T("Type"));
	if (i == CA_T_ARRAY)
		pList->SetItemText( i++, 1, _T("CA_T_ARRAY"));
	else if (i == CA_T_AXIS)
		pList->SetItemText( i++, 1, _T("CA_T_AXIS"));
	else if (i == CA_T_LOOKUP)
		pList->SetItemText( i++, 1, _T("CA_T_LOOKUP"));
	else   
		pList->SetItemText( i++, 1, _T("????"));

	i = pList->InsertItem( i, _T("Storage"));
	if (pCA->ca_flags == CA_S_CN_TEMPLATE)
		pList->SetItemText( i++, 1, _T("CA_S_CN_TEMPLATE"));
	else if (pCA->ca_flags == CA_S_CG_TEMPLATE)
		pList->SetItemText( i++, 1, _T("CA_S_CG_TEMPLATE"));
	else if (pCA->ca_flags == CA_S_DG_TEMPLATE)
		pList->SetItemText( i++, 1, _T("CA_S_DG_TEMPLATE"));
	else   
		pList->SetItemText( i++, 1, _T("????"));
	
	i = pList->InsertItem( i, _T("ndim"));
	str.Format(_T("%d"),pCA->ca_ndim);
		pList->SetItemText( i++, 1, str);

	i = pList->InsertItem( i, _T("Flags"));
	if (pCA->ca_flags == 0)
		pList->SetItemText( i++, 1, _T("0"));
	else
	{
		str.Empty();
		if (pCA->ca_flags & CA_F_DYNAMIC)
			str += _T("CA_F_DYNAMIC|");
		if (pCA->ca_flags & CA_F_INPUT)
			str += _T("CA_F_INPUT|");
		if (pCA->ca_flags & CA_F_OUTPUT)
			str += _T("CA_F_OUTPUT|");
		if (pCA->ca_flags & CA_F_COMPARISON)
			str += _T("CA_F_COMPARISON|");
		if (pCA->ca_flags & CA_F_AXIS)
			str += _T("CA_F_AXIS|");
		if (pCA->ca_flags & CA_F_FIXED)
			str += _T("CA_F_FIXED|");
		if (pCA->ca_flags & CA_F_INVERSE)
			str += _T("CA_F_INVERSE");
		if (str[str.GetLength()-1] == _T('|'))
			str.Delete(str.GetLength()-1);
		pList->SetItemText( i++, 1, str);
	}

	i = pList->InsertItem( i, _T("ca_byte_offset_base"));
	str.Format(_T("%d"),pCA->ca_byte_offset_base);
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem( i, _T("ca_inval_bit_pos_base"));
	str.Format(_T("%d"),pCA->ca_inval_bit_pos_base);
	pList->SetItemText( i++, 1, str);

	DisplayM4Block( pDlg, *pCA,i);
}

void CInfo4Tree::DisplayDTBlock(CFileInfo4 *pDlg, M4DTBlock *pDT)
{
	int i=0;
	DisplayM4Block( pDlg, *pDT,i);
}
void CInfo4Tree::DisplayRDBlock(CFileInfo4 *pDlg, M4RDBlock *pRD)
{
	int i=0;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	DisplayM4Block( pDlg, *pRD,i);
	i = pList->GetItemCount();

}
void CInfo4Tree::DisplayDLBlock(CFileInfo4 *pDlg, M4DLBlock *pDL)
{
	DisplayM4Block( pDlg, *pDL,0);
}
void CInfo4Tree::DisplayDZBlock(CFileInfo4 *pDlg, M4DZBlock *pDZ)
{
	int i=0;
	CString str;
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	
	i = pList->InsertItem( i, _T("Org. block type"));
	M_CHAR buf[4];
	buf[0] = pDZ->dz_org_block_type[0];
	buf[1] = pDZ->dz_org_block_type[1];
	buf[2] = 0;
	str = buf;
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem( i, _T("ZIP Type"));
	if (pDZ->dz_zip_type == 0)
		pList->SetItemText( i++, 1, _T("Deflate"));
	else
		pList->SetItemText( i++, 1, _T("Transpose & Deflate"));

	i = pList->InsertItem( i, _T("ZIP parameter"));
	str.Format(_T("%d"), pDZ->dz_zip_parameter);
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem( i, _T("Org. data length"));
	str.Format(_T("%I64u"), pDZ->dz_org_data_length);
	pList->SetItemText( i++, 1, str);

	i = pList->InsertItem( i, _T("Data length"));
	str.Format(_T("%I64u"), pDZ->dz_data_length);
	pList->SetItemText( i++, 1, str);

	DisplayM4Block( pDlg, *pDZ,i);
}


void CInfo4Tree::DisplayHdrBlock(CFileInfo4 *pDlg, M4HDBlock *pHdr)
{
	int i;
	CString str;
	TCHAR buf[40];
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("UTC start of measurement"));
  SYSTEMTIME st;
  M_DATE::get(pHdr->hd_start_time,st);
	_stprintf(buf,_T("%02hu.%02hu.%04hu %02hu:%02hu:%02hu,%03hu"),  st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	pList->SetItemText( i++, 1, buf);

	m4Block *b = pDlg->m_pMDF->m_m4.LoadLink( *pHdr, m4HDRecord::hd_md_comment);
	if (b)
	{
		if (b->hdrID()==M4ID_TX)
		{
			M4TXBlock *pTxt = (M4TXBlock *)b;
			const BYTE *t = pTxt ->m_utf8.data();
			utf8in in(pTxt ->m_utf8.data());
			str = in.data();
		}
		else if (b->hdrID()==M4ID_MD)
		{
			M4MDBlock *pMD = (M4MDBlock *)b;
			utf8in in(pMD->m_utf8.data());
			str = in.data();
		}
		delete b;
		i = pList->InsertItem(i, _T("Comment"));
		pList->SetItemText( i++, 1, str);
	}

	i = pList->InsertItem(i, _T("1st Data Group"));
	_stprintf(buf,_T("0x%I64X"),  pHdr->getLink(M4HDBlock::hd_dg_first));
	pList->SetItemText( i++, 1, buf);

	DisplayM4Block( pDlg, *pHdr, i);
}

void CInfo4Tree::DisplayM4Block( CFileInfo4 *pDlg, m4Block &block, int idx)
{
	TCHAR buf[40];
	CListCtrl *pList = &pDlg->m_List;
	if (idx == 0)
		pList->DeleteAllItems();
	idx = pList->InsertItem(idx, _T("Position"));
	_stprintf(buf, _T("0x%I64X"), block.m_At);
	pList->SetItemText( idx++, 1, buf);

	idx = pList->InsertItem(idx, _T("ID"));
	char c1,c2,c3,c4;
	c1 = block.m_Hdr.hdr_hdr & 0xFF;
	c2 = (block.m_Hdr.hdr_hdr>>8) & 0xFF;
	c3 = block.m_Hdr.hdr_id & 0xFF;
	c4 = (block.m_Hdr.hdr_id>>8) & 0xFF;
	_stprintf(buf, _T("%c%c%c%c"), c1, c2, c3, c4);
	pList->SetItemText( idx++, 1, buf);

	idx = pList->InsertItem(idx, _T("Length"));
	_stprintf(buf, _T("%I64d"), block.m_Hdr.hdr_len);
	pList->SetItemText( idx++, 1, buf);

	idx = pList->InsertItem(idx, _T("Links"));
	_stprintf(buf, _T("%I64d"), block.m_Hdr.hdr_links);
	pList->SetItemText( idx++, 1, buf);

	for (int i=0; i<block.m_Hdr.hdr_links; i++)
	{
		_stprintf(buf, _T(" Link %d"), i+1);
		idx = pList->InsertItem(idx, buf);
		_stprintf(buf, _T("0x%I64X"), block.getLink(i));
		pList->SetItemText( idx++, 1, buf);
	}
}

void CInfo4Tree::DisplayIDBlock(CFileInfo4 *pDlg, mdfFileId *pId)
{
	int i;
	TCHAR buf[40];
	CListCtrl *pList = &pDlg->m_List;
	pList->DeleteAllItems();
	i = pList->InsertItem(0, _T("id_file"));
	_tcsncpy(buf, _bstr_t(pId->id_file), 8); buf[8] = 0;
	pList->SetItemText( i, 1, buf);

	i = pList->InsertItem(i, _T("id_vers"));
	_tcsncpy(buf, _bstr_t(pId->id_vers), 8); buf[8] = 0;
	pList->SetItemText( i, 1, buf);

	i = pList->InsertItem(i, _T("id_prog"));
	_tcsncpy(buf, _bstr_t(pId->id_prog), 8); buf[8] = 0;
	pList->SetItemText( i, 1, buf);

	i = pList->InsertItem(i, _T("id_order"));
	pList->SetItemText(i, 1, pId->id_order==0 ? _T("Intel") : _T("Motorola (unsupported)"));

	i = pList->InsertItem(i, _T("id_float"));
	pList->SetItemText(i, 1, pId->id_float==0 ? _T("IEEE 754") : _T("(unsupported)"));

	i = pList->InsertItem(i, _T("id_ver"));
	_stprintf(buf, _T("%d"), (int)pId->id_ver);
	pList->SetItemText( i, 1, buf);
}


BOOL CInfo4Tree::Compare(CString strText, CString strTarget)
{
  if (!m_bCase)
  {
    strText.MakeUpper();
    strTarget.MakeUpper();
  }

  BOOL bFound = FALSE;
  if (m_bWords)
  {
    if (strTarget.Compare(strText) == 0)
      bFound = TRUE;
  }
  else
  {
    if (strText.Find(strTarget) >= 0)
      bFound = TRUE;
  }
  if (bFound && ++m_nFindIdx <= m_nLastFindIdx)
    return FALSE;
  return bFound;
}

HTREEITEM CInfo4Tree::find_child(HTREEITEM p,const TCHAR *name)
{
  const int bufLen = 256;
  TCHAR buffer[256];
  HTREEITEM node, h;
  TV_ITEM tvItem;

	if (p)
	 	node = GetChildItem(p); // 1. Child
  else
		node = GetRootItem();

  if (node==NULL) 
		return NULL;

	while (1)
  {
    buffer[0] = 0;
    tvItem.pszText = buffer;
	  tvItem.cchTextMax = bufLen;
		tvItem.hItem=node;
		tvItem.mask = TVIF_TEXT;
    BOOL res = GetItem(&tvItem);
    if (res && Compare(buffer,name))
      return node;
    h = GetNextItem(node,TVGN_CHILD);
    if (h)
    {
      h = find_child(node,name);
      if (h)
        return h;
    }
    node = GetNextItem(node,TVGN_NEXT);
    if (!node)
      return node;
  }
	return 0;
}


HTREEITEM CInfo4Tree::find(HTREEITEM hInit, const TCHAR *name)
{
  HTREEITEM hNext, h;
  if (hInit)
  {
    h = GetNextItem( hInit, TVGN_NEXT);
    if (!h)
      h = GetNextItem( hInit, TVGN_CHILD);
    hInit = h;
  }
  else
    hInit = GetRootItem();
  
  CString str;

	while (hInit)
  {
    str = GetItemText(hInit);
    if (Compare(str,name))
      return hInit;
    hNext = GetNextItem(hInit,TVGN_CHILD);
    if (hNext)
    {
      hNext = find_child(hInit,name);
      if (hNext)
        return hNext;
    }
    hNext = GetNextItem(hInit,TVGN_NEXT);
    if (!hNext)
      return find( hInit, name);
    else
      hInit = hNext;
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CFileInfo4 dialog
/////////////////////////////////////////////////////////////////////////////////////////


CFileInfo4::CFileInfo4(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileInfo4::IDD, pParent)
	, m_bPhysVal(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pCurCtrl = NULL;
  m_pMDF = NULL;
  m_hItemLast = NULL;
  m_strFindLast.Empty();
  m_bShowingData = FALSE;
  m_bOneCol = FALSE;
  m_Cache = NULL;
}

CFileInfo4::~CFileInfo4()
{
  if (m_Cache)
  {
    TCHAR **p = m_Cache;
    for (DWORD i=0; i<m_dwInCache; i++,p++)
      free(*p);
    free(m_Cache);
    m_Cache = NULL;
  }
}

void CFileInfo4::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_Tree);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_DATALIST, m_DataList);
	DDX_Control(pDX, IDC_XMLTREE, m_XMLTree);
	DDX_Check( pDX, IDC_PHYS, m_bPhysVal);
}

BEGIN_MESSAGE_MAP(CFileInfo4, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FINDNEXT, OnFindNext)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_DATALIST, OnGetDispInfoList)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_DATALIST, OnColumnClickList)
	ON_BN_CLICKED(IDC_PHYS, &CFileInfo4::OnBnClickedPhys)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREE, &CFileInfo4::OnTvnItemExpandedTree)
	ON_NOTIFY(TVN_GETDISPINFO, IDC_TREE, &CFileInfo4::OnTvnGetdispinfo)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE, &CFileInfo4::OnTvnItemExpandingTree)
  ON_WM_TIMER()
END_MESSAGE_MAP()



// CFileInfo4-Meldungshandler

BOOL CFileInfo4::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	m_pCurCtrl = &m_List;

	m_DataList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

  CString str, str1;
  HTREEITEM hParent;
  TCHAR name[_MAX_FNAME];
  
  if (m_pMDF == NULL)
    return FALSE;

	m_List.InsertColumn(0, _T("Property"), 0, 100);
	m_List.InsertColumn(1, _T("Contents"), 0, 500);

  _tsplitpath(m_pMDF->m_strPathName, NULL, NULL, name, NULL);
  hParent = m_Tree.InsertItem(name);
  m_Tree.SetItemData(hParent, (DWORD)(void*)m_pMDF);

	FillTree(hParent);

  m_Tree.Expand(m_Tree.GetRootItem(), TVE_EXPAND);

  SetTimer(101, 300, NULL);
	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}


void CFileInfo4::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CFileInfo4::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFileInfo4::AddDataBlocks( M_LINK at, HTREEITEM hParent)
{
	FI4Type *pEntry;
	CString str;
	HTREEITEM hItem;
	int nBlocks = 0;
	m4Block *b = m_pMDF->m_m4.LoadBlock(at);
	while (b)
	{
		if (b->hdrID()==M4ID_DT)
		{
			M4DTBlock* dt = (M4DTBlock*)b;
			pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pEntry->type = 12;
			pEntry->u.pDT = dt;
			pEntry->nummer = nBlocks;
			hItem = m_Tree.InsertItem(_T("Data block"), hParent);
			m_Tree.SetItemData(hItem, (DWORD)pEntry);
			b = m_pMDF->m_m4.LoadLink(*dt,m4DTRecord::dt_dt_next);
		}
		else if (b->hdrID()==M4ID_DL)
		{
			M4DLBlock* dl = (M4DLBlock*)b;
			pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pEntry->type = 14;
			pEntry->u.pDL = dl;
			pEntry->nummer = nBlocks++;
			hItem = m_Tree.InsertItem(_T("Data list block"), hParent);
			m_Tree.SetItemData(hItem, (DWORD)pEntry);
			for (M_UINT32 i=0; i<dl->dl_count; i++)
			{
				AddDataBlocks( dl->getLink(m4DLRecord::dl_data+i), hItem);
			}
			b = m_pMDF->m_m4.LoadLink(*dl, m4DLRecord::dl_dl_next);
		}
		else if (b->hdrID()==M4ID_RD)
		{
			M4RDBlock* rd = (M4RDBlock*)b;
			pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pEntry->type = 13;
			pEntry->u.pRD = rd;
			pEntry->nummer = nBlocks++;
			hItem = m_Tree.InsertItem(_T("Reduced data block"), hParent);
			m_Tree.SetItemData(hItem, (DWORD)pEntry);
			b = NULL;
		}
		else if (b->hdrID()==M4ID_DZ)
		{
			M4DZBlock* dz = (M4DZBlock*)b;
			pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pEntry->type = 102;
			pEntry->u.pDZ = dz;
			pEntry->nummer = nBlocks++;
			hItem = m_Tree.InsertItem(_T("Compressed data block"), hParent);
			m_Tree.SetItemData(hItem, (DWORD)pEntry);
			b = NULL;
		}
		else
		{
			delete b;
			b = NULL;
		}
	}
}

void CFileInfo4::FillTree(HTREEITEM hParent)
{
	FI4Type *pEntry;
	FI4Type *pData;
	FI4Type *pGroup;
	FI4Type **pWork;
	int nChans;
	CString str, str1;
	HTREEITEM hItem, hGroup, hCG, hCN, hEvents, hHdr, hHL;
	M4HDBlock *pHdr = m_pMDF->m_m4.GetHdr();
	CWaitCursor w;

	pWork = (FI4Type **)calloc( 10000, sizeof(FI4Type*));

	pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
	pEntry->type = 1;
	pEntry->u.pId = m_pMDF->m_m4.GetIdBlk();
  hItem = m_Tree.InsertItem(_T("ID Block"), hParent);
	pEntry->hItem = hItem;
	m_Tree.SetItemData(hItem, (DWORD)pEntry);

	pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
	pEntry->type = 2;
	pEntry->u.pHdr = pHdr;
	hHdr = m_Tree.InsertItem(_T("Header Block"), hParent);
	pEntry->hItem = hHdr;
	m_Tree.SetItemData(hHdr, (DWORD)pEntry);
	// MDcomment
	m4Block *b = m_pMDF->m_m4.LoadLink( *pHdr, m4HDRecord::hd_md_comment);
	if (b)
	{
		if (b->hdrID()==M4ID_MD)
		{
			M4MDBlock *pMD = (M4MDBlock *)b;
			utf8in in(pMD->m_utf8.data());
			str = in.data();
			pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pEntry->type = 10;
			pEntry->u.pszXML = _tcsdup(str);
			pEntry->nummer = 0;
			hItem = m_Tree.InsertItem(_T("MDcomment"), hHdr);
			m_Tree.SetItemData(hItem, (DWORD)pEntry);
		}
		delete b;
	}

	// File History
	int nFHBlocks = 0;
	M4FHBlock *fh = (M4FHBlock *)m_pMDF->m_m4.LoadLink( *pHdr, m4HDRecord::hd_fh_first, M4ID_FH);
	if (fh)
		hEvents = m_Tree.InsertItem(_T("File History"), hHdr);
	while (fh)
	{
		pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
		pEntry->type = 9;
		pEntry->u.pFH = fh;
		pEntry->nummer = nFHBlocks;
		str.Format(_T("File History %d"),++nFHBlocks);
		hItem = m_Tree.InsertItem(str, hEvents);
		m_Tree.SetItemData(hItem, (DWORD)pEntry);

		fh = (M4FHBlock *)m_pMDF->m_m4.LoadLink(*fh,M4FHBlock::fh_fh_next, M4ID_FH);
	}

	// Events
	M4EVBlock *ev;
	int nEvBlocks = 0;
  ev = (M4EVBlock *)m_pMDF->m_m4.LoadLink(*pHdr,M4HDBlock::hd_ev_first, M4ID_EV);
	if (ev)
		hEvents = m_Tree.InsertItem(_T("Events"), hHdr);
	while (ev)
	{
		pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
		pEntry->type = 8;
		pEntry->u.pEV = ev;
		pEntry->nummer = nEvBlocks;
		M4TXBlock *tx = (M4TXBlock *)m_pMDF->m_m4.LoadLink( *ev, M4EVBlock::ev_tx_name, M4ID_TX);
		if (tx)
		{
			str = m_pMDF->GetString(tx);
			delete tx;
		}
		else 
			str.Format(_T("<Event %d>"),++nEvBlocks);
		hItem = m_Tree.InsertItem(str, hEvents);
		m_Tree.SetItemData(hItem, (DWORD)pEntry);
		// evcomment
		b = m_pMDF->m_m4.LoadLink( *ev, m4EVRecord::ev_md_comment);
		if (b)
		{
			if (b->hdrID()==M4ID_MD)
			{
				M4MDBlock *pMD = (M4MDBlock *)b;
				utf8in in(pMD->m_utf8.data());
				str = in.data();
				pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
				pEntry->type = 10;
				pEntry->u.pszXML = _tcsdup(str);
				pEntry->nummer = 0;
				HTREEITEM h = m_Tree.InsertItem(_T("Comment"), hItem);
				m_Tree.SetItemData(h, (DWORD)pEntry);
			}
			delete b;
		}
		ev = (M4EVBlock *)m_pMDF->m_m4.LoadLink(*ev,M4EVBlock::ev_ev_next, M4ID_EV);
  }


	M4DGBlock *dg;
	M4HLBlock *hl;
	M4CGBlock *cg;
	M4CNBlock *cn;
	int nGrp = 1;
  dg = (M4DGBlock *)m_pMDF->m_m4.LoadLink(*pHdr,M4HDBlock::hd_dg_first, M4ID_DG);
	while (dg)
	{
		TVINSERTSTRUCT tvs = {0}; 
		pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
		pEntry->type = 3;
		pEntry->u.pDG = dg;
		pEntry->nummer = nGrp;
		str.Format(_T("Data Group %d"),++nGrp);
		tvs.item.mask = TVIF_TEXT|TVIF_CHILDREN|TVIF_PARAM;
		tvs.item.pszText = str.LockBuffer();
		tvs.item.iImage = 0;
		tvs.item.iSelectedImage = 0;
		tvs.item.cChildren = I_CHILDRENCALLBACK;
		tvs.hParent = hParent;
		tvs.hInsertAfter = TVI_LAST;
		tvs.item.lParam = (DWORD)pEntry;
		hGroup = m_Tree.InsertItem( &tvs);
		pEntry->hItem = hGroup;
		pEntry->bNotYetInitialized = TRUE;
		str.UnlockBuffer();
		// dgcomment
		b = m_pMDF->m_m4.LoadLink( *dg, m4DGRecord::dg_md_comment);
		if (b)
		{
			if (b->hdrID()==M4ID_MD)
			{
				M4MDBlock *pMD = (M4MDBlock *)b;
				utf8in in(pMD->m_utf8.data());
				str = in.data();
				pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
				pEntry->type = 10;
				pEntry->u.pszXML = _tcsdup(str);
				pEntry->nummer = 0;
				hItem = m_Tree.InsertItem(_T("Comment"), hGroup);
				m_Tree.SetItemData(hItem, (DWORD)pEntry);
			}
			delete b;
		}
		pGroup = pEntry;
		nChans = 0;
		pData = NULL;

		hl = (M4HLBlock *)m_pMDF->m_m4.LoadLink(*dg, M4DGBlock::dg_data, M4ID_HL);
		if (hl)
		{
			pData = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pData->type = 101;
			pData->u.pHL = hl;
			pData->nummer = nGrp-1;
			str.Format(_T("Header List Block %d"),nGrp);
			memset(&tvs, 0, sizeof(TVINSERTSTRUCT));
			tvs.item.mask = TVIF_TEXT|TVIF_CHILDREN|TVIF_PARAM;
			tvs.item.pszText = str.LockBuffer();
			tvs.item.iImage = 0;
			tvs.item.iSelectedImage = 0;
			tvs.item.cChildren = I_CHILDRENCALLBACK;
			tvs.hParent = hGroup;
			tvs.hInsertAfter = TVI_LAST;
			tvs.item.lParam = (DWORD)pData;
			hHL = m_Tree.InsertItem( &tvs);
			//hHL = m_Tree.InsertItem(str, hGroup);
			pData->hItem = hHL;
			pData->bNotYetInitialized = TRUE;
			str.UnlockBuffer();
			TVITEM tvi = {0};
			tvi.hItem = hHL;
			tvi.mask = TVIF_CHILDREN;
			tvi.cChildren = I_CHILDRENCALLBACK;
			m_Tree.SetItem(&tvi);
			//m_Tree.SetItemData(hHL, (DWORD)pData);
			//AddDataBlocks( hl->getLink(M4HLBlock::hl_dl_first), hHL);
		}
		//else
		//	AddDataBlocks( dg->getLink(M4DGBlock::dg_data), hGroup);


		pData = NULL;
		cg = (M4CGBlock *)m_pMDF->m_m4.LoadLink(*dg, M4DGBlock::dg_cg_first, M4ID_CG);

		if (cg)
		{
			pData = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pData->type = 6;
			pData->u.pDG = dg;
			pData->pCG = cg;
			pData->nummer = nGrp-1;
		}
		int nCGrp = 0;
		while (cg)
		{
			pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
			pEntry->type = 4;
			pEntry->pCG = cg;
			pEntry->nummer = nCGrp;
			str.Format(_T("Channel Group %d"),++nCGrp);
			hCG = m_Tree.InsertItem(str, hGroup);
			pEntry->hItem = hCG;
			m_Tree.SetItemData(hCG, (DWORD)pEntry);
			// acq source
			M4TXBlock *tx = (M4TXBlock *)m_pMDF->m_m4.LoadLink( *cg, M4CGBlock::cg_tx_acq_name, M4ID_TX);
			if (tx)
			{
				str1 = m_pMDF->GetString(tx);
				delete tx;
				str.Format(_T("Data Group %d: %s"),nGrp,str1);
				m_Tree.SetItemText(hGroup, str);
				str.Format(_T("Channel Group %d: %s"),nCGrp,str1);
				m_Tree.SetItemText(hCG, str);
			}

			// cgcomment
			BOOL bGroupHasComment = FALSE;
			HTREEITEM hCommentItem;
			b = m_pMDF->m_m4.LoadLink( *cg, m4CGRecord::cg_md_comment);
			if (b)
			{
				if (b->hdrID()==M4ID_MD)
				{
					M4MDBlock *pMD = (M4MDBlock *)b;
					utf8in in(pMD->m_utf8.data());
					str = in.data();
					FI4Type *pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
					pEntry->type = 10;
					pEntry->u.pszXML = _tcsdup(str);
					pEntry->nummer = 0;
					hCommentItem = m_Tree.InsertItem(_T("Comment"), hCG);
					m_Tree.SetItemData(hCommentItem, (DWORD)pEntry);
					bGroupHasComment = TRUE;
				}
				delete b;
			}

			cn = (M4CNBlock *)m_pMDF->m_m4.LoadLink(*cg, M4CGBlock::cg_cn_first, M4ID_CN);
			while (cn)
			{
				pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
				pEntry->type = 5;
				pEntry->u.pCN = cn;
				M4TXBlock *tx = (M4TXBlock *)m_pMDF->m_m4.LoadLink( *cn, M4CNBlock::cn_tx_name);
				if (tx)
				{
					str = m_pMDF->GetString(tx);
					delete tx;
				}
				else str = _T("unnamed signal");
				if (cn->cn_type == CN_T_MASTER)
				{
					if (bGroupHasComment && hCommentItem)
						hCN = m_Tree.InsertItem( str, hCG, hCommentItem);
					else
						hCN = m_Tree.InsertItem( str, hCG, TVI_FIRST);
					if (nChans)
					{
						for (int j=nChans; j>0; j--)
							pWork[j] = pWork[j-1];
						pWork[0] = pEntry;
					}
					else
						pWork[nChans] = pEntry;
					nChans++;
				}
				else
				{
					hCN = m_Tree.InsertItem( str, hCG);
					pWork[nChans++] = pEntry;
				}
				pEntry->hItem = hCN;
				pEntry->nummer = nChans;
				m_Tree.SetItemData(hCN, (DWORD)pEntry);
				// cncomment
				b = m_pMDF->m_m4.LoadLink( *cn, m4CNRecord::cn_md_comment);
				if (b)
				{
					if (b->hdrID()==M4ID_MD)
					{
						M4MDBlock *pMD = (M4MDBlock *)b;
						utf8in in(pMD->m_utf8.data());
						str = in.data();
						FI4Type *pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
						pEntry->type = 10;
						pEntry->u.pszXML = _tcsdup(str);
						pEntry->nummer = 0;
						hItem = m_Tree.InsertItem(_T("Comment"), hCN);
						m_Tree.SetItemData(hItem, (DWORD)pEntry);
					}
					delete b;
				}
				M4CCBlock *pConv = (M4CCBlock *)m_pMDF->m_m4.LoadLink(*cn, M4CNBlock::cn_cc_conversion, M4ID_CC);
				if (pConv)
				{
					int i = pConv->cc_type;
					if (i == CC_T_IDENTITY)
					{
						delete pConv;
						pConv = NULL;
					}
					else if (i == CC_T_LINEAR && *pConv->m_var.get(0) == 0 && *pConv->m_var.get(1)==1.0)
					{
						delete pConv;
						pConv = NULL;
					}
					else if (i>CC_T_R2N)
					{
						delete pConv;
						pConv = NULL;
					}
				}
				if (pConv)
				{
					pEntry->pCalc = new CMdf4Calc( cn, m_pMDF->m_m4);

					FI4Type *pCC = (FI4Type *)calloc( 1, sizeof(FI4Type));
					pCC->type = 7;
					pCC->u.pCC = pConv;
					pCC->hItem	= m_Tree.InsertItem(_T("Conversion"), hCN);
					m_Tree.SetItemData(pCC->hItem, (DWORD)pCC);
				}
				M4CABlock *ca = (M4CABlock *)m_pMDF->m_m4.LoadLink(*cn, M4CNBlock::cn_composition, M4ID_CA);
				if (ca)
				{
					FI4Type *pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
					pEntry->type = 15;
					pEntry->u.pCA = ca;
					pEntry->hItem	= m_Tree.InsertItem(_T("Composition"), hCN);
					m_Tree.SetItemData(pEntry->hItem, (DWORD)pEntry);
				}
				cn = (M4CNBlock *)m_pMDF->m_m4.LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);// cn nicht freigeben!
			}
			int nSR = 0;
			M4SRBlock *sr = (M4SRBlock *)m_pMDF->m_m4.LoadLink(*cg, M4CGBlock::cg_sr_first, M4ID_SR);
			while (sr)
			{
				pEntry = (FI4Type *)calloc( 1, sizeof(FI4Type));
				pEntry->type = 11;
				pEntry->u.pSR = sr;
				str.Format(_T("Sample Reduction %d (%lg s)"),++nSR, sr->sr_interval);
				HTREEITEM hSR = m_Tree.InsertItem(str, hCG);
				pEntry->hItem = hSR;
				m_Tree.SetItemData(hSR, (DWORD)pEntry);
				b = m_pMDF->m_m4.LoadLink( *sr, M4SRBlock::sr_data);
				if (b && b->hdrID()==M4ID_HL)
				{
					M4HLBlock *hl = (M4HLBlock *)b;
					FI4Type *pData = (FI4Type *)calloc( 1, sizeof(FI4Type));
					pData->type = 101;
					pData->u.pHL = hl;
					pData->nummer = 1;
					HTREEITEM hHL = m_Tree.InsertItem(_T("Header List Block"), hSR);
					pData->hItem = hHL;
					m_Tree.SetItemData(hHL, (DWORD)pData);
					AddDataBlocks( hl->getLink(M4HLBlock::hl_dl_first), hHL);
				}
				else
				{
    			AddDataBlocks( sr->getLink(M4SRBlock::sr_data), hSR);
					if (b)
						delete b;
				}

				sr = (M4SRBlock *)m_pMDF->m_m4.LoadLink(*sr, M4SRBlock::sr_sr_next, M4ID_SR);
			}
			cg = (M4CGBlock *)m_pMDF->m_m4.LoadLink(*cg, M4CGBlock::cg_cg_next, M4ID_CG); // wäre unsorted!
		}
		if (pData)
		{
		  pData->pChans = (FI4Type *)calloc( nChans, sizeof(FI4Type));
		  for (int i=0; i<nChans; i++)
			  pData->pChans[i] = *pWork[i];
		  pData->nChans = nChans;

		  hItem = m_Tree.InsertItem(_T("Data"), hGroup);
		  pData->hItem = hItem;
		  m_Tree.SetItemData(hItem, (DWORD)pData);
    }
		dg = (M4DGBlock *)m_pMDF->m_m4.LoadLink(*dg,M4DGBlock::dg_dg_next, M4ID_DG);
		nGrp++;
  }
  free(pWork);
}

void CFileInfo4::OnTvnItemExpandedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
  if (pNMTreeView->itemNew.hItem == NULL)
    return;
  if (pNMTreeView->action == TVE_EXPAND)
  {
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != m_Tree.GetRootItem())
		{
			FI4Type *pData = (FI4Type *)m_Tree.GetItemData(hItem);
			if (pData && pData->bNotYetInitialized)
			{
				CWaitCursor w;
				if (pData->type == 101)
					AddDataBlocks( pData->u.pHL->getLink(M4HLBlock::hl_dl_first), hItem);
				else 
					AddDataBlocks( pData->u.pDG->getLink(M4DGBlock::dg_data), hItem);
				pData->bNotYetInitialized = FALSE;
			}
		}
	}
	*pResult = 0;
}

void CFileInfo4::OnTvnItemExpandingTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
  if (pNMTreeView->itemNew.hItem == NULL)
    return;
  if (pNMTreeView->action == TVE_EXPAND)
  {
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		if (hItem != m_Tree.GetRootItem())
		{
			FI4Type *pData = (FI4Type *)m_Tree.GetItemData(hItem);
			if (pData && pData->bNotYetInitialized)
			{
				CWaitCursor w;
				if (pData->type == 101)
				{
					AddDataBlocks( pData->u.pHL->getLink(M4HLBlock::hl_dl_first), hItem);
					pData->bNotYetInitialized = FALSE;
				}
			}
		}
	}
	*pResult = 0;
}

void CFileInfo4::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	TVITEM* pItem = &(pTVDispInfo)->item;
	if (pItem->mask & TVIF_CHILDREN)
	{
		pItem->cChildren = 1;    
	}
	*pResult = 0;
}
void CFileInfo4::OnSize(UINT nType, int cx, int cy) 
{
	int x,y;
	RECT r;
	CWnd *c;
	CDialog::OnSize(nType, cx, cy);
	
	c = GetDlgItem(IDOK);
	if (c && cx && cy)
	{
		c->GetClientRect(&r);
		x = cx - r.right-8;
		y = cy - r.bottom - 8;
		c->MoveWindow(x, y, r.right, r.bottom);
  
    c = GetDlgItem(IDC_FIND);
		c->GetClientRect(&r);
		x = 8;
		y = cy - r.bottom - 8;
		c->MoveWindow(x, y, r.right, r.bottom);

    c = GetDlgItem(IDC_FINDNEXT);
		x = r.right+8;
		c->MoveWindow(x, y, r.right, r.bottom);

		x = 8;
    y = 8;
		m_Tree.MoveWindow(x, y, cx / 3, cy - 16 - r.bottom);

    x = 16 + cx / 3;
		if (m_pCurCtrl)
			m_pCurCtrl->MoveWindow(x, 8, cx -x - 8, cy - 16 - r.bottom);

  	c = GetDlgItem(IDC_EXPORT);
    CRect rExport; 
    c->GetClientRect(&rExport);
		c->MoveWindow(x, cy-rExport.Height()-8, rExport.Width(), rExport.Height());

    x += 16 + rExport.Width();
  	c = GetDlgItem(IDC_PHYS);
    c->GetClientRect(&rExport);
		c->MoveWindow(x, cy-rExport.Height()-8, rExport.Width(), rExport.Height());
	}	
}

void CFileInfo4::OnFind() 
{
  CFindDlg dlg;
  dlg.m_strText = m_strFindLast;
  dlg.m_bCase = m_Tree.m_bCase;
  dlg.m_bWord = m_Tree.m_bWords;
  m_Tree.m_nFindIdx = 0;
  m_Tree.m_nLastFindIdx = 0;

  if (dlg.DoModal() == IDOK)
  {
    m_strFindLast = dlg.m_strText;
    m_Tree.m_bCase = dlg.m_bCase;
    m_Tree.m_bWords = dlg.m_bWord;
    m_hItemLast = m_Tree.GetRootItem();
    HTREEITEM hItem = m_Tree.find(m_hItemLast, m_strFindLast);
    if (hItem)
    {
      m_Tree.m_nLastFindIdx++;
      m_Tree.SelectItem( hItem );
    }
    else
			AfxMessageBox(_T("Der angegebene Text wurde nicht gefunden."), MB_ICONINFORMATION);
  }
}

void CFileInfo4::OnFindNext() 
{
  if (m_strFindLast.IsEmpty())
  {
    OnFind();
    return;
  }
  m_Tree.m_nFindIdx = 0;
  HTREEITEM hItem = m_Tree.find( m_hItemLast, m_strFindLast);
  if (hItem)
  {
    m_Tree.m_nLastFindIdx++;
    m_Tree.SelectItem( hItem );
  }
  else
		AfxMessageBox(_T("Der angegebene Text wurde nicht gefunden."), MB_ICONINFORMATION);
}

void CFileInfo4::OnGetDispInfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
  LVITEM* pItem= &(pDispInfo)->item;

  if (pItem->mask & LVIF_TEXT) //valid text buffer?
  {
		if (m_bShowingData)
    {
      if (pItem->iSubItem == 0) // Index
      {
				CString str; str.Format(_T("%ld"), pItem->iItem);
        _tcsncpy( pItem->pszText, str, pItem->cchTextMax);
        return;
      }
      else if (pItem->iSubItem > (int)m_nChan)
        return;
      DWORD idx = pItem->iItem * m_nChan + pItem->iSubItem-1;
      if (m_Cache && idx < m_dwInCache)
      {
        if (m_Cache[idx] == NULL)
				{
          m_Cache[idx] = _tcsdup(m_Tree.Show(pItem->iItem,pItem->iSubItem, m_bPhysVal));
					if (pItem->iSubItem==1 && pItem->iItem>1) // Zeitfehler?
					{
						double t, tlast;
						DWORD lastidx = (pItem->iItem-1) * m_nChan;
						if (m_Cache[lastidx])
						{
							t = _ttof(m_Cache[idx]);
							tlast = _ttof(m_Cache[lastidx]);
							if (t<=tlast)
							{
								CString str; 
								str.Format(_T("%s#"),m_Cache[idx]);
								free(m_Cache[idx]);
								m_Cache[idx] = _tcsdup(str);
								_tcsncpy( pItem->pszText, m_Cache[idx], pItem->cchTextMax);
							}
						}
					}
				}
        _tcsncpy( pItem->pszText, m_Cache[idx], pItem->cchTextMax);
      }
      else
      {
        CString str = _tcsdup(m_Tree.Show(pItem->iItem,pItem->iSubItem, m_bPhysVal));
        _tcsncpy( pItem->pszText, str, pItem->cchTextMax);
      }
    }
    else
    {
      if (m_DataList.GetItemCount() > pItem->iItem)
      {
        CString str = m_DataList.GetItemText(pItem->iItem,pItem->iSubItem);
        _tcsncpy( pItem->pszText, str, pItem->cchTextMax);
      }
    }
  }  

	*pResult = 0;
}

void CFileInfo4::OnExport() 
{
  if (!m_bShowingData || !m_Tree.m_pGroupEntryPtr)
    return;
	m_Tree.Export();
}


void CFileInfo4::OnColumnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
#if 0
  if (pNMListView->iItem == -1 && pNMListView->iSubItem > 1)
  {
    int i;
    CSize sz;
    CHeaderCtrl *pHdr = m_List.GetHeaderCtrl();
    sz.cx = -5000; sz.cy = 0;
    m_List.LockWindowUpdate();
    m_List.Scroll(sz);

    for (i=2; i<pHdr->GetItemCount(); i++)
    {
      if (i != pNMListView->iSubItem)
        m_List.SetColumnWidth( i, m_bOneCol ? LVSCW_AUTOSIZE_USEHEADER : 0);
    }
    m_List.UnlockWindowUpdate();

    m_bOneCol = !m_bOneCol;
  }
#endif
	*pResult = 0;
}

void CFileInfo4::OnOK()
{
	m_Tree.CleanUp(m_Tree.GetRootItem());

	CDialogEx::OnOK();
}

void CFileInfo4::OnCancel()
{
	m_Tree.CleanUp(m_Tree.GetRootItem());

	CDialogEx::OnCancel();
}

void CFileInfo4::OnBnClickedPhys()
{
	UpdateData();
	if (m_pCurCtrl == &m_DataList)
	{
		if (m_Cache) // Inhalt leeren
		{
			TCHAR **p = m_Cache;
			for (DWORD i=0; i<m_dwInCache; i++,p++)
			{
				free(*p);
				*p = NULL;
			}
			m_DataList.Invalidate();
		}
	}
}

void CFileInfo4::OnTimer(UINT_PTR nIDEvent)
{
  KillTimer(nIDEvent);
  CRect r;
  GetClientRect(&r);
  OnSize(0, r.Width(), r.Height());
  CDialogEx::OnTimer(nIDEvent);
}
