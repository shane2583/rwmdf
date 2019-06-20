// MDF4_SimpleWriter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mdf4.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[])
{
	// Find a reasonalbe time stamp:
	struct M_DATE now;
	memset(&now, 0, sizeof(M_DATE));
	time_t tt;
	time(&tt);
	now.time_ns = tt;
	now.time_ns *= 1000000000; // s -> ns

  // Create the Test-File
	int iVersion = 410; // v4.10
  MDF4File m4;
#ifdef WIN32
#ifdef UNICODE
  if (m4.Create(L"C:\\Temp\\Test.mf4", "SimplWrtr", iVersion))
#else
	if (m4.Create("C:\\Temp\\Test.mf4", "SimplWrtr", iVersion))
#endif
#else
	if (m4.Create("../Test.mf4", "SimplWrtr", 410)) // v4.10
#endif
	{
    // There MUST be at least one FileHistory (FH)
    // which MUST have an MD-Block
#ifdef UNICODE
		wchar_t *pszFileHistory =
			L"<FHcomment>"
			L"<TX>First Test of MDF4 library</TX>"
			L"<tool_id>toolblabla</tool_id>"
			L"<tool_vendor>MDZ Bührer&amp;Partner</tool_vendor>"
			L"<tool_version>0.1</tool_version>"
			L"</FHcomment>\0";
#else
		char *pszFileHistory =
			"<FHcomment>"
			  "<TX>First Test of MDF4 library</TX>"
				"<tool_id>toolblabla</tool_id>"
				"<tool_vendor>MDZ Bührer&amp;Partner</tool_vendor>"
				"<tool_version>0.1</tool_version>"
			"</FHcomment>\0";
#endif
    M4MDBlock fc(pszFileHistory);
    // Create the FH-Block
    M4FHBlock *fh=new M4FHBlock();
    fh->Create(&m4);
    // add the comment
    fh->setComment(fc);
		fh->fh_time = now;
    // and add to the HD Block
    m4.addHistory(fh);

#if 0
    // AT Attachment block
    M4ATBlock *at=new M4ATBlock(&m4);
    at->EmbedFile(_T("C:\\filter.dat"),false,true);
    m4.addAttachment(at);
#endif

	  // UTF8-Test
    M4TXBlock Comment;
    // MD-Block XML test (the sequence is significant for MDF4 spec!)
		char *pszHeaderComment =
      "<HDcomment>"
			  "<TX>nothing special</TX>"
			  "<time_source>EtherCAT DC Reference Time</time_source>"
			  "<common_properties>"
				  "<e name=\"author\">Autor</e>"
					"<e name=\"department\">Department</e>"
					"<e name=\"project\">Project</e>"
					"<e name=\"subject\">Subject</e>"
				"</common_properties>"
			"</HDcomment>";
    M4MDBlock hdComment(pszHeaderComment);
    m4.setComment(hdComment);
		m4.GetHdr()->hd_time_class = 10; // external time source

		m4.setFileTime(now);

    // Create one DG with one CG with two CN's
    // Note: hd.addDataGroup, dg.addChannelGroup, cg.addChannel
    // must be called with an allocated object; they will be deleted
    // when the next object is added or in the destructor!
    M4DGBlock *dg=m4.addDataGroup(new M4DGBlock);
#ifdef WIN32
		dg->setComment(M4TXBlock("DataGroup Comment"));
#else
		M4TXBlock dgc("DataGroup Comment");
    dg->setComment(*(m4Block*)&dgc);
#endif
    M4CGBlock *cg=dg->addChannelGroup(new M4CGBlock);
#ifdef WIN32
		cg->setAcqName(*(m4Block*)&M4TXBlock("AcqName"));
#else
		M4TXBlock dgan("AcqName");
    cg->setAcqName(*(m4Block*)&dgan);
#endif
		cg->setRecordSize(8+4,0);
	  dg->Save(true,true);

    // add the TIME channel
    M4CNBlock *cn=cg->addChannel(new M4CNBlock(CN_T_MASTER));
		cn->cn_sync_type |= CN_S_TIME;
	  cg->Save(true,true);
#ifdef WIN32
		cn->setComment(*(m4Block*)&M4TXBlock("Time Channel"));
		cn->setName(*(m4Block*)&M4TXBlock("Time"));
		cn->setConversion(*(M4CCBlock*)&M4CCLinear(0.0031));
#else
		M4TXBlock cdc("Time Channel");
    cn->setComment(*(m4Block*)&cdc);
		M4TXBlock cnn("Time");
    cn->setName(*(m4Block*)&cnn);
		M4CCLinear cncc(0.0031);
    cn->setConversion(*(M4CCBlock*)&cncc);
#endif
		cn->setLocation(CN_D_UINT_LE,0*8,8*8); // data type: uint 64
		M4TXBlock tx_time_unit("s");
    tx_time_unit.Create(&m4,3);
		tx_time_unit.Save(true,true);
		cn->setLink( m4CNRecord::cn_md_unit, tx_time_unit.m_At);
		cn->Save();
    // add the Value channel; CAUTION this will delete the cn!
    cn=cg->addChannel(new M4CNBlock(CN_T_FIXEDLEN));
		M4TXBlock cnc("Value Channel");
    cn->setComment(*(M4CCBlock*)&cnc);
		M4TXBlock cnname("Value");
    cn->setName(*(M4CCBlock*)&cnname);
    // Win32: cn->setConversion(M4CCLinear(0.25,-800));
		M4CCLinear cncc1(0.25,-800);
    cn->setConversion(*(M4CCBlock*)&cncc1);
    cn->setLocation(CN_D_UINT_LE,8*8,4*8); // data type: DWORD
		M4TXBlock tx_value_unit("V");
    tx_value_unit.Create(&m4,3);
		tx_value_unit.Save(true,true);
		cn->setLink( m4CNRecord::cn_md_unit, tx_value_unit.m_At);
		cn->Save();

		M_UINT32 uiDataRecordSize = cg->cg_data_bytes + dg->dg_rec_id_size; // + rec id
		// the following code is crucial for the effectiveness of storing/reading the data:
		M_UINT32 uiNoOfRecords = 100000; // No. of record per block
		// A data block should be large enough to be read effectively, but small enough to avoid empty space.
		M_UINT32 uiDataBlockSize = uiDataRecordSize * uiNoOfRecords/2; // a good size for large file is 5 MB per block
		M_UINT32 rec_id_size = dg->dg_rec_id_size;

		m4.addRecordCount(cg,1); // Group 1

		m4DataStream *out = NULL; // 4.0 normal blocks
		m4DZStream   *outDZ = NULL; // 4.10 compressed blocks
		if (iVersion == 410)
		{
			outDZ = dg->DZWriteStream(uiDataBlockSize, uiDataRecordSize, 5);
			out = outDZ;
		}
		else
			out = dg->WriteStream(uiDataBlockSize);
		cg->Save(true,true);
		m4.Prepare();

		M_UINT8 *pRecord = (M_UINT8*)calloc(uiDataRecordSize,1);
		for (M_UINT32 i=1; i<=uiNoOfRecords; i++)
		{
			M_UINT8 *p = pRecord;
			if (rec_id_size)
			{
				*p = (M_UINT8)1; // record id
				p+=rec_id_size;
			}
			*(M_UINT64*)p = (M_UINT64)i; // time
			p += sizeof(M_UINT64);
			*(M_UINT32*)p = (M_UINT32)i*10; // Value
			//p += sizeof(DWORD); // no invalid bytes used here

			out->Write(uiDataRecordSize,pRecord);	// append
		}
		out->AddRecords(uiNoOfRecords,1); // group 1

		//free(pRecord);
		out->Flush();
		out->Close();
	  delete out;

    m4.Save();
    m4.Close();
  }
  return 0;
}

