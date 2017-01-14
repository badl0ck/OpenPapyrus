// CHKPAN.CPP
// Copyright (c) A.Sobolev 1998-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017
// @codepage windows-1251
// ������ ����� �������� �����
//
#include <pp.h>
#pragma hdrstop
#include <ppsoapclient.h>
#include <gdiplus.h>
//
// ������� ��� ������ ������ �� �������� � �������� �������
//
// ESC 'p' m n1 n2
//
// ���������:
//    m = 0 ��� 1 - �� ���� ��� ������
//    n1 [0..255] - �� ����, ��� ������ (��������, ������������)
//    n2 [0..255] - �� ����, ��� ������ (��������, ������������)
//
// Example: 1B 70 0 5 5
// "1B70000505"
//

#define DEFAULT_TS_FONTSIZE        24
#define INSTVSRCH_THRESHOLD        50000 // @v8.6.6 7-->50000
#define TSGGROUPSASITEMS_FONTDELTA 4
#define UNDEF_CHARGEGOODSID        -1000000000L

#define GRP_IBG   1
#define GRP_SCARD 2

//PPCustDisp * SLAPI GetCustDisp(PPID cashNodeID); // Prototype (CUSTDISP.CPP)
//PPBnkTerminal * SLAPI GetBnkTerm(PPID bnkTermID, const char * pPort, const char * pPath); // Prototype (BNKTERM.CPP)
int SLAPI showInputLineCalc(TDialog *, uint);    // Prototype (VTBUTTON.CPP)
//
// ��������� ������ ����� �������� ����� � ���������� �������� //
//
// - (1) sEMPTYLIST_EMPTYBUF ������ ������ �����, ������ ����� �����
//   ! 1 ������� ����� �� ����                  ��� + Enter
//   ! 2 ������� ����� �� ������������          F2
//   ! 3 ������� ����� �� ����                  ���� + F4
//   ! 4 �������� ����� �������� ������         Ctrl-F5
//    11 ��������� �����                        ��� ����� + F3
//   ! 5 ������� ������                         Escape
//    13 ������� ���������� ���                 F8
//
// - (2) sEMPTYLIST_BUF      ������ ������ �����, � ������ ����� ���� �����
//   ! 1 ������� ����� �� ����                  ��� + Enter
//   ! 2 ������� ����� �� ������������          F2
//   ! 3 ������� ����� �� ����                  ���� + F4
//   ! 6 ������ ����������                      ���������� + F6 or "* ����������" or "���������� *"
//    11 ��������� �����                        ��� ����� + F3
//   ! 7 ������ ����� �� ������ ����� � ������  Enter
//   ! 8 �������� ����� �����                   Escape
//
// - (3) sLIST_EMPTYBUF      �������� ������ �����, ������ ����� �����
//   ! 1 ������� ����� �� ����                  ��� + Enter
//   ! 2 ������� ����� �� ������������          F2
//   ! 3 ������� ����� �� ����                  ���� + F4
//    11 ��������� �����                        ��� ����� + F3
//   ! 9 �������� ������ �����                  Escape
//   !10 �������� ���                           Enter
//    12 �������� ���                           F8
//
// - (4) sLIST_BUF           �������� ������ �����, � ������ ����� ���� �����
//   ! 1 ������� ����� �� ����                  ��� + Enter
//   ! 2 ������� ����� �� ������������          F2
//   ! 3 ������� ����� �� ����                  ���� + F4
//   ! 6 ������ ����������                      ���������� + F6 or "* ����������" or "���������� *"
//    11 ��������� �����                        ��� ����� + F3
//   ! 7 ������ ����� �� ������ ����� � ������  Enter
//   ! 8 �������� ����� �����                   Escape
//
// - (5) sLISTSEL_EMPTYBUF   ����� ������ ����� �� ���� �������, ������ ����� �����
//   ! 2 ������� ����� �� ���� �������          F2
//   ! 9 �������� ������ �����                  Escape
//   !10 �������� ���                           Enter
//
// - (6) sLISTSEL_BUF        ����� ������ ����� �� ���� �������, � ������ ����� ���� �����
//   ! 2 ������� ����� �� ���� �������          F2
//   ! 6 ������ ����������                      ���������� + F6 or "* ����������" or "���������� *"
//   ! 7 ������ ����� �� ������ ����� � ������  Enter
//   ! 8 �������� ����� �����                   Escape
//
//
//
//
struct SaComplexEntry {
	SaComplexEntry()
	{
		GoodsID = 0;
		FinalGoodsID = 0;
		Qtty = 1.0;
		OrgPrice = 0.0;
		FinalPrice = 0.0;
		Flags = 0;
	}
	int    SaComplexEntry::IsComplete() const
	{
		return BIN(GoodsID && (!(Flags & fGeneric) || FinalGoodsID));
	}
	int    SaComplexEntry::Subst(uint genListIdx)
	{
		int    ok = 1;
		if(genListIdx < GenericList.getCount()) {
			FinalGoodsID = GenericList.at(genListIdx).Key;
			OrgPrice = GenericList.at(genListIdx).Val;
		}
		else
			ok = 0;
		return ok;
	}
	enum {
		fGeneric = 0x0001 // ����� GoodsID �������� ����������
	};
	PPID   GoodsID;
	PPID   FinalGoodsID;
	long   Flags;
	double Qtty;
	double OrgPrice;
	double FinalPrice;
	RAssocArray GenericList; // Key - goodsID, Val - price
};

class SaComplex : public TSArray <SaComplexEntry> {
public:
	SaComplex();
	void   Init(PPID goodsID, PPID strucID, double qtty);
	int    SetQuantity(double qtty);
	int    RecalcFinalPrice();
	int    Subst(uint itemIdx, uint entryItemIdx);
	int    IsComplete() const;

	PPID   GoodsID;
	PPID   StrucID;
	double Qtty;
	double Price;
private:
	virtual void FASTCALL freeItem(void *);
};

SaComplex::SaComplex() : TSArray <SaComplexEntry> ()
{
	GoodsID = 0;
	StrucID = 0;
	Qtty = 0.0;
	Price = 0.0;
}

void SaComplex::Init(PPID goodsID, PPID strucID, double qtty)
{
	GoodsID = goodsID;
	StrucID = strucID;
	Qtty = qtty;
	Price = 0.0;
	freeAll();
}

int SaComplex::IsComplete() const
{
	for(uint i = 0; i < getCount(); i++)
		if(!at(i).IsComplete())
			return 0;
	return 1;
}

int SaComplex::SetQuantity(double qtty)
{
	int    ok = 1;
	if(qtty > 0.0) {
		const uint c = getCount();
		if(c) {
			if(Qtty > 0.0) {
				double rel = qtty / Qtty;
				for(uint i = 0; i < c; i++)
					at(i).Qtty *= rel;
			}
			else {
				for(uint i = 0; i < c; i++)
					at(i).Qtty = qtty;
			}
		}
		Qtty = qtty;
	}
	else
		ok = 0;
	return ok;
}

int SaComplex::RecalcFinalPrice()
{
	int    ok = 1;
	RAssocArray template_list, list;
	for(uint i = 0; i < getCount(); i++) {
		const SaComplexEntry & r_entry = at(i);
		template_list.Add(r_entry.GoodsID, r_entry.OrgPrice * r_entry.Qtty);
	}
	if(template_list.Distribute(Price * Qtty, RAssocArray::dfRound|RAssocArray::dfReset, 0, list) > 0) {
		for(uint i = 0; i < getCount(); i++) {
			SaComplexEntry & r_entry = at(i);
			r_entry.FinalPrice = (r_entry.Qtty > 0.0) ? (list.Get(r_entry.GoodsID) / r_entry.Qtty) : r_entry.OrgPrice;
		}
	}
	else
		ok = 0;
	return ok;
}

int SaComplex::Subst(uint itemIdx, uint entryItemIdx)
{
	return (itemIdx < getCount() && at(itemIdx).Subst(entryItemIdx) > 0) ? RecalcFinalPrice() : 0;
}

//virtual
void FASTCALL SaComplex::freeItem(void * pItem)
{
	((SaComplexEntry *)pItem)->GenericList.freeAll();
}
//
//
//
CPosProcessor::Packet::Packet()
{
	Clear();
}

CPosProcessor::Packet & FASTCALL CPosProcessor::Packet::operator = (CCheckItemArray & rS)
{
	*(CCheckItemArray *)this = rS;
	return *this;
}

void CPosProcessor::Packet::Clear()
{
	TableCode = 0;
	GuestCount = 0;
	Reserve = 0;
	AgentID__ = 0;
	OrgAgentID = 0; // @v8.2.0
	OrderCheckID = 0;
	OrgUserID = 0;
	freeAll();
	ClearCur();
	IterIdx = 0;
	Eccd.Clear();
	GiftAssoc.clear();
}

void CPosProcessor::Packet::ClearCur()
{
	CurPos = -1;
	MEMSZERO(Cur);
	CurModifList.freeAll();
	Rest = 0.0;
}

int CPosProcessor::Packet::ClearGift()
{
	int    ok = -1;
	uint c = getCount();
	if(c) do {
		--c;
		CCheckItem & r_item = at(c);
		if(r_item.Flags & cifGift) {
			atFree(c);
			ok = 1;
		}
		else {
			r_item.ResetGiftQuot();
			r_item.Flags &= ~(cifUsedByGift|cifMainGiftItem);
			ok = 1;
		}
	} while(c);
	GiftAssoc.clear();
	return 1;
}

PPID CPosProcessor::Packet::GetAgentID(int actual) const
{
	return actual ? AgentID__ : NZOR(OrgAgentID, AgentID__);
}

double CPosProcessor::Packet::GetGoodsQtty(PPID goodsID) const
{
	double qtty = 0.0;
	for(uint i = 0; i < getCount(); i++) {
		const CCheckItem & r_item = at(i);
		if(r_item.GoodsID == goodsID)
			qtty += r_item.Quantity;
	}
	return qtty;
}

int CPosProcessor::Packet::HasCur() const
{
	return BIN(CurPos >= 0);
}

int CPosProcessor::Packet::SetupCCheckPacket(CCheckPacket * pPack) const
{
	if(pPack) {
		// @v8.2.11 {
		if(OrgUserID) {
			pPack->Rec.UserID = OrgUserID;
		}
		else {
			GetCurUserPerson(&pPack->Rec.UserID, 0);
		}
		// } @v8.2.11
		// @v8.2.11 pPack->Rec.UserID = NZOR(OrgUserID, LConfig.User); // @v8.2.5 OrgUserID-->NZOR(OrgUserID, LConfig.User)
		pPack->Ext.SalerID = GetAgentID(0);
		pPack->Ext.TableNo = TableCode;
		pPack->Ext.GuestCount = GuestCount;
		pPack->Ext.LinkCheckID = (pPack->Rec.Flags & CCHKF_SKIP) ? 0 : OrderCheckID;
		pPack->Ext.CreationDtm = Eccd.InitDtm;
		Eccd.Memo.CopyTo(pPack->Ext.Memo, sizeof(pPack->Ext.Memo));
		SETFLAG(pPack->Rec.Flags, CCHKF_DELIVERY,   Eccd.Flags & Eccd.fDelivery);
		SETFLAG(pPack->Rec.Flags, CCHKF_FIXEDPRICE, Eccd.Flags & Eccd.fFixedPrice); // @v8.7.7
		if(Eccd.Flags & Eccd.fDelivery) {
			pPack->SetDlvrAddr(&Eccd.Addr_);
			pPack->Ext.StartOrdDtm = Eccd.DlvrDtm;
		}
	}
	return 1;
}

int CPosProcessor::Packet::SetupInfo(SString & rBuf)
{
	SString word;
	rBuf = 0;
	if(GetAgentID(1)) {
		//PPGetWord(PPWORD_SALER, 0, rBuf).CatDiv(':', 2);
		PPLoadString("seller", rBuf);
		rBuf.CatDiv(':', 2);
		GetArticleName(GetAgentID(1), word);
		if(rBuf.NotEmpty())
			rBuf.CatCharN(' ', 4);
		rBuf.Cat(word);
	}
	if(TableCode) {
		if(rBuf.NotEmpty())
			rBuf.CatCharN(' ', 4);
		PPLoadString("ftable", word);
		rBuf.Cat(word).CatDiv(':', 2).Cat(TableCode);
		if(GuestCount) {
			PPLoadString("guestcount", word);
			rBuf.CatCharN(' ', 4).Cat(word).CatDiv(':', 2).Cat(GuestCount);
		}
	}
	if(GetCur().Division) {
		if(rBuf.NotEmpty())
			rBuf.CatCharN(' ', 4);
		// @v9.2.7 PPGetWord(PPWORD_POSDIVISION, 0, word);
		PPLoadString("department", word); // @v9.2.7
		rBuf.Cat(word).CatDiv(':', 2).Cat(GetCur().Division);
	}
	return 1;
}

CCheckItem & CPosProcessor::Packet::GetCur()
{
	return Cur;
}

const CCheckItem & CPosProcessor::Packet::GetCurC() const
{
	return Cur;
}

double CPosProcessor::Packet::GetRest() const
{
	return Rest;
}

void CPosProcessor::Packet::SetRest(double rest)
{
	Rest = rest;
}

int CPosProcessor::Packet::MoveUp(uint itemIdx)
{
	if(itemIdx < getCount() && itemIdx > 0) {
		swap(itemIdx, itemIdx-1);
		return 1;
	}
	else
		return -1;
}

int CPosProcessor::Packet::MoveDown(uint itemIdx)
{
	if((itemIdx+1) < getCount()) {
		swap(itemIdx, itemIdx+1);
		return 1;
	}
	else
		return -1;
}

int CPosProcessor::Packet::Grouping(uint itemIdx)
{
	if(itemIdx > 0 && itemIdx < getCount()) {
		if(at(itemIdx).Flags & cifGrouped) {
			at(itemIdx).Flags &= ~cifGrouped;
		}
		else {
			at(itemIdx).Flags |= cifGrouped;
			//
			// ��� ������� �������� ��������������� ����� �� ����� �������, ��� � �
			// ����������� �������� (�������������� ������� ����� ��������� �� ������� ��������
			// ������ - ���������� ������ ����� ������������ ������� � ����� ������������).
			//
			int8   prev_queue = at(itemIdx-1).Queue;
			if(at(itemIdx).Queue != prev_queue)
				SetQueue(itemIdx, prev_queue);
		}
		return 1;
	}
	else
		return -1;
}

int CPosProcessor::Packet::SetQueue(uint itemIdx, int8 queue)
{
	int    ok = 1;
	const  uint c = getCount();
	uint   i = itemIdx;
	if(i < c) {
		at(i++).Queue = queue;
		//
		// ��� ���� ���������, ��������������� � ������, ���������� ��������� ��� �� �����
		// ����� �������.
		//
		while(i < c && at(i).Flags & (cifGrouped|cifModifier)) {
			at(i++).Queue = queue;
		}
		for(i = itemIdx; at(i).Flags & cifGrouped && i > 0;)
			at(--i).Queue = queue;
	}
	else
		ok = 0;
	return ok;
}

int CPosProcessor::Packet::InitIteration()
{
	IterIdx = 0;
	//
	// ����������� ��������� ����� ����� �� �������� cifGrouped
	//
	int    grp_n = 0;
	int    is_grp = 0;
	for(uint i = 0; i < getCount(); i++) {
		CCheckItem & r_item = at(i);
		if(r_item.Flags & cifGrouped) {
			if(!is_grp) {
				grp_n++;
				is_grp = 1;
				if(i > 0)
					at(i-1).LineGrpN = grp_n;
			}
			r_item.LineGrpN = grp_n;
		}
		else
			is_grp = 0;
	}
	return 1;
}

int CPosProcessor::Packet::NextIteration(CCheckItem * pItem)
{
	int    ok = -1;
	CCheckItem * p_item;
	if(pItem && enumItems(&IterIdx, (void**)&p_item)) {
		ASSIGN_PTR(pItem, *p_item);
		ok = 1;
	}
	return ok;
}
//
//
//
CPosProcessor::PgsBlock::PgsBlock(double qtty)
{
	Qtty = (qtty != 0.0) ? qtty : 1.0;
	PriceBySerial = 0.0;
}
//
//
//
CPosProcessor::AcceptCheckProcessBlock::AcceptCheckProcessBlock()
{
	R = 1;
	SyncPrnErr = 0;
	RExt = 1;
	ExtSyncPrnErr = 0;
	IsPack = 0;
	IsExtPack = 0;
	MEMSZERO(LastChkRec);
}
//
//
//
CPosProcessor::ExtCcData::ExtCcData()
{
	Clear();
}

void CPosProcessor::ExtCcData::Clear()
{
	Flags = 0;
	SCardID_ = 0;
	DlvrDtm.SetZero();
	InitDtm.SetZero();
	MEMSZERO(Addr_);
	Memo = 0;
}
//
//
//
CPosProcessor::RetBlock::RetBlock()
{
	Clear();
}

CPosProcessor::RetBlock & CPosProcessor::RetBlock::Clear()
{
	SellCheckID = 0;
	SellCheckAmount = 0.0;
	SellCheckCredit = 0.0;
	AmL.Clear();
	return *this;
}
//
//
//
CPosProcessor::CardState::CardState()
{
	P_DisByAmtRule = 0;
	P_Eqb = 0;
	Reset();
}

CPosProcessor::CardState::~CardState()
{
	Reset();
}

void CPosProcessor::CardState::Reset()
{
	ZDELETE(P_DisByAmtRule);
	ZDELETE(P_Eqb);
	THISZERO();
}

PPID CPosProcessor::CardState::GetID() const
{
	return SCardID;
}

const char * CPosProcessor::CardState::GetCode() const
{
	return Code;
}

void CPosProcessor::CardState::SetID(PPID id, const char * pCode)
{
	SCardID = id;
	STRNSCPY(Code, pCode);
}

double CPosProcessor::CardState::GetDiscount(double ccAmount) const
{
	double ret = 0.0;
	if(P_DisByAmtRule && ccAmount > 0.0) {
		const TrnovrRngDis * p_item = P_DisByAmtRule->SearchItem(ccAmount);
		if(p_item && !(p_item->Flags & TrnovrRngDis::fBonusAbsoluteValue))
			ret = p_item->Value;
		else
			ret = Discount;
	}
	else
		ret = Discount;
	return ret;
}

const RetailPriceExtractor::ExtQuotBlock * CPosProcessor::GetCStEqb(PPID goodsID, int * pNoDiscount)
{
  	const  int cfg_dsbl_no_dis = BIN(CsObj.GetEqCfg().Flags & PPEquipConfig::fIgnoreNoDisGoodsTag);
	const  int nodis = BIN(!cfg_dsbl_no_dis && GObj.CheckFlag(goodsID, GF_NODISCOUNT) > 0);
	ASSIGN_PTR(pNoDiscount, nodis);
	return nodis ? 0 : CSt.P_Eqb;
}

const RetailPriceExtractor::ExtQuotBlock * CPosProcessor::GetCStEqbND(int nodiscount) const
{
	return nodiscount ? 0 : CSt.P_Eqb;
}

int CPosProcessor::LoadModifiers(PPID goodsID, SaModif & rModif)
{
	int    ok = -1;
	Goods2Tbl::Rec goods_rec, item_goods_rec;
	rModif.freeAll();
	if(GObj.Fetch(goodsID, &goods_rec) > 0) {
		int    r = 0;
		PPGoodsStruc gs;
		PPID   gen_goods_id = 0;
		{
			PPGoodsStruc::Ident gs_ident(goodsID, GSF_PARTITIAL|GSF_POSMODIFIER, 0, getcurdate_());
			THROW(r = GObj.LoadGoodsStruc(&gs_ident, &gs));
		}
		if(r < 0 && GObj.BelongToGen(goodsID, &gen_goods_id, 0) > 0) {
			PPGoodsStruc::Ident gs_ident(gen_goods_id, GSF_PARTITIAL|GSF_POSMODIFIER, 0, getcurdate_());
			THROW(r = GObj.LoadGoodsStruc(&gs_ident, &gs));
		}
		if(r > 0) {
			SString temp_buf;
			StringSet ss(SLBColumnDelim);
			double item_qtty = 0.0;
			PPIDArray gen_list;
			PPGoodsStrucItem gs_item;
			for(uint p = 0; gs.EnumItemsExt(&p, &gs_item, 0, 1.0, &item_qtty) > 0;) {
				if(GObj.Fetch(gs_item.GoodsID, &item_goods_rec) > 0 && !(item_goods_rec.Flags & GF_GENERIC)) {
					SaModifEntry entry;
					MEMSZERO(entry);
					entry.GoodsID = gs_item.GoodsID;
					entry.Qtty = fabs(item_qtty);
					{
						RetailGoodsInfo rgi;
						GetRgi(entry.GoodsID, 0.0, 0, rgi);
						entry.Price = rgi.Price;
					}
					THROW_SL(rModif.insert(&entry));
				}
			}
			if(rModif.getCount())
				ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int CPosProcessor::Backend_GetCCheckList(long ctblId, TSArray <CCheckViewItem> & rList)
{
	rList.clear();

	int    ok = 1;
	CCheckViewItem item;
	CCheckFilt flt;
	THROW(InitCcView());
	flt.Flags = CCheckFilt::fShowSuspended|CCheckFilt::fSuspendedOnly|CCheckFilt::fCTableStatus;
	flt.Flags |= CCheckFilt::fLostJunkAsSusp;
    if(!SessUUID.IsZero()) {
        flt.LostJunkUUID = SessUUID;
    }
	flt.TableCode = ctblId;
	flt.AgentID = P.GetAgentID(); // @v8.7.7
	flt.NodeList.Add(GetPosNodeID());
	flt.Period.upp = getcurdate_();
	flt.Period.low = plusdate(flt.Period.upp, -Scf.DaysPeriod);
	P_CcView->Init_(&flt);
	for(P_CcView->InitIteration(0); P_CcView->NextIteration(&item) > 0;) {
		THROW_SL(rList.insert(&item));
	}
	CATCHZOK
	return ok;
}

int CPosProcessor::ExportCTblList(SString & rBuf)
{
	rBuf = 0;
	int    ok = 1;
	int    use_def_ctbl = 0;
	uint   i;
	SString temp_buf;

	xmlTextWriterPtr writer = 0;
	xmlBuffer * p_xml_buf = 0;

	TSArray <CCheckViewItem> cc_list;
	LongArray ctbl_list;
	if(CTblList.getCount()) {
		for(i = 0; i < CTblList.getCount(); i++) {
			ctbl_list.add(CTblList.get(i));
		}
	}
	else {
		ctbl_list.add(PPObjCashNode::SubstCTblID);
		use_def_ctbl = 1;
	}
	THROW(Backend_GetCCheckList(0, cc_list));
	//
	THROW(p_xml_buf = xmlBufferCreate());
	THROW(writer = xmlNewTextWriterMemory(p_xml_buf, 0));
	{
		SXml::WDoc _doc(writer, cpUTF8);
		xmlTextWriterStartDTD(writer, (temp_buf = "CPosProcessorCTableList").ucptr(), 0, 0);
		XMLWriteSpecSymbEntities(writer);
		xmlTextWriterEndDTD(writer);
		{
			SXml::WNode n_list(writer, "CPosProcessorCTableList");
			for(i = 0; i < ctbl_list.getCount(); i++) {
				const long ctbl_id = ctbl_list.get(i);
				SXml::WNode n_item(writer, "CPosProcessorCTable");
				n_item.PutInner("ID", (temp_buf = 0).Cat(ctbl_id));
				PPObjCashNode::GetCafeTableName(ctbl_id, temp_buf = 0);
				n_item.PutInner("Name", temp_buf);
				n_item.PutInner("State", (temp_buf = (long)0));
				{
					int    cc_count = 0;
					int    cc_guest_count = 0;
					double cc_amount = 0.0;
					if(ctbl_id == PPObjCashNode::SubstCTblID) {
						for(uint p = 0; p < cc_list.getCount(); p++) {
							cc_count++;
							cc_guest_count += cc_list.at(p).GuestCount;
							cc_amount += MONEYTOLDBL(cc_list.at(p).Amount);
						}
					}
					else {
						for(uint p = 0; cc_list.lsearch(&ctbl_id, &p, CMPF_LONG, offsetof(CCheckViewItem, TableCode)); p++) {
							cc_count++;
							cc_guest_count += cc_list.at(p).GuestCount;
							cc_amount += MONEYTOLDBL(cc_list.at(p).Amount);
						}
					}
					n_item.PutInner("CCCount", (temp_buf = 0).Cat(cc_count));
					n_item.PutInner("CCAmount", (temp_buf = 0).Cat(cc_amount, MKSFMTD(0, 2, 0)));
					n_item.PutInner("CCGuestCount", (temp_buf = 0).Cat(cc_guest_count));
				}
			}
		}
		xmlTextWriterFlush(writer);
		rBuf.CopyFromN((char *)p_xml_buf->content, p_xml_buf->use)/*.UTF8ToChar()*/;
		rBuf.Transf(CTRANSF_INNER_TO_UTF8);
	}
	CATCHZOK
	xmlFreeTextWriter(writer);
	xmlBufferFree(p_xml_buf);
    return ok;
}

int CPosProcessor::ExportCCheckList(long ctblId, SString & rBuf)
{
	rBuf = 0;
	int    ok = 1;
	uint   i;
	SString temp_buf;

	xmlTextWriterPtr writer = 0;
	xmlBuffer * p_xml_buf = 0;

	TSArray <CCheckViewItem> cc_list;
	THROW(Backend_GetCCheckList(ctblId, cc_list));
	//
	THROW(p_xml_buf = xmlBufferCreate());
	THROW(writer = xmlNewTextWriterMemory(p_xml_buf, 0));
	{
		SXml::WDoc _doc(writer, cpUTF8);
		xmlTextWriterStartDTD(writer, (temp_buf = "CPosProcessorCCheckList").ucptr(), 0, 0);
		XMLWriteSpecSymbEntities(writer);
		xmlTextWriterEndDTD(writer);
		{
			SXml::WNode n_list(writer, "CPosProcessorCCheckList");
			for(i = 0; i < cc_list.getCount(); i++) {
				const CCheckViewItem & r_item = cc_list.at(i);
				SXml::WNode n_item(writer, "CPosProcessorCCheck");
				n_item.PutInner("ID", (temp_buf = 0).Cat(r_item.ID));
				n_item.PutInner("Code", (temp_buf = 0).Cat(r_item.Code));
				n_item.PutInner("Flags", (temp_buf = 0).Cat(r_item.Flags));
				n_item.PutInner("AgentID", (temp_buf = 0).Cat(r_item.AgentID));
				temp_buf = 0;
				if(r_item.AgentID)
					GetArticleName(r_item.AgentID, temp_buf);
				n_item.PutInner("AgentName", temp_buf);
				n_item.PutInner("SCardID", (temp_buf = 0).Cat(r_item.SCardID));
				{
					temp_buf = 0;
					SCardTbl::Rec sc_rec;
					if(r_item.SCardID && ScObj.Fetch(r_item.SCardID, &sc_rec) > 0)
						temp_buf = sc_rec.Code;
					n_item.PutInner("SCardCode", temp_buf);
				}
				n_item.PutInner("CTableID", (temp_buf = 0).Cat(r_item.TableCode));
				PPObjCashNode::GetCafeTableName(r_item.TableCode, temp_buf = 0);
				n_item.PutInner("CTableName", temp_buf);
				n_item.PutInner("GuestCount", (temp_buf = 0).Cat(r_item.GuestCount));
				n_item.PutInner("Amount", (temp_buf = 0).Cat(MONEYTOLDBL(r_item.Amount), MKSFMTD(0, 2, 0)));
				n_item.PutInner("Discount", (temp_buf = 0).Cat(MONEYTOLDBL(r_item.Discount), MKSFMTD(0, 2, 0)));
				n_item.PutInner("CreationTime", (temp_buf = 0).Cat(r_item.CreationDtm, DATF_ISO8601|DATF_CENTURY, 0));
			}
		}
		xmlTextWriterFlush(writer);
		rBuf.CopyFromN((char *)p_xml_buf->content, p_xml_buf->use);
		rBuf.Transf(CTRANSF_INNER_TO_UTF8);
	}
	CATCHZOK
	xmlFreeTextWriter(writer);
	xmlBufferFree(p_xml_buf);
    return ok;
}

int CPosProcessor::ExportCurrentState(SString & rBuf) const
{
	int    ok = 1;
	SString temp_buf;
	xmlTextWriterPtr writer = 0;
	xmlBuffer * p_xml_buf = 0;

	rBuf = 0;
	THROW(p_xml_buf = xmlBufferCreate());
	THROW(writer = xmlNewTextWriterMemory(p_xml_buf, 0));
	{
		SXml::WDoc _doc(writer, cpUTF8);
		xmlTextWriterStartDTD(writer, (temp_buf = "CPosProcessorState").ucptr(), 0, 0);
		XMLWriteSpecSymbEntities(writer);
		xmlTextWriterEndDTD(writer);
		{
			SXml::WNode n_state(writer, "CPosProcessorState");
			{
				#define PUTNODE_F(f) SXml::WNode(writer, #f, (temp_buf = 0).Cat(f))
				#define PUTNODE_TF(t, f) SXml::WNode(writer, #t, (temp_buf = 0).Cat(f))
				PUTNODE_F(AuthAgentID);
				PUTNODE_F(CheckID);
				PUTNODE_F(SuspCheckID);
				PUTNODE_TF(State, State_p);
				PUTNODE_F(Flags);
				PUTNODE_F(OperRightsFlags);
				PUTNODE_F(CnFlags);
				PUTNODE_F(CnExtFlags);
				#undef PUTNODE_TF
				#undef PUTNODE_F
				{
					SXml::WNode n_cardstate(writer, "CardState");
					{
						SXml::WNode(writer, "ID", (temp_buf = 0).Cat(CSt.GetID()));
						SXml::WNode(writer, "Code", (temp_buf = 0).Cat(CSt.GetCode()));
						#define PUTNODE_F(f) SXml::WNode(writer, #f, (temp_buf = 0).Cat(CSt.f))
						PUTNODE_F(Flags);
						PUTNODE_F(Discount);
						PUTNODE_F(SettledDiscount);
						PUTNODE_F(UhttRest);
						PUTNODE_F(RestByCrdCard);
						PUTNODE_F(UsableBonus);
						PUTNODE_F(MaxCreditByCrdCard);
						PUTNODE_F(UhttCode);
						PUTNODE_F(UhttHash);
						#undef PUTNODE_F
					}
				}
				{
					SXml::WNode n_packet(writer, "Packet");
					{
						#define PUTNODE_F(f) SXml::WNode(writer, #f, (temp_buf = 0).Cat(P.f))
						SXml::WNode(writer, "CTableID", (temp_buf = 0).Cat(P.TableCode));
						PUTNODE_F(GuestCount);
						PUTNODE_F(OrderCheckID);
						PUTNODE_F(CurPos);
						SXml::WNode(writer, "Rest", (temp_buf = 0).Cat(P.GetRest()));
						SXml::WNode(writer, "AgentID", (temp_buf = 0).Cat(P.GetAgentID()));
						#undef PUTNODE_F
						for(uint i = 0; i < P.getCount(); i++) {
							SXml::WNode n_row(writer, "CCRow");
							{
								const CCheckItem & r_item = P.at(i);
								#define PUTNODE_F(f) SXml::WNode(writer, #f, (temp_buf = 0).Cat(r_item.f))
								PUTNODE_F(GoodsID);
								PUTNODE_F(Quantity);
								PUTNODE_F(PhQtty);
								PUTNODE_F(Price);
								PUTNODE_F(Discount);
								PUTNODE_F(BeforeGiftPrice);
								PUTNODE_F(GiftID);
								PUTNODE_F(Flags);
								PUTNODE_F(Division);
								PUTNODE_F(LineGrpN);
								PUTNODE_F(Queue);
								PUTNODE_F(BarCode);
								PUTNODE_F(GoodsName);
								PUTNODE_F(Serial);
								#undef PUTNODE_F
							}
						}
						if(P.HasCur()) {
							SXml::WNode n_row(writer, "CCRowCurrent");
							{
								const CCheckItem & r_item = P.GetCurC();
								#define PUTNODE_F(f) SXml::WNode(writer, #f, (temp_buf = 0).Cat(r_item.f))
								PUTNODE_F(GoodsID);
								PUTNODE_F(Quantity);
								PUTNODE_F(PhQtty);
								PUTNODE_F(Price);
								PUTNODE_F(Discount);
								PUTNODE_F(BeforeGiftPrice);
								PUTNODE_F(GiftID);
								PUTNODE_F(Flags);
								PUTNODE_F(Division);
								PUTNODE_F(LineGrpN);
								PUTNODE_F(Queue);
								PUTNODE_F(BarCode);
								PUTNODE_F(GoodsName);
								PUTNODE_F(Serial);
								#undef PUTNODE_F
							}
						}
					}
				}
			}
		}
	}
	xmlTextWriterFlush(writer);
	rBuf.CopyFromN((char *)p_xml_buf->content, p_xml_buf->use)/*.UTF8ToChar()*/;
	rBuf.Transf(CTRANSF_INNER_TO_UTF8);
	CATCHZOK
	xmlFreeTextWriter(writer);
	xmlBufferFree(p_xml_buf);
	return ok;
}

int CPosProcessor::ExportModifList(PPID goodsID, SString & rBuf)
{
	int    ok = 1;
	SString temp_buf;
	xmlTextWriterPtr writer = 0;
	xmlBuffer * p_xml_buf = 0;

	rBuf = 0;
	THROW(p_xml_buf = xmlBufferCreate());
	THROW(writer = xmlNewTextWriterMemory(p_xml_buf, 0));
	{
		SXml::WDoc _doc(writer, cpUTF8);
		xmlTextWriterStartDTD(writer, (temp_buf = "CPosProcessorModifList").ucptr(), 0, 0);
		XMLWriteSpecSymbEntities(writer);
		xmlTextWriterEndDTD(writer);
		{
			SXml::WNode n_list(writer, "CPosProcessorModifList");
			SaModif mlist;
			n_list.PutInner("GoodsID", (temp_buf = 0).Cat(goodsID));
			if(LoadModifiers(goodsID, mlist) > 0) {
				Goods2Tbl::Rec goods_rec;
				for(uint i = 0; i < mlist.getCount(); i++) {
					const SaModifEntry & r_item = mlist.at(i);
					if(GObj.Fetch(r_item.GoodsID, &goods_rec) > 0) {
						SXml::WNode n_item(writer, "Item");
						n_item.PutInner("GoodsID", (temp_buf = 0).Cat(r_item.GoodsID));
						n_item.PutInner("GoodsName", (temp_buf = goods_rec.Name));
						n_item.PutInner("Flags", (temp_buf = 0).Cat(r_item.Flags));
						n_item.PutInner("Price", (temp_buf = 0).Cat(r_item.Price, MKSFMTD(0, 5, NMBF_NOTRAILZ)));
						n_item.PutInner("Qtty",  (temp_buf = 0).Cat(r_item.Qtty, MKSFMTD(0, 6, NMBF_NOTRAILZ)));
					}
				}
			}
		}
	}
	xmlTextWriterFlush(writer);
	rBuf.CopyFromN((char *)p_xml_buf->content, p_xml_buf->use)/*.UTF8ToChar()*/;
	rBuf.Transf(CTRANSF_INNER_TO_UTF8);
	CATCHZOK
	xmlFreeTextWriter(writer);
	xmlBufferFree(p_xml_buf);
	return ok;
}

int CPosProcessor::GetTblOrderList(LDATE lastDate, TSArray <CCheckViewItem> & rList)
{
	rList.clear();
	CCheckFilt cc_filt;
	cc_filt.Period.low = plusdate(getcurdate_(), -7);
	cc_filt.Flags |= CCheckFilt::fOrderOnly;
	//cc_filt.CashNodeID = CashNodeID;
	//cc_filt.TableCode = (P_AddParam) ? P_AddParam->TableCode : 0;
	//cc_filt.AgentID = (P_AddParam) ? P_AddParam->AgentID : 0;
	if(InitCcView()) {
		if(P_CcView->Init_(&cc_filt)) {
			CCheckViewItem item;
			for(P_CcView->InitIteration(0); P_CcView->NextIteration(&item) > 0;) {
				if(!(item.Flags & (CCHKF_SKIP|CCHKF_CLOSEDORDER))) {
					if(!checkdate(lastDate, 0) || item.OrderTime.Start.d == lastDate || item.OrderTime.Finish.d == lastDate)
						rList.insert(&item);
				}
			}
		}
	}
	return 1;
}
//
//
//
CPosProcessor::CPosProcessor(PPID cashNodeID, PPID checkID, CCheckPacket * pOuterPack, int isTouchScreen) : CashNodeID(cashNodeID)
{
	P_CcView = 0; // @v8.6.12
	P_TSesObj = 0;
	P_EgPrc = 0; // @v9.0.9
	EgaisMode = 0; // @v9.0.9
	OuterOi.Set(0, 0);
	Flags = 0;
	BonusMaxPart = 1.0;
	OperRightsFlags = 0;
	OrgOperRights = 0;
	MEMSZERO(R);
	SuspCheckID = 0;
	CheckID     = checkID;
	AuthAgentID = 0; // @v8.6.10
	P_CM     = 0;
	P_CM_EXT = 0;
	P_GTOA   = 0;
	P_ChkPack  = pOuterPack;
	SessUUID.SetZero(); // @v8.7.7
	SETFLAG(Flags, fNoEdit, (P_ChkPack || !CashNodeID));

	PPCashNode    cn_rec;
	CnObj.Search(CashNodeID, &cn_rec);
	if(cn_rec.Flags & CASHF_SYNC) {
		PPSyncCashNode cn_pack;
		if(CnObj.GetSync(CashNodeID, &cn_pack) > 0) {
			CTblList = cn_pack.CTblList;
			Scf      = cn_pack.Scf; // @v8.6.12
			// @v8.8.3 {
			cn_pack.GetPropString(SCN_RPTPRNPORT, RptPrnPort);
			RptPrnPort.Strip();
			// } @v8.8.3
			// @v9.0.9 {
			if(oneof3(cn_pack.EgaisMode, 0, 1, 2)) {
				EgaisMode = cn_pack.EgaisMode;
				if(oneof2(EgaisMode, 1, 2) && !(Flags & fNoEdit)) {
					long   egcf = PPEgaisProcessor::cfDirectFileLogging;
					if(EgaisMode == 2)
						egcf |= PPEgaisProcessor::cfDebugMode;
					P_EgPrc = new PPEgaisProcessor(egcf, 0);
				}
			}
			// } @v9.0.9
		}
	}
	CnName = cn_rec.Name;
	CnSymb = cn_rec.Symb; // @v8.8.3
	CnFlags = cn_rec.Flags & (CASHF_SELALLGOODS | CASHF_USEQUOT | CASHF_NOASKPAYMTYPE |
		CASHF_SHOWREST | CASHF_KEYBOARDWKEY | CASHF_WORKWHENLOCK | CASHF_DISABLEZEROAGENT |
		CASHF_UNIFYGDSATCHECK | CASHF_UNIFYGDSTOPRINT | CASHF_CHECKFORPRESENT | CASHF_ABOVEZEROSALE | CASHF_SYNC);
	CnExtFlags = cn_rec.ExtFlags;
	CnSpeciality = (long)cn_rec.Speciality;
	CnLocID = cn_rec.LocID;
	ExtCnLocID     = 0;
	ExtCashNodeID  = 0;
	P_DivGrpList   = 0;
	{
		SArray temp_list(sizeof(PPGenCashNode::DivGrpAssc));
		if(PPRef->GetPropArray(PPOBJ_CASHNODE, CashNodeID, CNPRP_DIVGRPASSC, &temp_list) > 0 && temp_list.getCount())
			P_DivGrpList = new SArray(temp_list);
	}
	SETFLAG(Flags, fAsSelector, (P_ChkPack && !CashNodeID && !CheckID));
	SETFLAG(Flags, fTouchScreen, isTouchScreen);
	SETFLAG(Flags, fCashNodeIsLocked, CnObj.IsLocked(CashNodeID) > 0);
	//
	PPObjLocPrinter lp_obj;
	SETFLAG(Flags, fLocPrinters, lp_obj.IsPrinter());
	{
		int    esc_chk = CsObj.CheckRights(CSESSRT_ESCCHECK);
		SETFLAG(OperRightsFlags, orfReturns,     CsObj.CheckRights(CSESSOPRT_RETCHECK, 1));
		SETFLAG(OperRightsFlags, orfEscCheck,    esc_chk);
		SETFLAG(OperRightsFlags, orfEscChkLine, /*esc_chk && @?*/CsObj.CheckRights(CSESSOPRT_ESCCLINE, 1));
		SETFLAG(OperRightsFlags, orfBanking,     CsObj.CheckRights(CSESSOPRT_BANKING, 1));
		SETFLAG(OperRightsFlags, orfZReport,     CsObj.CheckRights(CSESSRT_CLOSE));
		SETFLAG(OperRightsFlags, orfPreCheck,    CsObj.CheckRights(CSESSOPRT_PREPRT,    1));
		SETFLAG(OperRightsFlags, orfSuspCheck,   CsObj.CheckRights(CSESSOPRT_SUSPCHECK, 1));
		SETFLAG(OperRightsFlags, orfCopyCheck,   CsObj.CheckRights(CSESSOPRT_COPYCHECK, 1));
		SETFLAG(OperRightsFlags, orfCopyZReport, CsObj.CheckRights(CSESSOPRT_COPYZREPT, 1));
		SETFLAG(OperRightsFlags, orfPrintCheck,  CsObj.CheckRights(CSESSRT_ADDCHECK));
		SETFLAG(OperRightsFlags, orfRowDiscount, CsObj.CheckRights(CSESSOPRT_ROWDISCOUNT, 1));
		SETFLAG(OperRightsFlags, orfXReport,     CsObj.CheckRights(CSESSOPRT_XREP, 1));
		SETFLAG(OperRightsFlags, orfSplitCheck,      CsObj.CheckRights(CSESSOPRT_SPLITCHK, 1));
		SETFLAG(OperRightsFlags, orfMergeChecks,     CsObj.CheckRights(CSESSOPRT_MERGECHK, 1)); // @v8.5.5
		SETFLAG(OperRightsFlags, orfChgPrintedCheck, CsObj.CheckRights(CSESSOPRT_CHGPRINTEDCHK, 1));
		SETFLAG(OperRightsFlags, orfRestoreSuspWithoutAgent, CsObj.CheckRights(CSESSOPRT_RESTORESUSPWOA, 1));
		SETFLAG(OperRightsFlags, orfChgAgentInCheck, CsObj.CheckRights(CSESSOPRT_CHGCCAGENT, 1)); // @v8.2.1
		SETFLAG(OperRightsFlags, orfEscChkLineBeforeOrder, CsObj.CheckRights(CSESSOPRT_ESCCLINEBORD, 1)); // @v8.7.3
		OrgOperRights = OperRightsFlags;
	}
}

CPosProcessor::~CPosProcessor()
{
	delete P_CM;
	delete P_CM_EXT;
	delete P_GTOA;
	delete P_DivGrpList;
	delete P_TSesObj;
	delete P_CcView;
	delete P_EgPrc; // @v9.0.9
}

int CPosProcessor::InitCashMachine()
{
	return BIN((P_CM || (P_CM = PPCashMachine::CreateInstance(CashNodeID)) != 0) &&
		(!ExtCashNodeID || P_CM_EXT || (P_CM_EXT = PPCashMachine::CreateInstance(ExtCashNodeID)) != 0));
}

int CPosProcessor::InitCcView()
{
	return BIN(SETIFZ(P_CcView, new PPViewCCheck(CC)));
}

//virtual
int CPosProcessor::MessageError(int errCode, const char * pAddedMsg, long outputMode)
{
	SString err_msg;
	if(errCode < 0)
		errCode = PPErrCode;
	else if(errCode > 0)
		PPSetError(errCode, pAddedMsg);
	PPGetMessage(mfError, errCode, pAddedMsg, 1, err_msg);
	PPLogMessage(PPFILNAM_ERRMSG_LOG, err_msg, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_DBINFO);
	return 0;
}

//virtual
int CPosProcessor::ConfirmMessage(int msgId, const char * pAddedMsg, int defaultResponse)
{
	return defaultResponse;
}

//virtual
int CPosProcessor::CDispCommand(int cmd, int iVal, double rv1, double rv2)
{
	return -1;
}

//virtual
int CPosProcessor::Implement_AcceptCheckOnEquipment(const CcAmountList * pPl, AcceptCheckProcessBlock & rB)
{
	return 1;
}

// virtual
int CPosProcessor::NotifyGift(PPID giftID, SaGiftArray::Gift * pGift)
{
	return -1;
}

int CPosProcessor::GetNewCheckCode(PPID cashNodeID, long * pCode)
{
	int   ok = 1;
	long  code = 1;
	CCheckTbl::Rec chk_rec;
	if(CC.GetLastCheckByCode(cashNodeID, &chk_rec) > 0)
		code = chk_rec.Code + 1;
	ASSIGN_PTR(pCode, code);
	return ok;
}

int CPosProcessor::GetCheckInfo(CCheckPacket * pPack)
{
	int    ok = 1;
	CCheckTbl::Rec rec;
	pPack->Rec.CashID = CashNodeID;
	if(CheckID && CC.Search(CheckID, &pPack->Rec) > 0) {
		CCheckExtTbl::Rec ext_rec;
		if(CC.GetExt(CheckID, &ext_rec) > 0)
			pPack->Ext = ext_rec;
	}
	else {
		if(SuspCheckID && CC.Search(SuspCheckID, &rec) > 0) {
			pPack->Rec.Code = rec.Code;
			pPack->Rec.ID   = SuspCheckID;
		}
		else
			GetNewCheckCode(CashNodeID, &pPack->Rec.Code);
		THROW(Helper_InitCcPacket(pPack, 0, 0, iccpSetCurTime | iccpDontFillLines));
	}
	if(pPack->Rec.SCardID) {
		SCardOpTbl::Rec scop_rec;
		if(pPack->Ext.LinkCheckID) {
			if(ScObj.P_Tbl->SearchOpByCheck(pPack->Ext.LinkCheckID, &scop_rec) > 0) {
				if(scop_rec.Amount > 0.0)
					pPack->_OrdPrepay = scop_rec.Amount;
			}
		}
		else if(pPack->Rec.Flags & CCHKF_ORDER) {
			if(ScObj.P_Tbl->SearchOpByCheck(pPack->Rec.ID, &scop_rec) > 0) {
				if(scop_rec.Amount > 0.0)
					pPack->_OrdPrepay = scop_rec.Amount;
			}
		}
	}
	CATCHZOK
	return ok;
}

int CPosProcessor::SetupState(int st)
{
	int    ok = -1;
	if(State_p != st) {
		State_p = st;
		ok = 1;
	}
	return ok;
}

int CPosProcessor::GetState() const
{
	return State_p;
}

int CPosProcessor::IsState(int s) const
{
	return BIN(State_p == s);
}

// virtual
void CPosProcessor::SetPrintedFlag(int set)
{
	SETFLAG(Flags, fPrinted, set);
}

int CPosProcessor::SetupExt(const CCheckPacket * pPack)
{
	SetupAgent((pPack ? pPack->Ext.SalerID : 0), 0);
	P.TableCode = pPack ? pPack->Ext.TableNo : 0;
	P.GuestCount = pPack ? pPack->Ext.GuestCount : 0;
	P.OrderCheckID = pPack ? pPack->Ext.LinkCheckID : 0;
	P.Eccd.Clear();
	if(pPack) {
		P.Eccd.Memo = pPack->Ext.Memo;
		SETFLAG(P.Eccd.Flags, P.Eccd.fDelivery,   pPack->Rec.Flags & CCHKF_DELIVERY);
		SETFLAG(P.Eccd.Flags, P.Eccd.fFixedPrice, pPack->Rec.Flags & CCHKF_FIXEDPRICE); // @v8.7.7
		if(pPack->Ext.AddrID) {
			LocationTbl::Rec loc_rec;
			if(PsnObj.LocObj.Search(pPack->Ext.AddrID, &loc_rec) > 0) {
				P.Eccd.Addr_ = loc_rec;
				//
				PPIDArray sc_list;
				if(ScObj.P_Tbl->GetListByLoc(loc_rec.ID, 0, &sc_list) > 0) {
					if(sc_list.getCount()) {
						P.Eccd.SCardID_ = sc_list.get(0);
						// @todo ���-�� ���� ������� � ������ ��������������� (sc_list.getCount() > 1)
					}
				}
			}
		}
		P.Eccd.InitDtm = pPack->Ext.CreationDtm;
		if(P.Eccd.Flags & P.Eccd.fDelivery)
			P.Eccd.DlvrDtm = pPack->Ext.StartOrdDtm;
		P.AmL = pPack->AL_Const(); // @v8.0.0
	}
	return 1;
}

int CPosProcessor::SetupSessUuid(S_GUID & rUuid)
{
	SessUUID = rUuid;
	return 1;
}

int CPosProcessor::SetupAgent(PPID agentID, int asAuthAgent)
{
	int    ok = 1;
	ArticleTbl::Rec agent_ar_rec;
	THROW(!agentID || ArObj.Search(agentID, &agent_ar_rec) > 0);
	Flags &= ~fUsedRighsByAgent;
	if(agentID) {
		ObjTagItem tag;
		PPID   psn_id = ObjectToPerson(agentID, 0);
		if(psn_id && PPRef->Ot.GetTag(PPOBJ_PERSON, psn_id, PPTAG_PERSON_POSRIGHTS, &tag) > 0) {
			SString rt_buf;
			long   rt = 0, ort = 0, f = 0;
			tag.GetStr(rt_buf);
			PPObjCSession::StringToRights(rt_buf, &rt, &ort);
			f = OrgOperRights;
			//
			// ����� ���������� ��� //x ���������������� �� ������� ����,
			// ��� � ������� �������������� ���� ���� ���������� � �������� ���������
			// ������ �������� �� ������������, � ������ �� ����� ���� ���������.
			//
			SETFLAG(f, orfReturns,     ort & CSESSOPRT_RETCHECK);
			SETFLAG(f, orfEscCheck,    rt & CSESSRT_ESCCHECK);
			SETFLAG(f, orfEscChkLine,  ort & CSESSOPRT_ESCCLINE);
			SETFLAG(f, orfBanking,     ort & CSESSOPRT_BANKING);
			SETFLAG(f, orfZReport,     rt & CSESSRT_CLOSE);
			//x SETFLAG(f, orfPreCheck,    ort & CSESSOPRT_PREPRT);
			//x SETFLAG(f, orfSuspCheck,   ort & CSESSOPRT_SUSPCHECK);
			SETFLAG(f, orfCopyCheck,   ort & CSESSOPRT_COPYCHECK);
			SETFLAG(f, orfCopyZReport, ort & CSESSOPRT_COPYZREPT);
			SETFLAG(f, orfPrintCheck,  rt & CSESSRT_ADDCHECK);
			SETFLAG(f, orfRowDiscount, ort & CSESSOPRT_ROWDISCOUNT);
			SETFLAG(f, orfXReport,     ort & CSESSOPRT_XREP);
			//x SETFLAG(f, orfCTblOrd,         ort & CSESSOPRT_CTBLORD);
			SETFLAG(f, orfSplitCheck,      ort & CSESSOPRT_SPLITCHK);
			SETFLAG(f, orfMergeChecks,     ort & CSESSOPRT_MERGECHK); // @v8.5.5
			SETFLAG(f, orfChgPrintedCheck, ort & CSESSOPRT_CHGPRINTEDCHK);
			SETFLAG(f, orfChgAgentInCheck, ort & CSESSOPRT_CHGCCAGENT); // @v8.2.1
			SETFLAG(f, orfEscChkLineBeforeOrder, ort & CSESSOPRT_ESCCLINEBORD); // @v8.7.3
			OperRightsFlags = f;
			Flags |= fUsedRighsByAgent;
			{
				SString added_msg_str;
				GetPersonName(psn_id, added_msg_str);
				DS.GetTLA().AddedMsgStrNoRights = added_msg_str;
			}
		}
		else {
			OperRightsFlags = OrgOperRights;
			DS.GetTLA().AddedMsgStrNoRights = 0;
		}
	}
	else {
		OperRightsFlags = OrgOperRights;
		DS.GetTLA().AddedMsgStrNoRights = 0;
	}
	P.AgentID__ = agentID;
	// @v8.6.10 {
	if(asAuthAgent)
		AuthAgentID = agentID;
	// } @v8.6.10
	// @v8.2.0 {
	if(SuspCheckID) {
		SETIFZ(P.OrgAgentID, agentID);
		// @v8.2.1 {
		if(OperRightsFlags & orfChgAgentInCheck && agentID && P.OrgAgentID != agentID) {
			SString added_msg_buf;
			GetArticleName(P.OrgAgentID, added_msg_buf);
			if(ConfirmMessage(PPCFM_CHANGECCAGENT, added_msg_buf, 1)) {
				{
					CCheckTbl::Rec org_cc_rec;
					CCheckPacket pack;
					InitCashMachine();
					pack.Rec.SessID = P_CM ? P_CM->GetCurSessID() : 0;
					pack.Rec.CashID = CashNodeID;
					pack.Rec.Flags |= (CCHKF_SYNC | CCHKF_NOTUSED);
					Helper_InitCcPacket(&pack, 0, 0, 0);
					if(SuspCheckID && CC.Search(SuspCheckID, &org_cc_rec) > 0) {
						pack.Rec.Code = org_cc_rec.Code;
						pack.Rec.ID   = SuspCheckID;
					}
					CC.WriteCCheckLogFile(&pack, 0, CCheckCore::logAgentChanged, 0);
				}
				P.OrgAgentID = agentID;
			}
		}
		// } @v8.2.1
	}
	else
		P.OrgAgentID = agentID;
	// } @v8.2.0
	CATCHZOK
	return ok;
}

double CPosProcessor::CalcCurrentRest(PPID goodsID, int checkInputBuffer)
{
	double rest = 0.0;
	if(CC.CalcGoodsRest(goodsID, LConfig.OperDate, GetCnLocID(goodsID), &rest)) {
		for(uint pos = 0; P.lsearch(&goodsID, &pos, CMPF_LONG) > 0; pos++)
			rest -= P.at(pos).Quantity;
		if(checkInputBuffer && P.HasCur()) {
			const CCheckItem & r_buf_item = P.GetCurC();
			if(r_buf_item.GoodsID == goodsID)
				rest -= r_buf_item.Quantity;
		}
	}
	return rest;
}

//virtual
void CPosProcessor::SetupRowData(int calcRest)
{
	const CCheckItem & r_item = P.GetCur();
	SetupState(oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF) ? sLISTSEL_BUF : (P.getCount() ? sLIST_BUF : sEMPTYLIST_BUF));
	if(calcRest)
		P.SetRest(CalcCurrentRest(r_item.GoodsID, 0));
	SetupInfo(0);
}

int FASTCALL CPosProcessor::BelongToExtCashNode(PPID goodsID) const
{
	PPID   assoc_id = 0;
	/* @v8.2.1 ���� ��������� � CCheckPane::CCheckPane() ���� ����, ����� ������� BelongToExtCashNode ��������� ������ const
	if(ExtCashNodeID && !P_GTOA) {
		P_GTOA = DS.CheckExtFlag(ECF_CHKPAN_USEGDSLOCASSOC) ?
			new GoodsToObjAssoc(PPASS_GOODS2LOC, PPOBJ_LOCATION) : new GoodsToObjAssoc(PPASS_GOODS2CASHNODE, PPOBJ_CASHNODE);
		if(P_GTOA)
			P_GTOA->Load();
	}
	*/
	return BIN(P_GTOA && P_GTOA->Get(goodsID, &assoc_id) > 0 && assoc_id &&
		assoc_id == (DS.CheckExtFlag(ECF_CHKPAN_USEGDSLOCASSOC) ? ExtCnLocID : ExtCashNodeID));
}

PPID FASTCALL CPosProcessor::GetChargeGoodsID(PPID scardID)
{
	PPID   id = ScObj.GetChargeGoodsID(scardID);
	return NZOR(id, UNDEF_CHARGEGOODSID);
}

PPID CPosProcessor::GetPosNodeID() const
{
	return CashNodeID;
}

long CPosProcessor::GetTableCode() const
{
	return P.TableCode;
}

int CPosProcessor::GetGuestCount() const
{
	return P.GuestCount;
}

PPID CPosProcessor::GetCnLocID(PPID goodsID) const
{
	return (ExtCashNodeID && ExtCnLocID && BelongToExtCashNode(goodsID)) ? ExtCnLocID : CnLocID;
}

PPID CPosProcessor::GetAuthAgentID() const
{
	if(!AuthAgentID)
		PPSetError(PPERR_CPOS_UNDEFAUTHAGENT);
	return AuthAgentID;
}

int CPosProcessor::InitIteration()
{
	return P.InitIteration();
}

int CPosProcessor::NextIteration(CCheckItem * pItem)
{
	return P.NextIteration(pItem);
}

//virtual
void CPosProcessor::SetupInfo(const char * pErrMsg)
{
}

//virtual
int CPosProcessor::OnUpdateList(int goBottom)
{
	return -1;
}

int CPosProcessor::SetupCTable(int tableNo, int guestCount)
{
	if(tableNo >= 0 && (P.TableCode != tableNo || (guestCount && P.GuestCount != guestCount))) {
		P.TableCode  = tableNo;
		P.GuestCount = guestCount;
		SetupInfo(0);
		return 1;
	}
	else
		return -1;
}

/*
// @test
int CPosProcessor::SetupItem(PPID goodsID, double qtty, double price)
{
	CCheckItem & r_item = P.GetCur();
	r_item.GoodsID = goodsID;
	r_item.Quantity = fabs(qtty);
	r_item.Price    = 10;
	r_item.Discount = 0.0;
	P.insert(&r_item);
	P.CurPos = P.getCount();
	return 1;
}
*/

int CPosProcessor::OpenSession(LDATE * pDt, int ifClosed)
{
	int r = 0, r_ext = 0, open = 0, ok = 0;

	THROW(InitCashMachine());
	if(ifClosed) {
		PPCashNode    cn_rec;
		CnObj.Search(CashNodeID, &cn_rec);
		if(cn_rec.Flags & CASHF_DAYCLOSED || cn_rec.CurDate == 0 || ((cn_rec.Flags & CASHF_CHKPAN) && !cn_rec.CurSessID))
			open = 1;
	}
	else
		open = 1;
	if(open) {
		if(P_CM)
			r = P_CM->SyncOpenSession(pDt);
		if(P_CM_EXT)
			if(P_CM_EXT->GetNodeData().CashType == PPCMT_PAPYRUS) {
				if(r > 0) {
					P_CM_EXT->SetParentNode(CashNodeID);
					P_CM_EXT->AsyncOpenSession(0, 0);
				}
			}
			else
				r_ext = P_CM_EXT->SyncOpenSession(pDt);
		if(r > 0 || r_ext > 0) {
			Flags &= ~fOnlyReports;
			ok = 1;
		}
	}
	else
		ok = 1;
	CATCHZOK
	return ok;
}
//
//
//
int CPosProcessor::CalcRestByCrdCard_(int checkCurItem)
{
	int    ok = 1;
	SCardTbl::Rec sc_rec;
	PPSCardSeries scs_rec;
	PPObjSCardSeries scs_obj;
	CSt.MaxCreditByCrdCard = 0.0;
	CSt.RestByCrdCard = 0.0;
	CSt.UsableBonus = 0.0;
	CSt.AdditionalPayment = 0.0;
	Flags &= ~(fSCardCredit|fSCardBonus|fSCardBonusReal);
	if(ScObj.Search(CSt.GetID(), &sc_rec) > 0 && scs_obj.Fetch(sc_rec.SeriesID, &scs_rec) > 0 && (scs_rec.Flags & (SCRDSF_CREDIT|SCRDSF_BONUS))) {
		const int scst = scs_rec.GetType();
		if(F(fRetCheck)) {
			if(Flags & fRetByCredit) {
				double total = 0.0, discount = 0.0;
				CalcTotal(&total, &discount);
				double credit_part = fdivnz(Rb.SellCheckCredit, Rb.SellCheckAmount);
				double ret_by_credit = total * credit_part;
				if(ret_by_credit < 0.0 && ret_by_credit > total) {
					CSt.AdditionalPayment = R2(total - ret_by_credit);
				}
			}
		}
		else {
			int    skip_crd_processing = 0;
			double init_rest = R2((scs_rec.Flags & SCRDSF_UHTTSYNC) ? CSt.UhttRest : sc_rec.Rest);
			// @v8.0.2 {
			if(init_rest < 0.0 && scst == scstBonus) // @v8.2.0 (&& scst == scstBonus)
				init_rest = 0.0;
			// } @v8.0.2
			double rest = 0.0;
			Flags |= fSCardCredit;
			CSt.RestByCrdCard = init_rest;
			if(scst == scstBonus) {
				PPSCardConfig sc_cfg;
				ScObj.FetchConfig(&sc_cfg);
				//
				// @v8.0.6 {
				// ���������� ��������� ������ ������������� ������� � ���������� �������.
				// ���� fSCardBonusReal ��������� ��������� �� �������� ����� ������, �� �� ���������
				// ���������, ���� �� fSCardBonus ��������� ��������� (������� �� �������).
				if(!(sc_cfg.Flags & sc_cfg.fDontUseBonusCards))
					Flags |= fSCardBonusReal;
				// } @v8.0.6
				if(!(sc_cfg.Flags & sc_cfg.fDontUseBonusCards) && init_rest > 0.0)
					Flags |= fSCardBonus;
				else {
					Flags &= ~fSCardCredit;
					CSt.RestByCrdCard = 0.0;
					skip_crd_processing = 1;
				}
			}
			else if(scst == scstCredit) {
				CSt.MaxCreditByCrdCard = sc_rec.MaxCredit;
			}
			if(!skip_crd_processing) {
				double non_crd_amt = 0.0;
				double add_paym = 0.0;
				double cc = -CalcCreditCharge(0, 0, checkCurItem ? &P.GetCur() : 0, &non_crd_amt, 0);
				if(Flags & fSCardBonus && BonusMaxPart < 1.0 && cc > 0.0) {
					double total = 0.0, discount = 0.0;
					CalcTotal(&total, &discount);
					double cc_ = R2(MIN(cc, total * BonusMaxPart));
					cc = MIN(cc_, CSt.RestByCrdCard);
					add_paym = total-cc;
				}
				// @v8.2.0 {
				else if(scst == scstCredit && CSt.MaxCreditByCrdCard > 0.0 && cc > (CSt.RestByCrdCard + CSt.MaxCreditByCrdCard)) {
					//
					// ���� �� ����� ���������� ������������� ��������� ����� � �� ��������, �� ��������� ������������
					// ��� ����� ��� ���������.
 					//
					Flags &= ~fSCardCredit;
					CSt.RestByCrdCard = 0.0;
					skip_crd_processing = 1;
					MessageError(PPERR_CHKPAN_SCOUTOFCRDLIMIT, sc_rec.Code, /*eomMsgWindow*/eomPopup|eomBeep); // @v8.2.2
				}
				// } @v8.2.0
				else
					add_paym = cc - (CSt.RestByCrdCard + CSt.MaxCreditByCrdCard);
				if(!skip_crd_processing) { // @v8.2.0
					if(add_paym <= 0.0)
						add_paym = non_crd_amt;
					else
						add_paym += non_crd_amt;
					if(add_paym > 0.0) {
						if(R2(CSt.AdditionalPayment) == 0.0 && !(Flags & fSCardBonus)) { // ��� �������� ���� ������ ������� �� ��������� //
							if(scs_rec.Flags & SCRDSF_DISABLEADDPAYM)
								ok = MessageError(PPERR_UNABLEADDPAYMONCRDCARD, 0, /*eomMsgWindow*/eomPopup|eomBeep);
							else {
								SString  buf;
								buf.Cat(add_paym, SFMT_MONEY);
								if(!ConfirmMessage(PPCFM_MAXCRD_OVERDRAFT, buf, 1)) {
									//
									// @? �������� ������� ��� ����� �������������
									//
									CSt.AdditionalPayment = R2(add_paym);
									//
									ok = 0;
								}
							}
						}
						if(ok) {
							CSt.AdditionalPayment = R2(add_paym);
							if((CSt.RestByCrdCard - cc) < -CSt.MaxCreditByCrdCard)
								CSt.RestByCrdCard = -CSt.MaxCreditByCrdCard;
							else
								CSt.RestByCrdCard -= cc;
						}
					}
					else {
						CSt.RestByCrdCard -= cc;
						CSt.AdditionalPayment = (add_paym < 0.0) ? 0.0 : R2(add_paym);
					}
					if(Flags & fSCardBonus)
						CSt.UsableBonus = MIN(MAX(cc, 0.0), init_rest);
					CSt.RestByCrdCard = R2(CSt.RestByCrdCard);
					CSt.UsableBonus = R2(CSt.UsableBonus);
				}
			}
		}
		SetupInfo(0);
	}
	return ok;
}

void CPosProcessor::CalcTotal(double * pTotal, double * pDiscount) const
{
	double total = 0.0, discount = 0.0;
	CCheckItem * p_item;
	for(uint i = 0; P.enumItems(&i, (void**)&p_item);) {
		if(p_item->Flags & cifGift) {
			total = R2(total - p_item->Quantity * p_item->Discount);
			discount = R2(discount + p_item->Quantity * p_item->Discount);
		}
		else {
			total    = R2(total + p_item->GetAmount()); // @R2
			discount = R2(discount + p_item->Quantity * p_item->Discount); // @R2
		}
	}
	ASSIGN_PTR(pTotal, R2(total));
	ASSIGN_PTR(pDiscount, discount);
}

double CPosProcessor::GetUsableBonus() const
{
	return (Flags & fSCardBonus) ? CSt.UsableBonus : 0.0;
}

double CPosProcessor::RoundDis(double d) const
{
	return Round(d, R.DisRoundPrec, R.DisRoundDir);
}

void CPosProcessor::Helper_SetupDiscount(double roundingDiscount, int distributeGiftDiscount)
{
	//
	// ���� ����� �� ����� ���������, �� �� ���� ���������������� ����� ������.
	//
	const  int is_rounding = BIN(roundingDiscount != 0.0);
	uint   last_index = 0;
	const  int cfg_dsbl_no_dis = BIN(CsObj.GetEqCfg().Flags & PPEquipConfig::fIgnoreNoDisGoodsTag);
	double amount  = 0.0; // ������� ����� ��� ������� ����� ������
	RAssocArray gift_dis_list;
	CCheckItem * p_item;
	LongArray wodis_pos_list;
	{
		double min_qtty  = SMathConst::Max;
		double max_price = 0.0;
		const  long rpe_flags = ((CSt.Flags & CardState::fUseMinQuotVal) ? RTLPF_USEMINEXTQVAL : 0) | RTLPF_USEQUOTWTIME;
		for(uint i = 0; P.enumItems(&i, (void**)&p_item);) {
			double qtty = fabs(p_item->Quantity);
			double gift_item_dis = 0.0; // ���������� �������� ������ �� ������
			const  PPID goods_id = p_item->GoodsID;
			if(p_item->Flags & cifGiftDiscount && distributeGiftDiscount) {
				gift_item_dis = qtty * p_item->Discount;
				if(gift_item_dis != 0.0) {
					gift_dis_list.Add((long)(i-1), gift_item_dis);
					if(!is_rounding) {
						//
						// ����� ���������� ������ ���������� ������� �� ���� ��� ������� ����� ������
						//
						amount = R2(amount - gift_item_dis);
					}
				}
				assert(p_item->Price == 0.0);
				p_item->Discount = 0.0;
			}
			int    no_discount = 0;
			int    no_calcprice = 0;
			if(p_item->Flags & (cifGift|cifQuotedByGift|cifPartOfComplex)) {
				no_calcprice = 1;
				no_discount = 1;
			}
			if(is_rounding || (p_item->Flags & cifFixedPrice) || (P.Eccd.Flags & P.Eccd.fFixedPrice)) // @v8.7.7 (p_item->Flags & cifFixedPrice)
				no_calcprice = 1;
			if(!no_calcprice) {
				double price_by_serial = 0.0;
				if(p_item->Serial[0]) {
					PPIDArray  lot_list;
					PPObjBill * p_bobj = BillObj;
					if(p_bobj->SearchLotsBySerial(p_item->Serial, &lot_list) > 0) {
						ReceiptCore & r_rcpt = p_bobj->trfr->Rcpt;
						LDATE  last_date = ZERODATE;
						const  PPID  loc_id = GetCnLocID(goods_id);
						for(uint j = 0; j < lot_list.getCount(); j++) {
							ReceiptTbl::Rec lot_rec;
							if(r_rcpt.Search(lot_list.get(j), &lot_rec) > 0 && lot_rec.GoodsID == goods_id && lot_rec.LocID == loc_id) {
								if(last_date < lot_rec.Dt) {
									price_by_serial = lot_rec.Price;
									last_date = lot_rec.Dt;
								}
							}
						}
					}
				}
				RetailGoodsInfo rgi;
				long   ext_rgi_flags = 0;
				if(price_by_serial > 0.0) {
					ext_rgi_flags |= PPObjGoods::rgifUseOuterPrice;
					rgi.OuterPrice = price_by_serial;
				}
				const int _gpr = GetRgi(goods_id, qtty, ext_rgi_flags, rgi);
				if(rgi.Flags & RetailGoodsInfo::fNoDiscount)
					no_discount = 1;
				//
				if(_gpr > 0) {
					p_item->Price = rgi.Price;
				}
				if(_gpr > 0 && (rgi.QuotKindUsedForExtPrice && rgi.ExtPrice >= 0.0)) { // @v8.7.11 (rgi.ExtPrice)-->(rgi.QuotKindUsedForExtPrice && rgi.ExtPrice >= 0.0)
					if(rgi.Flags & rgi.fDisabledQuot) { // @v8.7.12 �������������� ��������: ExtPrice ���������� �� ���������� ������������� ���������
						p_item->Price = rgi.ExtPrice;
						p_item->Discount = 0.0;
					}
					else
						p_item->Discount = (p_item->Price - rgi.ExtPrice);
					no_discount = 1;
				}
				else {
					const RetailPriceExtractor::ExtQuotBlock * p_eqb = GetCStEqbND(no_discount);
					if(p_eqb && p_eqb->QkList.getCount() && !(CSt.Flags & CardState::fUseDscntIfNQuot))
						no_discount = 1;
					p_item->Discount = 0.0;
				}
			}
			if(no_discount) {
				wodis_pos_list.addUnique(i);
			}
			else {
				double p = R2(is_rounding ? p_item->NetPrice() : p_item->Price);
				amount = R2(amount + p * qtty);
				if(qtty > 0.0 && (qtty < min_qtty || (qtty == min_qtty && p > max_price))) {
					last_index = i;
					min_qtty = qtty;
					max_price = p;
				}
			}
		}
	}
	amount = R2(amount);
	{
		//
		// ����������� �������, ������������ ��, ��� �������� ������
		// �� ������� (last_index-1) ������ ������������ � ������������ ������,
		// � �� �������� ��. ���� ����� ���� ������� ������ ����� is_rounding,
		// ������, ���� ���������� ������������� ���������� ������ �� �������
		// (last_index-1), �� finish_addendum ���������� ������ 1.
		//
		int    finish_addendum = is_rounding;
		//
		double discount = 0.0;
		if(is_rounding)
			discount = roundingDiscount;
		else {
			double _dis = CSt.GetDiscount(amount);
			discount = _dis * fdiv100r(amount);
			CSt.SettledDiscount = _dis;
		}
		double part_dis = 0.0, part_amount = 0.0;
		if(!is_rounding) {
			if(discount != 0.0) {
				double temp_dis = this->RoundDis(discount);
				if(temp_dis < amount)
					discount = temp_dis;
			}
			for(uint i = 0; P.enumItems(&i, (void**)&p_item);)
				if(i != last_index && !wodis_pos_list.lsearch(i)) {
					const double qtty = fabs(p_item->Quantity);
					const double p    = R2(is_rounding ? p_item->NetPrice() : p_item->Price); // @R2
					double d = this->RoundDis(fdivnz(p * (discount - part_dis), (amount - part_amount)));
					SETMIN(d, p); // ����������� ��, ��� ������ �� �������� ����
					p_item->Discount = d;
					part_dis    += (d * qtty);
					part_amount += (p * qtty);
				}
			//
			// ������������ ����������� ���������� ������ �� ��� ��������, �� ���������
			// ������� ��� ���� �������������.
			// {
			for(uint j = 0; j < gift_dis_list.getCount(); j++) {
				uint   i;
				const  uint main_pos = gift_dis_list.at(j).Key;
				const  double dis = gift_dis_list.at(j).Val;
				double gift_part_dis = 0.0;
				double gift_part_amt = 0.0;
				double gift_amt = 0.0;
				LongArray pos_list;
				LongArray main_pos_list;
				P.GiftAssoc.GetListByKey(main_pos, pos_list);
				uint   plc = pos_list.getCount();
				for(i = 0; i < plc; i++) {
					const uint pos = pos_list.get(i);
					assert(pos < P.getCount() && pos != main_pos);
					p_item = &P.at(pos);
					const double item_amt = p_item->NetPrice() * fabs(p_item->Quantity);
					gift_amt = R2(gift_amt + R2(item_amt));
					if(p_item->Flags & cifMainGiftItem /*&& dis < item_amt*/)
						main_pos_list.addUnique((int)pos);
				}
				if(main_pos_list.getCount()) {
					//
					// ����������� ������: ���������� ������ �������������� ������ �� �������� ��������� ���������� ���������
					//
					pos_list = main_pos_list; // ��������� pos_list �� main_pos_list
					gift_amt = 0.0;
					//
					plc = pos_list.getCount();
					for(i = 0; i < plc; i++) {
						const uint pos = pos_list.get(i);
						assert(pos < P.getCount() && pos != main_pos);
						p_item = &P.at(pos);
						const double item_amt = p_item->NetPrice() * fabs(p_item->Quantity);
						gift_amt = R2(gift_amt + R2(item_amt));
					}
				}
				for(i = 0; i < plc; i++) {
					const uint _pos = pos_list.get(i);
					if(last_index && _pos == (last_index-1))
						finish_addendum = 1;
					p_item = &P.at(_pos);
					const double qtty = fabs(p_item->Quantity);
					const double p    = R2(p_item->NetPrice()); // @R2
					double d = this->RoundDis((i == (plc-1)) ? ((dis - gift_part_dis) / qtty) : (fdivnz(p * (dis - gift_part_dis), (gift_amt - gift_part_amt))));
					SETMIN(d, p); // ����������� ��, ��� ������ �� �������� ����
					p_item->Discount = this->RoundDis(p_item->Discount + d);
					gift_part_dis += (d * qtty);
					gift_part_amt += (p * qtty);
				}
			}
		}
		if(last_index) {
			p_item = &P.at(last_index-1);
			const double qtty = fabs(p_item->Quantity);
			const double org_p = R2(p_item->Price);
			const double p    = R2(is_rounding ? p_item->NetPrice() : p_item->Price); // @R2
			double d = this->RoundDis(((discount - part_dis) / qtty));
			if(finish_addendum) {
				if((p_item->Discount + d) > org_p)  // @v9.0.2 p-->org_p
					d = org_p;                      // @v9.0.2 p-->org_p
				p_item->Discount = this->RoundDis(p_item->Discount + d);
			}
			else {
				SETMIN(d, org_p); // ����������� ��, ��� ������ �� �������� ���� // @v9.0.2 p-->org_p
				p_item->Discount = d;
			}
			part_dis    += (d * qtty); // @debug
			part_amount += (p * qtty); // @debug
		}
	}
}

void CPosProcessor::SetupDiscount(int distributeGiftDiscount /*=0*/)
{
	Helper_SetupDiscount(0.0, distributeGiftDiscount);
	double amt, dis;
	CalcTotal(&amt, &dis);
	double new_amt = (R.AmtRoundPrec != 0.0) ? Round(amt, R.AmtRoundPrec, R.AmtRoundDir) : R2(amt);
	double diff = R2(amt - new_amt);
	if(diff != 0.0) {
		Helper_SetupDiscount(diff, 0);
		/*
		CalcTotal(&amt, &dis);
		diff = R2(amt - new_amt);
		if(diff != 0.0) {
			Helper_SetupDiscount(diff, 0);
			CalcTotal(&amt, &dis);
		}
		*/
		/*
		if(amt != new_amt) {
			SString fmt_buf, msg_buf, chk_code;
			CCheckPacket cc_pack;
			GetCheckInfo(&cc_pack);
			CCheckCore::MakeCodeString(&cc_pack.Rec, chk_code);
			PPLoadText(PPTXT_UNABLEDISTRIBCCHECKDIS, fmt_buf);
			msg_buf.Printf(fmt_buf, chk_code.cptr());
			PPLogMessage(PPFILNAM_ERR_LOG, msg_buf, LOGMSGF_TIME|LOGMSGF_USER);
		}
		*/
	}
}

void CPosProcessor::ResetSCard()
{
	CSt.Reset();
	Flags &= ~(fSCardCredit|fSCardBonus|fSCardBonusReal|fPctDis); // @scard // @v8.2.2 (|fSCardBonus|fSCardBonusReal)
	SetupDiscount();
}

int CPosProcessor::GetRgi(PPID goodsID, double qtty, long extRgiFlags, RetailGoodsInfo & rRgi)
{
	long   rgi_flags = PPObjGoods::rgifUseQuotWTimePeriod;
	if(CnFlags & CASHF_USEQUOT)
		rgi_flags |= PPObjGoods::rgifUseBaseQuotAsPrice;
	int    nodis = 0;
	LDATETIME actual_dtm = P.Eccd.InitDtm;
	int    r = GObj.GetRetailGoodsInfo(goodsID, GetCnLocID(goodsID), GetCStEqb(goodsID, &nodis),
		P.GetAgentID(), actual_dtm, fabs(qtty), &rRgi, rgi_flags|extRgiFlags);
	SETFLAG(rRgi.Flags, RetailGoodsInfo::fNoDiscount, nodis);
	return r;
}

int CPosProcessor::LoadComplex(PPID goodsID, SaComplex & rComplex)
{
	int    ok = -1;
	Goods2Tbl::Rec goods_rec, item_goods_rec;
	rComplex.Init(goodsID, 0, 1.0);
	if(GObj.Fetch(goodsID, &goods_rec) > 0 && goods_rec.Flags & GF_GENERIC) {
		PPGoodsStruc::Ident gs_ident(goodsID, GSF_COMPLEX, 0, getcurdate_());
		PPGoodsStruc gs;
		if(GObj.LoadGoodsStruc(&gs_ident, &gs) > 0) {
			rComplex.Init(goodsID, gs.Rec.ID, 1.0);
			SString temp_buf;
			StringSet ss(SLBColumnDelim);
			double item_qtty = 0.0;
			PPIDArray gen_list;
			PPGoodsStrucItem gs_item;
			{
				RetailGoodsInfo rgi;
				THROW(GetRgi(goodsID, 0.0, 0, rgi));
				rComplex.Price = rgi.Price;
			}
			for(uint p = 0; gs.EnumItemsExt(&p, &gs_item, 0, rComplex.Qtty, &item_qtty) > 0;) {
				if(GObj.Fetch(gs_item.GoodsID, &item_goods_rec) > 0) {
					{
						SaComplexEntry entry;
						entry.GoodsID = gs_item.GoodsID;
						entry.Qtty = fabs(item_qtty);
						THROW_SL(rComplex.insert(&entry));
					}
					SaComplexEntry & r_entry = rComplex.at(rComplex.getCount()-1);
					if(item_goods_rec.Flags & GF_GENERIC) {
						gen_list.clear();
						r_entry.Flags |= r_entry.fGeneric;
						if(GObj.GetGenericList(r_entry.GoodsID, &gen_list) > 0 && gen_list.getCount()) {
							double sum = 0.0;
							for(uint i = 0; i < gen_list.getCount(); i++) {
								const PPID gen_goods_id = gen_list.get(i);
								RetailGoodsInfo rgi;
								THROW(GetRgi(gen_goods_id, 0.0, PPObjGoods::rgifUseBaseQuotAsPrice, rgi));
								if(rgi.Price > 0.0) {
									r_entry.GenericList.Add(gen_goods_id, rgi.Price, 0);
									sum += (rgi.Price * r_entry.Qtty);
								}
							}
							if(r_entry.Qtty > 0.0)
								r_entry.OrgPrice = (sum / r_entry.Qtty);
						}
					}
					else {
						RetailGoodsInfo rgi;
						THROW(GetRgi(r_entry.GoodsID, 0.0, 0, rgi));
						r_entry.OrgPrice = rgi.Price;
					}
				}
			}
			if(rComplex.getCount())
				ok = 1;
		}
	}
	CATCHZOK
	return ok;
}
//
//
//
double CPosProcessor::CalcSCardOpAmount(const CCheckLineTbl::Rec & rItem, PPID chargeGoodsID, PPID crdGoodsGrpID, double * pNonCrdAmt)
{
	double charge = 0.0;
	double non_crd_amt = 0.0;
	const double s  = intmnytodbl(rItem.Price) * rItem.Quantity;
	const double ds = rItem.Dscnt * rItem.Quantity;
	if(crdGoodsGrpID) {
		if(GObj.BelongToGroup(rItem.GoodsID, crdGoodsGrpID, 0) > 0)
			charge = -R3(rItem.Quantity);
		else if(!oneof2(chargeGoodsID, 0, UNDEF_CHARGEGOODSID) && rItem.GoodsID == chargeGoodsID) {
			if(crdGoodsGrpID)
				charge = R3(rItem.Quantity);
			else
				charge = R2(s - ds);
		}
		else
			non_crd_amt = R2(s - ds);
	}
	else {
		if(!oneof2(chargeGoodsID, 0, UNDEF_CHARGEGOODSID) && rItem.GoodsID == chargeGoodsID)
			charge = R2(s - ds);
		else
			charge = -R2(s - ds);
	}
	ASSIGN_PTR(pNonCrdAmt, non_crd_amt);
	return charge;
}

double CPosProcessor::CalcSCardOpBonusAmount(const CCheckLineTbl::Rec & rItem, PPID bonusGoodsGrpID, double * pNonCrdAmt)
{
	double charge = 0.0;
	double non_crd_amt = 0.0;
	const double s  = intmnytodbl(rItem.Price) * rItem.Quantity;
	const double ds = rItem.Dscnt * rItem.Quantity;
	if(bonusGoodsGrpID) {
		if(GObj.BelongToGroup(rItem.GoodsID, bonusGoodsGrpID, 0) > 0)
			charge = -R2(s - ds);
		else
			non_crd_amt = R2(s - ds);
	}
	else {
		charge = -R2(s - ds);
	}
	ASSIGN_PTR(pNonCrdAmt, non_crd_amt);
	return charge;
}

double CPosProcessor::CalcCreditCharge(const CCheckPacket * pPack, const CCheckPacket * pExtPack,
	const CCheckItem * pCurItem, double * pNonCrdAmt, double * pBonusChargeAmt)
{
	double charge = 0.0;
	double non_crd_amt = 0.0;
	double bonus_charge_amt = 0.0;
	const  PPID scard_id = pPack ? pPack->Rec.SCardID : CSt.GetID();
	if(scard_id) {
		uint i;
		SCardTbl::Rec sc_rec;
		if(ScObj.Search(scard_id, &sc_rec) > 0) {
			PPObjSCardSeries scs_obj;
			PPSCardSeries scs_rec;
			if(scs_obj.Fetch(sc_rec.SeriesID, &scs_rec) > 0) {
				double nca = 0.0;
				CCheckLineTbl::Rec ccl_rec;
				CCheckItem * p_item;
				if(scs_rec.GetType() == scstBonus) {
					PPSCardConfig sc_cfg;
					ScObj.FetchConfig(&sc_cfg);
					if(!(sc_cfg.Flags & sc_cfg.fDontUseBonusCards)) {
						if(!F(fRetCheck)) { // �� �������� ����� � �������� ����� �� ��������� (����)
							const PPID bonus_goods_grp_id = scs_rec.BonusGrpID;
							const PPID bonus_charge_grp_id = scs_rec.BonusChrgGrpID;
							if(pPack) {
								for(i = 0; i < pPack->GetCount(); i++) {
									charge += CalcSCardOpBonusAmount(pPack->GetLine(i), bonus_goods_grp_id, &nca);
									non_crd_amt += nca;
									bonus_charge_amt += CalcSCardOpBonusAmount(pPack->GetLine(i), bonus_charge_grp_id, 0);
								}
								if(pExtPack)
									for(i = 0; i < pExtPack->GetCount(); i++) {
										charge += CalcSCardOpBonusAmount(pExtPack->GetLine(i), bonus_goods_grp_id, &nca);
										non_crd_amt += nca;
										bonus_charge_amt += CalcSCardOpBonusAmount(pExtPack->GetLine(i), bonus_charge_grp_id, 0);
									}
							}
							else {
								for(i = 0; P.enumItems(&i, (void**)&p_item);) {
									p_item->GetRec(ccl_rec, F(fRetCheck));
									charge += CalcSCardOpBonusAmount(ccl_rec, bonus_goods_grp_id, &nca);
									non_crd_amt += nca;
									bonus_charge_amt += CalcSCardOpBonusAmount(ccl_rec, bonus_charge_grp_id, 0);
								}
							}
							if(pCurItem) {
								pCurItem->GetRec(ccl_rec, F(fRetCheck));
								charge += CalcSCardOpBonusAmount(ccl_rec, bonus_goods_grp_id, &nca);
								non_crd_amt += nca;
								bonus_charge_amt += CalcSCardOpBonusAmount(ccl_rec, bonus_charge_grp_id, 0);
							}
						}
					}
				}
				else if(scs_rec.GetType() == scstCredit) {
					const PPID crd_goods_grp_id = scs_rec.CrdGoodsGrpID;
					const PPID charge_goods_id = GetChargeGoodsID(scard_id);
					if(charge_goods_id != UNDEF_CHARGEGOODSID || crd_goods_grp_id) {
						if(pPack) {
							for(i = 0; i < pPack->GetCount(); i++) {
								charge += CalcSCardOpAmount(pPack->GetLine(i), charge_goods_id, crd_goods_grp_id, &nca);
								non_crd_amt += nca;
							}
							if(pExtPack) {
								for(i = 0; i < pExtPack->GetCount(); i++) {
									charge += CalcSCardOpAmount(pExtPack->GetLine(i), charge_goods_id, crd_goods_grp_id, &nca);
									non_crd_amt += nca;
								}
							}
						}
						else {
							for(i = 0; P.enumItems(&i, (void**)&p_item);) {
								p_item->GetRec(ccl_rec, F(fRetCheck));
								charge += CalcSCardOpAmount(ccl_rec, charge_goods_id, crd_goods_grp_id, &nca);
								non_crd_amt += nca;
							}
						}
						if(pCurItem) {
							pCurItem->GetRec(ccl_rec, F(fRetCheck));
							charge += CalcSCardOpAmount(ccl_rec, charge_goods_id, crd_goods_grp_id, &nca);
							non_crd_amt += nca;
						}
					}
					else if(pPack) {
						charge -= MONEYTOLDBL(pPack->Rec.Amount);
						if(pExtPack)
							charge -= MONEYTOLDBL(pExtPack->Rec.Amount);
					}
					else {
						for(i = 0; P.enumItems(&i, (void**)&p_item);)
							charge -= p_item->GetAmount();
						if(pCurItem)
							charge -= pCurItem->GetAmount();
					}
				}
			}
		}
	}
	ASSIGN_PTR(pNonCrdAmt, non_crd_amt);
	ASSIGN_PTR(pBonusChargeAmt, bonus_charge_amt);
	return charge;
}

int CPosProcessor::Helper_InitCcPacket(CCheckPacket * pPack, CCheckPacket * pExtPack, const CcAmountList * pCcPl, long options)
{
	int    ok = 1;
	double amt = 0.0, dscnt = 0.0;
	double debug_amt = 0.0, debug_dscnt = 0.0; // @debug
	if(F(fRetCheck)) {
		if(pPack)
			pPack->Rec.Flags |= CCHKF_RETURN;
		if(pExtPack)
			pExtPack->Rec.Flags |= CCHKF_RETURN;
	}
	CalcTotal(&debug_amt, &debug_dscnt); // @debug
	if(options & iccpDontFillLines) {
		CalcTotal(&amt, &dscnt);
	}
	else {
		int   to_fill_ext_pack = BIN(!(pPack->Rec.Flags & CCHKF_SUSPENDED) && pExtPack && ExtCashNodeID);
		int   has_gift = 0;
		uint  i;
		CCheckItem * p_item = 0;
		CCheckItemArray to_fill_items;
		if(CnFlags & CASHF_UNIFYGDSATCHECK) {
			for(i = 0; P.enumItems(&i, (void**)&p_item);) {
				int  to_insert = 1;
				for(uint p = 0; to_fill_items.lsearch(&p_item->GoodsID, &p, CMPF_LONG); p++) {
					CCheckItem & r_dest_item = to_fill_items.at(p);
					if(p_item->CanMerge(pPack, r_dest_item)) {
						r_dest_item.Quantity += p_item->Quantity;
						to_insert = 0;
						break;
					}
				}
				if(to_insert)
					to_fill_items.insert(p_item);
			}
		}
		else
			to_fill_items = (CCheckItemArray &)P;
		for(i = 0; to_fill_items.enumItems(&i, (void**)&p_item);) {
			CCheckPacket * p_pack = (to_fill_ext_pack && BelongToExtCashNode(p_item->GoodsID)) ? pExtPack : pPack;
			if(p_item->Flags & cifUsedByGift)
				has_gift = 1;
			{
				short  division = p_item->Division;
				if(!division && P_DivGrpList) {
					int    use_default_div = 1;
					short  default_div = 0;
					PPGenCashNode::DivGrpAssc * p_dg_item;
					for(uint p = 0; !division && P_DivGrpList->enumItems(&p, (void **)&p_dg_item);)
						if(p_dg_item->GrpID == 0)
							default_div = p_dg_item->DivN;
						else if(GObj.BelongToGroup(p_item->GoodsID, p_dg_item->GrpID, 0) > 0) {
							division = p_dg_item->DivN;
							use_default_div = 0;
						}
					if(use_default_div)
						division = default_div;
					p_item->Division = division;
				}
			}
			THROW(p_pack->InsertItem(*p_item));
		}
		SETFLAG(pPack->Rec.Flags, CCHKF_HASGIFT, has_gift);
		THROW(pPack->CalcAmount(&amt, &dscnt));
	}
	LDBLTOMONEY(amt, pPack->Rec.Amount);
	LDBLTOMONEY(dscnt, pPack->Rec.Discount);
	pPack->Rec.SCardID = CSt.GetID();
	pPack->UhttScHash = CSt.UhttHash;
	P.SetupCCheckPacket(pPack);
	pPack->SetupPaymList(pCcPl); // @v8.0.0
	if(options & iccpSetCurTime)
		getcurdatetime(&pPack->Rec.Dt, &pPack->Rec.Tm);
	CATCHZOK
	return ok;
}

int CPosProcessor::AcceptCheckToBeCleared()
{
	int    ok = 1;
	CCheckPacket pack;
	GetNewCheckCode(CashNodeID, &pack.Rec.Code);
	pack.Rec.SessID = 0;
	pack.Rec.CashID = 0;
	pack.Rec.Flags |= (CCHKF_SYNC|CCHKF_NOTUSED|CCHKF_SKIP);
	THROW(Helper_InitCcPacket(&pack, 0, 0, iccpSetCurTime));
	{
		PPTransaction tra(1);
		THROW(tra);
		if(SuspCheckID) {
			pack.Rec.ID = SuspCheckID;
			THROW(CC.UpdateCheck(&pack, 0));
			SuspCheckID = 0;
		}
		else {
			THROW(CC.TurnCheck(&pack, 0));
		}
		DS.LogAction(PPACN_CLEARCHECK, PPOBJ_CCHECK, pack.Rec.ID, 0, 0);
		THROW(tra.Commit());
	}
	CATCHZOKPPERR
	return ok;
}

int CPosProcessor::AutosaveCheck()
{
	return (CsObj.GetEqCfg().Flags & PPEquipConfig::fAutosaveSyncChecks) ? AcceptCheck(0, 0.0, accmJunk) : -1;
}

// virtual
int CPosProcessor::AcceptCheck(const CcAmountList * pPl, double cash, int mode /* accmXXX */)
{
	int    ok = 1;
	const  int turn_check_before_printing = 1;
	int    was_turned_before_printing = 0;
	SString before_printing_check_text, msg_buf, fmt_buf;
	THROW_PP((mode != accmAveragePrinting) || P_ChkPack, PPERR_INVPARAM);
	if(CashNodeID) {
		AcceptCheckProcessBlock epb;
		if(!(Flags & fNoEdit)) {
			THROW_PP(!(CnFlags & CASHF_DISABLEZEROAGENT) || P.GetAgentID(), PPERR_CHKPAN_SALERNEEDED);
			THROW_PP(!(CnExtFlags & CASHFX_DISABLEZEROSCARD) || CSt.GetID(), PPERR_CHKPAN_SCARDNEEDED); // @v8.3.4
		}
		THROW(InitCashMachine());
		if(mode == accmAveragePrinting) {
			THROW_PP(OperRightsFlags & orfPrintCheck, PPERR_NORIGHTS);
			epb.Pack = *P_ChkPack;
		}
		else {
			if(F(fRetCheck)) {
				THROW_PP(OperRightsFlags & orfReturns, PPERR_NORIGHTS);
				epb.Pack.Rec.Flags |= CCHKF_RETURN;
			}
			if(pPl && pPl->Get(CCAMTTYP_BANK) != 0.0) {
				THROW_PP(OperRightsFlags & orfBanking, PPERR_NORIGHTS);
				epb.Pack.Rec.Flags |= CCHKF_BANKING;
			}
			if(SuspCheckID && CC.Search(SuspCheckID, &epb.LastChkRec) > 0) {
				// @debug {
				if((epb.LastChkRec.Flags & (CCHKF_JUNK|CCHKF_SUSPENDED)) != (CCHKF_JUNK|CCHKF_SUSPENDED)) {
					PPSetError(PPERR_CCHKMUSTBEJUNK, CCheckCore::MakeCodeString(&epb.LastChkRec, msg_buf));
					PPLogMessage(PPFILNAM_ERR_LOG, 0, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_LASTERR);
				}
				// } @debug
				epb.Pack.Rec.Code = epb.LastChkRec.Code;
				//
				// ��������� 2 ������ ������� �����������������, ���� ���� � ����� ����������� ���� ������
				// ��������������� ���������� ��������� (� ��������� ������ ��� ����� ��������������� ������� �������� ����).
				//
				//pack.Rec.Dt = last_chk_rec.Dt;
				//pack.Rec.Tm = last_chk_rec.Tm;
			}
			else
				GetNewCheckCode(CashNodeID, &epb.Pack.Rec.Code);
			if(mode == accmSuspended) {
				THROW_PP(OuterOi.Id == 0, PPERR_UNABLESUSPTSESSCHECK);
				//
				// ����� ���, ��� ��������� ���������� ���, ������� ������ ��������.
				// �������� ��� ������, ��� ��� �������������� ����� ���������� ������� ���������� //
				// ��������, ����� ����, � ��� ��� ��������� ���������� �������� cifGift � ������ ����.
				// ��� �������������� ������� ����� ��������� �����.
				//
				P.ClearGift();
			}
			else if(mode != accmJunk) {
				THROW_PP(OperRightsFlags & orfPrintCheck, PPERR_NORIGHTS);
			}
			epb.Pack.Rec.SessID = P_CM->GetCurSessID();
			epb.Pack.Rec.CashID = CashNodeID;
			epb.Pack.Rec.Flags |= (CCHKF_SYNC|CCHKF_NOTUSED);
			SETFLAG(epb.Pack.Rec.Flags, CCHKF_INCORPCRD, CSt.GetID() && Flags & fSCardCredit);
			epb.Pack.Rec.Flags &= ~CCHKF_BONUSCARD;
			SETFLAG(epb.Pack.Rec.Flags, CCHKF_SUSPENDED|CCHKF_SKIP, oneof2(mode, accmSuspended, accmJunk));
			SETFLAG(epb.Pack.Rec.Flags, CCHKF_JUNK, mode == accmJunk);
			SETFLAG(epb.Pack.Rec.Flags, CCHKF_PREPRINT, Flags & fPrinted);
			//
			// ����� ������������� ����������� ���� ���������� ������������ ���������� ������ (���� ��� ����)
			// �� ������� ����.
			//
			if(mode == accmRegular) {
				CCheckItem * p_item;
				for(uint i = 0; P.enumItems(&i, (void**)&p_item);)
					if(p_item->Flags & cifGiftDiscount) {
						SetupDiscount(1);
						break;
					}
			}
			THROW(Helper_InitCcPacket(&epb.Pack, ((mode == accmRegular) ? &epb.ExtPack : 0), pPl, 0));
			if(mode == accmRegular && P_CM_EXT) {
				epb.Pack._Cash = MONEYTOLDBL(epb.Pack.Rec.Amount);
				epb.IsExtPack = BIN(epb.ExtPack.GetCount());
				if(epb.IsExtPack) {
					double amt, dscnt;
					GetNewCheckCode(ExtCashNodeID, &epb.ExtPack.Rec.Code);
					epb.ExtPack.Rec.SessID = P_CM_EXT->GetCurSessID(); // @!
					epb.ExtPack.Rec.CashID = ExtCashNodeID;
					epb.ExtPack.Rec.Flags  = epb.Pack.Rec.Flags;
					THROW(epb.ExtPack.SetupAmount(&amt, &dscnt));
					epb.ExtPack.Rec.SCardID = CSt.GetID();
					P.SetupCCheckPacket(&epb.ExtPack);
					epb.ExtPack._Cash = amt;
				}
			}
			else {
				/* @v9.0.4
				epb.Pack.Ext.AddPaym = 0; // @v8.0.2 dbltointmny(CSt.AdditionalPayment);
				SETFLAG(epb.Pack.Rec.Flags, CCHKF_ADDPAYM, epb.Pack.Ext.AddPaym);
				if(CSt.AddCrdCardID) {
					epb.Pack.Ext.AddCrdCardID = CSt.AddCrdCardID;
					epb.Pack.Ext.AddCrdCardPaym = dbltointmny(CSt.AddCrdCardPayment);
				}
				SETFLAG(epb.Pack.Rec.Flags, CCHKF_ADDINCORPCRD, CSt.AddCrdCardID);
				*/
				epb.Pack._Cash = cash;
			}
		}
		epb.IsPack = BIN(epb.Pack.GetCount());
		if(mode == accmJunk) {
			THROW(StoreCheck(&epb.Pack, 0, mode));
		}
		else {
			const long org_code = epb.Pack.Rec.Code;
			const long org_flags = epb.Pack.Rec.Flags;
			const long org_ext_code = epb.ExtPack.Rec.Code;
			const long org_ext_flags = epb.ExtPack.Rec.Flags;
			const long org_sess_id = epb.Pack.Rec.SessID;
			const long org_ext_sess_id = epb.ExtPack.Rec.SessID;
			int   dont_accept_ccode_from_printer = 0; // @v9.1.10
			// @v9.0.11 {
			if(mode == accmRegular) {
				if(oneof2(EgaisMode, 1, 2)) {
					// @v9.1.10 {
					if(EgaisMode == 1) {
						dont_accept_ccode_from_printer = 1;
					}
					// } @v9.1.10
					if(P_EgPrc) {
						// @v9.1.0 {
						//
						// ����� ��������� � ����� ���������� ��������������� ���������� �����
						// ��� ����, ��� �� �������������� �������� ������� �����.
						// ������, ��� �� ��������� ��������, ����������� � ������, ���� �����
						// ��������� ���������� ����� ����� ���������� ����������� ��� ������
						// �������� ���� ����������� �����. ��� ���� ��������� ���, ��� � �� �
						// ���������� ������ ����� �������� �� �� ������, ������� ���� �������� � �����.
						//
						if(turn_check_before_printing && !was_turned_before_printing && mode != accmAveragePrinting) {
							THROW(StoreCheck(&epb.Pack, epb.IsExtPack ? &epb.ExtPack : 0, mode));
							CCheckCore::MakeCodeString(&epb.Pack.Rec, before_printing_check_text);
							was_turned_before_printing = 1;
						}
						// } @v9.1.0
						{
							PPEgaisProcessor::Ack eg_ack;
							THROW(P_EgPrc->PutCCheck(epb.Pack, CnLocID, eg_ack));
                            if(eg_ack.Sign[0] && eg_ack.SignSize) {
								(msg_buf = 0).CatN((const char *)eg_ack.Sign, eg_ack.SignSize);
                                epb.Pack.PutExtStrData(CCheckPacket::extssSign, msg_buf);
                            }
                            // @v9.1.8 {
                            if(eg_ack.Url.NotEmpty()) {
								epb.Pack.PutExtStrData(CCheckPacket::extssEgaisUrl, eg_ack.Url);
                            }
                            // } @v9.1.8
						}
						if(epb.IsExtPack) {
							PPEgaisProcessor::Ack eg_ack;
							THROW(P_EgPrc->PutCCheck(epb.ExtPack, ExtCnLocID, eg_ack));
                            if(eg_ack.Sign[0] && eg_ack.SignSize) {
								(msg_buf = 0).CatN((const char *)eg_ack.Sign, eg_ack.SignSize);
                                epb.ExtPack.PutExtStrData(CCheckPacket::extssSign, msg_buf);
                            }
                            // @v9.1.8 {
                            if(eg_ack.Url.NotEmpty()) {
								epb.ExtPack.PutExtStrData(CCheckPacket::extssEgaisUrl, eg_ack.Url);
                            }
                            // } @v9.1.8
						}
					}
				}
			}
			// } @v9.0.11
			if(turn_check_before_printing && !was_turned_before_printing && mode != accmAveragePrinting) {
				THROW(StoreCheck(&epb.Pack, epb.IsExtPack ? &epb.ExtPack : 0, mode));
				CCheckCore::MakeCodeString(&epb.Pack.Rec, before_printing_check_text);
				was_turned_before_printing = 1;
			}
			if(mode != accmSuspended) {
				if(!Implement_AcceptCheckOnEquipment(pPl, epb))
					ok = 0;
			}
			// @v9.1.10 {
			if(dont_accept_ccode_from_printer) {
				epb.Pack.Rec.Code = org_code;
				if(epb.IsExtPack)
					epb.ExtPack.Rec.Code = org_ext_code;
			}
			// } @v9.1.10
			if(was_turned_before_printing) { // @v9.1.0 turn_check_before_printing-->was_turned_before_printing
				//
				// ��� ���������� ���� ����� ������� ����� ���������� ��������, �����
				// ������ �������� ��������� ���� ����. � ���� ������ ���������� � �� �������� �������� ���� �����.
				//
				PPTransaction tra(1);
				THROW(tra);
				if(org_code != epb.Pack.Rec.Code || org_flags != epb.Pack.Rec.Flags || org_sess_id != epb.Pack.Rec.SessID) {
					THROW_DB(updateFor(&CC, 0, CC.ID == epb.Pack.Rec.ID,
						set(CC.Code, dbconst(epb.Pack.Rec.Code)).set(CC.Flags, dbconst(epb.Pack.Rec.Flags)).set(CC.SessID, dbconst(epb.Pack.Rec.SessID))));
				}
				if(org_ext_code != epb.ExtPack.Rec.Code || org_ext_flags != epb.ExtPack.Rec.Flags || org_ext_sess_id != epb.ExtPack.Rec.SessID) {
					THROW_DB(updateFor(&CC, 0, CC.ID == epb.ExtPack.Rec.ID,
						set(CC.Code, dbconst(epb.ExtPack.Rec.Code)).set(CC.Flags, dbconst(epb.ExtPack.Rec.Flags)).set(CC.SessID, dbconst(epb.ExtPack.Rec.SessID))));
				}
				THROW(tra.Commit());
			}
			else if(mode != accmAveragePrinting) {
				THROW(StoreCheck(&epb.Pack, epb.IsExtPack ? &epb.ExtPack : 0, mode));
			}
			//
			// �� ������ ������ ���� � ��., ��������� � Implement_AcceptCheckOnEquipment(), ��������� ��� �����
			// ���������� ���������� � ���� ������.
			//
			if((epb.R == 0 && epb.SyncPrnErr != 3) || (epb.RExt == 0 && epb.ExtSyncPrnErr != 3)) // ???
				PPError();
		}
	}
	if(!oneof2(mode, accmAveragePrinting, accmJunk))
		ClearCheck();
	CATCH
		ok = PPErrorZ();
		if(was_turned_before_printing) {
			ClearCheck();
			MessageError(PPERR_CHKPAN_CHKTURNEDBEFOREPRNERR, before_printing_check_text, eomPopup);
		}
	ENDCATCH
	return ok;
}

//virtual
int CPosProcessor::ClearCheck()
{
	if(SuspCheckID) {
		if(!CC.RemovePacket(SuspCheckID, 1))
			PPLogMessage(PPFILNAM_ERR_LOG, 0, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_LASTERR);
		SuspCheckID = 0;
	}
	//SetPrintedFlag(0);
	SetupAgent(0, 0);
	P.OrderCheckID = 0;
	P.Clear();
	OuterOi.Set(0, 0);
	if(oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF))
		SetupState(sEMPTYLIST_EMPTYBUF);
	//ClearRow();
	ResetSCard();
	Flags &= ~fBankingPayment;
	OnUpdateList(0);
	SetPrintedFlag(0);
	//setupRetCheck(0);
	return 1;
}

//virtual
int CPosProcessor::ClearRow()
{
	P.ClearCur();
	SetupRowData(0);
	SetupState(oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF) ? sLISTSEL_EMPTYBUF : (P.getCount() ? sLIST_EMPTYBUF : sEMPTYLIST_EMPTYBUF));
	return 1;
}

int CPosProcessor::Helper_SetupSessUuidForCheck(PPID checkID)
{
	int    ok = 1;
	if(checkID) {
		ObjTagItem tag_item;
		S_GUID uuid;
		if(SessUUID.IsZero())
			uuid = SLS.GetSessUuid();
		else
			uuid = SessUUID;
		if(tag_item.SetGuid(PPTAG_CCHECK_JS_UUID, &uuid))
			PPRef->Ot.PutTag(PPOBJ_CCHECK, checkID, &tag_item, 0);
	}
	else
		ok = -1;
	return ok;
}

int CPosProcessor::IsOnlyChargeGoodsInPacket(PPID scID, const CCheckPacket * pPack)
{
	int    only_charge_goods = 1;
	const PPID charge_goods_id = GetChargeGoodsID(scID);
	if(pPack) {
		for(uint i = 0; only_charge_goods && i < pPack->GetCount(); i++) {
			const CCheckLineTbl::Rec & r_line = pPack->GetLine(i);
			if(r_line.GoodsID != charge_goods_id)
				only_charge_goods = 0;
		}
	}
	else {
		for(uint i = 0; only_charge_goods && i < P.getCount(); i++) {
			const CCheckItem & r_line = P.at(i);
			if(r_line.GoodsID != charge_goods_id)
				only_charge_goods = 0;
		}
	}
	return only_charge_goods;
}

int CPosProcessor::StoreCheck(CCheckPacket * pPack, CCheckPacket * pExtPack, int mode)
{
 	int    ok = 1;
 	int    _turn_done = 0; // ������� ����, ��� ���� ��������� ������� CCheckCore::TurnCheck ��� CCheckCore::UpdateCheck
	int    do_clear_junk_attrs = 0;
	SString temp_buf;
	PPCheckInPersonMngr cip_mgr;
	const  LDATETIME dtm = getcurdatetime_();
	assert(pPack);
	THROW_PP(pPack != 0, PPERR_INVPARAM);
	const  PPID  preserve_csess_id = pPack->Rec.SessID;
	{
		PPTransaction tra(1);
		THROW(tra);
		double temp_val = 0.0;
		double non_crd_amt = 0.0;
		const double _charge = (mode != accmJunk) ? CalcCreditCharge(pPack, pExtPack, 0, &non_crd_amt, &temp_val) : 0.0;
		const double bonus_charge_amt = (mode != accmJunk && temp_val < 0.0) ? -temp_val : 0.0;
		if(pPack->GetCount()) {
			if(P.Eccd.Flags & P.Eccd.fDelivery) {
				pPack->Rec.Flags |= CCHKF_DELIVERY;
				pPack->Ext.StartOrdDtm = P.Eccd.DlvrDtm;
				if(P.Eccd.Addr_.Tail[0]) {
					if(!P.Eccd.Addr_.ID) {
						LocationCore::GetExField(&P.Eccd.Addr_, LOCEXSTR_PHONE, temp_buf);
						if(temp_buf.NotEmptyS()) {
							//
							// ���� � ������ ���� �������, �� ��������� ����� ����� ����������,
							// ���� ��� ��������� ��������� ����� ������� ����� ���� ��
							// �������� ������������ ���� �����.
							//
							P.Eccd.Addr_.Flags |= LOCF_STANDALONE;
						}
						SETIFZ(P.Eccd.Addr_.Type, LOCTYP_ADDRESS);
						THROW(PsnObj.LocObj.PutRecord(&P.Eccd.Addr_.ID, &P.Eccd.Addr_, 0));
					}
					// @v9.4.5 {
					if(P.Eccd.Addr_.ID && P.Eccd.SCardID_) {
						PPSCardPacket sc_pack;
						if(ScObj.GetPacket(P.Eccd.SCardID_, &sc_pack) > 0) {
							if(sc_pack.Rec.LocID == 0 && sc_pack.Rec.PersonID == 0) {
								sc_pack.Rec.LocID = P.Eccd.Addr_.ID;
								THROW(ScObj.PutPacket(&P.Eccd.SCardID_, &sc_pack, 0));
							}
						}
					}
					// } @v9.4.5
					pPack->Ext.AddrID = P.Eccd.Addr_.ID;
				}
			}
			SETFLAG(pPack->Rec.Flags, CCHKF_FIXEDPRICE, P.Eccd.Flags & P.Eccd.fFixedPrice); // @v8.7.7
			pPack->Ext.CreationDtm = P.Eccd.InitDtm;
			SETFLAG(pPack->Rec.Flags, CCHKF_EXT, pPack->HasExt()); // @v9.4.5
			SETIFZ(pPack->Rec.Dt, dtm.d);
			SETIFZ(pPack->Rec.Tm, dtm.t);
			{
				if(mode == accmJunk) {
					pPack->Rec.Flags |= (CCHKF_SUSPENDED|CCHKF_JUNK);
					pPack->Rec.SessID = 0;
					do_clear_junk_attrs = 1;
				}
				if(SuspCheckID) {
					pPack->Rec.ID = SuspCheckID;
					THROW(CC.UpdateCheck(pPack, 0));
					SuspCheckID = 0;
				}
				else {
					THROW(CC.TurnCheck(pPack, 0));
				}
				_turn_done = 1;
				if(mode == accmJunk) {
					Helper_SetupSessUuidForCheck(pPack->Rec.ID);
					SuspCheckID = pPack->Rec.ID;
				}
			}
		}
		if(mode != accmJunk) {
			if(pExtPack) {
				SETIFZ(pExtPack->Rec.Dt, dtm.d);
				SETIFZ(pExtPack->Rec.Tm, dtm.t);
				THROW(CC.TurnCheck(pExtPack, 0));
			}
			if(OuterOi.Obj == PPOBJ_TSESSION && OuterOi.Id && P_TSesObj) {
				TSessionTbl::Rec tses_rec;
				if(P_TSesObj->Search(OuterOi.Id, &tses_rec) > 0) {
					tses_rec.CCheckID_ = pPack->Rec.ID;
					PPID   tsess_id = OuterOi.Id;
					THROW(P_TSesObj->PutRec(&tsess_id, &tses_rec, 0));
				}
			}
			else if(OuterOi.Obj == PPOBJ_CHKINP && OuterOi.Id) {
				PPCheckInPersonItem cip_item;
				if(cip_mgr.Search(OuterOi.Id, &cip_item) > 0) {
					cip_item.CCheckID = pPack->Rec.ID;
					THROW(cip_mgr.Put(cip_item, 0));
				}
			}
			if(mode == accmRegular && pPack && !(pPack->Rec.Flags & (CCHKF_JUNK|CCHKF_SKIP))) {
				int    scst = scstUnkn;
				SCardTbl::Rec sc_rec;
				MEMSZERO(sc_rec);
				if(CSt.GetID()) {
					THROW(ScObj.Search(CSt.GetID(), &sc_rec) > 0);
					scst = ScObj.GetSeriesType(sc_rec.SeriesID);
					//
					// ������������� �����
					//
					const long scf_mask = SCRDF_NEEDACTIVATION|SCRDF_AUTOACTIVATION|SCRDF_CLOSED;
					if((sc_rec.Flags & scf_mask) == scf_mask) {
						//
						// ���������� �� ����� �� ������� ���������, ������� ����� ������������ �����
						//
						const int only_charge_goods = IsOnlyChargeGoodsInPacket(sc_rec.ID, pPack);
						if(!only_charge_goods && ScObj.ActivateRec(&sc_rec) > 0)
							THROW(ScObj.P_Tbl->Update(sc_rec.ID, &sc_rec, 0));
					}
				}
				if(pPack->AL_Const().getCount()) {
					//
					// ���� � ���� ���������� ����������� ������ �����, �� ��� �������� ����������� �������� CCheckCore::TurnCheck
					// ����� ��� �������� ������ ��������� ������ �� ����� (���� ����������)
					//
					if(!F(fRetCheck)) {
						if(CSt.GetID() && CSt.Flags & CSt.fUhtt && F(fSCardBonusReal|fSCardCredit) && bonus_charge_amt != 0.0) { // @v8.0.6 fSCardBonus-->fSCardBonusReal
							if(CSt.UhttCode[0]) {
								int    is_error = 0;
								PPUhttClient uhtt_cli;
								if(uhtt_cli.Auth()) {
									if(F(fSCardBonusReal) && bonus_charge_amt > 0.0) { // @v8.0.6 fSCardBonus-->fSCardBonusReal
										UhttCheckPacket uhtt_cc_pack;
										double finish_bonus_charge_amt = bonus_charge_amt;
										// @v8.2.10 {
										{
											PPObjSCardSeries scs_obj;
											PPSCardSeries scs_rec;
											if(scs_obj.Fetch(sc_rec.SeriesID, &scs_rec) > 0 && scs_rec.BonusChrgExtRule) {
												if(scs_rec.Flags & SCRDSF_BONUSER_ONBNK) {
													if(pPack->AL_Const().Get(CCAMTTYP_BANK) != 0.0) {
														double _coeff = 1.0 + (((double)scs_rec.BonusChrgExtRule) / (10.0 * 100.0));
														finish_bonus_charge_amt *= _coeff;
													}
												}
											}
										}
										// } @v8.2.10
										uhtt_cc_pack.Amount = finish_bonus_charge_amt;
										LocationTbl::Rec loc_rec;
										if(PsnObj.LocObj.Fetch(CnLocID, &loc_rec) > 0 && !isempty(loc_rec.Code)) {
											if(!uhtt_cli.CreateSCardCheck(loc_rec.Code, CSt.UhttCode, uhtt_cc_pack)) {
												PPSetError(PPERR_UHTT_SCBONUSREG, uhtt_cli.GetLastMessage());
												is_error = 1;
											}
										}
										else {
											SString uhtt_err_added_msg;
											PPGetMessage(mfError, PPERR_LOCSYMBUNDEF, loc_rec.Name, 0, uhtt_err_added_msg);
											PPSetError(PPERR_UHTT_SCBONUSREG, uhtt_err_added_msg);
											is_error = 1;
										}
									}
								}
								else {
									is_error = 1;
								}
								if(is_error)
									MessageError(-1, 0, eomPopup);
							}
						}
					}
				}
				else {
					if(F(fRetCheck)) {
						//
						// ������������� �������� �� fRetCheck � fRetByCredit - ��������������, �� ���������� - ��� �������.
						//
						if(F(fRetByCredit) && CSt.GetID() && oneof2(scst, scstCredit, scstBonus)) {
							double total = MONEYTOLDBL(pPack->Rec.Amount);
							//
							// ��� ��������� ����� �������������, �� ���������� ������ ���� �������������
							//
							double charge_amount = R2(-(total - CSt.AdditionalPayment));
							if(charge_amount >= 0.01) {
								if(CSt.Flags & CSt.fUhtt && CSt.UhttCode[0]) {
									if(!ScObj.PutUhttOp(CSt.GetID(), charge_amount)) {
										MessageError(-1, 0, eomPopup);
									}
								}
								SCardCore::OpBlock blk;
								blk.Dtm.Set(pPack->Rec.Dt, pPack->Rec.Tm);
								blk.SCardID = CSt.GetID();
								blk.LinkOi.Set(PPOBJ_CCHECK, pPack->Rec.ID);
								blk.Amount = charge_amount;
								THROW(ScObj.P_Tbl->PutOpBlk(blk, 0));
							}
						}
					}
					else {
						// @v8.1.0 SCardOpTbl::Rec scop_rec;
						double bonus_withdraw_amt = 0.0;
						if(_charge != 0.0) {
		#if 0 // @v8.1.0 �������� � �������� (���������) ���� �������������� ������ �������� ���������� ���� {
		#endif // } 0 @v8.1.0
						}
						// @v8.0.6 if(CSt.GetID() && CSt.Flags & CSt.fUhtt && F(fSCardBonusReal|fSCardCredit) && (bonus_withdraw_amt != 0.0 || bonus_charge_amt != 0.0)) {
						if(CSt.GetID() && (CSt.Flags & CSt.fUhtt) && ((F(fSCardBonusReal) && bonus_charge_amt != 0.0) || (F(fSCardBonus|fSCardCredit) && bonus_withdraw_amt != 0.0))) { // @v8.0.6
							if(CSt.UhttCode[0]) {
								int    is_error = 0;
								PPUhttClient uhtt_cli;
								if(uhtt_cli.Auth()) {
									if(bonus_withdraw_amt > 0.01 && F(fSCardBonus|fSCardCredit)) {
										if(!uhtt_cli.WithdrawSCardAmount(CSt.UhttCode, bonus_withdraw_amt)) {
											PPSetError(PPERR_UHTT_SCWITHDRAW, uhtt_cli.GetLastMessage());
											is_error = 1;
										}
									}
									if(F(fSCardBonusReal) && bonus_charge_amt > 0.0) { // @v8.0.6 fSCardBonus-->fSCardBonusReal
										UhttCheckPacket uhtt_cc_pack;
										double finish_bonus_charge_amt = bonus_charge_amt;
										// @v8.2.10 {
										{
											PPObjSCardSeries scs_obj;
											PPSCardSeries scs_rec;
											if(scs_obj.Fetch(sc_rec.SeriesID, &scs_rec) > 0 && scs_rec.BonusChrgExtRule) {
												if(scs_rec.Flags & SCRDSF_BONUSER_ONBNK) {
													if(pPack->Rec.Flags & CCHKF_BANKING) {
														double _coeff = 1.0 + (((double)scs_rec.BonusChrgExtRule) / (10.0 * 100.0));
														finish_bonus_charge_amt *= _coeff;
													}
												}
											}
										}
										// } @v8.2.10
										uhtt_cc_pack.Amount = finish_bonus_charge_amt;
										LocationTbl::Rec loc_rec;
										if(PsnObj.LocObj.Fetch(CnLocID, &loc_rec) > 0 && !isempty(loc_rec.Code)) {
											if(!uhtt_cli.CreateSCardCheck(loc_rec.Code, CSt.UhttCode, uhtt_cc_pack)) {
												PPSetError(PPERR_UHTT_SCBONUSREG, uhtt_cli.GetLastMessage());
												is_error = 1;
											}
										}
										else {
											SString uhtt_err_added_msg;
											PPGetMessage(mfError, PPERR_LOCSYMBUNDEF, loc_rec.Name, 0, uhtt_err_added_msg);
											PPSetError(PPERR_UHTT_SCBONUSREG, uhtt_err_added_msg);
											is_error = 1;
										}
									}
								}
								else {
									is_error = 1;
								}
								if(is_error)
									MessageError(-1, 0, eomPopup);
							}
						}
					}
				}
			}
		}
		if(!_turn_done && SuspCheckID) {
			THROW(CC.RemovePacket(SuspCheckID, 0));
			SuspCheckID = 0;
		}
		CC.WriteCCheckLogFile(pPack, 0, (mode == accmRegular) ? CCheckCore::logWrited : CCheckCore::logSuspended, 0);
		THROW(tra.Commit());
	}
	CATCHZOK
	if(do_clear_junk_attrs) {
		pPack->Rec.Flags &= ~(CCHKF_SUSPENDED|CCHKF_JUNK);
		pPack->Rec.SessID = preserve_csess_id;
	}
	return ok;
}
//
//static
int CheckPaneDialog::PalmImport(PalmBillPacket * pPack, void * extraPtr)
{
	int    ok = PIPR_ERROR_BREAK;
	CheckPaneDialog * dlg = (CheckPaneDialog *)extraPtr; // This func don't ownes by dlg
	if(pPack && dlg->IsState(sEMPTYLIST_EMPTYBUF)) {
		SString palm_name;
		GetObjectName(PPOBJ_STYLOPALM, pPack->Hdr.PalmID, palm_name);
		int    r = PPMessage(mfConf|mfYes|mfNo|mfCancel|mfDefaultYes, PPCFM_ACCEPTCHECKFROMPALM, palm_name);
		if(r == cmYes) {
			PalmBillItem item;
			for(uint i = 0; pPack->EnumItems(&i, &item) > 0;) {
				RetailGoodsInfo rgi;
				if(dlg->GetRgi(item.GoodsID, item.Qtty, 0, rgi)) {
					double price = rgi.Price;
					CCheckItem chk_item;
					chk_item.GoodsID = item.GoodsID;
					STRNSCPY(chk_item.GoodsName, rgi.Name);
					STRNSCPY(chk_item.BarCode,   rgi.BarCode);
					chk_item.Quantity = R6((dlg->Flags & fRetCheck) ? -fabs(item.Qtty) : fabs(item.Qtty));
					chk_item.Price    = price;
					chk_item.Discount = 0.0;
					dlg->P.insert(&chk_item);
				}
			}
			dlg->SetupDiscount();
			dlg->OnUpdateList(1);
			dlg->ClearRow();
			ok = PIPR_OK_BREAK;
		}
		else if(r == cmNo)
			ok = PIPR_OK_DESTROY;
		else if(r == cmCancel)
			ok = PIPR_OK_DESTROY;
		else // Default
			ok = PIPR_ERROR_BREAK;
	}
	return ok;
}

//static
int CheckPaneDialog::SetLbxItemHight(TDialog *, long extraParam)
{
	int    ok = -1;
	PPSyncCashNode  scn;
	PPObjCashNode   cn_obj;
	if(cn_obj.GetSync(extraParam, &scn) > 0) {
		const PPID ts_id = NZOR(scn.LocalTouchScrID, scn.TouchScreenID);
		if(ts_id) {
			PPObjTouchScreen ts_obj;
			PPTouchScreenPacket ts_pack;
			if(ts_obj.GetPacket(ts_id, &ts_pack) > 0) {
				int     height  = 0;
				PPID    ctrls[] = { CTL_CHKPAN_GDSLIST, CTL_CHKPAN_GRPLIST };
				if(ts_pack.Rec.GdsListFontHight && ts_pack.Rec.GdsListFontName[0]) {
					HDC  dc = GetDC(0);
					int  cy = GetDeviceCaps(dc, LOGPIXELSY);
					height  = labs(ts_pack.Rec.GdsListFontHight) * 72 / cy;
					ReleaseDC(0, dc);
				}
				else
					height = DEFAULT_TS_FONTSIZE;
				for(int i = 0, j = 0; i < 32; i++)
					if(OwnerDrawCtrls[i].CtrlType == 0 && OwnerDrawCtrls[i].CtrlID == 0) {
						OwnerDrawCtrls[i].CtrlType = ctListBox;
						OwnerDrawCtrls[i].CtrlID   = ctrls[j];
						OwnerDrawCtrls[i].ExtraParam = height + ts_pack.Rec.GdsListEntryGap;
						ok = 1;
						if(++j == sizeof(ctrls)/sizeof(PPID))
							break;
					}
			}
		}
	}
	return ok;
}

int CheckPaneDialog::SelectGroup(PPID * pGrpID)
{
	int    ok = -1;
	SmartListBox * p_box = (SmartListBox *)getCtrlView(CTL_CHKPAN_GRPLIST);
	if(p_box && p_box->def) {
		PPID   grp_id = 0;
		p_box->def->getCurID(&grp_id);
		GrpListItem * p_item = GroupList.Get(grp_id, 0);
		if(p_item) {
			if(p_item->Flags & GrpListItem::fFolder) {
				if(UiFlags & uifOneGroupLevel) {
					GroupList.TopID = p_item->ID;
				}
				else {
					INVERSEFLAG(p_item->Flags, GrpListItem::fOpened);
				}
				ok = 1;
			}
			else
				ok = 2;
			ASSIGN_PTR(pGrpID, p_item->ID);
		}
	}
	return ok;
}

int CheckPaneDialog::MakeGroupEntryList(StrAssocArray * pTreeList, PPID parentID, uint level)
{
	int    ok = -1;
	for(uint i = 0; i < pTreeList->getCount(); i++) {
		StrAssocArray::Item item = pTreeList->at(i);
		if(item.ParentId == parentID) {
			uint   pos = 0;
			int    is_opened = -1;
			GrpListItem gli;
			if(GroupList.Get(item.Id, &pos)) {
				is_opened = BIN(GroupList.at(pos).Flags & GrpListItem::fOpened);
				GroupList.atFree(pos);
			}
			MEMSZERO(gli);
			gli.ID = item.Id;
			gli.ParentID = item.ParentId;
			gli.Level = (uint16)level;
			pos = GroupList.getCount();
			GroupList.insert(&gli);
			if(MakeGroupEntryList(pTreeList, item.Id, level + 1) > 0) { // @recursion
				GroupList.at(pos).Flags |= GrpListItem::fFolder;
				if(is_opened > 0)
					GroupList.at(pos).Flags |= GrpListItem::fOpened;
			}
			ok = 1;
		}
	}
	return ok;
}
//
// ������������� ������ �����, ������� ����� ��������
//
int CheckPaneDialog::InitGroupList(const PPTouchScreenPacket & rTsPack)
{
	int    ok = 1;
	SString   temp_buf;
	Goods2Tbl::Rec goods_rec;
	PPIDArray grp_id_list(rTsPack.GrpIDList);
	PPID   grp_id = 0;
	GroupList.freeAll(); // @v8.6.9 @fix
	if(grp_id_list.getCount() == 0) {
		for(GoodsGroupIterator gg_iter(0); gg_iter.Next(&grp_id, temp_buf) > 0;) {
			while(grp_id && GObj.Fetch(grp_id, &goods_rec) > 0) {
				grp_id_list.add(grp_id); // @v8.0.10 addUnique-->add
				grp_id = goods_rec.ParentID;
			}
		}
	}
	grp_id_list.sortAndUndup(); // @v8.0.10
	{
		StrAssocArray temp_list;
		for(uint p = 0; p < grp_id_list.getCount(); p++) {
			if(GObj.Fetch(grp_id_list.get(p), &goods_rec) > 0) {
				const PPID par_id = goods_rec.ParentID;
				temp_list.AddFast(goods_rec.ID, (par_id && grp_id_list.bsearch(par_id)) ? par_id : 0, goods_rec.Name); // v8.0.10 Add-->AddFast; lsearch-->bsearch
			}
		}
		temp_list.SortByText();
		THROW(MakeGroupEntryList(&temp_list, 0, 0));
	}
	CATCHZOK
	return ok;
}

IMPLEMENT_PPFILT_FACTORY(CashNodePane); CashNodePaneFilt::CashNodePaneFilt(): PPBaseFilt(PPFILT_CASHNODEPANE, 0, 1)
{
	SetFlatChunk(offsetof(CashNodePaneFilt, ReserveStart),
		offsetof(CashNodePaneFilt, ReserveEnd) - offsetof(CashNodePaneFilt, ReserveStart));
	Init(1, 0);
}

CheckPaneDialog::GroupArray::GroupArray() : TSArray <CheckPaneDialog::GrpListItem> ()
{
	TopID = 0;
}

CheckPaneDialog::GrpListItem * CheckPaneDialog::GroupArray::Get(PPID id, uint * pPos) const
{
	uint   pos = 0;
	if(lsearch(&id, &pos, CMPF_LONG)) {
		ASSIGN_PTR(pPos, pos);
		return &at(pos);
	}
	else {
		ASSIGN_PTR(pPos, 0);
		return 0;
	}
}

CheckPaneDialog::CheckPaneDialog(PPID cashNodeID, PPID checkID, CCheckPacket * pOuterPack, int isTouchScreen) :
	TDialog(pOuterPack ? (isTouchScreen ? DLG_CHKPANV_L : DLG_CHKPANV) : (isTouchScreen ? DLG_CHKPAN_TS : DLG_CHKPAN),
		isTouchScreen ? CheckPaneDialog::SetLbxItemHight : 0, cashNodeID),
	CPosProcessor(cashNodeID, checkID, pOuterPack, isTouchScreen),
	PhnSvcTimer(1000), UhttImportTimer(180000)
{
	SString font_face, temp_buf;

	UiFlags = 0;
	BarrierViolationCounter = 0; // @debug
	TouchScreenID = 0;
	ScaleID = 0;
	SetupState(sEMPTYLIST_EMPTYBUF);
	SETFLAG(DlgFlags, fLarge, isTouchScreen);
	SelGoodsGrpID = AltGoodsGrpID = 0;
	ActiveListID  = 0;
	GoodsListFontHeight = 0;
	GoodsListEntryGap   = 0;
	Ptb.SetColor(clrFocus,  RGB(0x20, 0xAC, 0x90));
	Ptb.SetColor(clrEven,   RGB(0xD3, 0xEF, 0xF4));
	Ptb.SetColor(clrOdd,    RGB(0xDC, 0xED, 0xD5));
	Ptb.SetColor(clrGrp,    RGB(0xDA, 0xD7, 0xD0));
	Ptb.SetColor(clrParent, RGB(0x80, 0xB9, 0xE8));
	Ptb.SetBrush(brSel,  SPaintObj::psSolid, Ptb.GetColor(clrFocus), 0);
	Ptb.SetBrush(brEven, SPaintObj::psSolid, Ptb.GetColor(clrEven), 0);
	Ptb.SetBrush(brOdd,  SPaintObj::psSolid, Ptb.GetColor(clrOdd), 0);
	Ptb.SetBrush(brErrorBkg,   SPaintObj::psSolid, GetColorRef(SClrRed), 0);
	Ptb.SetBrush(brPresentBkg, SPaintObj::psSolid, GetColorRef(SClrGreen), 0);
	Ptb.SetBrush(brGrpSel,     SPaintObj::psSolid, Ptb.GetColor(clrFocus), 0);
	Ptb.SetBrush(brGrp,        SPaintObj::psSolid, Ptb.GetColor(clrGrp), 0);
	Ptb.SetBrush(brGrpParent,  SPaintObj::psSolid, Ptb.GetColor(clrParent), 0);
	Ptb.SetPen(penSel, SPaintObj::psSolid, 1, RGB(0x66, 0x33, 0xFF));
	Ptb.SetBrush(brTotalGift,    SPaintObj::psSolid, GetColorRef(SClrSalmon), 0);
	Ptb.SetBrush(brDiscountGift, SPaintObj::psSolid, GetColorRef(SClrSeagreen), 0);
	Ptb.SetBrush(brOrderBkg,     SPaintObj::psSolid, GetColorRef(SClrLightsteelblue), 0);
	//
	LastGrpListUpdTime = getcurdatetime_();
	P_PalmWaiter = 0;
	P_UhttImporter = 0;
	//
	CnSleepTimeout = 0;
	AutoInputTolerance = 5;
	IdleClock = clock();
	PrintCheckClock = 0;
	ClearCDYTimeout = 0;
	P_CDY    = 0;
	P_BNKTERM = 0;
	P_PhnSvcClient = 0;
	if(!(Flags & fNoEdit)) {
		P_PalmWaiter = new PalmImportWaiter(CheckPaneDialog::PalmImport, this); // @newok
		// @v8.1.6 {
		{
			UserInterfaceSettings uis;
			if(uis.Restore() > 0 && uis.Flags & UserInterfaceSettings::fDisableBeep)
				EnableBeep(0);
		}
		// } @v8.1.6
		if(CnFlags & CASHF_SYNC) {
			PPIniFile ini_file;
			PPSyncCashNode  scn;
			if(CnObj.GetSync(CashNodeID, &scn) > 0) {
				P_CDY = GetCustDisp(scn.CustDispType, scn.CustDispPort, scn.CustDispFlags);
				ClearCDYTimeout = scn.ClearCDYTimeout * CLOCKS_PER_SEC;
				CnSleepTimeout  = scn.SleepTimeout * CLOCKS_PER_SEC;
				P_BNKTERM       = GetBnkTerm(scn.BnkTermType, scn.BnkTermLogNum, scn.BnkTermPort, scn.BnkTermPath);
				TouchScreenID   = NZOR(scn.LocalTouchScrID, scn.TouchScreenID);
				ExtCashNodeID   = scn.ExtCashNodeID;
				ScaleID         = scn.ScaleID;
				// @v8.6.12 ���������� � CPosProcessor Scf             = scn.Scf;
				BonusMaxPart    = (scn.BonusMaxPart > 0 && scn.BonusMaxPart <= 1000) ? R3(((double)scn.BonusMaxPart) / 1000.0) : 1.0;
				scn.GetRoundParam(&R);
				SETFLAG(Flags, fSelSerial, scn.ExtFlags & CASHFX_SELSERIALBYGOODS);
				SETFLAG(Flags, fForceDivision, scn.ExtFlags & CASHFX_FORCEDIVISION);
				if(ExtCashNodeID) {
					if(CnObj.GetSync(ExtCashNodeID, &scn) > 0)
						ExtCnLocID = scn.LocID;
					// @v8.2.1 {
					P_GTOA = DS.CheckExtFlag(ECF_CHKPAN_USEGDSLOCASSOC) ?
						new GoodsToObjAssoc(PPASS_GOODS2LOC, PPOBJ_LOCATION) : new GoodsToObjAssoc(PPASS_GOODS2CASHNODE, PPOBJ_CASHNODE);
					CALLPTRMEMB(P_GTOA, Load());
					// } @v8.2.1
				}
				TableSelWhatman = scn.TableSelWhatman;
				{
					// @v7.8.1 {
					/* @v7.8.1 @construction {
					scn.GetPropString(SCN_KITCHENBELL_CMD, temp_buf);
					KitchenBellCmd = temp_buf.NotEmptyS() ? temp_buf : "1B70000505";
					scn.GetPropString(SCN_KITCHENBELL_PORT, temp_buf);
					KitchenBellPort = temp_buf.Strip();
					} @v7.8.1 */
					ini_file.Get(PPINISECT_CONFIG, PPINIPARAM_KITCHENBELLCMD, temp_buf);
					KitchenBellCmd = temp_buf.NotEmptyS() ? temp_buf : "1B70000505";
					int    is_err = 0;
					if(KitchenBellCmd.Len() % 2 != 0)
						is_err = 1;
					else {
						for(uint i = 0; i < KitchenBellCmd.Len(); i++) {
							if(!ishex(KitchenBellCmd.C(i))) {
								is_err = 1;
								break; // @error
							}
						}
					}
					if(is_err) {
						KitchenBellCmd = 0;
						PPSetError(PPERR_INVKITCHENBELLCMD, KitchenBellCmd);
						PPLogMessage(PPFILNAM_ERR_LOG, 0, LOGMSGF_LASTERR|LOGMSGF_TIME|LOGMSGF_USER);
					}
				}
				{
					int    ait = 0;
					ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_AUTOINPUTTOLERANCE, &ait);
					if(ait > 0 && ait <= 100)
						AutoInputTolerance = ait;
				}
				if(scn.PhnSvcID) {
					PPObjPhoneService ps_obj(0);
					PPPhoneServicePacket ps_pack;
					if(ps_obj.GetPacket(scn.PhnSvcID, &ps_pack) > 0) {
						SString addr_buf, user_buf, secret_buf, temp_buf;
						int    port = 0;
						ps_pack.GetExField(PHNSVCEXSTR_ADDR, addr_buf);
						ps_pack.GetExField(PHNSVCEXSTR_PORT, temp_buf);
						port = temp_buf.ToLong();
						ps_pack.GetExField(PHNSVCEXSTR_USER, user_buf);
						ps_pack.GetPassword(secret_buf);
						AsteriskAmiClient * p_client = new AsteriskAmiClient;
						if(p_client) {
							if(p_client->Connect(addr_buf, port)) {
								if(p_client->Login(user_buf, secret_buf)) {
									P_PhnSvcClient = p_client;
									p_client = 0;
									PhnSvcLocalChannelSymb = ps_pack.LocalChannelSymb;
								}
								else {
									// @error
								}
							}
							else {
								// @error
							}
						}
						delete p_client; // ���� ���������� � �������� ������ �������, �� p_client == 0, � P_PhnSvcClient != 0
					}
				}
				// @v8.3.2 {
				if(scn.ExtFlags & CASHFX_UHTTORDIMPORT) {
					PPAlbatrosConfig acfg;
					PPAlbatrosCfgMngr::Get(&acfg);
					if(acfg.UhttAccount.NotEmpty() && acfg.Hdr.OpID) {
						P_UhttImporter = new PPBillImporter;
					}
				}
				// } @v8.3.2
			}
			CDispCommand(cdispcmdText, cdisptxtOpened, 0.0, 0.0);
			if(Flags & fTouchScreen && TouchScreenID) {
				PPTouchScreenPacket ts_pack;
				PPObjTouchScreen    ts_obj;
				if(ts_obj.GetPacket(TouchScreenID, &ts_pack) > 0) {
					LOGFONT log_font; // @unicodeproblem
					int    r = 1; // 0-->1
					ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_TSGOODSGROUPSASBUTTONS, &r);
					SETFLAG(UiFlags, uifTSGGroupsAsButtons, r);
					ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_TSGGROUPLISTFLAT, &(r = 0));
					SETFLAG(UiFlags, uifOneGroupLevel, r);
					SelGoodsGrpID = AltGoodsGrpID = ts_pack.Rec.AltGdsGrpID;
					InitGroupList(ts_pack);
					MEMSZERO(log_font);
					log_font.lfCharSet = DEFAULT_CHARSET;
					GoodsListEntryGap = ts_pack.Rec.GdsListEntryGap;
					if(ts_pack.Rec.GdsListFontName[0]) {
						HDC    dc = GetDC(0);
						int    cy = GetDeviceCaps(dc, LOGPIXELSY);
						int    height = labs(ts_pack.Rec.GdsListFontHight) * 72 / cy - ((UiFlags & uifTSGGroupsAsButtons) ? TSGGROUPSASITEMS_FONTDELTA : 0);
						ReleaseDC(0, dc);
						log_font.lfHeight = height;
						STRNSCPY(log_font.lfFaceName, ts_pack.Rec.GdsListFontName); // @unicodeproblem
					}
					else {
						PPGetSubStr(PPTXT_FONTFACE, PPFONT_MSSANSSERIF, temp_buf);
						STRNSCPY(log_font.lfFaceName, temp_buf); // @unicodeproblem
						log_font.lfHeight = (UiFlags & uifTSGGroupsAsButtons) ? (DEFAULT_TS_FONTSIZE - TSGGROUPSASITEMS_FONTDELTA) : DEFAULT_TS_FONTSIZE;
					}
					GoodsListFontHeight = log_font.lfHeight + GoodsListEntryGap + TSGGROUPSASITEMS_FONTDELTA;
					Ptb.SetFont(fontGoodsList, ::CreateFontIndirect(&log_font)); // @unicodeproblem
					SETFLAG(Flags, fPrintSlipDoc, ts_pack.Rec.Flags & TSF_PRINTSLIPDOC);
				}
			}
			//setTitle(PPGetWord(PPWORD_CASHNODE, 0, temp_buf).CatDiv(':', 2).Cat(CnName));
			PPLoadString("posnode", temp_buf);
			setTitle(temp_buf.CatDiv(':', 2).Cat(CnName));
		}
		PPGetSubStr(PPTXT_FONTFACE, PPFONT_IMPACT, font_face);
		SetCtrlFont(CTL_CHKPAN_TOTAL, font_face, 54);
		SetCtrlFont(CTL_CHKPAN_INFO,  font_face, 20);
		SetCtrlFont(CTL_CHKPAN_CAFE_STATUS, font_face, 20);
		DefInputLine = CTL_CHKPAN_INPUT;
	}
	SetupExt(P_ChkPack);
	P_EGSDlg = 0;
	disableCtrl(CTL_CHKPAN_INPUT, Flags & fNoEdit);
	showCtrl(STDCTL_ALLBUTTON, Flags & fAsSelector);
	showCtrl(STDCTL_SYSINFOBUTTON, !(Flags & fAsSelector)); // @v8.2.8
	showCtrl(STDCTL_OKBUTTON,  Flags & fAsSelector);
	showCtrl(STDCTL_PRINT,    !(Flags & fAsSelector));
	setCtrlOption(CTL_CHKPAN_INPUT, ofFramed, 1);
	setupHint();
	SetupInfo(0);
	if(!SetupStrListBox(this, CTL_CHKPAN_LIST) || !LoadCheck(P_ChkPack, 0))
		PPError();
	if(Flags & fTouchScreen) {
		SETFLAG(CnFlags, CASHF_NOASKPAYMTYPE, !(CsObj.GetEqCfg().Flags & PPEquipConfig::fUnifiedPayment));
		SmartListBox * p_list = (SmartListBox *)getCtrlView(CTL_CHKPAN_GDSLIST);
		if(p_list)
		CALLPTRMEMB(p_list, SetOwnerDrawState());
		SmartListBox * p_grp_list = (SmartListBox *)getCtrlView(CTL_CHKPAN_GRPLIST);
		if(p_grp_list) {
			p_grp_list->SetOwnerDrawState();
			SetupStrListBox(p_grp_list);
		}
		PPGetSubStr(PPTXT_FONTFACE, PPFONT_MSSANSSERIF, font_face);
		SetCtrlFont(CTL_CHKPAN_INPUT, font_face, 20);
		enableCommand(cmaAltSelect, BIN(AltGoodsGrpID));
		// @v8.8.3 selectCtrl(CTL_CHKPAN_GRPLIST);
		selectCtrl((Flags & fNoEdit) ? CTL_CHKPAN_GRPLIST : CTL_CHKPAN_INPUT); // @v8.8.3
		// @v9.1.1 DS.SetLCfgFlags(DS.GetTLA().Lc.Flags | CCFLG_USELARGEDIALOG);
		SLS.SetUiFlag(sluifUseLargeDialogs, 1); // @v9.1.1
	}
	else
		selectCtrl((Flags & fNoEdit) ? CTL_CHKPAN_LIST : CTL_CHKPAN_INPUT);
	TView::message(this, evCommand, cmSetupTooltip);
	if(CnSpeciality == PPCashNode::spCafe) {
		setButtonBitmap(cmChkPanF2, IDB_GUESTS);
		setButtonBitmap(cmChkPanF1, IDB_TABLE_ORDERS);
	}
	else if(CnSpeciality == PPCashNode::spDelivery) {
		setButtonBitmap(cmChkPanF1, IDB_DELIVERY);
	}
	LastCtrlID = 0;
}

CheckPaneDialog::~CheckPaneDialog()
{
	// @v9.1.1 DS.SetLCfgFlags(DS.GetTLA().Lc.Flags & ~CCFLG_USELARGEDIALOG);
	SLS.SetUiFlag(sluifUseLargeDialogs, 0); // @v9.1.1
	DS.GetTLA().AddedMsgStrNoRights = 0;
	for(int i = 0; i < 32; i++)
		MEMSZERO(OwnerDrawCtrls[i]);
	delete P_EGSDlg;
	{
		CDispCommand(cdispcmdText, cdisptxtClosed, 0.0, 0.0);
		ZDELETE(P_CDY);
	}
	delete P_BNKTERM;
	delete P_PalmWaiter;
	delete P_PhnSvcClient;
	delete P_UhttImporter; // @v8.3.2
}

int CheckPaneDialog::SetupState(int st)
{
	int    ok = CPosProcessor::SetupState(st);
	if(ok > 0)
		showButton(cmSelModifier, oneof3(State_p, sEMPTYLIST_BUF, sLIST_BUF, sLISTSEL_BUF));
	return ok;
}

int CheckPaneDialog::EnableBeep(int enbl)
{
	SETFLAG(Flags, fDisableBeep, !enbl);
	return 1;
}

int FASTCALL CheckPaneDialog::valid(ushort command)
{
	int    r = 1;
	Flags |= fSuspSleepTimeout;
	const int prev_state = GetState();
	const PPID prev_agent_id = P.GetAgentID(1); // @v8.2.0
	if(command == cmCancel && !(Flags & fNoEdit)) {
		r = 0;
		if(GetInput())
			ClearInput(0);
		else if(ResetCurrentLine() > 0)
			;
		else if((Flags & fTouchScreen) && LastCtrlID == CTL_CHKPAN_LIST)
			RemoveRow();
		else if(P.getCount()) {
			if(OperRightsFlags & orfEscCheck) {
				if(PPMessage(mfConf|mfYesNo, PPCFM_CLEARCHECK, 0) == cmYes) {
					AcceptCheckToBeCleared();
					ClearCheck();
					if(CConfig.Flags & CCFLG_DEBUG) {
						CCheckPacket pack;
						if(SuspCheckID)
							CC.LoadPacket(SuspCheckID, 0, &pack);
						else {
							InitCashMachine();
							pack.Rec.SessID = P_CM ? P_CM->GetCurSessID() : 0;
							pack.Rec.CashID = CashNodeID;
							pack.Rec.Flags |= (CCHKF_SYNC | CCHKF_NOTUSED);
							Helper_InitCcPacket(&pack, 0, 0, 0);
						}
						CC.WriteCCheckLogFile(&pack, 0, CCheckCore::logCleared, 1);
					}
				}
			}
			else
				PPMessage(mfInfo|mfOK, PPINF_CHKPAN_TURNCHECK, 0);
		}
		else if(P.OrderCheckID) {
			ClearCheck();
		}
		else if((UiFlags & uifCloseWOAsk) || PPMessage(mfConf|mfYesNo, PPCFM_CLOSECCHKPANE, 0) == cmYes)
			r = 1;
	}
	else
		r = TDialog::valid(command);
	if(GetState() != prev_state || P.GetAgentID(1) != prev_agent_id)
		setupHint();
	Flags &= ~fSuspSleepTimeout;
	IdleClock = clock();
	return r;
}

// virtual
int CheckPaneDialog::AcceptCheck(const CcAmountList * pPl, double cash, int mode /* accmXXX */)
{
	int    ok = CPosProcessor::AcceptCheck(pPl, cash, mode /*suspended*/);
	if(!oneof2(mode, accmJunk, accmAveragePrinting)) {
		if(ClearCDYTimeout)
			PrintCheckClock = clock();
		else
			CDispCommand(cdispcmdText, cdisptxtOpened, 0.0, 0.0);
		if(ok > 0 && UiFlags & uifOnce)
			TView::message(this, evCommand, cmCancel);
	}
	return ok;
}

int CheckPaneDialog::SuspendCheck()
{
	int    ok = -1;
	const int  prev_state = GetState();
	const PPID prev_agent_id = P.GetAgentID(1); // @v8.2.0
	if(IsState(sEMPTYLIST_EMPTYBUF)) {
		SelectSuspendedCheck();
	}
	else if(IsState(sLIST_EMPTYBUF)) {
		if(OperRightsFlags & orfSuspCheck) {
			double total = 0.0, discount = 0.0;
			CalcTotal(&total, &discount);
			//
			CDispCommand(cdispcmdClear, 0, 0.0, 0.0);
			CDispCommand(cdispcmdTotal, 0, total, 0.0);
			if(discount != 0.0)
				CDispCommand(cdispcmdTotalDiscount, 0, (discount * 100.0) / (total + discount), discount);
			//
			AcceptCheck(0, total, accmSuspended);
			ok = 1;
		}
	}
	if(GetState() != prev_state || P.GetAgentID(1) != prev_agent_id)
		setupHint();
	return ok;
}

int CheckPaneDialog::IsCode(const SString & rInput, SString & rPfx, int asterix, SString & rCode) const
{
	if(asterix)
		rPfx.PadLeft(1, '*');
	if(rInput.CmpPrefix(rPfx, 1) == 0) {
		(rCode = rInput).ShiftLeft(rPfx.Len());
		if(asterix)
			rCode.TrimRightChr('*');
		return 1;
	}
	else
		return 0;
}

int CheckPaneDialog::IsSalCode(const SString & rInput, SString & rCode)
{
	int    ok = 0;
	int    asterix = 0;
	SString pfx;
	const PPEquipConfig & r_cfg = CsObj.GetEqCfg();
	if(r_cfg.AgentPrefix[0] && strlen(r_cfg.AgentPrefix) < (uint)r_cfg.AgentCodeLen) {
		pfx = r_cfg.AgentPrefix;
		do {
			if(IsCode(rInput, pfx, asterix, rCode))
				ok = 1;
		} while(!ok && ++asterix < 2);
	}
	else {
		do {
			if(IsCode(rInput, pfx = "SAL", asterix, rCode))
				ok = 1;
			else if(IsCode(rInput, pfx = "���", asterix, rCode))
				ok = 1;
			else if(IsCode(rInput, (pfx = "���").ToOem(), asterix, rCode))
				ok = 1;
		} while(!ok && ++asterix < 2);
	}
	return ok;
}

int CheckPaneDialog::SetupSalByCode(const SString & rInput)
{
	int    ok = -1;
	SString code;
	if(IsSalCode(rInput, code)) {
		if(code.ToLong() == 0) {
			SetupAgent(0, 0);
			SetupInfo(0);
			ok = 2;
		}
		else {
			PPID   ar_id = 0, reg_type_id = 0;
			PPID   acs_id = GetAgentAccSheet();
			if(acs_id && code.NotEmpty() && PPObjArticle::GetSearchingRegTypeID(acs_id, 0, 0, &reg_type_id) > 0)
				ArObj.SearchByRegCode(acs_id, reg_type_id, code, &ar_id, 0);
			if(ar_id) {
				SetupAgent(ar_id, 0);
				SetupInfo(0);
				ok = 1;
			}
			else
				ok = MessageError(PPERR_ARCODENFOUND, Input, eomMsgWindow);
		}
	}
	return ok;
}

void CheckPaneDialog::AddFromBasket()
{
	int    ok = 1, is_locked = 0;
	PPBasketCombine bc;
	if((ok = GoodsBasketDialog(bc, 2)) > 0 && bc.BasketID) {
		ILTI * p_item;
		for(uint i = 0; bc.Pack.Lots.enumItems(&i, (void**)&p_item);) {
			PgsBlock pgsb(p_item->Quantity);
			SetupNewRow(p_item->GoodsID, pgsb);
		}
		AcceptRow();
	}
}

void CheckPaneDialog::ViewStoragePlaces(PPID goodsId)
{
	if(CheckID <= 0 && CnSpeciality == PPCashNode::spApteka) {
		PPID   goods_id = goodsId ? goodsId : (P.HasCur() ? P.GetCur().GoodsID : 0);
		if(goods_id) {
			PPID   assc = PPASS_GOODS2WAREPLACE;
			SString out_msg, loc_name, tag_name;
			GoodsToObjAssoc gtoa(assc, PPOBJ_LOCATION);
			if(gtoa.IsValid() && gtoa.Load()) {
				PPID   loc_id = 0;
				if(gtoa.Get(goods_id, &loc_id) > 0 && loc_id) {
					LocationTbl::Rec loc_rec;
					if(PsnObj.LocObj.Search(loc_id, &loc_rec) > 0)
						loc_name = loc_rec.Name;
				}
			}
			{
				const ObjTagItem * p_item = 0;
				int    found = 0;
				ObjTagList tag_list;
				PPObjTag obj_tag;
				GObj.GetTagList(goods_id, &tag_list);
				for(uint pos = 0; !found && (p_item = tag_list.EnumItems(&pos));) {
					PPObjTagPacket tag_pack;
					if(obj_tag.GetPacket(p_item->TagID, &tag_pack) > 0 && tag_pack.Rec.Flags & OTF_NOTICEINCASHPANE) {
						tag_name = tag_pack.Rec.Name;
						found = 1;
					}
				}
			}
			if(tag_name.Len())
				(out_msg = tag_name).CR().CR();
			if(loc_name.Len()) {
				SString buf;
				PPLoadString("storageplace", buf);
				buf.CatChar(':');
				out_msg.Cat(buf).CR();
				out_msg.Cat(loc_name);
			}
			if(out_msg.Len()) {
				PPTooltipMessage(out_msg, 0, H(), 20000, GetColorRef(SClrCyan),
					SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus|SMessageWindow::fLargeText);
			}
			else
				SMessageWindow::DestroyByParent(H());
		}
	}
}

int CheckPaneDialog::SetupOrder(PPID ordCheckID)
{
	int    ok = 1;
	if(ordCheckID == 0) {
		P.OrderCheckID = 0;
	}
	else {
		SString cc_text;
		CCheckPacket cc_pack;
		THROW(CC.LoadPacket(ordCheckID, 0, &cc_pack) > 0);
		CCheckCore::MakeCodeString(&cc_pack.Rec, cc_text);
		THROW_PP_S(cc_pack.Rec.Flags & CCHKF_ORDER, PPERR_CCHKNORDER, cc_text);
		THROW_PP_S(!(cc_pack.Rec.Flags & CCHKF_SKIP), PPERR_CCHKORDCANCELED, cc_text);
		THROW_PP_S(!(cc_pack.Rec.Flags & CCHKF_CLOSEDORDER), PPERR_CCHKORDCLOSED, cc_text);
		P.OrderCheckID = cc_pack.Rec.ID;
		P.TableCode = cc_pack.Ext.TableNo;
		if(cc_pack.Rec.SCardID)
			AcceptSCard(0, cc_pack.Rec.SCardID, 1 /* ignoreRights */);
	}
	SetupInfo(0);
	CATCH
		ok = MessageError(PPErrCode, 0, eomStatusLine|eomBeep);
	ENDCATCH
	return ok;
}

int CPosProcessor::CalculatePaymentList(PosPaymentBlock & rBlk, int interactive)
{
	int    ok = 1;
	const  double add_paym_epsilon = 0.0099;
	const  int unified_paym_interface = BIN(CsObj.GetEqCfg().Flags & PPEquipConfig::fUnifiedPayment);
	double non_crd_amt = 0.0;
	const  double credit_charge = CalcCreditCharge(0, 0, 0, &non_crd_amt, 0);
	/* @v8.2.2 const*/ double addpaym_r2 = R2(CSt.AdditionalPayment);
	uint   v = 0;
	double diff = 0.0;
	rBlk.Init(this);
	if(Flags & fSCardCredit && !(Flags & fSCardBonus) && CSt.GetID() && addpaym_r2 <= add_paym_epsilon) {
		if(unified_paym_interface && credit_charge > 0.0)
			rBlk.Kind = cpmUndef;
		else
			rBlk.Kind = cpmIncorpCrd;
	}
	else if(Flags & fBankingPayment && OperRightsFlags & orfBanking) {
		rBlk.Kind = cpmBank;
	}
	else if(!(OperRightsFlags & orfBanking) || (CnFlags & CASHF_NOASKPAYMTYPE)) {
		rBlk.Kind = cpmCash;
	}
	else if(unified_paym_interface) {
		rBlk.Kind = cpmUndef;
	}
	else if(interactive) {
		if(SelectorDialog(DLG_CHKPAYM, CTL_CHKPAYM_METHOD, &v) > 0) {
			if(v == 0)
				rBlk.Kind = cpmCash;
			else if(v == 1)
				rBlk.Kind = cpmBank;
		}
		else
			ok = -1;
	}
	else
		rBlk.Kind = cpmCash;
	if(ok > 0) {
		assert(oneof4(rBlk.Kind, cpmUndef, cpmCash, cpmBank, cpmIncorpCrd));
		if(F(fRetCheck) && Rb.AmL.GetTotal() != 0.0) {
			//
			// ���� ������� �������������� �� ���� �� �������� �������, �� ������������ ������
			// �� ����� �������� � (������� ����) ���������� ��� �������.
			//
			rBlk.CcPl = Rb.AmL;
			rBlk.CcPl.InvertSign();
			rBlk.CcPl.ScaleTo(-fabs(rBlk.GetTotal()));
			rBlk.Kind = cpmUndef;
		}
		else if(F(fRetByCredit)) {
			if(addpaym_r2 > add_paym_epsilon) {
				rBlk.AmtToPaym = R2(rBlk.GetTotal());
				assert(CSt.GetID() != 0); // ���� ����� �� �����������, �� ������� � ��� ����� ���� ����������
				if(rBlk.Kind == cpmBank) {
					rBlk.CcPl.Add(CCAMTTYP_BANK, rBlk.AmtToPaym);
				}
				else if(rBlk.Kind == cpmIncorpCrd) {
					// ��� ����������������� ���� ������ cmpIncorpCrd �� �� ������ ������� � ��� �����
					assert(rBlk.Kind != cpmIncorpCrd);
				}
				else {
					assert(oneof2(rBlk.Kind, cpmCash, cpmUndef)); // ������ �������� ���������
					rBlk.CcPl.Add(CCAMTTYP_CASH, rBlk.AmtToPaym);
				}
				rBlk.CcPl.Add(CCAMTTYP_CRDCARD, rBlk.GetTotal() - addpaym_r2, CSt.GetID());
			}
			else {
				rBlk.AmtToPaym = addpaym_r2;
				if(rBlk.Kind == cpmIncorpCrd) {
					assert(Flags & fSCardCredit && !(Flags & fSCardBonus) && CSt.GetID());
					rBlk.CcPl.Add(CCAMTTYP_CRDCARD, rBlk.GetTotal(), CSt.GetID());
				}
				else {
					if(rBlk.Kind == cpmBank) {
						rBlk.CcPl.Add(CCAMTTYP_BANK, rBlk.AmtToPaym);
					}
					else {
						assert(oneof2(rBlk.Kind, cpmCash, cpmUndef)); // ������ �������� ���������
						rBlk.CcPl.Add(CCAMTTYP_CASH, rBlk.AmtToPaym);
					}
					rBlk.CcPl.Add(CCAMTTYP_CRDCARD, rBlk.GetTotal() - rBlk.AmtToPaym, CSt.GetID());
				}
			}
		}
		else {
			if(fabs(addpaym_r2) > add_paym_epsilon) {
				// @v8.2.2 {
				if(addpaym_r2 > fabs(rBlk.GetTotal())) { // @v8.5.5 �������������� ��� ������ ��������
					MessageError(PPERR_CHKPAN_ADDPAYMABOVEAMT, 0, eomBeep|eomPopup);
					addpaym_r2 = rBlk.GetTotal() - rBlk.GetUsableBonus();
				}
				// } @v8.2.2
				rBlk.AmtToPaym = addpaym_r2;
				assert(CSt.GetID() != 0); // ���� ����� �� �����������, �� ������� � ��� ����� ���� ����������
				if(rBlk.Kind == cpmBank) {
					rBlk.CcPl.Add(CCAMTTYP_BANK, rBlk.AmtToPaym);
				}
				else if(rBlk.Kind == cpmIncorpCrd) {
					// ��� ����������������� ���� ������ cmpIncorpCrd �� �� ������ ������� � ��� �����
					assert(rBlk.Kind != cpmIncorpCrd);
				}
				else {
					assert(oneof2(rBlk.Kind, cpmCash, cpmUndef)); // ������ �������� ���������
					rBlk.CcPl.Add(CCAMTTYP_CASH, rBlk.AmtToPaym);
				}
				rBlk.CcPl.Add(CCAMTTYP_CRDCARD, rBlk.GetTotal() - rBlk.AmtToPaym, CSt.GetID());
			}
			else {
				rBlk.AmtToPaym = R2(rBlk.GetTotal() - rBlk.GetUsableBonus());
				if(rBlk.Kind == cpmBank) {
					rBlk.CcPl.Add(CCAMTTYP_BANK, rBlk.AmtToPaym);
				}
				else if(rBlk.Kind == cpmIncorpCrd) {
					assert(Flags & fSCardCredit && !(Flags & fSCardBonus) && CSt.GetID());
					rBlk.CcPl.Add(CCAMTTYP_CRDCARD, rBlk.GetTotal(), CSt.GetID());
				}
				else {
					assert(oneof2(rBlk.Kind, cpmCash, cpmUndef)); // ������ �������� ���������
					rBlk.CcPl.Add(CCAMTTYP_CASH, rBlk.AmtToPaym);
				}
				if(rBlk.GetUsableBonus() != 0.0) {
					rBlk.CcPl.Add(CCAMTTYP_CRDCARD, rBlk.GetUsableBonus(), CSt.GetID());
				}
			}
			assert(rBlk.CcPl.GetTotal() == rBlk.GetTotal());
		}
	}
	return ok;
}

int CheckPaneDialog::ConfirmPosPaymBank(double amount)
{
	int    yes = 1;
	if(amount != 0.0) {
		TDialog * dlg = new TDialog(DLG_POSPAYMBNK);
		if(CheckDialogPtr(&dlg, 1)) {
			dlg->setCtrlReal(CTL_POSPAYMBNK_AMOUNT, amount);
			if(ExecViewAndDestroy(dlg) != cmOK)
				yes = 0;
		}
	}
	return yes;
}

int CPosProcessor::RecognizeCode(int mode, const char * pCode, int autoInput)
{
	int    ok = -1;
	if(!isempty(pCode)) {
		int    try_next = 1;
		PPID   ar_id = 0;
		PPID   reg_type_id = 0;
		PPID   acs_id = GetAgentAccSheet();
		SString ss_code;
		if(oneof2(mode, crmodeAuto, crmodeAgent)) {
			if(acs_id && PPObjArticle::GetSearchingRegTypeID(acs_id, 0, 0, &reg_type_id) > 0)
				ArObj.SearchByRegCode(acs_id, reg_type_id, pCode, &ar_id, 0);
			if(ar_id) {
				SetupAgent(ar_id, BIN(mode == crmodeAgent)); // asAuthAgent=1 ��� ��������. ������ ����������� ������,
					// ��� ������� ���������� ������ ��������� ������� � � ���� ������ ��� ���������.
				SetupInfo(0);
				try_next = 0;
				ok = 1;
			}
			else if(mode == crmodeAgent) {
				try_next = 0;
				ok = 0; // @error
			}
		}
		if(try_next && oneof2(mode, crmodeAuto, crmodeSCard)) {
			//
			// ����� �����
			//
			if(PPObjSCard::PreprocessSCardCode(ss_code = pCode) > 0) {
				char   card_code[64];
				ss_code.CopyTo(card_code, sizeof(card_code));
				if(card_code[0]) {
					SCardTbl::Rec sc_rec;
					if(ScObj.SearchCode(0, card_code, &sc_rec) > 0) {
						if(autoInput || !(CsObj.GetEqCfg().Flags & PPEquipConfig::fDisableManualSCardInput))
							ok = Backend_AcceptSCard(sc_rec.ID, 0);
						else
							ok = MessageError(PPERR_MANUALSCARDINPUTDISABLED, 0, eomBeep|eomStatusLine);
						try_next = 0;
					}
					else if(mode == crmodeSCard) {
						try_next = 0;
						ok = MessageError(PPERR_SCARDNOTFOUND, pCode, eomBeep|eomStatusLine);
					}
				}
			}
			else
				ok = MessageError(PPERR_GDSBYBARCODENFOUND, pCode, eomBeep|eomStatusLine);
		}
		if(try_next && oneof2(mode, crmodeAuto, crmodeGoods)) {
			ok = 0;
		}
	}
	return ok;
}

void CheckPaneDialog::ProcessEnter(int selectInput)
{
	const int  prev_state = GetState();
	const PPID prev_agent_id = P.GetAgentID(1); // @v8.2.0
	SString temp_buf;
	if(selectInput)
		selectCtrl(CTL_CHKPAN_INPUT);
	if(isCurrCtlID(CTL_CHKPAN_INPUT)) {
		if(Flags & fWaitOnSCard)
			AcceptSCard(1, 0);
		else if(Flags & fSelByPrice)
			SelectGoods__(sgmByPrice);
		else if(GetInput()) {
			const int auto_input = BIN(UiFlags & uifAutoInput);
			SString ss_code;
			CCheckPacket::BarcodeIdentStruc bis;
			if(Input.CmpPrefix("TBL", 1) == 0) {
				int    table_no = Input.ShiftLeft(3).ToLong();
				long   guest_count = 0;
				if(CnExtFlags & CASHFX_INPGUESTCFTBL)
					SelectGuestCount(table_no, &guest_count);
				SetupCTable(table_no, guest_count);
				ClearInput(0);
			}
			else if(SetupSalByCode(Input) >= 0)
				ClearInput(0);
			else if(Input.CmpNC("SUS00") == 0)
				SuspendCheck();
			else if(Input.CmpNC("SUS01") == 0)
				SelectSuspendedCheck();
			else if(CCheckPacket::ParseBarcodeIdent(Input, &bis)) {
				if(IsState(sEMPTYLIST_EMPTYBUF)) {
					if(OperRightsFlags & orfRestoreSuspWithoutAgent || P.GetAgentID()) {
						if(bis.PosId == CashNodeID) {
							uint   candid_count = 0;
							LDATETIME max_dtm;
							uint   _pos = 0;
							max_dtm.SetZero();
							TSArray <CCheckTbl::Rec> cc_list;
							if(CC.GetListByCode(bis.PosId, bis.CcCode, &cc_list) > 0) {
                                for(uint i = 0; i < cc_list.getCount(); i++) {
                                	const CCheckTbl::Rec & r_rec = cc_list.at(i);
                                	assert(r_rec.CashID == CashNodeID && r_rec.Code == bis.CcCode);
                                    if(r_rec.CashID == CashNodeID && r_rec.Code == bis.CcCode) { // @paranoic
                                        if(r_rec.Flags & CCHKF_SUSPENDED) {
                                        	candid_count++;
											LDATETIME cc_dtm;
											cc_dtm.Set(r_rec.Dt, r_rec.Tm);
											if(cmp(cc_dtm, max_dtm) > 0) {
												max_dtm = cc_dtm;
												_pos = i+1;
											}
                                        }
                                    }
                                }
                                if(_pos > 0) {
									assert(_pos <= cc_list.getCount());
									assert(candid_count > 0);
									if(candid_count > 1) {
									}
									if(!RestoreSuspendedCheck(cc_list.at(_pos-1).ID)) {
										MessageError(-1, 0, eomBeep|eomStatusLine);
									}
                                }
                                else {
									(temp_buf = 0).Cat(bis.PosId).Space().Cat(bis.CcCode);
									MessageError(PPERR_CHKPAN_SUSPCHKNFOUND, temp_buf, eomBeep|eomStatusLine);
                                }
							}
						}
					}
					else
						MessageError(PPERR_NORIGHTSELSUSPCHECK, 0, eomBeep|eomStatusLine);
				}
				else
					MessageError(PPERR_CHKPAN_CANTRESTORESUSPNE, 0, eomBeep|eomStatusLine);
			}
			else if(Input.CmpPrefix("DIV", 1) == 0) {
				int    div = Input.ShiftLeft(3).ToLong();
				if(P.HasCur() && P.GetCur().GoodsID && (div > 0 && div < 1000)) {
					P.GetCur().Division = (int16)div;
					SetupInfo(0);
				}
				ClearInput(0);
			}
			else if(Input.CmpPrefix("TSN", 1) == 0) {
				PPID   sess_id = Input.ShiftLeft(3).ToLong();
				LoadTSession(sess_id);
				ClearInput(0);
			}
			else if(Input.CmpPrefix("CHKINP", 1) == 0) {
				PPID   cip_id = 0;
				PPID   goods_id = 0;
				double qtty = 0.0;

				StringSet ss;
				temp_buf = Input;
				temp_buf.Tokenize(":", ss);
				uint p = 0;
				if(ss.get(&p, temp_buf)) {
					cip_id = temp_buf.ShiftLeft(6).ToLong();
					if(ss.get(&p, temp_buf)) {
						goods_id = temp_buf.ToLong();
						if(ss.get(&p, temp_buf))
							qtty = temp_buf.ToReal();
					}
				}
				if(qtty <= 0.0)
					qtty = 1.0;
				if(cip_id && goods_id) {
					LoadChkInP(cip_id, goods_id, qtty);
				}
				ClearInput(0);
			}
			else if(Input.Cmp("99999", 0) == 0) {
				Sleep();
				ClearInput(0);
			}
			else if(Input.C(0) == '*' || Input.Last() == '*')
				AcceptQuantity();
			else if(Input.CmpNC("$GENERATOR$") == 0)
				GenerateChecks();
			else if(Input.CmpNC("BASKET") == 0) {
				AddFromBasket();
				ClearInput(0);
			}
			else if(Input.CmpNC("TEST") == 0) {
				TestCheck(cpmBank);
				ClearInput(0);
			}
			else if(!oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
				Flags |= fSuspSleepTimeout;
				/* @v8.2.9
				if(CnFlags & CASHF_DISABLEZEROAGENT && !P.GetAgentID())
					MessageError(PPERR_CHKPAN_SALERNEEDED, 0, eomBeep|eomStatusLine);
				*/
				if(Flags & fPrinted && !(OperRightsFlags & orfChgPrintedCheck))
					MessageError(PPERR_NORIGHTS, 0, eomBeep|eomStatusLine);
				else {
					int    r = -1; // r == 1000 - �������� �� �������� ��-�� ������������ ������-�� ������� //
					char   code[128];
					int    is_serial = 0; // !0 ���� code �������� ���������� �������� �������
					double qtty = 1.0, price = 0.0;
					PPID   goods_id = 0, loc_id = 0;
					GoodsCodeSrchBlock gcsb;
					Input.CopyTo(gcsb.Code, sizeof(gcsb.Code));
					Input.CopyTo(code, sizeof(code));
					gcsb.Flags |= (GoodsCodeSrchBlock::fAdoptSearch | GoodsCodeSrchBlock::fUse2dTempl);
					if(GObj.SearchByCodeExt(&gcsb) > 0) {
						if(CnFlags & CASHF_DISABLEZEROAGENT && !P.GetAgentID()) {
							r = 1000;
							MessageError(PPERR_CHKPAN_SALERNEEDED, 0, eomBeep|eomStatusLine);
						}
						else {
							goods_id = gcsb.GoodsID;
							qtty = gcsb.Qtty;
							r = 1;
						}
					}
					else {
						PPObjBill * p_bobj = BillObj;
						PPID   lot_id = 0;
						ReceiptTbl::Rec lot_rec;
						PPIDArray  lot_list;
						if(p_bobj->SearchLotsBySerial(code, &lot_list) > 0) {
							if(CnFlags & CASHF_DISABLEZEROAGENT && !P.GetAgentID()) {
								r = 1000;
								MessageError(PPERR_CHKPAN_SALERNEEDED, 0, eomBeep|eomStatusLine);
							}
							else {
								if(ExtCashNodeID && ExtCnLocID) {
									if(p_bobj->SelectLotFromSerialList(&lot_list, ExtCnLocID, &lot_id, &lot_rec) > 0 &&
										BelongToExtCashNode(labs(lot_rec.GoodsID))) {
										goods_id = labs(lot_rec.GoodsID);
										price  = lot_rec.Price;
										loc_id = ExtCnLocID;
										is_serial = 1;
										r = 1;
									}
								}
								if(!goods_id && p_bobj->SelectLotFromSerialList(&lot_list, CnLocID, &lot_id, &lot_rec) > 0) {
									goods_id = labs(lot_rec.GoodsID);
									price  = lot_rec.Price;
									loc_id = CnLocID;
									is_serial = 1;
									r = 1;
								}
							}
						}
					}
					if(r > 0 && r != 1000) {
						PgsBlock pgsb(qtty);
						//SETIFZ(qtty, 1.0);
						//SString serial = is_serial ? code : 0;
						pgsb.PriceBySerial = price;
						pgsb.Serial = is_serial ? code : 0;
						if(PreprocessGoodsSelection(goods_id, loc_id, /*&qtty, serial, &price*/pgsb) > 0)
							SetupNewRow(goods_id, pgsb);
					}
					else if(CsObj.GetEqCfg().Flags & PPEquipConfig::fRecognizeCode) {
						PPID   ar_id = 0, reg_type_id = 0;
						PPID   acs_id = GetAgentAccSheet();
						if(acs_id && code[0] && PPObjArticle::GetSearchingRegTypeID(acs_id, 0, 0, &reg_type_id) > 0)
							ArObj.SearchByRegCode(acs_id, reg_type_id, code, &ar_id, 0);
						if(ar_id) {
							SetupAgent(ar_id, 0);
							SetupInfo(0);
						}
						//
						// ����� �����
						//
						else if(PPObjSCard::PreprocessSCardCode(ss_code = code) > 0) {
							char   card_code[64];
							ss_code.CopyTo(card_code, sizeof(card_code));
							if(card_code[0]) {
								SCardTbl::Rec sc_rec;
								if(ScObj.SearchCode(0, card_code, &sc_rec) > 0)
									if(auto_input || !(CsObj.GetEqCfg().Flags & PPEquipConfig::fDisableManualSCardInput))
										AcceptSCard(0, sc_rec.ID);
									else
										MessageError(PPERR_MANUALSCARDINPUTDISABLED, 0, eomBeep|eomStatusLine);
								else
									MessageError(PPERR_GDSBYBARCODENFOUND, code, eomBeep|eomStatusLine);
							}
						}
						else
							MessageError(PPERR_GDSBYBARCODENFOUND, code, eomBeep|eomStatusLine);
					}
					else
						MessageError(PPERR_GDSBYBARCODENFOUND, code, eomBeep|eomStatusLine);
				}
				Flags &= ~fSuspSleepTimeout;
				ClearInput(0);
			}
			else
				ClearInput(0);
		}
		else if(P.HasCur())
			AcceptRow();
		else if(P.getCount()) {
			//
			// ���������� � ������ ����
			//
			// @v8.3.4 {
			if(CnExtFlags & CASHFX_DISABLEZEROSCARD && !CSt.GetID())
				MessageError(PPERR_CHKPAN_SCARDNEEDED, 0, eomBeep|eomStatusLine);
			// } @v8.3.4
			else if(!(OperRightsFlags & orfPrintCheck))
				MessageError(PPERR_NORIGHTS, 0, eomBeep|eomStatusLine);
			else {
				double diff = 0.0;
				PosPaymentBlock paym_blk2(0, BonusMaxPart);
				if(CalculatePaymentList(paym_blk2, 1) > 0) {
					CDispCommand(cdispcmdClear, 0, 0.0, 0.0);
					CDispCommand(cdispcmdTotal, 0, paym_blk2.GetTotal(), 0.0);
					if(paym_blk2.GetDiscount() != 0.0)
						CDispCommand(cdispcmdTotalDiscount, 0, paym_blk2.GetPctDiscount(), paym_blk2.GetDiscount());
					switch(paym_blk2.Kind) {
						case cpmCash:
							if(CalcDiff(paym_blk2.AmtToPaym, &diff) > 0) {
								// @paul {
								SMessageWindow * p_win = new SMessageWindow;
								if(p_win) {
									SString msg_buf, words;
									PPLoadText(PPTXT_CUSTDISP_WORDS, words);
									PPGetSubStr(words, PPCDY_TOTAL, temp_buf);
									msg_buf.Cat(temp_buf).Space().Cat(paym_blk2.AmtToPaym, SFMT_MONEY).CR();
									PPGetSubStr(words, PPCDY_CASH, temp_buf);
									msg_buf.Cat(temp_buf).CatDiv(':', 2).Cat(paym_blk2.AmtToPaym+diff, SFMT_MONEY).CR();
									PPGetSubStr(words, PPCDY_CHANGE, temp_buf);
									msg_buf.Cat(temp_buf).CatDiv(':', 2).Cat(diff, SFMT_MONEY).CR();
									p_win->Open(msg_buf, 0, H(), 0, 10000, GetColorRef(SClrCyan),
										SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus|SMessageWindow::fLargeText, 0);
								}
								// } @paul
								CDispCommand(cdispcmdChange, 0, paym_blk2.AmtToPaym + diff, diff);
								AcceptCheck(&paym_blk2.CcPl, paym_blk2.AmtToPaym + diff, accmRegular);
							}
							break;
						case cpmBank:
							{
								// @vmiller comment
								/*if(yes)
									AcceptCheck(paym_method, amt_to_paym + diff, accmRegular);*/
								// @vmiller {
								if(ConfirmPosPaymBank(paym_blk2.AmtToPaym)) {
									if(P_BNKTERM) {
										int    r = (paym_blk2.AmtToPaym < 0) ? P_BNKTERM->Refund(-paym_blk2.AmtToPaym) : P_BNKTERM->Pay(paym_blk2.AmtToPaym);
										if(r)
											AcceptCheck(&paym_blk2.CcPl, paym_blk2.AmtToPaym + diff, accmRegular);
										else
											PPError();
									}
									else
										AcceptCheck(&paym_blk2.CcPl, paym_blk2.AmtToPaym + diff, accmRegular);
								}
								// } @vmiller
							}
							break;
						case cpmIncorpCrd:
							AcceptCheck(&paym_blk2.CcPl, paym_blk2.AmtToPaym + diff, accmRegular);
							break;
						case cpmUndef:
							{
								int    r = 1; // @vmiller
								paym_blk2.ExclSCardID = CSt.GetID();
								const double ccpl_total = paym_blk2.CcPl.GetTotal();
								for(int _again = 1; _again && paym_blk2.EditDialog2() > 0;) {
									assert(feqeps(paym_blk2.CcPl.GetTotal(), ccpl_total, 0.00001));
									assert(oneof3(paym_blk2.Kind, cpmCash, cpmBank, cpmIncorpCrd));
									if(CsObj.GetEqCfg().Flags & PPEquipConfig::fUnifiedPaymentCfmBank &&
										paym_blk2.CcPl.Get(CCAMTTYP_CASH) == 0.0 && !ConfirmPosPaymBank(paym_blk2.CcPl.Get(CCAMTTYP_BANK))) {
										_again = 1;
									}
									else {
										_again = 0;
										// @vmiller {
										if(P_BNKTERM) { // ����� �� �������� ��� ��������, ������ ��� ��� ��������� ������ � paym_blk2.Kind ����� ������ cpmCash
											for(uint i = 0; i < paym_blk2.CcPl.getCount(); i++) {
												if(paym_blk2.CcPl.at(i).Type == CCAMTTYP_BANK) {
													double bank_amt = paym_blk2.CcPl.at(i).Amount;
													if(bank_amt > 0) {
														r = P_BNKTERM->Pay(bank_amt);
													}
													else {
														r = P_BNKTERM->Refund(bank_amt);
													}
													if(!r)
														PPError();
													break;
												}
											}
										}
										// } @vmiller
										if(paym_blk2.NoteAmt > 0.0 && paym_blk2.DeliveryAmt > 0.0)
											CDispCommand(cdispcmdChange, 0, paym_blk2.NoteAmt, paym_blk2.DeliveryAmt);
										else
											CDispCommand(cdispcmdChange, 0, paym_blk2.Amount, 0.0);
										if(!P_BNKTERM || r)
											AcceptCheck(&paym_blk2.CcPl, paym_blk2.NoteAmt, accmRegular);
									}
								}
							}
							break;
					}
				}
			}
		}
	}
	if(GetState() != prev_state || P.GetAgentID(1) != prev_agent_id)
		setupHint();
}

int CheckPaneDialog::Sleep()
{
	int    ok = -1;
	if(!(Flags & fSleepMode)) {
		Flags |= fSleepMode;
		TDialog * dlg = new TDialog(DLG_CHKPANSLEEP); // @newok
		SString code;
		if(CheckDialogPtr(&dlg, 1))
			while(1)
				if(ExecView(dlg) == cmOK) {
					dlg->getCtrlString(CTL_CHKPANSLEEP_CODE, code);
					int    r = SetupSalByCode(code);
					// @v9.0.5 {
					if(r != 1 && CsObj.GetEqCfg().Flags & PPEquipConfig::fRecognizeCode) {
						PPID   ar_id = 0, reg_type_id = 0;
						PPID   acs_id = GetAgentAccSheet();
						if(acs_id && code[0] && PPObjArticle::GetSearchingRegTypeID(acs_id, 0, 0, &reg_type_id) > 0)
							ArObj.SearchByRegCode(acs_id, reg_type_id, code, &ar_id, 0);
						if(ar_id) {
							SetupAgent(ar_id, 0);
							SetupInfo(0);
							r = 1;
						}
					}
					// } @v9.0.5
					if(r == 1 || code.Cmp("99990", 0) == 0) {
						Flags &= ~fSleepMode;
						IdleClock = clock();
						break;
					}
				}
		delete dlg;
		ok = 1;
	}
	return ok;
}

int FASTCALL CheckPaneDialog::Barrier(int rmv)
{
	if(Flags & fBarrier) {
		if(rmv)
			Flags &= ~fBarrier;
		else
			BarrierViolationCounter++;
		return 1;
	}
	else {
		if(!rmv)
			Flags |= fBarrier;
		return 0;
	}
}

class ComplexDinnerDialog : public PPListDialog {
public:
	ComplexDinnerDialog(PPID locID) : PPListDialog(DLG_COMPLDIN, CTL_COMPLDIN_ELEMENTS)
	{
		LocID = locID;
		{
			SmartListBox * p_list = (SmartListBox *)getCtrlView(CTL_COMPLDIN_ALTLIST);
			if(!SetupStrListBox(p_list))
				PPError();
			setSmartListBoxOption(CTL_COMPLDIN_ALTLIST,  lbtSelNotify);
			setSmartListBoxOption(CTL_COMPLDIN_ELEMENTS, lbtFocNotify);
		}
		Ptb.SetColor(clrFocus,  RGB(0x20, 0xAC, 0x90));
		Ptb.SetColor(clrUnsel,  RGB(0xDA, 0xD7, 0xD0));
		Ptb.SetBrush(brSel,     SPaintObj::psSolid, Ptb.GetColor(clrFocus), 0);
		Ptb.SetBrush(brOdd,     SPaintObj::psSolid, Ptb.GetColor(clrOdd), 0);
		Ptb.SetBrush(brUnsel,   SPaintObj::psSolid, Ptb.GetColor(clrUnsel), 0);
		{
		 	SString temp_buf;
			LOGFONT log_font;
			MEMSZERO(log_font);
			log_font.lfCharSet = DEFAULT_CHARSET;
			ListEntryGap = 5;
			PPGetSubStr(PPTXT_FONTFACE, PPFONT_ARIAL, temp_buf);
			STRNSCPY(log_font.lfFaceName, temp_buf); // @unicodeproblem
			log_font.lfHeight = (DEFAULT_TS_FONTSIZE - TSGGROUPSASITEMS_FONTDELTA);
			Ptb.SetFont(fontList, ::CreateFontIndirect(&log_font));
		}
	}
	int    setDTS(const SaComplex * pData)
	{
		RVALUEPTR(Data, pData);
		Data.RecalcFinalPrice();
		updateList(-1);
		enableCommand(cmOK, Data.IsComplete());
		return 1;
	}
	int    getDTS(SaComplex * pData)
	{
		int    ok = -1;
		if(Data.IsComplete()) {
			ASSIGN_PTR(pData, Data);
			ok = 1;
		}
		return ok;
	}
private:
	DECL_HANDLE_EVENT;
	virtual int setupList();
	void   DrawListItem(TDrawItemData *);

	enum {
		dummyFirst = 1,
		fontList,
		brSel,
		brOdd,
		brUnsel,
		brGrp,
		clrFocus,
		clrOdd,
		clrUnsel
	};
	long   ListEntryGap;
	PPID   LocID;
	SPaintToolBox Ptb;
	SaComplex Data;
	PPObjGoods GObj;
};

IMPL_HANDLE_EVENT(ComplexDinnerDialog)
{
	PPListDialog::handleEvent(event);
	if(event.isCmd(cmDrawItem))
		DrawListItem((TDrawItemData *)TVINFOPTR);
	else if(event.isCmd(cmSetupResizeParams)) {
		PPID   sb_id_altlist  = MAKE_BUTTON_ID(CTL_COMPLDIN_ALTLIST, 1);
		PPID   sb_id_elements = MAKE_BUTTON_ID(CTL_COMPLDIN_ELEMENTS, 1);
		SString font_face;

		PPGetSubStr(PPTXT_FONTFACE, PPFONT_ARIAL, font_face);
		SetCtrlFont(CTL_COMPLDIN_ELEMENTS, font_face, 26);
		SetCtrlFont(CTL_COMPLDIN_ALTLIST, font_face, 26);
		SetCtrlFont(CTL_COMPLDIN_TOTAL, font_face, 26);

		SetCtrlResizeParam(CTL_COMPLDIN_ELEMENTS, 0, 0, 0, CTL_COMPLDIN_TOTAL, crfResizeable);
		//SetCtrlResizeParam(sb_id_elements, CTL_COMPLDIN_ELEMENTS, 0, 0, CTL_COMPLDIN_TOTAL, crfResizeable);
		SetCtrlResizeParam(CTL_COMPLDIN_TOTAL, 0, CTL_COMPLDIN_ELEMENTS, 0, CTL_COMPLDIN_ALTLIST, crfResizeable);
		SetCtrlResizeParam(CTL_COMPLDIN_ALTLIST, 0, CTL_COMPLDIN_TOTAL, 0, 0, crfResizeable);
		//SetCtrlResizeParam(sb_id_altlist, CTL_COMPLDIN_ALTLIST, CTL_COMPLDIN_TOTAL, 0, BTN_COMPLDIN_INPUTQTTY, crfResizeable);
		SetCtrlResizeParam(BTN_COMPLDIN_INPUTQTTY, 0, CTL_COMPLDIN_ALTLIST, 0, 0, crfResizeable);
		SetCtrlResizeParam(STDCTL_OKBUTTON, BTN_COMPLDIN_INPUTQTTY, CTL_COMPLDIN_ALTLIST, 0, 0, crfResizeable);
		SetCtrlResizeParam(STDCTL_CANCELBUTTON, STDCTL_OKBUTTON, CTL_COMPLDIN_ALTLIST, 0, 0, crfResizeable);
		ResizeDlgToFullScreen();
	}
	else if(event.isCmd(cmInputQtty)) {
		double qtty = 1.0;
		if(InputQttyDialog(0, 0, &qtty) > 0) {
			Data.SetQuantity(qtty);
			updateList(-1);
		}
	}
	else if(event.isCmd(cmLBItemFocused) && event.isCtlEvent(CTL_COMPLDIN_ELEMENTS)) {
		SmartListBox * p_box = (SmartListBox*)getCtrlView(CTL_COMPLDIN_ALTLIST);
		if(p_box) {
			p_box->freeAll();
			long   pos = 0;
			getSelection(&pos);
			if(pos > 0 && pos <= (long)Data.getCount()) {
				SaComplexEntry & r_entry = Data.at(pos-1);
				if(r_entry.Flags & SaComplexEntry::fGeneric) {
					SString temp_buf;
					StringSet ss(SLBColumnDelim);
					long   focus_pos = 0;
					for(uint i = 0; i < r_entry.GenericList.getCount(); i++) {
						const PPID goods_id = r_entry.GenericList.at(i).Key;
						ss.clear(1);
						GetGoodsName(goods_id, temp_buf);
						ss.add(temp_buf);
						ss.add((temp_buf = 0).Cat(r_entry.GenericList.at(i).Val, SFMT_MONEY));
						p_box->addItem(i+1, ss.getBuf());
						if(r_entry.FinalGoodsID == goods_id)
							focus_pos = (long)i;
					}
					p_box->focusItem(focus_pos);
				}
				p_box->drawView();
			}
		}
	}
	else if(event.isCmd(cmLBItemSelected) && event.isCtlEvent(CTL_COMPLDIN_ALTLIST)) {
		SmartListBox * p_box = (SmartListBox *)getCtrlView(CTL_COMPLDIN_ALTLIST);
		if(p_box) {
			long   main_pos = 0, subst_pos = 0;
			getSelection(&main_pos);
			p_box->getCurID(&subst_pos);
			if(main_pos > 0 && subst_pos > 0 && Data.Subst((uint)(main_pos-1), (uint)(subst_pos-1)))
				updateList(main_pos-1);
		}
		enableCommand(cmOK, Data.IsComplete());
	}
	else
		return;
	clearEvent(event);
}

void ComplexDinnerDialog::DrawListItem(TDrawItemData * pDrawItem)
{
	if(pDrawItem && pDrawItem->P_View) {
		PPID   list_ctrl_id = pDrawItem->P_View->GetId();
		if(list_ctrl_id == CTL_COMPLDIN_ELEMENTS) {
			HDC    h_dc = pDrawItem->H_DC;
			HFONT  h_fnt_def  = 0;
			HBRUSH h_br_def   = 0;
			HPEN   h_pen_def  = 0;
			COLORREF clr_prev = 0;
			SmartListBox * p_lbx = (SmartListBox *)pDrawItem->P_View;
			RECT   rc = pDrawItem->ItemRect;
			SString temp_buf;
			if(pDrawItem->ItemAction & TDrawItemData::iaBackground) {
				FillRect(h_dc, &rc, (HBRUSH)Ptb.Get(brOdd));
				pDrawItem->ItemAction = 0; // �� ������������ ���
			}
			else if(pDrawItem->ItemID != 0xffffffff) {
				h_fnt_def = (HFONT)SelectObject(h_dc, (HFONT)Ptb.Get(fontList));
				p_lbx->getText((long)pDrawItem->ItemData, temp_buf);
				temp_buf.Transf(CTRANSF_INNER_TO_OUTER);
				if(pDrawItem->ItemState & (ODS_FOCUS|ODS_SELECTED)) {
					h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get(brSel));
					clr_prev = SetBkColor(h_dc, Ptb.GetColor(clrFocus));
					SInflateRect(rc, -1, -(1 + ListEntryGap / 4));
					RoundRect(h_dc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
					rc.left += 4;
				}
				else {
					int  draw_odd = pDrawItem->ItemID % 2;
					clr_prev = SetBkColor(h_dc, Ptb.GetColor(clrUnsel));
					h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get(brUnsel));
					SInflateRect(rc, -1, -(1 + ListEntryGap / 4));
					RoundRect(h_dc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
					rc.left += 4;
				}
				::DrawText(h_dc, temp_buf.cptr(), (int)temp_buf.Len(), &rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE); // @unicodeproblem
			}
		}
		else
			pDrawItem->ItemAction = 0; // ������ �� ������� - ������ �� ������
	}
}

int ComplexDinnerDialog::setupList()
{
	int    ok = 1;
	double total = 0.0;
	SString temp_buf;
	StringSet ss(SLBColumnDelim);
	for(uint i = 0; i < Data.getCount(); i++) {
		const SaComplexEntry & r_entry = Data.at(i);
		const PPID goods_id = NZOR(r_entry.FinalGoodsID, r_entry.GoodsID);
		ss.clear(1);
		GetGoodsName(goods_id, temp_buf);
		ss.add(temp_buf);
		ss.add((temp_buf = 0).Cat(r_entry.Qtty, MKSFMTD(0, 3, NMBF_NOTRAILZ)));
		ss.add((temp_buf = 0).Cat(r_entry.OrgPrice, SFMT_MONEY));
		ss.add((temp_buf = 0).Cat(r_entry.FinalPrice, SFMT_MONEY));
		total += (r_entry.FinalPrice * r_entry.Qtty);
		addStringToList(i+1, ss.getBuf());
	}
	setCtrlReal(CTL_COMPLDIN_TOTAL, total);
	return ok;
}

int CheckPaneDialog::InputComplexDinner(SaComplex & rComplex)
{
	DIALOG_PROC_BODY_P1(ComplexDinnerDialog, CnLocID, &rComplex);
}
//
// SelCheckListDialog
//
struct _SelCheck {
	PPID    CheckID;
	SString SelFormat;
};

class SelCheckListDialog : public PPListDialog {
public:
	struct AddedParam {
		AddedParam(PPID nodeID, long tableCode, PPID agentID, long rights)
		{
			NodeID    = nodeID;
			TableCode = tableCode;
			AgentID   = agentID;
			Rights    = rights;
			Flags     = 0;
		}
		enum {
			fAllowReturns = 0x0001
		};
		PPID   NodeID;
		long   TableCode;
		PPID   AgentID;
		long   Rights;       // ����� ������� �� �������� ������
		long   Flags;        // @v9.3.5
		SString FormatName;
	};
private:
	void Helper_Constructor(CPosProcessor * pSrv, const AddedParam * pAddParam)
	{
		P_Srv = pSrv;
		assert(P_Srv);
		P_Cto = 0;
		P_AddParam  = pAddParam;
		State = 0;
		setSmartListBoxOption(CTL_SELCHECK_LIST, lbtSelNotify);
		setSmartListBoxOption(CTL_SELCHECK_LIST, lbtFocNotify);
		if(oneof2(this->Id, DLG_ORDERCHECKS, DLG_ORDERCHECKS_L)) {
			State |= stTblOrders;
			P_Cto = new CTableOrder;
			enableCommand(cmaInsert, P_Cto && P_Cto->HasRight(PPR_INS));
			enableCommand(cmaEdit,   P_Cto && P_Cto->HasRight(PPR_MOD));
			enableCommand(cmaDelete, P_Cto && P_Cto->HasRight(PPR_DEL));
		}
	}
public:
	SelCheckListDialog(uint dlgId, int selectFormat, PPCashMachine * pCm, CPosProcessor * pSrv, const AddedParam * pAddParam = 0) :
		PPListDialog(dlgId, CTL_SELCHECK_LIST)
	{
		Helper_Constructor(pSrv, pAddParam);
		SETFLAG(State, stSelectFormat, selectFormat);
		SETFLAG(State, stSelectSlipFormat, (selectFormat > 0));
		Init(pCm);
	}
	SelCheckListDialog(uint dlgId, const TSArray <CCheckViewItem> * pChkList, int selToUnite, CPosProcessor * pSrv, const AddedParam * pAddParam = 0) :
		PPListDialog(dlgId, CTL_SELCHECK_LIST)
	{
		Helper_Constructor(pSrv, pAddParam);
		State |= stOuterList;
		SETFLAG(State, stSelToUnite, selToUnite);
		if(pChkList) {
			assert(ChkList.getItemSize() == pChkList->getItemSize());
			ChkList = *pChkList;
		}
		Init(0);
	}
	~SelCheckListDialog()
	{
		delete P_Cto;
	}
	int    getDTS(_SelCheck * pSelCheck);
	int    setList(const TSArray <CCheckViewItem> & rChkList)
	{
		int    ok = -1;
		if(State & stOuterList) {
			ChkList = rChkList;
			ok = Init(0);
		}
		return ok;
	}
private:
	DECL_HANDLE_EVENT;
	virtual int setupList();
	virtual int addItem(long * pPos, long * pID);
	virtual int editItem(long pos, long id);
	virtual int delItem(long pos, long id);

	int    SetupItemList();
	int    SplitCheck();
	int    UniteChecks();
	int    Init(PPCashMachine * pCm)
	{
		enableCommand(cmSplitCheck, 0);
		enableCommand(cmUniteChecks, 0);
		LastDate = getcurdate_();
		SetupCalDate(CTLCAL_SELCHECK_DATE, CTL_SELCHECK_DATE);
		setCtrlData(CTL_SELCHECK_DATE, &LastDate);
		if(!SetupStrListBox(this, CTL_SELCHECK_ITEMLIST))
			PPError();
		FmtList.Clear();
		LastChkNo = -1;
		LastChkID = 0;
		State &= ~(stInputUpdated | stListUpdated);
		updateList(-1);
		SetupItemList();
		if(State & stSelectFormat) {
			ListWindow * p_lw = 0;
			ComboBox   * p_cb = (ComboBox*)getCtrlView(CTLSEL_SELCHECK_FORMAT);
			if(p_cb && pCm && pCm->GetSlipFormatList(&FmtList, BIN(State & stSelectSlipFormat)) > 0) {
				ListWindow * p_lw = new ListWindow(new StrAssocListBoxDef(&FmtList, /*lbtDisposeData |*/ lbtDblClkNotify), 0, 0);
				long   fmt_id = 0;
				if(FmtList.getCount() == 1)
					fmt_id = FmtList.at(0).Id;
				else if(FmtList.getCount() > 1 && P_AddParam && P_AddParam->FormatName.NotEmpty()) {
					for(uint i = 0; !fmt_id && i < FmtList.getCount(); i++) {
						StrAssocArray::Item fmt_list_item = FmtList.at(i);
						if(P_AddParam->FormatName.CmpNC(fmt_list_item.Txt) == 0)
							fmt_id = fmt_list_item.Id;
					}
				}
				p_cb->setListWindow(p_lw, fmt_id);
			}
		}
		else {
			showCtrl(CTL_SELCHECK_FORMAT, 0);
			showCtrl(CTLSEL_SELCHECK_FORMAT, 0);
		}
		return 1;
	}
	CPosProcessor * P_Srv;
	PPObjGoods GObj;
	CTableOrder * P_Cto;
	StrAssocArray FmtList;
	TSArray <CCheckViewItem> ChkList;
	long   LastChkNo;
	PPID   LastChkID;
	LDATE  LastDate;
	enum {
		stTblOrders        = 0x0001,
		stInputUpdated     = 0x0002,
		stListUpdated      = 0x0004,
		stOuterList        = 0x0008,
		stSelectFormat     = 0x0010,
		stSelToUnite       = 0x0020,
		stSelectSlipFormat = 0x0040
	};
	long   State;
	const  AddedParam * P_AddParam; // @notowned
};

// virtual
int SelCheckListDialog::addItem(long * pPos, long * pID)
{
	int    ok = -1;
	if(State & stTblOrders) {
		if(SETIFZ(P_Cto, new CTableOrder)) {
			CTableOrder::Param param;
			if(P_AddParam)
				param.PosNodeID = P_AddParam->NodeID;
			if(param.PosNodeID && P_Cto->Create(&param) > 0) {
				ASSIGN_PTR(pPos, -1);
				ASSIGN_PTR(pID, -1);
				ok = 1;
			}
		}
	}
	return ok;
}

//virtual
int SelCheckListDialog::editItem(long pos, long id)
{
	int    ok = -1;
	if((State & stTblOrders) && pos >= 0 && pos < (long)ChkList.getCount()) {
		PPID   check_id = ChkList.at(pos).ID;
		if(check_id) {
			if(SETIFZ(P_Cto, new CTableOrder)) {
				CTableOrder::Packet pack;
				if(P_Cto->GetCheck(check_id, &pack) > 0 && P_Cto->Edit(&pack) > 0) {
					ok = P_Cto->Update(&pack, 1);
					if(!ok)
						PPError();
				}
			}
		}
	}
	return ok;
}

// virtual
int SelCheckListDialog::delItem(long pos, long id)
{
	int    ok = -1;
	if((State & stTblOrders) && pos >= 0 && pos < (long)ChkList.getCount() && CONFIRM(PPCFM_CANCELCTBLORD)) {
		if(SETIFZ(P_Cto, new CTableOrder)) {
			if(!P_Cto->Cancel(ChkList.at(pos).ID))
				ok = PPErrorZ();
			else {
				ChkList.atFree(pos);
				ok = 1;
			}
		}
	}
	return ok;
}

// virtual
int SelCheckListDialog::setupList()
{
	int    ok = -1;
	uint   i;
	if(State & stTblOrders)
		P_Srv->GetTblOrderList(LastDate, ChkList);
	else if(!(State & stOuterList)) {
		State &= ~stListUpdated;
		long   chk_no = getCtrlLong(CTL_SELCHECK_CODE);
		LDATE  dt = getCtrlDate(CTL_SELCHECK_DATE);
		if(!checkdate(dt, 1))
			dt = ZERODATE;
		if(chk_no || dt) {
			if(chk_no != LastChkNo || diffdate(dt, LastDate)) {
				SArray temp_list(sizeof(CCheckTbl::Rec));
				CCheckTbl::Rec * p_rec;
				THROW(P_Srv->GetCc().SearchByDateAndCode(chk_no, dt, &temp_list));
				for(i = 0; temp_list.enumItems(&i, (void **)&p_rec);) {
					// @v9.3.5 MONEYTOLDBL(p_rec->Amount) <= 0.0 --> MONEYTOLDBL(p_rec->Amount) == 0.0
					const double cc_amt = MONEYTOLDBL(p_rec->Amount);
					int   do_remove = 0;
					if(p_rec->Flags & CCHKF_SKIP)
						do_remove = 1;
					else if(P_AddParam && P_AddParam->NodeID && p_rec->CashID != P_AddParam->NodeID)
						do_remove = 1;
					else if(cc_amt == 0.0)
						do_remove = 1;
					else if(cc_amt < 0) {
						do_remove = BIN(!(P_AddParam && P_AddParam->Flags & P_AddParam->fAllowReturns));
					}
					if(do_remove)
						temp_list.atFree(--i);
				}
				ChkList.freeAll();
				for(i = 0; temp_list.enumItems(&i, (void **)&p_rec);) {
					CCheckViewItem item;
					MEMSZERO(item);
					*(CCheckTbl::Rec *)&item = *p_rec;
					if(item.Flags & CCHKF_EXT) {
						CCheckExtTbl::Rec ext_rec;
						if(P_Srv->GetCc().GetExt(p_rec->ID, &ext_rec) > 0) {
							item.TableCode  = ext_rec.TableNo;
							item.GuestCount = ext_rec.GuestCount;
							item.AgentID    = ext_rec.SalerID;
							item.LinkCheckID = ext_rec.LinkCheckID;
							if(item.Flags & CCHKF_ORDER)
								item.OrderTime.Init(ext_rec.StartOrdDtm, ext_rec.EndOrdDtm);
						}
					}
					ChkList.insert(&item);
				}
				State |= stListUpdated;
			}
		}
		else if(ChkList.getCount()) {
			ChkList.freeAll();
			State |= stListUpdated;
		}
		LastChkNo = chk_no;
		LastDate  = dt;
	}
	if(ChkList.getCount()) {
		SString temp_buf;
		StringSet  ss(SLBColumnDelim);
		for(i = 0; i < ChkList.getCount(); i++) {
			const CCheckViewItem & r_chk_rec = ChkList.at(i);
			ss.clear();
			LDATETIME dtm;
			dtm.Set(r_chk_rec.Dt, r_chk_rec.Tm);
			if(!(State & stTblOrders)) {
				ss.add((temp_buf = 0).Cat(dtm, DATF_DMY, TIMF_HMS));
				{
					temp_buf = 0;
					if(r_chk_rec.Flags & CCHKF_JUNK)
						temp_buf.CatDiv('*', 2);
					ss.add(temp_buf.Cat(r_chk_rec.Code));
				}
				ss.add((temp_buf = 0).Cat(MONEYTOLDBL(r_chk_rec.Amount), SFMT_MONEY));
				{
					temp_buf = 0;
					if(r_chk_rec.TableCode) {
						temp_buf.Cat(r_chk_rec.TableCode);
					}
					else if(r_chk_rec.Flags & CCHKF_IMPORTED) {
						temp_buf.Cat("UHTT");
					}
					ss.add(temp_buf);
				}
				GetArticleName(r_chk_rec.AgentID, temp_buf);
				ss.add(temp_buf);
			}
			else {
				// @lbt_tblordlist        "10,R,����;26,R,����� ������;10,L,�����;19,L,�������� �����;14,R,����������;18,L,����/�����;10,L,� ����"
				// @lbt_tblordlist_l      "20,R,����;35,R,����� ������;20,L,�����;30,L,�������� �����;25,R,����������;36,L,����/�����;20,L,� ����"

				SString scard_psn, scard_no;
				STimeChunk tm_chunk;
				CCheckExtTbl::Rec ext_chk_rec;

				MEMSZERO(ext_chk_rec);
				P_Srv->GetCc().GetExt(r_chk_rec.ID, &ext_chk_rec);
				{
					temp_buf = 0;
					if(r_chk_rec.TableCode) {
						temp_buf.Cat(r_chk_rec.TableCode);
					}
					else if(r_chk_rec.Flags & CCHKF_IMPORTED) {
						temp_buf.Cat("UHTT");
					}
					ss.add(temp_buf);
				}
				tm_chunk.Init(ext_chk_rec.StartOrdDtm, ext_chk_rec.EndOrdDtm);
				ss.add(tm_chunk.ToStr(temp_buf = 0, STimeChunk::fmtOmitSec));
				if(r_chk_rec.SCardID) {
					SCardTbl::Rec sc_rec;
					if(P_Srv->GetScObj().Fetch(r_chk_rec.SCardID, &sc_rec) > 0) { // @v8.4.3 Search-->Fetch
						scard_no = sc_rec.Code;
						if(sc_rec.PersonID)
							GetPersonName(sc_rec.PersonID, scard_psn);
					}
				}
				ss.add(scard_no);
				ss.add(scard_psn);
				{
					CCheckPacket chk_pack;
					temp_buf = 0;
					if(P_Srv->GetCc().LoadPacket(r_chk_rec.ID, 0, &chk_pack) > 0) {
						StringSet ss(SLBColumnDelim);
						if(chk_pack.GetCount()) {
							const CCheckLineTbl::Rec & cclr = chk_pack.GetLine(0);
							temp_buf.Cat(intmnytodbl(cclr.Price));
						}
					}
					ss.add(temp_buf);
				}
				ss.add((temp_buf = 0).Cat(dtm, DATF_DMY, TIMF_HMS));
				ss.add((temp_buf = 0).Cat(r_chk_rec.Code));
			}
			THROW(addStringToList(r_chk_rec.ID, ss.getBuf()));
		}
	}
	if((State & stListUpdated) || ChkList.getCount())
		ok = 1;
	CATCHZOK
	return ok;
}

int SelCheckListDialog::SetupItemList()
{
	int    ok = -1;
	SString sub, memo_buf;
	int    memo_has_addr = 0;
	if(/*IsOuterList &&*/ !(State & stTblOrders) && (ChkList.getCount() || (State & stInputUpdated))) {
		SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_SELCHECK_ITEMLIST);
		if(p_list) {
			PPID   chk_id = 0;
			getCurItem(0, &chk_id);
			if(chk_id != LastChkID) {
				CCheckPacket pack;
				p_list->freeAll();
				if(chk_id && P_Srv->GetCc().LoadPacket(chk_id, 0, &pack) > 0) {
					memo_buf = 0;
					if(pack.Ext.AddrID) {
						PPObjLocation loc_obj;
						LocationTbl::Rec loc_rec;
						if(loc_obj.Search(pack.Ext.AddrID, &loc_rec) > 0) {
							LocationCore::GetExField(&loc_rec, LOCEXSTR_PHONE, sub);
							memo_buf.Cat(sub);
							LocationCore::GetExField(&loc_rec, LOCEXSTR_SHORTADDR, sub);
							if(sub.NotEmptyS())
								memo_buf.CatDiv(',', 2, 1).Cat(sub);
							LocationCore::GetExField(&loc_rec, LOCEXSTR_CONTACT, sub);
							if(sub.NotEmptyS())
								memo_buf.CatDiv(',', 2, 1).Cat(sub);
						}
					}
					if(memo_buf.NotEmptyS())
						memo_has_addr = 1;
					else
						memo_buf = pack.Ext.Memo;
					StringSet ss(SLBColumnDelim);
					for(uint i = 0; i < pack.GetCount(); i++) {
						ss.clear();
						const  CCheckLineTbl::Rec & cclr = pack.GetLine(i);
						double price = intmnytodbl(cclr.Price);
						double sum = R2(cclr.Quantity * (price - cclr.Dscnt));
						GetGoodsName(cclr.GoodsID, sub);
						ss.add(sub);
						ss.add((sub = 0).Cat(price, SFMT_MONEY));
						ss.add((sub = 0).Cat(cclr.Quantity, SFMT_QTTY));
						ss.add((sub = 0).Cat(sum, SFMT_MONEY));
						THROW(p_list->addItem(i, ss.getBuf()));
					}
				}
				{
					int to_disable = BIN(!(State & stSelToUnite) && (pack.Rec.Flags & CCHKF_SUSPENDED) && !(pack.Rec.Flags & CCHKF_JUNK));
					enableCommand(cmSplitCheck, to_disable);
					enableCommand(cmUniteChecks, to_disable);
				}
				{
					PPLoadString(memo_has_addr ? "address" : "memo", sub = 0);
					setLabelText(CTL_SELCHECK_MEMO, sub);
					setCtrlString(CTL_SELCHECK_MEMO, memo_buf);
				}
				p_list->focusItem(0);
				p_list->drawView();
				LastChkID = chk_id;
			}
		}
	}
	CATCHZOK
	return ok;
}
//
// SplitSuspCheckDialog
//
class SplitSuspCheckDialog : public Lst2LstAryDialog {
public:
	struct ListItem {
		long   LineNo;
		long   GoodsID;
		long   DivID;
		double Quantity;
		double Price;
		double Discount;
		char   Serial[64];
	};
	SplitSuspCheckDialog(uint dlgId, CCheckPacket * pPack, ListToListUIData * pData, SArray * pLeft, SArray * pRight) : Lst2LstAryDialog(dlgId, pData, pLeft, pRight)
	{
		SString agent_name;
		P_Pack = pPack;
		double amount = MONEYTOLDBL(P_Pack->Rec.Amount);
		GetArticleName(P_Pack->Ext.SalerID, agent_name);
		setCtrlString(CTL_SPLITSUSCHK_AGENT, agent_name);
		setCtrlData(CTL_SPLITSUSCHK_AMOUNT, &amount);
		setCtrlData(CTL_SPLITSUSCHK_TABLE,  &P_Pack->Ext.TableNo);
		disableCtrls(1, CTL_SPLITSUSCHK_AGENT, CTL_SPLITSUSCHK_AMOUNT, CTL_SPLITSUSCHK_TABLE, 0L);
		SetupStrListBox(this, CTL_SPLITSUSCHK_LIST1);
		SetupStrListBox(this, CTL_SPLITSUSCHK_LIST2);
		setupLeftList();
		setupRightList();
	}
	int    getDTS(SArray * pLeftList, SArray * pRightList)
	{
		int    ok = 1;
		if(pLeftList) {
			pLeftList->copy(*GetLeft());
			THROW_PP(pLeftList->getCount(), PPERR_ONECHECKEMPTY);
		}
		if(pRightList) {
			pRightList->copy(*GetRight());
			THROW_PP(pRightList->getCount(), PPERR_ONECHECKEMPTY);
		}
		CATCHZOK
		return ok;
	}
private:
	virtual int addItem();
	virtual int removeItem();
	virtual int addAll() { return -1; }
	virtual int removeAll() { return -1; }
	virtual int SetupList(SArray *, SmartListBox *);

	CCheckPacket * P_Pack;
};

// virtual
int SplitSuspCheckDialog::addItem()
{
	int    ok  = 1;
	long   id = 0;
	uint   pos = 0;
	SmartListBox * p_view = GetLeftList();
	SArray * p_rl = GetRight(), * p_ll = GetLeft();
	if(p_view->getCurID(&id) && id && p_ll->lsearch(&id, &pos, CMPF_LONG, 0) > 0) {
		double qtty = 0.0;
		ListItem * p_litem = (ListItem *)p_ll->at(pos);
		if(InputQttyDialog(0, 0, &(qtty = p_litem->Quantity)) > 0 && qtty > 0.0) {
			qtty = (qtty > p_litem->Quantity) ? p_litem->Quantity : qtty;
			if(p_rl->lsearch(p_litem, &pos, CMPF_LONG, 0) > 0) {
				ListItem * p_ritem = (ListItem*)p_rl->at(pos);
				p_ritem->Quantity += qtty; // @v8.2.12 @fix (= qtty)-->(+= qtty)
			}
			else {
				ListItem item;
				item = *p_litem;
				item.Quantity = qtty;
				THROW_SL(p_rl->insert(&item));
			}
			p_litem->Quantity -= qtty;
			if(p_litem->Quantity == 0.0)
				THROW_SL(p_ll->atFree(pos));
			THROW(setupLeftList());
			THROW(setupRightList());
		}
	}
	CATCHZOKPPERR
	return ok;
}

// virtual
int SplitSuspCheckDialog::removeItem()
{
	int    ok  = 1;
	uint   pos = 0;
	long   id = 0;
	SmartListBox * l = GetRightList();
	SArray * p_rl = GetRight(), * p_ll = GetLeft();
	if(l && l->getCurID(&id) && id && p_rl->lsearch(&id, &pos, CMPF_LONG, 0) > 0) {
		uint lpos = 0;
		ListItem * p_ritem = (ListItem*)p_rl->at(pos);
		if(p_ll->lsearch(p_ritem, &lpos, CMPF_LONG, 0) > 0) {
			ListItem * p_litem = (ListItem*)p_ll->at(lpos);
			p_litem->Quantity += p_ritem->Quantity;
		}
		else {
			ListItem item;
			MEMSZERO(item);
			item = *p_ritem;
			THROW_SL(p_ll->insert(&item));
		}
		THROW_SL(p_rl->atFree(pos) > 0);
        THROW(setupRightList());
		THROW(setupLeftList());
	}
	CATCHZOKPPERR
	return ok;
}

// virtual
int SplitSuspCheckDialog::SetupList(SArray * pList, SmartListBox * pListBox)
{
	int    ok = 1;
	if(pList && pListBox && pListBox->def) {
		const long preserve_pos = pListBox->def->_curItem();
		SString temp_buf;
		StringSet ss(SLBColumnDelim);
		ListItem * p_item = 0;
		pListBox->freeAll();
		for(uint i = 0; pList->enumItems(&i, (void**)&p_item) > 0;) {
			ss.clear(1);
			GetGoodsName(p_item->GoodsID, temp_buf);
			ss.add(temp_buf, 0);
			ss.add((temp_buf = 0).Cat(p_item->Price), 0);
			ss.add((temp_buf = 0).Cat(p_item->Quantity), 0);
			if(!pListBox->addItem(p_item->LineNo, ss.getBuf()))
				ok = (PPSetErrorSLib(), 0);
		}
		pListBox->def->go(preserve_pos);
		pListBox->drawView();
	}
	return ok;
}

int SelCheckListDialog::SplitCheck()
{
	int    ok = -1;
	SplitSuspCheckDialog * dlg = 0;
	if(oneof2(resourceID, DLG_SELSUSCHECK_L, DLG_SELSUSCHECK)) {
		long   chk_id = 0;
		uint   dlg_id = (resourceID == DLG_SELSUSCHECK_L) ? DLG_SPLITSUSCHK_L : DLG_SPLITSUSCHK;
		SArray left_list(sizeof(SplitSuspCheckDialog::ListItem));
		SArray right_list(sizeof(SplitSuspCheckDialog::ListItem));
		CCheckPacket pack;
		THROW_PP(!P_AddParam || P_AddParam->Rights & CheckPaneDialog::orfSplitCheck, PPERR_NORIGHTS);
		{
			getCurItem(0, &chk_id);
			if(chk_id && P_Srv->GetCc().LoadPacket(chk_id, 0, &pack) > 0) {
				SString serial;
				CCheckLineTbl::Rec chk_item;
				THROW_PP(!P_AddParam || !(pack.Rec.Flags & CCHKF_PREPRINT) || P_AddParam->Rights & CheckPaneDialog::orfChgPrintedCheck, PPERR_NORIGHTS);
				for(uint pos = 0; pack.EnumLines(&pos, &chk_item, &serial) > 0;) {
					SplitSuspCheckDialog::ListItem item;
					MEMSZERO(item);
					item.LineNo   = pos;
					item.GoodsID  = chk_item.GoodsID;
					item.DivID    = (long)chk_item.DivID;
					item.Discount = chk_item.Dscnt;
					item.Price    = intmnytodbl(chk_item.Price);
					item.Quantity = chk_item.Quantity;
					serial.CopyTo(item.Serial, sizeof(item.Serial));
					THROW_SL(left_list.insert(&item));
				}
			}
		}
		if(left_list.getCount()) {
			ListToListUIData ui_data;
			ui_data.LeftCtlId  = CTL_SPLITSUSCHK_LIST1;
			ui_data.RightCtlId = CTL_SPLITSUSCHK_LIST2;
			THROW(CheckDialogPtr(&(dlg = new SplitSuspCheckDialog(dlg_id, &pack, &ui_data, &left_list, &right_list)), 0));
			for(int valid_data = 0; !valid_data && ExecView(dlg) == cmOK;) {
				if(!dlg->getDTS(&left_list, &right_list))
					PPError();
				else {
					CCheckPacket add_pack;
					CCheckCore & r_cc = P_Srv->GetCc();
					{
						PPTransaction tra(1);
						THROW(tra);
						{
							SplitSuspCheckDialog::ListItem * p_item = 0;
							pack.Rec.ID       = 0;
							pack.Ext.CheckID  = 0;
							pack.Ext.AddPaym_unused = 0; // @v9.0.4 _unused
							pack._Cash        = 0;
							pack.PctDis       = 0;
							add_pack.Rec = pack.Rec;
							add_pack.Ext = pack.Ext;
							add_pack.Ext.GuestCount = 0;
							{
								CCheckTbl::Rec chk_rec;
								add_pack.Rec.Code = 1 + ((r_cc.GetLastCheckByCode(pack.Rec.CashID, &chk_rec) > 0) ? chk_rec.Code : pack.Rec.Code);
							}
							getcurdatetime(&add_pack.Rec.Dt, &add_pack.Rec.Tm);
							THROW(pack.ClearLines());
							for(uint pos = 0; left_list.enumItems(&pos, (void**)&p_item) > 0;) {
								CCheckLineTbl::Rec chk_item;
								MEMSZERO(chk_item);
								chk_item.GoodsID  = p_item->GoodsID;
								chk_item.DivID    = (int16)p_item->DivID;
								chk_item.Price    = dbltointmny(p_item->Price);
								chk_item.Dscnt    = p_item->Discount;
								chk_item.Quantity = p_item->Quantity;
								THROW(pack.InsertItem_(&chk_item, p_item->Serial));
							}
							for(uint pos = 0; right_list.enumItems(&pos, (void**)&p_item) > 0;) {
								CCheckLineTbl::Rec chk_item;
								MEMSZERO(chk_item);
								chk_item.GoodsID  = p_item->GoodsID;
								chk_item.DivID    = (int16)p_item->DivID;
								chk_item.Price    = dbltointmny(p_item->Price);
								chk_item.Dscnt    = p_item->Discount;
								chk_item.Quantity = p_item->Quantity;
								THROW(add_pack.InsertItem_(&chk_item, p_item->Serial));
							}
							pack.SetupAmount(0, 0);
							add_pack.SetupAmount(0, 0);
						}
						// @v8.9.8 THROW(r_cc.RemovePacket(chk_id, 0));
						// @v8.9.8 THROW(r_cc.TurnCheck(&pack, 0));
						pack.Rec.ID = chk_id; // @v8.9.8
						THROW(r_cc.UpdateCheck(&pack, 0)); // @v8.9.8
						THROW(r_cc.TurnCheck(&add_pack, 0));
						THROW(tra.Commit());
					}
					if(ChkList.getCount()) {
						uint pos = 0;
						CCheckViewItem v_item;
						MEMSZERO(v_item);
						*((CCheckTbl::Rec*)&v_item) = pack.Rec;
						v_item.TableCode = pack.Ext.TableNo;
						v_item.AgentID   = pack.Ext.SalerID;
						THROW_SL(ChkList.lsearch(&chk_id, &pos, CMPF_LONG, 0) > 0);
						THROW_SL(ChkList.atFree(pos));
						THROW_SL(ChkList.insert(&v_item));
						MEMSZERO(v_item);
						*((CCheckTbl::Rec*)&v_item) = add_pack.Rec;
						v_item.TableCode = pack.Ext.TableNo;
						v_item.AgentID   = pack.Ext.SalerID;
						THROW_SL(ChkList.insert(&v_item));
					}
					ok = valid_data = 1;
				}
			}
		}
	}
	CATCHZOKPPERR
	delete dlg;
	return ok;
}

int SelCheckListDialog::UniteChecks()
{
	int    ok = -1;
	long   chk1_id = 0L;
	SelCheckListDialog * dlg = 0;
	getCurItem(0, &chk1_id);
	if(chk1_id && ChkList.getCount() > 1 && oneof2(resourceID, DLG_SELSUSCHECK_L, DLG_SELSUSCHECK)) {
		uint   pos = 0;
		TSArray <CCheckViewItem> list;
		THROW_PP(!P_AddParam || P_AddParam->Rights & CheckPaneDialog::orfMergeChecks, PPERR_NORIGHTS); // @v8.5.5
		list.copy(ChkList);
		THROW_SL(list.lsearch(&chk1_id, &pos, PTR_CMPFUNC(long)) > 0);
		list.atFree(pos);
		uint dlg_id = (DlgFlags & fLarge) ? DLG_SELSUSCHECK_L : DLG_SELSUSCHECK;
		THROW(CheckDialogPtr(&(dlg = new SelCheckListDialog(dlg_id, &list, 1, P_Srv, P_AddParam)), 0));
		for(int valid_data = 0; !valid_data && ExecView(dlg) == cmOK;) {
			_SelCheck check2;
			if(!dlg->getDTS(&check2))
				PPError();
			else {
				dlg_id = (resourceID == DLG_SELSUSCHECK_L) ? DLG_SPLITSUSCHK_L : DLG_SPLITSUSCHK;
				SString serial;
				CCheckLineTbl::Rec chk_item;
				CCheckPacket pack1, pack2;
				CCheckCore & r_cc = P_Srv->GetCc();
				{
					PPTransaction tra(1);
					THROW(tra);
					THROW(r_cc.LoadPacket(chk1_id, 0, &pack1));
					THROW(r_cc.LoadPacket(check2.CheckID, 0, &pack2));
					// @v9.0.4 pack1.Ext.AddPaym  += pack2.Ext.AddPaym;
					pack1._Cash        += pack2._Cash;
					SETIFZ(pack1.PctDis, pack2.PctDis);
					for(pos = 0; pack2.EnumLines(&pos, &chk_item, &serial) > 0;) {
						THROW(pack1.InsertItem_(&chk_item, serial));
					}
					// @v8.9.8 THROW(r_cc.RemovePacket(chk1_id, 0));
					THROW(r_cc.RemovePacket(check2.CheckID, 0));
					THROW(pack1.SetupAmount(0, 0));
					// @v8.9.8 THROW(r_cc.TurnCheck(&pack1, 0));
					pack1.Rec.ID = chk1_id; // @v8.9.8
					THROW(r_cc.UpdateCheck(&pack1, 0)); // @v8.9.8
					DS.LogAction(PPACN_UNITECCHECK, PPOBJ_CCHECK, pack1.Rec.ID, pack2.Rec.ID, 0);
					THROW(tra.Commit());
				}
				if(ChkList.getCount()) {
					uint   new_pos = 0;
					CCheckViewItem v_item;
					MEMSZERO(v_item);
					*((CCheckTbl::Rec*)&v_item) = pack1.Rec;
					v_item.TableCode = pack1.Ext.TableNo;
					v_item.AgentID   = pack1.Ext.SalerID;
					THROW_SL(ChkList.lsearch(&chk1_id, &(pos = 0), CMPF_LONG, 0) > 0);
					THROW_SL(ChkList.atInsert(pos, &v_item));
					THROW_SL(ChkList.atFree(pos + 1));
					THROW_SL(ChkList.lsearch(&check2.CheckID, &(pos = 0), CMPF_LONG, 0) > 0);
					THROW_SL(ChkList.atFree(pos));
					LastChkID = 0;
					if(P_Box) {
						THROW_SL(ChkList.lsearch(&pack1.Rec.ID, &(pos = 0), CMPF_LONG, 0) > 0);
						P_Box->focusItem(pos);
					}
				}
				ok = valid_data = 1;
			}
		}
	}
	CATCHZOKPPERR
	delete dlg;
	return ok;
}

IMPL_HANDLE_EVENT(SelCheckListDialog)
{
	if(event.isCmd(cmOK)) {
		updateList(-1);
		if(State & stListUpdated) {
			selectCtrl(CTL_SELCHECK_LIST);
			State &= ~stListUpdated;
			clearEvent(event);
		}
	}
	else if(event.isCmd(cmLBDblClk))
		TVCMD = cmOK;
	else if(event.isCmd(cmNewCheck) && State & stTblOrders) {
		if(IsInState(sfModal)) {
			endModal(cmNewCheck);
			return; // ����� endModal �� ������� ���������� � this
		}
	}
	TDialog::handleEvent(event);
	if(TVCOMMAND) {
		const uint ev_ctl_id = event.getCtlID();
		if(TVCMD == cmInputUpdated) {
			State |= stInputUpdated;
			if(event.isCtlEvent(CTL_SELCHECK_DATE)) {
				if(State & stTblOrders) {
					LDATE  dt = getCtrlDate(CTL_SELCHECK_DATE);
					if(dt != LastDate && checkdate(dt, 1)) {
						LastDate = dt;
						State |= stListUpdated;
						updateList(-1);
					}
				}
			}
		}
		else if(oneof2(TVCMD, cmLBItemSelected, cmLBItemFocused) && ev_ctl_id == CTL_SELCHECK_LIST)
			SetupItemList();
		else if(TVCMD == cmSplitCheck) {
			if(SplitCheck() > 0) {
				State |= stListUpdated;
				updateList(-1);
			}
		}
		else if(TVCMD == cmUniteChecks) {
			if(UniteChecks() > 0) {
				State |= stListUpdated;
				updateList(-1);
			}
		}
		else if(TVCMD == cmaInsert) {
			if(addItem(0, 0) > 0) {
				P_Srv->GetTblOrderList(LastDate, ChkList);
				updateList(-1);
			}
		}
		else if(TVCMD == cmaEdit) {
			long pos = -1, id = -1;
			getCurItem(&pos, &id);
			if(editItem(pos, id) > 0) {
				P_Srv->GetTblOrderList(LastDate, ChkList);
				updateList(-1);
			}
		}
		else if(TVCMD == cmaDelete) {
			long pos = -1, id = -1;
			getCurItem(&pos, &id);
			if(delItem(pos, id) > 0)
				updateList(-1);
		}
		else
			return;
	}
	else if(TVBROADCAST && oneof2(TVCMD, cmReceivedFocus, cmCommitInput) && (State & stInputUpdated)) {
		updateList(-1);
		State &= ~stInputUpdated;
	}
	else
		return;
	clearEvent(event);
}

int SelCheckListDialog::getDTS(_SelCheck * pSelCheck)
{
	int    ok = 1;
	ushort sel = CTL_SELCHECK_CODE;
	_SelCheck  sel_chk;
	sel_chk.CheckID   = 0;
	sel_chk.SelFormat = 0;
	if(!(State & stOuterList)) {
		LDATE  dt = ZERODATE;
		long   fmt_id = getCtrlLong(CTL_SELCHECK_FORMAT);
		getCtrlData(sel = CTL_SELCHECK_DATE, &dt);
		THROW_PP(dt || (State & stSelectFormat), PPERR_CHKDATENEEDED);
		THROW_SL(checkdate(dt, 0));
		if(fmt_id) {
			StrAssocArray::Item fmt_item = FmtList.at((uint)(fmt_id - 1));
			if(fmt_item.Txt[0])
				sel_chk.SelFormat = fmt_item.Txt;
		}
	}
	sel = 0;
	THROW_PP(ChkList.getCount() || (State & stSelectFormat), PPERR_CHECKNOTFOUND);
	getCurItem(0, &sel_chk.CheckID);
	CATCH
		ok = PPErrorByDialog(this, sel, -1);
	ENDCATCH
	ASSIGN_PTR(pSelCheck, sel_chk);
	return ok;
}

struct AddrByPhoneItem {
	PPID   ObjType;
	PPID   ObjID;
	long   ObjFlags;
	PPID   CityID;
	PPID   AddrID;     // ���� ObjType == PPOBJ_PERSON, �� AddrID �������� ����� �� ������ �������, ������������� ����������
	char   Phone[32];
	char   Addr[128];
	char   Contact[128];
};

int CheckPaneDialog::EditMemo(const char * pDlvrPhone, const char * pChannel)
{
	class CheckDlvrDialog : public TDialog {
	public:
		CheckDlvrDialog(PPID scardID, const char * pDlvrPhone, const char * pChannel) :
			TDialog(DLG_CCHKDLVR), Channel(pChannel)
		{
			addGroup(GRP_SCARD, new SCardCtrlGroup(0, CTL_CCHKDLVR_SCARD, 0)); // @v9.4.5
			LockAddrModChecking = 0;
			MEMSZERO(OrgLocRec);
			Data.SCardID_ = scardID;
			PersonID = 0;
			SetupCalDate(CTLCAL_CCHKDLVR_DT, CTL_CCHKDLVR_DT);
			SetupTimePicker(this, CTL_CCHKDLVR_TM, CTLTM_CCHKDLVR_TM);
			setStaticText(CTL_CCHKDLVR_ST_SIP, Channel);
			DefCityID = 0;
			DlvrPhone = pDlvrPhone;
			GetMainCityID(&DefCityID);
		}
		int    setDTS(const CheckPaneDialog::ExtCcData * pData)
		{
			int    ok = 1;
			{
				const  PPID preserve_sc_id = Data.SCardID_;
				Data = *pData;
				SETIFZ(Data.SCardID_, preserve_sc_id);
			}
			setCtrlString(CTL_CCHKDLVR_MEMO, Data.Memo);
			AddClusterAssoc(CTL_CCHKDLVR_FLAGS, 0, Data.fDelivery);
			SetClusterData(CTL_CCHKDLVR_FLAGS, Data.Flags);
			SetupPPObjCombo(this, CTLSEL_CCHKDLVR_CITY, PPOBJ_WORLD, NZOR(Data.Addr_.CityID, 0/*DefCityID*/),
				0/*OLW_LOADDEFONOPEN*/, PPObjWorld::MakeExtraParam(WORLDOBJ_CITY, 0, 0));
			SetupDeliveryCtrls(0);
			if(DlvrPhone.NotEmpty()) {
				Data.Flags |= Data.fDelivery;
				SetClusterData(CTL_CCHKDLVR_FLAGS, Data.Flags);
				SetupDeliveryCtrls(DlvrPhone);
				ReplyPhone(1);
			}
			if(Data.Flags & Data.fDelivery)
				selectCtrl(CTL_CCHKDLVR_ADDR);
			// @v9.4.5 {
			{
				SCardCtrlGroup::Rec screc;
				screc.SCardID = Data.SCardID_;
				setGroupData(GRP_SCARD, &screc);
			}
			// } @v9.4.5
			return ok;
		}
		int    getDTS(CheckPaneDialog::ExtCcData * pData)
		{
			int    ok = 1;
			SString temp_buf;
			const  PPID preserve_loc_id = CheckAddrModif(0) ? 0 : Data.Addr_.ID;
			const  LDATETIME preserve_init_dtm = Data.InitDtm;
			Data.Clear();
			Data.Addr_.ID = preserve_loc_id;
			Data.InitDtm = preserve_init_dtm;
			getCtrlString(CTL_CCHKDLVR_MEMO, Data.Memo);
			GetClusterData(CTL_CCHKDLVR_FLAGS, &Data.Flags);
			if(Data.Flags & Data.fDelivery) {
				Data.Addr_.Type = LOCTYP_ADDRESS;
				Data.Addr_.CityID = getCtrlLong(CTLSEL_CCHKDLVR_CITY);
				getCtrlString(CTL_CCHKDLVR_ADDR, temp_buf);
				LocationCore::SetExField(&Data.Addr_, LOCEXSTR_SHORTADDR, temp_buf);
				getCtrlString(CTL_CCHKDLVR_PHONE, temp_buf);
				LocationCore::SetExField(&Data.Addr_, LOCEXSTR_PHONE, temp_buf);
				getCtrlString(CTL_CCHKDLVR_CONTACT, temp_buf);
				LocationCore::SetExField(&Data.Addr_, LOCEXSTR_CONTACT, temp_buf);
				Data.DlvrDtm.d = getCtrlDate(CTL_CCHKDLVR_DT);
				Data.DlvrDtm.t = getCtrlTime(CTL_CCHKDLVR_TM);
			}
			else {
			}
			// @v9.4.5 {
			{
				SCardCtrlGroup::Rec screc;
				getGroupData(GRP_SCARD, &screc);
				Data.SCardID_ = screc.SCardID;
			}
			// } @v9.4.5
			ASSIGN_PTR(pData, Data);
			return ok;
		}
		int    getSCardID(PPID * pScID) const
		{
			if(Data.SCardID_) {
				ASSIGN_PTR(pScID, Data.SCardID_);
				return 1;
			}
			else
				return 0;
		}
	private:
		DECL_HANDLE_EVENT
		{
			TDialog::handleEvent(event);
			if(event.isClusterClk(CTL_CCHKDLVR_FLAGS)) {
				GetClusterData(CTL_CCHKDLVR_FLAGS, &Data.Flags);
				SetupDeliveryCtrls(0);
			}
			/* @v9.4.5 else if(event.isCbSelected(CTLSEL_CCHKDLVR_SCARD)) {
				SCardID = getCtrlLong(CTLSEL_CCHKDLVR_SCARD);
			} */
			else if(event.isCmd(cmSelAddrByPhone)) {
				const  uint c = AddrByPhoneList.getCount();
				if(Data.Flags & Data.fDelivery && c) {
					if(c == 1)
						SetupAddr(&AddrByPhoneList.at(0));
					else
						SelectAddrByPhone();
				}
			}
			else if(TVCMD == cmInputUpdated) {
				if(event.isCtlEvent(CTL_CCHKDLVR_PHONE)) {
					static int __lock = 0;
					if(!__lock) {
						__lock = 1;
						ReplyPhone(0); // @v9.4.5 0-->1 // @v9.4.7 1-->0
						CheckAddrModif(1);
						__lock = 0;
					}
				}
				else if(event.isCtlEvent(CTL_CCHKDLVR_ADDR) || event.isCtlEvent(CTL_CCHKDLVR_CONTACT))
					CheckAddrModif(1);
				else
					return;
			}
			else if(TVCMD == cmWSSelected) {
				if(event.isCtlEvent(CTL_CCHKDLVR_SCARD)) {
					SCardCtrlGroup::Rec scgrec;
					getGroupData(GRP_SCARD, &scgrec);
					if(scgrec.SCardID) {
						int    local_ok = 0;
						SCardTbl::Rec sc_rec;
						if(ScObj.Search(scgrec.SCardID, &sc_rec) > 0) {
							if((!sc_rec.LocID || sc_rec.LocID == Data.Addr_.ID) && (!sc_rec.PersonID || sc_rec.PersonID == Data.Addr_.OwnerID)) {
								Data.SCardID_ = scgrec.SCardID;
								local_ok = 1;
							}
						}
						if(!local_ok && scgrec.SCardID) {
							scgrec.SCardID = 0;
							setGroupData(GRP_SCARD, &scgrec);
						}
					}
				}
			}
			else
				return;
			clearEvent(event);
		}
		int  CheckAddrModif(int doSetupCtrls)
		{
			int    is_mod = 0;
			SString org_buf, now_buf;
			if(!LockAddrModChecking && Data.Addr_.ID && Data.Addr_.ID == OrgLocRec.ID) {
				if(Data.Addr_.CityID != OrgLocRec.CityID)
					is_mod = 1;
				else {
					getCtrlString(CTL_CCHKDLVR_ADDR, now_buf);
					LocationCore::GetExField(&OrgLocRec, LOCEXSTR_SHORTADDR, org_buf);
					if(now_buf.CmpNC(org_buf) != 0)
						is_mod = 1;
					else {
						getCtrlString(CTL_CCHKDLVR_PHONE, now_buf);
						LocationCore::GetExField(&OrgLocRec, LOCEXSTR_PHONE, org_buf);
						if(now_buf.CmpNC(org_buf) != 0)
							is_mod = 1;
						else {
							getCtrlString(CTL_CCHKDLVR_CONTACT, now_buf);
							LocationCore::GetExField(&OrgLocRec, LOCEXSTR_CONTACT, org_buf);
							if(now_buf.CmpNC(org_buf) != 0)
								is_mod = 1;
						}
					}
				}
			}
			if(doSetupCtrls) {
				if(is_mod)
					PPLoadText(PPTXT_DLVRADDRMODIFIED, now_buf);
				else
					now_buf = 0;
				setStaticText(CTL_CCHKDLVR_ST_ADDRMOD, now_buf);
			}
			return is_mod;
		}
		void SetupAddr(const AddrByPhoneItem * pEntry)
		{
			if(pEntry) {
				SString temp_buf;
				PersonID = 0;
				if(pEntry->ObjType == PPOBJ_LOCATION && (pEntry->ObjFlags & LOCF_STANDALONE) && PsnObj.LocObj.Search(pEntry->ObjID, &Data.Addr_) > 0) {
					OrgLocRec = Data.Addr_;
				}
				else if(pEntry->ObjType == PPOBJ_PERSON) {
					PersonID = pEntry->ObjID;
					if(pEntry->Phone[0])
						DlvrPhone = pEntry->Phone;
					if(pEntry->AddrID && PsnObj.LocObj.Search(pEntry->AddrID, &Data.Addr_) > 0)
						OrgLocRec = Data.Addr_;
				}
				else {
					MEMSZERO(OrgLocRec);
					if(pEntry->CityID)
						Data.Addr_.CityID = pEntry->CityID;
					LocationCore::SetExField(&Data.Addr_, LOCEXSTR_SHORTADDR, pEntry->Addr);
					LocationCore::SetExField(&Data.Addr_, LOCEXSTR_PHONE, pEntry->Phone);
					LocationCore::SetExField(&Data.Addr_, LOCEXSTR_CONTACT, pEntry->Contact);
					Data.Addr_.Flags |= LOCF_STANDALONE;
					SETIFZ(Data.Addr_.Type, LOCTYP_ADDRESS);
				}
				setStaticText(CTL_CCHKDLVR_ST_ADDRID,  (temp_buf = 0).Cat(Data.Addr_.ID));
				setStaticText(CTL_CCHKDLVR_ST_ADDRMOD, temp_buf = 0);
				// @v9.4.5 {
				{
					PPIDArray sc_list;
					PPID   sc_id = 0;
					if(pEntry->Phone[0]) {
						PPEAddr::Phone::NormalizeStr(pEntry->Phone, temp_buf);
						if(temp_buf.NotEmptyS()) {
							PPIDArray phone_id_list;
							PsnObj.LocObj.P_Tbl->SearchPhoneIndex(temp_buf, 0, phone_id_list);
							for(uint i = 0; i < phone_id_list.getCount(); i++) {
								const PPID ea_id = phone_id_list.get(i);
								EAddrTbl::Rec ea_rec;
								if(PsnObj.LocObj.P_Tbl->GetEAddr(ea_id, &ea_rec) > 0 && ea_rec.LinkObjType == PPOBJ_SCARD) {
									sc_list.add(ea_rec.LinkObjID);
								}
							}
						}
					}
					if(!sc_list.getCount()) {
						if(PersonID) {
							ScObj.P_Tbl->GetListByPerson(PersonID, 0, &sc_list);
						}
						else if(Data.Addr_.ID) {
							ScObj.P_Tbl->GetListByLoc(Data.Addr_.ID, 0, &sc_list);
						}
					}
					if(sc_list.getCount()) {
						sc_id = sc_list.get(0);
					}
					setCtrlLong(CTL_CCHKDLVR_SCARD, sc_id);
				}
				// } @v9.4.5
				SetupDeliveryCtrls(0);
			}
		}
		void SelectAddrByPhone()
		{
			class SelAddrByPhoneDialog : public PPListDialog {
			public:
				SelAddrByPhoneDialog(const TSArray <AddrByPhoneItem> * pData) : PPListDialog(DLG_SELADDRBYPH, CTL_SELADDRBYPH_LIST)
				{
					RVALUEPTR(Data, pData);
					updateList(-1);
				}
				const AddrByPhoneItem * getSelectedItem()
				{
					long   sel_id = 0;
					return (getSelection(&sel_id) && sel_id > 0 && sel_id <= (long)Data.getCount()) ? &Data.at(sel_id-1) : 0;
				}
			private:
				DECL_HANDLE_EVENT
				{
					if(event.isCmd(cmLBDblClk)) {
						long   idx = 0;
						if(getSelection(&idx) && idx > 0 && idx <= (long)Data.getCount())
							TVCMD = cmOK;
					}
					PPListDialog::handleEvent(event);
				}
				virtual int setupList()
				{
					int    ok = 1;
					SString temp_buf;
					PPIDArray sc_list;
					StringSet ss(SLBColumnDelim);
					for(uint i = 0; i < Data.getCount(); i++) {
						const AddrByPhoneItem & r_entry = Data.at(i);
						ss.clear(1);
						sc_list.clear();
						if(r_entry.ObjType == PPOBJ_LOCATION) {
							PPLoadString("address", temp_buf);
							ScObj.P_Tbl->GetListByLoc(r_entry.ObjID, 0, &sc_list);
						}
						else if(r_entry.ObjType == PPOBJ_PERSON) {
							PPLoadString("person", temp_buf);
							ScObj.P_Tbl->GetListByPerson(r_entry.ObjID, 0, &sc_list);
						}
						else
							(temp_buf = 0).Cat(r_entry.ObjType);
						ss.add(temp_buf);
						ss.add((temp_buf = 0).Cat(r_entry.ObjID));
						ss.add(temp_buf = r_entry.Contact);
						ss.add(temp_buf = r_entry.Addr);
						{
							temp_buf = 0;
							if(sc_list.getCount()) {
								PPID   sc_id = sc_list.get(0);
								SCardTbl::Rec sc_rec;
								if(sc_list.getCount() > 1)
									temp_buf.Cat("...");
								if(ScObj.Fetch(sc_id, &sc_rec) > 0)
									temp_buf.Cat(sc_rec.Code);
							}
							ss.add(temp_buf);
						}
						addStringToList(i+1, ss.getBuf());
					}
					return ok;
				}
				TSArray <AddrByPhoneItem> Data;
				PPObjSCard ScObj;
			};
			{
				static int __lock = 0;
				if(!__lock) {
					__lock = 1;
					SelAddrByPhoneDialog * dlg = new SelAddrByPhoneDialog(&AddrByPhoneList);
					if(CheckDialogPtr(&dlg, 1)) {
						if(ExecView(dlg) == cmOK)
							SetupAddr(dlg->getSelectedItem());
					}
					delete dlg;
					__lock = 0;
				}
			}
		}
		int    ReplyPhone(int immSelect)
		{
			AddrByPhoneList.clear();
			if(Data.Flags & Data.fDelivery && CConfig.Flags2 & CCFLG2_INDEXEADDR) {
				SString temp_buf, phone_buf;
				getCtrlString(CTL_CCHKDLVR_PHONE, temp_buf);
				PPEAddr::Phone::NormalizeStr(temp_buf, phone_buf);
				if(phone_buf.NotEmptyS()) {
					PPIDArray addr_list, dlvr_addr_list;
					PPIDArray phone_id_list;
					PsnObj.LocObj.P_Tbl->SearchPhoneIndex(phone_buf, 0, phone_id_list);
					for(uint i = 0; i < phone_id_list.getCount(); i++) {
						EAddrTbl::Rec ea_rec;
						LocationTbl::Rec loc_rec;
						PersonTbl::Rec psn_rec;
						if(PsnObj.LocObj.P_Tbl->GetEAddr(phone_id_list.get(i), &ea_rec) > 0) {
							if(ea_rec.LinkObjType == PPOBJ_LOCATION && PsnObj.LocObj.Search(ea_rec.LinkObjID, &loc_rec) > 0) {
								AddrByPhoneItem ap_item;
								MEMSZERO(ap_item);
								ap_item.ObjType = ea_rec.LinkObjType;
								ap_item.ObjID = ea_rec.LinkObjID;
								ap_item.ObjFlags = loc_rec.Flags;
								ap_item.CityID = loc_rec.CityID;
								phone_buf.CopyTo(ap_item.Phone, sizeof(ap_item.Phone));
								LocationCore::GetExField(&loc_rec, LOCEXSTR_CONTACT, temp_buf);
								temp_buf.CopyTo(ap_item.Contact, sizeof(ap_item.Contact));
								LocationCore::GetExField(&loc_rec, LOCEXSTR_SHORTADDR, temp_buf);
								temp_buf.CopyTo(ap_item.Addr, sizeof(ap_item.Addr));
								AddrByPhoneList.insert(&ap_item);
							}
							else if(ea_rec.LinkObjType == PPOBJ_PERSON && PsnObj.Search(ea_rec.LinkObjID, &psn_rec) > 0) {
								uint j;
								addr_list.clear();
								addr_list.addnz(psn_rec.RLoc);
								PsnObj.GetDlvrLocList(psn_rec.ID, &dlvr_addr_list);
								for(j = 0; j < dlvr_addr_list.getCount(); j++) {
									addr_list.addnz(dlvr_addr_list.get(j));
								}
								addr_list.addnz(psn_rec.MainLoc);
								if(addr_list.getCount() == 0) {
									//
									// ���� � ���������� ��� �� ������ ������, �� ��� ����� ����������
									// ���������� ��� ���������� � ������ � ������ �������, ����
									// ������������ ��� �� ������� � ������ �����, ������� ��� ������� ������.
									//
									addr_list.add(0L);
								}
								for(j = 0; j < addr_list.getCount(); j++) {
									const PPID addr_id = addr_list.get(j);
									AddrByPhoneItem ap_item;
									MEMSZERO(ap_item);
									ap_item.ObjType = ea_rec.LinkObjType;
									ap_item.ObjID = ea_rec.LinkObjID;
									phone_buf.CopyTo(ap_item.Phone, sizeof(ap_item.Phone));
									STRNSCPY(ap_item.Contact, psn_rec.Name);
									ap_item.AddrID = addr_id;
									if(addr_id) {
										PsnObj.LocObj.P_Tbl->GetAddress(addr_id, 0, temp_buf);
										temp_buf.CopyTo(ap_item.Addr, sizeof(ap_item.Addr));
									}
									AddrByPhoneList.insert(&ap_item);
								}
							}
						}
					}
				}
			}
			{
				const  uint c = AddrByPhoneList.getCount();
				enableCommand(cmSelAddrByPhone, BIN(c));
				if(c && immSelect)
					if(c == 1)
						SetupAddr(&AddrByPhoneList.at(0));
					else
						SelectAddrByPhone();
				return BIN(c);
			}
		}
		void   SetupDeliveryCtrls(const char * pPhone)
		{
			SString temp_buf;
			LockAddrModChecking = 1;
			if(Data.Flags & Data.fDelivery) {
				if(LocationCore::IsEmptyAddressRec(Data.Addr_)) {
					if(!isempty(pPhone)) {
						LocationCore::SetExField(&Data.Addr_, LOCEXSTR_PHONE, pPhone);
					}
					else if(Data.SCardID_) {
						SCardTbl::Rec sc_rec;
						PPPersonPacket pack;
						if(ScObj.Search(Data.SCardID_, &sc_rec) > 0 && sc_rec.PersonID && PsnObj.GetPacket(sc_rec.PersonID, &pack, 0) > 0) {
							PersonID = sc_rec.PersonID;
							LocationTbl::Rec loc_rec;
							loc_rec.ID = 0;
							if(!pack.RLoc.IsEmptyAddress())
								loc_rec = pack.RLoc;
							else if(!pack.Loc.IsEmptyAddress())
								loc_rec = pack.Loc;
							if(loc_rec.ID) {
								pack.GetPhones(1, temp_buf);
								if(temp_buf.NotEmptyS())
									LocationCore::SetExField(&loc_rec, LOCEXSTR_PHONE, temp_buf);
								//
								// �������� ������������� ������ ��������� ��� ������ ������
								// �������������� ����.
								//
								loc_rec.ID = 0;
								Data.Addr_ = loc_rec;
							}
						}
					}
				}
				setCtrlLong(CTL_CCHKDLVR_ADDRID, Data.Addr_.ID);
				setCtrlLong(CTLSEL_CCHKDLVR_CITY, NZOR(Data.Addr_.CityID, DefCityID));
				LocationCore::GetExField(&Data.Addr_, LOCEXSTR_SHORTADDR, temp_buf);
				setCtrlString(CTL_CCHKDLVR_ADDR, temp_buf);
				LocationCore::GetExField(&Data.Addr_, LOCEXSTR_PHONE, temp_buf);
				if(temp_buf.Empty() && PersonID)
					temp_buf = DlvrPhone;
				setCtrlString(CTL_CCHKDLVR_PHONE, temp_buf);
				LocationCore::GetExField(&Data.Addr_, LOCEXSTR_CONTACT, temp_buf);
				if(temp_buf.Empty() && PersonID)
					GetPersonName(PersonID, temp_buf);
				setCtrlString(CTL_CCHKDLVR_CONTACT, temp_buf);
				SETIFZ(Data.DlvrDtm.d, getcurdate_());
				setCtrlDate(CTL_CCHKDLVR_DT, Data.DlvrDtm.d);
				setCtrlTime(CTL_CCHKDLVR_TM, Data.DlvrDtm.t);
				if(PersonID) {
					ComboBox * p_cb = (ComboBox *)getCtrlView(CTLSEL_CCHKDLVR_SCARD);
					if(p_cb) {
						PPIDArray sc_list;
						ScObj.P_Tbl->GetListByPerson(PersonID, 0, &sc_list);
						if(sc_list.getCount()) {
							StrAssocArray * p_list = new StrAssocArray;
							if(p_list) {
								for(uint i = 0; i < sc_list.getCount(); i++) {
									SCardTbl::Rec sc_rec;
									if(ScObj.Search(sc_list.get(i), &sc_rec) > 0)
										p_list->Add(sc_rec.ID, sc_rec.Code);
								}
								ListWindow * p_lw = CreateListWindow(p_list, lbtDisposeData | lbtDblClkNotify);
								if(p_lw) {
									p_cb->setListWindow(p_lw);
									if(Data.SCardID_)
										p_cb->TransmitData(+1, &Data.SCardID_);
									else {
										p_cb->setInputLineText(0);
										p_cb->setUndefTag(1);
									}
								}
							}
						}
					}
				}
			}
			disableCtrls(!(Data.Flags & Data.fDelivery), CTL_CCHKDLVR_ADDRID, CTLSEL_CCHKDLVR_CITY,
				CTL_CCHKDLVR_ADDR, CTL_CCHKDLVR_PHONE, CTL_CCHKDLVR_CONTACT, CTL_CCHKDLVR_DT, CTL_CCHKDLVR_TM, 0);
			LockAddrModChecking = 0;
		}

		CheckPaneDialog::ExtCcData Data;
		LocationTbl::Rec OrgLocRec;
		PPID   DefCityID;
		//PPID   _SCardID;
		PPID   PersonID;
		int    LockAddrModChecking;
		SString DlvrPhone;
		const SString Channel;
		TSArray <AddrByPhoneItem> AddrByPhoneList;
		PPObjPerson PsnObj;
		PPObjSCard ScObj;
	};
	int    ok = -1;
	PPID   sc_id = 0;
	int    preserve_delivery_flag = BIN(P.Eccd.Flags & P.Eccd.fDelivery);
	if(LocationCore::IsEmptyAddressRec(P.Eccd.Addr_)) {
		if(P.Eccd.Memo.Empty() && CnSpeciality == PPCashNode::spDelivery)
			P.Eccd.Flags |= P.Eccd.fDelivery;
		sc_id = CSt.GetID();
	}
	CheckDlvrDialog * dlg = new CheckDlvrDialog(sc_id, pDlvrPhone, pChannel);
	if(CheckDialogPtr(&dlg, 1) && dlg->setDTS(&P.Eccd)) {
		while(ok <= 0 && ExecView(dlg) == cmOK) {
			if(dlg->getDTS(&P.Eccd)) {
				if(dlg->getSCardID(&sc_id) && sc_id != CSt.GetID())
					AcceptSCard(0, sc_id);
				ok = 1;
			}
		}
		if(ok < 0) {
			SETFLAG(P.Eccd.Flags, P.Eccd.fDelivery, preserve_delivery_flag);
		}
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

#undef BARRIER
#define BARRIER(f) if(!Barrier()) { f; Barrier(1); }

int CheckPaneDialog::ProcessPhnSvc(int mode)
{
	int    ok = 1;
	int    pop_dlvr_pane = 0;
	SString phone_buf, channel_buf, caller_buf;
	if(P_PhnSvcClient) {
		if(mode == 1) {
			PhnSvcChannelStatusPool status_list;
			PhnSvcChannelStatus cnl_status;
			SString ringing_line;
			if(P_PhnSvcClient->GetChannelStatus(0, status_list)) {
				if(status_list.GetCount()) {
					PPEAddrArray phn_list; // ������ ���������, ����������� � ������. ��������� ��� ���������� ����������� �����.
					PPIDArray ea_id_list;
					SString contact_buf;
					for(uint i = 0; !pop_dlvr_pane && i < status_list.GetCount(); i++) {
						status_list.Get(i, cnl_status);
						if(cnl_status.State == PhnSvcChannelStatus::stUp) {
							if(cnl_status.Channel.CmpPrefix("SIP", 1) == 0) {
								if(cnl_status.ConnectedLineNum.Empty() || cnl_status.ConnectedLineNum.ToLong() != 0) {
									if(PhnSvcLocalChannelSymb.NotEmpty() && cnl_status.Channel.NotEmpty() && (cnl_status.Channel.CmpPrefix(PhnSvcLocalChannelSymb, 1) == 0 || PhnSvcLocalChannelSymb.CmpNC("@all") == 0)) {
										if(CnSpeciality == PPCashNode::spDelivery && !(P.Eccd.Flags & P.Eccd.fDelivery) && IsState(sEMPTYLIST_EMPTYBUF) && !(Flags & fBarrier)) {
											pop_dlvr_pane = 1;
											if(cnl_status.ConnectedLineNum.Len() > cnl_status.CallerId.Len())
												phone_buf = cnl_status.ConnectedLineNum;
											else
												phone_buf = cnl_status.CallerId;
											channel_buf = cnl_status.Channel;
										}
									}
								}
							}
						}
						else if(cnl_status.State == PhnSvcChannelStatus::stRinging) {
							if(cnl_status.ConnectedLineNum.Len() > cnl_status.CallerId.Len())
								caller_buf = cnl_status.ConnectedLineNum;
							else
								caller_buf = cnl_status.CallerId;
							if(caller_buf.Len() && !phn_list.SearchPhone(caller_buf, 0, 0)) {
								if(ringing_line.NotEmpty())
									ringing_line.CR();
								ringing_line.Cat(cnl_status.Channel).CatDiv(':', 2).Cat(caller_buf);
								phn_list.AddPhone(caller_buf);
								PsnObj.LocObj.P_Tbl->SearchPhoneIndex(caller_buf, 0, ea_id_list);
								contact_buf = 0;
								for(uint j = 0; !contact_buf.NotEmpty() && j < ea_id_list.getCount(); j++) {
									EAddrTbl::Rec ea_rec;
									if(PsnObj.LocObj.P_Tbl->GetEAddr(ea_id_list.get(j), &ea_rec) > 0) {
										if(ea_rec.LinkObjType == PPOBJ_PERSON) {
											GetPersonName(ea_rec.LinkObjID, contact_buf);
										}
										else if(ea_rec.LinkObjType == PPOBJ_LOCATION) {
											LocationTbl::Rec loc_rec;
											if(PsnObj.LocObj.Search(ea_rec.LinkObjID, &loc_rec) > 0)
												LocationCore::GetExField(&loc_rec, LOCEXSTR_CONTACT, contact_buf);
										}
									}
								}
								if(!contact_buf.NotEmpty()) {
									contact_buf = "UNKNOWN";
								}
								ringing_line.CatDiv(';', 2).Cat(contact_buf);
							}
						}
					}
				}
				if(ringing_line.NotEmpty()) {
					SMessageWindow * p_win = new SMessageWindow;
					if(p_win) {
						p_win->Open(ringing_line, 0, H(), 0, 10000, GetColorRef(SClrCadetblue),
							SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus, 0);
					}
				}
			}
		}
	}
	else
		ok = -1;
	if(pop_dlvr_pane) {
		BARRIER(EditMemo(phone_buf, channel_buf));
	}
	return ok;
}

struct SelectGuestCountParam {
	SelectGuestCountParam()
	{
		TableNo = 0;
		GuestCount = 0;
	}
	int    TableNo;
	int    GuestCount;
};

class SelectGuestCountDialog : public TDialog {
public:
	SelectGuestCountDialog() : TDialog(DLG_SELGUESTCOUNT)
	{
	}
	int    setDTS(const SelectGuestCountParam * pData)
	{
		RVALUEPTR(Data, pData);
		return 1;
	}
	int    getDTS(SelectGuestCountParam * pData)
	{
		ASSIGN_PTR(pData, Data);
		return 1;
	}
private:
	DECL_HANDLE_EVENT
	{
		TDialog::handleEvent(event);
		if(TVCMD >= cmSelGuestCount01 && TVCMD <= cmSelGuestCount15) {
			Data.GuestCount = (TVCMD - cmSelGuestCount01 + 1);
			endModal(cmOK);
		}
	}
	SelectGuestCountParam Data;
};

int CheckPaneDialog::SelectGuestCount(int tableCode, long * pGuestCount)
{
	int    ok = -1;
	SelectGuestCountDialog * dlg = 0;
	SelectGuestCountParam param;
	if(tableCode) {
		param.TableNo = tableCode;
		param.GuestCount = 0;
		if(CheckDialogPtr(&(dlg = new SelectGuestCountDialog), 1)) {
			dlg->setDTS(&param);
			while(ok < 0 && ExecView(dlg) == cmOK) {
				dlg->getDTS(&param);
				if(param.GuestCount > 0 && param.GuestCount < 1000)
					ok = 1;
				else
					param.GuestCount = 0;
			}
		}
	}
	ASSIGN_PTR(pGuestCount, param.GuestCount);
	delete dlg;
	return ok;
}

IMPL_HANDLE_EVENT(CheckPaneDialog)
{
	const int  prev_state = GetState();
	const PPID prev_agent_id = P.GetAgentID(1); // @v8.2.0
	if(TVCOMMAND) {
		if(TVCMD == cmInputUpdated)
			IdleClock = clock(); // �� ������������ ��� ���������, � ���� ��������� ������� ��������� //
		else if(TVCMD == cmCtlColor) {
			TDrawCtrlData * p_dc = (TDrawCtrlData *)TVINFOPTR;
			if(p_dc) {
				if(p_dc->Src == TDrawCtrlData::cScrollBar) {
					int ctl_id = GetDlgCtrlID(p_dc->H_Ctl);
					if(ctl_id && oneof2(ctl_id, MAKE_BUTTON_ID(CTL_CHKPAN_GRPLIST, 1), MAKE_BUTTON_ID(CTL_CHKPAN_GDSLIST, 1))) {
						::SendMessage(::GetDlgItem(H(), CTL_CHKPAN_ARROW_UP),   BM_SETSTYLE, BS_BITMAP, TRUE);
						::SendMessage(::GetDlgItem(H(), CTL_CHKPAN_ARROW_DOWN), BM_SETSTYLE, BS_BITMAP, TRUE);
					}
				}
				else {
					if(p_dc->H_Ctl == getCtrlHandle(CTL_CHKPAN_INFO)) {
						TCanvas canv(p_dc->H_DC);
						::SetBkMode(p_dc->H_DC, TRANSPARENT);
						if(Flags & fError) {
							canv.SetTextColor(GetColorRef(SClrWhite));
							p_dc->H_Br = (HBRUSH)Ptb.Get(brErrorBkg);
							clearEvent(event);
						}
						else if(Flags & fPresent) {
							canv.SetTextColor(GetColorRef(SClrWhite));
							p_dc->H_Br = (HBRUSH)Ptb.Get(brPresentBkg);
							clearEvent(event);
						}
					}
					else if(p_dc->H_Ctl == getCtrlHandle(CTL_CHKPAN_CAFE_STATUS)) {
						if(P.OrderCheckID) {
							TCanvas canv(p_dc->H_DC);
							::SetBkMode(p_dc->H_DC, TRANSPARENT);
							//canv.SetTextColor(GetColorRef(SClrWhite));
							p_dc->H_Br = (HBRUSH)Ptb.Get(brOrderBkg);
							clearEvent(event);
						}
					}
					else if(p_dc->H_Ctl == getCtrlHandle(CTL_CHKPAN_TOTAL)) {
						::SetBkMode(p_dc->H_DC, TRANSPARENT);
						for(uint i = 0; i < P.getCount(); i++) {
							if(P.at(i).Flags & cifGift) {
								p_dc->H_Br = (HBRUSH)Ptb.Get(brTotalGift);
								clearEvent(event);
								break;
							}
						}
					}
					else if(p_dc->H_Ctl == getCtrlHandle(CTL_CHKPAN_DISCOUNT)) {
						::SetBkMode(p_dc->H_DC, TRANSPARENT);
						for(uint i = 0; i < P.getCount(); i++) {
							if(P.at(i).Flags & cifGift && P.at(i).Discount != 0.0) {
								p_dc->H_Br = (HBRUSH)Ptb.Get(brDiscountGift);
								clearEvent(event);
								break;
							}
						}
					}
				}
			}
			return;
		}
		else if(Flags & fAsSelector) {
			if(TVCMD == cmLBDblClk)
				TVCMD = cmOK;
			if(TVCMD == cmOK) {
				if(P_ChkPack) {
					CCheckLineTbl::Rec * p_line = 0, chk_line;
					SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_CHKPAN_LIST);
					if(p_list) {
						long  cur = p_list->def ? p_list->def->_curItem() : -1;
						if(cur >= 0 && cur < (long)P_ChkPack->GetCount()) {
							chk_line = P_ChkPack->GetLine((uint)cur);
							p_line   = &chk_line;
						}
					}
					P_ChkPack->Init();
					if(p_line) {
						if(!P_ChkPack->InsertItem(p_line->GoodsID, p_line->Quantity, intmnytodbl(p_line->Price), p_line->Dscnt))
							PPErrorZ();
						else
							P_ChkPack->Discount = p_line->Dscnt;
					}
					else
						P_ChkPack->Discount = 0.0;
				}
			}
		}
		else if(TVCMD == cmOK) {
			BARRIER(ProcessEnter(0));
			clearEvent(event);
		}
	}
	TDialog::handleEvent(event);
	if(TVBROADCAST) {
		if(TVCMD == cmIdle) {
			clock_t diff = 0;
			if(P_PalmWaiter && IsState(sEMPTYLIST_EMPTYBUF))
				P_PalmWaiter->Activate();
			if(CnSleepTimeout && !(Flags & fSuspSleepTimeout)) {
				diff = clock() - IdleClock;
				if(diff >= CnSleepTimeout)
					Sleep();
			}
			if(PrintCheckClock) {
				diff = clock() - PrintCheckClock;
				if(diff >= ClearCDYTimeout) {
					PrintCheckClock = 0;
					CDispCommand(cdispcmdText, cdisptxtOpened, 0.0, 0.0);
				}
			}
			if(PhnSvcTimer.Check(0)) {
				ProcessPhnSvc(1);
			}
			// @v8.3.2 {
			if(UhttImportTimer.Check(0) && P_UhttImporter) {
				PPAlbatrosConfig acfg;
				PPAlbatrosCfgMngr::Get(&acfg);
				P_UhttImporter->InitUhttImport(acfg.Hdr.OpID, CnLocID, CashNodeID);
				if(P_UhttImporter->Run() > 0) {
					SMessageWindow::DestroyByParent(H()); // ������� � ������ ���������� �����������
					SMessageWindow * p_win = new SMessageWindow;
					if(p_win) {
						SString msg_buf;
						PPLoadText(PPTXT_CHKPAN_UHTTORDER, msg_buf);
						p_win->Open(msg_buf, 0, H(), 0, 10000, GetColorRef(SClrCornsilk),
							SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus, 0);
					}
				}
			}
			// } @v8.3.2
			clearEvent(event);
			return;
		}
		else if(TVCMD == cmDefault && isCurrCtlID(CTL_CHKPAN_INPUT)) {
			BARRIER(ProcessEnter(0));
		}
		else if(TVCMD == cmReleasedFocus && (Flags & fTouchScreen))
			LastCtrlID = event.message.infoView->GetId();
		else
			return;
	}
	else if(TVCOMMAND) {
		const uint ev_ctl_id = event.getCtlID();
		switch(TVCMD) {
			case cmSetupResizeParams:
				SetDlgResizeParams();
				break;
			case cmSetupTooltip:
				{
					SString  tt_names, name;
					if(PPLoadTextWin(PPTXT_CHKPAN_TOOLTIPS, tt_names)) {
						for(uint idx = 0; idx < CTL_CHKPAN_NUMBUTTONS; idx++) {
							if(PPGetSubStr(tt_names, idx, name))
								SetCtrlToolTip(CTL_CHKPAN_STARTBUTTON + idx, name);
						}
					}
					if(CnSpeciality == PPCashNode::spCafe) {
						PPLoadString("guestcount", name);
						SetCtrlToolTip(CTL_CHKPAN_BYPRICE, name.Transf(CTRANSF_INNER_TO_OUTER));
						PPLoadString("ftableorders", name = 0);
						SetCtrlToolTip(CTL_CHKPAN_DIVISION, name.Transf(CTRANSF_INNER_TO_OUTER));
					}
					else if(CnSpeciality == PPCashNode::spDelivery) {
						PPLoadString("delivery", name = 0);
						SetCtrlToolTip(CTL_CHKPAN_DIVISION, name.Transf(CTRANSF_INNER_TO_OUTER));
					}
				}
				break;
			case cmDrawItem:
				DrawListItem((TDrawItemData *)TVINFOPTR);
				break;
			case cmInputDblClk:
				if(!Barrier()) {
					if(TVINFOVIEW && TVINFOVIEW->GetId() == CTL_CHKPAN_INPUT)
						EditMemo(0, 0);
					Barrier(1);
				}
				break;
			case cmaInsert:
				if(!Barrier()) {
					if(GetInput())
						ProcessEnter(1);
					else if(LastCtrlID == CTL_CHKPAN_GRPLIST)
						UpdateGList(1, 0);
					else if(LastCtrlID == CTL_CHKPAN_GDSLIST) {
						SelectGoods__(sgmInnerGoodsList);
					}
					else if(oneof3(GetState(), sEMPTYLIST_BUF, sLIST_BUF, sLISTSEL_BUF) && LastCtrlID != CTL_CHKPAN_LIST)
						ProcessEnter(1);
					Barrier(1);
				}
				break;
			case cmaDelete:
				if(ev_ctl_id == CTL_CHKPAN_LIST) {
					BARRIER(RemoveRow());
				}
				break;
			case cmaEdit:
				if(ev_ctl_id == CTL_CHKPAN_LIST) {
					int  no_edit = 1;
				}
				break;
			case cmaSelect:
				if(!Barrier()) {
					GroupList.TopID = 0;
					UpdateGList(0, 0);
					Barrier(1);
				}
				break;
			case cmaLevelUp:
				if(!Barrier()) {
					PPID   _sel_id = GroupList.TopID;
					if(UiFlags & uifOneGroupLevel && GroupList.TopID) {
						if(ActiveListID == CTL_CHKPAN_GDSLIST) {
							_sel_id = 0;
						}
						else {
							GrpListItem * p_item = GroupList.Get(GroupList.TopID, 0);
							GroupList.TopID = p_item ? p_item->ParentID : 0;
						}
					}
					UpdateGList(0, _sel_id);
					Barrier(1);
				}
				break;
			case cmaAltSelect:
				BARRIER(UpdateGList(-1, AltGoodsGrpID));
				break;
			case cmLBItemSelected:
			case cmLBDblClk:
				if(!Barrier()) {
					if(ev_ctl_id == CTL_CHKPAN_GRPLIST) {
						PPID   grp_id = 0;
						int    r = SelectGroup(&grp_id);
						if(r == 2)
							UpdateGList(-1, grp_id);
						else if(r == 1)
							UpdateGList(0, grp_id);
					}
					else if(ev_ctl_id == CTL_CHKPAN_GDSLIST) {
						SelectGoods__(sgmInnerGoodsList);
					}
					else if(ev_ctl_id == CTL_CHKPAN_LIST) {
						if(!(Flags & fNoEdit)) {
							SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_CHKPAN_LIST);
							if(p_list) {
								long   cur = p_list->def ? p_list->def->_curItem() : -1;
								if(cur >= 0 && cur < (long)P.getCount()) {
									enum {
										rowopNone = 0,
										rowopUp,
										rowopDown,
										rowopDoGroup,
										rowopDelete
									};
									int    r = -1;
									CCheckItem & r_cur_item = P.at(cur);
									long   verb = rowopNone;
									int8   queue = r_cur_item.Queue;
									TDialog * dlg = new TDialog(DLG_CHKPANROWOP);
									if(CheckDialogPtr(&dlg, 1)) {
										dlg->AddClusterAssoc(CTL_CHKPANROWOP_VERB, 0, rowopNone);
										dlg->AddClusterAssoc(CTL_CHKPANROWOP_VERB, -1, rowopNone);
										dlg->AddClusterAssoc(CTL_CHKPANROWOP_VERB, 1, rowopUp);
										dlg->AddClusterAssoc(CTL_CHKPANROWOP_VERB, 2, rowopDown);
										dlg->AddClusterAssoc(CTL_CHKPANROWOP_VERB, 3, rowopDoGroup);
										dlg->AddClusterAssoc(CTL_CHKPANROWOP_VERB, 4, rowopDelete);
										dlg->SetClusterData(CTL_CHKPANROWOP_VERB, verb);
										dlg->SetupSpin(CTLSPIN_CHKPANROWOP_QUEUE, CTL_CHKPANROWOP_QUEUE, 0, 20, queue);
										dlg->setCtrlData(CTL_CHKPANROWOP_QUEUE, &queue);
										if(cur == 0) {
											dlg->DisableClusterItem(CTL_CHKPANROWOP_VERB, 1, 1);
											dlg->DisableClusterItem(CTL_CHKPANROWOP_VERB, 3, 1);
										}
										if((cur+1) >= ((long)P.getCount())) {
											dlg->DisableClusterItem(CTL_CHKPANROWOP_VERB, 2, 1);
										}
										if(!(OperRightsFlags & orfEscChkLine) || cur < 0 || cur >= (long)P.getCount()) {
											dlg->DisableClusterItem(CTL_CHKPANROWOP_VERB, 4, 1);
										}
										while(r < 0 && ExecView(dlg) == cmOK) {
											verb = dlg->GetClusterData(CTL_CHKPANROWOP_VERB);
											dlg->getCtrlData(CTL_CHKPANROWOP_QUEUE, &queue);
											r = 1;
											if(queue != r_cur_item.Queue) {
												if(P.SetQueue(cur, queue) > 0)
													r = 2;
											}
											switch(verb) {
												case rowopUp:
													if(P.MoveUp(cur) > 0)
														r = 2;
													break;
												case rowopDown:
													if(P.MoveDown(cur) > 0)
														r = 2;
													break;
												case rowopDoGroup:
													if(P.Grouping(cur) > 0)
														r = 2;
													break;
												case rowopDelete:
													RemoveRow();
													break;
											}
										}
									}
									if(r == 2)
										OnUpdateList(0);
									delete dlg;
								}
								/*
								TMenuPopup menu;
								if(cur > 0) {
									menu.Add("@moveup",   cmUp);
								}
								if((cur+1) < ((long)P.getCount()))
									menu.Add("@movedown", cmDown);
								if(cur > 0)
									menu.Add("@dogroup",  cmGrouping);
								if(OperRightsFlags & orfEscChkLine && cur >= 0 && cur < (long)P.getCount()) {
									menu.AddSeparator();
									menu.Add("@delete",   cmaDelete);
								}
								if(menu.GetCount()) {
									int    cmd = menu.Execute(*this, TMenuPopup::efRet);
									switch(cmd) {
										case cmUp:
											if(P.MoveUp(cur) > 0)
												updateList();
											break;
										case cmDown:
											if(P.MoveDown(cur) > 0)
												updateList();
											break;
										case cmGrouping:
											if(P.Grouping(cur) > 0)
												updateList();
											break;
										case cmaDelete:
											RemoveRow();
											break;
									}
								}
								*/
							}
						}
					}
					Barrier(1);
				}
				break;
			case cmLBItemFocused:
				if(ActiveListID == CTL_CHKPAN_GRPLIST) {
					SmartListBox * p_list = (SmartListBox *)getCtrlView(CTL_CHKPAN_GRPLIST);
					CALLPTRMEMB(p_list, getCurID(&SelGoodsGrpID));
				}
				if(event.isCtlEvent(CTL_CHKPAN_LIST)) {
					SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_CHKPAN_LIST);
					if(p_list && p_list->def) {
						uint pos = p_list->def->_curItem();
						if(pos < P.getCount())
							ViewStoragePlaces(P.at(p_list->def->_curItem()).GoodsID);
					}
				}
				break;
			case cmPrev:
			case cmNext:
				if(ActiveListID) {
					if(!Barrier()) {
						SmartListBox * p_list = (SmartListBox *)getCtrlView((ushort)ActiveListID);
						if(p_list)
							p_list->Scroll((TVCMD == cmPrev) ? SB_LINEUP : SB_LINEDOWN, 0);
						Barrier(1);
					}
				}
				break;
			case cmCash:
				if(!Barrier()) {
					ClearInput(0);
					Flags &= ~fBankingPayment; // @bank
					ProcessEnter(0);
					Barrier(1);
				}
				break;
			case cmBanking:
				if(!Barrier()) {
					ClearInput(0);
					Flags |= fBankingPayment; // @bank
					ProcessEnter(0);
					Barrier(1);
				}
				break;
			case cmChkPanPrint:
				if(!Barrier()) {
					if(Flags & fTouchScreen) {
						if(Flags & fPrintSlipDoc)
							PrintSlipDocument();
						else
							PrintToLocalPrinters(0);
					}
					else if(Flags & fNoEdit)
						Print(0, 0, 0);
					else
						PrintToLocalPrinters(0);
					Barrier(1);
				}
				break;
			case cmQuantity:
				BARRIER(AcceptQuantity());
				break;
			case cmRetCheck:
				BARRIER(setupRetCheck(F(fRetCheck) ? 0 : 1));
				break;
			case cmSelSCard:
				BARRIER(AcceptSCard(((Flags & fWaitOnSCard) ? 1 : 100), 0));
				break;
			case cmChkPanSuspend:
				BARRIER(SuspendCheck());
				break;
			case cmToLocPrinters:
				BARRIER(PrintToLocalPrinters(-1));
				break;
			case cmSelTable:
				BARRIER(SelectTable());
				break;
			case cmCashOper:
				if(!Barrier() || (Flags & fOnlyReports))
					Barrier((PrintCashReports() < 0) ? 0 : 1);
				break;
			case cmSelModifier:
				BARRIER(SelectGoods__(sgmModifier));
				break;
			case cmSelGoods:
				BARRIER(SelectGoods__(sgmNormal));
				break;
			case cmByPrice: /* cmChkPanF2 */
				if(!Barrier()) {
					if(CnSpeciality == PPCashNode::spCafe) {
						//
						// ����� ���������� ������ �� ������ (P.GuestCount)
						//
						if(P.TableCode) {
							int    is_input = GetInput();
							long   guest_count = 0;
							if(!is_input) {
								if(SelectGuestCount(P.TableCode, &guest_count) > 0) {
									P.GuestCount = (uint16)guest_count;
									SetupInfo(0);
								}
							}
							else {
								if(Input.IsDigit()) {
									guest_count = Input.ToLong();
									if(guest_count > 0 && guest_count < 1000) {
										P.GuestCount = (uint16)guest_count;
										SetupInfo(0);
									}
								}
								ClearInput(0);
							}
						}
					}
					else if(Flags & fTouchScreen) {
						UpdateGList(-2, 0);
					}
					else {
						SelectGoods__(sgmByPrice);
					}
					Barrier(1);
				}
				break;
			case cmGoodsDiv: /* cmChkPanF1 */
				if(!Barrier()) {
					if(CnSpeciality == PPCashNode::spCafe) {
						if(IsState(sEMPTYLIST_EMPTYBUF)) {
							SelCheckListDialog::AddedParam param(/* @v8.4.3 CashNodeID */0, P.TableCode, P.GetAgentID(), OperRightsFlags);
							const uint dlg_id = (DlgFlags & fLarge) ? DLG_ORDERCHECKS_L : DLG_ORDERCHECKS;
							SelCheckListDialog * dlg = new SelCheckListDialog(dlg_id, (TSArray <CCheckViewItem> *)0, 0, this, &param);
							if(CheckDialogPtr(&dlg, 1)) {
								if(ExecView(dlg) == cmNewCheck) {
									_SelCheck sc;
									if(dlg->getDTS(&sc)) {
										SetupOrder(sc.CheckID);
									}
								}
								delete dlg;
							}
						}
					}
					else if(CnSpeciality == PPCashNode::spDelivery) {
						EditMemo(0, 0);
					}
					else
						AcceptDivision();
					Barrier(1);
				}
				break;
			case cmMemo:
				BARRIER(EditMemo(0, 0));
				break;
			// @v8.2.8 {
			case cmSysInfo:
				if(P_ChkPack) {
					CCheckPacket pack = *P_ChkPack;
					PPViewCCheck::EditCCheckSystemInfo(pack);
				}
				break;
			// } @v8.2.8
			default:
				return;
		}
	}
	else if(TVKEYDOWN) {
		switch(TVKEY) {
#ifndef NDEBUG // {
			case kbF12:
				{
					SString temp_buf, file_name;
					if(ExportCurrentState(temp_buf)) {
						PPGetFilePath(PPPATH_OUT, "CPosProcessorState.xml", file_name);
						SFile f_out(file_name, SFile::mWrite);
						f_out.WriteLine(temp_buf);
					}
					else
						PPError();
					if(ExportCTblList(temp_buf)) {
						PPGetFilePath(PPPATH_OUT, "CPosCTableList.xml", file_name);
						SFile f_out(file_name, SFile::mWrite);
						f_out.WriteLine(temp_buf);
					}
					else
						PPError();
					if(ExportCCheckList(0, temp_buf)) {
						PPGetFilePath(PPPATH_OUT, "CPosCCheckList.xml", file_name);
						SFile f_out(file_name, SFile::mWrite);
						f_out.WriteLine(temp_buf);
					}
					else
						PPError();
				}
				break;
#endif // } !NDEBUG
			case kbF2:
				BARRIER(SelectGoods__(sgmNormal));
				break;
			case kbF3:
				BARRIER(AcceptSCard(((Flags & fWaitOnSCard) ? 1 : 100), 0));
				break;
			case kbCtrlF3:
				if(InitCashMachine() && P_CM->GetNodeData().Flags & CASHF_OPENBOX)
					P_CM->SyncOpenBox();
				break;
			case kbF4:
				BARRIER(SelectGoods__(sgmByPrice));
				break;
			case kbF5:
				BARRIER(SetupRowByScale());
				break;
			case kbCtrlF5:
				BARRIER(setupRetCheck(F(fRetCheck) ? 0 : 1));
				break;
			case kbF6:
				BARRIER(AcceptQuantity());
				break;
			case kbCtrlF6:
				if(!Barrier()) {
					if(P.HasCur() && P.GetCur().GoodsID) {
						PPID   goods_id = P.GetCur().GoodsID;
						GObj.ViewUhttGoodsRestList(goods_id);
					}
					Barrier(1);
				}
			case kbF7:
				BARRIER(PrintCheckCopy());
				break;
			case kbCtrlF7:
				BARRIER(PrintSlipDocument());
				break;
			case kbAltF7:
				BARRIER(PrintToLocalPrinters(1));
				break;
			case kbShiftF7:
				BARRIER(PrintToLocalPrinters(0));
				break;
			case kbF8:
				BARRIER(SuspendCheck());
				break;
			case kbCtrlF8: // �������� ���������� � ����������� �����
				if(!Barrier()) {
					PPID   scard_id = CSt.GetID();
					ViewSCardInfo(&scard_id, 1);
					Barrier(1);
				}
				break;
			case kbF9:
				BARRIER(AcceptDivision());
				break;
			case kbCtrlF9:
				if(!Barrier() || (Flags & fOnlyReports))
					Barrier((PrintCashReports() < 0) ? 0 : 1);
				break;
			case kbF10:
				BARRIER(EditMemo(0, 0));
				break;
			case kbF11:
				BARRIER(ResetOperRightsByKey());
				break;
			case kbDown:
			case kbPgDn:
			case kbUp:
			case kbPgUp:
				if(ActiveListID) {
					SmartListBox * p_list = (SmartListBox *)getCtrlView((ushort)ActiveListID);
					if(p_list) {
						int   scroll_code = -1;
						switch(TVKEY) {
							case kbUp:    scroll_code = SB_LINEUP;   break;
							case kbDown:  scroll_code = SB_LINEDOWN; break;
							case kbPgDn:  scroll_code = SB_PAGEDOWN; break;
							case kbPgUp:  scroll_code = SB_PAGEUP;   break;
						}
						if(scroll_code != -1)
							p_list->Scroll(scroll_code, 0);
					}
				}
				break;
			default:
				return;
		}
	}
	else
		return;
	if(!(Flags & fBarrier) && (GetState() != prev_state || P.GetAgentID(1) != prev_agent_id))
		setupHint();
	clearEvent(event);
	IdleClock = clock();
}

#undef BARRIER

void CheckPaneDialog::DrawListItem(TDrawItemData * pDrawItem)
{
	if(pDrawItem && pDrawItem->P_View) {
		PPID   list_ctrl_id = pDrawItem->P_View->GetId();
		if(list_ctrl_id == ActiveListID) {
			HDC    h_dc = pDrawItem->H_DC;
			HFONT  h_fnt_def  = 0;
			HBRUSH h_br_def   = 0;
			HPEN   h_pen_def  = 0;
			COLORREF clr_prev = 0;
			SmartListBox * p_lbx = (SmartListBox *)pDrawItem->P_View;
			RECT   rc = pDrawItem->ItemRect;
			// char   temp_buf[256];
			SString temp_buf;
			if(list_ctrl_id == CTL_CHKPAN_GDSLIST) {
				if(pDrawItem->ItemAction & TDrawItemData::iaBackground) {
					FillRect(h_dc, &rc, (HBRUSH)Ptb.Get(brOdd));
					pDrawItem->ItemAction = 0; // �� ������������ ���
				}
				else if(pDrawItem->ItemID != 0xffffffff) {
					h_fnt_def = (HFONT)SelectObject(h_dc, (HFONT)Ptb.Get(fontGoodsList));
					//p_lbx->getText((long)pDrawItem->ItemData, temp_buf, sizeof(temp_buf));
					//SOemToChar(temp_buf);
					p_lbx->getText((long)pDrawItem->ItemData, temp_buf);
					temp_buf.Transf(CTRANSF_INNER_TO_OUTER);
					if(pDrawItem->ItemState & (ODS_FOCUS|ODS_SELECTED)) {
						h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get(brSel));
						clr_prev = SetBkColor(h_dc, Ptb.GetColor(clrFocus));
						if(UiFlags & uifTSGGroupsAsButtons) {
							SInflateRect(rc, -1, -(1 + GoodsListEntryGap / 4));
							RoundRect(h_dc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
							rc.left += 4;
						}
						else
							FillRect(h_dc, &rc, (HBRUSH)Ptb.Get(brSel));
					}
					else {
						int  draw_odd = pDrawItem->ItemID % 2;
						if(UiFlags & uifTSGGroupsAsButtons) {
							clr_prev = SetBkColor(h_dc, Ptb.GetColor(clrGrp));
							h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get(brGrp));
							SInflateRect(rc, -1, -(1 + GoodsListEntryGap / 4));
							RoundRect(h_dc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
							rc.left += 4;
						}
						else {
							h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get(draw_odd ? brOdd : brEven));
							clr_prev = SetBkColor(h_dc, Ptb.GetColor(draw_odd ? clrOdd : clrEven));
							FillRect(h_dc, &rc, (HBRUSH)Ptb.Get(draw_odd ? brOdd : brEven));
						}
					}
					::DrawText(h_dc, temp_buf.cptr(), temp_buf.Len(), &rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE); // @unicodeproblem
				}
			}
			else if(list_ctrl_id == CTL_CHKPAN_GRPLIST) {
				uint   level = 0;
				if(pDrawItem->ItemAction & TDrawItemData::iaBackground) {
					clr_prev = SetBkColor(h_dc, Ptb.GetColor(clrGrp));
					FillRect(h_dc, &rc, (HBRUSH)Ptb.Get(brGrp));
					pDrawItem->ItemAction = 0; // �� ������������ ���
				}
				else if(pDrawItem->ItemID != 0xffffffff) {
					GrpListItem gli;
					MEMSZERO(gli);
					if(p_lbx && p_lbx->def) {
						uint   pos = 0;
						const  void * p_row_data = p_lbx->def->getRow_((long)pDrawItem->ItemData);
						PPID   grp_id = p_row_data ? *(long *)p_row_data : 0;
						if(GroupList.Get(grp_id, &pos))
							gli = GroupList.at(pos);
					}
					h_fnt_def = (HFONT)SelectObject(h_dc, Ptb.Get(fontGoodsList));
					//p_lbx->getText((long)pDrawItem->ItemData, temp_buf, sizeof(temp_buf));
					//SOemToChar(temp_buf);
					p_lbx->getText((long)pDrawItem->ItemData, temp_buf);
					temp_buf.Transf(CTRANSF_INNER_TO_OUTER);
					if(pDrawItem->ItemState & (ODS_FOCUS | ODS_SELECTED)) {
						clr_prev = SetBkColor(h_dc, Ptb.GetColor(clrFocus));
						h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get(brGrpSel));
						if(UiFlags & uifTSGGroupsAsButtons) {
							SInflateRect(rc, -1, -(1 + GoodsListEntryGap / 4));
							RoundRect(h_dc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
						}
						else
							FillRect(h_dc, &rc, (HBRUSH)Ptb.Get(brGrpSel));
					}
					else {
						clr_prev = SetBkColor(h_dc, Ptb.GetColor((gli.Flags & GrpListItem::fFolder) ? clrParent : clrGrp));
						h_br_def = (HBRUSH)SelectObject(h_dc, Ptb.Get((gli.Flags & GrpListItem::fFolder) ? brGrpParent : brGrp));
						if(UiFlags & uifTSGGroupsAsButtons) {
							SInflateRect(rc, -1, -(1 + GoodsListEntryGap / 4));
							RoundRect(h_dc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
						}
						else
							FillRect(h_dc, &rc, (HBRUSH)Ptb.Get((gli.Flags & GrpListItem::fFolder) ? brGrpParent : brGrp));
					}
					rc.left += gli.Level * 24 + ((UiFlags & uifTSGGroupsAsButtons) ? 4 : 0);
					::DrawText(h_dc, temp_buf.cptr(), temp_buf.Len(), &rc, DT_LEFT|DT_VCENTER|DT_SINGLELINE); // @unicodeproblem
				}
			}
			if(h_fnt_def)
				SelectObject(h_dc, h_fnt_def);
			if(h_br_def)
				SelectObject(h_dc, h_br_def);
			if(h_pen_def)
				SelectObject(h_dc, h_pen_def);
			if(clr_prev)
				SetBkColor(h_dc, clr_prev);
		}
		else
			pDrawItem->ItemAction = 0; // ������ �� ������� - ������ �� ������
	}
}

void CheckPaneDialog::ResetListWindows(int listCtrlID)
{
	const int   sx  = GetSystemMetrics((DlgFlags & fResizeable) ? SM_CXSIZEFRAME : SM_CXFIXEDFRAME);
	const int   sy  = GetSystemMetrics((DlgFlags & fResizeable) ? SM_CYSIZEFRAME : SM_CYFIXEDFRAME);
	const int   cy  = GetSystemMetrics(SM_CYCAPTION);
	const int   vsx = GetSystemMetrics(SM_CXVSCROLL);
	const int   vsy = GetSystemMetrics(SM_CYVSCROLL);
	RECT  dlg_rect, cr;
	HWND  ctrl_wnd = ::GetDlgItem(H(), listCtrlID);
	::GetWindowRect(H(), &dlg_rect);
	const int adj_left = (dlg_rect.left + sx);
	const int adj_top  = (dlg_rect.top + sy + cy);
	GetWindowRect(ctrl_wnd, &cr);
	cr.right -= vsx;
	MoveWindow(ctrl_wnd, cr.left - adj_left, cr.top - adj_top, cr.right - cr.left, cr.bottom - cr.top, 1);
	ctrl_wnd = GetDlgItem(H(), MAKE_BUTTON_ID(listCtrlID, 1));
	GetWindowRect(ctrl_wnd, &cr);
	cr.left -= vsx;
	MoveWindow(ctrl_wnd, cr.left - adj_left, cr.top + vsy * 2 - adj_top, cr.right - cr.left, cr.bottom - cr.top - vsy * 4, 1);
	ctrl_wnd = GetDlgItem(H(), CTL_CHKPAN_ARROW_UP);
	MoveWindow(ctrl_wnd, cr.left - adj_left, cr.top - adj_top, cr.right - cr.left, vsy * 3, 1);
	ctrl_wnd = GetDlgItem(H(), CTL_CHKPAN_ARROW_DOWN);
	MoveWindow(ctrl_wnd, cr.left - adj_left, cr.bottom - vsy * 3 - adj_top, cr.right - cr.left, vsy * 3, 1);
}

int CheckPaneDialog::SetDlgResizeParams()
{
	int    ok = -1;
	if(!(Flags & fNoEdit)) {
		if(Flags & fTouchScreen) {
			PPID   sb_id = MAKE_BUTTON_ID(CTL_CHKPAN_GRPLIST, 1);
			SString font_face;
			PPGetSubStr(PPTXT_FONTFACE, /*PPFONT_MSSANSSERIF*/PPFONT_ARIAL, font_face);
			SetCtrlFont(CTL_CHKPAN_LIST, font_face, /*16*//*22*/18);
			ResetListWindows(CTL_CHKPAN_GDSLIST);
			ResetListWindows(CTL_CHKPAN_GRPLIST);
			SetCtrlResizeParam(CTL_CHKPAN_LIST,        0, 0, CTL_CHKPAN_GDSLIST, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GDSLIST,     CTL_CHKPAN_LIST, 0, 0, 0, crfResizeable);
			SetCtrlResizeParam(MAKE_BUTTON_ID(CTL_CHKPAN_GDSLIST, 1), -1, 0, 0, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPLIST,     CTL_CHKPAN_GDSLIST, 0, 0, 0, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(sb_id,                  -1, 0, 0, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_ARROW_UP,    sb_id,  0, sb_id, -1, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_ARROW_DOWN,  sb_id, -1, sb_id,  0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SELMODIFIER, CTL_CHKPAN_GRPNAME,     0, CTL_CHKPAN_LEVELUP,  -1, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_LEVELUP,     CTL_CHKPAN_SELMODIFIER, 0, CTL_CHKPAN_ARROW_UP, -1, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPNAME,     CTL_CHKPAN_GDSLIST,   0, 0, -1, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX1,     0,                    -1, CTL_CHKPAN_LIST,  0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GOODS,       0,                    -1, CTL_CHKPAN_LIST,  0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_QTTY,        0,                    -1, CTL_CHKPAN_INPUT, 0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_PRICE,       CTL_CHKPAN_INFO,      -1, CTL_CHKPAN_SUM, 0, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SUM,         CTL_CHKPAN_PRICE,     -1, CTL_CHKPAN_GOODS, 0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_TOTAL,       CTL_CHKPAN_TOLOCPRN,  -1, CTL_CHKPAN_LIST, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_DISCOUNT,    CTL_CHKPAN_TOLOCPRN,  -1, CTL_CHKPAN_LIST, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SETQTTY,     CTL_CHKPAN_ENTER,     -1, CTL_CHKPAN_ENTER, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SCARD,       CTL_CHKPAN_ENTER,     -1, CTL_CHKPAN_ENTER, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_BYPRICE,     CTL_CHKPAN_CANCEL,    -1, CTL_CHKPAN_CANCEL, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SELTABLE,    CTL_CHKPAN_CANCEL,    -1, CTL_CHKPAN_CANCEL, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_ENTER,       CTL_CHKPAN_LIST,      -1, CTL_CHKPAN_CANCEL, 0, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(STDCTL_OKBUTTON,        CTL_CHKPAN_ENTER,     CTL_CHKPAN_ENTER, CTL_CHKPAN_ENTER, CTL_CHKPAN_ENTER, CRF_LINK_ALL | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CANCEL,      CTL_CHKPAN_ENTER,     -1, CTL_CHKPAN_CASH, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_RETCHECK,    CTL_CHKPAN_CASH,      -1, CTL_CHKPAN_CASH, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SUSPCHECK,   CTL_CHKPAN_CASH,      -1, CTL_CHKPAN_CASH, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_DIVISION,    CTL_CHKPAN_BANKING,   -1, CTL_CHKPAN_BANKING, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CASHOPER,    CTL_CHKPAN_BANKING,   -1, CTL_CHKPAN_BANKING, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CASH,        CTL_CHKPAN_CANCEL,    -1, CTL_CHKPAN_BANKING, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_BANKING,     CTL_CHKPAN_CASH,      -1, CTL_CHKPAN_TOLOCPRN, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_TOLOCPRN,    CTL_CHKPAN_BANKING,   -1, CTL_CHKPAN_TODEFPRN, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_TODEFPRN,    CTL_CHKPAN_TOLOCPRN,  -1, CTL_CHKPAN_SELGDSGRP, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SELGDSGRP,   CTL_CHKPAN_TODEFPRN,  -1, CTL_CHKPAN_GRPBYDEF, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBYDEF,    CTL_CHKPAN_SELGDSGRP, -1, CTL_CHKPAN_LIST, 0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX2,     CTL_CHKPAN_CASH,      -1, CTL_CHKPAN_TODEFPRN, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX3,     CTL_CHKPAN_SELGDSGRP, -1, CTL_CHKPAN_LIST, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX4,     0,                    -1, 0, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_INPUT,       0,                    -1, CTL_CHKPAN_CASH, 0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_INFO,        CTL_CHKPAN_BANKING,   -1, CTL_CHKPAN_LIST, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CAFE_STATUS, CTL_CHKPAN_GDSLIST,   -1, 0, 0, crfLinkLeft | crfResizeable);
		}
		else {
			SetCtrlResizeParam(CTL_CHKPAN_LIST, 0, 0, 0, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX1, 0, -1, CTL_CHKPAN_TOTAL, 0, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_TOTAL, CTL_CHKPAN_GRPBOX1, -1, 0, 0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_DISCOUNT, CTL_CHKPAN_TOTAL, -1, CTL_CHKPAN_TOTAL, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GOODS, CTL_CHKPAN_GRPBOX1, -1, CTL_CHKPAN_GRPBOX1, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_QTTY, CTL_CHKPAN_GRPBOX1, -1, CTL_CHKPAN_PRICE, 0, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_PRICE, CTL_CHKPAN_QTTY, -1, CTL_CHKPAN_SUM, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SUM, CTL_CHKPAN_PRICE, -1, CTL_CHKPAN_GRPBOX1, 0, crfLinkRight | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CAFE_STATUS, CTL_CHKPAN_GRPBOX1, -1, CTL_CHKPAN_GRPBOX1, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX4, 0, -1, 0, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX5, 0, -1, 0, 0, crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_INPUT, CTL_CHKPAN_GRPBOX4, -1, CTL_CHKPAN_INFO, 0, crfLinkLeft | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_INFO, CTL_CHKPAN_INPUT, -1, CTL_CHKPAN_GRPBOX4, 0, crfLinkRight | crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_SELGOODS,    CTL_CHKPAN_INPUT,     CTL_CHKPAN_GRPBOX5, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable); // @anchor
			SetCtrlResizeParam(CTL_CHKPAN_BYPRICE,     CTL_CHKPAN_SELGOODS,  CTL_CHKPAN_GRPBOX5, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SETQTTY,     CTL_CHKPAN_BYPRICE,   CTL_CHKPAN_GRPBOX5, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_ENTER,       CTL_CHKPAN_INPUT,     CTL_CHKPAN_BYPRICE, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(STDCTL_OKBUTTON,        CTL_CHKPAN_ENTER,     CTL_CHKPAN_ENTER, CTL_CHKPAN_ENTER, CTL_CHKPAN_ENTER, CRF_LINK_ALL | crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CANCEL,      CTL_CHKPAN_ENTER,     CTL_CHKPAN_BYPRICE, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX2,     CTL_CHKPAN_GRPBOX1,   CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_INFO, 0, crfLinkRight|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_CASH,        CTL_CHKPAN_GRPBOX2,   CTL_CHKPAN_GRPBOX2, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_BANKING,     CTL_CHKPAN_CASH,      CTL_CHKPAN_GRPBOX2, -1, 0, crfLinkLeft|crfLinkTop|crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_CASHOPER,    -1, CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_GRPBOX5, 0, crfLinkRight|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SELTABLE,    CTL_CHKPAN_CASH, CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_CASH, 0, crfLinkLeft|crfLinkRight|crfLinkTop|crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_TODEFPRN,    -1, CTL_CHKPAN_GRPBOX2, CTL_CHKPAN_GRPBOX2,  0, crfLinkRight|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_TOLOCPRN,    -1, CTL_CHKPAN_GRPBOX2, CTL_CHKPAN_TODEFPRN, 0, crfLinkRight|crfLinkTop|crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_DIVISION,    -1, CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_SELTABLE, 0, crfLinkRight|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SCARD,       -1, CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_DIVISION, 0, crfLinkRight|crfLinkTop|crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_RETCHECK,    -1, CTL_CHKPAN_GRPBOX2, CTL_CHKPAN_CASH,     0, crfLinkRight|crfLinkTop|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_SUSPCHECK,   -1, CTL_CHKPAN_GRPBOX2, CTL_CHKPAN_RETCHECK, 0, crfLinkRight|crfLinkTop|crfResizeable);


			SetCtrlResizeParam(CTL_CHKPAN_GRPBOX4,     0,                    -1, 0, 0, crfResizeable);

			SetCtrlResizeParam(CTL_CHKPAN_BIGHINT,    CTL_CHKPAN_SETQTTY, CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_BIGHINT_KB, 0, crfLinkLeft|crfLinkTop|crfLinkRight|crfResizeable);
			SetCtrlResizeParam(CTL_CHKPAN_BIGHINT_KB, CTL_CHKPAN_INFO,    CTL_CHKPAN_GRPBOX5, CTL_CHKPAN_SCARD, 0, crfLinkLeft|crfLinkTop|crfLinkRight|crfResizeable);


			/*
			LinkCtrlsToDlgBorders(CRF_LINK_LEFTBOTTOM, CTL_CHKPAN_HINT1, CTL_CHKPAN_KBHINT1,
				CTL_CHKPAN_HINT2, CTL_CHKPAN_KBHINT2, CTL_CHKPAN_HINT3, CTL_CHKPAN_KBHINT3,
				CTL_CHKPAN_HINT4, CTL_CHKPAN_KBHINT4, CTL_CHKPAN_HINT5, CTL_CHKPAN_KBHINT5,
				CTL_CHKPAN_HINT6, CTL_CHKPAN_KBHINT6, CTL_CHKPAN_HINT7, CTL_CHKPAN_KBHINT7, 0L);
			*/
		}
//#ifdef NDEBUG
		ResizeDlgToFullScreen();
//#endif
		UpdateGList(0, 0);  // ��������� ������ �������� �����
		ok = 1;
	}
	return ok;
}

int CheckPaneDialog::UpdateGList(int updGoodsList, PPID selGroupID)
{
	int    ok = 1;
	if(Flags & fTouchScreen && !oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
		SString  grp_name, temp_buf;
		if(updGoodsList) {
			selGroupID = NZOR(selGroupID, SelGoodsGrpID);
			ListBoxDef   * p_def = 0;
			SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_CHKPAN_GDSLIST);
			StrAssocArray * p_ts_ary = 0;
			if(updGoodsList == -2) {
				int    is_input = GetInput();
				double price = 0.0;
				PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SELBYPRICE, grp_name);
				if(!is_input) {
					showInputLineCalc(this, CTL_CHKPAN_INPUT);
					is_input = GetInput();
				}
				if(is_input) {
					price = Input.ToReal();
					PPWait(1);
					if(price != 0.0) {
						p_ts_ary = GObj.CreateListByPrice(LConfig.Location, R2(price));
						grp_name.Space().Cat(price, SFMT_MONEY);
					}
					else {
						if(Input.Len() >= INSTVSRCH_THRESHOLD)
							(temp_buf = 0).CatChar('!').Cat(Input);
						else
							temp_buf = Input;
						p_ts_ary = new StrAssocArray;
						GObj.P_Tbl->GetListBySubstring(temp_buf, p_ts_ary, -1, 1);
						grp_name.Space().CatQStr(temp_buf);
					}
					PPWait(0);
					ClearInput(0);
				}
				SETIFZ(p_ts_ary, new StrAssocArray); // empty list // @newok
				p_def = new StrAssocListBoxDef(p_ts_ary, lbtDblClkNotify | lbtFocNotify | lbtDisposeData);
			}
			else {
				Goods2Tbl::Rec grp_rec;
				PPWait(1);
				if(GObj.Fetch(selGroupID, &grp_rec) > 0)
					PPGetWord(PPWORD_GROUP, 0, grp_name).CatDiv(':', 2).Cat(grp_rec.Name);
				else
					grp_name = 0;
				p_def = GObj.Selector((void *)selGroupID);
				PPWait(0);
			}
			if(ok > 0) {
				if(!(Flags & fNoEdit)) {
					RECT   list_rect;
					GetClientRect(p_list->getHandle(), &list_rect);
					if(p_def) {
						p_def->setViewHight((list_rect.bottom - list_rect.top) / GoodsListFontHeight);
						p_def->SetOption(lbtHSizeAlreadyDef, 1);
					}
				}
				p_list->setDef(p_def);
				if(p_list->def)
					p_list->def->SetOption(lbtSelNotify, 1);
				ActiveListID = CTL_CHKPAN_GDSLIST;
				p_list->drawView();
			}
		}
		else {
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SELGROUP, grp_name);
			//
			SysJournal * p_sj = DS.GetTLA().P_SysJ;
			PPIDArray act_list, obj_list;
			act_list.addzlist(PPACN_OBJADD, PPACN_OBJUPD, PPACN_OBJRMV, PPACN_OBJUNIFY, 0);
			if(p_sj->GetObjListByEventSince(PPOBJ_GOODSGROUP, &act_list, LastGrpListUpdTime, obj_list) > 0)
				if(TouchScreenID) {
					PPTouchScreenPacket ts_pack;
					PPObjTouchScreen    ts_obj;
					if(ts_obj.GetPacket(TouchScreenID, &ts_pack) > 0)
						InitGroupList(ts_pack);
				}
			//
			SmartListBox * p_grp_list = (SmartListBox *)getCtrlView(CTL_CHKPAN_GRPLIST);
			ListBoxDef * p_def = p_grp_list ? (ListBoxDef *)p_grp_list->def : 0;
			if(p_grp_list && p_def) {
				int    sav_pos = (int)p_def->_curItem();
				int    focus_item_found = 0;
				p_grp_list->freeAll();
				GrpListItem * p_item = 0;
				for(uint i = 0; GroupList.enumItems(&i, (void **)&p_item);) {
					uint   p = 0;
					Goods2Tbl::Rec goods_rec;
					int    do_insert = 0;
					if(UiFlags & uifOneGroupLevel) {
						if(p_item->ParentID == GroupList.TopID)
							do_insert = 1;
					}
					else {
						if(!p_item->ParentID || (p_def->search(&p_item->ParentID, 0, lbSrchByID) &&
							GroupList.Get(p_item->ParentID, &p) && GroupList.at(p).Flags & GrpListItem::fOpened))
							do_insert = 1;
					}
					if(do_insert && GObj.Fetch(p_item->ID, &goods_rec) > 0) {
						p_grp_list->addItem(goods_rec.ID, goods_rec.Name);
						if(goods_rec.ID == selGroupID)
							focus_item_found = 1;
					}
				}
				RECT   list_rect;
				GetClientRect(p_grp_list->getHandle(), &list_rect);
				p_def->setViewHight((list_rect.bottom - list_rect.top) / GoodsListFontHeight);
				p_def->SetOption(lbtHSizeAlreadyDef, 1);
				p_def->SetOption(lbtSelNotify, 1);
				p_def->SetOption(lbtFocNotify, 1);
				ActiveListID = CTL_CHKPAN_GRPLIST;
				if(selGroupID) {
					if(focus_item_found)
						p_grp_list->TransmitData(+1, &selGroupID);
					else
						p_grp_list->focusItem(0);
				}
				else
					p_grp_list->focusItem(sav_pos);
				p_grp_list->drawView();
				p_def->getCurID(&SelGoodsGrpID);
			}
			LastGrpListUpdTime = getcurdatetime_();
		}
		if(ok > 0) {
			showCtrl(CTL_CHKPAN_GRPLIST,    !updGoodsList);
			disableCtrl(CTL_CHKPAN_GRPLIST,  updGoodsList);
			ShowWindow(GetDlgItem(H(), MAKE_BUTTON_ID(CTL_CHKPAN_GRPLIST, 1)), updGoodsList ? SW_HIDE : SW_SHOW);
			showCtrl(CTL_CHKPAN_GDSLIST,     updGoodsList);
			disableCtrl(CTL_CHKPAN_GDSLIST, !updGoodsList);
			ShowWindow(GetDlgItem(H(), MAKE_BUTTON_ID(CTL_CHKPAN_GDSLIST, 1)), updGoodsList ? SW_SHOW : SW_HIDE);
			enableCommand(cmaSelect, updGoodsList);
			LastCtrlID = ActiveListID;
			setStaticText(CTL_CHKPAN_GRPNAME, grp_name);
		}
	}
	return ok;
}

void CheckPaneDialog::setupHint()
{
	const  int _state = GetState();
	uint   hint_count = 0, hint_list[32];
	switch(_state) {
		case sEMPTYLIST_EMPTYBUF:
			hint_list[hint_count++] =  1;
			hint_list[hint_count++] =  2;
			hint_list[hint_count++] =  3;
			hint_list[hint_count++] =  4;
			hint_list[hint_count++] = 11;
			hint_list[hint_count++] =  5;
			hint_list[hint_count++] = 13;
			break;
		case sEMPTYLIST_BUF:
			hint_list[hint_count++] =  1;
			hint_list[hint_count++] =  2;
			hint_list[hint_count++] =  3;
			hint_list[hint_count++] =  6;
			hint_list[hint_count++] = 11;
			hint_list[hint_count++] =  7;
			hint_list[hint_count++] =  8;
			break;
		case sLIST_EMPTYBUF:
			hint_list[hint_count++] =  1;
			hint_list[hint_count++] =  2;
			hint_list[hint_count++] =  3;
			hint_list[hint_count++] = 11;
			hint_list[hint_count++] =  9;
			hint_list[hint_count++] = 10;
			hint_list[hint_count++] = 12;
			break;
		case sLIST_BUF:
			hint_list[hint_count++] =  1;
			hint_list[hint_count++] =  2;
			hint_list[hint_count++] =  3;
			hint_list[hint_count++] =  6;
			hint_list[hint_count++] = 11;
			hint_list[hint_count++] =  7;
			hint_list[hint_count++] =  8;
			break;
		case sLISTSEL_EMPTYBUF:
			hint_list[hint_count++] =  2;
			hint_list[hint_count++] =  9;
			hint_list[hint_count++] = 10;
			break;
		case sLISTSEL_BUF:
			hint_list[hint_count++] = 2;
			hint_list[hint_count++] = 6;
			hint_list[hint_count++] = 7;
			hint_list[hint_count++] = 8;
			break;
	}
	{
		const int is_locked = 0;
		enableCommand(cmRetCheck, BIN(oneof3(_state, sEMPTYLIST_EMPTYBUF, sLISTSEL_EMPTYBUF, sLISTSEL_BUF) && (OperRightsFlags & orfReturns)));
		enableCommand(cmQuantity, BIN(oneof3(_state, sEMPTYLIST_BUF, sLIST_BUF, sLISTSEL_BUF)));
		enableCommand(cmChkPanSuspend, BIN(oneof2(_state, sEMPTYLIST_EMPTYBUF, sLIST_EMPTYBUF)));
		enableCommand(cmCash,    BIN(oneof2(_state, sLIST_EMPTYBUF, sLISTSEL_EMPTYBUF) && !is_locked && (OperRightsFlags & orfPrintCheck)));
		enableCommand(cmBanking, BIN(oneof2(_state, sLIST_EMPTYBUF, sLISTSEL_EMPTYBUF) && !is_locked &&
			((OperRightsFlags & (orfBanking | orfPrintCheck)) == (orfBanking | orfPrintCheck))));
		enableCommand(cmChkPanPrint,   BIN((oneof2(_state, sEMPTYLIST_EMPTYBUF, sLIST_EMPTYBUF) || (Flags & fNoEdit)) && (OperRightsFlags & orfPreCheck)));
		enableCommand(cmToLocPrinters, BIN(oneof2(_state, sEMPTYLIST_EMPTYBUF, sLIST_EMPTYBUF) && (Flags & fLocPrinters)));
	}
	if(getCtrlView(CTL_CHKPAN_BIGHINT)) {
		SString temp_buf, hint, keyb, hint_buf, hint_kb_buf;
		for(uint i = 0; i < CTL_CHKPAN_NUMHINTS; i++) {
			uint   idx = hint_list[i];
			if(i < hint_count && PPLoadText(PPTXT_CHKPAN_HINT01 - 1 + idx, temp_buf) > 0) {
				if(temp_buf.Strip().Divide('=', hint, keyb) > 0) {
					hint_buf.Space().Cat(hint.Strip()).CR();
					hint_kb_buf.Space().Cat(keyb.Strip()).CR();
				}
				else {
					hint_buf.Space().Cat(hint.Strip()).CR();
					hint_kb_buf.CR();
				}
			}
		}
		setStaticText(CTL_CHKPAN_BIGHINT, hint_buf);
		setStaticText(CTL_CHKPAN_BIGHINT_KB, hint_kb_buf);
	}
	if(!(Flags & fTouchScreen)) {
		SString temp_buf, hint, keyb;
		for(uint i = 0; i < CTL_CHKPAN_NUMHINTS; i++) {
			uint   idx = hint_list[i];
			if(i < hint_count && PPLoadText(PPTXT_CHKPAN_HINT01 - 1 + idx, temp_buf) > 0) {
				if(temp_buf.Divide('=', hint, keyb) > 0)
					setStaticText(CTL_CHKPAN_HINT1 + i + CTL_CHKPAN_KBHINTBIAS, keyb);
				setStaticText(CTL_CHKPAN_HINT1 + i, hint);
			}
			else {
				temp_buf = 0;
				setStaticText(CTL_CHKPAN_HINT1 + i, temp_buf);
				setStaticText(CTL_CHKPAN_HINT1 + i + CTL_CHKPAN_KBHINTBIAS, temp_buf);
			}
		}
	}
}
//
//
//
int CheckPaneDialog::SelectCheck(PPID * pChkID, SString * pSelFormat, long flags/*int selSlipDocForm, int thisNodeOnly*/)
{
	int    ok = -1, valid_data = 0;
	_SelCheck  sch;
	MEMSZERO(sch);
	Flags |= fSuspSleepTimeout;
	SelCheckListDialog::AddedParam param(((flags & scfThisNodeOnly) ? CashNodeID : 0), P.TableCode, P.GetAgentID(), OperRightsFlags);
	if(flags & scfAllowReturns)
		param.Flags |= param.fAllowReturns;
	RVALUEPTR(param.FormatName, pSelFormat);
	InitCashMachine();
	const uint dlg_id = (DlgFlags & fLarge) ? DLG_SELCHECK_L : DLG_SELCHECK;
	SelCheckListDialog * dlg = new SelCheckListDialog(dlg_id, (pSelFormat ? ((flags & scfSelSlipDocForm) ? 1 : -1) : 0), P_CM, this, &param);
	if(!CheckDialogPtr(&dlg, 1))
		ok = 0;
	while(ok < 0 && ExecView(dlg) == cmOK)
		if(dlg->getDTS(&sch))
			ok = 1;
	ASSIGN_PTR(pChkID, sch.CheckID);
	ASSIGN_PTR(pSelFormat, sch.SelFormat);
	delete dlg;
	Flags &= ~fSuspSleepTimeout;
	return ok;
}

int CPosProcessor::RestoreSuspendedCheck(PPID ccID)
{
	int    ok = 1;
	SString  serial, msg_buf;
	CCheckPacket cc_pack;
	Goods2Tbl::Rec goods_rec;
	//
	{
		PPTransaction tra(1);
		THROW(tra);
		THROW(CC.LoadPacket(ccID, 0, &cc_pack) > 0);
		CCheckCore::MakeCodeString(&cc_pack.Rec, msg_buf);
		const long _ccf = cc_pack.Rec.Flags;
		THROW_PP_S(_ccf & CCHKF_SUSPENDED && (!(_ccf & CCHKF_JUNK) || CC.IsLostJunkCheck(ccID, &SessUUID, 0)), PPERR_CCHKNOMORESUSPENDED, msg_buf);
		THROW(CC.UpdateFlags(ccID, _ccf|CCHKF_JUNK, 0));
		Helper_SetupSessUuidForCheck(ccID); // @v8.2.11
		THROW(tra.Commit());
	}
	//
	P.freeAll();
	P.OrgUserID = 0;
	{
		CCheckItem chk_item;
		for(uint i = 0; cc_pack.EnumLines(&i, &chk_item);) {
			if(GObj.Fetch(chk_item.GoodsID, &goods_rec) > 0) {
				STRNSCPY(chk_item.GoodsName, goods_rec.Name);
				GObj.GetSingleBarcode(chk_item.GoodsID, chk_item.BarCode, sizeof(chk_item.BarCode));
			}
			P.insert(&chk_item);
		}
	}
	SetupExt(&cc_pack);
	if(P.getCount()) {
		if(CnExtFlags & CASHFX_KEEPORGCCUSER)
			P.OrgUserID = cc_pack.Rec.UserID;
		SetupState(sLIST_EMPTYBUF);
	}
	SETFLAG(Flags, fRetCheck, cc_pack.Rec.Flags & CCHKF_RETURN);
	SetPrintedFlag(cc_pack.Rec.Flags & CCHKF_PREPRINT);
	Flags  |= fWaitOnSCard;
	SetupSCard(cc_pack.Rec.SCardID, 0);
	Flags &= ~fWaitOnSCard; // @v8.0.3
	SetupInfo(0); // @v8.0.3
	SuspCheckID = ccID;
	ProcessGift(); // ��������� � ��� ���������� �������
	CC.WriteCCheckLogFile(&cc_pack, 0, CCheckCore::logRestored, 1);
	CATCHZOK
	return ok;
}

int CPosProcessor::CheckRights(long rights) const
{
	return ((OperRightsFlags & rights) ==  rights) ? 1 : PPSetError(PPERR_NORIGHTS);
}

int CheckPaneDialog::SelectSuspendedCheck()
{
	int    ok = -1;
	PPID   chk_id = 0;
	SString msg_buf;
	SelCheckListDialog * dlg = 0;
	const  PPID single_agent_id = P.GetAgentID();
	THROW_PP(OperRightsFlags & orfRestoreSuspWithoutAgent || single_agent_id, PPERR_NORIGHTSELSUSPCHECK);
	if(IsState(sEMPTYLIST_EMPTYBUF)) {
		PPObjArticle ar_obj;
		ArticleTbl::Rec ar_rec;
		TSArray <CCheckViewItem> list;
		CCheckFilt cc_filt;
		SelCheckListDialog::AddedParam param(/* @v8.4.3 CashNodeID*/0, P.TableCode, single_agent_id, OperRightsFlags);
		THROW(InitCcView());
		const uint dlg_id = (DlgFlags & fLarge) ? DLG_SELSUSCHECK_L : DLG_SELSUSCHECK;
		dlg = new SelCheckListDialog(dlg_id, &list, 0, this, &param);
		Flags |= fSuspSleepTimeout;
		THROW(CheckDialogPtr(&dlg, 0));
		do {
			cc_filt.Period.low = plusdate(getcurdate_(), (Scf.DaysPeriod > 0) ? -Scf.DaysPeriod : -7);
			cc_filt.Flags |= (CCheckFilt::fSuspendedOnly | CCheckFilt::fShowSuspended);
			cc_filt.Flags |= CCheckFilt::fLostJunkAsSusp; // @v8.2.12
			//cc_filt.CashNodeID = CashNodeID;
			cc_filt.TableCode = P.TableCode;
			cc_filt.AgentID = single_agent_id;
			list.clear();
			if(P_CcView->Init_(&cc_filt)) {
				CCheckViewItem item;
				for(P_CcView->InitIteration(0); P_CcView->NextIteration(&item) > 0;) {
					if(Scf.DlvrItemsShowTag < 0) {
						if(item.Flags & CCHKF_DELIVERY)
							continue;
					}
					else if(Scf.DlvrItemsShowTag > 0) {
						if(!(item.Flags & CCHKF_DELIVERY))
							continue;
					}
					// @v9.0.8 {
					if(!single_agent_id && item.AgentID && ar_obj.Fetch(item.AgentID, &ar_rec) > 0 && (ar_rec.Flags & ARTRF_STOPBILL))
						continue;
					// } @v9.0.8
					list.insert(&item);
				}
			}
			dlg->setList(list);
			if(ExecView(dlg) == cmOK) {
				_SelCheck  sel_chk;
				if(dlg->getDTS(&sel_chk) && sel_chk.CheckID) {
					CCheckTbl::Rec cc_rec;
					if(CC.Search(sel_chk.CheckID, &cc_rec) > 0) {
						if(cc_rec.Flags & CCHKF_SUSPENDED && (!(cc_rec.Flags & CCHKF_JUNK) || CC.IsLostJunkCheck(sel_chk.CheckID, &SessUUID, 0))) {
							chk_id = sel_chk.CheckID;
							ok = 1;
						}
						else {
							CCheckCore::MakeCodeString(&cc_rec, msg_buf);
							ok = (MessageError(PPERR_CCHKNOMORESUSPENDED, msg_buf, eomMsgWindow), 2);
						}
					}
					else
						ok = (MessageError(-1, 0, eomMsgWindow), 2);
				}
			}
			else
				ok = -1;
		} while(ok == 2);
		Flags &= ~fSuspSleepTimeout;
		if(ok == 1) {
			THROW(RestoreSuspendedCheck(chk_id));
		}
	}
	CATCHZOKPPERR
	delete dlg;
	return ok;
}

int CheckPaneDialog::SelectTable()
{
	int    ok = -1;
	if(!SuspCheckID || !P.TableCode || CONFIRM(PPCFM_CHKPAN_CHNGTBL)) {
		if(TableSelWhatman.NotEmpty() && fileExists(TableSelWhatman)) {
			TWhatmanObject::SelectObjRetBlock sel_blk;
			if(PPWhatmanWindow::Launch(TableSelWhatman, 0, &sel_blk) > 0) {
				if(sel_blk.Val1 == PPOBJ_CAFETABLE && sel_blk.Val2 > 0) {
					long   guest_count = 0;
					if(CnExtFlags & CASHFX_INPGUESTCFTBL)
						SelectGuestCount(sel_blk.Val2, &guest_count);
					SetupCTable(sel_blk.Val2, guest_count);
					ok = 1;
				}
			}
		}
		else {
			int    is_input = GetInput();
			if(!is_input) {
				showInputLineCalc(this, CTL_CHKPAN_INPUT);
				is_input = GetInput();
			}
			if(is_input) {
				int    table_no = Input.ToLong();
				long   guest_count = 0;
				if(CnExtFlags & CASHFX_INPGUESTCFTBL)
					SelectGuestCount(table_no, &guest_count);
				SetupCTable(table_no, guest_count);
				ClearInput(0);
				ok = 1;
			}
		}
	}
	return ok;
}

void CheckPaneDialog::setupRetCheck(int ret)
{
	SString temp_buf;
	Flags |= fSuspSleepTimeout;
	if(!ret) {
		Rb.Clear();
	}
	Flags &= ~fRetByCredit;
	if((!P.HasCur() && !P.getCount()) || oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
		if(OperRightsFlags & orfReturns) {
			if(!P.HasCur() && !P.getCount()) {
				SETFLAG(Flags, fRetCheck, ret);
			}
			else if(F(fRetCheck))
				ret = 1;
			if(ret) {
				CCheckPacket  chk_pack;
				if(!oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
					PPID   chk_id = 0;
					if(SelectCheck(&chk_id, 0, 0 /*scfXXX*/) > 0) {
						CC.LoadPacket(chk_id, 0, &SelPack);
						chk_pack.CopyLines(SelPack);
						if(SelPack.Rec.SCardID)
							AcceptSCard(0, SelPack.Rec.SCardID, 1 /* ignoreRights */);
						else
							CSt.SetID(0, 0);
						Flags &= ~fWaitOnSCard;
					}
				}
				else {
					CCheckLineTbl::Rec line;
					for(uint i = 0; SelPack.EnumLines(&i, &line);) {
						double  sel_qtty = 0.0, rest_qtty = line.Quantity;
						if(SelLines.Search(i, &sel_qtty, 0))
							rest_qtty -= sel_qtty;
						if(rest_qtty)
							chk_pack.InsertItem(line.GoodsID, rest_qtty, intmnytodbl(line.Price), line.Dscnt);
					}
				}
				if(chk_pack.GetCount() > 0) {
					long  lc_flags = DS.GetTLA().Lc.Flags;
					chk_pack.Rec = SelPack.Rec;
					ushort r = CheckExecAndDestroyDialog(new CheckPaneDialog(0, 0, &chk_pack, BIN(Flags & fTouchScreen)), 1, 0);
					if(r) {
						int    crcc_arg = -1; // �������� ������������ ������ ������� CalcRestByCrdCard_ (-1 - �� ��������)
						if(r == cmaAll) {
							LoadCheck(&chk_pack, 1);
							SelLines.freeAll();
							CCheckLineTbl::Rec line;
							for(uint i = 0; SelPack.EnumLines(&i, &line);)
								SelLines.Add(i, line.Quantity);
							SetupState(sLISTSEL_EMPTYBUF);
							crcc_arg = 0;
						}
						else if(r == cmOK) {
							Goods2Tbl::Rec goods_rec;
							const CCheckLineTbl::Rec & cclr = chk_pack.GetLine(0);
							CCheckItem & r_cur_item = P.GetCur();
							r_cur_item.GoodsID = cclr.GoodsID;
							if(GObj.Fetch(r_cur_item.GoodsID, &goods_rec) > 0) {
								STRNSCPY(r_cur_item.GoodsName, goods_rec.Name);
								GObj.FetchSingleBarcode(r_cur_item.GoodsID, temp_buf = 0); // @v8.0.10 GetSingleBarcode-->FetchSingleBarcode
								temp_buf.CopyTo(r_cur_item.BarCode, sizeof(r_cur_item.BarCode));
							}
							else
								r_cur_item.GoodsName[0] = r_cur_item.BarCode[0] = 0;
							r_cur_item.Quantity = -fabs(cclr.Quantity);
							r_cur_item.Price    = intmnytodbl(cclr.Price);
							r_cur_item.Discount = cclr.Dscnt;
							P.CurPos = P.getCount();
							SetupRowData(1);
							SetupState(sLISTSEL_BUF);
							crcc_arg = 1;
						}
						{
							//CcAmountList ccal;
							//SelPack.GetAmountList(0, ccal);
							Rb.AmL = SelPack.AL();
							Rb.SellCheckID = chk_pack.Rec.ID;
							if(Rb.AmL.getCount()) {
								Rb.SellCheckAmount = Rb.AmL.GetTotal();
								Rb.SellCheckCredit = Rb.AmL.Get(CCAMTTYP_CRDCARD);
								assert(MONEYTOLDBL(SelPack.Rec.Amount) == Rb.SellCheckAmount); // @paranoic
							}
							else {
								Rb.SellCheckAmount = MONEYTOLDBL(chk_pack.Rec.Amount);
								Rb.SellCheckCredit = 0.0;
							}
							if(Rb.SellCheckCredit > 0.0 && Rb.SellCheckAmount > 0.0)
								Flags |= fRetByCredit;
						}
						if(crcc_arg >= 0)
							CalcRestByCrdCard_(crcc_arg);
					}
					DS.SetLCfgFlags(lc_flags);
				}
			}
			else if(!F(fRetCheck)) {
				SelPack.ClearLines();
				SelLines.freeAll();
				SetupState(sEMPTYLIST_EMPTYBUF);
				CSt.Reset();
			}
			SetupInfo(0);
			ClearInput(0);
		}
		else if(ret)
			PPError(PPERR_NORIGHTS);
	}
	Flags &= ~fSuspSleepTimeout;
}

void CheckPaneDialog::SetupInfo(const char * pErrMsg)
{
	double rest = 0.0;
	SString buf, word;
	if(pErrMsg) {
		Flags |= fError;
		buf = pErrMsg;
	}
	else {
		Flags &= ~fError;
		if(F(fOnlyReports))
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SESSISCLOSED, buf);
		else if(F(fRetCheck))
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_RETCHECK, buf);
		else if(F(fSelByPrice))
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SELBYPRICE, buf);
		else if(F(fWaitOnSCard))
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_WAITINGONSCARD, buf);
		else if(P_ChkPack && P_ChkPack->Rec.Flags & CCHKF_ORDER) {
			if(P_ChkPack->Rec.Flags & CCHKF_SKIP)
				PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_CTBLORDERCANCELED, buf);
			else
				PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_CTBLORDER, buf);
		}
		else
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SELLING, buf);
		if(CSt.GetID()) {
			SCardTbl::Rec sc_rec;
			if(ScObj.Fetch(CSt.GetID(), &sc_rec) > 0) {
				if(buf.NotEmpty())
					buf.Space();
				// @v9.0.2 PPGetWord(PPWORD_CARD, 0, word);
				PPLoadString("card", word); // @v9.0.2
				buf.Cat(word).Space().Cat(sc_rec.Code).Space();
				if(CSt.Flags & CSt.fUhtt)
					buf.CatParStr("UHTT").Space();
				if(F(fRetCheck)) {
					if(F(fRetByCredit) && CSt.AdditionalPayment) {
						buf.Cat(PPGetWord(PPWORD_ADDPAYMENT, 0, word)).Space().Cat(CSt.AdditionalPayment, SFMT_MONEY);
					}
				}
				else {
					buf.Cat(PPGetWord(PPWORD_DISCOUNT, 0, word)).Space();
					buf.Cat(CSt.SettledDiscount, MKSFMTD(0, 3, NMBF_NOTRAILZ)).CatChar('%');
					if(Flags & (fBankingPayment|fSCardCredit|fSCardBonus)) { // @bank_or_scard
						if(CSt.AdditionalPayment > 0.0 && !(Flags & fSCardBonus))
							buf.Space().Cat(PPGetWord(PPWORD_ADDPAYMENT, 0, word)).Space().Cat(CSt.AdditionalPayment, SFMT_MONEY);
						else {
							PPLoadString((Flags & fSCardBonus) ? "bonus" : "rest", word);
							buf.Space().Cat(word).Space().Cat(CSt.RestByCrdCard, SFMT_MONEY);
						}
					}
				}
				if(CSt.P_Eqb && CSt.P_Eqb->QkList.getCount()) {
					const PPID single_qk_id = CSt.P_Eqb->QkList.getSingle();
					if(single_qk_id) {
						PPObjQuotKind qk_obj;
						PPQuotKind qk_rec;
						PPLoadString("quote", word);
						buf.Space().Cat(word).CatDiv(':', 2);
						if(qk_obj.Fetch(single_qk_id, &qk_rec) > 0)
							buf.Cat(qk_rec.Name);
						else
							ideqvalstr(single_qk_id, buf);
					}
					else
						buf.Cat("QL");
				}
			}
		}
	}
	setStaticText(CTL_CHKPAN_INFO, buf);
	//
	P.SetupInfo(buf);
	if(CnFlags & CASHF_SHOWREST && P.GetCur().GoodsID) {
		if(buf.NotEmpty())
			buf.CatCharN(' ', 4);
		PPLoadString("rest", word);
		buf.Cat(word).CatDiv(':', 2).Cat(P.GetRest(), MKSFMTD(0, 3, NMBF_NOTRAILZ));
	}
	setStaticText(CTL_CHKPAN_CAFE_STATUS, buf);
}

int CPosProcessor::PreprocessRowBeforeRemoving(/*IN*/long rowNo, /*OUT*/double * pResultQtty)
{
	int    ok = -1;
	double qtty = 0.0;
	if(rowNo >= 0 && rowNo < (long)P.getCount()) {
		const  CCheckItem item = P.at((uint)rowNo); // @note ���������� �� ������ ���� ������� (�������� ����� ����������)
		qtty = fabs(item.Quantity);
		int    is_rights = 1;
		if((Flags & fPrinted) && !(OperRightsFlags & orfChgPrintedCheck))
			is_rights = 0;
		else if(!(OperRightsFlags & orfEscChkLine) && (!(OperRightsFlags & orfEscChkLineBeforeOrder) || (item.Flags & cifIsPrinted)))
			is_rights = 0;
		if(is_rights) {
			CCheckLineTbl::Rec line;
			for(uint i = 0; qtty > 0.0 && SelPack.EnumLines(&i, &line);) {
				if(line.GoodsID == item.GoodsID) {
					double  sel_qtty = 0.0;
					if(SelLines.Search(i, &sel_qtty, 0)) {
						if(qtty < sel_qtty)
							SelLines.Add(i, -qtty);
						else
							SelLines.Remove(i);
						qtty -= sel_qtty;
					}
				}
			}
			ok = 1;
		}
		else {
			Flags |= fSuspSleepTimeout;
			ok = (PPError(PPERR_NORIGHTS), 0);
			Flags &= ~fSuspSleepTimeout;
		}
	}
	ASSIGN_PTR(pResultQtty, qtty);
	return ok;
}

int CPosProcessor::Helper_PrintRemovedRow(const CCheckItem & rItem)
{
	int   ok = -1;
	if(rItem.Flags & cifIsPrinted) {
		const int do_debug_log = BIN(CConfig.Flags & CCFLG_DEBUG);
		SStrCollection debug_rep_list;
		SString msg_buf, chk_code, prn_name;
		SString buf_prn, buf_ulp, buf_errprn;
		if(do_debug_log) {
			PPLoadText(PPTXT_LOG_CHKPAN_PRINTING, buf_prn);
			PPLoadText(PPTXT_LOG_CHKPAN_UNDEFPRN, buf_ulp);
			PPLoadText(PPTXT_LOG_CHKPAN_ERRPRINTING, buf_errprn);
			CCheckPacket cc_pack;
			GetCheckInfo(&cc_pack);
			CCheckCore::MakeCodeString(&cc_pack.Rec, chk_code);
		}
		PPObjLocPrinter lp_obj;
		PPLocPrinter loc_prn_rec;
		DS.GetTLA().PrintDevice = 0;
		//
		// ������������� gtoa �� ����������, ������������� � �������� ����
		//
		InitCashMachine();
		GoodsToObjAssoc gtoa(NZOR(P_CM->GetNodeData().GoodsLocAssocID, PPASS_GOODS2LOC), PPOBJ_LOCATION);
		if(gtoa.IsValid() && gtoa.Load()) {
			PPID   loc_id = 0;
			gtoa.Get(rItem.GoodsID, &loc_id);
			{
				//
				// � ���� ������� ���������� ������ P ��������� ���������, � ����� ����������������� (P = saved_check).
				//
				CCheckItemArray saved_check(P);
				P.freeAll();
				if(lp_obj.GetPrinterByLocation(loc_id, prn_name, &loc_prn_rec) > 0) {
					P.insert(&rItem);
					if(Print(1, &loc_prn_rec, REPORT_CCHECKDETAILLOCROWCANCEL) > 0)
						MakeDbgPrintLogList(0, buf_prn, chk_code, prn_name, debug_rep_list);
					else
						MakeDbgPrintLogList(2, buf_errprn, chk_code, prn_name, debug_rep_list);
				}
				else
					MakeDbgPrintLogList(1, buf_ulp, chk_code, prn_name, debug_rep_list);
				P = saved_check;
			}
			if(do_debug_log)
				PPLogMessageList(PPFILNAM_DEBUG_LOG, debug_rep_list, LOGMSGF_TIME|LOGMSGF_USER);
			ok = 1;
		}
		else
			ok = MessageError(PPErrCode, 0, eomBeep|eomStatusLine);
	}
	return ok;
}

int CPosProcessor::Helper_RemoveRow(long rowNo, const CCheckItem & rItem)
{
	int    ok = 1;
	P.atFree((uint)rowNo);
	Helper_PrintRemovedRow(rItem);
	ProcessGift();
	//ReplyOnRemoveItem();
	AutosaveCheck(); // @v8.7.7
	DS.LogAction(PPACN_RMVCHKLINE, PPOBJ_CCHECK, 0, rItem.GoodsID, 1);
	if(!oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF))
		SetupDiscount();
	CalcRestByCrdCard_(0);
	if(CConfig.Flags & CCFLG_DEBUG) {
		CCheckPacket pack;
		CCheckLineTbl::Rec line_rec;
		if(SuspCheckID) {
			CC.Search(SuspCheckID, &pack.Rec);
		}
		else {
			InitCashMachine();
			pack.Rec.SessID = P_CM ? P_CM->GetCurSessID() : 0;
			pack.Rec.CashID = CashNodeID;
			pack.Rec.Flags |= (CCHKF_SYNC | CCHKF_NOTUSED);
		}
		Helper_InitCcPacket(&pack, 0, 0, 0);
		rItem.GetRec(line_rec, BIN(pack.Rec.Flags & CCHKF_RETURN));
		line_rec.CheckID = pack.Rec.ID;
		CC.WriteCCheckLogFile(&pack, &line_rec, CCheckCore::logRowCleared, 1);
	}
	OnUpdateList(0);
	return ok;
}

int CPosProcessor::ResetCurrentLine()
{
	int    ok = -1;
	if(P.HasCur()) {
		ClearRow();
		ok = 1;
	}
	else
		ok = -1;
	return ok;
}

int CPosProcessor::Backend_Release()
{
	int    ok = 1;
	ResetCurrentLine();
	if(P.getCount()) {
		THROW(AcceptCheck(0, 0, accmSuspended) > 0);
	}
	CATCHZOK
	return ok;
}

int CPosProcessor::Backend_SetModifList(const SaModif & rList)
{
    int    ok = -1;
    THROW_PP(P.HasCur() && !(P.GetCur().Flags & cifModifier), PPERR_CPOS_UNABLESEMODIFLIST);
    if(rList.getCount()) {
		for(uint i = 0; i < rList.getCount(); i++) {
			const SaModifEntry & r_entry = rList.at(i);
			THROW_SL(P.CurModifList.insert(&r_entry));
		}
		ok = 1;
    }
    CATCHZOK
    return ok;
}

int CPosProcessor::Backend_SetRowQueue(int rowNo, int queue)
{
	int    ok = -1;
	if(!(Flags & fNoEdit)) {
		long   cur = rowNo;
		THROW_PP_S(cur >= 0 && cur < (long)P.getCount(), PPERR_CPOS_INVCCROWINDEX, rowNo);
		THROW_PP_S(queue >= 0 && queue <= 100, PPERR_CPOS_INVCCROWQUEUE, queue);
		{
			CCheckItem & r_item = P.at((uint)cur);
			r_item.Queue = queue;
			ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int CPosProcessor::Backend_RemoveRow(int rowNo)
{
	int    ok = -1;
	if(!(Flags & fNoEdit)) {
		long   cur = rowNo;
		THROW_PP_S(cur >= 0 && cur < (long)P.getCount(), PPERR_CPOS_INVCCROWINDEX, rowNo);
		if(PreprocessRowBeforeRemoving(cur, 0) > 0) {
			const  CCheckItem item = P.at((uint)cur);
			if(!(item.Flags & cifIsPrinted) || ConfirmMessage(PPCFM_PRINTCANCELEDCCROW, 0, 0)) {
				Helper_RemoveRow(cur, item);
				// @interactive selectCtrl(CTL_CHKPAN_INPUT);
				ok = 1;
			}
			if(!P.getCount()) {
				int  prev_state = GetState();
				if(IsState(sLIST_EMPTYBUF)) {
					SetupState(sEMPTYLIST_EMPTYBUF);
					P.CurPos = -1;
				}
				else if(IsState(sLIST_BUF)) {
					SetupState(sEMPTYLIST_BUF);
					P.CurPos = 0;
				}
				/* @interactive
				if(GetState() != prev_state)
					setupHint();
				LastCtrlID = 0;
				*/
			}
		}
	}
	CATCHZOK
	return ok;
}

int CheckPaneDialog::RemoveRow()
{
	int    ok = -1;
	if(!(Flags & fNoEdit)) {
		SmartListBox * p_list = (SmartListBox *)getCtrlView(CTL_CHKPAN_LIST);
		if(p_list) {
			long   cur = p_list->def ? p_list->def->_curItem() : -1;
			if(PreprocessRowBeforeRemoving(cur, 0) > 0) {
				const  CCheckItem item = P.at((uint)cur);
				//
				// @v8.1.11 ��������� ���������� ���� ����� �������� ����� �������, ��� ����
				// ������ ���� ����� ���������� � ������������ �������� ������ �� ������ ������,
				// �� ��������� ������ �� �����.
				//
				if(!(item.Flags & cifIsPrinted) || ConfirmMessage(PPCFM_PRINTCANCELEDCCROW, 0, 1)) {
					Helper_RemoveRow(cur, item);
					selectCtrl(CTL_CHKPAN_INPUT);
					ok = 1;
				}
				if(!P.getCount()) {
					int  prev_state = GetState();
					if(IsState(sLIST_EMPTYBUF)) {
						SetupState(sEMPTYLIST_EMPTYBUF);
						P.CurPos = -1;
					}
					else if(IsState(sLIST_BUF)) {
						SetupState(sEMPTYLIST_BUF);
						P.CurPos = 0;
					}
					if(GetState() != prev_state)
						setupHint();
					LastCtrlID = 0;
				}
			}
		}
	}
	return ok;
}

//virtual
int CheckPaneDialog::OnUpdateList(int goBottom)
{
//@lbt_chkpan    "3,R,#;3,C,;70,L,�����;16,L,��������;11,R,����;10,R,���-��;11,R,�����;12,L,�����;9,R,�����;4,C,Q" // DLG_CHKPAN
//@lbt_chkpan_ts "4,R,#;3,C,;60,L,�����;12,R,����;12,R,���-��;12,R,�����;10,R,�����;4,C,Q"                         // DLG_CHKPAN_TS
//@lbt_chkpanv   "3,R,#;3,C,;40,L,�����;16,L,��������;9,R,����;8,R,������;8,R,���-��;9,R,�����;12,L,�����"         // DLG_CHKPANV, DLG_CHKPANV_L

	SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_CHKPAN_LIST);
	if(p_list) {
		const long column_egais_ident = 100;
		CCheckItem * p_item;
		long   cur = p_list->def ? p_list->def->_curItem() : 0;
		uint   i;
		int    do_show_egaismark = 0;
		lock();
		p_list->freeAll();
		StringSet ss(SLBColumnDelim);
		/*
		for(i = 0; !do_show_egaismark && P.enumItems(&i, (void**)&p_item);) {
			if(p_item->EgaisMark[0] != 0)
				do_show_egaismark = 1;
		}
		if(do_show_egaismark && !p_list->SearchColumnByIdent(column_egais_ident, 0)) {
			p_list->AddColumn(-1, "@egaisexcisemark", 20, 0, column_egais_ident);
		}
		*/
		for(i = 0; P.enumItems(&i, (void**)&p_item);) {
			ss.clear(1);
			char   sub[256];
			ss.add(itoa((int)i, sub, 10));
			sub[0] = (p_item->Flags & cifIsPrinted) ? 'v' : ' ';
			sub[1] = 0;
			ss.add(sub);
			{
				sub[0] = 0;
				uint   sp = 0;
				if(p_item->Flags & cifModifier) {
					sub[sp++] = '>';
					const uint sc = 7;
					memset(sub+sp, ' ', sc);
					sp += sc;
				}
				strnzcpy(sub+sp, p_item->GoodsName, sizeof(sub)-sp);
				ss.add(sub);
			}
			if(!(Flags & fTouchScreen) || (Flags & fNoEdit))
				ss.add(p_item->BarCode);
			ss.add(realfmt(p_item->Price,    SFMT_MONEY, sub));
			if(Flags & fNoEdit) {
				ss.add(realfmt(p_item->Discount, MKSFMTD(0, 5, NMBF_NOTRAILZ), sub));
			}
			ss.add(realfmt(p_item->Quantity, SFMT_QTTY, sub));
			ss.add(realfmt(p_item->GetAmount(), SFMT_MONEY, sub));
			if(!(Flags & fTouchScreen) || (Flags & fNoEdit))
				ss.add(p_item->Serial);
			ss.add(intfmt(p_item->Division, NMBF_NOZERO, sub));
			ss.add(intfmt(p_item->Queue, NMBF_NOZERO, sub));
			// @v9.2.9 {
			if(do_show_egaismark)
				ss.add(p_item->EgaisMark);
			// } @v9.2.9
			if(!p_list->addItem(i, ss.getBuf())) {
				Flags |= fSuspSleepTimeout;
				PPError(PPERR_SLIB, 0);
				Flags &= ~fSuspSleepTimeout;
				break;
			}
			else {
				if(p_item->Flags & cifGift)
					p_list->def->SetItemColor(i, SClrBlack, SClrGreen);
				else if(p_item->Flags & cifGrouped || (i < P.getCount() && P.at(i).Flags & cifGrouped))
					p_list->def->SetItemColor(i, SClrBlack, SClrLightgrey);
				else if(p_item->Flags & cifIsPrinted)
					p_list->def->SetItemColor(i, SClrBlack, SColor(0xFC, 0xD5, 0xB4));
				else if(p_item->Flags & cifPartOfComplex)
					p_list->def->SetItemColor(i, SClrBlack, SColor(0xD7, 0xE4, 0xBC));
				else
					p_list->def->ResetItemColor(i);
			}
		}
		if(goBottom)
			cur = P.getCount() - 1;
		p_list->focusItem(cur);
		p_list->drawView();
		{
			double total = 0.0, discount = 0.0;
			CalcTotal(&total, &discount);
			SString buf;
			setStaticText(CTL_CHKPAN_TOTAL, buf.Cat(total, MKSFMTD(0, 2, NMBF_NOZERO)));
			if(discount != 0.0)
				PPGetWord(PPWORD_DISCOUNT, 0, buf).CatChar(':').Cat(discount, SFMT_MONEY);
			else
				buf = 0;
			setStaticText(CTL_CHKPAN_DISCOUNT, buf);
		}
		unlock();
	}
	SetupInfo(0);
	return 1;
}

static void CatCharByFlag(long val, long flag, int chr, SString & rBuf, int inverse)
{
	if((!inverse && val & flag) || (inverse && !(val & flag)))
		rBuf.CatChar(chr);
}

// virtual
int CheckPaneDialog::MessageError(int errCode, const char * pAddedMsg, long outputMode)
{
	int    dest = (outputMode & 0xff);
	SString err_msg;
	if(outputMode & eomBeep && !(Flags & fDisableBeep)) {
		for(int i = 0; i < 2; i++) {
			Beep(500,  200);
			Beep(1500, 200);
		}
	}
	if(dest == eomMsgWindow) {
		Flags |= fSuspSleepTimeout;
		PPError(errCode, pAddedMsg);
		Flags &= ~fSuspSleepTimeout;
	}
	else if(dest == eomStatusLine) {
		PPGetMessage(mfError, (errCode < 0) ? PPErrCode : errCode, pAddedMsg, 1, err_msg);
		SetupInfo(err_msg);
	}
	else if(dest == eomPopup) {
		PPGetMessage(mfError, (errCode < 0) ? PPErrCode : errCode, pAddedMsg, 1, err_msg);
		SMessageWindow::DestroyByParent(H()); // ������� � ������ ���������� ����������� //
		PPTooltipMessage(err_msg, 0, H(), 20000, GetColorRef(SClrRed),
			SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus|SMessageWindow::fLargeText);
	}
	return 0;
}

//virtual
int CheckPaneDialog::ConfirmMessage(int msgId, const char * pAddedMsg, int defaultResponse)
{
	if(pAddedMsg)
		PPSetAddedMsgString(pAddedMsg);
	return CONFIRM(msgId);
}

//virtual
int CheckPaneDialog::CDispCommand(int cmd, int iVal, double rv1, double rv2)
{
	int    ok = 1;
	if(P_CDY) {
		switch(cmd) {
			case cdispcmdClear:
				ok = P_CDY->ClearDisplay();
				break;
			case cdispcmdText:
				{
					if(iVal == cdisptxtOpened) {
						ok = P_CDY->OpenedCash();
					}
					else if(iVal == cdisptxtClosed) {
						ok = P_CDY->ClosedCash();
					}
					else
						ok = -1;
				}
				break;
			case cdispcmdTotal:
				ok = P_CDY->SetTotal(rv1);
				break;
			case cdispcmdTotalDiscount:
				ok = P_CDY->SetDiscount(rv1, rv2);
				break;
			case cdispcmdChange:
				ok = (P_CDY->ClearDisplay() && P_CDY->SetChange(rv1, rv2));
				break;
			case cdispcmdCurrentItem:
				if(P.CurPos < (int)P.getCount() && P.HasCur()) {
					P_CDY->ClearDisplay();
					if(P_CDY->SetGoodsName(P.GetCur().GoodsName)) {
						delay(50);
						ok = P_CDY->SetAmt(P.GetCur().NetPrice(), P.GetCur().Quantity);
					}
					else
						ok = 0;
				}
				else
					ok = -1;
				break;
			case cdispcmdCurrentGiftItem:
				if(P.CurPos < (int)P.getCount() && P.HasCur()) {
					P_CDY->ClearDisplay();
					if(P_CDY->SetGoodsName(P.GetCur().GoodsName)) {
						delay(50);
						ok = P_CDY->SetPresent();
					}
					else
						ok = 0;
				}
				else
					ok = -1;
				break;
		}
	}
	else
		ok = -1;
	return ok;
}
//
//
//
int CheckPaneDialog::SelectSerial(PPID goodsID, SString & rSerial, double * pPrice)
{
	rSerial = 0;
	int    ok = -1;
	PPObjBill * p_bobj = BillObj;
	SelLotBrowser::Entry * p_sel = 0;
	int    r, found = 0;
	uint   s = 0;
	const  LDATE curdt = LConfig.OperDate;
	double total_exp = 0.0; // ����� ������ ������ goodsID ��������� ��������
	SString serial;
	DateIter diter;
	SArray * p_ary = 0;
	SelLotBrowser * p_brw = 0;
	ReceiptCore & r_rcpt = p_bobj->trfr->Rcpt;
	ReceiptTbl::Rec lot_rec;
	StringSet seek_serial_list;
	const   PPID loc_id = GetCnLocID(goodsID); // @v8.8.0
	THROW(p_ary = SelLotBrowser::CreateArray());
	diter.Init(0, curdt);
	THROW(CC.CalcActiveExpendByGoods(goodsID, loc_id, 0, &total_exp));
	while((r = r_rcpt.EnumLots(goodsID, loc_id, &diter, &lot_rec)) > 0) {
		double exp = 0.0;
		double rest = lot_rec.Rest;
		p_bobj->GetSerialNumberByLot(lot_rec.ID, serial = 0, 1);
		if(serial.NotEmpty()) {
			//
			// ������ �� ���������� ����� ������� ������ �� ������ �����, ������� ���������� �����
			//
			if(!seek_serial_list.search(serial, 0, 0)) {
				THROW(CC.CalcActiveExpendByGoods(goodsID, loc_id, serial, &exp));
				{
					CCheckItem * p_item;
					for(uint i = 0; P.enumItems(&i, (void **)&p_item);)
						if(p_item->GoodsID == goodsID && serial.CmpNC(p_item->Serial) == 0) {
							exp += p_item->Quantity;
							total_exp += p_item->Quantity;
						}
				}
				seek_serial_list.add(serial);
			}
			rest -= exp;
			total_exp -= exp;
			if(rest > 0.0 || !(CnFlags & CASHF_ABOVEZEROSALE))
				THROW(SelLotBrowser::AddItemToArray(p_ary, &lot_rec, curdt, rest, 1));
		}
	}
	THROW(r);
	if(p_ary->getCount()) {
		THROW_MEM(p_brw = new SelLotBrowser(p_bobj, p_ary, s, 0)); // @newok
		if(ExecView(p_brw) == cmOK && (p_sel = (SelLotBrowser::Entry *)p_brw->view->getCurItem()) != 0) {
			if(strip(p_sel->Serial)[0] != 0) {
				ASSIGN_PTR(pPrice, p_sel->Price);
				rSerial = p_sel->Serial;
				ok = 1;
			}
		}
	}
	else
		ok = -2;
	CATCH
		if(p_brw == 0)
			delete p_ary;
		PPError();
	ENDCATCH
	delete p_brw;
	return ok;
}
//
//
//
int CheckPaneDialog::PreprocessGoodsSelection(PPID goodsID, PPID locID, PgsBlock & rBlk)
{
	int    ok = -1;
	SString temp_buf;
	const  PPID sc_id = CSt.GetID();
	if(goodsID == GetChargeGoodsID(sc_id)) {
		// @todo ����� ���� ��������� ��� �� ����� �� ��� ����� ChargeGoodsID �� ����� ��������� ����� ����
		ok = (sc_id && ScObj.IsCreditCard(sc_id)) ? 1 : MessageError(PPERR_INVUSAGECHARGEGOODS, 0, eomStatusLine);
	}
	// @v8.8.0 {
	else if(sc_id && IsOnlyChargeGoodsInPacket(sc_id, 0)) {
		SCardTbl::Rec sc_rec;
		if(ScObj.Search(sc_id, &sc_rec) > 0 && !ScObj.CheckRestrictions(&sc_rec, 0, getcurdatetime_()))
			ok = MessageError(PPERR_CHKPAN_SCINVONGOODS, sc_rec.Code, eomBeep | eomPopup/*eomStatusLine*/);
	}
	// } @v8.8.0
	if(ok) {
		SaComplex complex;
		if(LoadComplex(goodsID, complex) > 0) {
			if(InputComplexDinner(complex) > 0 && complex.IsComplete()) {
				if(complex.getCount()) {
					AcceptRow(0);
					for(uint i = 0; i < complex.getCount(); i++) {
						SaComplexEntry & r_entry = complex.at(i);
						CCheckItem chk_item;
						chk_item.GoodsID = NZOR(r_entry.FinalGoodsID, r_entry.GoodsID);
						GetGoodsName(chk_item.GoodsID, temp_buf);
						temp_buf.CopyTo(chk_item.GoodsName, sizeof(chk_item.GoodsName));
						GObj.FetchSingleBarcode(chk_item.GoodsID, temp_buf);
						temp_buf.CopyTo(chk_item.BarCode, sizeof(chk_item.BarCode));
						chk_item.Quantity = r_entry.Qtty;
						chk_item.Price = r_entry.FinalPrice;
						chk_item.Flags |= cifPartOfComplex;
						P.insert(&chk_item);
						SetupState(sLIST_EMPTYBUF);
					}
					OnUpdateList(1);
					ClearInput(0);
				}
			}
			ok = -1;
		}
		if(GObj.CheckFlag(goodsID, GF_GENERIC)) {
			PPObject::SetLastErrObj(PPOBJ_GOODS, labs(goodsID));
			ok = MessageError(PPERR_INVGENGOODSCCOP, 0, eomBeep | eomStatusLine);
		}
		else {
			const  int is_unlim = BIN(GObj.CheckFlag(goodsID, GF_UNLIM));
			if((!(CnFlags & CASHF_SELALLGOODS) || CnFlags & CASHF_ABOVEZEROSALE) && !is_unlim) {
				const double rest = CalcCurrentRest(goodsID, 1);
				if(rest < rBlk.Qtty) {
					if(rBlk.Qtty == 1.0 && rest >= 0.001) {
						rBlk.Qtty = round(rest, 0.001, -1);
						ok = 1;
					}
					else if(CnFlags & CASHF_ABOVEZEROSALE) {
						PPObject::SetLastErrObj(PPOBJ_GOODS, labs(goodsID));
						ok = MessageError(PPERR_LOTRESTBOUND, 0, eomBeep|eomStatusLine);
					}
					else {
						GetGoodsName(goodsID, temp_buf);
						ok = ConfirmMessage(PPCFM_GOODSRESTNOTENOUGH, temp_buf, 1) ? 1 : -2;
					}
				}
				else
					ok = 1;
			}
			else
				ok = 1;
			if(ok > 0) {
				if(Flags & fSelSerial && rBlk.Serial.Empty()) {
					int    r = SelectSerial(goodsID, rBlk.Serial, &rBlk.PriceBySerial);
					ok = (r > 0 || r == -2) ? 1 : -1;
				}
				if(ok > 0) {
                    if(oneof2(EgaisMode, 1, 2)) {
						if(P_EgPrc && P_EgPrc->IsAlcGoods(goodsID)) {
							PrcssrAlcReport::GoodsItem agi;
							if(P_EgPrc->PreprocessGoodsItem(goodsID, 0, 0, 0, agi) && agi.StatusFlags & agi.stMarkWanted) {
								SString egais_mark;
								rBlk.Qtty = 1.0; // ������������� ����������� ���������� - ������ �� ����� ����� �� ������ ����
								if(PPEgaisProcessor::InputMark(&agi, egais_mark) > 0) {
									int    dup_mark = 0;
									for(uint i = 0; !dup_mark && i < P.getCount(); i++) {
										const CCheckItem & r_item = P.at(i);
										if(r_item.EgaisMark == egais_mark)
											dup_mark = 1;
									}
									if(!dup_mark) {
										const CCheckItem & r_item = P.GetCur();
										if(r_item.EgaisMark == egais_mark)
											dup_mark = 1;
									}
									if(!dup_mark)
										rBlk.EgaisMark = egais_mark;
									else {
										PPSetError(PPERR_DUPEGAISMARKINCC, egais_mark);
										ok = -1;
									}
								}
								else
									ok = -1;
							}
						}
                    }
				}
			}
		}
	}
	return ok;
}

void CheckPaneDialog::SelectGoods__(int mode)
{
	int    r = 1;
	Flags |= fSuspSleepTimeout;
	if(CnFlags & CASHF_DISABLEZEROAGENT && !P.GetAgentID())
		r = MessageError(PPERR_CHKPAN_SALERNEEDED, 0, eomBeep | eomStatusLine);
	else if(Flags & fPrinted && !(OperRightsFlags & orfChgPrintedCheck))
		MessageError(PPERR_NORIGHTS, 0, eomBeep | eomStatusLine);
	else if(mode == sgmInnerGoodsList) {
		SmartListBox * p_list = (SmartListBox *)getCtrlView(CTL_CHKPAN_GDSLIST);
		if(p_list && p_list->def) {
			PPID   goods_id = 0;
			p_list->def->getCurID(&goods_id);
			PgsBlock pgsb(1.0);
			if(PreprocessGoodsSelection(goods_id, 0, pgsb) > 0)
				r = SetupNewRow(goods_id, pgsb);
			ClearInput(0);
		}
	}
	else if(mode == sgmModifier) {
		if(P.HasCur() && !(P.GetCur().Flags & cifModifier)) {
			const PPID main_goods_id = P.GetCur().GoodsID;
			if(main_goods_id) {
				SaModif mlist;
				if(LoadModifiers(main_goods_id, mlist) > 0) {
					class CpSelModDialog : public PPListDialog {
					public:
						enum {
							dummyFirst = 1,
							fontList,
							brSel,
							brOdd,
							brUnsel,
							brGrp,
							clrFocus,
							clrOdd,
							clrUnsel
						};

						CpSelModDialog(SaModif & rList) : PPListDialog(DLG_CPSELMOD, CTL_CPSELMOD_ELEMENTS)
						{
							List = rList;
							setSmartListBoxOption(CTL_CPSELMOD_ELEMENTS, lbtFocNotify);
							setSmartListBoxOption(CTL_CPSELMOD_ELEMENTS, lbtDblClkNotify);
							Ptb.SetColor(clrFocus,  RGB(0x20, 0xAC, 0x90));
							Ptb.SetColor(clrUnsel,  RGB(0xDA, 0xD7, 0xD0));
							Ptb.SetBrush(brSel,     SPaintObj::psSolid, Ptb.GetColor(clrFocus), 0);
							Ptb.SetBrush(brOdd,     SPaintObj::psSolid, Ptb.GetColor(clrOdd), 0);
							Ptb.SetBrush(brUnsel,   SPaintObj::psSolid, Ptb.GetColor(clrUnsel), 0);
							{
		 						SString temp_buf;
								LOGFONT log_font;
								MEMSZERO(log_font);
								log_font.lfCharSet = DEFAULT_CHARSET;
								ListEntryGap = 5;
								PPGetSubStr(PPTXT_FONTFACE, PPFONT_ARIAL, temp_buf);
								STRNSCPY(log_font.lfFaceName, temp_buf); // @unicodeproblem
								log_font.lfHeight = (DEFAULT_TS_FONTSIZE - TSGGROUPSASITEMS_FONTDELTA);
								Ptb.SetFont(fontList, ::CreateFontIndirect(&log_font));
							}
							updateList(-1);
						}
					private:
						DECL_HANDLE_EVENT
						{
							if(event.isCmd(cmLBDblClk)) {
								long   idx = 0;
								if(getSelection(&idx) && idx > 0 && idx <= (long)List.getCount())
									TVCMD = cmOK;
							}
							else if(event.isCmd(cmClear)) {
								if(IsInState(sfModal)) {
									endModal(cmClear);
									return; // ����� endModal �� ������� ���������� � this
								}
							}
							PPListDialog::handleEvent(event);
						}
						virtual int setupList()
						{
							int    ok = 1;
							SString sub;
							StringSet ss(SLBColumnDelim);
							for(uint i = 0; i < List.getCount(); i++) {
								ss.clear(1);
								SaModifEntry & r_entry = List.at(i);
								GetGoodsName(r_entry.GoodsID, sub);
								ss.add(sub);
								ss.add((sub = 0).Cat(r_entry.Qtty, MKSFMTD(0, 3, 0)));
								ss.add((sub = 0).Cat(r_entry.Price, SFMT_MONEY));
								addStringToList(i+1, ss.getBuf());
							}
							return ok;
						}
						SaModif List;
						SPaintToolBox Ptb;
						long   ListEntryGap;
					};
					CpSelModDialog * dlg = new CpSelModDialog(mlist);
					if(CheckDialogPtr(&dlg)) {
						// @v8.9.0 {
						{
							SString label_text, goods_name, new_label_text;
							GetGoodsName(main_goods_id, goods_name);
							dlg->getLabelText(CTL_CPSELMOD_ELEMENTS, label_text);
							new_label_text.Printf(label_text, goods_name.cptr());
							dlg->setLabelText(CTL_CPSELMOD_ELEMENTS, new_label_text);
						}
						// @v8.9.0 {
						dlg->enableCommand(cmClear, BIN(P.CurModifList.getCount()));
						int    cmd = ExecView(dlg);
						if(cmd == cmOK) {
							long   idx = -1;
							if(dlg->getSelection(&idx) && idx > 0 && idx <= (long)mlist.getCount()) {
								const SaModifEntry & r_entry = mlist.at(idx-1);
								P.CurModifList.insert(&r_entry);
								SetupRowData(1);
							}
						}
						else if(cmd == cmClear) {
							P.CurModifList.freeAll();
							SetupRowData(1);
						}
					}
					ZDELETE(dlg);
				}
			}
		}
	}
	else if(oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
		if(mode == sgmNormal)
			setupRetCheck(1);
		else
			ClearInput(0);
	}
	else if(mode == sgmRandom) {
		PPIDArray  rand_gds_ary;
		if(GObj.GetRandomIdsAry(100, &rand_gds_ary) > 0) {
			for(uint i = 0; i < rand_gds_ary.getCount(); i++) {
				PgsBlock pgsb(1.0);
				SetupNewRow(rand_gds_ary.at(i), pgsb);
			}
			AcceptRow();
		}
	}
	else if(Flags & fTouchScreen){
		if(mode == sgmByPrice)
			UpdateGList(-2, 0);
	}
	else if(mode == sgmByPrice && !GetInput()) {
		INVERSEFLAG(Flags, fSelByPrice);
	}
	else {
		Flags &= ~fSelByPrice;
		SETIFZ(P_EGSDlg, new ExtGoodsSelDialog(GetCashOp(), 0, (CnFlags & CASHF_SELALLGOODS) ? ExtGoodsSelDialog::fForceExhausted : 0));
		if(CheckDialogPtr(&P_EGSDlg, 1)) {
			PPWait(1);
			SString temp_buf;
			const int inp_not_empty = GetInput();
			if(inp_not_empty) {
				temp_buf = Input;
				if(mode == sgmByPrice && temp_buf.ToReal() != 0.0)
					P_EGSDlg->setSelectionByPrice(R2(temp_buf.ToReal()));
				else {
					StrAssocArray goods_list;
					if(temp_buf.Len() >= INSTVSRCH_THRESHOLD && temp_buf.C(0) != '!')
						temp_buf.Insert(0, "!");
					if(GObj.P_Tbl->GetListBySubstring(temp_buf, &goods_list, -1, 1))
						P_EGSDlg->setSelectionByGoodsList(&goods_list);
				}
				ClearInput(0);
			}
			else {
				P_EGSDlg->setSelectionByGroup();
				SetupInfo(0);
			}
			PPWait(0);
			if(ExecView(P_EGSDlg) == cmOK) {
				TIDlgInitData tidi;
				if(P_EGSDlg->getDTS(&tidi) > 0) {
					PgsBlock pgsb(tidi.Quantity);
					if(PreprocessGoodsSelection(tidi.GoodsID, 0, pgsb) > 0)
						r = SetupNewRow(tidi.GoodsID, pgsb);
					else
						r = 0;
				}
			}
		}
	}
	Flags &= ~fSuspSleepTimeout;
	if(r)
		SetupInfo(0);
}

void CheckPaneDialog::AcceptQuantity()
{
	int    ok = -1;
	const PPID goods_id = P.HasCur() ? P.GetCur().GoodsID : 0;
	if(goods_id) {
		SString temp_buf;
		CCheckItem & r_cur = P.GetCur();
		const CCheckItem preserve_item = r_cur;
		int    is_input = GetInput();
		double prev_qtty = r_cur.Quantity;
		double qtty = 0.0;
		if(!is_input && ScaleID) {
			int  r = 0;
			while(r == 0 && !(Flags & fNotUseScale)) {
				r = GetDataFromScale(0, &qtty);
				if(r > 0)
					is_input = 1;
				else if(!r && PPMessage(mfConf|mfYesNo, PPCFM_SCALENOTREADY, 0) != cmYes)
					Flags |= fNotUseScale;
			}
		}
		if(!is_input) {
			showInputLineCalc(this, CTL_CHKPAN_INPUT);
			is_input = GetInput();
		}
		if(is_input) {
			int    last_slash = 0;
			Input.ShiftLeftChr('*').TrimRightChr('*');
			if(Input.Last() == '/') {
				last_slash = 1;
				Input.TrimRight();
			}
			if(qtty == 0.0) {
				double dr = 0.0, dd = 1.0;
				SString dr_buf, dd_buf;
				if(Input.Divide('/', dr_buf, dd_buf) > 0) {
					dr = dr_buf.ToReal();
					dd = dd_buf.ToReal();
					qtty = R6((dd != 0.0) ? (dr / dd) : dr);
				}
				else
					qtty = R6(Input.ToReal());
			}
			if(qtty > 0.0) {
				r_cur.PhQtty = 0.0;
				if(last_slash) {
					double phuperu;
					if(GObj.GetPhUPerU(goods_id, 0, &phuperu) > 0) {
						r_cur.PhQtty = qtty;
						qtty = R6(qtty / phuperu);
					}
				}
				//
				// ������������� ����������� ���������� - ������ �� ����� ����� �� ������ ����
				//
				if(oneof2(EgaisMode, 1, 2) && P_EgPrc && P_EgPrc->IsAlcGoods(goods_id)) {
					PrcssrAlcReport::GoodsItem agi;
					if(P_EgPrc->PreprocessGoodsItem(goods_id, 0, 0, 0, agi) && agi.StatusFlags & agi.stMarkWanted) {
						if(qtty != 1.0)
							ok = MessageError(PPERR_EGAIS_MARKEDQTTY, 0, eomBeep|eomStatusLine);
					}
				}
				//
				// �������� �� �� ���������� �������� ������� (��� ������������� ����� CASHF_ABOVEZEROSALE)
				//
				if(ok && CnFlags & CASHF_ABOVEZEROSALE && !GObj.CheckFlag(goods_id, GF_UNLIM)) {
					double rest = CalcCurrentRest(goods_id, 0);
					if(rest < qtty) {
						PPObject::SetLastErrObj(PPOBJ_GOODS, labs(goods_id));
						ok = MessageError(PPERR_LOTRESTBOUND, 0, eomBeep|eomStatusLine);
					}
				}
				//
				// �������� �� ��������� ������� ��������� //
				//
				if(ok && CsObj.GetEqCfg().Flags & PPEquipConfig::fRestrictQttyByUnitRnd) {
					Goods2Tbl::Rec goods_rec;
					PPUnit u_rec;
					if(GObj.Fetch(goods_id, &goods_rec) > 0 && GObj.FetchUnit(goods_rec.UnitID, &u_rec) > 0) {
						if(u_rec.Rounding > 0.0) {
							const double _r = round(qtty, u_rec.Rounding, 0); // @v8.8.1 direction: -1-->0
							// @v8.8.1 if(_r != qtty) {
							if(!feqeps(_r, qtty, 1E-7)) { // @v8.8.1
								(temp_buf = 0).Cat(u_rec.Rounding, MKSFMTD(0, 6, NMBF_NOTRAILZ));
								ok = MessageError(PPERR_QTTYMUSTBERND, temp_buf, eomStatusLine|eomBeep);
							}
						}
						else if(u_rec.Flags & PPUnit::IntVal) {
							if(ffrac(qtty) != 0.0)
								ok = MessageError(PPERR_QTTYMUSTBEINT, 0, eomStatusLine|eomBeep);
						}
					}
				}
				if(ok) {
					r_cur.Quantity = (goods_id == GetChargeGoodsID(CSt.GetID())) ? fabs(R3(qtty)) : qtty;
					//
					// ��� ��� ��������� ��������� ����� �������� �� ����������, ��� ��������� ���������� ����������
					// ����� ���������� ����.
					//
					if(fabs(qtty) != fabs(prev_qtty) && !(r_cur.Flags & cifPriceBySerial)) {
						RetailGoodsInfo rgi;
						GetRgi(goods_id, qtty, 0, rgi);
						if(rgi.Price != 0.0 && r_cur.Price != 0 && rgi.Price != r_cur.Price)
							r_cur.Price = rgi.Price;
					}
					//
					if(F(fRetCheck)) {
						r_cur.Quantity = -r_cur.Quantity;
						r_cur.PhQtty   = -r_cur.PhQtty;
						SetupRowData(0);
						ok = 1;
					}
					else if(!CalcRestByCrdCard_(1)) {
						r_cur = preserve_item;
						ok = 0;
					}
					else {
						SetupRowData(0);
						ok = 1;
					}
				}
			}
		}
	}
	ClearInput(0);
}

void CheckPaneDialog::AcceptDivision()
{
	int    is_input = GetInput();
	if(!is_input) {
		showInputLineCalc(this, CTL_CHKPAN_INPUT);
		is_input = GetInput();
	}
	if(P.HasCur() && P.GetCur().GoodsID && is_input && Input.IsDigit()) {
		long  div = Input.ToLong();
		if(div > 0 && div < 1000) {
			P.GetCur().Division = (int16)div;
			SetupRowData(0);
		}
	}
	ClearInput(0);
}

int CheckPaneDialog::AcceptRowDiscount()
{
	int    ok = -1;
	if(oneof3(GetState(), sEMPTYLIST_BUF, sLIST_BUF, sLISTSEL_BUF) && GetInput()) {
		char   prefx = Input[0];
		char   postfx = (strlen(Input) > 0) ? Input[strlen(Input) - 1] : 0;
		double pct_dis = 0.0;
		int    is_row_dis = 1;
		if(oneof3(prefx, '%', '/', '\\'))
			pct_dis = atof(Input + 1);
		else if(oneof3(postfx, '%', '/', '\\'))
			pct_dis = atof(Input);
		else
			is_row_dis = 0;
		if(is_row_dis) {
			if(pct_dis >= 0.0 && pct_dis <= 100.0) {
				if(OperRightsFlags & orfRowDiscount) { // @v8.4.8 @fix orfPrintCheck-->orfRowDiscount
					const  double price = P.GetCur().Price;
					double discount = round((price / 100.0) * pct_dis, 2);
					P.GetCur().Price = price - discount;
					P.GetCur().Flags |= cifFixedPrice; // @v8.8.1
					Flags &= ~fWaitOnSCard;
					SetupRowData(1);
					{
						CCheckLineTbl::Rec row;
						MEMSZERO(row);
						row.CheckID  = SelPack.Rec.ID;
						row.GoodsID  = P.GetCur().GoodsID;
						row.Quantity = P.GetCur().Quantity;
						row.Price    = dbltointmny(price);
						row.Dscnt    = discount;
						CC.WriteCCheckLogFile(&SelPack, &row, CCheckCore::logRowDiscount, 1);
					}
					ok = 1;
				}
				else
					ok = MessageError(PPERR_NORIGHTS, 0, eomBeep|eomStatusLine);
			}
			else
				ok = MessageError(PPERR_PERCENTINPUT, 0, eomBeep|eomMsgWindow);
		}
	}
	return ok;
}

class SCardInfoDialog : public PPListDialog {
public:
	SCardInfoDialog(int asSelector) : PPListDialog(DLG_SCARDVIEW, CTL_SCARDVIEW_LIST)
	{
		LocalState = 0;
		if(asSelector)
			LocalState |= stAsSelector;
		SCardID = 0;
		OwnerID = 0;
		addGroup(GRP_IBG, new ImageBrowseCtrlGroup(PPTXT_PICFILESEXTS, CTL_SCARDVIEW_IMAGE, cmAddImage, cmDelImage,
			PsnObj.CheckRights(PSNRT_UPDIMAGE), ImageBrowseCtrlGroup::fUseExtOpenDlg));
		selectCtrl(CTL_SCARDVIEW_INPUT);
		Ptb.SetColor(clrRed,   GetColorRef(SClrRed));
		Ptb.SetColor(clrGreen, GetColorRef(SClrGreen));
		Ptb.SetColor(clrYellow, GetColorRef(SClrYellow));
		Ptb.SetBrush(brRed,    SPaintObj::psSolid, Ptb.GetColor(clrRed),   0);
		Ptb.SetBrush(brGreen,  SPaintObj::psSolid, Ptb.GetColor(clrGreen), 0);
		Ptb.SetBrush(brYellow, SPaintObj::psSolid, Ptb.GetColor(clrYellow), 0);
		Ptb.SetBrush(brOrange, SPaintObj::psSolid, GetColorRef(SClrOrange), 0);
		Ptb.SetBrush(brMovCrdRest, SPaintObj::psSolid, SClrDarkviolet,  0);

		// @v9.1.11 PPGetWord(PPWORD_CHECKS,     1, ChecksText);
		PPLoadString("check_pl", ChecksText);
		ChecksText.Transf(CTRANSF_INNER_TO_OUTER); // @v9.1.11
		// @v9.1.11 PPGetWord(PPWORD_OPERATIONS, 1, OperationsText);
		PPLoadString("op_pl", OperationsText); // @v9.1.11
		OperationsText.Transf(CTRANSF_INNER_TO_OUTER); // @v9.1.11
		if(!(LocalState & stAsSelector))
			showCtrl(STDCTL_OKBUTTON, 0);
		showButton(cmActivate, 0);
		SetupMode(modeCheckView, 1);
	}
	int    setDTS(const PPID * pData)
	{
		return SetupCard(pData ? *pData : 0);
	}
	int    getDTS(PPID * pData)
	{
		int    ok = 1;
		ImageBrowseCtrlGroup::Rec rec;
		if(OwnerID && PsnObj.CheckRights(PSNRT_UPDIMAGE) && getGroupData(GRP_IBG, &rec) && rec.Flags & ImageBrowseCtrlGroup::Rec::fUpdated) {
			long   set_f = 0;
			long   reset_f = 0;
			ObjLinkFiles _lf(PPOBJ_PERSON);
			_lf.Load(OwnerID, 0L);
			if(rec.Path.NotEmptyS() && fileExists(rec.Path)) {
				_lf.Replace(0, rec.Path);
				set_f = PSNF_HASIMAGES;
			}
			else {
				_lf.Remove(0);
				reset_f = PSNF_HASIMAGES;
			}
			THROW(_lf.Save(OwnerID, 0L));
			THROW(PsnObj.P_Tbl->UpdateFlags(OwnerID, set_f, reset_f, 1));
		}
		ASSIGN_PTR(pData, SCardID);
		CATCH
			ok = PPErrorZ();
		ENDCATCH
		return ok;
	}
private:
	enum {
		dummyFirst = 1,
		brRed,             // ������� �����
		brGreen,           // ������� �����
		brYellow,          // ������ �����
		brOrange,          // ��������� �����
		brMovCrdRest,      // ���� ���� ������� �� ����� � ������ �������� �������� � ������ ����
		clrRed,
		clrGreen,
		clrYellow,
	};

	DECL_HANDLE_EVENT;
	virtual int editItem(long pos, long id);
	virtual int setupList();
	int    SetupMode(long mode, int force);
	int    SetupCard(PPID scardID);
	void   SetupMovCrd();
	void   CommitMovCrd();
	enum {
		modeCheckView = 1,   // ����� ��������� ����� �� �����
		modeOpView,          // ����� ��������� �������� ����������/�������� �� ��������� �����
		modeSelectByOwner,   // ����� ������ ����� �� ���������
		modeMovCrd,          // ����� �������� ��������� �������� � ������ ���� �� ��������� �����
		modeSelectByMultCode // @v8.1.6 ����� ������ ����� �� ����, ������� ���� � ��� �� ���
	};
	enum {
		stCreditCard     = 0x0002,
		stAsSelector     = 0x0004,
		stWarnCardInfo   = 0x0008, // ���������� � ����� ������ ������������ ��� ����������� �������� � �������������� �����
		stNeedActivation = 0x0010, // ����� ������� ���������. ���������� ������������ � ������ ����.
		stAutoActivation = 0x0020  // ������������� ����� //
	};
	struct SpcListItem {
		PPID   PersonID;
		PPID   SCardID;
		PPID   SCardSerID;
		uint   PersonNamePos;
		uint   SCardCodePos;
		LDATE  SCardExpiry;
		double Rest;
		double Amount;
	};
	class  SpcArray : public TSArray <SpcListItem> {
	public:
		SpcArray() : TSArray <SpcListItem> ()
		{
			StrPool.add("$");
		}
		void   Clear()
		{
			freeAll();
			StrPool.clear();
			StrPool.add("$");
		}
		int    Add(PPID cardID, PPID cardSerID, const char * pCardCode, double rest, double amount)
		{
			int    ok = 1;
			SString temp_buf;
			SpcListItem item;
			MEMSZERO(item);
			item.SCardID = cardID;
			item.SCardSerID = cardSerID;
			item.Rest = rest;
			item.Amount = amount;
			temp_buf = pCardCode;
			if(!temp_buf.NotEmptyS())
				ideqvalstr(cardID, temp_buf);
			StrPool.add(temp_buf, &item.SCardCodePos);
			insert(&item);
			return ok;
		}
		PPID   Get(uint pos, SString & rCardCode, SString & rCardSerName, double * pRest, double * pAmount)
		{
			rCardCode = 0;
			rCardSerName = 0;
			if(pos < getCount()) {
				const SpcListItem & r_item = at(pos);
				StrPool.get(r_item.SCardCodePos, rCardCode);
				if(r_item.SCardSerID) {
					PPSCardSeries scs_rec;
					if(ScsObj.Fetch(r_item.SCardSerID, &scs_rec) > 0) {
						rCardSerName = scs_rec.Name;
					}
				}
				ASSIGN_PTR(pRest, r_item.Rest);
				ASSIGN_PTR(pAmount, r_item.Amount);
				return r_item.SCardID;
			}
			else
				return 0;
		}
		int    Add(PPID personID, PPID cardID, PPID cardSerID, const char * pPersonName, const char * pCardCode, LDATE expiry)
		{
			int    ok = 1;
			SString temp_buf;
			SpcListItem item;
			MEMSZERO(item);
			item.PersonID = personID;
			item.SCardID = cardID;
			item.SCardSerID = cardSerID;
			temp_buf = pPersonName;
			if(!temp_buf.NotEmptyS())
				ideqvalstr(personID, temp_buf);
			StrPool.add(temp_buf, &item.PersonNamePos);
			temp_buf = pCardCode;
			if(!temp_buf.NotEmptyS())
				ideqvalstr(cardID, temp_buf);
			StrPool.add(temp_buf, &item.SCardCodePos);
			item.SCardExpiry = expiry;
			insert(&item);
			return ok;
		}
		PPID   Get(uint pos, SString & rPersonName, SString & rCardCode, SString & rCardSerName, LDATE & rExpiry)
		{
			rPersonName = 0;
			rCardCode = 0;
			rCardSerName = 0;
			rExpiry = ZERODATE;
			if(pos < getCount()) {
				const SpcListItem & r_item = at(pos);
				rExpiry = r_item.SCardExpiry;
				StrPool.get(r_item.PersonNamePos, rPersonName);
				StrPool.get(r_item.SCardCodePos, rCardCode);
				if(r_item.SCardSerID) {
					PPSCardSeries scs_rec;
					if(ScsObj.Fetch(r_item.SCardSerID, &scs_rec) > 0) {
						rCardSerName = scs_rec.Name;
					}
				}
				return r_item.SCardID;
			}
			else
				return 0;
		}
	private:
		StringSet StrPool;
		PPObjSCardSeries ScsObj;
	};

	long   Mode;
	long   LocalState;
	PPID   SCardID;
	PPID   OwnerID; // ����������-�������� �����
	SString ChecksText;
	SString OperationsText;
	SPaintToolBox Ptb;
	PPObjSCard  ScObj;
	PPObjPerson PsnObj;
	SpcArray OwnerList;
};

// virtual
int SCardInfoDialog::editItem(long pos, long id)
{
	if(id) {
		if(Mode == modeCheckView) {
			PPID   cn_id = 0;
			ScObj.P_CcTbl->GetNodeID(id, &cn_id);
			CCheckPane(cn_id, id);
		}
		else if(oneof2(Mode, modeSelectByOwner, modeSelectByMultCode)) {
			SetupCard(id);
		}
	}
	return -1;
}

int SCardInfoDialog::SetupCard(PPID scardID)
{
	SString temp_buf, card, info_buf, psn_name;
	ImageBrowseCtrlGroup::Rec ibg_rec;
	SCardTbl::Rec sc_rec;
	SCardID = scardID;
	OwnerID = 0;
	LocalState &= ~(stCreditCard|stWarnCardInfo|stNeedActivation|stAutoActivation);
	MEMSZERO(sc_rec);
	setStaticText(CTL_SCARDVIEW_SCINFO, info_buf = 0);
	setStaticText(CTL_SCARDVIEW_OWNERINFO, info_buf);
	if(ScObj.Search(SCardID, &sc_rec) > 0) {
		SETFLAG(LocalState, stCreditCard, ScObj.IsCreditSeries(sc_rec.SeriesID));
		card = sc_rec.Code;
		{
			LDATETIME cur_dtm = getcurdatetime_();
			info_buf = 0;
			if(sc_rec.Expiry) {
				PPLoadString("validuntil-fem", temp_buf);
				info_buf.CatDiv(' ', 0, 1).Cat(temp_buf).CatDiv(':', 2).Cat(sc_rec.Expiry, DATF_DMY);
				if(sc_rec.Expiry < cur_dtm.d)
					LocalState |= stWarnCardInfo;
			}
			{
				SString added_msg_buf;
				TSArray <SCardCore::OpBlock> frz_op_list;
				if(ScObj.P_Tbl->GetFreezingOpList(SCardID, frz_op_list) > 0) {
					uint   info_pos = 0;
					for(uint i = 0; i < frz_op_list.getCount(); i++) {
						const SCardCore::OpBlock & r_ob = frz_op_list.at(i);
						if(r_ob.CheckFreezingPeriod(ZERODATE)) {
							if(r_ob.FreezingPeriod.CheckDate(cur_dtm.d)) {
								LocalState |= stWarnCardInfo;
								info_pos = i+1;
								break;
							}
							else if(!info_pos && r_ob.FreezingPeriod.low > cur_dtm.d) {
								info_pos = i+1;
							}
						}
					}
					if(info_pos) {
						const SCardCore::OpBlock & r_ob = frz_op_list.at(info_pos-1);
						info_buf.CatDiv(' ', 0, 1).CatChar('<').Cat(r_ob.FreezingPeriod).CatChar('>');
					}
				}
			}
			if(sc_rec.UsageTmStart || sc_rec.UsageTmEnd) {
				PPLoadString("time", temp_buf);
				info_buf.CatDiv(' ', 0, 1).Cat(temp_buf).CatDiv(':', 2);
				if(sc_rec.UsageTmStart) {
					info_buf.Cat(sc_rec.UsageTmStart, TIMF_HM);
					if(sc_rec.UsageTmStart > cur_dtm.t)
						LocalState |= stWarnCardInfo;
				}
				if(sc_rec.UsageTmEnd) {
					info_buf.Dot().Dot().Cat(sc_rec.UsageTmEnd, TIMF_HM);
					if(sc_rec.UsageTmEnd < cur_dtm.t)
						LocalState |= stWarnCardInfo;
				}
			}
			if(sc_rec.Flags & SCRDF_CLOSED) {
				if(sc_rec.Flags & SCRDF_NEEDACTIVATION) {
					LocalState |= stNeedActivation;
					if(sc_rec.Flags & SCRDF_AUTOACTIVATION)
						LocalState |= stAutoActivation;
				}
				else {
					LocalState |= stWarnCardInfo;
				}
			}
			if(sc_rec.PDis) {
				PPLoadString("discount", temp_buf);
				info_buf.CatDiv(' ', 0, 1).Cat(temp_buf).CatDiv(':', 2).Cat(fdiv100i(sc_rec.PDis), MKSFMTD(0, 3, NMBF_NOTRAILZ)).CatChar('%');
			}
			setStaticText(CTL_SCARDVIEW_SCINFO, info_buf);
		}
		info_buf = 0;
		PPPersonPacket pack;
		if(PsnObj.GetPacket(sc_rec.PersonID, &pack, 0) > 0) {
			OwnerID = sc_rec.PersonID;
			psn_name = pack.Rec.Name;
			pack.LinkFiles.Init(PPOBJ_PERSON);
			if(pack.Rec.Flags & PSNF_HASIMAGES) {
				pack.LinkFiles.Load(pack.Rec.ID, 0L);
				pack.LinkFiles.At(0, ibg_rec.Path);
			}
			{
				PPObjPersonStatus ps_obj;
				PPPersonStatus ps_rec;
				if(ps_obj.Fetch(pack.Rec.Status, &ps_rec) > 0 && ps_rec.Flags & PSNSTF_PRIVATE) {
					const ObjTagItem * p_dob_tag = pack.TagL.GetItem(PPTAG_PERSON_DOB);
					if(p_dob_tag) {
						LDATE  dob = ZERODATE;
						p_dob_tag->GetDate(&dob);
						if(checkdate(dob, 0)) {
							LDATE curdt = getcurdate_();
							int years = curdt.year() - dob.year();
							curdt.setyear(dob.year());
							if(curdt < dob)
								years--;
							PPLoadString("age", temp_buf);
							info_buf.Cat(temp_buf).CatDiv(':', 2).Cat(years).Space().CatChar('(').Cat(dob, DATF_DMY|DATF_CENTURY).CatChar(')');
						}
					}
				}
			}
			{
				SString phone;
				if(pack.ELA.GetSinglePhone(phone, 0) > 0) {
					PPLoadString("phone", temp_buf);
					info_buf.Space().Space().Cat(temp_buf).CatDiv(':', 2).Cat(phone);
				}
			}
		}
		else
			info_buf = 0;
		setStaticText(CTL_SCARDVIEW_OWNERINFO, info_buf);
		setGroupData(GRP_IBG, &ibg_rec);
		{
			const int enbl_psn = BIN(OwnerID && PsnObj.CheckRights(PPR_MOD));
			const int enbl_pic = BIN(/* @v8.5.10 enbl_psn &&*/ PsnObj.CheckRights(PSNRT_UPDIMAGE));
			enableCommand(cmAddImage, enbl_pic);
			enableCommand(cmDelImage, enbl_pic);
			enableCommand(cmPasteImage, enbl_pic);
			enableCommand(cmEditPerson, enbl_psn);
		}
		showButton(cmActivate, (LocalState & stNeedActivation));
		OwnerList.Clear();
		updateList(-1);
	}
	else {
		SCardID = 0;
		setGroupData(GRP_IBG, &ibg_rec);
	}
	setCtrlData(CTL_SCARDVIEW_SALDO,   &sc_rec.Rest);
	setCtrlString(CTL_SCARDVIEW_OWNER, psn_name);
	setCtrlString(CTL_SCARDVIEW_CARD,  card);
	enableCommand(cmCheckOpSwitch, LocalState & stCreditCard && ScObj.CheckRights(SCRDRT_VIEWOPS));
	enableCommand(cmSCardMovCrd,   LocalState & stCreditCard && ScObj.CheckRights(SCRDRT_ADDOPS));
	SetupMode((LocalState & stCreditCard) ? modeOpView : modeCheckView, 0);
	return 1;
}

int SCardInfoDialog::setupList()
{
	int    ok = -1;
	SString buf;
	StringSet ss(SLBColumnDelim);
	if(Mode == modeCheckView) {
		if(SCardID) {
			CCheckFilt flt;
			CCheckViewItem item;
			PPViewCCheck view;
			flt.SCardID = SCardID;
			THROW(view.Init_(&flt));
			for(view.InitIteration(0); view.NextIteration(&item) > 0;) {
				if(!(item.Flags & CCHKF_SKIP)) {
					ss.clear();
					ss.add((buf = 0).Cat(item.Dt), 0);                                // ����
					ss.add((buf = 0).Cat(item.Tm), 0);                                // ����� //
					ss.add((buf = 0).Cat(item.CashID), 0);                            // �����
					ss.add((buf = 0).Cat(item.Code), 0);                              // ����� ����
					ss.add((buf = 0).Cat(MONEYTOLDBL(item.Amount), SFMT_MONEY), 0);   // �����
					ss.add((buf = 0).Cat(MONEYTOLDBL(item.Discount), SFMT_MONEY), 0); // ������
					THROW(addStringToList(item.ID, ss.getBuf()));
				}
			}
			ok = 1;
		}
	}
	else if(Mode == modeOpView) {
		if(SCardID) {
			SCardOpFilt flt;
			SCardOpViewItem item;
			PPViewSCardOp   view;
			flt.SCardID = SCardID;
			THROW(view.Init_(&flt));
			view.InitIteration();
			for(uint i = 1; view.NextIteration(&item) > 0; i++) {
				ss.clear();
				ss.add((buf = 0).Cat(item.Dt), 0);                 // ����
				ss.add((buf = 0).Cat(item.Tm), 0);                 // ����� //
				if(item.Flags & SCARDOPF_FREEZING) {
					DateRange frz_prd;
					frz_prd.Set(item.FreezingStart, item.FreezingEnd);
					ss.add((buf = 0).Cat(frz_prd));
					ss.add(buf = 0);
				}
				else {
					ss.add((buf = 0).Cat(item.Amount, SFMT_MONEY|NMBF_NOZERO), 0); // �����
					ss.add((buf = 0).Cat(item.Rest, SFMT_MONEY|NMBF_NOZERO), 0);   // �������
				}
				THROW(addStringToList(i, ss.getBuf()));
			}
			ok = 1;
		}
	}
	else if(oneof2(Mode, modeSelectByOwner, modeSelectByMultCode)) {
		SString person_name, card_code, ser_name, temp_buf;
		for(uint i = 0; i < OwnerList.getCount(); i++) {
			LDATE  expiry;
			PPID   card_id = OwnerList.Get(i, person_name, card_code, ser_name, expiry);
			ss.clear();
			ss.add(person_name);
			ss.add(card_code);
			ss.add(ser_name);
			temp_buf = 0;
			if(checkdate(expiry, 0))
				temp_buf.Cat(expiry);
			ss.add(temp_buf);
			THROW(addStringToList(card_id, ss.getBuf()));
		}
	}
	else if(Mode == modeMovCrd) {
		SString person_name, card_code, ser_name;
		for(uint i = 0; i < OwnerList.getCount(); i++) {
			double rest = 0.0, amount = 0.0;
			PPID card_id = OwnerList.Get(i, card_code, ser_name, &rest, &amount);
			ss.clear();
			ss.add(card_code);
			ss.add(ser_name);
			ss.add((buf = 0).Cat(rest, MKSFMTD(0, 2, NMBF_NOZERO)));
			ss.add((buf = 0).Cat(amount, MKSFMTD(0, 2, NMBF_NOZERO)));
			THROW(addStringToList(card_id, ss.getBuf()));
		}
	}
	CATCHZOKPPERR
	return ok;
}

int SCardInfoDialog::SetupMode(long mode, int force)
{
	long   prev_mode = Mode;
	if(oneof5(mode, modeCheckView, modeOpView, modeSelectByOwner, modeMovCrd, modeSelectByMultCode) && (force || mode != Mode)) {
		int    skip = 0;
		if(mode == modeMovCrd && !ScObj.CheckRights(SCRDRT_ADDOPS))
			skip = 1;
		else if(mode == modeOpView && !ScObj.CheckRights(SCRDRT_VIEWOPS))
			skip = 1;
		if(!skip) {
			Mode = mode;
			enableCommand(cmaEdit, Mode == modeCheckView);
			enableCommand(cmSelectByOwner, Mode != modeMovCrd);
			enableCommand(cmCommit, Mode == modeMovCrd && OwnerList.getCount());
			showButton(cmCommit, Mode == modeMovCrd && OwnerList.getCount());
			if(P_Box) {
				SString columns_buf, text, temp_buf;
				if(Mode == modeCheckView) {
					setButtonText(cmCheckOpSwitch, OperationsText);
					setLabelText(CTL_SCARDVIEW_LIST, (text = ChecksText).Transf(CTRANSF_OUTER_TO_INNER));
					columns_buf = "@lbt_scardcheck";
				}
				else if(Mode == modeOpView) {
					setButtonText(cmCheckOpSwitch, ChecksText);
					setLabelText(CTL_SCARDVIEW_LIST, (text = OperationsText).Transf(CTRANSF_OUTER_TO_INNER));
					columns_buf = "@lbt_scardop";
				}
				else if(Mode == modeSelectByOwner) {
					PPLoadString("person", temp_buf);
					setLabelText(CTL_SCARDVIEW_LIST, temp_buf);
					columns_buf = "@lbt_selscardbyowner";
				}
				else if(Mode == modeSelectByMultCode) {
					PPLoadString("code", temp_buf);
					setLabelText(CTL_SCARDVIEW_LIST, temp_buf);
					columns_buf = "@lbt_selscardbyowner";
				}
				else if(Mode == modeMovCrd) {
					// @v8.5.10 (temp_buf = "������� �������� � ����...").Transf(CTRANSF_OUTER_TO_INNER);
					PPLoadText(PPTXT_MOVSCARDREST, temp_buf); // @v8.5.10
					setLabelText(CTL_SCARDVIEW_LIST, temp_buf);
					columns_buf = "@lbt_scardmovlist";
				}
				enableCommand(cmCheckOpSwitch, !oneof2(Mode, modeSelectByOwner, modeSelectByMultCode));
				P_Box->SetupColumns(columns_buf);
				updateList(-1);
			}
		}
	}
	return prev_mode;
}

void SCardInfoDialog::SetupMovCrd()
{
	double rest = 0.0;
	ScObj.P_Tbl->GetRest(SCardID, MAXDATE, &rest);
	for(uint i = 0; i < OwnerList.getCount(); i++) {
		double v = 0.0;
		double amt = OwnerList.at(i).Amount;
		ScObj.P_Tbl->GetRest(OwnerList.at(i).SCardID, MAXDATE, &v);
		if(amt > 0.0 && amt <= v)
			rest += amt;
	}
	setCtrlReal(CTL_SCARDVIEW_SALDO, rest);
	enableCommand(cmCommit, OwnerList.getCount());
	showButton(cmCommit, OwnerList.getCount());
	updateList(-1);
}

void SCardInfoDialog::CommitMovCrd()
{
	int    ok = 1;
	const  uint c = OwnerList.getCount();
	if(Mode == modeMovCrd && c) {
		THROW(ScObj.CheckRights(SCRDRT_ADDOPS));
		PPTransaction tra(1);
		THROW(tra);
		for(uint i = 0; i < c; i++) {
			SpcListItem & r_item = OwnerList.at(i);
			if(r_item.SCardID && r_item.Amount > 0.0) {
				SCardCore::OpBlock op;
				op.SCardID = r_item.SCardID;
				op.DestSCardID = SCardID;
				op.Amount = r_item.Amount;
				op.Dtm = getcurdatetime_();
				THROW(ScObj.P_Tbl->PutOpBlk(op, 0));
			}
		}
		THROW(tra.Commit());
		SetupMode(modeOpView, 1);
	}
	CATCH
		PPError();
	ENDCATCH
}

IMPL_HANDLE_EVENT(SCardInfoDialog)
{
	if(TVCOMMAND && TVCMD == cmOK) {
		if(oneof2(Mode, modeSelectByOwner, modeSelectByMultCode)) {
			PPID   sc_id = 0;
			if(getSelection(&sc_id) && sc_id) {
				SetupCard(sc_id);
			}
		}
		else {
			SString code;
			getCtrlString(CTL_SCARDVIEW_INPUT, code);
			if(LocalState & stAsSelector && !code.NotEmptyS() && IsInState(sfModal)) {
				endModal(cmOK);
				return; // ����� endModal �� ������� ���������� � this
			}
			else {
				SCardTbl::Rec sc_rec;
				MEMSZERO(sc_rec);
				if(PPObjSCard::PreprocessSCardCode(code) > 0 && ScObj.SearchCode(0, code, &sc_rec) > 0) {
					PPIDArray mult_list;
					const int mc = ScObj.P_Tbl->GetListByCode(code, &mult_list);
					if(mult_list.getCount() > 1) {
						OwnerList.Clear();
						SString psn_name;
						for(uint i = 0; i < mult_list.getCount(); i++) {
							const PPID sc_id = mult_list.get(i);
							if(ScObj.Search(sc_id, &sc_rec) > 0) {
								psn_name = 0;
								if(sc_rec.PersonID)
									GetPersonName(sc_rec.PersonID, psn_name);
								OwnerList.Add(sc_rec.PersonID, sc_id, sc_rec.SeriesID, psn_name, sc_rec.Code, sc_rec.Expiry);
							}
						}
						if(OwnerList.getCount()) {
							SetupMode(modeSelectByMultCode, 1);
						}
					}
					else if(mult_list.getCount() == 1 && ScObj.Search(mult_list.get(0), &sc_rec) > 0) {
						if(Mode == modeMovCrd) {
							if(sc_rec.ID != SCardID && !OwnerList.lsearch(&sc_rec.ID, 0, CMPF_LONG, offsetof(SpcListItem, SCardID))) {
								if(ScObj.IsCreditSeries(sc_rec.SeriesID) && sc_rec.Rest > 0.0) {
									OwnerList.Add(sc_rec.ID, sc_rec.SeriesID, sc_rec.Code, sc_rec.Rest, sc_rec.Rest);
									SetupMovCrd();
								}
							}
						}
						else
							SetupCard(sc_rec.ID);
					}
				}
				setCtrlString(CTL_SCARDVIEW_INPUT, code = 0);
			}
		}
		clearEvent(event);
	}
	else
		PPListDialog::handleEvent(event);
	if(TVCOMMAND) {
		if(TVCMD == cmClear) {
			SetupCard(0);
			selectCtrl(CTL_SCARDVIEW_INPUT);
		}
		else if(TVCMD == cmEditPerson) {
			const int enbl_psn = BIN(OwnerID && PsnObj.CheckRights(PPR_MOD));
			if(enbl_psn) {
                if(PsnObj.Edit(&OwnerID, 0) > 0) {
                	SetupCard(SCardID);
                }
			}
		}
		else if(TVCMD == cmCreateSCard) {
			// @v9.1.1 const  long  preserve_lc_flags = DS.SetLCfgFlags(DS.GetTLA().Lc.Flags & ~CCFLG_USELARGEDIALOG);
			const  int preserve_slui_flag = SLS.CheckUiFlag(sluifUseLargeDialogs); // @v9.1.1
			int    do_create = 0;
			PPSCardConfig sc_cfg;
			PPObjSCardSeries scs_obj;
			PPSCardSeries scs_rec;
			PPObjPerson::EditBlock peb;
			ScObj.FetchConfig(&sc_cfg);
			if(scs_obj.Fetch(sc_cfg.DefCreditSerID, &scs_rec) > 0) {
				PsnObj.InitEditBlock(NZOR(scs_rec.PersonKindID, NZOR(sc_cfg.PersonKindID, PPPRK_CLIENT)), peb);
				peb.SCardSeriesID = sc_cfg.DefCreditSerID;
				do_create = 1;
			}
			else if(scs_obj.Fetch(sc_cfg.DefSerID, &scs_rec) > 0) {
				PsnObj.InitEditBlock(NZOR(scs_rec.PersonKindID, NZOR(sc_cfg.PersonKindID, PPPRK_CLIENT)), peb);
				peb.SCardSeriesID = sc_cfg.DefSerID;
				do_create = 1;
			}
			if(do_create) {
				peb.ShortDialog = 1;
				PPID   psn_id = 0;
				if(PsnObj.Edit_(&psn_id, peb) == cmOK) {
					SetupCard(peb.RetSCardID);
				}
			}
			selectCtrl(CTL_SCARDVIEW_INPUT);
			// @v9.1.1 DS.SetLCfgFlags(preserve_lc_flags);
			SLS.SetUiFlag(sluifUseLargeDialogs, preserve_slui_flag); // @v9.1.1
		}
		else if(TVCMD == cmSelectByOwner) {
			SString text;
			getCtrlString(CTL_SCARDVIEW_INPUT, text);
			if(text.Strip().Len() > 0/*2*/) {
				OwnerList.Clear();
				PersonTbl::Rec psn_rec;
				SCardTbl::Rec sc_rec;
				PPIDArray psn_list, sc_list;
				PPObjPerson::SrchAnalogPattern sap(text, 0);
				PsnObj.GetListByPattern(&sap, &psn_list);
				for(uint i = 0; i < psn_list.getCount(); i++) {
					const PPID psn_id = psn_list.get(i);
					sc_list.clear();
					if(ScObj.P_Tbl->GetListByPerson(psn_id, 0, &sc_list) > 0) {
						if(PsnObj.Fetch(psn_id, &psn_rec) > 0) {
							for(uint j = 0; j < sc_list.getCount(); j++) {
								const PPID sc_id = sc_list.get(j);
								if(ScObj.Search(sc_id, &sc_rec) > 0) {
									OwnerList.Add(psn_id, sc_id, sc_rec.SeriesID, psn_rec.Name, sc_rec.Code, sc_rec.Expiry);
								}
							}
						}
					}
				}
				if(OwnerList.getCount()) {
					SetupMode(modeSelectByOwner, 1);
				}
			}
			setCtrlString(CTL_SCARDVIEW_INPUT, text = 0);
		}
		else if(TVCMD == cmSCardMovCrd) {
			if(SCardID && LocalState & stCreditCard) {
				if(Mode != modeMovCrd)
					OwnerList.Clear();
				SetupMode(modeMovCrd, 0);
				selectCtrl(CTL_SCARDVIEW_INPUT);
			}
		}
		else if(TVCMD == cmCommit) {
			CommitMovCrd();
			selectCtrl(CTL_SCARDVIEW_INPUT);
		}
		else if(TVCMD == cmCheckOpSwitch) {
			if(SCardID) {
				if(Mode == modeCheckView) {
					SetupMode(modeOpView, 0);
				}
				else if(Mode == modeOpView) {
					SetupMode(modeCheckView, 0);
				}
			}
		}
		else if(TVCMD == cmActivate) {
			if(LocalState & stNeedActivation) {
				SCardTbl::Rec sc_rec;
				if(ScObj.Search(SCardID, &sc_rec) > 0) {
					if(ScObj.ActivateRec(&sc_rec) > 0) {
						if(ScObj.P_Tbl->Update(sc_rec.ID, &sc_rec, 1))
							SetupCard(sc_rec.ID);
						else
							PPError();
					}
				}
			}
		}
		else if(TVCMD == cmCtlColor) {
			TDrawCtrlData * p_dc = (TDrawCtrlData *)TVINFOPTR;
			if(p_dc && getCtrlHandle(CTL_SCARDVIEW_SALDO) == p_dc->H_Ctl) {
				double saldo = getCtrlReal(CTL_SCARDVIEW_SALDO);
				::SetBkMode(p_dc->H_DC, TRANSPARENT);
				::SetTextColor(p_dc->H_DC, GetColorRef(SClrWhite));
				if(Mode == modeMovCrd)
					p_dc->H_Br = (HBRUSH)Ptb.Get(brMovCrdRest);
				else if(!(LocalState & stCreditCard) || saldo > 0.0)
					p_dc->H_Br = (HBRUSH)Ptb.Get(brGreen);
				else
					p_dc->H_Br = (HBRUSH)Ptb.Get(brRed);
	 		}
			else if(p_dc && getCtrlHandle(CTL_SCARDVIEW_SCINFO) == p_dc->H_Ctl) {
				if(LocalState & stWarnCardInfo) {
					::SetBkMode(p_dc->H_DC, TRANSPARENT);
					::SetTextColor(p_dc->H_DC, GetColorRef(SClrWhite));
					p_dc->H_Br = (HBRUSH)Ptb.Get(brRed);
				}
				else if(LocalState & stNeedActivation) {
					::SetBkMode(p_dc->H_DC, TRANSPARENT);
					::SetTextColor(p_dc->H_DC, GetColorRef(SClrBlack));
					p_dc->H_Br = (HBRUSH)Ptb.Get((LocalState & stAutoActivation) ? brOrange : brYellow);
				}
				else
					return;
			}
			else
				return;
		}
		else
			return;
		clearEvent(event);
	}
}

int SLAPI ViewSCardInfo(PPID * pSCardID, int asSelector)
{
	DIALOG_PROC_BODY_P1(SCardInfoDialog, asSelector, pSCardID);
}

int CPosProcessor::Implement_AcceptSCard(const SCardTbl::Rec & rScRec)
{
	//
	// ��������� ������������� �� ������������� ��������� ��������� � �� �����
	//
	// @todo ���������� �������� ��������� ����������� ����� ������� ���� ������� ���������� -
	//  ��������, �� ������� ������ � ���� ���� �������.
	//
	int    ok = 1;
	CSt.SetID(rScRec.ID, rScRec.Code);
	ZDELETE(CSt.P_DisByAmtRule);
	{
		PPObjSCardSeries scs_obj;
		PPSCardSeries scs_rec;
		PPSCardSerPacket scs_pack;
		if(scs_obj.GetPacket(rScRec.SeriesID, &scs_pack) > 0) {
			ZDELETE(CSt.P_Eqb);
			RetailPriceExtractor::ExtQuotBlock temp_eqb(scs_pack);
			if(temp_eqb.QkList.getCount()) {
				CSt.P_Eqb = new RetailPriceExtractor::ExtQuotBlock(scs_pack);
			}
			SETFLAG(CSt.Flags, CardState::fUseDscntIfNQuot, scs_pack.Rec.Flags & SCRDSF_USEDSCNTIFNQUOT || !scs_pack.Rec.QuotKindID_s); // @v8.6.7 (|| !scs_pack.Rec.QuotKindID_s)
			SETFLAG(CSt.Flags, CardState::fUseMinQuotVal,   scs_pack.Rec.Flags & SCRDSF_MINQUOTVAL);
			if(scs_pack.Rec.Flags & SCRDSF_UHTTSYNC) {
				int   uhtt_err = 1;
				PPUhttClient uhtt_cli;
				if(uhtt_cli.Auth()) {
					UhttSCardPacket scp;
					if(uhtt_cli.GetSCardByNumber(rScRec.Code, scp)) {
						CSt.Flags |= CardState::fUhtt;
						scp.Code.CopyTo(CSt.UhttCode, sizeof(CSt.UhttCode));
						scp.Hash.CopyTo(CSt.UhttHash, sizeof(CSt.UhttHash));
						double uhtt_rest = 0.0;
						if(uhtt_cli.GetSCardRest(CSt.UhttCode, 0, uhtt_rest)) {
							CSt.UhttRest = R2(uhtt_rest);
							uhtt_err = 0;
						}
					}
				}
				if(uhtt_err) {
					CSt.Flags &= ~CardState::fUhtt;
					CSt.UhttRest = 0.0;
				}
			}
			if(rScRec.Flags & SCRDF_INHERITED && scs_pack.CcAmtDisRule.getCount()) {
				CSt.P_DisByAmtRule = new PPSCardSerRule;
				ASSIGN_PTR(CSt.P_DisByAmtRule, scs_pack.CcAmtDisRule);
			}
		}
		else {
			ZDELETE(CSt.P_Eqb);
			CSt.Flags &= ~(CardState::fUseDscntIfNQuot|CardState::fUseMinQuotVal);
		}
	}
	SETFLAG(CSt.Flags, CardState::fNoGift, rScRec.Flags & SCRDF_NOGIFT);
	return ok;
}

int CPosProcessor::SetupSCard(PPID scID, const SCardTbl::Rec * pScRec)
{
	int    ok = 1;
	//SString temp_buf;
	SCardTbl::Rec sc_rec;
	if(scID) {
		if(pScRec == 0) {
			THROW(ScObj.Search(scID, &sc_rec) > 0);
			pScRec = &sc_rec;
		}
		assert(pScRec);
		THROW(Implement_AcceptSCard(*pScRec));
		if(pScRec->AutoGoodsID) {
			//double qtty = 1.0;
			//double price = 0.0;
			PgsBlock pgsb(1.0);
			//
			// CheckPaneDialog::PreprocessGoodsSelection() - ������������� �������. �� ���������� ������� �� ������� � ������������� �����.
			// if(PreprocessGoodsSelection(pScRec->AutoGoodsID, 0, &qtty, temp_buf = 0, &price) > 0)
				SetupNewRow(pScRec->AutoGoodsID, /*qtty, price, temp_buf*/pgsb);
		}
		CSt.Discount = fdiv100i(pScRec->PDis);
		Flags |= fPctDis;
		SetupDiscount(); // ����� SetupDiscount ����� CalcRestByCrdCard_ ��������� ��� ����������� ������� ������� �� ��������� (��������) �����
		if(!CalcRestByCrdCard_(0)) {
			ResetSCard();
		}
	}
	else {
		ResetSCard();
	}
	OnUpdateList(0);
	CATCHZOK
	return ok;
}

int CPosProcessor::Backend_AcceptSCard(PPID scardID, int ignoreRights)
{
	int    ok = 1;
	SString temp_buf;
	if(Flags & fPrinted && !(OperRightsFlags & orfChgPrintedCheck) && !ignoreRights) {
		ok = MessageError(PPERR_NORIGHTS, 0, eomBeep | eomStatusLine);
		Flags &= ~fWaitOnSCard;
	}
	else {
		Flags |= fSuspSleepTimeout;
		const  int prev_no_gift_status = BIN(CSt.Flags & CardState::fNoGift);
		//
		// ������� ����, ��� ������ ��������������� ������ �� ������� ������ ���� (1 ������ �� ������, 0 - ������, -1 - ������ �� �����)
		//
		int    row_dscnt = -1;
		CSt.SetID(scardID, 0);
		//
		if(row_dscnt < 0 && !oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
			int    is_found = 0;
			SCardTbl::Rec sc_rec;
			const  PPID sc_id = CSt.GetID();
			//
			// ���� �� � ��������� ���������� �� ��������� �����, �� ������ ����� ��� ������ //
			//
			if(sc_id) {
				const PPID charge_goods_id = GetChargeGoodsID(sc_id);
				if(charge_goods_id != UNDEF_CHARGEGOODSID) {
					for(uint i = 0; ok && i < P.getCount(); i++)
						if(P.at(i).GoodsID == charge_goods_id)
							ok = MessageError(PPERR_UNABLECHNGCHARGESCARD, 0, eomStatusLine);
				}
			}
			if(ok) {
				if(sc_id && ScObj.Search(sc_id, &sc_rec) > 0) {
					const int only_charge_goods = IsOnlyChargeGoodsInPacket(sc_id, 0);
					int    cr = ScObj.CheckRestrictions(&sc_rec, (only_charge_goods ? PPObjSCard::chkrfIgnoreUsageTime : 0), getcurdatetime_());
					if(!cr) {
						ok = MessageError(-1, 0, eomPopup | eomBeep);
						CSt.SetID(0, 0);
					}
					else if(cr == 2) {
						/*
						SString msg_buf;
						PPLoadText(PPTXT_SCARDISAUTOACTIVATED, temp_buf = 0);
						msg_buf.Printf(temp_buf, sc_rec.Code);
						PPTooltipMessage(msg_buf, 0, hWnd, 10000, GetColorRef(SClrOrange),
							SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus|SMessageWindow::fLargeText);
						*/
					}
					else {
						THROW(Implement_AcceptSCard(sc_rec));
						if(sc_rec.AutoGoodsID) {
							PgsBlock pgsb(1.0);
							//double qtty = 1.0;
							//double price = 0.0;
							//temp_buf = 0;
							//if(PreprocessGoodsSelection(sc_rec.AutoGoodsID, 0, &qtty, temp_buf = 0, &price) > 0)
								SetupNewRow(sc_rec.AutoGoodsID, /*qtty, price, temp_buf*/pgsb);
						}
						CSt.Discount = fdiv100i(sc_rec.PDis);
						Flags |= fPctDis;
						SetupDiscount(); // ����� SetupDiscount ����� CalcRestByCrdCard_ ��������� ��� ����������� ������� ������� �� ��������� (��������) �����
						if(CalcRestByCrdCard_(0)) {
							AutosaveCheck(); // @v8.7.11
						}
						else {
							ResetSCard();
							ok = 0;
						}
						OnUpdateList(0);
					}
					Flags &= ~fWaitOnSCard;
				}
				else {
					if(Flags & fWaitOnSCard) {
						ResetSCard();
						OnUpdateList(0);
					}
					INVERSEFLAG(Flags, fWaitOnSCard);
				}
				SetupInfo(0);
				/*
				if(IsState(sLIST_EMPTYBUF))
					enableCommand(cmCash, !(Flags & fSCardCredit) || Flags & fWaitOnSCard); // @scard
				*/
			}
		}
		{
			const  int cur_no_gift_status = BIN(CSt.Flags & CardState::fNoGift);
			if(cur_no_gift_status != prev_no_gift_status)
				ProcessGift();
		}
		Flags &= ~fSuspSleepTimeout;
	}
	CATCHZOK
	return ok;
}

void CheckPaneDialog::AcceptSCard(int fromInput, PPID scardID, int ignoreRights)
{
	assert(oneof3(fromInput, 0, 1, 100));
	int    ok = 1;
	SString temp_buf;
	if(Flags & fPrinted && !(OperRightsFlags & orfChgPrintedCheck) && !ignoreRights) {
		MessageError(PPERR_NORIGHTS, 0, eomBeep | eomStatusLine);
		Flags &= ~fWaitOnSCard;
	}
	else {
		Flags |= fSuspSleepTimeout;
		const  int prev_no_gift_status = BIN(CSt.Flags & CardState::fNoGift);
		//
		// ������� ����, ��� ������ ��������������� ������ �� ������� ������ ���� (1 ������ �� ������, 0 - ������, -1 - ������ �� �����)
		//
		int    row_dscnt = fromInput ? AcceptRowDiscount() : -1;
		if(!fromInput)
			CSt.SetID(scardID, 0);
		//
		if(row_dscnt < 0 && !oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
			int    is_found = 0;
			SCardTbl::Rec sc_rec;
			//
			// ���� �� � ��������� ���������� �� ��������� �����, �� ������ ����� ��� ������ //
			//
			if(CSt.GetID()) {
				const PPID charge_goods_id = GetChargeGoodsID(CSt.GetID());
				if(charge_goods_id != UNDEF_CHARGEGOODSID) {
					for(uint i = 0; ok && i < P.getCount(); i++)
						if(P.at(i).GoodsID == charge_goods_id)
							ok = MessageError(PPERR_UNABLECHNGCHARGESCARD, 0, eomStatusLine);
				}
			}
			if(ok) {
				int    ext_cancel = 0; // ������� ������ ������������ ������ �����
				if(fromInput == 100 && CnExtFlags & CASHFX_EXTSCARDSEL) {
					//
					// ����������� ����� �����
					//
					PPID   scard_id = CSt.GetID();
					if(!scard_id) {
						GetInput();
						temp_buf = Input;
						if(temp_buf.NotEmptyS() && ScObj.SearchCode(0, temp_buf, &sc_rec) > 0)
							scard_id = sc_rec.ID;
					}
					if(ViewSCardInfo(&scard_id, 1) > 0) {
						if(scard_id && ScObj.Search(scard_id, &sc_rec) > 0) {
							CSt.SetID(scard_id, sc_rec.Code);
							is_found = 1;
						}
						else
							Flags |= fWaitOnSCard; // ���� �� ����� ����� ���������� ����� ��������� �����.
					}
					else
						ext_cancel = 1;
				}
				else if(fromInput) {
					//
					// ����� ����� �� ����, ���������� � ������ �����
					//
					GetInput();
					temp_buf = Input;
					if(PPObjSCard::PreprocessSCardCode(temp_buf) > 0) {
						if(UiFlags & uifAutoInput || !(CsObj.GetEqCfg().Flags & PPEquipConfig::fDisableManualSCardInput)) {
							char   card_code[64];
							temp_buf.CopyTo(card_code, sizeof(card_code));
							if(card_code[0])
								if(ScObj.SearchCode(0, card_code, &sc_rec) > 0)
									is_found = 1;
								else
									MessageError(PPERR_SCARDNOTFOUND, card_code, eomStatusLine | eomBeep);
						}
						else
							MessageError(PPERR_MANUALSCARDINPUTDISABLED, 0, eomStatusLine | eomBeep);
					}
				}
				else if(CSt.GetID() && ScObj.Search(CSt.GetID(), &sc_rec) > 0) {
					//
					// ������� ��������� �����
					//
					is_found = 1;
				}
				if(!ext_cancel) { // ���� ����������� ����� ��� �������, �� ��� ���� ������� ����������
					if(is_found) {
						const int only_charge_goods = IsOnlyChargeGoodsInPacket(CSt.GetID(), 0);
						int    cr = ScObj.CheckRestrictions(&sc_rec, (only_charge_goods ? PPObjSCard::chkrfIgnoreUsageTime : 0), getcurdatetime_());
						if(!cr) {
							MessageError(-1, 0, eomPopup | eomBeep);
							CSt.SetID(0, 0);
						}
						else {
							//
							// @v9.1.8 ���������������� ����� ������� � 9.1.8 �� ������ ���� ����� ���� ��� ��, ��� � �������
							//
							{
								Implement_AcceptSCard(sc_rec);
								if(sc_rec.AutoGoodsID) {
									PgsBlock pgsb(1.0);
									//double qtty = 1.0;
									//double price = 0.0;
									if(PreprocessGoodsSelection(sc_rec.AutoGoodsID, 0, pgsb) > 0)
										SetupNewRow(sc_rec.AutoGoodsID, pgsb);
								}
								CSt.Discount = fdiv100i(sc_rec.PDis);
								Flags |= fPctDis;
								SetupDiscount(); // ����� SetupDiscount ����� CalcRestByCrdCard_ ��������� ��� ����������� ������� ������� �� ��������� (��������) �����
								if(CalcRestByCrdCard_(0)) {
									AutosaveCheck(); // @v8.7.11
								}
								else
									ResetSCard();
								OnUpdateList(0);
							}
							if(cr == 2) {
								SString msg_buf;
								PPLoadText(PPTXT_SCARDISAUTOACTIVATED, temp_buf = 0);
								msg_buf.Printf(temp_buf, sc_rec.Code);
								PPTooltipMessage(msg_buf, 0, H(), 10000, GetColorRef(SClrOrange),
									SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus|SMessageWindow::fLargeText);
							}
						}
						Flags &= ~fWaitOnSCard;
					}
					else {
						if(Flags & fWaitOnSCard) {
							ResetSCard();
							OnUpdateList(0);
						}
						INVERSEFLAG(Flags, fWaitOnSCard);
					}
					SetupInfo(0);
					if(IsState(sLIST_EMPTYBUF))
						enableCommand(cmCash, !(Flags & fSCardCredit) || Flags & fWaitOnSCard); // @scard
				}
			}
		}
		ClearInput(0);
		{
			const  int cur_no_gift_status = BIN(CSt.Flags & CardState::fNoGift);
			if(cur_no_gift_status != prev_no_gift_status)
				ProcessGift();
		}
		Flags &= ~fSuspSleepTimeout;
	}
}

//virtual
void CheckPaneDialog::SetupRowData(int calcRest)
{
	const CCheckItem & r_item = P.GetCurC();
	SString   buf, temp_buf;
	double    sum = R2(r_item.Quantity * (r_item.Price - r_item.Discount));
	buf = r_item.GoodsName;
	if(P.CurModifList.getCount()) {
		buf.Space().CatChar('{').CatChar('+');
		for(uint i = 0; i < P.CurModifList.getCount(); i++) {
			GetGoodsName(P.CurModifList.at(i).GoodsID, temp_buf);
			if(i)
				buf.CatDiv(';', 2);
			buf.Cat(temp_buf);
		}
		buf.CatChar('}');
	}
	setCtrlString(CTL_CHKPAN_GOODS, buf);
	(buf = 0).Cat(r_item.Quantity);
	if(r_item.PhQtty)
		buf.CatChar('(').Cat(r_item.PhQtty).CatChar(')');
	setCtrlString(CTL_CHKPAN_QTTY, buf);
	setCtrlReal(CTL_CHKPAN_PRICE, r_item.Price);
	setCtrlReal(CTL_CHKPAN_SUM,   sum);
	CPosProcessor::SetupRowData(calcRest);
	ViewStoragePlaces(0);
}

//int CPosProcessor::SetupNewRow(PPID goodsID, double qtty, double priceBySerial, const char * pSerial, PPID giftID /*=0*/)
int CPosProcessor::SetupNewRow(PPID goodsID, PgsBlock & rBlk, PPID giftID/*=0*/)
{
	int    ok = 1, r = 0;
	SString temp_buf;
	if(Flags & fPrinted && !(OperRightsFlags & orfChgPrintedCheck)) {
		ok = MessageError(PPERR_NORIGHTS, 0, eomBeep|eomStatusLine);
	}
	else {
		const  int gift_money = BIN(giftID && rBlk.PriceBySerial > 0.0);
		if(AcceptRow(giftID)) {
			RetailGoodsInfo rgi;
			long   ext_rgi_flags = 0;
			if(gift_money)
				ext_rgi_flags |= PPObjGoods::rgifAllowUnlimWoQuot;
			else if(rBlk.PriceBySerial > 0.0) {
				ext_rgi_flags |= PPObjGoods::rgifUseOuterPrice;
				rgi.OuterPrice = rBlk.PriceBySerial;
			}
			r = GetRgi(goodsID, rBlk.Qtty, ext_rgi_flags, rgi);
			if(goodsID == GetChargeGoodsID(CSt.GetID())) {
				//
				// ���������� �� ��������� �����
				//
				rgi.ID = goodsID;
				Goods2Tbl::Rec goods_rec;
				if(GObj.Fetch(goodsID, &goods_rec) > 0) {
					STRNSCPY(rgi.Name, goods_rec.Name);
					GObj.FetchSingleBarcode(goodsID, temp_buf);
					temp_buf.CopyTo(rgi.BarCode, sizeof(rgi.BarCode));
				}
				rBlk.Qtty = 0.0; // @v8.7.10 1.0-->0.0
				if(r <= 0)
					rgi.Price = 1.0;
				r = 2000;
			}
			if(r > 0 || (r == -2 && (CnFlags & CASHF_USEQUOT || gift_money))) {
				double price = 0.0;
				int    use_ext_price = 0;
				if(rgi.QuotKindUsedForExtPrice && rgi.ExtPrice >= 0.0 && !(rgi.Flags & rgi.fDisabledExtQuot)) // @v8.7.11 (rgi.ExtPrice > 0.0)-->(rgi.ExtPrice >= 0.0)
					use_ext_price = 1;
				else if(rgi.Flags & rgi.fDisabledQuot) {
					GetGoodsName(goodsID, temp_buf);
					ok = MessageError(PPERR_DISABLEDGOODSQUOT, temp_buf, eomStatusLine|eomBeep);
				}
				if(ok) {
					int    serial_price_tag = 0;
					if(!giftID) {
						if(rBlk.PriceBySerial > 0.0) {
							// @v8.0.12 ������ priceBySerial ��� ������ ��� ������ GetRetailGoodsInfo// price = priceBySerial;
							serial_price_tag = 1;
						}
						if(use_ext_price)
							price = rgi.ExtPrice;
						else
							price = rgi.Price;
					}
					if(price > 0.0 || (price == 0.0 && rgi.QuotKindUsedForPrice) || giftID || r == 2000) {
						CCheckItem & r_item = P.GetCur();
						r_item.GoodsID = rgi.ID;
						STRNSCPY(r_item.GoodsName, rgi.Name);
						STRNSCPY(r_item.BarCode,   rgi.BarCode);
						r_item.Quantity = R6(F(fRetCheck) ? -fabs(rBlk.Qtty) : fabs(rBlk.Qtty));
						r_item.Price    = price;
						r_item.Discount = 0.0;
						SETFLAG(r_item.Flags, cifPriceBySerial, serial_price_tag);
						STRNSCPY(r_item.Serial, rBlk.Serial);
						STRNSCPY(r_item.EgaisMark, rBlk.EgaisMark); // @v9.0.9
						if(giftID) {
							r_item.Flags |= cifGift;
							r_item.GiftID = giftID;
							if(gift_money) {
								r_item.Price = 0.0;
								r_item.Discount = fabs(rBlk.PriceBySerial);
								r_item.Flags |= cifGiftDiscount;
							}
						}
						P.CurPos = P.getCount();
						if(CalcRestByCrdCard_(1)) {
							SetupRowData(1);
						}
						else {
							ClearRow();
							ok = 0;
						}
					}
					else {
						GetGoodsName(goodsID, temp_buf);
						PPSetError(PPERR_INVGOODSPRICE, temp_buf);
						ok = MessageError(PPERR_INVGOODSPRICE, temp_buf, eomStatusLine|eomBeep);
					}
				}
			}
			else if(r == 0)
				ok = MessageError(PPErrCode, 0, eomStatusLine|eomBeep);
		}
		else
			ok = 0;
	}
	return ok;
}

void CheckPaneDialog::ClearInput(int selectOnly)
{
	if(selectOnly) {
		TInputLine * p_il = (TInputLine*)getCtrlView(CTL_CHKPAN_INPUT);
		CALLPTRMEMB(p_il, selectAll(true));
	}
	else {
		setCtrlString(CTL_CHKPAN_INPUT, Input = 0);
		selectCtrl(CTL_CHKPAN_INPUT);
	}
}

int CheckPaneDialog::SetSCard(const char * pStr)
{
	Flags |= fWaitOnSCard;
	SString temp_buf = pStr;
	setCtrlString(CTL_CHKPAN_INPUT, temp_buf);
	UiFlags |= uifAutoInput; // @v7.0.10 @trick AcceptSCard(1, ) ����� ��������� ����� ����� ��-�� ���������� ������� �����.
	AcceptSCard(1, 0);
	UiFlags &= ~uifAutoInput; // @trick (see above)
	return 1;
}

int CheckPaneDialog::GetInput()
{
	UiFlags &= ~uifAutoInput;
	getCtrlString(CTL_CHKPAN_INPUT, Input);
	TInputLine * p_il = (TInputLine *)getCtrlView(CTL_CHKPAN_INPUT);
	if(p_il) {
		TInputLine::Statistics stat;
		p_il->GetStatistics(&stat);
		if(stat.Flags & stat.fSerialized && !(stat.Flags & stat.fPaste)) {
			if(stat.SymbCount && stat.IntervalMean <= (double)AutoInputTolerance)
				UiFlags |= uifAutoInput;
		}
	}
	return Input.NotEmptyS();
}

int CheckPaneDialog::SetInput(const char * pStr)
{
	Input = pStr;
	setCtrlString(CTL_CHKPAN_INPUT, Input);
	ProcessEnter(1);
	return 1;
}

// virtual
int CheckPaneDialog::NotifyGift(PPID giftID, SaGiftArray::Gift * pGift)
{
	SString msg_buf, fmt_buf, goods_name;
	if(giftID > 0) {
		Flags |= fPresent;
		// @v9.2.6 PPGetWord(PPWORD_GIFT, 0, msg_buf);
		PPLoadString("gift", msg_buf); // @v9.2.6
		GetGoodsName(giftID, goods_name);
		setStaticText(CTL_CHKPAN_INFO, msg_buf.Space().Cat(goods_name.Quot(39, 39)));
		if(!(Flags & fDisableBeep))
			Beep(500, 200);
		Flags &= ~fPresent;
	}
	else if(pGift) {
		SMessageWindow::DestroyByParent(H()); // ������� � ������ ���������� ����������� � ��������� ��������
		if(pGift->Pot.Name.NotEmpty()) {
			SMessageWindow * p_win = new SMessageWindow;
			if(p_win) {
				GetGoodsName(pGift->Pot.GoodsID, goods_name);
				// PPTXT_CHKPAN_GIFTNOT    "��� ��������� ������� '%s'\n�������� ������ ����� '%s'";
				// PPTXT_CHKPAN_GIFTNOTAMT "��� ��������� ������� '%s'\n�������� ������ ����� '%s' �� ����� %.2lf";
				PPLoadText((pGift->Pot.Amount > 0.0) ? PPTXT_CHKPAN_GIFTNOTEAMT : PPTXT_CHKPAN_GIFTNOTE, fmt_buf);
				msg_buf.Printf(fmt_buf, pGift->Pot.Name.cptr(), goods_name.cptr(), pGift->Pot.Deficit);
				p_win->Open(msg_buf, 0, H(), 0, 30000, GetColorRef(SClrAquamarine),
					SMessageWindow::fTopmost|SMessageWindow::fSizeByText|SMessageWindow::fPreserveFocus, 0);
			}
		}
	}
	return 1;
}

int CPosProcessor::AddGiftSaleItem(TSArray <SaSaleItem> & rList, const CCheckItem & rItem) const
{
	int    ok = 1;
	SaSaleItem sa_item;
	sa_item.GoodsID = rItem.GoodsID;
	sa_item.Qtty = fabs(rItem.Quantity);
	sa_item.Price = fabs(rItem.Price);
	uint pos = 0;
	if(rList.lsearch(&rItem.GoodsID, &pos, PTR_CMPFUNC(long))) {
		SaSaleItem & r_sa_item = rList.at(pos);
		double qtty = r_sa_item.Qtty + fabs(rItem.Quantity);
		r_sa_item.Price = ((r_sa_item.Price * r_sa_item.Qtty) + (sa_item.Price * sa_item.Qtty)) / qtty;
		r_sa_item.Qtty = qtty;
	}
	else
		rList.insert(&sa_item);
	return ok;
}

int CPosProcessor::ProcessGift()
{
	int    ok = -1;
	uint   i;
	PPIDArray last_item_by_giftq_goods_list; // ������ �������, �� ������� ��� �������������
		// ���������� ������ �� ����� GSGIFTQ_LASTITEMBYGIFTQ
	if(CnFlags & CASHF_CHECKFORPRESENT) {
		if(CSt.GetID() && CSt.Flags & CardState::fNoGift) {
			if(P.ClearGift() > 0)
				OnUpdateList(0);
		}
		else {
			PPID   last_gift_id = 0;
			int    is_gift = 0;
			SaGiftArray::Gift gift;

			SString buf, goods_name;
			TSArray <SaSaleItem> sale_list, full_sale_list;
			LAssocArray ex_gift_list;    // ������ ���������� �������, ��� ����������� � ����
			RAssocArray ex_gift_id_list; // ������ ��������, ��� ����������� � ����
			LAssocArray preserve_gift_assoc = P.GiftAssoc;
			TSCollection <SaGiftArray::Gift> _gift_list; // ������ ������������ ���� �������� - ��������� ��� ��������� ������������
			P.GiftAssoc.clear();
			for(i = 0; i < P.getCount(); i++) {
				CCheckItem & r_item = P.at(i);
				if(r_item.Flags & cifGift) {
					ex_gift_list.Add(r_item.GoodsID, i, 0);
					ex_gift_id_list.Add(r_item.GiftID, fabs(r_item.Quantity), 0);
				}
				else {
					r_item.Flags &= ~(cifUsedByGift|cifMainGiftItem);
					r_item.ResetGiftQuot();
					AddGiftSaleItem(full_sale_list, r_item);
				}
			}
			sale_list = full_sale_list;
			if(sale_list.getCount()) {
				int    overlap = 0; // ������� ��������� ������������� ��������
				PPObjGoodsStruc gs_obj;
				SaGiftArray sa_gift_list;
				THROW(gs_obj.FetchGiftList(&sa_gift_list));
				while(1) {
					{
						//
						// ���� ���������� ������
						//
						int  do_process = 0;
						if(!overlap) {
							if(sa_gift_list.SelectGift(sale_list, ex_gift_id_list, 0 /* overlap */, gift) > 0)
								do_process = 1;
							else {
								overlap = 1;
								SaGiftArray::Potential ppot;
								gift.PreservePotential(ppot);
								if(sa_gift_list.SelectGift(full_sale_list, ex_gift_id_list, 1 /* overlap */, gift) > 0)
									do_process = 1;
								if(gift.Pot.Name.Empty())
									gift.RestorePotential(ppot);
							}
						}
						// @v8.8.0 {
						for(uint gi = 0; do_process && gi < _gift_list.getCount(); gi++) {
							const SaGiftArray::Gift * p_gift = _gift_list.at(gi);
							if(p_gift && p_gift->IsEqualForResult(gift))
								do_process = 0;
						}
						// } @v8.8.0
						if(do_process) {
							SaGiftArray::Gift * p_new_item = new SaGiftArray::Gift(gift);
							_gift_list.insert(p_new_item);
						}
						else
							break;
					}
					PPID   gift_id = 0;
					gift.Qtty = fint(gift.Qtty);
					if(gift.Qtty > 0.0) {
						int    skip = 0;
						int    manual_gift = 0;
						gift_id = gift.List.get(0);
						if(gift_id) {
							is_gift = 1;
							Flags |= fPresent;
							if(gift.QuotKindID) {
								const  uint pcnt = P.getCount();
								int    is_there_gift_quot = 0;
								if(gift.QuotKindID == GSGIFTQ_CHEAPESTITEMFREE) {
									if(pcnt > 0) {
										double min_price = SMathConst::Max;
										uint   min_pos = UINT_MAX;
										for(i = 0; i < pcnt; i++) {
											CCheckItem & r_item = P.at(i);
											if(!(r_item.Flags & cifGift) && gift.CheckList.Has(r_item.GoodsID)) {
												if(r_item.NetPrice() <= min_price) {
													min_price = r_item.NetPrice();
													min_pos = i;
												}
												r_item.Flags |= cifUsedByGift;
											}
										}
										if(min_pos < pcnt) {
											assert(min_price < SMathConst::Max);
											CCheckItem & r_item = P.at(min_pos);
											const double qtty = fabs(r_item.Quantity);
											const double net_price = r_item.NetPrice();
											double gift_price = round((net_price * qtty - net_price) / qtty, 2);
											is_there_gift_quot = r_item.SetupGiftQuot(gift_price, 1);
										}
									}
								}
								if(gift.QuotKindID == GSGIFTQ_CHEAPESTITEMBYGIFTQ) {
									if(pcnt > 0) {
										LongArray _pos_list;
										for(double _rest = gift.Qtty; _rest > 0.0;) {
											double min_price = SMathConst::Max;
											uint   min_pos = UINT_MAX;
											for(i = 0; i < pcnt; i++) {
												CCheckItem & r_item = P.at(i);
												if(!(r_item.Flags & cifGift) && !_pos_list.lsearch(i) && gift.CheckList.Has(r_item.GoodsID)) {
													if(r_item.NetPrice() <= min_price) {
														min_price = r_item.NetPrice();
														min_pos = i;
													}
													r_item.Flags |= cifUsedByGift;
												}
											}
											if(min_pos < pcnt) {
												assert(min_price < SMathConst::Max);
												_pos_list.add(min_pos);
												CCheckItem & r_item = P.at(min_pos);
												const double qtty = fabs(r_item.Quantity);
												const double gift_qtty = MIN(_rest, fint(qtty));
												const double net_price = r_item.NetPrice();
												double qprice = 0.0;
												QuotIdent qi(QIDATE(getcurdate_()), CnLocID, PPQUOTK_GIFT);
												if(GObj.GetQuotExt(r_item.GoodsID, qi, 0.0, r_item.Price, &qprice, 1) > 0) {
													if(qprice < net_price) {
														double gift_price = round((net_price * qtty - (gift_qtty * (net_price - qprice))) / qtty, 2);
														is_there_gift_quot = r_item.SetupGiftQuot(gift_price, 1);
														_rest -= gift_qtty;
													}
												}
											}
											else
												break;
										}
									}
								}
								else if(gift.QuotKindID == GSGIFTQ_LASTITEMBYGIFTQ) {
									const uint clc = gift.CheckList.getCount();
									const PPID last_goods_id = clc ? gift.CheckList.at(clc-1).Key : 0;
									if(last_goods_id && !last_item_by_giftq_goods_list.lsearch(last_goods_id)) {
										int    single_pos = -1;
										int    max_qtty_pos = -1;
										double max_qtty = 0.0;
										i = pcnt;
										if(i) do {
											CCheckItem & r_item = P.at(--i);
											if(!(r_item.Flags & (cifGift|cifUsedByGift)) && r_item.GoodsID == last_goods_id) {
												if(r_item.Quantity == 1.0) {
													// ���� ��������� 1 ����� - �� ��� ���������� ��������� � ������� - �������.
													single_pos = i;
												}
												else if(r_item.Quantity > 1.0 && r_item.Quantity > max_qtty) {
													// ���� ���������� ������ 1, �� ������� - ������������ ����� ���� ������� �� ���
													// � ����� ��������� ��������� ������, �� ������� ����� �������� ���������� ���������.
													max_qtty_pos = i;
													max_qtty = r_item.Quantity;
												}
											}
										} while(i && single_pos < 0);
										if(single_pos < 0 && max_qtty_pos >= 0) {
											//
											// ��������� ������� �� ����� - �������� ��������� ������ � ������������ �����������.
											//
											CCheckItem & r_item = P.at(max_qtty_pos);
											CCheckItem new_item;
											if(r_item.SplitByQtty(1.0, new_item) > 0) {
												single_pos = (int)P.getCount();
												new_item.Flags |= cifUsedByGift;
												P.insert(&new_item);
											}
										}
										if(single_pos >= 0) {
											CCheckItem & r_item = P.at(single_pos);
											double gift_price = 0.0;
											QuotIdent qi(QIDATE(getcurdate_()), CnLocID, PPQUOTK_GIFT);
											if(GObj.GetQuotExt(r_item.GoodsID, qi, 0.0, r_item.Price, &gift_price, 1) > 0)
												is_there_gift_quot = r_item.SetupGiftQuot(gift_price, 1);
											for(i = 0; i < pcnt; i++) {
												CCheckItem & r_item = P.at(i);
												if(!(r_item.Flags & cifGift) && gift.CheckList.Has(r_item.GoodsID))
													r_item.Flags |= cifUsedByGift;
											}
											last_item_by_giftq_goods_list.addUnique(last_goods_id);
										}
									}
								}
								else {
									const  uint clc = gift.CheckList.getCount();
									double total_gift_qtty = gift.Qtty;
									for(uint m = 0; m < clc && total_gift_qtty > 0.0; m++) {
										const RAssoc & r_check_item = gift.CheckList.at(m);
										//double total_gift_qtty = r_check_item.Val * gift.Qtty;
										for(i = 0; i < pcnt && total_gift_qtty > 0.0; i++) {
											CCheckItem & r_item = P.at(i);
											if(!(r_item.Flags & cifGift) && r_item.GoodsID == r_check_item.Key) {
												double qprice = 0.0;
												QuotIdent qi(QIDATE(getcurdate_()), CnLocID, gift.QuotKindID);
												if(GObj.GetQuotExt(r_item.GoodsID, qi, 0.0, r_item.Price, &qprice, 1) > 0) {
													const double qtty = fabs(r_item.Quantity);
													const double gift_qtty = MIN(total_gift_qtty, qtty);
													const double net_price = r_item.NetPrice();
													double gift_price = round((net_price * qtty - (gift_qtty * (net_price - qprice))) / qtty, 2);
													is_there_gift_quot = r_item.SetupGiftQuot(gift_price, 1);
													total_gift_qtty -= gift_qtty;
												}
												r_item.Flags |= cifUsedByGift;
											}
										}
									}
								}
								if(is_there_gift_quot)
									last_gift_id = gift_id;
							}
							else {
								if(GObj.IsGeneric(gift_id) > 0) {
									PPIDArray gen_list;
									GObj.GetGenericList(gift_id, &gen_list);
									i = gen_list.getCount();
									if(i)
										do {
											const PPID goods_id = gen_list.get(--i);
											Goods2Tbl::Rec goods_rec;
											if(GObj.Fetch(goods_id, &goods_rec) <= 0 || goods_rec.Flags & GF_GENERIC)
												gen_list.atFree(i);
										} while(i);
									if(gen_list.getCount()) {
										//
										// ����� ������� ������� ����, ����������� ��� ����������� ����� ��������������� ������� ������
										// ����� �� �������. ���� �� ����� ����� �������, �� �� ����� ������� ������������ ������� ��� �����.
										//
										for(i = 0; !manual_gift && i < ex_gift_list.getCount(); i++) {
											uint ex_pos = (uint)ex_gift_list.at(i).Val;
											CCheckItem & r_item = P.at((uint)ex_gift_list.at(i).Val);
											assert(r_item.Flags & cifGift);
											if(r_item.Flags & cifManualGift && gen_list.lsearch(r_item.GoodsID)) {
												LongArray ex_pos_list;
												preserve_gift_assoc.GetListByKey(ex_pos, ex_pos_list);
												if(ex_pos_list.getCount()) {
													int    is_same = 1;
													for(uint j = 0; is_same && j < ex_pos_list.getCount(); j++) {
														uint _p = ex_pos_list.get(j);
														if(_p < P.getCount()) {
															CCheckItem & r_item2 = P.at(_p);
															uint   gift_pos = 0;
															if((r_item2.Flags & cifGift) || !gift.CheckList.Search(r_item2.GoodsID, 0, &gift_pos))
																is_same = 0;
														}
													}
													if(is_same) {
														gift_id = r_item.GoodsID;
														manual_gift = 1;
													}
												}
											}
										}
										//
										if(!manual_gift) {
											ExtGoodsSelDialog * dlg = new ExtGoodsSelDialog(GetCashOp(), 0, (CnFlags & CASHF_SELALLGOODS) ? ExtGoodsSelDialog::fForceExhausted : 0);
											THROW(CheckDialogPtr(&dlg, 1));
											dlg->setSelectionByGoodsList(&gen_list);
											if(ExecView(dlg) == cmOK) {
												TIDlgInitData tidi;
												dlg->getDTS(&tidi);
												if(tidi.GoodsID) {
													gift_id = tidi.GoodsID;
													manual_gift = 1;
												}
												else
													skip = 1;
											}
											else
												skip = 1;
											delete dlg;
										}
									}
								}
								if(!skip) {
									int    gift_accepted = 0;
									double gift_discount = 0.0;
									long   lpos = 0; // ������ ���������� ������� � ���������� P
									uint   egl_pos = 0;
									if(ex_gift_list.Search(gift_id, &lpos, &egl_pos)) {
										CCheckItem & r_item = P.at((uint)lpos);
										assert(r_item.Flags & cifGift);
										r_item.Quantity = gift.Qtty;
										ex_gift_list.atFree(egl_pos);
										gift_accepted = 1;
									}
									else {
										double gift_quot = 0.0;
										QuotIdent qi(QIDATE(getcurdate_()), CnLocID, PPQUOTK_GIFT);
										if(GObj.GetQuotExt(gift_id, qi, 0.0, 0.0, &gift_quot, 1) > 0)
											gift_discount = -gift_quot;
										PgsBlock pgsb(gift.Qtty);
										pgsb.PriceBySerial = gift_discount;
										if(SetupNewRow(gift_id, /*gift.Qtty, gift_discount, 0*/pgsb, 1 /*gift*/)) {
											if(AcceptRow(1 /*gift*/)) {
												assert(P.getCount());
												lpos = P.getCount()-1;
												if(manual_gift)
													P.at(lpos).Flags |= cifManualGift;
												last_gift_id = gift_id;
												gift_accepted = 1;
											}
										}
									}
									if(gift_accepted) {
										assert(lpos < (long)P.getCount());
										for(i = 0; i < P.getCount(); i++) {
											CCheckItem & r_item = P.at(i);
											uint   gift_pos = 0;
											if(!(r_item.Flags & (cifGift|cifUsedByGift)) && gift.CheckList.Search(r_item.GoodsID, 0, &gift_pos)) {
												r_item.Flags |= cifUsedByGift;
												if(gift.MainPosList.lsearch((long)gift_pos))
													r_item.Flags |= cifMainGiftItem;
												P.GiftAssoc.Add(lpos, (long)i, 0);
											}
										}
									}
								}
							}
							if(!skip)
								ok = 1;
							Flags &= ~fPresent;
							//
							// ��������� � ��������� ��������, ���� ������ ��������� ��������
							//
							sale_list.clear();
							for(i = 0; i < P.getCount(); i++)
								if(!(P.at(i).Flags & (cifGift | cifUsedByGift)))
									AddGiftSaleItem(sale_list, P.at(i));
						}
						else
							break;
					}
					else
						break;
				}
			}
			if(ex_gift_list.getCount()) {
				//
				// ������� ���� ������� � ����� ������ ������� - �����.
				//
				LongArray pos_list;
				for(i = 0; i < ex_gift_list.getCount(); i++)
					pos_list.add(ex_gift_list.at(i).Val);
				pos_list.sort();
				i = pos_list.getCount();
				if(i) do {
					const uint pos = pos_list.get(--i);
					assert(P.at(pos).Flags & cifGift);
					P.atFree((uint)pos);
				} while(i);
			}
			OnUpdateList(0);
			NotifyGift(0, is_gift ? 0 : &gift);
			if(last_gift_id)
				NotifyGift(last_gift_id, 0);
		}
	}
	CATCHZOK
	return ok;
}

int CPosProcessor::AcceptRow(PPID giftID)
{
	int    ok = 1;
	Flags |= fSuspSleepTimeout;
	if(P.CurPos == (int)P.getCount()) {
		RAssocArray sl_ary = SelLines;
		if(Flags & fForceDivision && P.GetCur().Division == 0) {
			ok = MessageError(PPERR_POSDIVISIONNEEDED, 0, eomBeep|eomStatusLine);
		}
		else if(oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF)) {
			double   qtty = fabs(P.GetCur().Quantity);
			CCheckLineTbl::Rec line;
			for(uint i = 0; qtty > 0.0 && SelPack.EnumLines(&i, &line);)
				if(line.GoodsID == P.GetCur().GoodsID) {
					double  sel_qtty = 0.0, rest_qtty = line.Quantity;
					if(sl_ary.Search(i, &sel_qtty, 0))
						rest_qtty -= sel_qtty;
					if(rest_qtty) {
						sl_ary.Add(i, MIN(qtty, rest_qtty));
						qtty -= rest_qtty;
					}
				}
			if(qtty > 0.0)
				ok = (PPError(PPERR_TIMAXQTTY), 0);
		}
		if(ok && (ok = CalcRestByCrdCard_(1)) != 0) { // @01
			const CCheckItem & r_cur = P.GetCurC();
			P.insert(&r_cur);
			SString temp_buf;
			for(uint i = 0; i < P.CurModifList.getCount(); i++) {
				const SaModifEntry & r_entry = P.CurModifList.at(i);
				CCheckItem item;
				item.GoodsID = r_entry.GoodsID;
				item.Quantity = (r_entry.Qtty != 0.0) ? fabs(r_entry.Qtty * r_cur.Quantity) : 1.0;
				item.Price = r_entry.Price;
				item.Division = r_cur.Division;
				item.Queue = r_cur.Queue;
				item.Flags |= cifModifier;
				GetGoodsName(item.GoodsID, temp_buf);
				temp_buf.CopyTo(item.GoodsName, sizeof(item.GoodsName));
				P.insert(&item);
			}
			if(!SuspCheckID && !P.Eccd.InitDtm)
				P.Eccd.InitDtm = getcurdatetime_();
			if(!oneof2(GetState(), sLISTSEL_EMPTYBUF, sLISTSEL_BUF))
				SetupDiscount();
			CalcRestByCrdCard_(0); // @v7.4.9 ���������� ����� (@01) �� ���� ������ ��� ������� ����� �������
			AutosaveCheck(); // @v8.7.7
			OnUpdateList(1);
			SelLines = sl_ary;
		}
	}
	// ��� ������� ������� �� ����������� //
	else if(P.CurPos < (int)P.getCount() && P.HasCur()) {
		assert(0); // @v8.7.7
		P.at(P.CurPos) = P.GetCurC();
		SetupDiscount();
		OnUpdateList(0);
	}
	if(ok) {
		CDispCommand(giftID ? cdispcmdCurrentGiftItem : cdispcmdCurrentItem, 0, 0.0, 0.0);
		ClearRow();
		if(!F(fRetCheck) && !giftID) {
			if(ProcessGift() > 0) {
				CalcRestByCrdCard_(0); // @v7.5.12 ���������� ����� (@01) �� ���� ���������� ������
			}
		}
	}
	Flags &= ~fSuspSleepTimeout;
	return ok;
}

//virtual
int CheckPaneDialog::ClearRow()
{
	CPosProcessor::ClearRow();
	ViewStoragePlaces(0);
	return 1;
}

// virtual
void CheckPaneDialog::SetPrintedFlag(int set)
{
	CPosProcessor::SetPrintedFlag(set);
	setButtonBitmap(cmChkPanPrint, (Flags & fPrinted) ? IDB_PRINTDEFMARK : IDB_PRINTDEF);
}

//virtual
int CheckPaneDialog::ClearCheck()
{
	CPosProcessor::ClearCheck();
	ClearRow();
	setupRetCheck(0);
	return 1;
}

int CheckPaneDialog::LoadCheck(const CCheckPacket * pPack, int makeRetCheck, int notShow)
{
	if(pPack) {
		Goods2Tbl::Rec goods_rec;
		SString temp_buf;
		CCheckItem cc_item;
		for(uint i = 0; pPack->EnumLines(&i, &cc_item);) {
			cc_item.Quantity  = makeRetCheck ? -fabs(cc_item.Quantity) : cc_item.Quantity;
			cc_item.Flags    |= cc_item.Quantity ? 0 : cifGift;
			if(GObj.Fetch(cc_item.GoodsID, &goods_rec) > 0) {
				STRNSCPY(cc_item.GoodsName, goods_rec.Name);
				GObj.FetchSingleBarcode(cc_item.GoodsID, temp_buf = 0); // @v8.0.10 GetSingleBarcode-->FetchSingleBarcode
				temp_buf.CopyTo(cc_item.BarCode, sizeof(cc_item.BarCode));
			}
			if(!P.insert(&cc_item)) {
				MessageError(PPERR_SLIB, 0, eomMsgWindow);
				break;
			}
		}
		if(!notShow) {
			if(P_ChkPack) {
				setStaticText(CTL_CHKPAN_CHKID,   (temp_buf = 0).Cat(P_ChkPack->Rec.ID));
				setStaticText(CTL_CHKPAN_CHKDTTM, (temp_buf = 0).Cat(P_ChkPack->Rec.Dt).Space().Cat(P_ChkPack->Rec.Tm));
				setStaticText(CTL_CHKPAN_CHKNUM,  (temp_buf = 0).Cat(P_ChkPack->Rec.Code));
				setStaticText(CTL_CHKPAN_CASHNUM, (temp_buf = 0).Cat(P_ChkPack->Rec.CashID));
				setStaticText(CTL_CHKPAN_INITDTM, (temp_buf = 0).Cat(P_ChkPack->Ext.CreationDtm, DATF_DMY|DATF_NOZERO, TIMF_HMS|TIMF_NOZERO));
				temp_buf = 0;
				const long f = P_ChkPack->Rec.Flags;
				CatCharByFlag(f, CCHKF_NOTUSED,   'G', temp_buf, 1);
				CatCharByFlag(f, CCHKF_PRINTED,   'P', temp_buf, 0);
				CatCharByFlag(f, CCHKF_RETURN,    'R', temp_buf, 0);
				CatCharByFlag(f, CCHKF_ZCHECK,    'Z', temp_buf, 0);
				CatCharByFlag(f, CCHKF_TRANSMIT,  'T', temp_buf, 0);
				CatCharByFlag(f, CCHKF_BANKING,   'B', temp_buf, 0);
				CatCharByFlag(f, CCHKF_INCORPCRD, 'I', temp_buf, 0);
				CatCharByFlag(f, CCHKF_SUSPENDED, 'S', temp_buf, 0);
				CatCharByFlag(f, CCHKF_JUNK,      'J', temp_buf, 0);
				setStaticText(CTL_CHKPAN_FLAGS, temp_buf);
				if(P_ChkPack->Rec.SCardID) {
					SCardTbl::Rec sc_rec;
					if(ScObj.Search(P_ChkPack->Rec.SCardID, &sc_rec) > 0) {
						(temp_buf = 0).Cat(sc_rec.Code);
						setStaticText(CTL_CHKPAN_SCARDCODE, temp_buf);
					}
				}
			}
			OnUpdateList(0);
			ClearRow();
		}
	}
	return 1;
}

int CheckPaneDialog::TestCheck(CheckPaymMethod paymMethod)
{
	int    ok = 1;
	Packet preserve_packet = P;
	if(CashNodeID) {
		int    r = 1, sync_prn_err = 0;
		int    r_ext = 1, ext_sync_prn_err = 0, is_pack = 0, is_ext_pack = 0;
		CCheckTbl::Rec last_chk_rec;
		CCheckPacket pack, ext_pack;
		THROW(InitCashMachine());
		if(paymMethod == cpmBank) {
			pack.Rec.Flags |= CCHKF_BANKING;
		}
		if(SuspCheckID && CC.Search(SuspCheckID, &last_chk_rec) > 0) {
			pack.Rec.Code = last_chk_rec.Code;
			//
			// ��������� 2 ������ ������� �����������������, ���� ���� � ����� ����������� ���� ������
			// ��������������� ���������� ��������� (� ��������� ������ ��� ����� ��������������� ������� �������� ����).
			//
			//pack.Rec.Dt = last_chk_rec.Dt;
			//pack.Rec.Tm = last_chk_rec.Tm;
		}
		else
			GetNewCheckCode(CashNodeID, &pack.Rec.Code);
		pack.Rec.SessID = P_CM->GetCurSessID();
		pack.Rec.CashID = CashNodeID;
		pack.Rec.Flags |= (CCHKF_SYNC | CCHKF_NOTUSED);
		SETFLAG(pack.Rec.Flags, CCHKF_INCORPCRD, CSt.GetID() && Flags & fSCardCredit);
		// @v8.0.2 SETFLAG(pack.Rec.Flags, CCHKF_BONUSCARD, CSt.GetID() && Flags & fSCardBonus);
		pack.Rec.Flags &= ~CCHKF_BONUSCARD; // @v8.0.2
		SETFLAG(pack.Rec.Flags, CCHKF_SUSPENDED | CCHKF_SKIP, 0);
		SETFLAG(pack.Rec.Flags, CCHKF_PREPRINT, Flags & fPrinted);
		{
			//
			// ����� ������������� ����������� ���� ���������� ������������ ���������� ������ (���� ��� ����)
			// �� ������� ����.
			//
			CCheckItem * p_item;
			for(uint i = 0; P.enumItems(&i, (void**)&p_item);)
				if(p_item->Flags & cifGiftDiscount) {
					SetupDiscount(1);
					break;
				}
		}
		THROW(Helper_InitCcPacket(&pack, &ext_pack, 0, 0));
		is_pack = BIN(pack.GetCount());
		if(P_CM_EXT) {
			pack._Cash = MONEYTOLDBL(pack.Rec.Amount);
			is_ext_pack = BIN(ext_pack.GetCount());
			if(is_ext_pack) {
				double amt, dscnt;
				GetNewCheckCode(ExtCashNodeID, &ext_pack.Rec.Code);
				ext_pack.Rec.SessID = P_CM_EXT->GetCurSessID();
				ext_pack.Rec.CashID = ExtCashNodeID;
				ext_pack.Rec.Flags  = pack.Rec.Flags;
				THROW(ext_pack.SetupAmount(&amt, &dscnt));
				ext_pack.Rec.SCardID = CSt.GetID();
				P.SetupCCheckPacket(&ext_pack);
				ext_pack._Cash = amt;
			}
		}
		else {
			/* @v9.0.4
			pack.Ext.AddPaym = 0; // @v8.0.2 dbltointmny(CSt.AdditionalPayment);
			SETFLAG(pack.Rec.Flags, CCHKF_ADDPAYM, pack.Ext.AddPaym);
			if(CSt.AddCrdCardID) {
				pack.Ext.AddCrdCardID = CSt.AddCrdCardID;
				pack.Ext.AddCrdCardPaym = dbltointmny(CSt.AddCrdCardPayment);
			}
			SETFLAG(pack.Rec.Flags, CCHKF_ADDINCORPCRD, CSt.AddCrdCardID);
			*/
			pack._Cash = 0.0;
		}
		if(is_pack) {
			pack.Rec.SessID = P_CM->GetCurSessID();
			THROW(P_CM->TestPrintCheck(&pack));
		}
		if(is_ext_pack) {
			ext_pack.Rec.SessID = P_CM_EXT->GetCurSessID();
			THROW(P_CM_EXT->TestPrintCheck(&ext_pack));
		}
	}
	CATCHZOKPPERR
	P = preserve_packet;
	return ok;
}

//virtual
int CheckPaneDialog::Implement_AcceptCheckOnEquipment(const CcAmountList * pPl, AcceptCheckProcessBlock & rB)
{
	int    ok = 1;
	if(/*paymMethod == cpmCash*/pPl && pPl->Get(CCAMTTYP_CASH) != 0.0) {
		if(P_CM->GetNodeData().Flags & CASHF_OPENBOX)
			P_CM->SyncOpenBox();
	}
	if(rB.IsPack) {
		THROW(rB.R = P_CM->SyncCheckForSessionOver());
		if(rB.R > 0) {
			rB.Pack.Rec.SessID = P_CM->GetCurSessID();
			if((rB.R = P_CM->SyncPrintCheck(&rB.Pack, 1)) == 0)
				rB.SyncPrnErr = P_CM->SyncGetPrintErrCode();
		}
	}
	THROW(rB.R > 0 || rB.SyncPrnErr == 1);
	if(rB.IsPack) {
		rB.Pack.Rec.Flags |= CCHKF_PRINTED;
		CC.WriteCCheckLogFile(&rB.Pack, 0, CCheckCore::logPrinted, 1);
	}
	if(rB.IsExtPack) {
		THROW(rB.RExt = P_CM_EXT->SyncCheckForSessionOver());
		if(rB.RExt > 0) {
			rB.ExtPack.Rec.SessID = P_CM_EXT->GetCurSessID();
			if((rB.RExt = P_CM_EXT->SyncPrintCheck(&rB.ExtPack, 1)) == 0)
				rB.ExtSyncPrnErr = P_CM_EXT->SyncGetPrintErrCode();
		}
		THROW(rB.RExt > 0 || rB.ExtSyncPrnErr == 1);
		rB.ExtPack.Rec.Flags |= CCHKF_PRINTED;
		CC.WriteCCheckLogFile(&rB.ExtPack, 0, CCheckCore::logPrinted, 1);
	}
	CATCHZOK
	return ok;
}

int CheckPaneDialog::GetLastCheckPacket(PPID nodeID, PPID sessID, CCheckPacket * pPack)
{
	int    ok = -1;
	CCheckTbl::Rec chk_rec;
	MEMSZERO(chk_rec);
	if(pPack && CC.GetLastCheck(sessID, nodeID, &chk_rec) > 0 && CC.LoadPacket(chk_rec.ID, 0, pPack) > 0) {
		if(CnFlags & CASHF_UNIFYGDSATCHECK)
			pPack->MergeLines(0);
		ok = 1;
	}
	return ok;
}

int CheckPaneDialog::PrintCheckCopy()
{
	int     ok = -1;
	PPID    chk_id = 0;
	SString format_name = "CCheckCopy";
	THROW_PP(OperRightsFlags & orfCopyCheck, PPERR_NORIGHTS);
	if(IsState(sEMPTYLIST_EMPTYBUF) && SelectCheck(&chk_id, &format_name, scfThisNodeOnly|scfAllowReturns /*0, 1*/ /* thisNodeOnly */) > 0) {
		CCheckPacket   pack, ext_pack;
		THROW(InitCashMachine());
		if(chk_id) {
			int  r = -1;
			THROW(r = CC.LoadPacket(chk_id, 0, &pack));
			if(r > 0) {
				PPCashNode     cn_rec;
				if(P_CM_EXT && pack.Rec.CashID && CnObj.Fetch(pack.Rec.CashID, &cn_rec) > 0 && cn_rec.LocID == ExtCnLocID) {
					THROW(P_CM_EXT->SyncPrintCheckCopy(&pack, format_name));
				}
				else {
					THROW(P_CM->SyncPrintCheckCopy(&pack, format_name));
				}
				ok = 1;
			}
		}
		else {
			int r1 = GetLastCheckPacket(P_CM->GetNodeData().ID, P_CM->GetCurSessID(), &pack);
			int r2 = P_CM_EXT ? GetLastCheckPacket(P_CM_EXT->GetNodeData().ID, P_CM_EXT->GetCurSessID(), &ext_pack) : 1;
			if(r1 > 0 && r2 > 0) {
				LDATETIME  pack_dttm, ext_pack_dttm;
				pack_dttm.Set(pack.Rec.Dt, pack.Rec.Tm);
				ext_pack_dttm.Set(ext_pack.Rec.Dt, ext_pack.Rec.Tm);
				if(cmp(pack_dttm, ext_pack_dttm) >= 0)
					THROW(P_CM->SyncPrintCheckCopy(&pack, format_name));
				if(cmp(pack_dttm, ext_pack_dttm) <= 0)
					THROW(P_CM_EXT->SyncPrintCheckCopy(&ext_pack, format_name));
				ok = 1;
			}
		}
	}
	CATCHZOKPPERR
	return ok;
}

int CheckPaneDialog::PrintSlipDocument()
{
	int     ok = -1;
	SString format_name;
	if(oneof2(GetState(), sEMPTYLIST_EMPTYBUF, sLIST_EMPTYBUF) && CashNodeID) {
		PPID    chk_id = 0;
		if(IsState(sLIST_EMPTYBUF)) {
			StrAssocArray fmt_list;
			THROW(InitCashMachine());
			if(P_CM->GetSlipFormatList(&fmt_list, 1) > 0) {
				if(fmt_list.getCount() == 1) {
					format_name = fmt_list.at(0).Txt;
				}
				//
				// @todo ��������, ���������� ������� ������, ���� fmt_list.getCount() == 0
				//
				//
			}
		}
		if(format_name.NotEmpty() || SelectCheck(&chk_id, &format_name, scfSelSlipDocForm|scfThisNodeOnly|scfAllowReturns/*1, 1*/ /* thisNodeOnly */) > 0) {
			int   r = -1;
			CCheckPacket  pack;
			THROW(InitCashMachine());
			if(IsState(sLIST_EMPTYBUF)) {
				CCheckTbl::Rec  last_chk_rec;
				if(SuspCheckID && CC.Search(SuspCheckID, &last_chk_rec) > 0)
					pack.Rec.Code = last_chk_rec.Code;
				else
					GetNewCheckCode(CashNodeID, &pack.Rec.Code);
				THROW(Helper_InitCcPacket(&pack, 0, 0, iccpSetCurTime));
				r = 1; // @v7.2.10 (��-�� ����, ��� �������� ���������� ������ ���� �������� �� ���������)
			}
			else {
				if(chk_id)
					THROW(r = CC.LoadPacket(chk_id, 0, &pack));
				if(r < 0)
					r = GetLastCheckPacket(P_CM->GetNodeData().ID, P_CM->GetCurSessID(), &pack);
			}
			if(r > 0)
				THROW(ok = P_CM->SyncPrintSlipDocument(&pack, format_name));
		}
	}
	CATCHZOKPPERR
	return ok;
}

int CPosProcessor::Print(int noAsk, const PPLocPrinter2 * pLocPrn, uint rptId)
{
	int    ok = 1;
	int    is_print_dvc_setted = 0;
	if(!pLocPrn && Flags & fPrinted && !(Flags & fNoEdit) && !(OperRightsFlags & orfChgPrintedCheck)) {
		ok = MessageError(PPERR_NORIGHTS, 0, eomBeep | eomStatusLine);
	}
	else {
		uint   rpt_id = rptId ? rptId : (pLocPrn ? REPORT_CCHECKDETAILVIEWLOC : REPORT_CCHECKDETAILVIEW);
		SString loc_prn_port = pLocPrn ? pLocPrn->Port : 0;
		loc_prn_port.Strip();
		PView  pv(this);
		CCheckItemArray saved_items(P);
		PPReportEnv env;
		env.ContextSymb = CnSymb; // @v8.8.3
		env.PrnFlags = noAsk ? SReport::PrintingNoAsk : 0;
		if(CnFlags & CASHF_UNIFYGDSTOPRINT) {
			//
			// @v8.6.9 ��������� ��������� ������� ����������� �������, ������� ������������
			//
			CCheckItem * p_item = 0;
			CCheckItemArray to_print_items;
			for(uint i = 0; P.enumItems(&i, (void**)&p_item);) {
				int    has_modifier = BIN(i < P.getCount() && P.at(i).Flags & cifModifier);
				int    _found = 0;
				if(!has_modifier) {
					for(uint p = 0; !_found && to_print_items.lsearch(&p_item->GoodsID, &p, CMPF_LONG); p++) {
						CCheckItem & r_ci = to_print_items.at(p);
						if(!(r_ci.Flags & cifHasModifier)) {
							r_ci.Quantity += p_item->Quantity;
							_found = 1;
						}
					}
				}
				if(!_found) {
					to_print_items.insert(p_item);
					CCheckItem & r_ci = to_print_items.at(to_print_items.getCount()-1);
					SETFLAG(r_ci.Flags, cifHasModifier, has_modifier);
				}
			}
			P = to_print_items;
		}
		if(pLocPrn) {
			if(loc_prn_port.NotEmpty()) {
				DS.GetTLA().PrintDevice = loc_prn_port;
				is_print_dvc_setted = 1;
			}
		}
		// @v8.8.3 {
		else if(RptPrnPort.NotEmpty()) {
			DS.GetTLA().PrintDevice = RptPrnPort;
			is_print_dvc_setted = 1;
		}
		// } @v8.8.3
		PPAlddPrint(rpt_id, &pv, &env);
		P = saved_items;
		if(pLocPrn) {
			if(pLocPrn->Flags & PPLocPrinter::fHasKitchenBell && KitchenBellCmd.NotEmpty()) {
				SString kitchen_bell_port = KitchenBellPort;
				if(kitchen_bell_port.Empty())
					kitchen_bell_port = loc_prn_port;
				if(kitchen_bell_port.NotEmptyS()) {
					size_t out_size = 0;
					char   out_buf[64];
					const char * p = KitchenBellCmd;
					while(p[0] && p[1]) {
						uint8 byte = (hex(p[0]) << 4) | hex(p[1]);
						out_buf[out_size++] = byte;
						p += 2;
					}
					SString file_name;
					FILE * f = fopen(loc_prn_port, "w");
					if(f) {
						fwrite(out_buf, out_size, 1, f);
						SFile::ZClose(&f);
					}
				}
			}
		}
		else if(!(Flags & fNoEdit))
			SetPrintedFlag(1);
	}
	if(is_print_dvc_setted)
		DS.GetTLA().PrintDevice = 0;
	return ok;
}
//
// ARG(event IN):
//   0 - ������ ������� ���������� �� ������� rPrnName
//   1 - ��� ����� �� ��������� �������
//   2 - �� ������� ���������� ������ �� ������� rPrnName
//
int CPosProcessor::MakeDbgPrintLogList(int event, const SString & rFmtBuf, const SString & rChkBuf, const SString & rPrnName, SStrCollection & rList)
{
	int    ok = -1;
	if((CConfig.Flags & CCFLG_DEBUG) && oneof3(event, 0, 1, 2)) {
		CCheckItem * p_item = 0;
		SString msg_buf;
		for(uint j = 0; P.enumItems(&j, (void **)&p_item);) {
			if(event == 0)
				PPFormat(rFmtBuf, &msg_buf, rChkBuf.cptr(), rPrnName.cptr(), p_item->GoodsID, p_item->Quantity);
			else if(event == 1)
				PPFormat(rFmtBuf, &msg_buf, rChkBuf.cptr(), p_item->GoodsID, p_item->Quantity);
			else if(event == 2)
				PPFormat(rFmtBuf, &msg_buf, rChkBuf.cptr(), rPrnName.cptr(), p_item->GoodsID, p_item->Quantity);
			rList.insert(newStr(msg_buf));
			ok = 1;
		}
	}
	return ok;
}

int CPosProcessor::PrintToLocalPrinters(int selPrnType)
{
	int    ok = -1;
	PPID   cur_chk_id = CheckID;
	if(oneof2(GetState(), sEMPTYLIST_EMPTYBUF, sLIST_EMPTYBUF)) {
		if(P.getCount() > 0) {
			int    to_local_prn = -1;
			THROW_PP(!(CnFlags & CASHF_DISABLEZEROAGENT) || P.GetAgentID(), PPERR_CHKPAN_SALERNEEDED);
			if(selPrnType && (Flags & fLocPrinters)) {
				if(selPrnType > 0) {
					TDialog * dlg = 0;
					THROW(CheckDialogPtr(&(dlg = new TDialog(DLG_SELLOCPRN))));
					dlg->setCtrlUInt16(CTL_SELLOCPRN_PRNTYPE, 0);
					if(ExecView(dlg) == cmOK) {
						ushort v = dlg->getCtrlUInt16(CTL_SELLOCPRN_PRNTYPE);
						if(v == 0)
							to_local_prn = 1;
						else if(v == 1)
							to_local_prn = 0;
					}
					delete dlg;
				}
				else
					to_local_prn = 1;
			}
			else
				to_local_prn = 0;
			if(to_local_prn > 0) {
				const int do_debug_log = BIN(CConfig.Flags & CCFLG_DEBUG);
				SStrCollection debug_rep_list;
				SString msg_buf, chk_code, prn_name;
				SString buf_ap, buf_prn, buf_ulp, buf_errprn;
				if(do_debug_log) {
					PPLoadText(PPTXT_LOG_CHKPAN_ALLREADYPRN, buf_ap);
					PPLoadText(PPTXT_LOG_CHKPAN_PRINTING, buf_prn);
					PPLoadText(PPTXT_LOG_CHKPAN_UNDEFPRN, buf_ulp);
					PPLoadText(PPTXT_LOG_CHKPAN_ERRPRINTING, buf_errprn);
					CCheckPacket cc_pack;
					GetCheckInfo(&cc_pack);
					CCheckCore::MakeCodeString(&cc_pack.Rec, chk_code);
				}
				PPObjLocPrinter lp_obj;
				PPLocPrinter loc_prn_rec;
				CCheckItem * p_item = 0;
				DS.GetTLA().PrintDevice = 0;
				CCheckItemArray saved_check(P);
				uint   i, j, first_loc_assoc_pos = 0;
				PPID   loc_id;
				LAssocArray pos_assoc_list;
				PPIDArray printed_pos_list;
				//
				// ������������� gtoa �� ����������, ������������� � �������� ����
				//
				InitCashMachine();
				GoodsToObjAssoc gtoa(NZOR(P_CM->GetNodeData().GoodsLocAssocID, PPASS_GOODS2LOC), PPOBJ_LOCATION);
				THROW(gtoa.IsValid());
				THROW(gtoa.Load());
				for(i = 0; saved_check.enumItems(&i, (void **)&p_item);) {
					if(!(p_item->Flags & cifIsPrinted)) {
						gtoa.Get(p_item->GoodsID, &(loc_id = 0));
						pos_assoc_list.Add(loc_id, i-1, 0);
					}
					else if(do_debug_log) {
						PPFormat(buf_ap, &msg_buf, chk_code.cptr(), p_item->GoodsID, p_item->Quantity);
						debug_rep_list.insert(newStr(msg_buf));
					}
				}
				pos_assoc_list.sort(PTR_CMPFUNC(_2long));
				P.freeAll();
				loc_id = 0;
				for(i = 0; i < pos_assoc_list.getCount(); i++) {
					const LAssoc & r_assoc = pos_assoc_list.at(i);
					if(r_assoc.Key != loc_id) {
						if(P.getCount()) {
							if(lp_obj.GetPrinterByLocation(loc_id, prn_name, &loc_prn_rec) > 0) {
								if(Print(1, &loc_prn_rec, 0) > 0) {
									for(j = first_loc_assoc_pos; j < i; j++)
										printed_pos_list.addUnique(pos_assoc_list.at(j).Val);
									MakeDbgPrintLogList(0, buf_prn, chk_code, prn_name, debug_rep_list);
								}
								else
									MakeDbgPrintLogList(2, buf_errprn, chk_code, prn_name, debug_rep_list);
							}
							else
								MakeDbgPrintLogList(1, buf_ulp, chk_code, prn_name, debug_rep_list);
						}
						P.freeAll();
						first_loc_assoc_pos = i;
						loc_id = r_assoc.Key;
					}
					P.insert(&saved_check.at(r_assoc.Val));
				}
				if(P.getCount())
					if(lp_obj.GetPrinterByLocation(loc_id, prn_name, &loc_prn_rec) > 0) {
						if(Print(1, &loc_prn_rec, 0) > 0) {
							for(j = first_loc_assoc_pos; j < i; j++)
								printed_pos_list.addUnique(pos_assoc_list.at(j).Val);
							MakeDbgPrintLogList(0, buf_prn, chk_code, prn_name, debug_rep_list);
						}
						else
							MakeDbgPrintLogList(2, buf_errprn, chk_code, prn_name, debug_rep_list);
					}
					else
						MakeDbgPrintLogList(1, buf_ulp, chk_code, prn_name, debug_rep_list);
				for(i = 0; i < printed_pos_list.getCount(); i++)
					saved_check.at(printed_pos_list.at(i)).Flags |= cifIsPrinted;
				P = saved_check;
				AutosaveCheck(); // @v8.7.11
				OnUpdateList(0);
				if(do_debug_log)
					PPLogMessageList(PPFILNAM_DEBUG_LOG, debug_rep_list, LOGMSGF_TIME|LOGMSGF_USER);
			}
			else if(to_local_prn == 0) {
				Print((selPrnType ? 0 : 1), 0, 0);
				AutosaveCheck(); // @v8.7.11
			}
			ok = 1;
		}
		if(IsState(sEMPTYLIST_EMPTYBUF)) {
			P.freeAll();
			CheckID = cur_chk_id;
		}
	}
	CATCHZOKPPERR
	return ok;
}

int CheckPaneDialog::ResetOperRightsByKey()
{
	int    ok = -1;
	if((CnFlags & CASHF_KEYBOARDWKEY) && GetInput() && Input.IsDigit()) {
		long   key_pos = Input.ToLong();
		PPIDArray oper_rights_ary;
		if(GetOperRightsByKeyPos((int)key_pos, &oper_rights_ary) > 0) {
			long   f = OrgOperRights;
			int    esc_chk = oper_rights_ary.lsearch(CSESSRT_ESCCHECK);
			SETFLAG(f, orfReturns,     oper_rights_ary.lsearch(CSESSOPRT_RETCHECK));
			SETFLAG(f, orfEscCheck,    esc_chk);
			SETFLAG(f, orfEscChkLine, /*esc_chk && @?*/oper_rights_ary.lsearch(CSESSOPRT_ESCCLINE));
			SETFLAG(f, orfBanking,     oper_rights_ary.lsearch(CSESSOPRT_BANKING));
			SETFLAG(f, orfZReport,     oper_rights_ary.lsearch(CSESSRT_CLOSE));
			SETFLAG(f, orfCopyCheck,   oper_rights_ary.lsearch(CSESSOPRT_COPYCHECK));
			SETFLAG(f, orfCopyZReport, oper_rights_ary.lsearch(CSESSOPRT_COPYZREPT));
			SETFLAG(f, orfRowDiscount, oper_rights_ary.lsearch(CSESSOPRT_ROWDISCOUNT));
			SETFLAG(f, orfXReport,     oper_rights_ary.lsearch(CSESSOPRT_XREP << 16));
			SETFLAG(f, orfSplitCheck,      oper_rights_ary.lsearch(CSESSOPRT_SPLITCHK));
			SETFLAG(f, orfMergeChecks,     oper_rights_ary.lsearch(CSESSOPRT_MERGECHK));      // @v8.5.5
			SETFLAG(f, orfChgPrintedCheck, oper_rights_ary.lsearch(CSESSOPRT_CHGPRINTEDCHK));
			SETFLAG(f, orfChgAgentInCheck, oper_rights_ary.lsearch(CSESSOPRT_CHGCCAGENT));    // @v8.2.1
			SETFLAG(f, orfEscChkLineBeforeOrder, oper_rights_ary.lsearch(CSESSOPRT_ESCCLINEBORD)); // @v8.7.3
			if(!(Flags & fUsedRighsByAgent))
				OrgOperRights = OperRightsFlags = f;
			else
				OrgOperRights = f;
			{
				SString added_msg_str;
				(added_msg_str = "Key").Space().Cat(key_pos);
				DS.GetTLA().AddedMsgStrNoRights = added_msg_str;
			}
			ok = 1;
		}
		setupHint();
	}
	ClearInput(0);
	return ok;
}

int CheckPaneDialog::PrintCashReports()
{
	int    ok = 1;
	int    r = -1;
	int    r_ext = -1;
	int    c = cmCancel;
	int    zreport_printed = 0;
	CSPanel * dlg = 0;
	if(!(Flags & fNoEdit) && IsState(sEMPTYLIST_EMPTYBUF)) {
		int    csp_flags = 0;
		/*
			����� X-�����;������� ������;����� Z-������;����������;�����������;��������������;������� �������� ����;����� ����� ����-�����
		*/
		{
#if 0 // @construction {
			SString temp_buf;
			if(PPLoadTextWin(PPTXT_MENU_CHKPAN, temp_buf)) {
				TMenuPopup menu;
				menu.AddSubstr(temp_buf, 0, cmSCSXReport);
				menu.AddSubstr(temp_buf, 1, cmCSOpen);
				menu.AddSubstr(temp_buf, 2, cmSCSZReportCopy);
				menu.AddSubstr(temp_buf, 3, cmSCSIncasso);
				menu.AddSubstr(temp_buf, 4, cmSCSLock);
				menu.AddSubstr(temp_buf, 5, cmSCSUnlock);
				menu.AddSubstr(temp_buf, 6, /*������� �������� ����*/0);
				menu.AddSubstr(temp_buf, 7, cmSCSLock);
				int    cmd = menu.Execute(H(), TMenuPopup::efRet);
				/*
				if(cmd > 0)
					::message(this, evCommand, cmd, 0);
				*/
			}
#endif // } 0 @construction
		}
		LDATE  dt;
		SETFLAG(csp_flags, CSPanel::fcspZReport,  OperRightsFlags & orfZReport);
		SETFLAG(csp_flags, CSPanel::fcspZRepCopy, OperRightsFlags & orfCopyZReport);
		SETFLAG(csp_flags, CSPanel::fcspXReport,  OperRightsFlags & orfXReport);
		dlg = new CSPanel((DlgFlags & fLarge) ? DLG_CASHREPORTS_L : DLG_CASHREPORTS, CashNodeID, 0, csp_flags); // @newok
		THROW(CheckDialogPtr(&dlg));
		THROW(InitCashMachine());
		dlg->showCtrl(CTL_CSPANEL_CSESSOPEN,  (Flags & fOnlyReports) ? 1 : 0);
		dlg->showCtrl(CTL_CSPANEL_CSESSCLOSE, (Flags & fOnlyReports) ? 0 : 1);
		while((c = ExecView(dlg)) != cmCancel) {
			switch(c) {
				case cmCSOpen:
					r = P_CM->SyncOpenSession(&(dt = ZERODATE));
					if(P_CM_EXT)
						if(P_CM_EXT->GetNodeData().CashType == PPCMT_PAPYRUS) {
							if(r > 0) {
								P_CM_EXT->SetParentNode(CashNodeID);
								P_CM_EXT->AsyncOpenSession(0, 0);
							}
						}
						else
							r_ext = P_CM_EXT->SyncOpenSession(&dt);
					if(r > 0 || r_ext > 0)
						Flags &= ~fOnlyReports;
					break;
				case cmCSClose:
					r = P_CM->SyncCloseSession();
					if(P_CM_EXT)
						if(P_CM_EXT->GetNodeData().CashType == PPCMT_PAPYRUS) {
							if(r > 0) {
								P_CM_EXT->SetParentNode(CashNodeID);
								P_CM_EXT->AsyncCloseSession(0);
							}
						}
						else
							r_ext = P_CM_EXT->SyncCloseSession();
					if(r > 0 || r_ext > 0)
						zreport_printed = 1;
					break;
				case cmSCSXReport:
					r = P_CM->SyncPrintXReport();
					if(P_CM_EXT)
						r_ext = P_CM_EXT->SyncPrintXReport();
					break;
				case cmSCSZReportCopy:
					if(OperRightsFlags & orfCopyZReport) {
						CSessInfo  cs_info;
						r = SelectCSession(CashNodeID, ExtCashNodeID, &cs_info);
						if(r == 1)
							r = P_CM->SyncPrintZReportCopy(&cs_info);
						else if(r == 2 && P_CM_EXT)
							r_ext = P_CM_EXT->SyncPrintZReportCopy(&cs_info);
					}
					else
						r = 0;
					break;
				case cmSCSLock:
					r = Sleep();
					break;
				case cmSCSIncasso:
					r = P_CM->SyncPrintIncasso();
					break;
			}
			if(r == 0 || r_ext == 0)
				PPError();
			else
				break;
		}
		delete dlg;
		dlg = 0;
		if(zreport_printed)
			if(PPMessage(mfConf|mfYesNo, PPCFM_PREVCASHDAYCLOSED, 0) == cmYes) {
				dt = ZERODATE;
				if(r > 0)
					THROW(ok = P_CM->SyncOpenSession(&dt));
				if(ok > 0 && r_ext > 0)
					THROW(ok = P_CM_EXT->SyncOpenSession(&dt));
				if(ok > 0)
					Flags &= ~fOnlyReports;
			}
			else {
				Flags |= fOnlyReports;
				SetupInfo(0);
			}
	}
	ok = (Flags & fOnlyReports) ? -1 : 1;
	CATCHZOKPPERR
	delete dlg;
	return ok;
}

int CheckPaneDialog::GetDataFromScale(PPID * pGoodsID, double * pWeight)
{
	TIDlgInitData  tidi;
	int  ok = GetScaleData(ScaleID, &tidi);
	ASSIGN_PTR(pGoodsID, tidi.GoodsID);
	ASSIGN_PTR(pWeight, tidi.Quantity);
	return ok;
}

int CheckPaneDialog::SetupRowByScale()
{
	int    ok = -1;
	if(ScaleID) {
		PPID   goods_id = 0;
		double weight = 0.0;
		int    r = GetDataFromScale(&goods_id, &weight);
		if(r > 0 && goods_id) {
			PgsBlock pgsb(weight);
			ok = SetupNewRow(goods_id, pgsb);
			Flags &= ~fNotUseScale;
		}
		else if(r != -2)
			ok = PPErrorZ();
	}
	return ok;
}

int CheckPaneDialog::LoadTSession(PPID tsessID)
{
	int    ok = 1;
	SString temp_buf;
	TSessionTbl::Rec tses_rec;
	TSessLineTbl::Rec ln_rec;
	THROW_MEM(SETIFZ(P_TSesObj, new PPObjTSession));
	THROW(P_TSesObj->Search(tsessID, &tses_rec) > 0);
	if(tses_rec.CCheckID_) {
		//
		// �� ������ ��� ��� ����������� ���
		//
		CCheckTbl::Rec cc_rec;
		if(CC.Search(tses_rec.CCheckID_, &cc_rec) > 0)
			CCheckCore::MakeCodeString(&cc_rec, temp_buf);
		else
			temp_buf = 0;
		CALLEXCEPT_PP_S(PPERR_TSESSALREADYCCHECKED, temp_buf);
	}
	//
	// ������������ ���� �������� ������ �� �������� ������
	//
	THROW_PP(tses_rec.Status == TSESST_CLOSED, PPERR_TSESSCCHECKNOTCLOSED);
	{
		PPObjTSession::WrOffAttrib attrib;
		// @v8.5.7 @1 {
		THROW(P_TSesObj->GetWrOffAttrib(&tses_rec, &attrib));
		SetupAgent(attrib.AgentID, 0);
		if(attrib.SCardID)
			AcceptSCard(0, attrib.SCardID);
		// }
		for(SEnum en = P_TSesObj->P_Tbl->EnumLines(tsessID); en.Next(&ln_rec) > 0;) {
			if(ln_rec.Sign < 0) {
				PgsBlock pgsb(fabs(ln_rec.Qtty));
				pgsb.PriceBySerial = ln_rec.Price;
				/*THROW(*/SetupNewRow(ln_rec.GoodsID, /*fabs(ln_rec.Qtty), ln_rec.Price, 0*/pgsb)/*)*/;
			}
		}
		AcceptRow();
		/* @v8.5.7 @moved to @1
		THROW(P_TSesObj->GetWrOffAttrib(&tses_rec, &attrib));
		SetupAgent(attrib.AgentID, 0);
		if(attrib.SCardID)
			AcceptSCard(0, attrib.SCardID);
		*/
		OuterOi.Set(PPOBJ_TSESSION, tsessID);
		SetupInfo(0);
		ClearInput(0);
	}
	CATCH
		ok = MessageError(PPErrCode, 0, eomBeep | eomStatusLine);
	ENDCATCH
	return ok;
}

int CheckPaneDialog::LoadChkInP(PPID chkinpID, PPID goodsID, double qtty)
{
	int    ok = 1;
	PPObjTSession::WrOffAttrib wroff_attrib;
	SString temp_buf;
	PPCheckInPersonMngr cip_mgr;
	PPCheckInPersonItem cip_item;
	MEMSZERO(wroff_attrib);
	THROW(cip_mgr.Search(chkinpID, &cip_item) > 0);
	if(cip_item.CCheckID) {
		//
		// �� ������ ��� ��� ����������� ���
		//
		CCheckTbl::Rec cc_rec;
		if(CC.Search(cip_item.CCheckID, &cc_rec) > 0)
			CCheckCore::MakeCodeString(&cc_rec, temp_buf);
		else
			temp_buf = 0;
		CALLEXCEPT_PP_S(PPERR_CHKINPALREADYCCHECKED, temp_buf);
	}
	//
	// ������������ ���� �������� ������ �� �������� ������
	//
	THROW_PP(cip_item.GetStatus() == cip_item.statusCheckedIn, PPERR_CHKINPCCHECKNOTCLOSED);
	if(cip_item.Kind == PPCheckInPersonItem::kTSession && cip_item.PrmrID) {
		TSessionTbl::Rec tses_rec;
		THROW_MEM(SETIFZ(P_TSesObj, new PPObjTSession));
		if(P_TSesObj->Search(cip_item.PrmrID, &tses_rec) > 0) {
			THROW(P_TSesObj->GetWrOffAttrib(&tses_rec, &wroff_attrib));
		}
	}
	{
		PgsBlock pgsb(fabs(qtty));
		/*THROW(*/SetupNewRow(goodsID, /*fabs(qtty), 0.0, 0*/pgsb)/*)*/;
		AcceptRow();
		SetupAgent(wroff_attrib.AgentID, 0);
		if(cip_item.SCardID)
			AcceptSCard(0, cip_item.SCardID);
		OuterOi.Set(PPOBJ_CHKINP, chkinpID);
		SetupInfo(0);
		ClearInput(0);
	}
	CATCH
		ok = MessageError(PPErrCode, 0, eomBeep | eomStatusLine);
	ENDCATCH
	return ok;
}
//
//
//
int SLAPI CCheckPane(PPID cashNodeID, PPID chkID, const char * pInitLine, long flags)
{
	MemLeakTracer mlt;
	int    ok = 1, is_touch_screen = 0;
	const  PPID sav_loc_id = LConfig.Location;
	CCheckPacket pack;
	CheckPaneDialog * dlg = 0;
	PPObjSCard sc_obj;
  	if(cashNodeID) {
		PPCashNode    cn_rec;
		PPObjCashNode cn_obj;
		THROW(cn_obj.Search(cashNodeID, &cn_rec) > 0);
		if(cn_rec.CashType != PPCMT_DISTRIB)
			THROW(DS.SetLocation(cn_rec.LocID));
		if(!chkID && cn_rec.Flags & CASHF_SYNC) {
			PPSyncCashNode  scn;
			if(cn_obj.GetSync(cashNodeID, &scn) > 0) {
				const PPID ts_id = NZOR(scn.LocalTouchScrID, scn.TouchScreenID);
				if(ts_id)
					is_touch_screen = 1;
			}
		}
	}
	if(chkID) {
		THROW(sc_obj.P_CcTbl->LoadPacket(chkID, 0, &pack));
	}
	THROW(CheckDialogPtr(&(dlg = new CheckPaneDialog(cashNodeID, chkID, chkID ? &pack : 0, is_touch_screen)), 0));
	if(pInitLine)
		dlg->SetInput(pInitLine);
	if(flags & cchkpanfOnce)
		dlg->UiFlags |= (CheckPaneDialog::uifOnce | CheckPaneDialog::uifCloseWOAsk);
	ExecViewAndDestroy(dlg);
	CATCHZOKPPERR
	DS.SetLocation(sav_loc_id);
	return ok;
}
//
// InfoKiosk
//
class InfoKioskDialog : public TDialog {
public:
	InfoKioskDialog(const PPGoodsInfo * pRec, PPID defGoodsGrpID) : TDialog(pRec && pRec->TouchScreenID ? DLG_INFKIOSK_TS : DLG_INFKIOSK)
	{
		SString font_face;
		SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_INFKIOSK_LOTS);
		if(pRec)
			Rec = *pRec;
		else
			MEMSZERO(Rec);
		SelGoodsGrpID = AltGoodsGrpID = defGoodsGrpID;
		LastCtrlID = 0;
		if(p_list)
			SetupStrListBox(p_list);
		SetupLots(0);
		Flags = (pRec && pRec->TouchScreenID) ? fTouchScreen : 0;
		if(Flags & fTouchScreen) {
			int  set_tool_tips = 1;
			PPTouchScreenPacket ts_pack;
			PPObjTouchScreen    ts_obj;
			if(ts_obj.GetPacket(pRec->TouchScreenID, &ts_pack) > 0) {
				if(!defGoodsGrpID)
					SelGoodsGrpID = AltGoodsGrpID = ts_pack.Rec.AltGdsGrpID;
				if(ts_pack.Rec.Flags & TSF_TXTSTYLEBTN) {
					ResetButtonToTextStyle(CTL_INFKIOSK_SCARD);
					ResetButtonToTextStyle(CTL_INFKIOSK_BYPRICE);
					ResetButtonToTextStyle(CTL_INFKIOSK_BYNAME);
					ResetButtonToTextStyle(CTL_INFKIOSK_PRINTLBL);
					ResetButtonToTextStyle(CTL_INFKIOSK_ENTER);
					ResetButtonToTextStyle(CTL_INFKIOSK_SELGDSGRP);
					ResetButtonToTextStyle(CTL_INFKIOSK_GRPBYDEF);
					set_tool_tips = 0;
				}
			}
			if(!SetupStrListBox(this, CTL_INFKIOSK_GRPLIST))
				PPError();
			SmartListBox * p_tree_list = (SmartListBox *)getCtrlView(CTL_INFKIOSK_GRPLIST);
			if(p_tree_list) {
				PPObjGoodsGroup gg_obj;
				p_tree_list->setDef(gg_obj.Selector(0));
				p_tree_list->drawView();
				p_tree_list->def->SetOption(lbtSelNotify, 1);
			}
			UpdateGList(-1);
			if(set_tool_tips)
				TView::message(this, evCommand, cmSetupTooltip);
			enableCommand(cmaAltSelect, AltGoodsGrpID ? 1 : 0);
			DlgFlags |= fLarge;
		}
		selectCtrl(CTL_INFKIOSK_INPUT);
		DefInputLine = CTL_INFKIOSK_INPUT;
		PPGetSubStr(PPTXT_FONTFACE, PPFONT_TIMESNEWROMAN, font_face);
		SetCtrlsFont(font_face, (Flags & fTouchScreen) ? 32 : 24, CTL_INFKIOSK_GOODS, CTL_INFKIOSK_CODE,
			CTL_INFKIOSK_PRICE, CTL_INFKIOSK_DSCNTPRICE, CTL_INFKIOSK_STATUS, 0);
		SetCtrlsFont(font_face, 20, CTL_INFKIOSK_LOTS, CTL_INFKIOSK_ADDINF1, CTL_INFKIOSK_ADDINF2,
			CTL_INFKIOSK_ADDINF3, CTL_INFKIOSK_ADDINF4, CTL_INFKIOSK_ADDINF5, 0);
		SetCtrlFont(CTL_INFKIOSK_INPUT,      font_face, (Flags & fTouchScreen) ? 20 : 16);
	}
private:
	DECL_HANDLE_EVENT;

	enum SearchParam {
		srchByNone    = 1,
		srchByBarcode,
		srchBySerial,
		srchByPrice,
		srchByName
	};

	int    SelectSCard();
	int    SelectGoods(SearchParam srch);
	int    SetupGoods(PPID goodsID, double qtty);
	int    SetupLots(PPID goodsID);
	int    SetupInfo();
	int    PrintLabel();
	int    GetInput();
	void   ClearInput();
	void   UpdateGList(int updGdsList);
	int    ProcessGoodsSelection();
	void   ResetListWindows();
	int    SetDlgResizeParams();
	void   ResetButtonToTextStyle(uint ctrlID);
	int    ProcessEnter();
	enum {
		fWaitOnSCard  = 0x0001, // �������� ����� ���������� �����
		fTouchScreen  = 0x0002  // ������������ TouchScreen
	};
	long   Flags;
	PPID   LastCtrlID;
	PPID   AltGoodsGrpID;
	PPID   SelGoodsGrpID;
	SString Input;
	struct State {
		State()
		{
			Reset();
		}
		void Reset()
		{
			THISZERO();
		}
		PPID   GoodsID;
		double Price;
		double Rest;
		double Qtty;
	};
	State  St;
    PPGoodsInfo Rec;
	PPObjGoods  GObj;
	PPObjQCert  QCObj;
	PPObjSCard  SCObj;
	PPObjSCardSeries SCSerObj;
};

int InfoKioskDialog::SetupInfo()
{
	int    ok = 1;
	SString status_buf, word;
	if(St.Rest > 0.0) {
		PPLoadString("rest", word);
		status_buf.Space().Cat(word).CatDiv(':', 2).Cat(St.Rest, MKSFMTD(0, 3, NMBF_NOTRAILZ)).Space();
	}
	if(St.Qtty > 0.0) {
		PPLoadString("qtty", word);
		status_buf.Space().Cat(word).CatDiv(':', 2).Cat(St.Qtty, MKSFMTD(0, 3, NMBF_NOTRAILZ));
	}
	setStaticText(CTL_INFKIOSK_STATUS, status_buf);
	return ok;
}

int InfoKioskDialog::ProcessEnter()
{
	int    ok = 1;
	if(GetInput()) {
		if(Input.C(0) == '*' || Input.Last() == '*') {
			if(St.GoodsID) {
				St.Qtty = R6(Input.ShiftLeftChr('*').ToReal());
				if(St.Qtty > 0.0) {
					if(Input.TrimRightChr('*').Last() == '/') {
						double phuperu;
						if(GObj.GetPhUPerU(St.GoodsID, 0, &phuperu) > 0)
							St.Qtty = R6(St.Qtty / phuperu);
					}
					SetupInfo();
					ClearInput();
				}
			}
		}
		else if(Flags & fWaitOnSCard)
			SelectSCard();
		else
			SelectGoods(srchByBarcode);
	}
	else
		ok = -1;
	return ok;
}

IMPL_HANDLE_EVENT(InfoKioskDialog)
{
	if(TVCOMMAND && TVCMD == cmOK) {
		ProcessEnter();
		clearEvent(event);
	}
	TDialog::handleEvent(event);
	if(TVBROADCAST)
		if(TVCMD == cmReleasedFocus && (Flags & fTouchScreen))
			LastCtrlID = event.message.infoView->GetId();
		else
			return;
	else if(TVCOMMAND) {
		const uint ev_ctl_id = event.getCtlID();
		switch(TVCMD) {
			case cmSetupResizeParams:
				SetDlgResizeParams();
				break;
			case cmSetupTooltip:
				if(Flags & fTouchScreen) {
					SString  tt_names, name;
					if(PPLoadTextWin(PPTXT_INFKIOSK_TOOLTIPS, tt_names))
						for(uint idx = 0; idx < CTL_INFKIOSK_NUMBUTTONS; idx++)
							if(PPGetSubStr(tt_names, idx, name))
								SetCtrlToolTip(CTL_INFKIOSK_STARTBUTTON + idx, name);
				}
				break;
			case cmaInsert:
				if(ProcessEnter() < 0) {
					if(LastCtrlID == CTL_INFKIOSK_GRPLIST)
						UpdateGList(1);
					else if(LastCtrlID == CTL_INFKIOSK_GDSLIST)
						ProcessGoodsSelection();
				}
				break;
			case cmaSelect:
				UpdateGList(0);
				break;
			case cmaAltSelect:
				SelGoodsGrpID = AltGoodsGrpID;
				UpdateGList(-1);
				break;
			case cmLBItemSelected:
			case cmLBDblClk:
				if(ev_ctl_id == CTL_INFKIOSK_GRPLIST) {
					UpdateGList(1);
					if(TVCMD == cmLBItemSelected)
						selectCtrl(CTL_INFKIOSK_INPUT);
				}
				else if(ev_ctl_id == CTL_INFKIOSK_GDSLIST)
					ProcessGoodsSelection();
				break;
			case cmSelSCard:
				SelectSCard();
				break;
			case cmByPrice:
				UpdateGList(-2);
				break;
			case cmByName:
				UpdateGList(-3);
				break;
			case cmPrint:
				PrintLabel();
				break;
			default:
				return;
		}
	}
	else if(TVKEYDOWN) {
		switch(TVKEY) {
			case kbF2:
				SelectGoods(srchByNone);
				break;
			case kbF3:
				SelectSCard();
				break;
			case kbF4:
				if(Flags & fTouchScreen)
					UpdateGList(-3);
				else
					SelectGoods(srchByName);
				break;
			case kbF5:
				if(Flags & fTouchScreen)
					UpdateGList(-2);
				else
					SelectGoods(srchByPrice);
				break;
			case kbF7:
				PrintLabel();
				break;
			default:
				return;
		}
	}
	else
		return;
	clearEvent(event);
}

void InfoKioskDialog::ResetButtonToTextStyle(uint ctrlID)
{
	long   style = TView::GetWindowStyle(::GetDlgItem(H(), ctrlID));
	style &= ~BS_BITMAP;
	TView::SetWindowProp(::GetDlgItem(H(), ctrlID), GWL_STYLE, style);
}

int InfoKioskDialog::GetInput()
{
	getCtrlString(CTL_INFKIOSK_INPUT, Input);
	return Input.NotEmptyS();
}

void InfoKioskDialog::ClearInput()
{
	setCtrlString(CTL_INFKIOSK_INPUT, Input = 0);
	selectCtrl(CTL_INFKIOSK_INPUT);
}

void InfoKioskDialog::UpdateGList(int updGdsList)
{
	if(Flags & fTouchScreen) {
		SString  grp_name;
		if(updGdsList > 0) {
			PPID  cur_id = 0;
			SmartListBox * p_tree_list = (SmartListBox *)getCtrlView(CTL_INFKIOSK_GRPLIST);
			p_tree_list->def->getCurID(&cur_id);
			if(((StdTreeListBoxDef *)p_tree_list->def)->HasChild(cur_id))
				updGdsList = 0;
			else
				SelGoodsGrpID = cur_id;
		}
		if(updGdsList) {
			ListBoxDef   * p_def = 0;
			SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_INFKIOSK_GDSLIST);
			StrAssocArray * p_ts_ary = 0;
			PPWait(1);
			if(updGdsList == -2) {
				if(GetInput()) {
					double p = Input.ToReal();
					p_ts_ary = GObj.CreateListByPrice(LConfig.Location, R2(p));
					p_def = new StrAssocListBoxDef(p_ts_ary, lbtDblClkNotify | lbtFocNotify | lbtDisposeData);
					PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SELBYPRICE, grp_name);
					grp_name.Space().Cat(p, SFMT_MONEY);
				}
				ClearInput();
			}
			else if(updGdsList == -3) {
				if(GetInput()) {
					SString   pattern;
					if(Input.Len() >= INSTVSRCH_THRESHOLD)
						pattern.CatChar('!').Cat(Input);
					else
						pattern = Input;
					p_ts_ary = new StrAssocArray;
					GObj.P_Tbl->GetListBySubstring(pattern, p_ts_ary, -1, 1);
					p_def = new StrAssocListBoxDef(p_ts_ary, lbtDblClkNotify | lbtFocNotify | lbtDisposeData);
					grp_name = Input;
				}
				ClearInput();
			}
			else {
				Goods2Tbl::Rec grp_rec;
				if(GObj.Fetch(SelGoodsGrpID, &grp_rec) > 0)
					PPGetWord(PPWORD_GROUP, 0, grp_name).CatDiv(':', 2).Cat(grp_rec.Name);
				else
					grp_name = 0;
				p_def = GObj.Selector((void *)SelGoodsGrpID);
			}
			p_list->setDef(p_def);
			if(p_list->def)
				p_list->def->SetOption(lbtSelNotify, 1);
			p_list->drawView();
			PPWait(0);
		}
		else
			PPGetSubStr(PPTXT_CHKPAN_INFO, PPCHKPAN_SELGROUP, grp_name);
		showCtrl(CTL_INFKIOSK_GRPLIST,    !updGdsList);
		disableCtrl(CTL_INFKIOSK_GRPLIST,  updGdsList);
		::ShowWindow(::GetDlgItem(H(), MAKE_BUTTON_ID(CTL_INFKIOSK_GRPLIST, 1)), updGdsList ? SW_HIDE : SW_SHOW);
		showCtrl(CTL_INFKIOSK_GDSLIST,     updGdsList);
		disableCtrl(CTL_INFKIOSK_GDSLIST, !updGdsList);
		::ShowWindow(::GetDlgItem(H(), MAKE_BUTTON_ID(CTL_INFKIOSK_GDSLIST, 1)), updGdsList ? SW_SHOW : SW_HIDE);
		enableCommand(cmaSelect, updGdsList);
		setStaticText(CTL_INFKIOSK_GRPNAME, grp_name);
		LastCtrlID = updGdsList ? CTL_INFKIOSK_GDSLIST : CTL_INFKIOSK_GRPLIST;
	}
}

int InfoKioskDialog::ProcessGoodsSelection()
{
	int    ok = 1;
	PPID   goods_id = 0;
	SString  buf;
	SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_INFKIOSK_GDSLIST);
	p_list->def->getCurID(&goods_id);
	ClearInput();
	setCtrlReal(CTL_INFKIOSK_DSCNTPRICE, 0.0);
	setStaticText(CTL_INFKIOSK_INFO, buf);
	if(!SetupGoods(goods_id, 0.0)) {
		PPGetMessage(mfError, PPErrCode, DS.GetTLA().AddedMsgString, 0, buf);
		setStaticText(CTL_INFKIOSK_INFO, buf);
		ok = 0;
	}
	return ok;
}

void InfoKioskDialog::ResetListWindows()
{
	if(Flags & fTouchScreen) {
		SString font_face;
		int   sx  = GetSystemMetrics((DlgFlags & fResizeable) ? SM_CXSIZEFRAME : SM_CXFIXEDFRAME);
		int   sy  = GetSystemMetrics((DlgFlags & fResizeable) ? SM_CYSIZEFRAME : SM_CYFIXEDFRAME);
		int   cy  = GetSystemMetrics(SM_CYCAPTION);
		int   vsx = GetSystemMetrics(SM_CXVSCROLL);
		RECT  dlg_rect, ctrl_rect;
		HWND  ctrl_wnd = GetDlgItem(H(), CTL_INFKIOSK_GDSLIST);
		::GetWindowRect(H(), &dlg_rect);
		GetWindowRect(ctrl_wnd, &ctrl_rect);
		ctrl_rect.right -= vsx;
		MoveWindow(ctrl_wnd, ctrl_rect.left - (dlg_rect.left + sx), ctrl_rect.top - (dlg_rect.top + sy + cy),
			ctrl_rect.right - ctrl_rect.left, ctrl_rect.bottom - ctrl_rect.top, 1);
		ctrl_wnd = ::GetDlgItem(H(), MAKE_BUTTON_ID(CTL_INFKIOSK_GDSLIST, 1));
		::GetWindowRect(ctrl_wnd, &ctrl_rect);
		ctrl_rect.left -= vsx;
		MoveWindow(ctrl_wnd, ctrl_rect.left - (dlg_rect.left + sx), ctrl_rect.top - (dlg_rect.top + sy + cy),
			ctrl_rect.right - ctrl_rect.left, ctrl_rect.bottom - ctrl_rect.top, 1);
		PPGetSubStr(PPTXT_FONTFACE, PPFONT_TIMESNEWROMAN, font_face);
		SetCtrlFont(CTL_INFKIOSK_GRPNAME, font_face, 32);
		SetCtrlFont(CTL_INFKIOSK_GDSLIST, font_face, 24);
		SetCtrlFont(CTL_INFKIOSK_GRPLIST, font_face, 24);
	}
}

int InfoKioskDialog::SetDlgResizeParams()
{
	if(Flags & fTouchScreen) {
		SetCtrlResizeParam(CTL_INFKIOSK_GOODS, 0, 0, CTL_INFKIOSK_GRPNAME, -1, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPNAME, CTL_INFKIOSK_GOODS, 0, 0, -1, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GDSLIST, CTL_INFKIOSK_GRPNAME, 0, 0, 0, crfLinkLeft | crfResizeable);
		SetCtrlResizeParam(MAKE_BUTTON_ID(CTL_INFKIOSK_GDSLIST, 1), -1, 0, 0, 0, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPLIST, CTL_INFKIOSK_GRPNAME, 0, 0, 0, crfLinkLeft | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_CODE, CTL_INFKIOSK_GOODS, 0, CTL_INFKIOSK_PRICE, -1,
			crfLinkLeft | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_PRICE, CTL_INFKIOSK_CODE, 0, CTL_INFKIOSK_GOODS, -1,
			crfLinkRight | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_STATUS, CTL_INFKIOSK_CODE, 0, CTL_INFKIOSK_CODE, -1,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_DSCNTPRICE, CTL_INFKIOSK_PRICE, 0, CTL_INFKIOSK_PRICE, -1,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_IMAGE, 0, 0, CTL_INFKIOSK_CODE, CTL_INFKIOSK_LOTS,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_LOTS, 0, CTL_INFKIOSK_IMAGE, CTL_INFKIOSK_GOODS, 0,
			crfLinkRight | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_PRICE, 0, CTL_INFKIOSK_GOODS, CTL_INFKIOSK_IMAGE,
			CRF_LINK_LEFTRIGHTBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF1, CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_GRPBOX1,
			CTL_INFKIOSK_ADDINF2, CRF_LINK_LEFTRIGHTTOP | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF2, CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_ADDINF1, CTL_INFKIOSK_GRPBOX1,
			CTL_INFKIOSK_ADDINF3, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF3, CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_ADDINF2, CTL_INFKIOSK_GRPBOX1,
			CTL_INFKIOSK_ADDINF4, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF4, CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_ADDINF3, CTL_INFKIOSK_GRPBOX1,
			CTL_INFKIOSK_ADDINF5, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF5, CTL_INFKIOSK_GRPBOX1, CTL_INFKIOSK_ADDINF4, CTL_INFKIOSK_GRPBOX1,
			CTL_INFKIOSK_GRPBOX1, CRF_LINK_LEFTRIGHTBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_GOODS, CTL_INFKIOSK_LOTS, CTL_INFKIOSK_GOODS,
			CTL_INFKIOSK_IMAGE, CRF_LINK_ALL | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_SCARD, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_BYPRICE,
			CTL_INFKIOSK_GRPBOX2, CRF_LINK_LEFTTOPBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_BYPRICE, CTL_INFKIOSK_SCARD, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_BYNAME,
			CTL_INFKIOSK_GRPBOX2, CRF_LINK_TOPBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_BYNAME, CTL_INFKIOSK_BYPRICE, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_PRINTLBL,
			CTL_INFKIOSK_GRPBOX2, CRF_LINK_TOPBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_PRINTLBL, CTL_INFKIOSK_BYNAME, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_ENTER,
			CTL_INFKIOSK_GRPBOX2, CRF_LINK_TOPBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ENTER, CTL_INFKIOSK_PRINTLBL, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_GRPBOX2,
			CTL_INFKIOSK_GRPBOX2, CRF_LINK_RIGHTTOPBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_INPUT, 0, -1, CTL_INFKIOSK_IMAGE, 0, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBOX3, 0, -1, CTL_INFKIOSK_GRPBOX4, 0, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBOX4, CTL_INFKIOSK_GRPBOX3, -1, 0, 0, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_INFO, CTL_INFKIOSK_GRPBOX1, -1, CTL_INFKIOSK_GRPBOX3, 0,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_SELGDSGRP, CTL_INFKIOSK_GRPBOX4, -1, CTL_INFKIOSK_GRPBYDEF, 0,
			crfLinkLeft | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBYDEF, CTL_INFKIOSK_SELGDSGRP, -1, CTL_INFKIOSK_GRPBOX4, 0,
			crfLinkRight | crfResizeable);
	}
	else {
		SetCtrlResizeParam(CTL_INFKIOSK_GOODS, 0, 0, CTL_INFKIOSK_IMAGE, -1, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_IMAGE, CTL_INFKIOSK_GOODS, 0, 0, CTL_INFKIOSK_LOTS, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_LOTS, CTL_INFKIOSK_IMAGE, CTL_INFKIOSK_IMAGE, CTL_INFKIOSK_IMAGE, 0,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_CODE, CTL_INFKIOSK_GOODS, 0, CTL_INFKIOSK_GOODS, -1,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_PRICE, CTL_INFKIOSK_GOODS, 0, CTL_INFKIOSK_DSCNTPRICE, -1,
			crfLinkLeft | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_DSCNTPRICE, CTL_INFKIOSK_PRICE, 0, CTL_INFKIOSK_GOODS, -1,
			crfLinkRight | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_STATUS, CTL_INFKIOSK_GOODS, 0, CTL_INFKIOSK_GOODS, -1,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBOX1, 0, 0, 0, 0, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_GOODS, 0, CTL_INFKIOSK_GOODS, 0,
			CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF1, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_GRPBOX2,
			CTL_INFKIOSK_ADDINF2, CRF_LINK_LEFTRIGHTTOP | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF2, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_ADDINF1, CTL_INFKIOSK_GRPBOX2,
			CTL_INFKIOSK_ADDINF3, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF3, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_ADDINF2, CTL_INFKIOSK_GRPBOX2,
			CTL_INFKIOSK_ADDINF4, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF4, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_ADDINF3, CTL_INFKIOSK_GRPBOX2,
			CTL_INFKIOSK_ADDINF5, CRF_LINK_LEFTRIGHT | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_ADDINF5, CTL_INFKIOSK_GRPBOX2, CTL_INFKIOSK_ADDINF4, CTL_INFKIOSK_GRPBOX2,
			CTL_INFKIOSK_GRPBOX2, CRF_LINK_LEFTRIGHTBOTTOM | crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_INPUT, 0, -1, CTL_INFKIOSK_INFO, 0, crfResizeable);
		SetCtrlResizeParam(CTL_INFKIOSK_INFO, CTL_INFKIOSK_INPUT, -1, 0, 0, crfResizeable);
		LinkCtrlsToDlgBorders(CRF_LINK_RIGHTBOTTOM, STDCTL_OKBUTTON, CTL_INFKIOSK_PRINTLBL, STDCTL_CANCELBUTTON, 0L);
	}
	ResetListWindows();
	ResizeDlgToFullScreen();
	return 1;
}

int InfoKioskDialog::SelectSCard()
{
	int     ok = 1;
	double  dscnt_price = 0.0;
	GetInput();
	SString buf = Input;
	if(PPObjSCard::PreprocessSCardCode(buf) > 0) {
		double   pct_dis = 0.0;
		SString  info, person;
		SCardTbl::Rec  rec;
		PPSCardSerPacket  ser_pack;
		Flags &= ~fWaitOnSCard;
		THROW(ok = SCObj.SearchCode(0, buf, &rec));
		PPSetAddedMsgString(buf);
		THROW_PP(ok > 0, PPERR_SCARDNOTFOUND);
		THROW(SCSerObj.GetPacket(rec.SeriesID, &ser_pack));
		pct_dis = fdiv100i((rec.Flags & SCRDF_INHERITED) ? ser_pack.Rec.PDis : rec.PDis);
		PPLoadText(PPTXT_SCARDINFO, buf);
		GetPersonName(rec.PersonID, person);
		info.Printf(buf, rec.Code, (const char*)person, pct_dis);
		setStaticText(CTL_INFKIOSK_INFO, info);
		if(St.GoodsID)
			if(GObj.CheckFlag(St.GoodsID, GF_NODISCOUNT))
				getCtrlData(CTL_INFKIOSK_PRICE, &dscnt_price);
			else {
				RetailExtrItem item;
				RetailPriceExtractor rpe(Rec.LocID, 0, 0, ZERODATETIME, RTLPF_USEQUOTWTIME);
				if(ser_pack.Rec.QuotKindID_s)
					item.QuotList.Add(ser_pack.Rec.QuotKindID_s, 0);
				THROW(rpe.GetPrice(St.GoodsID, 0, 0.0, &item));
				dscnt_price = item.QuotList.Get(ser_pack.Rec.QuotKindID_s);
				SETIFZ(dscnt_price, (item.Price - fdiv100r(item.Price * pct_dis)));
			}
	}
	else {
		PPLoadText(PPTXT_INPSCARDNUMBER, buf);
		setStaticText(CTL_INFKIOSK_INFO, buf);
		Flags |= fWaitOnSCard;
	}
	CATCH
		PPGetMessage(mfError, PPErrCode, DS.GetTLA().AddedMsgString, 0, buf);
		ok = (setStaticText(CTL_INFKIOSK_INFO, buf), 0);
	ENDCATCH
	setCtrlReal(CTL_INFKIOSK_DSCNTPRICE, dscnt_price);
	ClearInput();
	return ok;
}

int InfoKioskDialog::SelectGoods(SearchParam srch)
{
	int    ok = -1;
	PPID   goods_id = 0, ggrp_id = 0;
	double qtty = 0.0;
	SString  buf;
	ExtGoodsSelDialog * dlg = 0;
	if(GetInput() || srch == srchByNone) {
		PPSetAddedMsgString(Input);
		if(oneof2(srch, srchByBarcode, srchBySerial)) {
			Goods2Tbl::Rec grec;
			THROW_PP(GObj.GetGoodsByBarcode(Input, 0, &grec, &qtty, 0) > 0, PPERR_BARCODENFOUND);
			goods_id = grec.ID;
			ggrp_id  = grec.ParentID;
		}
		else if(oneof3(srch, srchByName, srchByPrice, srchByNone)) {
			TIDlgInitData tidi;
			THROW(CheckDialogPtr(&(dlg = new ExtGoodsSelDialog(0)), 0));
			if(srch == srchByName) {
				SString pattern;
				StrAssocArray goods_list;
				if(Input.Len() >= INSTVSRCH_THRESHOLD)
					pattern.CatChar('!').Cat(Input);
				else
					pattern = Input;
				THROW(GObj.P_Tbl->GetListBySubstring(pattern, &goods_list, -1, 1));
				dlg->setSelectionByGoodsList(&goods_list);
			}
			else if(srch == srchByPrice)
				dlg->setSelectionByPrice(Input.ToReal());
			else {
				tidi.GoodsGrpID = SelGoodsGrpID;
				tidi.GoodsID    = St.GoodsID;
				dlg->setDTS(&tidi);
			}
			while(ExecView(dlg) == cmOK) {
				if(dlg->getDTS(&tidi) > 0) {
					goods_id = tidi.GoodsID;
					ggrp_id  = tidi.GoodsGrpID;
					break;
				}
			}
		}
	}
	SelGoodsGrpID = ggrp_id;
	THROW(ok = SetupGoods(goods_id, qtty));
	CATCH
		PPGetMessage(mfError, PPErrCode, DS.GetTLA().AddedMsgString, 0, buf);
		ok = (setStaticText(CTL_INFKIOSK_INFO, buf), 0);
		SetupGoods(0, 0.0);
	ENDCATCH
	delete dlg;
	ClearInput();
	setCtrlReal(CTL_INFKIOSK_DSCNTPRICE, 0.0);
	if(ok > 0)
		setStaticText(CTL_INFKIOSK_INFO, Input);
	return ok;
}

int InfoKioskDialog::SetupGoods(PPID goodsID, double qtty)
{
	int    ok = -1;
	SString image, word, code, buf;
	SString line_buf;
	PPGoodsPacket pack;
	St.Reset();
	if(goodsID) {
		int    r = 0;
		THROW(r = GObj.GetPacket(goodsID, &pack, 0));
		if(r > 0) {
			CCheckCore cchk;
			RetailExtrItem item;
			RetailPriceExtractor rpe(Rec.LocID, 0, 0, ZERODATETIME, RTLPF_USEQUOTWTIME);
			THROW(rpe.GetPrice(goodsID, 0, 0.0, &item));
			pack.LinkFiles.Init(PPOBJ_GOODS);
			pack.LinkFiles.Load(goodsID, 0L);
			pack.LinkFiles.At(0, image);
			pack.Codes.GetSingle(code);
			cchk.CalcGoodsRest(goodsID, LConfig.OperDate, Rec.LocID, &St.Rest);
			line_buf = pack.ExtString;
			St.GoodsID = goodsID;
			St.Qtty = qtty;
			St.Price = item.Price;
			ok = 1;
		}
	}
	SetupInfo();
	setCtrlString(CTL_INFKIOSK_IMAGE,  image);
	setCtrlString(CTL_INFKIOSK_CODE,   code);
	setCtrlData(CTL_INFKIOSK_GOODS,    pack.Rec.Name);
	setCtrlReal(CTL_INFKIOSK_PRICE,    St.Price);
	setCtrlReal(CTL_INFKIOSK_DSCNTPRICE, 0.0);

	PPGetExtStrData(GDSEXSTR_STORAGE, line_buf, buf);
	setStaticText(CTL_INFKIOSK_ADDINF1, buf);
	PPGetExtStrData(GDSEXSTR_STANDARD, line_buf, buf);
	setStaticText(CTL_INFKIOSK_ADDINF2, buf);
	PPGetExtStrData(GDSEXSTR_INGRED, line_buf, buf);
	setStaticText(CTL_INFKIOSK_ADDINF3, buf);
	PPGetExtStrData(GDSEXSTR_ENERGY, line_buf, buf);
	setStaticText(CTL_INFKIOSK_ADDINF4, buf);
	PPGetExtStrData(GDSEXSTR_USAGE, line_buf, buf);
	setStaticText(CTL_INFKIOSK_ADDINF5, buf);

	SetupLots(goodsID);
	CATCHZOK
	return ok;
}

int InfoKioskDialog::SetupLots(PPID goodsID)
{
	int    show_lots = BIN(Rec.Flags & GIF_SHOWLOTS);
	if(show_lots) {
		SmartListBox * p_list = (SmartListBox*)getCtrlView(CTL_INFKIOSK_LOTS);
		if(p_list) {
			p_list->freeAll();
			if(goodsID) {
				uint   i = 0, lots_count = 5;
				long   lots = 0;
				long   oprno = MAXLONG;
				ReceiptTbl::Rec lot;
				for(LDATE dt = LConfig.OperDate; i < (uint)lots_count &&
					BillObj->trfr->Rcpt.EnumLastLots(goodsID, Rec.LocID, &dt, &oprno, &lot) > 0; i++) {
					char sub[64];
					QualityCertTbl::Rec qc_rec;
					StringSet ss(SLBColumnDelim);
					MEMSZERO(qc_rec);
					ss.add(datefmt(&lot.Dt, DATF_DMY, sub));
					ss.add(datefmt(&lot.Expiry, DATF_DMY, sub));
					QCObj.Search(lot.QCertID, &qc_rec);
					ss.add(qc_rec.Code);
					if(!p_list->addItem(i + 1, ss.getBuf())) {
						PPError(PPERR_SLIB, 0);
						break;
					}
				}
			}
			p_list->focusItem(0);
			p_list->drawView();
		}
	}
	showCtrl(CTL_INFKIOSK_LOTS, show_lots);
	return 1;
}

int InfoKioskDialog::PrintLabel()
{
	RetailGoodsInfo rgi;
	rgi.Qtty = St.Qtty;
	if(GObj.GetRetailGoodsInfo(St.GoodsID, 0, 0, 0, 0.0, &rgi, PPObjGoods::rgifConcatQttyToCode) > 0)
		BarcodeLabelPrinter::PrintGoodsLabel(&rgi, Rec.LabelPrinterID, 1);
	return 1;
}

IMPLEMENT_PPFILT_FACTORY(InfoKioskPane); InfoKioskPaneFilt::InfoKioskPaneFilt(): PPBaseFilt(PPFILT_INFOKIOSKPANE, 0, 0)
{
	SetFlatChunk(offsetof(InfoKioskPaneFilt, ReserveStart),
		offsetof(InfoKioskPaneFilt, ReserveEnd) - offsetof(InfoKioskPaneFilt, ReserveStart) + sizeof(ReserveEnd));
	Init(1, 0);
}

int SLAPI PPObjGoodsInfo::EditInfoKioskPaneFilt(InfoKioskPaneFilt * pData)
{
	int    ok = -1;
	InfoKioskPaneFilt filt;
	RVALUEPTR(filt, pData);
	TDialog * dlg = new TDialog(DLG_INFOKIOSKFLT);
	THROW(CheckDialogPtr(&dlg, 0));
	SetupPPObjCombo(dlg, CTLSEL_INFOKIOSKFLT_NODE, PPOBJ_GOODSINFO, filt.InfoKioskID, OLW_CANINSERT, 0);
	SetupPPObjCombo(dlg, CTLSEL_INFOKIOSKFLT_GRP, PPOBJ_GOODSGROUP, filt.DefaultGrpID, OLW_CANSELUPLEVEL, 0);
	while(ok < 0 && ExecView(dlg) == cmOK) {
		uint   sel = 0;
		filt.InfoKioskID = dlg->getCtrlLong(sel = CTLSEL_INFOKIOSKFLT_NODE);
		if(!filt.InfoKioskID)
			PPErrorByDialog(dlg, sel, PPERR_INFOKIOSKNEEDED);
		else {
			filt.DefaultGrpID = dlg->getCtrlLong(CTLSEL_INFOKIOSKFLT_GRP);
			ASSIGN_PTR(pData, filt);
			ok = 1;
		}
	}
	CATCHZOKPPERR
	delete dlg;
	return ok;
}

int SLAPI ViewGoodsInfo(const InfoKioskPaneFilt * pFilt)
{
	int    ok = -1;
	PPID   goods_info_id = 0;
	PPObjGoodsInfo gi_obj;
	if(pFilt && pFilt->InfoKioskID)
		goods_info_id = pFilt->InfoKioskID;
	else {
		PPObjGoodsInfo gi_obj;
		goods_info_id = gi_obj.GetSingle();
		if(!goods_info_id)
			ListBoxSelDialog(PPOBJ_GOODSINFO, &goods_info_id, 0);
	}
	if(goods_info_id) {
		int    r = 0;
		PPGoodsInfo rec;
		THROW(r = gi_obj.GetPacket(goods_info_id, &rec));
		if(r > 0) {
			THROW(CheckExecAndDestroyDialog(new InfoKioskDialog(&rec, (pFilt ? pFilt->DefaultGrpID : 0)), 0, 0));
			ok = 1;
		}
	}
	CATCHZOKPPERR
	return ok;
}
//
// ��������� �������� �����
//
class PrcssrCCheckGenerator {
public:
	struct Param {
		Param()
		{
			SCardPeriod = 5;
			P_Pan = 0;
			MaxCc = 0;
			MaxTime = 0;
			LineDelay = 100;
			MaxCheckDelay = 5;
		}
		uint   SCardPeriod;      // ��������������� ������ �����, ������� ����������� � ����������� �������
			// ���� SCardPeriod = 5, �� �������� ������ ����� ��� ����� ������ �� �����.
		uint   MaxCc;            // ������������ ���������� ������������ �����. 0 - �� ����������. def=0
		long   MaxTime;          // ������������� ����� ��������� ����� (���). 0 - �� ����������. def=0
		uint   LineDelay;        // �������� ����� �������� ���� (��������). def=100
		uint   MaxCheckDelay;    // ������������ �������� ����� ������ (���). ����������� ��������
			// �������� ����������� ��������� ��������� [0..MaxCheckDelay]. def=5
		CheckPaneDialog * P_Pan; // @notowned
	};
	PrcssrCCheckGenerator();
	~PrcssrCCheckGenerator();
	int    Init(Param * pParam);
	int    Run();
private:
	Param P;
	PPObjGoods GObj;
	PPObjSCard ScObj;
	PPIDArray GoodsList;
	StrAssocArray * P_ScList;
	SRng * P_RngGoods;  // ��������� �������
	SRng * P_RngSCard;  // ��������� ���������� ����
	SRng * P_RngDelay;  // ��������� �������� ����� ������ (���)
	SRng * P_RngQtty;   // ��������� ���������� � ������ ����
	SRng * P_RngCount;  // ��������� ���������� ����� � ����
};

PrcssrCCheckGenerator::PrcssrCCheckGenerator()
{
	P_ScList = 0;
	P_RngGoods = 0;
	P_RngSCard = 0;
	P_RngDelay = 0;
	P_RngQtty = 0;
	P_RngCount = 0;
}

PrcssrCCheckGenerator::~PrcssrCCheckGenerator()
{
	delete P_ScList;
	delete P_RngGoods;
	delete P_RngSCard;
	delete P_RngDelay;
	delete P_RngQtty;
	delete P_RngCount;
}

int PrcssrCCheckGenerator::Init(Param * pParam)
{
	int    ok = 1;
	GoodsList.clear();
	ZDELETE(P_ScList);
	ZDELETE(P_RngGoods);
	ZDELETE(P_RngSCard);
	ZDELETE(P_RngDelay);
	ZDELETE(P_RngQtty);
	ZDELETE(P_RngCount);
	P = *pParam;
	THROW_PP(P.P_Pan, PPERR_INVPARAM);
	{
		PPIniFile ini_file;
		int    enbl = 0;
		ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_ENABLECCHECKGENERATOR, &enbl);
		THROW_PP(enbl, PPERR_CCHECKGENDISABLED);
	}
	PPWaitMsg(PPSTR_TEXT, PPTXT_CCGENINIT, 0);
	{
		const  int  dont_sel_passive = BIN(GObj.GetConfig().Flags & GCF_DONTSELPASSIVE);
		GoodsFilt gf;
		if(dont_sel_passive)
			gf.Flags |= GoodsFilt::fHidePassive;
		THROW(GoodsIterator::GetListByFilt(&gf, &GoodsList));
		THROW(GoodsList.getCount());
	}
	//
	//
	//
	{
		THROW(P_ScList = ScObj.MakeStrAssocList(0));
	}
	{
		LTIME ct = getcurtime_();
		THROW(P_RngGoods = SRng::CreateInstance(SRng::algMT, 0));
		P_RngGoods->Set(ct.v);
		THROW(P_RngSCard = SRng::CreateInstance(SRng::algMT, 0));
		P_RngSCard->Set(ct.v + 17);
		THROW(P_RngDelay = SRng::CreateInstance(SRng::algMT, 0));
		P_RngDelay->Set(ct.v + 23);
		THROW(P_RngQtty  = SRng::CreateInstance(SRng::algMT, 0));
		P_RngQtty->Set(ct.v + 37);
		THROW(P_RngCount = SRng::CreateInstance(SRng::algMT, 0));
		P_RngCount->Set(ct.v + 71);
	}
	CATCHZOK
	return ok;
}

int SLAPI PrcssrCCheckGenerator::Run()
{
	int    ok = 1;
	ulong  cc_count = 0; // ���������� ��������������� �����
	const  uint sc_count = P_ScList ? P_ScList->getCount() : 0;
	SString sc_code, temp_buf;
	//
	//
	//
	LDATETIME tm_start = getcurdatetime_();
	LDATETIME tm_cur = getcurdatetime_();
	LDATETIME tm_limit = tm_start;
	tm_limit.addsec(P.MaxTime);
	PPWait(1);
	P.P_Pan->EnableBeep(0);
	while((!P.MaxCc || cc_count < P.MaxCc) && (!P.MaxTime || cmp(tm_cur, tm_limit) < 0) && PPCheckUserBreak()) {
		uint   cl_count = (uint)fabs(P_RngCount->GetGaussian(3.0) + 10.0);
		for(uint i = 0; i < cl_count;) {
			uint goods_pos = P_RngCount->GetUniformInt(GoodsList.getCount());
			if(goods_pos < GoodsList.getCount()) {
				PPID   goods_id = GoodsList.get(goods_pos);
				Goods2Tbl::Rec goods_rec;
				if(GObj.Fetch(goods_id, &goods_rec) > 0) {
					double qtty = fabs(P_RngQtty->GetGaussian(3.0) + 1.0);
					CPosProcessor::PgsBlock pgsb(round(qtty, (goods_rec.Flags & GF_INTVAL) ? 0 : 3));
					if(pgsb.Qtty > 0.0 && P.P_Pan->SetupNewRow(goods_id, /*qtty, 0, 0*/pgsb) > 0) {
						if(P.LineDelay)
							delay(P.LineDelay);
						i++;
					}
				}
			}
		}
		P.P_Pan->AcceptRow();
		if(sc_count) {
			uint sc_pos = P_RngSCard->GetUniformInt(sc_count * P.SCardPeriod);
			if(sc_pos < sc_count) {
				sc_code = P_ScList->at(sc_pos).Txt;
				if(sc_code.NotEmptyS())
					P.P_Pan->SetSCard(P_ScList->at(sc_pos).Txt);
			}
		}
		{
			double total = 0.0;
			double discount = 0.0;
			P.P_Pan->CalcTotal(&total, &discount);
			//
			// ����� ������ ������� � ���������� ����
			//
			CcAmountList pl;
			if(cc_count%20 == 0) {
				pl.Add(CCAMTTYP_BANK, total);
			}
			else {
				pl.Add(CCAMTTYP_CASH, total);
			}
			P.P_Pan->AcceptCheck(&pl, total, CPosProcessor::accmRegular);
			P.P_Pan->ClearCheck();
		}
		if(P.MaxCheckDelay) {
			uint t = P_RngDelay->GetUniformInt(P.MaxCheckDelay);
			(temp_buf = 0).Cat(t);
			PPWaitMsg(PPSTR_TEXT, PPTXT_CCGENCHECKDELAY, temp_buf);
			delay(t * 1000);
		}
		cc_count++;
		tm_cur = getcurdatetime_();
		PPWaitMsg((temp_buf = 0).Cat(cc_count));
	}
	P.P_Pan->EnableBeep(1);
	PPWait(0);
	return ok;
}

int CheckPaneDialog::GenerateChecks()
{
	int	   ok = 1;
	PrcssrCCheckGenerator generator;
	PrcssrCCheckGenerator::Param param;
	param.P_Pan = this;
	param.SCardPeriod = 2;
	param.MaxCc = 3; // @vmiller
	PPWait(1);
	THROW(generator.Init(&param));
	THROW(generator.Run());
	PPWait(0);
	CATCHZOKPPERR
	return ok;
}