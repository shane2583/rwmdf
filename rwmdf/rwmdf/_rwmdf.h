#pragma once
#include "_rwmdftypes.h"
#include <vector>

using namespace std;


typedef vector<DG*>	ary_dgs;
class _RWMDF
{

public:
	_RWMDF();
	~_RWMDF();

private:
	MDF4File	m_m4;
	M4HDBlock	*m_pHdr;
	ary_dgs		m_dgs;

	M4DGBlock* get_dg(unsigned long index);
	unsigned long get_dg_count();
	unsigned long load_dgs();


	m4Block* LoadLink(MDF4File &m4, m4Block *pParent, int linkNo);
public:
	bool open(const char *filename);
	
	//get_mdf_info();

	CG* get_cg(unsigned long index);
	unsigned long get_cg_count();

	CN* get_cn(const char* cn_name);
	unsigned long get_cn_count();

	ary_dgs& get_dgs();
	void clear_dgs();

	BOOL get_record(M4DGBlock *dg, M4CGBlock *cg, M_BYTE *ptr, M_UINT64 ix1, M_UINT64 ix2);

};

/*

	Public Slots:
	void CacheData(int bTime, int lFirstIndex, int lLastIndex, int& nValuesCached);
	void CacheSRData(int nBlock, int bTime, int lFirstIndex, int lLastIndex, int& nValuesCached);
	void CachedSRValues(int bTime, int lIndex, double& Min, double& Max, double& Mean);
	double CachedValue(int bTime, int lIndex);
	QString Comment(int nBlock, int nElement);

	void GetData(int bTime, int lFirstIndex, int lLastIndex, QVariant& pBuffer, int& nValuesRead);
	void GetDataEx(int bTime, int lFirstIndex, int lLastIndex, QVariant& pBuffer, int& nValuesRead);
	void GetGroupName(QString& strGroupName, int iGroupIndex);
	void GetRecord(qlonglong i64StartIndex, qlonglong i64EndIndex, uint& pBuffer);
	void GetSignal(QString& strDisplayName, QString& strAliasName, QString& strUnit, QString& strComment, int& discrete);
	void GetSignalDetail(int& lDataType, int& nFirstBit, int& nBits, double& Factor, double& Offset, double& RawMin, double& RawMax, int& bHasNoValues, int& invalPos);
	void GetTimeSignal(QString& strUnit, int& monotony, double& rmin, double& rmax, double& raster, int& bIsVirtual);
	void LoadGroup(int iGroupIndex);
	void LoadSignal(QString& strName, int iSignalIndex);
	void LoadTimeSignal(QString& strTimeName);
	void OpenMDF4(QString strPathName);
	int SRCycleCount(int nBlock);
	double SRdt(int nBlock);
	void TimeToIndex(double TimeStamp, int lStartIndex, int& lIndex);

	Signals:
	void exception(int code, QString source, QString disc, QString help);
	void propertyChanged(QString name);
	void signal(QString name, int argc, void* argv);

	Properties:
	QString objectName;
	int FileTime;
	double FirstTimestamp;
	int InvalidBytes;
	double LastTimestamp;
	qulonglong MDF4File;
	int NGroups;
	int NSamples;
	qulonglong NSamples64;
	int NSignals;
	qlonglong NanoTimeUTC;
	int NoOfSRBlocks;
	int RecordSize;
	double TimeFraction;
	int TimerQualityClass;
	int Version;
	QString control;
	int dst_offset_min;
	int time_flags;
	int tz_offset_min;

*/


