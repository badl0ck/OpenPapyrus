// SLTEST.CPP
// Copyright (c) A.Sobolev 2006, 2007, 2008, 2010, 2012, 2015, 2016
//
// Test Suits
//
#include <slib.h>
#include <tv.h>
#pragma hdrstop
//
//
//
STestDataArray::Item::Item()
{
	THISZERO();
}

STestDataArray::STestDataArray()
{
	HexPool.add("$", 0); // zero index - is empty string
}

const STestDataArray::Item & STestDataArray::Get(uint idx) const
{
	return (idx < L.getCount()) ? L.at(idx) : EmptyItem;
}

int STestDataArray::GetDataByPos(uint pos, SString & rData) const
{
	return HexPool.getnz(pos, rData);
}

uint STestDataArray::GetCount() const
{
	return L.getCount();
}

int SLAPI STestDataArray::ReadBotanTestSequence(int formatVer, const char * pFileName, const char * pZone)
{
	int    ok = -1;
	int    my_zone = 0;
	SString line_buf, temp_buf;
	SFile f_in(pFileName, SFile::mRead);
	THROW(f_in.IsValid());
	while(f_in.ReadLine(line_buf)) {
		line_buf.Chomp();
		if(line_buf.NotEmptyS() && line_buf.C(0) != '#') {
			if(line_buf.C(0) == '[') {
				const char * p = line_buf+1;
				temp_buf = 0;
				while(*p && *p != ']') {
					temp_buf.CatChar(*p++);
				}
				if(temp_buf.CmpNC(pZone) == 0)
					my_zone = 1;
				else
					my_zone = 0;
			}
			else if(my_zone) {
				int    line_contin = 0;
				temp_buf = 0;
				do {
					if(line_contin) {
						line_buf.Chomp().Strip();
					}
					if(line_buf.Last() == '\\') {
						line_contin = 1;
						line_buf.TrimRight().Strip();
					}
					else
						line_contin = 0;
					temp_buf.Cat(line_buf);
				} while(line_contin && f_in.ReadLine(line_buf));
				{
					StringSet ss(':', temp_buf);
					Item   item;
					for(uint sp = 0; ss.get(&sp, temp_buf);) {
						if(temp_buf.NotEmptyS()) {
							THROW(HexPool.add(temp_buf, &item.ValPos[item.Count]));
						}
						item.Count++;
					}
					THROW(L.insert(&item));
				}
				ok = 1;
			}
		}
	}
	CATCHZOK
	return ok;
}
//
//
//
static SString & catval(long v, const char * pV, SString & rBuf)
	{ return rBuf.CatChar('(').CatEq(pV, v).CatChar(')'); }
static SString & catval(ulong v, const char * pV, SString & rBuf)
	{ return rBuf.CatChar('(').Cat(pV).Eq().Cat(v).CatChar(')'); }
static SString & catval(double v, const char * pV, SString & rBuf)
	{ return rBuf.CatChar('(').CatEq(pV, v, MKSFMTD(0, 15, NMBF_NOTRAILZ)).CatChar(')'); }
static SString & catval(LDATE v, const char * pV, SString & rBuf)
	{ return rBuf.CatChar('(').CatEq(pV, v, DATF_DMY|DATF_CENTURY).CatChar(')'); }
static SString & catval(const SString & rS, const char * pV, SString & rBuf)
	{ return rBuf.CatChar('(').CatEq(pV, rS).CatChar(')'); }
static SString & catval(const char * pS, const char * pV, SString & rBuf)
	{ return rBuf.CatChar('(').CatEq(pV, pS).CatChar(')'); }

int STestCase::_check_nz(int result, const char * pV)
{
	if(!result) {
		SString buf;
		SetInfo(buf.CatChar('(').Cat(pV).CatChar(')').Cat("==").CatChar('0'), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_z(int result, const char * pV)
{
	if(result) {
		SString buf;
		buf.CatParStr(pV).Cat("!=").CatChar('0');
		SetInfo(buf, 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_mem(const void * pMem, size_t sz, uint8 pattern, const char * pV)
{
	SString buf;
	for(size_t i = 0; i < sz; i++) {
		if(((uint8 *)pMem)[i] != pattern) {
			(buf = 0).Cat(pV).CatChar('[').Cat(i).CatChar(']').Cat("!=").Cat(pattern);
			SetInfo(buf, 0);
			return 0;
		}
	}
	return 1;
}

enum {
	smtfIncons = 0x0001,
	smtfErrNeg = 0x0002,
	smtfTolBad = 0x0004,
	smtfRetBad = 0x0008,
	smtfErrBad = 0x0010
};

static double test_frac_diff(double x1, double x2)
{
	if(x1 == 0.0 && x2 == 0.0)
		return 0.0;
	else if(x1 == 0.0)
		return fabs(x2);
	else if(x1 <= SMathConst::Max && x2 <= SMathConst::Max && (x1 + x2 != 0.0))
		return fabs((x1-x2)/(x1+x2));
	else
		return 1.0;
}

int STestCase::_check_math_result(SMathResult & r, double val, double tol, const char * pF)
{
#ifdef NDEBUG
	#define TEST_FACTOR 100.0
	#define TEST_SIGMA  1.5
#else
	#define TEST_FACTOR 1.0
	#define TEST_SIGMA  1.0
#endif
	int    s = 0;
	double f = 0.0;
	if(fisnan(r.V) || fisnan(val))
		s = (fisnan(r.V) != fisnan(val)) ? smtfIncons : s;
	else if(fisinf(r.V) || fisinf(val))
		s = (fisinf(r.V) != fisinf(val)) ? smtfIncons : s;
	else {
		f = test_frac_diff(val, r.V);
		if(fabs(val - r.V) > 2.0 * TEST_SIGMA * r.E)
			s |= smtfIncons;
		if(r.E < 0.0)
			s |= smtfErrNeg;
		if(fisinf(r.E))
			s |= smtfErrBad;
		if(f > TEST_FACTOR * tol)
			s |= smtfTolBad;
	}
	if(s != 0) {
		SString msg_buf, temp_buf;
		(msg_buf = pF).CatDiv(':', 1, 1);
		msg_buf.Cat(temp_buf.Printf("expected: %20.16g", val)).Space();
		msg_buf.Cat(temp_buf.Printf("obtained: %20.16g +/- %.16g (rel=%g)", r.V, r.E, r.E/(fabs(r.V) + r.E))).Space();
		msg_buf.Cat(temp_buf.Printf("fracdiff: %20.16g", f));
		if(s & smtfIncons)
			msg_buf.CatDiv(';', 2).Cat("value/expected not consistent within reported error");
		if(s & smtfErrNeg)
			msg_buf.CatDiv(';', 2).Cat("reported error negative");
		if(s & smtfErrBad)
			msg_buf.CatDiv(';', 2).Cat("reported error is bad");
		if(s & smtfTolBad)
			msg_buf.CatDiv(';', 2).Cat("value not within tolerance of expected value");
		SetInfo(msg_buf, 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(uint8 a, uint8 b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval((ulong)b, pB, catval((ulong)a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(uint a, uint b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval((ulong)b, pB, catval((ulong)a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(long a, long b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(ulong a, ulong b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(double a, double b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(float a, float b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq_tolerance(double a, double b, double tol, const char * pA, const char * pB)
{
	if(fabs(a-b) > tol) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq_tolerance(float a, float b, float tol, const char * pA, const char * pB)
{
	if(fabs(a-b) > tol) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(LDATE a, LDATE b, const char * pA, const char * pB)
{
	if(a != b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(SString & rVA, SString & rVB, const char * pA, const char * pB)
{
	if(rVA != rVB) {
		SString buf;
		SetInfo(catval(rVB, pB, catval(rVA, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_eq(SString & rVA, const char * pVB, const char * pA, const char * pB)
{
	if(rVA != pVB) {
		SString buf;
		SetInfo(catval(pVB, pB, catval(rVA, pA, buf).Cat("!=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_le(long a, long b, const char * pA, const char * pB)
{
	if(a > b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat(">")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_le(ulong a, ulong b, const char * pA, const char * pB)
{
	if(a > b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat(">")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_le(double a, double b, const char * pA, const char * pB)
{
	if(a > b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat(">")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_lt(long a, long b, const char * pA, const char * pB)
{
	if(a >= b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat(">=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_lt(double a, double b, const char * pA, const char * pB)
{
	if(a >= b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat(">=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_lt(float a, float b, const char * pA, const char * pB)
{
	if(a >= b) {
		SString buf;
		SetInfo(catval(b, pB, catval(a, pA, buf).Cat(">=")), 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_range(long a, long low, long upp, const char * pA, const char * pLow, const char * pUpp)
{
	if(a < low || a > upp) {
		SString buf;
		catval(a, pA, buf).Space().Cat("out of range").Space().CatChar('[');
		catval(low, pLow, buf).CatCharN('.', 2);
		catval(upp, pUpp, buf).CatChar(']');
		SetInfo(buf, 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_range(ulong a, ulong low, ulong upp, const char * pA, const char * pLow, const char * pUpp)
{
	if(a < low || a > upp) {
		SString buf;
		catval(a, pA, buf).Space().Cat("out of range").Space().CatChar('[');
		catval(low, pLow, buf).CatCharN('.', 2);
		catval(upp, pUpp, buf).CatChar(']');
		SetInfo(buf, 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_range(double a, double low, double upp, const char * pA, const char * pLow, const char * pUpp)
{
	if(a < low || a > upp) {
		SString buf;
		catval(a, pA, buf).Space().Cat("out of range").Space().CatChar('[');
		catval(low, pLow, buf).CatCharN('.', 2);
		catval(upp, pUpp, buf).CatChar(']');
		SetInfo(buf, 0);
		return 0;
	}
	else
		return 1;
}

int STestCase::_check_range(float a, float low, float upp, const char * pA, const char * pLow, const char * pUpp)
{
	if(a < low || a > upp) {
		SString buf;
		catval(a, pA, buf).Space().Cat("out of range").Space().CatChar('[');
		catval(low, pLow, buf).CatCharN('.', 2);
		catval(upp, pUpp, buf).CatChar(']');
		SetInfo(buf, 0);
		return 0;
	}
	else
		return 1;
}
//
// STestCase {
//
STestCase::STestCase(STestSuite * pSuite)
{
	P_Suite = pSuite;
}

STestCase::~STestCase()
{
}

const STestSuite::Entry * STestCase::GetSuiteEntry() const
{
	return P_Suite->GetCurEntry();
}

const char * STestCase::GetTestName() const
{
	return P_Suite->GetCurEntry()->TestName;
}

const char * STestCase::MakeInputFilePath(const char * pFileName)
{
	return (TempBuf = P_Suite->GetCurEntry()->InPath).SetLastSlash().Cat(pFileName);
}

const char * STestCase::MakeOutputFilePath(const char * pFileName)
{
	return (TempBuf = P_Suite->GetCurEntry()->OutPath).SetLastSlash().Cat(pFileName);
}

int STestCase::Run(const char *)
{
	return -1;
}

int STestCase::SetInfo(const char * pMsg, int currentStatus)
{
	if(P_Suite && pMsg)
		P_Suite->PutCaseInfo(pMsg);
	if(currentStatus >= 0)
		CurrentStatus = currentStatus;
	return 1;
}

int STestCase::EnumArg(uint * pArgNo, SString & rBuf) const
{
	const STestSuite::Entry * p_entry = GetSuiteEntry();
	return p_entry->ArgList.get(pArgNo, rBuf);
}

STestCase::TabEnum::TabEnum(const char * pTabFileName, const char * pTabName)
{
	RowIdx = 0;
	State = 0;
	STabFile tab_file(pTabFileName);
	if(!tab_file.LoadTab(pTabName, Tab))
		State |= stError;
}

int STestCase::TabEnum::Next(void * pData)
{
	int    ok = 0;
	if(!(State & stError)) {
		if(RowIdx < Tab.GetCount()) {
			if(pData) {
				STab::Row * p_row = (STab::Row *)pData;
				assert(p_row->IsConsistent());
				if(p_row->IsConsistent())
					ok = Tab.GetRow(RowIdx++, *p_row);
			}
		}
		else
			SLS.SetError(SLERR_BOUNDS);
	}
	return ok;
}

SEnumImp * STestCase::EnumTab(const char * pTabName)
{
	TabEnum * p_en = 0;
	if(P_Suite) {
		const char * p_tab_file_name = P_Suite->GetTabFileName();
		if(fileExists(p_tab_file_name)) {
			p_en = new TabEnum(p_tab_file_name, pTabName);
			if(p_en->State & TabEnum::stError)
				ZDELETE(p_en);
		}
	}
	return p_en;
}
//
// }
//
STestSuite::Entry::Entry() : BmrList(sizeof(Benchmark))
{
	MaxCount = SuccCount = FailCount = 0;
	MEMSZERO(HeapBefore);
	MEMSZERO(HeapAfter);
	Timing = 0;
}
//
//
//
STestSuite::STestSuite()
{
	P_List = new TSCollection <Entry>;
	CurIdx = 0;
}

STestSuite::~STestSuite()
{
	delete (TSCollection <Entry> *)P_List;
}

void STestSuite::PutCaseInfo(const char * pMsg)
{
	if(pMsg)
		CaseBuffer.CatDiv(';', 0, 1).Cat(pMsg);
}

int STestSuite::LoadTestList(const char * pIniFileName)
{
	int    ok = 1;
	uint   i;
	SString def_in_path, def_out_path;
	StringSet sect_list;
	SString sect_buf, param_buf, cur_path, temp_buf;
	SIniFile ini_file(pIniFileName);
	THROW(ini_file.IsValid());
	THROW(ini_file.GetSections(&sect_list));
	cur_path = pIniFileName;
	{
		SPathStruc ps;
		ps.Split(cur_path);
		ps.Merge(0, SPathStruc::fNam|SPathStruc::fExt, cur_path);
		cur_path.SetLastSlash();
	}
	((TSCollection <Entry> *)P_List)->freeAll();
	LogFileName = 0;
	for(i = 0; sect_list.get(&i, sect_buf);) {
		if(sect_buf.CmpNC("common") == 0) {
			if(ini_file.GetParam(sect_buf, "logfile", param_buf) > 0)
				LogFileName = param_buf;
			if(ini_file.GetParam(sect_buf, "input", param_buf) > 0)
				def_in_path = param_buf;
			if(ini_file.GetParam(sect_buf, "output", param_buf) > 0)
				def_out_path = param_buf;
			if(ini_file.GetParam(sect_buf, "tabfile", param_buf) > 0)
				TabFileName = param_buf;
		}
		else {
			int ival = 0;
			if(ini_file.GetIntParam(sect_buf, "disable", &ival) > 0 && ival != 0) {
				; // Test disabled
			}
			else {
				Entry * p_entry = new Entry;
				p_entry->MaxCount = 1;
				p_entry->TestName = sect_buf;
				if(ini_file.GetParam(sect_buf, "descr", param_buf) > 0)
					p_entry->Descr = param_buf;
				if(ini_file.GetParam(sect_buf, "arglist", param_buf) > 0) {
					StringSet ss(';', param_buf);
					for(uint k = 0; ss.get(&k, temp_buf);)
						p_entry->ArgList.add(temp_buf.Strip());
				}
				if(ini_file.GetParam(sect_buf, "benchmark", param_buf) > 0) {
					StringSet ss(';', param_buf);
					for(uint k = 0; ss.get(&k, temp_buf);) {
						if(temp_buf.NotEmptyS()) {
							p_entry->BenchmarkList.add(temp_buf);
							Benchmark bm;
							MEMSZERO(bm);
							p_entry->BmrList.insert(&bm);
						}
					}
				}
				if(ini_file.GetParam(sect_buf, "input", param_buf) > 0)
					p_entry->InPath = param_buf;
				if(p_entry->InPath.Empty())
					p_entry->InPath = (!!def_in_path.NotEmptyS()) ? def_in_path : cur_path;
				if(ini_file.GetParam(sect_buf, "output", param_buf) > 0)
					p_entry->OutPath = param_buf;
				if(p_entry->OutPath.Empty())
					p_entry->OutPath = (!!def_out_path.NotEmptyS()) ? def_out_path : cur_path;
				if(ini_file.GetIntParam(sect_buf, "count", &ival) > 0)
					p_entry->MaxCount = (ival > 0) ? ival : 1;
				((TSCollection <Entry> *)P_List)->insert(p_entry);
			}
		}
	}
	if(!TabFileName.NotEmptyS())
		SPathStruc::ReplaceExt(TabFileName = pIniFileName, "tab", 1);
	CATCH
		ok = 0;
	ENDCATCH
	return ok;
}

#define FILE_TIME_TO_QWORD(ft) (Int64ShllMod32(ft.dwHighDateTime, 32) | ft.dwLowDateTime)

static inline int64 SLAPI getprofiletime(HANDLE thrId)
{
	FILETIME ct_tm, end_tm, k_tm, user_tm;
	GetThreadTimes(thrId, &ct_tm, &end_tm, &k_tm, &user_tm);
	return (FILE_TIME_TO_QWORD(user_tm) + FILE_TIME_TO_QWORD(k_tm));
}

static inline int64 SLAPI getprofilesystime()
{
	FILETIME tm;
	SYSTEMTIME stm;
	GetSystemTime(&stm);
	SystemTimeToFileTime(&stm, &tm);
	return FILE_TIME_TO_QWORD(tm);
}

int STestSuite::ReportTestEntry(int title, const Entry * pEntry)
{
	int    ok = 1;
	SString line_buf, bm_buf;
	if(title) {
		line_buf.Cat("Result").Semicol();
		line_buf.Cat("Test").Semicol().
			Cat("Descr").Semicol().
			Cat("SuccCount").Semicol().
			Cat("FailCount").Semicol().
			Cat("Timing").Semicol().
			Cat("SysTiming").Semicol().
			Cat("IncMemBlk").Semicol().
			Cat("IncMemSize");
		SLS.LogMessage(LogFileName, line_buf);
	}
	else {
		ulong inc_mem_blk  = pEntry->HeapAfter.UsedBlockCount - pEntry->HeapBefore.UsedBlockCount;
		ulong inc_mem_size = pEntry->HeapAfter.UsedSize - pEntry->HeapBefore.UsedSize;
		if(pEntry->FailCount != 0)
			line_buf.Cat("FAIL");
		else if(pEntry->SuccCount)
			line_buf.Cat("OK");
		else
			line_buf.Cat("NOTHING");
		line_buf.Semicol();
		line_buf.Cat(pEntry->TestName).Semicol().Cat(pEntry->Descr).Semicol().
			Cat(pEntry->SuccCount).Semicol().Cat(pEntry->FailCount).Semicol().
			Cat((long)(pEntry->Timing / 10000L)).Semicol().
			Cat((long)(pEntry->SysTiming / 10000L)).Semicol().
			Cat(inc_mem_blk).Semicol().Cat(inc_mem_size);
		if(CaseBuffer.NotEmpty())
			line_buf.Semicol().Cat(CaseBuffer);
		SLS.LogMessage(LogFileName, line_buf);
		for(uint i = 0; i < pEntry->BmrList.getCount(); i++) {
			const Benchmark * p_bm = (Benchmark *)pEntry->BmrList.at(i);
			ulong inc_mem_blk  = p_bm->HeapAfter.UsedBlockCount - p_bm->HeapBefore.UsedBlockCount;
			ulong inc_mem_size = p_bm->HeapAfter.UsedSize - p_bm->HeapBefore.UsedSize;
			(line_buf = 0).Cat("BENCHMARK").Semicol();
			for(uint j = 0, k = 0; pEntry->BenchmarkList.get(&k, bm_buf); j++) {
				if(j == i)
					break;
				else
					bm_buf = 0;
			}
			line_buf.Cat(pEntry->TestName).CatDiv('-', 0).Cat(bm_buf);
			line_buf.Semicol().Semicol().Semicol().Semicol().
				Cat((long)(p_bm->Timing / 10000L)).Semicol().
				Cat((long)(p_bm->SysTiming / 10000L)).Semicol().
				Cat(inc_mem_blk).Semicol().Cat(inc_mem_size);
			if(CaseBuffer.NotEmpty())
				line_buf.Semicol().Cat(CaseBuffer);
			SLS.LogMessage(LogFileName, line_buf);
		}
	}
	return ok;
}

const SString & STestSuite::GetTabFileName() const
{
	return TabFileName;
}

const STestSuite::Entry * STestSuite::GetCurEntry() const
{
	if(CurIdx < ((TSCollection <Entry> *)P_List)->getCount())
		return ((TSCollection <Entry> *)P_List)->at(CurIdx);
	else
		return 0;
}

int STestSuite::Run(const char * pIniFileName)
{
	int    ok = 1;
	if(LoadTestList(pIniFileName)) {
		const HANDLE thr_id = GetCurrentThread();
		if(LogFileName.NotEmpty() && !fileExists(LogFileName))
			ReportTestEntry(1, 0);
		// ������������ ���������� ������� ����� ��� ������ ������ ������, ���� � ����������
		// �� ���� ���������� ����� ������ (����� ����� ������ ��������� ����������� ������ ������������ ������� ������)
		CaseBuffer.CatCharN(' ', 8192);
		SString temp_buf(8192);
		for(CurIdx = 0; CurIdx < ((TSCollection <Entry> *)P_List)->getCount(); CurIdx++) {
			STestCase * p_case = 0;
			Entry * p_entry = ((TSCollection <Entry> *)P_List)->at(CurIdx);
			MEMSZERO(p_entry->HeapBefore);
			MEMSZERO(p_entry->HeapAfter);
			p_entry->SuccCount = p_entry->FailCount = 0;
			p_entry->Timing = 0;
			p_entry->SysTiming = 0;
			SString ffn;
			ffn.Cat("SLTCF_").Cat(p_entry->TestName);
			FN_SLTEST_FACTORY f = (FN_SLTEST_FACTORY)GetProcAddress(SLS.GetHInst(), ffn);
			if(f) {
				CaseBuffer = 0;
				// @v9.0.8 assert(Mht.CalcStat(&p_entry->HeapBefore));
				p_case = f(this);
				if(p_case) {
					int64 tm_start, tm_finish;
					int64 tmsys_start, tmsys_finish;
					{
						uint  c = p_entry->MaxCount;
						uint  succ = 0, fail = 0; // �������� �������� � ��������� ��������
						tmsys_start = getprofilesystime();
						tm_start = getprofiletime(thr_id);
						if(c) do {
							p_case->SetInfo(0, 1); // ��������������� ��������� ������� ���������� (OK=1)
							if(p_case->Run(0))
								++succ;
							else
								++fail;
						} while(--c);
						tm_finish = getprofiletime(thr_id);
						tmsys_finish = getprofilesystime();
						p_entry->Timing = tm_finish - tm_start;
						p_entry->SysTiming = tmsys_finish - tmsys_start;
						p_entry->SuccCount = succ;
						p_entry->FailCount = fail;
					}
					//
					// ��������� ��������� (���� ���� �� ���� �������� ������ ���������� � �������,
					// �� ��������� �� ���������, ���, ��� ������).
					//
					if(!p_entry->FailCount) {
						for(uint bm_p = 0, bmr_i = 0; p_entry->BenchmarkList.get(&bm_p, temp_buf); bmr_i++) {
							Benchmark * p_bm = (Benchmark *)p_entry->BmrList.at(bmr_i);
							uint  c = p_entry->MaxCount;
							tmsys_start = getprofilesystime();
							tm_start = getprofiletime(thr_id);
							if(c) do {
								p_case->Run(temp_buf);
							} while(--c);
							tm_finish = getprofiletime(thr_id);
							tmsys_finish = getprofilesystime();
							p_bm->Timing = tm_finish - tm_start;
							p_bm->SysTiming = tmsys_finish - tmsys_start;
						}
					}
					ZDELETE(p_case);
					// @v9.0.8 assert(Mht.CalcStat(&p_entry->HeapAfter));
				}
			}
			else { // @v5.7 ANDREW ��������� � ���, ��� ������� ����� �� ������� {
				LPVOID msg_buff;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
				    FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &msg_buff, 0, NULL);
				MessageBox(NULL, ffn.CatDiv(':', 2).Cat((LPCTSTR)msg_buff), "Error", MB_OK);
				LocalFree(msg_buff);
			} // } @v5.7 ANDREW
			ReportTestEntry(0, p_entry);
		}
	}
	else
		ok = 0;
	return ok;
}
//
//
//
#if 0 // {

typedef int (*STestMemTransformFunc)(int op, const SBaseBuffer & rSrcBuf, SBaseBuffer & rDestBuf, void * pExtra);

struct TestMemParam {
	enum {
		fReversibility = 0x0001 // ����������� ����������� �������������� //
	};
	TestMemParam();

	long   Flags;
	size_t MaxInputSize;
	double DestBufRate;    // ����������� ������� ������-��������� ������������ �������
		// ������-���������. �� ��������� 1.0f.
		// ������������� �������� �������� ������������� ������ ������-��������� ������ ((size_t)-DestBufRate)
	const SBaseBuffer SrcBuf;
	const SBaseBuffer ExpectedDest;
};

TestMemParam::TestMemParam()
{
	Flags = 0;
	MaxInputSize = 0;
	DestBufRate = 1.0;
	SrcBuf.Init();
	ExpectedDest.Init();
}

static size_t CalcDestBufSize(size_t srcSize, double rate)
{
	if(rate < 0.0)
		return (size_t)(-rate);
	else if(rate == 0.0)
		return srcSize;
	else
		return (size_t)(srcSize * rate);
}

class TestMemBuffer : public STempBuffer {
public:
	TestMemBuffer(size_t sz)
	{
		NomSize = sz;
		GapSize = SLS.GetTLA().Rg.GetUniformInt(sizeof(GapData))+1;
		assert(GapSize > 0 && GapSize <= sizeof(GapData));
		Alloc(sz+GapSize*2);
		memcpy(P_Buf, GapData, GapSize);
		memcpy(P_Buf+GapSize+NomSize, GapData, GapSize);
	}
	SBaseBuffer GetBuf()
	{
		SBaseBuffer ret_buf;
		ret_buf.Set(P_Buf+GapSize, NomSize);
		return ret_buf;
	}
	int TestGap() const
	{
		for(uint i = 0; i < GapSize; i++) {
			if(P_Buf[i] != GapData[i])
				return 0;
			else if(P_Buf[GapSize+NomSize+i] != GapData[i])
				return 0;
		}
		return 1;
	}
private:
	size_t NomSize;  // ����������� ������ ������
	size_t GapSize;
	static const uint8 GapData[9];
};

//static
const uint8 TestMemBuffer::GapData[9] = { 1, 3, 7, 9, 2, 4, 8, 10, 11 };

int STestCase::TestMemTransform(TestMemParam & rParam, STestMemTransformFunc func, void * pExtra)
{
	enum {
		errSuccess = 0,
		errSlErr,
		errTransformFunc_Forward,
		errTransformFunc_Backward,
		errGapViolation,
		errFwdCmpSize,
		errFwdCmpMem,
		errBkwdCmpSize,
		errBkwdCmpMem,
	};
	int    ok = 1;
	int    err_code = 0;
	int    outer_data = 0;
	size_t dest_buf_size = 0;
	SBaseBuffer src_buf;
	src_buf = rParam.SrcBuf;
	EXCEPTVAR(err_code);
	if(src_buf.P_Buf) {
		outer_data = 1;
	}
	else {
		outer_data = 0;
		size_t src_size = NZOR(rParam.MaxInputSize, 8192);
		src_size = (size_t)SLS.GetTLA().Rg.GetUniformInt(src_size);
		THROW_V(src_buf.Alloc(src_size, errSlErr);
		IdeaRandMem(src_buf.P_Buf, src_buf.Size);
	}
	{
		dest_buf_size = CalcDestBufSize(rParam.SrcBuf.Size, rParam.DestBufRate);
		TestMemBuffer dest_buf(dest_buf_size);
		THROW_V(dest_buf.IsValid(), errSlErr);
		{
			SBaseBuffer _dest = dest_buf.GetBuf()
			THROW_V(func(1, rParam.SrcBuf, _dest, pExtra), errTransformFunc_Forward);
			THROW_V(dest_buf.TestGap(), errGapViolation);
			if(outer_data && rParam.ExpectedDest.P_Buf) {
				THROW_V(rParam.ExpectedDest.Size == _dest.Size, errFwdCmpSize);
				THROW_V(memcmp(rParam.ExpectedDest.P_Buf, _dest.P_Buf, _dest.Size) == 0, errFwdCmpMem);
			}
			if(outer_data && rParam.Flags & TestMemParam::fReversibility) {
				TestMemBuffer rev_buf(rParam.SrcBuf.Size);
				SBaseBuffer _rev = rev_buf.GetBuf();
				THROW_V(rev_buf.IsValid(), errSlErr);
				THROW_V(func(-1, dest_buf.GetBuf(), _rev, pExtra), errTransformFunc_Backward);
				THROW_V(rev_buf.TestGap(), errGapViolation);
				THROW_V(_rev.Size == src_buf.Size, errBkwdCmpSize);
				THROW_V(memcmp(_rev.P_Buf, src_buf.P_Buf, src_buf.Size) == 0, errBkwdCmpMem);
			}
		}
	}
	CATCH
		ok = 0;
	ENDCATCH
	if(!outer_data)
		src_buf.Destroy();
	return ok;
}

#endif // } 0