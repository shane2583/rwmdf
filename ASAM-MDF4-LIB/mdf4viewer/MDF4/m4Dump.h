#pragma once
#include "mdf4.h"

#if !_MDF4_DUMP
#error please #define _MDF4_DUMP 1
#endif

#if _MDF4_WONLY
#error DUMP is impossible with WONLY
#endif

class m4TreeItem;
typedef ptrlist<m4TreeItem> m4Tree;
class m4TreeItem
{
public:
  m4TreeItem(LPCTSTR Text=NULL) : m_Text(Text)
  {
    m_hwTree=NULL;
    m_hItem=NULL;
  }
  virtual ~m4TreeItem()
  {
  }
  void InsertTree(HWND hwTree,HTREEITEM hParent=TVI_ROOT)
  {
    m_hwTree=hwTree;
    m_hItem=m_hwTree.InsertItem(m_Text,hParent,TVI_LAST);
    m_hwTree.SetItemData(m_hItem,(DWORD_PTR)this);
    for(m4Tree::iterator s=m_Tree; s; ++s)
      s->InsertTree(hwTree,m_hItem);
  }
  virtual m4TreeItem *GetFile()
  {
    if (m_hItem)
    {
      m4TreeItem *p;
      HTREEITEM hPar=m_hwTree.GetParentItem(m_hItem);
      if (hPar && (p=(m4TreeItem *)m_hwTree.GetItemData(hPar))!=NULL)
        return p->GetFile();
    }
    return NULL;
  }
  void Delete()
  {
    if (m_hItem) m_hwTree.DeleteItem(m_hItem);
  }
  CString m_Text;
  CString m_Definition;
  CTreeViewCtrl  m_hwTree;
  HTREEITEM m_hItem;
  m4Tree    m_Tree;
};
class m4TreeFile : public m4TreeItem
{
public:
  m4TreeFile(LPCTSTR Title) : m4TreeItem(Title)
  {
  }
  virtual ~m4TreeFile()
  {
  }
  virtual m4TreeItem *GetFile()
  {
    return this;
  }
  bool Open(LPCTSTR Path);
  MDF4File m_m4;
};
class m4TreeLink : public m4TreeItem
{
public:
  m4TreeLink(LPCTSTR linkNam,M_LINK linkAddr)
  {
    m_Text.Format(_T("%s=%016I64X"),linkNam,linkAddr);
  }
};
