// EGAIS.CPP
// Copyright (c) A.Sobolev 2015, 2016
//
// ��������� �������� ��� ������ � �������� EGAIS
//
#include <pp.h>
#pragma hdrstop

/*
#define BEDIUS_DESADV_OUT_SENDED            1 // �������� ��������� ���������� � ���� DESADV
#define BEDIUS_DESADV_OUT_PROCESSED         2 // ��������, ������������ ���������� � ���� DESADV, ������ EDI-��������
#define BEDIUS_DESADV_OUT_RECADV_ACC        3 // �� ��������, ������������ ���������� � ���� DESADV, ������� RECADV "������"
#define BEDIUS_DESADV_OUT_RECADV_PACC       4 // �� ��������, ������������ ���������� � ���� DESADV, ������� RECADV "�������� ������"
#define BEDIUS_DESADV_OUT_RECADV_REJ        5 // �� ��������, ������������ ���������� � ���� DESADV, ������� RECADV "��������"
#define BEDIUS_DESADV_OUT_RECADV_CONF_ACC   6 // � ����� �� RECADV ���������� ������������� "�������"
#define BEDIUS_DESADV_OUT_RECADV_CONF_REJ   7 // � ����� �� RECADV ���������� ������������� "���������"

#define BEDIUS_DESADV_IN_ACCEPTED           8 // �������� ������� �� ����������� � ���� DESADV
#define BEDIUS_DESADV_IN_RECADV_ACC         9 // �� ��������, ���������� �� ����������� � ���� DESADV, ��������� RECADV "������"
#define BEDIUS_DESADV_IN_RECADV_PACC       10 // �� ��������, ���������� �� ����������� � ���� DESADV, ��������� RECADV "�������� ������"
#define BEDIUS_DESADV_IN_RECADV_REJ        11 // �� ��������, ���������� �� ����������� � ���� DESADV, ��������� RECADV "��������"
*/

/*
BILL_EDI_USER_STATE

���������������� ��������� ������������ ����������:

- ��������� � ���                        yellow    ��� PPTAG_BILL_EDIACK
- ������� ��������� �����                orange    ���� PPTAG_BILL_EDIACK � PPTAG_BILL_EDIIDENT
- �������� ������������� �� ����������
	-- ��������� ������                  green          ��� PPTAG_BILL_EDIIDENT � BillCore::GetRecadvStatus() == PPEDI_RECADV_STATUS_ACCEPT
	-- �������� ������                   light green    ��� PPTAG_BILL_EDIIDENT � BillCore::GetRecadvStatus() == PPEDI_RECADV_STATUS_PARTACCEPT
	-- �����                             brown          ��� PPTAG_BILL_EDIIDENT � BillCore::GetRecadvStatus() == PPEDI_RECADV_STATUS_REJECT
- ���������� ������������� ����������
	-- �������                           blue           ��� PPTAG_BILL_EDIIDENT � BillCore::GetRecadvStatus() != PPEDI_RECADV_STATUS_UNDEF �
															BillCore::GetRecadvConfStatus() == PPEDI_RECADVCONF_STATUS_ACCEPT
	-- ��������                          grey           ��� PPTAG_BILL_EDIIDENT � BillCore::GetRecadvStatus() != PPEDI_RECADV_STATUS_UNDEF �
															BillCore::GetRecadvConfStatus() == PPEDI_RECADVCONF_STATUS_REJECT

���������������� ��������� �������� ����������:
- ������ �� �����                        orange      EdiOp == PPEDIOP_EGAIS_WAYBILL
- ��������� RECADV
	-- ��������� ������                  green       EdiOp == PPEDIOP_EGAIS_WAYBILL && BillCore::GetRecadvConfStatus() == PPEDI_RECADVCONF_STATUS_ACCEPT
	-- �������� ������                   light green EdiOp == PPEDIOP_EGAIS_WAYBILL && BillCore::GetRecadvConfStatus() == PPEDI_RECADVCONF_STATUS_PARTACCEPT
	-- �����                             brown       EdiOp == PPEDIOP_EGAIS_WAYBILL && BillCore::GetRecadvConfStatus() == PPEDI_RECADVCONF_STATUS_REJECT
*/


static const char * P_TempOutputDirName = "temp-query";
static const char * P_IntrExpndNotePrefix = "$INTREXPND";

PPEgaisProcessor::Ack::Ack()
{
	Clear();
}

void PPEgaisProcessor::Ack::Clear()
{
	Ver = 0;
	SignSize = 0;
	Sign[0] = 0;
	Id.SetZero();
	Status = 0;
	Url = 0;
	Message = 0;
}

PPEgaisProcessor::Reply::Reply()
{
	Status = 0;
	Id.SetZero();
}

PPEgaisProcessor::Ticket::Result::Result()
{
	Clear();
}

void PPEgaisProcessor::Ticket::Result::Clear()
{
    Conclusion = -1;
    Time.SetZero();
    OpName = 0;
    Comment = 0;
}

PPEgaisProcessor::Ticket::Ticket()
{
	Clear();
}

void PPEgaisProcessor::Ticket::Clear()
{
	TicketTime.SetZero();
	DocUUID.SetZero();
	TranspUUID.SetZero();
	DocType = 0;
	RegIdent = 0;
	R.Clear();
	OpR.Clear();
}

PPEgaisProcessor::ConfirmTicket::ConfirmTicket()
{
	BillID = 0;
	Conclusion = -1;
	Date = ZERODATE;
}

SLAPI PPEgaisProcessor::InformAReg::InformAReg()
{
	Qtty = 0;
	ManufDate = ZERODATE;
	TTNDate = ZERODATE;
	EGAISDate = ZERODATE;
}

int SLAPI PPEgaisProcessor::InformAReg::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pSCtx)
{
	int    ok = 1;
	THROW_SL(pSCtx->Serialize(dir, Qtty, rBuf));
	THROW_SL(pSCtx->Serialize(dir, ManufDate, rBuf));
	THROW_SL(pSCtx->Serialize(dir, TTNDate, rBuf));
	THROW_SL(pSCtx->Serialize(dir, TTNCode, rBuf));
	THROW_SL(pSCtx->Serialize(dir, EGAISDate, rBuf));
	THROW_SL(pSCtx->Serialize(dir, EGAISCode, rBuf));
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::InformAReg::ToStr(SString & rBuf)
{
	int    ok = 1;
	rBuf = 0;
	SSerializeContext sctx;
	SBuffer sbuf;
	THROW(Serialize(+1, sbuf, &sctx));
	rBuf.EncodeMime64(sbuf.GetBuf(sbuf.GetRdOffs()), sbuf.GetAvailableSize());
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::InformAReg::FromStr(const SString & rBuf)
{
	int    ok = 1;
	uint8  mime_buf[512];
	size_t binary_size = 0;
	SSerializeContext sctx;
	SBuffer sbuf;
	THROW_SL(rBuf.DecodeMime64(mime_buf, sizeof(mime_buf), &binary_size));
	THROW_SL(sbuf.Write(mime_buf, binary_size));
	THROW(Serialize(-1, sbuf, &sctx));
	CATCHZOK
	return ok;
}

PPEgaisProcessor::InformBItem::InformBItem()
{
	Clear();
}

void PPEgaisProcessor::InformBItem::Clear()
{
	THISZERO();
}

PPEgaisProcessor::InformB::InformB()
{
	Clear();
}

void PPEgaisProcessor::InformB::Clear()
{
	Id = 0;
	WBRegId = 0;
	FixNumber = 0;
	FixDate = ZERODATE;
	OuterCode = 0;
	OuterDate = ZERODATE;
	ShipperPsnID = 0;
	ConsigneePsnID = 0;
	SupplPsnID = 0;
	Items.clear();
}

PPEgaisProcessor::ActInformItem::ActInformItem()
{
	P = 0;
	AIdent[0] = 0;
}

PPEgaisProcessor::ActInform::ActInform()
{
}

PPEgaisProcessor::RepealWb::RepealWb()
{
	BillID = 0;
	Confirm = 0;
}

PPEgaisProcessor::QueryBarcode::QueryBarcode()
{
	RowId = 0;
	CodeType = 0;
}

SLAPI PPEgaisProcessor::SendBillsParam::SendBillsParam()
{
	LocID = 0;
	Period.SetZero();
}

PPEgaisProcessor::Packet::Packet(int docType)
{
	DocType = docType;
	Flags = 0;
	IntrBillID = 0;
	P_Data = 0;
	SrcReplyPos = 0;
	switch(DocType) {
		case PPEDIOP_EGAIS_QUERYCLIENTS:
		case PPEDIOP_EGAIS_QUERYAP:
			P_Data = new StrStrAssocArray;
			break;
		case PPEDIOP_EGAIS_QUERYFORMA:
		case PPEDIOP_EGAIS_QUERYFORMB:
			P_Data = new SString;
			break;
		case PPEDIOP_EGAIS_TICKET:
			P_Data = new Ticket;
			break;
		case PPEDIOP_EGAIS_TTNINFORMBREG:
			P_Data = new InformB;
			break;
		case PPEDIOP_EGAIS_REPLYCLIENT:
			P_Data = new TSCollection <PPPersonPacket>;
			break;
		case PPEDIOP_EGAIS_REPLYAP:
			P_Data = new TSCollection <PPGoodsPacket>;
			break;
		case PPEDIOP_EGAIS_REPLYFORMA:
			P_Data = new EgaisRefATbl::Rec;
			memzero(P_Data, sizeof(EgaisRefATbl::Rec));
			break;
		case PPEDIOP_EGAIS_WAYBILL:
		case PPEDIOP_EGAIS_WAYBILLACT:
		case PPEDIOP_EGAIS_ACTCHARGEON:
		case PPEDIOP_EGAIS_ACTCHARGEON_V2: // @v9.3.12
		case PPEDIOP_EGAIS_ACTCHARGEONSHOP: // @v9.2.11
		case PPEDIOP_EGAIS_REPLYRESTS:
		case PPEDIOP_EGAIS_REPLYRESTSSHOP:
		case PPEDIOP_EGAIS_ACTWRITEOFF:
		case PPEDIOP_EGAIS_ACTWRITEOFF_V2: // @v9.3.12
		case PPEDIOP_EGAIS_ACTWRITEOFFSHOP: // @v9.4.0
		case PPEDIOP_EGAIS_TRANSFERTOSHOP:
		case PPEDIOP_EGAIS_TRANSFERFROMSHOP: // @v9.3.10
			P_Data = new PPBillPacket;
			break;
		case PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG:
			P_Data = new ActInform;
			break;
		case PPEDIOP_EGAIS_CONFIRMTICKET:
			P_Data = new ConfirmTicket();
			break;
		case PPEDIOP_EGAIS_REQUESTREPEALWB:
		case PPEDIOP_EGAIS_CONFIRMREPEALWB:
			P_Data = new RepealWb();
			break;
        case PPEDIOP_EGAIS_QUERYBARCODE:
		case PPEDIOP_EGAIS_REPLYBARCODE:
			P_Data = new TSCollection <QueryBarcode>;
			break;
	}
}

PPEgaisProcessor::Packet::~Packet()
{
	switch(DocType) {
		case PPEDIOP_EGAIS_QUERYCLIENTS:
		case PPEDIOP_EGAIS_QUERYAP:
			delete ((StrStrAssocArray *)P_Data);
			break;
		case PPEDIOP_EGAIS_QUERYFORMA:
		case PPEDIOP_EGAIS_QUERYFORMB:
			delete ((SString *)P_Data);
			break;
		case PPEDIOP_EGAIS_TICKET:
			delete ((Ticket *)P_Data);
			break;
		case PPEDIOP_EGAIS_TTNINFORMBREG:
			delete ((InformB *)P_Data);
			break;
		case PPEDIOP_EGAIS_REPLYCLIENT:
			delete ((TSCollection <PPPersonPacket> *)P_Data);
			break;
		case PPEDIOP_EGAIS_REPLYAP:
			delete ((TSCollection <PPGoodsPacket> *)P_Data);
			break;
		case PPEDIOP_EGAIS_REPLYFORMA:
			delete ((EgaisRefATbl::Rec *)P_Data);
			break;
		case PPEDIOP_EGAIS_WAYBILL:
		case PPEDIOP_EGAIS_WAYBILLACT:
		case PPEDIOP_EGAIS_ACTCHARGEON:
		case PPEDIOP_EGAIS_ACTCHARGEON_V2: // @v9.3.12
		case PPEDIOP_EGAIS_ACTCHARGEONSHOP: // @v9.2.11
		case PPEDIOP_EGAIS_REPLYRESTS:
		case PPEDIOP_EGAIS_REPLYRESTSSHOP:
		case PPEDIOP_EGAIS_ACTWRITEOFF:
		case PPEDIOP_EGAIS_ACTWRITEOFF_V2: // @v9.3.12
		case PPEDIOP_EGAIS_ACTWRITEOFFSHOP: // @v9.4.0
		case PPEDIOP_EGAIS_TRANSFERTOSHOP:
		case PPEDIOP_EGAIS_TRANSFERFROMSHOP: // @v9.3.10
			delete ((PPBillPacket *)P_Data);
			break;
		case PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG:
			delete ((ActInform *)P_Data);
			break;
		case PPEDIOP_EGAIS_CONFIRMTICKET:
			delete ((ConfirmTicket *)P_Data);
			break;
		case PPEDIOP_EGAIS_REQUESTREPEALWB:
		case PPEDIOP_EGAIS_CONFIRMREPEALWB:
			delete ((RepealWb *)P_Data);
			break;
        case PPEDIOP_EGAIS_QUERYBARCODE:
		case PPEDIOP_EGAIS_REPLYBARCODE:
			delete ((TSCollection <QueryBarcode> *)P_Data);
			break;
	}
}

int PPEgaisProcessor::LogSended(const Packet & rPack)
{
	int    ok = -1;
	SString msg_buf, temp_buf;
	switch(rPack.DocType) {
		case PPEDIOP_EGAIS_QUERYCLIENTS:
		case PPEDIOP_EGAIS_QUERYAP:
		case PPEDIOP_EGAIS_QUERYRESTS:
			PPLoadText(PPTXT_EGAIS_QS_QUERY, msg_buf);
			msg_buf.CatDiv(':', 2).Cat(PPEgaisProcessor::GetDocTypeTag(rPack.DocType));
			if(rPack.P_Data) {
				const StrStrAssocArray * p_list = (const StrStrAssocArray *)rPack.P_Data;
				if(p_list->getCount()) {
					msg_buf.CatDiv('-', 2);
					for(uint i = 0; i < p_list->getCount(); i++) {
						StrStrAssocArray::Item param = p_list->at(i);
						msg_buf.Space().CatChar('[').Cat(param.Key).CatDiv(';', 2).Cat(param.Val).CatChar(']');
					}
				}
			}
			Log(msg_buf);
			ok = 1;
			break;
		case PPEDIOP_EGAIS_WAYBILL:
			if(msg_buf.Empty())
				PPLoadText(PPTXT_EGAIS_QS_WAYBILL, msg_buf);
			// @nobreak
		case PPEDIOP_EGAIS_WAYBILLACT:
			if(msg_buf.Empty())
				PPLoadText(PPTXT_EGAIS_QS_WAYBILLACT, msg_buf);
			// @nobreak
		case PPEDIOP_EGAIS_ACTCHARGEON:
		case PPEDIOP_EGAIS_ACTCHARGEON_V2:
			if(msg_buf.Empty())
				PPLoadText(PPTXT_EGAIS_QS_CHARGEON, msg_buf);
			{
				const PPBillPacket * p_pack = (const PPBillPacket *)rPack.P_Data;
				if(p_pack) {
					PPObjBill::MakeCodeString(&p_pack->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, temp_buf);
					msg_buf.CatDiv(':', 2).Cat(temp_buf);
				}
			}
			Log(msg_buf);
			ok = 1;
			break;
		case PPEDIOP_EGAIS_CONFIRMTICKET:
			PPLoadText(PPTXT_EGAIS_QS_CFMTICKET, msg_buf);
			{
				const ConfirmTicket * p_ct = (const ConfirmTicket *)rPack.P_Data;
				if(p_ct) {
					BillTbl::Rec bill_rec;
					if(p_ct->BillID && BillObj->Search(p_ct->BillID, &bill_rec) > 0) {
						PPObjBill::MakeCodeString(&bill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, temp_buf);
						msg_buf.CatDiv(':', 2).Cat(temp_buf);
					}
					msg_buf.CatDiv('-', 1);
					if(p_ct->Conclusion == 1)
						msg_buf.Cat("Accepted");
					else if(p_ct->Conclusion == 0)
						msg_buf.Cat("Rejected");
					else
						msg_buf.Cat("Conclusion undefined!");
				}
			}
			Log(msg_buf);
			ok = 1;
			break;
	}
	return ok;
}

int SLAPI PPEgaisProcessor::GetReplyList(void * pCtx, PPID locID, int direction /* +1 out, -1 - in */, TSCollection <PPEgaisProcessor::Reply> & rList)
{
    int    ok = 1;
    xmlParserCtxt * p_ctx = (xmlParserCtxt *)pCtx;
	xmlDoc * p_doc = 0;
	xmlNode * p_root = 0;
	ScURL  c;
	SString url;
	SBuffer ack_buf;
	THROW(GetURL(locID, url));
	url.RmvLastSlash().CatChar('/').Cat("opt").CatChar('/').Cat((direction > 0) ? "out" : "in");
	{
		SFile wr_stream(ack_buf, SFile::mWrite);
		THROW_SL(c.HttpGet(url, 0, &wr_stream));
		{
			SBuffer * p_ack_buf = (SBuffer *)wr_stream;
			if(p_ack_buf) {
				SString temp_buf;
				const int avl_size = (int)p_ack_buf->GetAvailableSize();
				THROW_LXML((p_doc = xmlCtxtReadMemory(p_ctx, (const char *)p_ack_buf->GetBuf(), avl_size, 0, 0, XML_PARSE_NOENT)), p_ctx);
				THROW(p_root = xmlDocGetRootElement(p_doc));
				if(SXml::IsName(p_root, "A")) {
					THROW(!SXml::IsContent(p_root, "error"))
					for(xmlNode * p_c = p_root->children; p_c; p_c = p_c->next) {
						if(SXml::IsName(p_c, "url")) {
							Reply * p_new_reply = new Reply;
							THROW_MEM(p_new_reply);
							SXml::GetContent(p_c, temp_buf);
							p_new_reply->Url = temp_buf;
							if(SXml::GetAttrib(p_c, "replyId", temp_buf) && temp_buf.NotEmptyS())
								p_new_reply->Id.FromStr(temp_buf);
							THROW_SL(rList.insert(p_new_reply));
						}
						else if(SXml::GetContentByName(p_c, "ver", temp_buf)) {
							// ����������, ����� ����� ������ �� ����������
						}
					}
				}
			}
		}
	}
    CATCHZOK
	xmlFreeDoc(p_doc);
    return ok;
}

int SLAPI PPEgaisProcessor::GetTempFileName(const char * pPath, const char * pSubPath, const char * pPrefix, SString & rFn)
{
	int    ok = 1;
	const  int is_cc = sstreqi_ascii(pSubPath, "cc");
	rFn = 0;
	SString temp_path;
	if(!isempty(pPath)) {
		(temp_path = pPath).SetLastSlash();
		if(is_cc)
			temp_path.Cat("xml");
		else
			temp_path.Cat("opt").SetLastSlash().Cat(P_TempOutputDirName);
	}
	else {
		PPGetPath(PPPATH_TEMP, temp_path);
	}
	if(!is_cc && !isempty(pSubPath)) {
		temp_path.SetLastSlash().Cat(pSubPath);
	}
	temp_path.RmvLastSlash();
	THROW_SL(::createDir(temp_path));
	MakeTempFileName(temp_path.SetLastSlash(), pPrefix, "XML", 0, rFn);
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::ReadAck(SBuffer * pBuf, PPEgaisProcessor::Ack & rAck)
{
	rAck.Clear();
	int    ok = -1;
    xmlParserCtxt * p_ctx = 0;
	xmlDoc * p_doc = 0;
	if(pBuf) {
		const int avl_size = (int)pBuf->GetAvailableSize();
		SString temp_buf;
		xmlNode * p_root = 0;
		THROW(p_ctx = xmlNewParserCtxt());
		THROW_LXML(p_doc = xmlCtxtReadMemory(p_ctx, (const char *)pBuf->GetBuf(), avl_size, 0, 0, XML_PARSE_NOENT), p_ctx);
		THROW(p_root = xmlDocGetRootElement(p_doc));
		if(SXml::IsName(p_root, "A")) {
			for(xmlNode * p_c = p_root->children; p_c; p_c = p_c->next) {
				if(SXml::GetContentByName(p_c, "error", temp_buf)) {
					rAck.Status |= Ack::stError;
					rAck.Message = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
				}
				else if(SXml::GetContentByName(p_c, "url", temp_buf)) {
					rAck.Url = temp_buf; // @v9.1.8
					if(rAck.Id.FromStr(temp_buf))
						rAck.Status &= ~Ack::stError;
				}
				else if(SXml::GetContentByName(p_c, "sign", temp_buf)) {
					strnzcpy((char *)rAck.Sign, temp_buf, sizeof(rAck.Sign));
					rAck.SignSize = (uint8)temp_buf.Len();
				}
				else if(SXml::GetContentByName(p_c, "ver", temp_buf)) {
					rAck.Ver = temp_buf.ToLong();
				}
			}
			ok = 1;
		}
	}
	CATCHZOK
	xmlFreeDoc(p_doc);
	xmlFreeParserCtxt(p_ctx);
	return ok;
}

int SLAPI PPEgaisProcessor::PutCCheck(const CCheckPacket & rPack, PPID locID, PPEgaisProcessor::Ack & rAck)
{
	const int omit_nonmarked_goods = 1;

    int    ok = -1;
    xmlParserCtxt * p_ctx = 0;
	xmlTextWriter * p_x = 0;
	xmlDoc * p_doc = 0;
	xmlNode * p_root = 0;
	SString file_name, temp_buf;
	SString temp_path;
	SString mark_buf;
	SBuffer ack_buf;
	LongArray marked_pos_list;    // ������ ������� ����, ���������� ������������� �������� (������ ������� [1..])
	LongArray nonmarked_pos_list; // ������ ������� ����, ���������� �� ������������� �������� (������ ������� [1..])
	PrcssrAlcReport::GoodsItem agi;
	SString url;
	{
		for(uint i = 0; i < rPack.GetCount(); i++) {
			const CCheckLineTbl::Rec & r_item = rPack.GetLine(i);
			if(IsAlcGoods(r_item.GoodsID)) {
				if(PreprocessGoodsItem(r_item.GoodsID, 0, 0, 0, agi) > 0) {
					if(agi.StatusFlags & agi.stMarkWanted) {
						marked_pos_list.add((long)(i+1));
						rPack.GetLineTextExt(i+1, CCheckPacket::lnextEgaisMark, mark_buf);
						THROW_PP_S(IsEgaisMark(mark_buf, 0), PPERR_TEXTISNTEGAISMARK, mark_buf);
					}
					else {
						nonmarked_pos_list.add((long)(i+1));
					}
				}
			}
		}
	}
	if(marked_pos_list.getCount() || (!omit_nonmarked_goods && nonmarked_pos_list.getCount())) {
		SString cc_text;
		SString result_barcode;
		CCheckCore::MakeCodeString(&rPack.Rec, cc_text);
		PPID   main_org_id = 0;
		SString fsrar_ident;
		PPObjCashNode cn_obj;
		PPSyncCashNode cn_pack;
        THROW(cn_obj.GetSync(rPack.Rec.CashID, &cn_pack) > 0);
        const PPID loc_id = NZOR(locID, cn_pack.LocID); // ���������� ���������� locID ����� ��������� ����� cn_pack.LocID
		THROW(GetURL(loc_id, url));
		THROW(GetDebugPath(loc_id, temp_path));
		THROW(GetTempFileName(temp_path, "cc", "EGH", file_name));
		THROW(GetFSRARID(loc_id, fsrar_ident, &main_org_id));
		THROW(p_x = xmlNewTextWriterFilename(file_name, 0));
		{
			SXml::WDoc _doc(p_x, cpUTF8);
			SString inn, kpp, org_name, org_addr;
			RegisterTbl::Rec reg_rec;
			PsnObj.GetRegNumber(main_org_id, PPREGT_TPID, rPack.Rec.Dt, inn);
			if(GetWkrRegister(wkrKPP, main_org_id, loc_id, rPack.Rec.Dt, &reg_rec) > 0)
				kpp = reg_rec.Num;
			//
			PrcssrAlcReport::EgaisMarkBlock emb;
			SXml::WNode n_doc(_doc, "Cheque");
			n_doc.PutAttrib("inn", EncText(temp_buf = inn));
			n_doc.PutAttrib("kpp", EncText(temp_buf = kpp));
			{
				org_addr = 0;
				LocationTbl::Rec loc_rec;
				PersonTbl::Rec psn_rec;
				if(loc_id && PsnObj.LocObj.Search(loc_id, &loc_rec) > 0) {
					PsnObj.LocObj.P_Tbl->GetAddress(loc_rec, 0, org_addr);
					org_name = loc_rec.Name;
				}
				if(!org_addr.NotEmptyS() || !org_name.NotEmptyS()) {
					if(PsnObj.Search(main_org_id, &psn_rec) > 0) {
						if(psn_rec.RLoc && PsnObj.LocObj.Search(psn_rec.RLoc, &loc_rec) > 0) {
							PsnObj.LocObj.P_Tbl->GetAddress(loc_rec, 0, org_addr);
						}
						if(!org_addr.NotEmptyS()) {
							if(psn_rec.MainLoc && PsnObj.LocObj.Search(psn_rec.MainLoc, &loc_rec) > 0) {
								PsnObj.LocObj.P_Tbl->GetAddress(loc_rec, 0, org_addr);
							}
						}
						if(!org_name.NotEmptyS()) {
							org_name = psn_rec.Name;
						}
					}
				}
			}
			{
				if(!org_addr.NotEmptyS())
					org_addr = "dummy address";
				n_doc.PutAttrib("address", EncText(org_addr));
			}
			{
				if(!org_name.NotEmptyS())
					org_name = "dummy org name";
				n_doc.PutAttrib("name", EncText(org_name));
			}
			{
				cn_pack.GetPropString(SCN_MANUFSERIAL, temp_buf);
				if(!temp_buf.NotEmptyS())
					temp_buf.CatLongZ(1, 6);
				n_doc.PutAttrib("kassa", EncText(temp_buf)); // ��������� ����� �����
			}
			n_doc.PutAttrib("shift",  (temp_buf = 0).Cat(rPack.Rec.SessID)); // ����� ����� @todo ��������, ����� ����� �����, � �� �������������
			n_doc.PutAttrib("number", (temp_buf = 0).Cat(rPack.Rec.Code));
			{
				LDATETIME _dtm;
				if(checkdate(rPack.Rec.Dt, 0))
					_dtm.Set(rPack.Rec.Dt, rPack.Rec.Tm);
				else
					_dtm = getcurdatetime_();
				(temp_buf = 0).Cat(_dtm.d, DATF_DMY|DATF_NODIV).CatLongZ(_dtm.t.hour(), 2).CatLongZ(_dtm.t.minut(), 2);
				n_doc.PutAttrib("datetime", temp_buf);
			}
			{
				for(uint i = 0; i < marked_pos_list.getCount(); i++) {
					uint   _pos = (uint)(marked_pos_list.get(i) - 1);
					assert(_pos >= 0 && _pos < rPack.GetCount()); // @paranoic
					if(_pos >= 0 && _pos < rPack.GetCount()) { // @paranoic
						const CCheckLineTbl::Rec & r_item = rPack.GetLine(_pos);
						if(PreprocessGoodsItem(r_item.GoodsID, 0, 0, 0, agi) > 0) {
							assert(agi.StatusFlags & agi.stMarkWanted); // @paranoic
							rPack.GetLineTextExt(_pos+1, CCheckPacket::lnextEgaisMark, mark_buf);
							assert(IsEgaisMark(mark_buf, 0)); // @paranoic
							SXml::WNode n_item(_doc, "Bottle");
							{
								double _p = intmnytodbl(r_item.Price) - r_item.Dscnt;
								// @v9.2.9 {
								if(rPack.Rec.Flags & CCHKF_RETURN)
									_p = -_p;
								// } @v9.2.9
								(temp_buf = 0).Cat(_p, MKSFMTD(0, 2, 0));
								n_item.PutAttrib("price", EncText(temp_buf));
							}
							n_item.PutAttrib("barcode", EncText(mark_buf));
							{
								GObj.GetSingleBarcode(r_item.GoodsID, temp_buf);
								result_barcode = 0;
								int    dbr = 0; // ��������� ����������� ���������
								if(temp_buf.NotEmptyS()) {
									int    diag = 0, std = 0;
									dbr = PPObjGoods::DiagBarcode(temp_buf, &diag, &std, &result_barcode);
								}
								if(dbr == 0 || !oneof4(result_barcode.Len(), 8, 12, 13, 14)) {
									result_barcode = "4600000000008"; // dummy ean
								}
								n_item.PutAttrib("ean", result_barcode);
							}
							n_item.PutAttrib("volume", EncText((temp_buf = 0).Cat(agi.Volume, MKSFMTD(0, 3, 0))));
						}
					}
				}
			}
			if(!omit_nonmarked_goods) {
				for(uint i = 0; i < nonmarked_pos_list.getCount(); i++) {
					uint   _pos = (uint)(nonmarked_pos_list.get(i) - 1);
					assert(_pos >= 0 && _pos < rPack.GetCount()); // @paranoic
					if(_pos >= 0 && _pos < rPack.GetCount()) { // @paranoic
						const CCheckLineTbl::Rec & r_item = rPack.GetLine(_pos);
						if(PreprocessGoodsItem(r_item.GoodsID, 0, 0, 0, agi) > 0) {
							assert((agi.StatusFlags & agi.stMarkWanted) == 0); // @paranoic
							SXml::WNode n_item(_doc, "nopdf");
							temp_buf = agi.CategoryCode;
							n_item.PutAttrib("code", EncText(temp_buf));
							GetGoodsName(r_item.GoodsID, temp_buf);
							n_item.PutAttrib("bname", EncText(temp_buf));
							n_item.PutAttrib("volume", EncText((temp_buf = 0).Cat(agi.Volume, MKSFMTD(0, 3, 0))));
							n_item.PutAttrib("alc", EncText((temp_buf = 0).Cat(agi.Proof, MKSFMTD(0, 1, 0))));
							{
								double _p = intmnytodbl(r_item.Price) - r_item.Dscnt;
								(temp_buf = 0).Cat(_p, MKSFMTD(0, 2, 0));
								n_item.PutAttrib("price", EncText(temp_buf));
							}
							{
								GObj.GetSingleBarcode(r_item.GoodsID, temp_buf);
								if(!oneof4(temp_buf.Len(), 8, 12, 13, 14))
									temp_buf = "4600000000008"; // dummy ean
								n_item.PutAttrib("ean", temp_buf);
							}
							(temp_buf = 0).Cat(fabs(r_item.Quantity), MKSFMTD(0, 0, 0));
							n_item.PutAttrib("count", EncText(temp_buf));
						}
					}
				}
			}
		}
		xmlFreeTextWriter(p_x);
		p_x = 0;
		{
            // ������� ����� ��������� �������� ��� '%s'
            //
            PPLoadText(PPTXT_EGAIS_QS_CCHECK, temp_buf);
            temp_buf.Space().CatChar('\'').Cat(cc_text).CatChar('\'');
            if(!(State & stTestSendingMode))
				temp_buf.Space().CatChar('\'').Cat(file_name).CatChar('\'');
            Log(temp_buf);
		}
		if(State & stTestSendingMode) {
			Log(PPLoadTextS(PPTXT_EGAIS_TESTSEDNING, temp_buf).CatDiv(':', 2).Cat(file_name));
			// 91F2257133C70DC649E903F203ED461D91AC9E3D4E363DF95A48BCA416FC374E4926C22F3E7002B135819AC5A7AEBD681CB81688B0AD2389EE1777412089AF86
			rAck.Status = 0;
			rAck.Ver = 2;
			strnzcpy((char *)rAck.Sign, "91F2257133C70DC649E903F203ED461D91AC9E3D4E363DF95A48BCA416FC374E4926C22F3E7002B135819AC5A7AEBD681CB81688B0AD2389EE1777412089AF86", sizeof(rAck.Sign));
			rAck.SignSize = (uint8)sstrlen(rAck.Sign);
			rAck.Id.Generate();
			rAck.Url = "http://check.egais.ru?id=a4a702b9-3f03-49f4-800c-e69874bf3302&dt=1906162019&cn=020000448086";
			//rAck.Url = "91F2257133C70DC649E903F203ED461D91AC9E3D4E363DF95A48BCA416FC374E4926C22F3E7002B135819AC5A7AEBD681CB81688B0AD2389EE1777412089AF86";
			rAck.Message = "EGAIS reply emulation";
		}
		else {
			ScURL c;
			SString req;
			url.RmvLastSlash().CatChar('/').Cat("xml");
			SFile wr_stream(ack_buf, SFile::mWrite);
			ScURL::HttpForm hf;
			hf.AddContentFile(file_name, "application/xml", "xml_file");
			THROW_SL(c.HttpPost(url, 0, hf, &wr_stream));
			//LogSended(rPack);
			THROW(ReadAck((SBuffer *)wr_stream, rAck));
			/*
			<?xml version="1.0" encoding="UTF-8" standalone="no"?>
			<A><url>https://146.120.90.148:1444?id=ab43d5e0-855c-4b54-b1d8-ddeb34d1d110&amp;dt=1503271510&amp;cn=00040218</url>
			<sign>32B7136A7BCEAFEBE4DA92D9510C196CC42DC(1.174)
			56260E1E2802B8C24A8C3EA75F3978EA6963F370F
			3165B89DBF98FD94F2F0C7F4803965B62A6616A12D8159A5D3</sign>
			<ver>2</ver></A>
			*/
			{
				/*
					PPTXT_CCACKOK                 "�������� ���� '@zstr' ������� ����� ����������� �������: @zstr"
					PPTXT_CCACKERROR              "�������� ���� '@zstr' ������� ����� ����������� � �������: @zstr"
				*/
				if(rAck.Status & rAck.stError) {
					PPFormatT(PPTXT_CCACKERROR, &temp_buf, cc_text.cptr(), rAck.Message.cptr());
					Log(temp_buf);
					CALLEXCEPT_PP_S(PPERR_EGAIS_CCSENDFAULT, rAck.Message.cptr());
				}
				else {
					PPFormatT(PPTXT_CCACKOK, &temp_buf, cc_text.cptr(), (const char *)rAck.Sign);
					Log(temp_buf);
				}
			}
		}
		ok = 1;
	}
	CATCHZOK
	xmlFreeDoc(p_doc);
	xmlFreeParserCtxt(p_ctx);
	xmlFreeTextWriter(p_x);
    return ok;
}

int SLAPI PPEgaisProcessor::PutQuery(PPEgaisProcessor::Packet & rPack, PPID locID, const char * pUrlSuffix, PPEgaisProcessor::Ack & rAck)
{
	rAck.Clear();
    int    ok = 1;
	SString file_name, temp_buf;
	SString temp_path;
	SBuffer ack_buf;
	SString url;
	THROW(GetURL(locID, url));
	THROW(GetDebugPath(locID, temp_path));
	THROW(GetTempFileName(temp_path, pUrlSuffix, "EGQ", file_name));
	THROW(Write(rPack, locID, file_name));
	if(State & stTestSendingMode) {
		Log(PPLoadTextS(PPTXT_EGAIS_TESTSEDNING, temp_buf).CatDiv(':', 2).Cat(file_name));
	}
	else {
		ScURL c;
		SString req;
		url.RmvLastSlash().CatChar('/').Cat("opt").CatChar('/').Cat("in");
		if(!isempty(pUrlSuffix))
			url.CatChar('/').Cat(pUrlSuffix);
		SFile wr_stream(ack_buf, SFile::mWrite);
		ScURL::HttpForm hf;
		hf.AddContentFile(file_name, "application/xml", "xml_file");
		THROW_SL(c.HttpPost(url, 0, hf, &wr_stream));
		LogSended(rPack);
		THROW(ReadAck((SBuffer *)wr_stream, rAck));
		if(rAck.Status & Ack::stError) {
			PPLoadText(PPTXT_EGAIS_ERRONQUERY, temp_buf);
			Log(temp_buf.CatDiv(':', 2).Cat(rAck.Message));
		}
		else if(!rAck.Id.IsZero()) {
			BillTbl::Rec bill_rec;
			ObjTagItem tag_item;
			PPIDArray edi_op_list;
			edi_op_list.addzlist(PPEDIOP_EGAIS_WAYBILLACT, PPEDIOP_EGAIS_WAYBILL, PPEDIOP_EGAIS_ACTCHARGEON,
				PPEDIOP_EGAIS_ACTCHARGEON_V2, PPEDIOP_EGAIS_ACTCHARGEONSHOP, PPEDIOP_EGAIS_ACTWRITEOFF,
				PPEDIOP_EGAIS_ACTWRITEOFF_V2, PPEDIOP_EGAIS_TRANSFERTOSHOP, PPEDIOP_EGAIS_TRANSFERFROMSHOP,
				PPEDIOP_EGAIS_ACTWRITEOFFSHOP,
				0);
			THROW_MEM(SETIFZ(P_Dgq, new DGQCore));
			{
				Reference * p_ref = PPRef;
				PPTransaction tra(1);
				THROW(tra);
				THROW(P_Dgq->PutQuery(rAck.Id, rPack.DocType, 0));
				if(rPack.DocType == PPEDIOP_EGAIS_REQUESTREPEALWB) {
					RepealWb * p_req = (RepealWb *)rPack.P_Data;
                    if(p_req) {
						THROW(P_BObj->Search(p_req->BillID, &bill_rec) > 0);
						rAck.Id.ToStr(S_GUID::fmtIDL, temp_buf);
						THROW(tag_item.SetStr(PPTAG_BILL_EDIREPEALACK, temp_buf));
						THROW(p_ref->Ot.PutTag(PPOBJ_BILL, p_req->BillID, &tag_item, 0));
                    }
				}
				else if(edi_op_list.lsearch(rPack.DocType)) {
					PPBillPacket * p_bp = (PPBillPacket *)rPack.P_Data;
					if(p_bp) {
						THROW(P_BObj->Search(p_bp->Rec.ID, &bill_rec) > 0);
						{
							const long org_flags2 = bill_rec.Flags2;
							bill_rec.Flags2 |= BILLF2_ACKPENDING;
							THROW(P_BObj->P_Tbl->SetRecFlag2(p_bp->Rec.ID, BILLF2_ACKPENDING, 1, 0));
							if(rPack.DocType == PPEDIOP_EGAIS_WAYBILLACT) {
								const int recadv_status = BillCore::GetRecadvStatus(p_bp->Rec);
								BillCore::SetRecadvStatus(recadv_status, bill_rec);
							}
							if(bill_rec.Flags2 != org_flags2) {
								THROW_DB(P_BObj->P_Tbl->rereadForUpdate(0, 0));
								THROW_DB(P_BObj->P_Tbl->updateRecBuf(&bill_rec)); // @sfu
							}
						}
						{
							rAck.Id.ToStr(S_GUID::fmtIDL, temp_buf);
							if(rPack.DocType == PPEDIOP_EGAIS_WAYBILLACT && !p_bp->Rec.EdiOp && p_bp->Rec.Flags2 & BILLF2_DECLINED) {
								//
								// ��� ������ �� ������������ ��������� ���������� ���������� ����������� ��� ���������
								//
								THROW(tag_item.SetStr(PPTAG_BILL_EDIREJECTACK, temp_buf));
							}
							else {
								THROW(tag_item.SetStr(PPTAG_BILL_EDIACK, temp_buf));
							}
							THROW(p_ref->Ot.PutTag(PPOBJ_BILL, p_bp->Rec.ID, &tag_item, 0));
						}
					}
				}
				THROW(tra.Commit());
			}
		}
	}
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::QueryClients(PPID locID, int queryby, const char * pQ)
{
	int    ok = -1;
	THROW_PP(oneof2(queryby, querybyINN, querybyCode) && !isempty(pQ), PPERR_INVPARAM);
	{
		Ack    ack;
		Packet qp(PPEDIOP_EGAIS_QUERYCLIENTS);
		if(qp.P_Data) {
           	// ������ ����������� �� ��������������: "���"
           	// ������ ������ �� ���� �����: "���"
			SString code_buf;
			if(queryby == querybyINN)
				code_buf = "���";
			else if(queryby == querybyCode)
				code_buf = "���";
			else
				assert(0);
			code_buf.Transf(CTRANSF_OUTER_TO_INNER);
			((StrStrAssocArray *)qp.P_Data)->Add(code_buf, pQ);
		}
		THROW(PutQuery(qp, locID, "QueryPartner", ack));
	}
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::QueryRests(PPID locID, const char *)
{
	int    ok = -1;
	Ack    ack;
    Packet qp(PPEDIOP_EGAIS_QUERYRESTS);
	THROW(PutQuery(qp, locID, "QueryRests", ack));
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::QueryRestsShop(PPID locID, const char *)
{
	int    ok = -1;
	Ack    ack;
    Packet qp(PPEDIOP_EGAIS_QUERYRESTSSHOP);
	THROW(PutQuery(qp, locID, "QueryRestsShop_v2", ack));
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::QueryProducts(PPID locID, int queryby, const char * pQ)
{
	int    ok = -1;
	THROW_PP(oneof2(queryby, querybyINN, querybyCode) && !isempty(pQ), PPERR_INVPARAM);
	{
		Ack    ack;
		Packet qp(PPEDIOP_EGAIS_QUERYAP);
		if(qp.P_Data) {
			SString code_buf;
			if(queryby == querybyINN)
				code_buf = "���";
			else if(queryby == querybyCode)
				code_buf = "���";
			else
				assert(0);
			code_buf.Transf(CTRANSF_OUTER_TO_INNER);
			((StrStrAssocArray *)qp.P_Data)->Add(code_buf, pQ);
		}
		THROW(PutQuery(qp, locID, "QueryAP", ack));
	}
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::QueryInfA(PPID locID, const char * pInfA)
{
	int    ok = -1;
	Ack    ack;
    Packet qp(PPEDIOP_EGAIS_QUERYFORMA);
	if(qp.P_Data) {
        *(SString *)qp.P_Data = pInfA;
	}
	THROW(PutQuery(qp, locID, "QueryFormA", ack));
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::QueryInfB(PPID locID, const char * pInfB)
{
	int    ok = -1;
	Ack    ack;
    Packet qp(PPEDIOP_EGAIS_QUERYFORMB);
	if(qp.P_Data) {
		*(SString *)qp.P_Data = pInfB;
	}
	THROW(PutQuery(qp, locID, "QueryFormB", ack));
	CATCHZOK
    return ok;
}

SLAPI PPEgaisProcessor::PPEgaisProcessor(long cflags, PPLogger * pOuterLogger) : PrcssrAlcReport()
{
	State = 0;
	P_Dgq = 0;
	P_LecT = 0;
	P_Logger = 0;
	P_UtmEntry = 0;
	P_Las = new PPLocAddrStruc(0, 0);
	THROW(SetConfig(0));
	THROW(Init());
	if(pOuterLogger) {
		P_Logger = pOuterLogger;
		State |= stOuterLogger;
	}
	else if(cflags & cfDirectFileLogging) {
		State |= stDirectFileLogging;
	}
	else {
		THROW_MEM(SETIFZ(P_Logger, new PPLogger));
	}
	if(cflags & cfDebugMode) {
		State |= stTestSendingMode;
	}
	if(DS.CheckExtFlag(ECF_OPENSOURCE))
		State |= stValidLic;
	else {
		PPLicData ld;
		if(PPGetLicData(&ld) > 0 && ld.ExtFunc & PPLicData::effEgais)
			State |= stValidLic;
	}
	CATCH
		State |= stError;
	ENDCATCH
}

SLAPI PPEgaisProcessor::~PPEgaisProcessor()
{
	P_UtmEntry = 0;
	delete P_LecT;
	delete P_Dgq;
	delete P_Las;
	if(!(State & stOuterLogger))
		delete P_Logger;
}

int SLAPI PPEgaisProcessor::operator !() const
{
	return BIN(State & stError);
}

int SLAPI PPEgaisProcessor::SetTestSendingMode(int set)
{
	SETFLAG(State, stTestSendingMode, set);
	return 1;
}

int SLAPI PPEgaisProcessor::SetNonRvmTagMode(int set)
{
	SETFLAG(State, stDontRemoveTags, set);
	return 1;
}

int SLAPI PPEgaisProcessor::CheckLic() const
{
	return (State & stValidLic) ? 1 : PPSetError(PPERR_EGAIS_NOLIC);
}

void FASTCALL PPEgaisProcessor::Log(const SString & rMsg)
{
	if(P_Logger) {
		P_Logger->Log(rMsg);
	}
	else if(State & stDirectFileLogging) {
		PPLogMessage(PPFILNAM_EGAIS_LOG, rMsg, LOGMSGF_DBINFO|LOGMSGF_TIME|LOGMSGF_USER);
	}
}

void SLAPI PPEgaisProcessor::LogTextWithAddendum(int msgCode, const SString & rAddendum)
{
	SString fmt_buf, msg_buf;
	PPLoadText(msgCode, fmt_buf);
	Log(msg_buf.Printf(fmt_buf, (const char *)rAddendum));
}

void SLAPI PPEgaisProcessor::LogLastError()
{
	if(P_Logger)
		P_Logger->LogLastError();
	else if(State & stDirectFileLogging) {
		PPLogMessage(PPFILNAM_EGAIS_LOG, 0, LOGMSGF_LASTERR|LOGMSGF_DBINFO|LOGMSGF_TIME|LOGMSGF_USER);
	}
}

const SString & FASTCALL PPEgaisProcessor::EncText(const char * pS)
{
	EncBuf = pS;
	EncBuf.ReplaceChar('\x07', ' '); // @v9.4.8
	PROFILE(XMLReplaceSpecSymb(EncBuf, "&<>\'"));
	return EncBuf.Transf(CTRANSF_INNER_TO_UTF8);
}

const SString & FASTCALL PPEgaisProcessor::EncText(const SString & rS)
{
	EncBuf = rS;
	EncBuf.ReplaceChar('\x07', ' '); // @v9.4.8
	PROFILE(XMLReplaceSpecSymb(EncBuf, "&<>\'"));
	return EncBuf.Transf(CTRANSF_INNER_TO_UTF8);
}

struct EgaisDocTypeEntry {
	int    DocType;
	const  char * P_Tag;
};

static const EgaisDocTypeEntry _EgaisDocTypes[] = {
	{ PPEDIOP_EGAIS_TICKET,          "Ticket" },
	{ PPEDIOP_EGAIS_WAYBILLACT,      "WayBillAct" },
	{ PPEDIOP_EGAIS_WAYBILL,         "WayBill" },
	{ PPEDIOP_EGAIS_CONFIRMTICKET,   "ConfirmTicket" },
	{ PPEDIOP_EGAIS_TTNINFORMBREG,   "TTNInformBReg" },
	{ PPEDIOP_EGAIS_ACTREJECT,       "ActReject" },
	{ PPEDIOP_EGAIS_ACTUSING,        "ActUsing" },
	{ PPEDIOP_EGAIS_ACTINVENTORYPARTIAL,    "ActInventoryPartial" },
	{ PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG, "ActInventoryInformBReg" },
	{ PPEDIOP_EGAIS_ACTINVENTORY,    "ActInventory" },
	{ PPEDIOP_EGAIS_QUERYAP,         "QueryAP" },
	{ PPEDIOP_EGAIS_QUERYSSP,        "QuerySSP" },
	{ PPEDIOP_EGAIS_QUERYSP,         "QuerySP" },
	{ PPEDIOP_EGAIS_QUERYCLIENTS,    "QueryClients" },
	{ PPEDIOP_EGAIS_QUERYRESTS,      "QueryRests" },
	{ PPEDIOP_EGAIS_QUERYRESTSSHOP,  "QueryRestsShop_v2" },
	{ PPEDIOP_EGAIS_QUERYFORMA,      "QueryFormA" },
	{ PPEDIOP_EGAIS_QUERYFORMB,      "QueryFormB" },
	{ PPEDIOP_EGAIS_REPLYSSP,        "ReplySSP" },
	{ PPEDIOP_EGAIS_REPLYSPIRIT,     "ReplySpirit" },
	{ PPEDIOP_EGAIS_REPLYCLIENT,     "ReplyClient" },
	{ PPEDIOP_EGAIS_REPLYAP,         "ReplyAP" },
	{ PPEDIOP_EGAIS_REPLYRESTS,      "ReplyRests" },
	{ PPEDIOP_EGAIS_REPLYRESTSSHOP,  "ReplyRestsShop_v2" },
	{ PPEDIOP_EGAIS_REPLYFORMA,      "ReplyFormA" },
	{ PPEDIOP_EGAIS_REPLYFORMB,      "ReplyFormB" },
	{ PPEDIOP_EGAIS_ACTCHARGEON,     "ActChargeOn" },
	{ PPEDIOP_EGAIS_ACTCHARGEON_V2,  "ActChargeOn_v2" }, // @v9.3.12
	{ PPEDIOP_EGAIS_ACTWRITEOFF,     "ActWriteOff" },
	{ PPEDIOP_EGAIS_ACTWRITEOFF_V2,  "ActWriteOff_v2" }, // @v9.3.12
	{ PPEDIOP_EGAIS_REQUESTREPEALWB, "RequestRepealWB" }, // @v9.2.8
	{ PPEDIOP_EGAIS_CONFIRMREPEALWB, "ConfirmRepealWB" }, // @v9.2.8
	{ PPEDIOP_EGAIS_QUERYBARCODE,    "QueryBarcode" },    // @v9.2.8
	{ PPEDIOP_EGAIS_REPLYBARCODE,    "ReplyBarcode" },    // @v9.2.8
	{ PPEDIOP_EGAIS_TRANSFERTOSHOP,  "TransferToShop" },  // @v9.2.11
	{ PPEDIOP_EGAIS_TRANSFERFROMSHOP, "TransferFromShop" }, // @v9.3.10
	{ PPEDIOP_EGAIS_ACTCHARGEONSHOP,  "ActChargeOnShop_v2" }, // @v9.2.11
	{ PPEDIOP_EGAIS_ACTWRITEOFFSHOP, "ActWriteOffShop_v2" } // @v9.4.0
};

const char * FASTCALL PPEgaisProcessor::GetDocTypeTag(int docType)
{
    for(uint i = 0; i < SIZEOFARRAY(_EgaisDocTypes); i++) {
		const EgaisDocTypeEntry & r_entry = _EgaisDocTypes[i];
		if(r_entry.DocType == docType)
			return r_entry.P_Tag;
    }
    return 0;
}

// static
int FASTCALL PPEgaisProcessor::RecognizeDocTypeTag(const char * pTag)
{
	if(!isempty(pTag)) {
		SString ns, ntag;
		SString tag;
		if((tag = pTag).Strip().Divide(':', ns, ntag) > 0) {
			tag = ntag;
		}
		if(tag.NotEmptyS()) {
			for(uint i = 0; i < SIZEOFARRAY(_EgaisDocTypes); i++) {
				const EgaisDocTypeEntry & r_entry = _EgaisDocTypes[i];
				if(tag.CmpNC(r_entry.P_Tag) == 0)
					return r_entry.DocType;
			}
		}
	}
    return 0;
}
//
//
//
struct EgaisWayBillTypeEntry {
	int    Wbt;
	const char * P_Text;
};

EgaisWayBillTypeEntry _EgaisWayBillTypes[] = {
	{ PPEgaisProcessor::wbtInvcFromMe, "WBInvoiceFromMe" },
	{ PPEgaisProcessor::wbtInvcToMe, "WBInvoiceToMe" },
	{ PPEgaisProcessor::wbtRetFromMe, "WBReturnFromMe" },
	{ PPEgaisProcessor::wbtRetToMe, "WBReturnToMe" }
};

const char * FASTCALL PPEgaisProcessor::GetWayBillTypeText(int wbType)
{
    for(uint i = 0; i < SIZEOFARRAY(_EgaisWayBillTypes); i++) {
		const EgaisWayBillTypeEntry & r_entry = _EgaisWayBillTypes[i];
		if(r_entry.Wbt == wbType)
			return r_entry.P_Text;
    }
    return 0;
}

int FASTCALL PPEgaisProcessor::RecognizeWayBillTypeText(const char * pText)
{
	if(!isempty(pText)) {
		for(uint i = 0; i < SIZEOFARRAY(_EgaisWayBillTypes); i++) {
			const EgaisWayBillTypeEntry & r_entry = _EgaisWayBillTypes[i];
			if(stricmp(pText, r_entry.P_Text) == 0)
				return r_entry.Wbt;
		}
	}
    return 0;
}

int SLAPI PPEgaisProcessor::WriteOrgInfo(SXml::WDoc & rXmlDoc, const char * pScopeXmlTag, PPID personID, PPID addrLocID, LDATE actualDate, long flags)
{
	// woifDontSendWithoutFSRARID
	int   ok = 1;
	Reference * p_ref = PPRef;
	PPPersonPacket psn_pack;
	SString temp_buf;
	SString inn, kpp;
	SString rar_id;
	RegisterTbl::Rec reg_rec;
	THROW(PsnObj.GetPacket(personID, &psn_pack, PGETPCKF_USEINHERITENCE) > 0); // @v9.3.6 PGETPCKF_USEINHERITENCE
	{
		ObjTagItem tag_item;
		if(addrLocID)
			p_ref->Ot.GetTagStr(PPOBJ_LOCATION, addrLocID, PPTAG_LOC_FSRARID, rar_id);
		if(!rar_id.NotEmptyS())
			p_ref->Ot.GetTagStr(PPOBJ_PERSON, personID, PPTAG_PERSON_FSRARID, rar_id);
		if(flags & woifStrict) {
			THROW_PP_S(rar_id.NotEmptyS(), PPERR_EGAIS_PERSONFSRARIDUNDEF, psn_pack.Rec.Name);
		}
		else if(flags & woifDontSendWithoutFSRARID) {
			if(!rar_id.NotEmptyS()) {
				LogTextWithAddendum(PPTXT_EGAIS_PERSONHASNTRARID, temp_buf = psn_pack.Rec.Name);
                ok = -1;
			}
		}
	}
	if(ok > 0) {
		SXml::WNode w_s(rXmlDoc, pScopeXmlTag);
		// oref:
		w_s.PutInner("oref:Identity", (temp_buf = 0).Cat(personID));
		w_s.PutInnerSkipEmpty("oref:ClientRegId", EncText(rar_id));
		if(psn_pack.GetExtName(temp_buf = 0) <= 0)
			temp_buf = psn_pack.Rec.Name;
		w_s.PutInner("oref:FullName", EncText(temp_buf));
		w_s.PutInner("oref:ShortName", EncText((temp_buf = psn_pack.Rec.Name).Trim(64)));
		{
			// PPPersonPack
			if(psn_pack.Regs.GetRegister(PPREGT_TPID, actualDate, 0, &reg_rec) > 0) {
				(inn = reg_rec.Num).Strip();
			}
			else
				inn = 0;
			if(flags & woifStrict) {
				THROW_PP_S(inn.NotEmptyS(), PPERR_EGAIS_PERSONINNUNDEF, psn_pack.Rec.Name);
			}
			w_s.PutInnerSkipEmpty("oref:INN", EncText(inn));
		}
		{
			if(GetWkrRegister(wkrKPP, personID, addrLocID, actualDate, &reg_rec) > 0) {
				kpp = reg_rec.Num;
			}
			else
				kpp = 0;
			if(flags & woifStrict) {
				THROW_PP_S(kpp.NotEmptyS() || inn.Len() == 12, PPERR_EGAIS_PERSONKPPUNDEF, psn_pack.Rec.Name);
			}
			w_s.PutInnerSkipEmpty("oref:KPP", EncText(kpp));
		}
		w_s.PutInnerSkipEmpty("oref:UNP", ""); // ��� ��������
		w_s.PutInnerSkipEmpty("oref:RNN", ""); // ��� ����������
		{
			LocationTbl::Rec loc_rec;
			PPID   loc_id = 0;
			SString full_addr_buf;
			if(addrLocID && PsnObj.LocObj.Search(addrLocID, &loc_rec) > 0)
				loc_id = addrLocID;
			else if(psn_pack.Rec.RLoc) {
				loc_id = psn_pack.RLoc.ID;
				loc_rec = psn_pack.RLoc;
			}
			else if(psn_pack.Rec.MainLoc) {
				loc_id = psn_pack.Loc.ID;
				loc_rec = psn_pack.Loc;
			}
			if(loc_id)
				PsnObj.LocObj.P_Tbl->GetAddress(loc_rec, 0, full_addr_buf);
			else
				full_addr_buf = 0;
			P_Las->Recognize((temp_buf = full_addr_buf).Transf(CTRANSF_INNER_TO_OUTER));
			{
				SXml::WNode w_a(rXmlDoc, "oref:address");
				PPCountryBlock cntryb;
				PsnObj.LocObj.GetCountry(&loc_rec, 0, &cntryb);
				{
					temp_buf = cntryb.Code.NotEmptyS() ? cntryb.Code.cptr() : "643";
					long country_id = temp_buf.ToLong();
					(temp_buf = 0).CatLongZ(country_id, 3);
					w_a.PutInner("oref:Country", EncText(temp_buf)); // �� ��������� 643 (������)
				}
				LocationCore::GetExField(&loc_rec, LOCEXSTR_ZIP, temp_buf);
				if(!temp_buf.IsDigit() || temp_buf.Len() != 6) // �������� �� "�����" �������� ������
					temp_buf = 0;
				w_a.PutInnerSkipEmpty("oref:Index", EncText(temp_buf));
				//
				// ��� � ��� ��� ������ ������ ��������� � ���� �������� ��� �������.
				// ������� �� ����� �������� �������� ���� ��� �� ������, � ���������� ��� (� �����������) ���,
				// ���� ��� ������ - ���.
				//
				if(kpp.Len() > 2)
					kpp.Sub(0, 2, temp_buf);
				else if(inn.Len() > 2)
					inn.Sub(0, 2, temp_buf);
				else
					temp_buf = 0;
				w_a.PutInnerSkipEmpty("oref:RegionCode", EncText(temp_buf));
				w_a.PutInnerSkipEmpty("oref:area", "");
				PsnObj.LocObj.GetCity(loc_id, 0, &temp_buf, 1);
				w_a.PutInnerSkipEmpty("oref:city", EncText(temp_buf));
				w_a.PutInnerSkipEmpty("oref:place", "");
				if(P_Las) {
					P_Las->Get(P_Las->tStreet, temp_buf);
					temp_buf.Strip().Transf(CTRANSF_OUTER_TO_INNER);
				}
				else
					temp_buf = 0;
				w_a.PutInnerSkipEmpty("oref:street", EncText(temp_buf));
				if(P_Las) {
					P_Las->Get(P_Las->tHouse, temp_buf);
					temp_buf.Strip().Transf(CTRANSF_OUTER_TO_INNER);
				}
				else
					temp_buf = 0;
				w_a.PutInnerSkipEmpty("oref:house", EncText(temp_buf));
				if(P_Las) {
					P_Las->Get(P_Las->tHouseAddendum, temp_buf);
					temp_buf.Strip().Transf(CTRANSF_OUTER_TO_INNER);
				}
				else
					temp_buf = 0;
				w_a.PutInnerSkipEmpty("oref:building", EncText(temp_buf));
				w_a.PutInnerSkipEmpty("oref:liter", "");
				w_a.PutInner("oref:description", EncText(full_addr_buf));
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::WriteOrgInfo(SXml::WDoc & rXmlDoc, const char * pScopeXmlTag, const EgaisPersonCore::Item & rRefcItem, long flags)
{
	// woifDontSendWithoutFSRARID
	int   ok = 1;
	SString temp_buf;
	SString inn = rRefcItem.INN;
	SString kpp = rRefcItem.KPP;
	SString rar_id = rRefcItem.RarIdent;
	int   j_status = 0; // 1 - ���� ��, 2 - ���� ��, 3 - ���������� (�� ��), 4 - ���������� (���������� ����)
	const char * p_j_scope = 0;
	{
		if(flags & woifStrict) {
			THROW_PP_S(rar_id.NotEmptyS(), PPERR_EGAIS_PERSONFSRARIDUNDEF, rRefcItem.Name);
		}
		else if(flags & woifDontSendWithoutFSRARID) {
			if(!rar_id.NotEmptyS()) {
				LogTextWithAddendum(PPTXT_EGAIS_PERSONHASNTRARID, temp_buf = rRefcItem.Name);
                ok = -1;
			}
		}
	}
	if(ok > 0) {
		if(rRefcItem.CountryCode == 643 && inn.Len() == 10) {
			j_status = 1;
			p_j_scope = "oref:UL";
		}
		else if(rRefcItem.CountryCode == 643 && inn.Len() == 12) {
			j_status = 2;
			p_j_scope = "oref:FL";
		}
		else if(rRefcItem.RNN[0] || rRefcItem.UNP[0]) {
			j_status = 4;
			p_j_scope = "oref:TS";
		}
		else {
			j_status = 3;
			p_j_scope = "oref:FO";
		}
		SXml::WNode w_s(rXmlDoc, pScopeXmlTag);
		if(flags & woifVersion2) {
			SXml::WNode w_j(rXmlDoc, p_j_scope);
			w_s.PutInnerSkipEmpty("oref:ClientRegId", EncText(rar_id));
			w_s.PutInner("oref:FullName", EncText(rRefcItem.FullName));
			w_s.PutInner("oref:ShortName", EncText((temp_buf = rRefcItem.Name).Trim(64)));
			if(j_status == 1) {
				if(flags & woifStrict) {
					THROW_PP_S(inn.NotEmptyS(), PPERR_EGAIS_PERSONINNUNDEF, rRefcItem.Name);
				}
				w_s.PutInnerSkipEmpty("oref:INN", EncText(inn));
				if(flags & woifStrict) {
					THROW_PP_S(kpp.NotEmptyS() || inn.Len() == 12, PPERR_EGAIS_PERSONKPPUNDEF, rRefcItem.Name);
				}
				w_s.PutInnerSkipEmpty("oref:KPP", EncText(kpp));
			}
			else if(j_status == 2) {
				if(flags & woifStrict) {
					THROW_PP_S(inn.NotEmptyS(), PPERR_EGAIS_PERSONINNUNDEF, rRefcItem.Name);
				}
				w_s.PutInnerSkipEmpty("oref:INN", EncText(inn));
			}
			else if(j_status == 3) {
				if(rRefcItem.RNN[0])
					w_s.PutInnerSkipEmpty("oref:TSNUM", EncText(rRefcItem.RNN));
				else if(rRefcItem.UNP[0])
					w_s.PutInnerSkipEmpty("oref:TSNUM", EncText(rRefcItem.UNP));
			}
			{
				SXml::WNode w_a(rXmlDoc, "oref:address");
				w_a.PutInner("oref:Country", EncText((temp_buf = 0).CatLongZ(rRefcItem.CountryCode, 3)));
				if(rRefcItem.RegionCode)
					w_a.PutInnerSkipEmpty("oref:RegionCode", EncText((temp_buf = 0).CatLongZ(rRefcItem.RegionCode, 2)));
				else if(oneof2(j_status, 1, 2)) {
					LogTextWithAddendum(PPTXT_EGAIS_PERSONHASNTREGCODE, temp_buf = rRefcItem.Name);
				}
				w_a.PutInnerSkipEmpty("oref:area", "");
				w_a.PutInner("oref:description", EncText(temp_buf = rRefcItem.AddressDescr)); // @v9.4.6 PutInnerSkipEmpty-->PutInner
				/*if(temp_buf.IsEmpty()) {
					LogTextWithAddendum(PPTXT_EGAIS_PERSONHASNTREGCODE, temp_buf = rRefcItem.Name);
				}*/
			}
		}
		else {
			// oref:
			w_s.PutInner("oref:Identity", (temp_buf = 0).Cat(rRefcItem.ID));
			w_s.PutInnerSkipEmpty("oref:ClientRegId", EncText(rar_id));
			w_s.PutInner("oref:FullName", EncText(rRefcItem.FullName));
			w_s.PutInner("oref:ShortName", EncText((temp_buf = rRefcItem.Name).Trim(64)));
			{
				if(flags & woifStrict) {
					THROW_PP_S(inn.NotEmptyS(), PPERR_EGAIS_PERSONINNUNDEF, rRefcItem.Name);
				}
				w_s.PutInnerSkipEmpty("oref:INN", EncText(inn));
			}
			{
				if(flags & woifStrict) {
					THROW_PP_S(kpp.NotEmptyS() || inn.Len() == 12, PPERR_EGAIS_PERSONKPPUNDEF, rRefcItem.Name);
				}
				w_s.PutInnerSkipEmpty("oref:KPP", EncText(kpp));
			}
			w_s.PutInnerSkipEmpty("oref:UNP", EncText(rRefcItem.UNP)); // ��� ��������
			w_s.PutInnerSkipEmpty("oref:RNN", EncText(rRefcItem.RNN)); // ��� ����������
			{
				SXml::WNode w_a(rXmlDoc, "oref:address");
				w_a.PutInner("oref:Country", EncText((temp_buf = 0).CatLongZ(rRefcItem.CountryCode, 3)));
				if(rRefcItem.RegionCode)
					w_a.PutInnerSkipEmpty("oref:RegionCode", EncText((temp_buf = 0).CatLongZ(rRefcItem.RegionCode, 2)));
				w_a.PutInnerSkipEmpty("oref:area", "");
				w_a.PutInnerSkipEmpty("oref:description", EncText(temp_buf = rRefcItem.AddressDescr));
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::WriteProductInfo(SXml::WDoc & rXmlDoc, const char * pScopeXmlTag,
	PPID goodsID, PPID lotID, long flags, const ObjTagList * pLotTagList)
{
	int    ok = -1;
	Goods2Tbl::Rec goods_rec;
	PrcssrAlcReport::GoodsItem agi;
	if(GObj.Fetch(goodsID, &goods_rec) > 0 && PreprocessGoodsItem(goodsID, lotID, pLotTagList, 0, agi) > 0) {
		const int use_refc_data = BIN(ACfg.Hdr.Flags & ACfg.Hdr.fUseOwnEgaisObjects && P_RefC);
		SString temp_buf, added_msg_buf;
		PPID   manuf_id = 0;
		PPID   importer_id = 0;
		int    manuf_done = 0;
		SXml::WNode w_s(rXmlDoc, pScopeXmlTag /*"wb:Product"*/);
		if(!(flags & wpifVersion2))
			w_s.PutInner("pref:Identity", (temp_buf = 0).Cat(goodsID));
		if(flags & wpifVersion2) {
			w_s.PutInner("pref:UnitType", EncText((temp_buf = (agi.UnpackedVolume > 0.0) ? "Unpacked" : "Packed").Transf(CTRANSF_OUTER_TO_INNER)));
		}
		w_s.PutInner("pref:Type", EncText((temp_buf = "��").Transf(CTRANSF_OUTER_TO_INNER))); // �� ��� ���-�� ���
		{
			int    names_done = 0;
			if(use_refc_data && agi.RefcProductID) {
				EgaisProductCore::Item item;
                if(P_RefC->PrC.Search(agi.RefcProductID, item) > 0 && item.FullName.NotEmpty()) {
					w_s.PutInner("pref:FullName", EncText(item.FullName));
					temp_buf = item.Name;
					if(!temp_buf.NotEmptyS())
						temp_buf = item.FullName;
					w_s.PutInner("pref:ShortName", EncText(temp_buf.Trim(64)));
					names_done = 1;
                }
			}
			if(!names_done) {
				w_s.PutInner("pref:FullName", EncText(goods_rec.Name));
				w_s.PutInner("pref:ShortName", EncText((temp_buf = goods_rec.Name).Trim(64)));
			}
		}
		if(agi.EgaisCode.Empty()) {
			(added_msg_buf = 0).CatEq("Goods", goods_rec.Name).CatDiv(';', 2).CatEq("LotID", lotID);
			LogTextWithAddendum(PPTXT_EGAIS_NOEGAISCODE, added_msg_buf);
		}
		w_s.PutInner("pref:AlcCode", EncText(agi.EgaisCode)); // ��� �������� �� ������ �����
		if(agi.UnpackedVolume > 0.0) {
		}
		else {
			if(use_refc_data && agi.RefcVolume)
				w_s.PutInner("pref:Capacity", (temp_buf = 0).Cat(agi.RefcVolume, MKSFMTD(0, 4, 0)));
			else
				w_s.PutInner("pref:Capacity", (temp_buf = 0).Cat(agi.Volume, MKSFMTD(0, 4, 0)));
		}
		if(use_refc_data && agi.RefcProof)
			w_s.PutInner("pref:AlcVolume", (temp_buf = 0).Cat(agi.RefcProof, MKSFMTD(0, 3, 0)));
		else
			w_s.PutInner("pref:AlcVolume", (temp_buf = 0).Cat(agi.Proof, MKSFMTD(0, 3, 0)));
		if(use_refc_data && agi.RefcCategoryCode.NotEmpty())
			w_s.PutInner("pref:ProductVCode", agi.RefcCategoryCode);
		else
			w_s.PutInner("pref:ProductVCode", agi.CategoryCode);
		if(flags & wpifPutManufInfo) {
			const  long woif = (flags & wpifVersion2) ? (woifDontSendWithoutFSRARID|woifVersion2) : woifDontSendWithoutFSRARID;
			if(use_refc_data && (agi.RefcImporterCode.NotEmpty() || agi.RefcManufCode.NotEmpty())) {
				TSArray <EgaisPersonTbl::Rec> epr_list;
				if(agi.RefcManufCode.NotEmpty()) {
					const int idx = P_RefC->PsC.SearchByCode(agi.RefcManufCode, epr_list);
					if(idx > 0) {
                        const EgaisPersonTbl::Rec & r_rec = epr_list.at(idx-1);
                        EgaisPersonCore::Item epr_item;
                        P_RefC->PsC.RecToItem(r_rec, epr_item);
						WriteOrgInfo(rXmlDoc, "pref:Producer", epr_item, woif);
						manuf_done = 1;
					}
				}
				if(agi.RefcImporterCode.NotEmpty() && !(flags & wpifVersion2)) {
					const int idx = P_RefC->PsC.SearchByCode(agi.RefcImporterCode, epr_list);
					if(idx > 0) {
						const EgaisPersonTbl::Rec & r_rec = epr_list.at(idx-1);
						EgaisPersonCore::Item epr_item;
						P_RefC->PsC.RecToItem(r_rec, epr_item);
						WriteOrgInfo(rXmlDoc, "pref:Importer", epr_item, woif);
						manuf_done = 1;
					}
				}
			}
			if(!manuf_done) {
				if(lotID) {
					if(Cfg.LotManufTagList.getCount()) {
						ObjTagItem tag_item;
						long   temp_id = 0;
						PersonTbl::Rec psn_rec;
						for(uint i = 0; !temp_id && i < Cfg.LotManufTagList.getCount(); i++) {
							const PPID tag_id = Cfg.LotManufTagList.get(i);
							if(PPRef->Ot.GetTag(PPOBJ_LOT, lotID, tag_id, &tag_item) > 0)
								tag_item.GetInt(&temp_id);
						}
						if(temp_id && PsnObj.Fetch(temp_id, &psn_rec) > 0) {
							int manuf_type = GetManufPersonType(temp_id);
							if(manuf_type == 1)
								manuf_id = temp_id;
							else if(manuf_type == 2)
								importer_id = temp_id;
						}
					}
				}
				if(manuf_id) {
					WriteOrgInfo(rXmlDoc, "pref:Producer", manuf_id, 0, getcurdate_(), woif);
					manuf_done = 1;
				}
				if(importer_id && !(flags & wpifVersion2))
					WriteOrgInfo(rXmlDoc, "pref:Importer", importer_id, 0, getcurdate_(), woif);
			}
		}
		if((flags & wpifVersion2) && !manuf_done) {
			(added_msg_buf = 0).Cat(agi.EgaisCode).CatDiv('-', 1).Cat(goods_rec.Name);
			LogTextWithAddendum(PPTXT_EGAIS_ALCWAREHASNTMANUF, added_msg_buf);
		}
		ok = 1;
	}
	return ok;
}

int SLAPI PPEgaisProcessor::WriteInformCode(SXml::WDoc & rXmlDoc, const char * pNs, char informKind, SString & rCode)
{
	assert(oneof2(informKind, 'A', 'B'));
    int    ok = 1;
    if(oneof2(informKind, 'A', 'B')) {
		SString temp_buf;
		(temp_buf = pNs).CatChar(':').Cat("Inform").CatChar(informKind);
		SXml::WNode w_s(rXmlDoc, temp_buf);
		if(informKind == 'A') {
			w_s.PutInner("pref:RegId", EncText(rCode));
		}
		else {
			SXml::WNode w_i(rXmlDoc, "pref:InformBItem");
			w_i.PutInner("pref:BRegId", EncText(rCode));
		}
    }
    else
		ok = 0;
    return ok;
}

int SLAPI PPEgaisProcessor::SetUtmEntry(PPID locID, const UtmEntry * pEntry, const DateRange * pPeriod)
{
	P_UtmEntry = pEntry;
	if(P_UtmEntry) {
		PPID   main_org_id = 0;
		SString fsrar_id;
		SString url;
		SString msg_buf, temp_buf;
		GetURL(locID, url);
		GetFSRARID(locID, fsrar_id, &main_org_id);
		GetPersonName(main_org_id, temp_buf);
		msg_buf.Cat(temp_buf).CatDiv('-', 1);
		if(locID) {
			GetLocationName(locID, temp_buf);
			msg_buf.Cat(temp_buf).CatDiv('-', 1);
		}
		msg_buf.Cat(url).CatDiv('-', 1);
		msg_buf.Cat(fsrar_id);
		if(pPeriod && !pPeriod->IsZero()) {
			msg_buf.CatDiv('-', 1).Cat(*pPeriod, 1);
		}
		Log(msg_buf);
	}
	return 1;
}

int SLAPI PPEgaisProcessor::GetUtmList(PPID locID, TSArray <UtmEntry> & rList)
{
	rList.clear();
	int    ok = -1;
	SString fsrar_id;
	SString url;
	PPIDArray main_psn_list;
	{
		PersonKindTbl * t = &PsnObj.P_Tbl->Kind;
		PersonKindTbl::Key0 k0;
		PersonTbl::Rec psn_rec;
		MEMSZERO(k0);
		k0.KindID = PPPRK_MAIN;
		BExtQuery q(t, 0, 128);
		q.select(t->PersonID, 0).where(t->KindID == PPPRK_MAIN);
		for(q.initIteration(0, &k0, spGe); q.nextIteration() > 0;) {
			const PPID psn_id = t->data.PersonID;
			if(PsnObj.Search(psn_id, &psn_rec) > 0)
				main_psn_list.add(psn_id);
		}
		main_psn_list.sortAndUndup();
	}
	{
		Reference * p_ref = PPRef;
		for(uint i = 0; i < main_psn_list.getCount(); i++) {
			const PPID psn_id = main_psn_list.get(i);
			p_ref->Ot.GetTagStr(PPOBJ_PERSON, psn_id, PPTAG_PERSON_FSRARID, fsrar_id);
			p_ref->Ot.GetTagStr(PPOBJ_PERSON, psn_id, PPTAG_PERSON_EGAISSRVURL, url);
			if(fsrar_id.NotEmptyS() && url.NotEmptyS()) {
				UtmEntry new_entry;
				MEMSZERO(new_entry);
				new_entry.MainOrgID = psn_id;
				STRNSCPY(new_entry.Url, url);
				STRNSCPY(new_entry.FSRARID, fsrar_id);
				THROW_SL(rList.insert(&new_entry));
				ok = 2;
			}
		}
	}
	if(!rList.getCount()) {
		PPID   main_org_id = 0;
		fsrar_id = 0;
		url = 0;
		THROW(GetURL(locID, url));
		THROW(GetFSRARID(locID, fsrar_id, &main_org_id));
		{
			UtmEntry new_entry;
			MEMSZERO(new_entry);
			new_entry.Flags |= UtmEntry::fDefault;
			new_entry.MainOrgID = main_org_id;
			STRNSCPY(new_entry.Url, url);
			STRNSCPY(new_entry.FSRARID, fsrar_id);
			THROW_SL(rList.insert(&new_entry));
			ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::GetURL(PPID locID, SString & rBuf)
{
	rBuf = 0;
	int    ok = 0;
	if(locID && (!P_UtmEntry || P_UtmEntry->Flags & UtmEntry::fDefault)) {
		if(PPRef->Ot.GetTagStr(PPOBJ_LOCATION, locID, PPTAG_LOC_EGAISSRVURL, rBuf) > 0)
			ok = 2;
	}
	if(!ok) {
		if(P_UtmEntry) {
			if(P_UtmEntry->Url[0]) {
				rBuf = P_UtmEntry->Url;
				ok = 3;
			}
		}
		else {
			rBuf = ACfg.EgaisServerURL;
			if(rBuf.NotEmptyS())
				ok = 1;
		}
	}
	return NZOR(ok, PPSetError(PPERR_EGAIS_SRVURLUNDEF));
}

int SLAPI PPEgaisProcessor::GetFSRARID(PPID locID, SString & rBuf, PPID * pMainOrgID)
{
	rBuf = 0;
	int    ok = 0;
	Reference * p_ref = PPRef;
	PPID   main_org_id = 0;
	ObjTagItem rarid_tag;
	GetMainOrgID(&main_org_id);
	if(locID && (!P_UtmEntry || P_UtmEntry->Flags & UtmEntry::fDefault)) {
		if(p_ref->Ot.GetTagStr(PPOBJ_LOCATION, locID, PPTAG_LOC_FSRARID, rBuf) > 0)
			ok = 2;
	}
	if(!ok) {
		if(P_UtmEntry) {
			main_org_id = P_UtmEntry->MainOrgID;
			if(P_UtmEntry->FSRARID[0]) {
				rBuf = P_UtmEntry->FSRARID;
				ok = 3;
			}
		}
		else if(p_ref->Ot.GetTagStr(PPOBJ_PERSON, main_org_id, PPTAG_PERSON_FSRARID, rBuf) > 0)
			ok = 1;
	}
	ASSIGN_PTR(pMainOrgID, main_org_id);
	if(!ok)
		PPSetError(PPERR_EGAIS_FSRARIDUNDEF);
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_MakeMarkList(PPID lotID, StringSet & rSsExtCodes, uint * pExtCodeCount)
{
	rSsExtCodes.clear();

	int    ok = -1;
	uint   ext_code_count = 0;
	THROW_MEM(SETIFZ(P_LecT, new LotExtCodeTbl));
	if(lotID) {
		LotExtCodeTbl::Key0 k0;
		MEMSZERO(k0);
		k0.LotID = lotID;
		if(P_LecT->search(0, &k0, spGe) && P_LecT->data.LotID == lotID) {
			ok = 1;
			SString temp_buf;
			do {
				temp_buf = P_LecT->data.Code;
				if(!ExclChrgOnMarks.search(temp_buf, 0, 1)) {
					ext_code_count++;
					rSsExtCodes.add(temp_buf);
				}
			} while(P_LecT->search(0, &k0, spNext) && P_LecT->data.LotID == lotID);
		}
	}
	CATCHZOK
	ASSIGN_PTR(pExtCodeCount, ext_code_count);
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_Write(Packet & rPack, PPID locID, xmlTextWriter * pX)
{
	int    ok = -1;
	Reference * p_ref = PPRef;
	const  int doc_type = rPack.DocType;
	const  char * p_doc_type_tag = GetDocTypeTag(doc_type);
	PPID   main_org_id = 0;
	ReceiptTbl::Rec lot_rec;
	SString fsrar_ident;
	SString temp_buf;
	SString bill_text;
	SString lot_text;
	PrcssrAlcReport::GoodsItem agi;
	assert(pX);
	THROW_PP(pX, PPERR_INVPARAM);
	THROW(p_doc_type_tag);
	THROW(GetFSRARID(locID, fsrar_ident, &main_org_id));
	if(rPack.P_Data || oneof2(doc_type, PPEDIOP_EGAIS_QUERYRESTS, PPEDIOP_EGAIS_QUERYRESTSSHOP)) {
		SXml::WDoc _doc(pX, cpUTF8);
		{
			SXml::WNode n_docs(_doc, "ns:Documents");
			n_docs.PutAttrib("Version", "1.0");
			{
				const struct EgaisNsEntry {
					int   Nn;
					const char * P_Ns;
					const char * P_Sub;
				} ns_entries[] = {
					{  1, "ns",   "WB_DOC_SINGLE_01" },
					{  2, "c",    "Common" },
					{  3, "oref", "ClientRef" },       // ambiguity
					{  4, "pref", "ProductRef" },      // ambiguity
					{  5, "wb",   "TTNSingle" },
					{  6, "qp",   "QueryParameters" }, // ambiguity
					{  7, "qp",   "RequestRepealWB" }, // ambiguity
					{  8, "wa",   "ActTTNSingle" },
					{  9, "ain",  "ActChargeOn" },
					{ 10, "iab",  "ActInventoryABInfo" }, // ambiguity
					{ 11, "wt",   "ConfirmTicket" },      // ambiguity
					{ 12, "wt",   "ConfirmRepealWB" },    // ambiguity
					{ 13, "awr",  "ActWriteOff" }, // ambiguity
					{ 14, "qf",   "QueryFormAB" },
					{ 15, "bk",   "QueryBarcode" },
					{ 16, "ce",   "CommonEnum" },
					{ 17, "pref", "ProductRef_v2" }, // ambiguity
					{ 18, "tts",  "TransferToShop" },
					{ 19, "ainp", "ActChargeOnShop_v2" }, // ambiguity
					{ 20, "oref", "ClientRef_v2" }, // ambiguity
					{ 21, "tfs",  "TransferFromShop" },
					{ 22, "ainp", "ActChargeOn_v2" }, // ambiguity
					{ 23, "iab",  "ActInventoryF1F2Info" }, // ambiguity
					{ 24, "awr",  "ActWriteOffShop_v2" } // ambiguity
				};
				const SString fsrar_url_prefix = "http://fsrar.ru/WEGAIS/";
				n_docs.PutAttrib("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
				for(uint eidx = 0; eidx < SIZEOFARRAY(ns_entries); eidx++) {
					const EgaisNsEntry & r_entry = ns_entries[eidx];
					int    skip = 0;
					//
					// ���������� ���������������� � namespace'��
					//
					switch(r_entry.Nn) {
						case  3: skip = BIN(oneof3(doc_type, PPEDIOP_EGAIS_ACTCHARGEONSHOP, PPEDIOP_EGAIS_ACTCHARGEON_V2, PPEDIOP_EGAIS_ACTWRITEOFFSHOP)); break; // "oref" "ClientRef"
						case  4: skip = BIN(oneof5(doc_type, PPEDIOP_EGAIS_TRANSFERTOSHOP, PPEDIOP_EGAIS_ACTCHARGEONSHOP, PPEDIOP_EGAIS_TRANSFERFROMSHOP, PPEDIOP_EGAIS_ACTCHARGEON_V2, PPEDIOP_EGAIS_ACTWRITEOFFSHOP)); break; // "pref" "ProductRef"
						case  7: skip = BIN(doc_type != PPEDIOP_EGAIS_REQUESTREPEALWB); break;
						case  6: skip = BIN(doc_type == PPEDIOP_EGAIS_REQUESTREPEALWB); break;
						case 10: skip = BIN(doc_type == PPEDIOP_EGAIS_ACTCHARGEON_V2); break; // "iab"
						case 12: skip = BIN(doc_type != PPEDIOP_EGAIS_CONFIRMREPEALWB); break;
						case 13: skip = BIN(oneof2(doc_type, PPEDIOP_EGAIS_CONFIRMREPEALWB, PPEDIOP_EGAIS_ACTWRITEOFFSHOP)); break; // "awr"
						case 17: skip = BIN(!oneof5(doc_type, PPEDIOP_EGAIS_TRANSFERTOSHOP, PPEDIOP_EGAIS_ACTCHARGEONSHOP, PPEDIOP_EGAIS_TRANSFERFROMSHOP, PPEDIOP_EGAIS_ACTCHARGEON_V2, PPEDIOP_EGAIS_ACTWRITEOFFSHOP)); break; // "pref" "ProductRef_v2"
						case 19: skip = BIN(doc_type == PPEDIOP_EGAIS_ACTCHARGEON_V2); break; // "ainp"
						case 20: skip = BIN(!oneof3(doc_type, PPEDIOP_EGAIS_ACTCHARGEONSHOP, PPEDIOP_EGAIS_ACTCHARGEON_V2, PPEDIOP_EGAIS_ACTWRITEOFFSHOP)); break; // "oref" "ClientRef_v2"
						case 22: skip = BIN(doc_type != PPEDIOP_EGAIS_ACTCHARGEON_V2); break; // "ainp"
						case 23: skip = BIN(doc_type != PPEDIOP_EGAIS_ACTCHARGEON_V2); break; // "iab"
						case 24: skip = BIN(doc_type != PPEDIOP_EGAIS_ACTWRITEOFFSHOP); break; // "awr"
					}
					if(!skip) {
						(bill_text = 0).Cat("xmlns").CatChar(':').Cat(r_entry.P_Ns); // bill_text as temporary buffer
						n_docs.PutAttrib(bill_text, (temp_buf = fsrar_url_prefix).Cat(r_entry.P_Sub));
					}
				}
			}
			{
				SXml::WNode n_owner(_doc, "ns:Owner");
				n_owner.PutInner("ns:FSRAR_ID", EncText(fsrar_ident)); // ��� ����� �����������
			}
			{
				SXml::WNode n_d(_doc, "ns:Document");
				{
					(temp_buf = "ns").CatChar(':').Cat(p_doc_type_tag);
					SXml::WNode n_dt(_doc, temp_buf);
					if(doc_type == PPEDIOP_EGAIS_WAYBILL) {
						const  PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						int    wb_type = 0;
						PPOprKind op_rec;
						PPOprKind link_op_rec;
						THROW(GetOpData(p_bp->Rec.OpID, &op_rec) > 0);
						PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text = 0);
						if(op_rec.LinkOpID) {
							THROW(GetOpData(op_rec.LinkOpID, &link_op_rec) > 0);
						}
						n_dt.PutInner("wb:Identity", (temp_buf = 0).Cat(p_bp->Rec.ID));
                        {
                        	SXml::WNode n_h(_doc, "wb:Header");
                        	temp_buf = 0;
                        	PPID   consignee_psn_id = 0;
                        	PPID   consignee_loc_id = 0;
                        	PPID   shipper_psn_id = 0;
                        	PPID   shipper_loc_id = 0;
                        	PPID   suppl_psn_id = 0;
                        	PPID   suppl_loc_id = 0;
                        	int    do_skip = 0;
                        	int    is_intrexpend = 0;
                        	if(rPack.Flags & PPEgaisProcessor::Packet::fReturnBill) {
								wb_type = wbtRetFromMe;
								consignee_psn_id = ObjectToPerson(p_bp->Rec.Object, 0);
								consignee_loc_id = p_bp->P_Freight ? p_bp->P_Freight->DlvrAddrID : 0;
								shipper_psn_id = main_org_id;
								shipper_loc_id = p_bp->Rec.LocID;
                        	}
                        	else if(oneof2(p_bp->OprType, PPOPT_GOODSEXPEND, PPOPT_DRAFTEXPEND)) {
								wb_type = wbtInvcFromMe;
								PPID   ar2_main_org_id = 0;
								if(op_rec.AccSheet2ID && p_bp->Rec.Object2) {
									PPAccSheet acs_rec;
									if(AcsObj.Fetch(op_rec.AccSheet2ID, &acs_rec) > 0 && acs_rec.Assoc == PPOBJ_PERSON && acs_rec.ObjGroup == PPPRK_MAIN)
										ar2_main_org_id = ObjectToPerson(p_bp->Rec.Object2, 0);
								}
								if(IsIntrExpndOp(p_bp->Rec.OpID)) {
									consignee_psn_id = main_org_id;
									consignee_loc_id = PPObjLocation::ObjToWarehouse(p_bp->Rec.Object);
									shipper_psn_id = main_org_id;
									shipper_loc_id = p_bp->Rec.LocID;
									is_intrexpend = 1;
								}
								else {
									consignee_psn_id = ObjectToPerson(p_bp->Rec.Object, 0);
									consignee_loc_id = p_bp->P_Freight ? p_bp->P_Freight->DlvrAddrID : 0;
									shipper_psn_id = main_org_id;
									shipper_loc_id = p_bp->Rec.LocID;
								}
								if(consignee_psn_id == shipper_psn_id) {
									RegisterTbl::Rec reg_rec;
									SString consignee_kpp, shipper_kpp;
									if(GetWkrRegister(wkrKPP, shipper_psn_id, shipper_loc_id, p_bp->Rec.Dt, &reg_rec) > 0)
										(shipper_kpp = reg_rec.Num).ToLower();
									if(GetWkrRegister(wkrKPP, consignee_psn_id, consignee_loc_id, p_bp->Rec.Dt, &reg_rec) > 0)
										(consignee_kpp = reg_rec.Num).ToLower();
                                    if(shipper_kpp == consignee_kpp) {
										LogTextWithAddendum(PPTXT_EGAIS_SELFTRANSFBILL, bill_text);
										do_skip = 1;
                                    }
								}
                        	}
                        	else if(oneof2(p_bp->OprType, PPOPT_GOODSRECEIPT, PPOPT_DRAFTRECEIPT)) {
                        		wb_type = wbtInvcToMe;
								consignee_psn_id = main_org_id;
								consignee_loc_id = p_bp->Rec.LocID;
								shipper_psn_id = ObjectToPerson(p_bp->Rec.Object, 0);
								shipper_loc_id = p_bp->P_Freight ? p_bp->P_Freight->DlvrAddrID : 0;
                        	}
                        	else if(p_bp->OprType == PPOPT_GOODSRETURN) {
								if(op_rec.LinkOpID) {
									if(link_op_rec.OpTypeID == PPOPT_GOODSRECEIPT) {
										wb_type = wbtRetFromMe;
										consignee_psn_id = ObjectToPerson(p_bp->Rec.Object, 0);
										consignee_loc_id = p_bp->P_Freight ? p_bp->P_Freight->DlvrAddrID : 0;
										shipper_psn_id = main_org_id;
										shipper_loc_id = p_bp->Rec.LocID;
									}
									else if(link_op_rec.OpTypeID == PPOPT_GOODSEXPEND) {
										wb_type = wbtRetToMe;
										consignee_psn_id = main_org_id;
										consignee_loc_id = p_bp->Rec.LocID;
										shipper_psn_id = ObjectToPerson(p_bp->Rec.Object, 0);
										shipper_loc_id = p_bp->P_Freight ? p_bp->P_Freight->DlvrAddrID : 0;
									}
								}
                        	}
                        	if(wb_type && !do_skip) {
								enum {
									gppfAlc      = 0x0001, // � ��������� ���� ����������� ������
									gppfPacked   = 0x0002, // � ��������� ���� ����������� ����������� ������
									gppfUnpacked = 0x0004  // � ��������� ���� ����������� ������������� ������
								};
								long   gppf = 0;
								PrcssrAlcReport::GoodsItem agi;
								for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
									const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
									if(IsAlcGoods(r_ti.GoodsID)) {
										gppf |= gppfAlc;
										if(PreprocessGoodsItem(r_ti.GoodsID, r_ti.LotID, 0, 0, agi) > 0)
											gppf |= ((agi.UnpackedVolume > 0.0) ? gppfUnpacked : gppfPacked);
									}
								}
								THROW_PP_S(gppf & gppfAlc, PPERR_EGAIS_NOALCINBILL, bill_text);
								THROW_PP_S((gppf & (gppfPacked|gppfUnpacked)) != (gppfPacked|gppfUnpacked), PPERR_EGAIS_PKUPKMIXINBILL, bill_text);
								temp_buf = GetWayBillTypeText(wb_type);
								THROW(temp_buf.NotEmpty());
								n_h.PutInner("wb:Type", temp_buf);
								BillCore::GetCode(temp_buf = p_bp->Rec.Code);
								n_h.PutInner("wb:NUMBER", EncText(temp_buf));
								n_h.PutInner("wb:Date", (temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY));
								n_h.PutInner("wb:ShippingDate", (temp_buf = 0).
									Cat((p_bp->P_Freight && checkdate(p_bp->P_Freight->IssueDate, 0)) ? p_bp->P_Freight->IssueDate : p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY));
								if(gppf & gppfUnpacked)
									n_h.PutInner("wb:UnitType", "Unpacked");
								else
									n_h.PutInner("wb:UnitType", "Packed");
								THROW(WriteOrgInfo(_doc, "wb:Shipper", shipper_psn_id, shipper_loc_id, p_bp->Rec.Dt, woifStrict));
								THROW(WriteOrgInfo(_doc, "wb:Consignee", consignee_psn_id, consignee_loc_id, p_bp->Rec.Dt, woifStrict));
								if(suppl_psn_id) {
									THROW(WriteOrgInfo(_doc, "wb:Supplier", suppl_psn_id, suppl_loc_id, p_bp->Rec.Dt, woifStrict));
								}
								{
									SXml::WNode n_tr(_doc, "wb:Transport");
									n_tr.PutInner("wb:TRAN_TYPE", "413"); // ���� �� �������. �� ���� �������� ������ ��� ��������.
									temp_buf = 0;
									if(p_bp->P_Freight)
										GetPersonName(p_bp->P_Freight->AgentID, temp_buf);
									n_tr.PutInnerSkipEmpty("wb:TRAN_COMPANY", EncText(temp_buf));
									temp_buf = 0;
									if(p_bp->P_Freight)
										GetObjectName(PPOBJ_TRANSPORT, p_bp->P_Freight->ShipID, temp_buf);
									n_tr.PutInnerSkipEmpty("wb:TRAN_CAR", EncText(temp_buf));
									n_tr.PutInnerSkipEmpty("wb:TRAN_TRAILER", "");
									n_tr.PutInnerSkipEmpty("wb:TRAN_CUSTOMER", "");
									temp_buf = 0;
									if(p_bp->P_Freight)
										GetPersonName(p_bp->P_Freight->CaptainID, temp_buf);
									n_tr.PutInnerSkipEmpty("wb:TRAN_DRIVER", EncText(temp_buf));
									temp_buf = 0;
									if(p_bp->P_Freight)
                                        GetObjectName(PPOBJ_WORLD, p_bp->P_Freight->PortOfLoading, temp_buf);
									n_tr.PutInnerSkipEmpty("wb:TRAN_LOADPOINT", EncText(temp_buf));
									temp_buf = 0;
									if(p_bp->P_Freight)
                                        GetObjectName(PPOBJ_WORLD, p_bp->P_Freight->PortOfDischarge, temp_buf);
									n_tr.PutInnerSkipEmpty("wb:TRAN_UNLOADPOINT", EncText(temp_buf));
									n_tr.PutInnerSkipEmpty("wb:TRAN_REDIRECT", "");
									n_tr.PutInnerSkipEmpty("wb:TRAN_FORWARDER", "");
								}
								n_h.PutInnerSkipEmpty("wb:Base", ""); // ���������
								{
									temp_buf = 0;
									if(is_intrexpend) {
										temp_buf.CatEq(P_IntrExpndNotePrefix, p_bp->Rec.ID);
									}
									/* @v8.9.10 ���������� �� ����� ���������� ���� ���������� � �����
									if(p_bp->Rec.Memo[0])
										temp_buf.CatDiv('-', 1, 1).Cat(p_bp->Rec.Memo);
									*/
									n_h.PutInnerSkipEmpty("wb:Note", EncText(temp_buf)); // ����������
								}
                        	}
                        }
                        {
                        	SXml::WNode w_c(_doc, "wb:Content");
                        	for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								if(IsAlcGoods(r_ti.GoodsID) && PreprocessGoodsItem(r_ti.GoodsID, r_ti.LotID, 0, 0, agi) > 0) {
									SXml::WNode w_p(_doc, "wb:Position");
									w_p.PutInner("wb:Identity", EncText((temp_buf = 0).Cat(r_ti.RByBill)));
									THROW(WriteProductInfo(_doc, "wb:Product", r_ti.GoodsID, r_ti.LotID, wpifPutManufInfo, 0))
									w_p.PutInnerSkipEmpty("wb:Pack_ID", "");
									{
										double qtty = fabs(r_ti.Qtty());
										double price = (wb_type == wbtRetFromMe) ? r_ti.Cost : fabs(r_ti.NetPrice());
										if(agi.UnpackedVolume > 0.0) {
											const double mult = agi.UnpackedVolume / 10.0;
											qtty = (qtty * mult); // ������������� ��������� ���������� � ����������
											price = (price / mult);
										}
										w_p.PutInner("wb:Quantity", EncText((temp_buf = 0).Cat(qtty, MKSFMTD(0, 0, NMBF_NOTRAILZ))));
										w_p.PutInner("wb:Price", EncText((temp_buf = 0).Cat(price, MKSFMTD(0, 2, 0))));
									}
									p_bp->SnL.GetNumber(tidx, &temp_buf);
									w_p.PutInnerSkipEmpty("wb:Party", EncText(temp_buf));
									{
										MEMSZERO(lot_rec);
										P_BObj->trfr->Rcpt.Search(r_ti.LotID, &lot_rec);
										P_BObj->MakeLotText(&lot_rec, PPObjBill::ltfGoodsName, temp_buf);
										(lot_text = 0).CatChar('[').Cat(r_ti.RByBill).CatChar(']').Space().Cat(temp_buf);
										//
										THROW_PP_S(p_bp->LTagL.GetTagStr(tidx, PPTAG_LOT_FSRARINFA, temp_buf) > 0, PPERR_EGAIS_NOINFAIDINLOT, lot_text);
										WriteInformCode(_doc, "wb", 'A', temp_buf);
										//
										THROW_PP_S(p_bp->LTagL.GetTagStr(tidx, PPTAG_LOT_FSRARINFB, temp_buf) > 0, PPERR_EGAIS_NOINFBIDINLOT, lot_text);
										WriteInformCode(_doc, "wb", 'B', temp_buf);
									}
								}
                        	}
                        }
					}
					else if(oneof6(doc_type, PPEDIOP_EGAIS_QUERYAP, PPEDIOP_EGAIS_QUERYCLIENTS, PPEDIOP_EGAIS_QUERYRESTS,
						PPEDIOP_EGAIS_QUERYSP, PPEDIOP_EGAIS_QUERYSSP, PPEDIOP_EGAIS_QUERYRESTSSHOP)) {
						const StrStrAssocArray * p_param_list = (const StrStrAssocArray *)rPack.P_Data;
						if(p_param_list && p_param_list->getCount()) {
							SXml::WNode n_arglist(_doc, "qp:Parameters");
							for(uint i = 0; i < p_param_list->getCount(); i++) {
								StrStrAssocArray::Item par = p_param_list->at(i);
								SXml::WNode n_p(_doc, "qp:Parameter");
								n_p.PutInner("qp:Name", EncText(temp_buf = par.Key));
								n_p.PutInner("qp:Value", EncText(temp_buf = par.Val));
							}
						}
					}
					else if(oneof2(doc_type, PPEDIOP_EGAIS_QUERYFORMA, PPEDIOP_EGAIS_REPLYFORMB)) {
						const SString * p_formab_regid = (const SString *)rPack.P_Data;
                        if(p_formab_regid->NotEmpty()) {
							n_dt.PutInner("qf:FormRegId", EncText(temp_buf = *p_formab_regid));
                        }
                        else {
							PPSetError(PPERR_EGAIS_QUERYFORMREGIDEMPTY);
							LogLastError();
                        }
					}
					else if(doc_type == PPEDIOP_EGAIS_QUERYBARCODE) {
                        const TSCollection <QueryBarcode> * p_qbl = (const TSCollection <QueryBarcode> *)rPack.P_Data;
                        if(p_qbl->getCount()) {
							const LDATETIME cdtm = getcurdatetime_();
							n_dt.PutInner("bk:QueryNumber", (temp_buf = 0).Cat((cdtm.d.v % 1000) * 1000 + (cdtm.t.v % 1000)));
							n_dt.PutInner("bk:Date", (temp_buf = 0).Cat(cdtm, DATF_ISO8601|DATF_CENTURY, 0));
							{
								long   global_row_id = 0;
								LongArray row_id_list;
								SXml::WNode n_items(_doc, "bk:Marks");
								for(uint qbidx = 0; qbidx < p_qbl->getCount(); qbidx++) {
									const QueryBarcode * p_qb = p_qbl->at(qbidx);
									if(p_qb && p_qb->CodeType && p_qb->Rank.NotEmpty() && p_qb->Number.NotEmpty()) {
										SXml::WNode n_mark(_doc, "bk:Mark");
										{
											long  row_id = p_qb->RowId;
											if(!row_id)
												row_id = ++global_row_id;
											else
												global_row_id = row_id;
											while(row_id_list.lsearch(row_id)) {
												row_id = ++global_row_id;
											}
											n_mark.PutInner("bk:Identity", (temp_buf = 0).Cat(row_id));
											n_mark.PutInner("bk:Type",   (temp_buf = 0).CatLongZ(p_qb->CodeType, 3));
											n_mark.PutInner("bk:Rank",   (temp_buf = 0).Cat(p_qb->Rank));
											n_mark.PutInner("bk:Number", (temp_buf = 0).Cat(p_qb->Number));
											row_id_list.add(row_id);
										}
									}
								}
							}
                        }
					}
					else if(doc_type == PPEDIOP_EGAIS_CONFIRMTICKET) {
                        ConfirmTicket * p_ticket = (ConfirmTicket *)rPack.P_Data;
						SXml::WNode n_h(_doc, "wt:Header");
						n_h.PutInner("wt:IsConfirm", p_ticket->Conclusion ? "Accepted" : "Rejected");
						n_h.PutInner("wt:TicketNumber", EncText(temp_buf = p_ticket->Code));
						n_h.PutInner("wt:TicketDate", (temp_buf = 0).Cat(p_ticket->Date, DATF_ISO8601|DATF_CENTURY));
						n_h.PutInner("wt:WBRegId", EncText(temp_buf = p_ticket->RegIdent));
						n_h.PutInnerSkipEmpty("wt:Note", EncText(temp_buf = p_ticket->Comment));
					}
					else if(doc_type == PPEDIOP_EGAIS_WAYBILLACT) {
						PPBillPacket * p_bp = (PPBillPacket *)rPack.P_Data;
						PPBillPacket * p_link_bp = 0;
						PPBillPacket _link_bp;
						int    is_status_suited = 0;
						SString edi_ident;
						SString bill_code;
						BillCore::GetCode(bill_code = p_bp->Rec.Code);
						bill_code.Strip();
						PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text = 0);
						p_bp->BTagL.GetItemStr(PPTAG_BILL_EDIIDENT, edi_ident);
						THROW_PP_S(edi_ident.NotEmptyS(), PPERR_EGAIS_BILLHASNEDIIDENTTAG, bill_text);
						if(P_BObj->CheckStatusFlag(p_bp->Rec.StatusID, BILSTF_READYFOREDIACK))
							is_status_suited = 1;
						if(p_bp->Rec.EdiOp == PPEDIOP_EGAIS_WAYBILL) {
							int    cmp_result = 0; // 0 - accepted, -1 - rejected,
								// & 0x01 - ���� ������� � ������� ������� �� ����������
								// & 0x02 - ���� ������� � ������� ������� �� ����������
							int    is_uncond_accept = 0;
							PPIDArray wroff_bill_list;
							BillTbl::Rec wroff_bill_rec;
							for(DateIter diter; P_BObj->P_Tbl->EnumLinks(p_bp->Rec.ID, &diter, BLNK_WROFFDRAFT, &wroff_bill_rec) > 0;)
								wroff_bill_list.add(wroff_bill_rec.ID);
							if(wroff_bill_list.getCount() > 1)
								LogTextWithAddendum(PPTXT_EGAIS_BILLWROFFCONFL, bill_text);
							else {
								if(wroff_bill_list.getCount() == 1) {
									THROW(P_BObj->ExtractPacket(wroff_bill_list.get(0), &_link_bp) > 0);
									p_link_bp = &_link_bp;
									if(P_BObj->CheckStatusFlag(_link_bp.Rec.StatusID, BILSTF_READYFOREDIACK)) {
										is_status_suited = 1;
                                        if(ACfg.Hdr.Flags & PPAlbatrosCfgHdr::fUncondAcceptEdiIntrMov && IsIntrOp(_link_bp.Rec.OpID))
											is_uncond_accept = 1;
									}
								}
								else if(p_bp->Rec.Flags2 & BILLF2_DECLINED)
									is_status_suited = 1;
								if(is_status_suited) {
									int    recadv_status = PPEDI_RECADV_STATUS_UNDEF;
									TSCollection <PPBillPacket::TiDifferenceItem> diff_list;
									if(p_link_bp) {
										if(ACfg.Hdr.Flags & PPAlbatrosCfgHdr::fRecadvEvalByCorrBill) {
											p_link_bp->CompareTIByCorrection(PPBillPacket::tidfRByBillPrec|PPBillPacket::tidfQtty, 0, diff_list);
										}
										else {
											p_bp->CompareTI(*p_link_bp, PPBillPacket::tidfRByBillPrec|PPBillPacket::tidfQtty|
												PPBillPacket::tidfIgnoreGoods|PPBillPacket::tidfIgnoreSign, 0, diff_list);
										}
									}
									int    all_absent = 0;
									if(diff_list.getCount()) {
										LongArray absent_pos_list;
										for(uint di = 0; di < diff_list.getCount(); di++) {
											const PPBillPacket::TiDifferenceItem * p_diff_item = diff_list.at(di);
											if(p_diff_item->Flags & PPBillPacket::tidfOtherAbsent) {
												cmp_result |= 0x01;
												absent_pos_list.add(&p_diff_item->ThisPList);
											}
											if(p_diff_item->Flags & PPBillPacket::tidfThisAbsent) {
												// @v9.0.0 {
												//
												// ���� � ����� ��������� ����������� ������� ��������� ��������,
												// �� ���������� �����������, �� ���������� ��� ���������������
												//
												int    is_other_alc = 0;
												for(uint opi = 0; !is_other_alc && opi < p_diff_item->OtherPList.getCount(); opi++) {
													const PPID op_goods_id = labs(p_link_bp->ConstTI(p_diff_item->OtherPList.get(opi)).GoodsID);
													if(IsAlcGoods(op_goods_id))
														is_other_alc = 1;
												}
												if(is_other_alc)  // } @v9.0.0
													cmp_result |= 0x02;
											}
											if(p_diff_item->Flags & PPBillPacket::tidfQtty) {
												if(p_diff_item->OtherQtty < p_diff_item->ThisQtty)
													cmp_result |= 0x01;
												else if(p_diff_item->OtherQtty > p_diff_item->ThisQtty)
													cmp_result |= 0x02;
											}
										}
										absent_pos_list.sortAndUndup();
										if(absent_pos_list.getCount() >= p_bp->GetTCount())
											all_absent = 1;
									}
									if(p_bp->Rec.Flags2 & BILLF2_DECLINED) {
										LogTextWithAddendum(PPTXT_EGAIS_BILLDECLINED, bill_text);
										cmp_result = -1;
										recadv_status = PPEDI_RECADV_STATUS_REJECT;
									}
									else if(is_uncond_accept) {
										LogTextWithAddendum(PPTXT_EGAIS_BILLUNCNDACCEPTED, bill_text);
										cmp_result = 0;
										recadv_status = PPEDI_RECADV_STATUS_ACCEPT;
									}
									else if(all_absent) {
										LogTextWithAddendum(PPTXT_EGAIS_BILLFULLYREJECTED, bill_text);
										cmp_result = -1;
										recadv_status = PPEDI_RECADV_STATUS_REJECT;
									}
									else if(cmp_result & 0x01) {
										recadv_status = PPEDI_RECADV_STATUS_PARTACCEPT;
									}
									else if(cmp_result & 0x02) {
										recadv_status = PPEDI_RECADV_STATUS_ACCEPT;
									}
									else if(cmp_result == 0) {
										LogTextWithAddendum(PPTXT_EGAIS_BILLFULLYACCEPTED, bill_text);
										recadv_status = PPEDI_RECADV_STATUS_ACCEPT;
									}
									{
										SXml::WNode n_h(_doc, "wa:Header");
										if(oneof2(cmp_result, -1, 0)) {
											temp_buf = (cmp_result == -1) ? "Rejected" : "Accepted";
											n_h.PutInner("wa:IsAccept", EncText(temp_buf));
										}
										(temp_buf = bill_code).CatChar('-').Cat("ACT");
										n_h.PutInner("wa:ACTNUMBER", EncText(temp_buf));
										n_h.PutInner("wa:ActDate", (temp_buf = 0).Cat(getcurdate_(), DATF_ISO8601|DATF_CENTURY));
										BillCore::GetCode(temp_buf = p_bp->Rec.Code);
										n_h.PutInner("wa:WBRegId", EncText(temp_buf = edi_ident));
										n_h.PutInner("wa:Note", EncText(/*p_bp->Rec.Memo*/"")); // @v8.9.10 �� ����� ���������� ���� ���������� � �����
									}
									{
										SXml::WNode n_c(_doc, "wa:Content");
										if(cmp_result == -1) {
										}
										else {
											if(cmp_result & 0x01) {
												LogTextWithAddendum(PPTXT_EGAIS_BILLPARTACCEPTEDM, bill_text);
												for(uint bi = 0; bi < p_bp->GetTCount(); bi++) {
													const PPTransferItem & r_ti = p_bp->ConstTI(bi);
													const PPTransferItem * p_lti = 0;
													uint  lti_pos = 0;
													double real_qtty = 0.0;
													for(uint lbi = 0; !p_lti && lbi < p_link_bp->GetTCount(); lbi++) {
														const PPTransferItem & r_lti = p_link_bp->ConstTI(lbi);
														if(r_lti.RByBill == r_ti.RByBill) {
															p_lti = &p_link_bp->ConstTI(lbi);
															lti_pos = lbi;
														}
													}
													if(ACfg.Hdr.Flags & PPAlbatrosCfgHdr::fRecadvEvalByCorrBill) {
														double other_qtty = r_ti.Quantity_;
														if(p_lti) {
															for(uint di = 0; di < diff_list.getCount(); di++) {
																const PPBillPacket::TiDifferenceItem * p_diff_item = diff_list.at(di);
																if(p_diff_item && p_diff_item->ThisPList.lsearch(lti_pos)) {
																	other_qtty = p_diff_item->OtherQtty;
																	break;
																}
															}
														}
														real_qtty = MIN(fabs(r_ti.Quantity_), fabs(other_qtty));
													}
													else {
														real_qtty = p_lti ? MIN(fabs(r_ti.Quantity_), fabs(p_lti->Quantity_)) : 0.0;
													}
													// @v8.9.10 {
													{
														GoodsItem _agi;
														PreprocessGoodsItem(r_ti.GoodsID, 0, 0, 0, _agi);
														if(_agi.UnpackedVolume > 0.0) {
															const double mult = _agi.UnpackedVolume / 10.0;
															real_qtty = (real_qtty * mult); // ������������� ��������� ���������� � ����������
														}
													}
													// } @v8.9.10
													{
														SXml::WNode n_pos(_doc, "wa:Position");
														(temp_buf = 0).Cat(r_ti.RByBill);
														n_pos.PutInner("wa:Identity", EncText(temp_buf));
														//
														p_bp->LTagL.GetTagStr(bi, PPTAG_LOT_FSRARINFB, temp_buf);
														n_pos.PutInner("wa:InformBRegId", EncText(temp_buf));
														//
														n_pos.PutInner("wa:RealQuantity", (temp_buf = 0).Cat(real_qtty, MKSFMTD(0, 6, NMBF_NOTRAILZ)));
													}
												}
											}
											if(cmp_result & 0x02)
												LogTextWithAddendum(PPTXT_EGAIS_BILLPARTACCEPTEDP, bill_text);
										}
									}
									BillCore::SetRecadvStatus(recadv_status, p_bp->Rec); // @v8.9.5
								}
								else
									LogTextWithAddendum(PPTXT_EGAIS_BILLSTATUSNREADY, bill_text);
							}
						}
						else if(p_bp->Rec.Flags2 & BILLF2_DECLINED && is_status_suited) { // ��� �������� EdiOp - �������� ����������� ���� � �� ����� ��������� ������ ��������
							{
								SXml::WNode n_h(_doc, "wa:Header");
								n_h.PutInner("wa:IsAccept", EncText(temp_buf = "Rejected"));
								(temp_buf = bill_code).CatChar('-').Cat("REJECT");
								n_h.PutInner("wa:ACTNUMBER", EncText(temp_buf));
								n_h.PutInner("wa:ActDate", (temp_buf = 0).Cat(getcurdate_(), DATF_ISO8601|DATF_CENTURY));
								n_h.PutInner("wa:WBRegId", EncText(temp_buf = edi_ident));
								// @v8.9.10 (�� ����� ���������� ���� ���������� � �����) n_h.PutInnerSkipEmpty("wa:Note", EncText(p_bp->Rec.Memo));
							}
							{
								SXml::WNode n_c(_doc, "wa:Content");
							}
							LogTextWithAddendum(PPTXT_EGAIS_BILLSELFREJECTED, bill_text);
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						n_dt.PutInner("ainp:Identity", (temp_buf = 0).Cat(p_bp->Rec.ID));
						{
							SXml::WNode n_h(_doc, "ainp:Header");
							n_h.PutInner("ainp:Number", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("ainp:ActDate", EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
							if(p_bp->BTagL.GetItemStr(PPTAG_BILL_FORMALREASON, temp_buf) <= 0)
								(temp_buf = "���������, ���������� �� 01.01.2016").Transf(CTRANSF_OUTER_TO_INNER);
							n_h.PutInner("ainp:TypeChargeOn", EncText(temp_buf));
						}
						{
							SXml::WNode n_c(_doc, "ainp:Content");
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								if(IsAlcGoods(r_ti.GoodsID) && PreprocessGoodsItem(r_ti.GoodsID, 0, 0, 0, agi) > 0) { // @v9.4.7 && PreprocessGoodsItem>0
									double _qtty = fabs(r_ti.Quantity_);
									long   _fmt = 0;
									if(agi.UnpackedVolume > 0.0) {
										_qtty = R4(_qtty);
										_fmt = MKSFMTD(0, 4, 0);
									}
									else {
										_qtty = R0(_qtty);
									}
									SXml::WNode w_p(_doc, "ainp:Position");
									w_p.PutInner("ainp:Identity", EncText((temp_buf = 0).Cat(r_ti.RByBill)));
									const ObjTagList * p_ti_tag_list = p_bp->LTagL.Get(tidx);
									THROW(WriteProductInfo(_doc, "ainp:Product", r_ti.GoodsID, 0, wpifPutManufInfo|wpifVersion2, p_ti_tag_list))
									w_p.PutInner("ainp:Quantity", EncText((temp_buf = 0).Cat(_qtty, _fmt)));
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTWRITEOFFSHOP) { // @v9.4.0
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						n_dt.PutInner("awr:Identity", (temp_buf = 0).Cat(p_bp->Rec.ID));
						{
							SXml::WNode n_h(_doc, "awr:Header");
							n_h.PutInner("awr:ActNumber", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("awr:ActDate", EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
							if(p_bp->BTagL.GetItemStr(PPTAG_BILL_FORMALREASON, temp_buf) <= 0)
								(temp_buf = "���������").Transf(CTRANSF_OUTER_TO_INNER);
							n_h.PutInner("awr:TypeWriteOff", EncText(temp_buf));
						}
						{
							SXml::WNode n_c(_doc, "awr:Content");
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								if(IsAlcGoods(r_ti.GoodsID) && PreprocessGoodsItem(r_ti.GoodsID, 0, 0, 0, agi) > 0) { // @v9.4.7 && PreprocessGoodsItem>0
									double _qtty = fabs(r_ti.Quantity_);
									long   _fmt = 0;
									if(agi.UnpackedVolume > 0.0) {
										_qtty = R4(_qtty);
										_fmt = MKSFMTD(0, 4, 0);
									}
									else {
										_qtty = R0(_qtty);
									}
									SXml::WNode w_p(_doc, "awr:Position");
									w_p.PutInner("awr:Identity", EncText((temp_buf = 0).Cat(r_ti.RByBill)));
									const ObjTagList * p_ti_tag_list = p_bp->LTagL.Get(tidx);
									THROW(WriteProductInfo(_doc, "awr:Product", r_ti.GoodsID, 0, wpifPutManufInfo|wpifVersion2, p_ti_tag_list))
									w_p.PutInner("awr:Quantity", EncText((temp_buf = 0).Cat(_qtty, _fmt)));
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTCHARGEON_V2) { // @v9.3.12 @construction
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						{
							SXml::WNode n_h(_doc, "ainp:Header");
							n_h.PutInner("ainp:Number", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("ainp:ActDate", EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
							if(p_bp->BTagL.GetItemStr(PPTAG_BILL_FORMALREASON, temp_buf) <= 0)
								(temp_buf = "�����������").Transf(CTRANSF_OUTER_TO_INNER);
							n_h.PutInner("ainp:TypeChargeOn", EncText(temp_buf));
						}
						THROW_MEM(SETIFZ(P_LecT, new LotExtCodeTbl));
						{
							SXml::WNode n_c(_doc, "ainp:Content");
							SString infa_ident;
							SString infb_ident;
							StringSet ss_ext_codes;
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								const PPID lot_id = r_ti.LotID;
								if(lot_id && IsAlcGoods(r_ti.GoodsID)) {
									ObjTagList tag_list;
									p_ref->Ot.GetList(PPOBJ_LOT, lot_id, &tag_list);
									uint   ext_code_count = 0;
									int    ext_code_exists = 0;
									if(tag_list.GetCount()) {
										const int mmlr = Helper_MakeMarkList(lot_id, ss_ext_codes, &ext_code_count);
										THROW(mmlr);
										if(mmlr > 0)
											ext_code_exists = 1;
										SXml::WNode w_p(_doc, "ainp:Position");
										w_p.PutInner("ainp:Identity", EncText((temp_buf = 0).Cat(r_ti.RByBill)));
										THROW(WriteProductInfo(_doc, "ainp:Product", r_ti.GoodsID, lot_id, wpifPutManufInfo|wpifVersion2, 0));
										{
											//
											// �������� ����������, ��������������� �� ������.
											// ���� � ������������� � ����� ���� ���� �����, �� � �������� ���������� ��������� ���������� ���� �����.
											// � ��������� ������ ����� ���������� �� ������ ��������� (������� �� ���� �� ������ ������������ ���������).
											//
											const long org_qtty = (long)R0(fabs(r_ti.Qtty()));
											const long sqtty = /*ext_code_count*/ext_code_exists ? (long)ext_code_count : org_qtty;
											w_p.PutInner("ainp:Quantity", EncText((temp_buf = 0).Cat(sqtty)));
											if(/*ext_code_count*/ext_code_exists && sqtty != org_qtty) {
												(temp_buf = 0).CatEq("LotID", lot_id).Space().CatEq("Qtty", org_qtty).Space().CatEq("Sended Qtty", sqtty);
												LogTextWithAddendum(PPTXT_EGAIS_MARKQTYNEQLOTQTY, temp_buf);
											}
										}
										{
											const ObjTagItem * p_infaid_tag = tag_list.GetItem(PPTAG_LOT_FSRARINFA);
											const ObjTagItem * p_infbid_tag = tag_list.GetItem(PPTAG_LOT_FSRARINFB);
											SXml::WNode n_infab(_doc, "ainp:InformF1F2");
											if(tag_list.GetItemStr(PPTAG_LOT_EGIASINFAREG, temp_buf) > 0) {
												InformAReg iar;
												if(iar.FromStr(temp_buf)) {
													SXml::WNode n_infabreg(_doc, "ainp:InformF1F2Reg");
													{
														SXml::WNode n_infa(_doc, "ainp:InformF1");
														n_infa.PutInner("iab:Quantity", EncText((temp_buf = 0).Cat(iar.Qtty)));
														if(checkdate(iar.ManufDate, 0))
															n_infa.PutInner("iab:BottlingDate", EncText((temp_buf = 0).Cat(iar.ManufDate, DATF_ISO8601|DATF_CENTURY)));
														n_infa.PutInnerSkipEmpty("iab:TTNNumber", EncText(temp_buf = iar.TTNCode));
														if(checkdate(iar.TTNDate, 0))
															n_infa.PutInner("iab:TTNDate", EncText((temp_buf = 0).Cat(iar.TTNDate, DATF_ISO8601|DATF_CENTURY)));
														n_infa.PutInnerSkipEmpty("iab:EGAISFixNumber", EncText(temp_buf = iar.EGAISCode));
														if(checkdate(iar.EGAISDate, 0))
															n_infa.PutInner("iab:EGAISFixDate", EncText((temp_buf = 0).Cat(iar.EGAISDate, DATF_ISO8601|DATF_CENTURY)));
													}
												}
												else
													LogLastError();
											}
										}
										if(ext_code_count) {
											SXml::WNode n_makr(_doc, "ainp:MarkCodeInfo");
											for(uint ssp = 0; ss_ext_codes.get(&ssp, temp_buf);)
												n_makr.PutInner("ainp:MarkCode", EncText(temp_buf));
										}
									}
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTCHARGEON) {
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						{
							SXml::WNode n_h(_doc, "ain:Header");
							n_h.PutInner("ain:Number", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("ain:ActDate", EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
							// @v8.9.10 (�� ����� ���������� ���� ���������� � �����) n_h.PutInnerSkipEmpty("wb:Note", EncText(temp_buf = p_bp->Rec.Memo)); // ����������
						}
						THROW_MEM(SETIFZ(P_LecT, new LotExtCodeTbl));
						{
							SXml::WNode n_c(_doc, "ain:Content");
							SString infa_ident;
							SString infb_ident;
							StringSet ss_ext_codes;
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								const PPID lot_id = (PPID)r_ti.QuotPrice;
								if(lot_id && IsAlcGoods(r_ti.GoodsID)) {
									ObjTagList tag_list;
									p_ref->Ot.GetList(PPOBJ_LOT, lot_id, &tag_list);
									uint   ext_code_count = 0;
									int    ext_code_exists = 0;
									if(tag_list.GetCount()) {
										const int mmlr = Helper_MakeMarkList(lot_id, ss_ext_codes, &ext_code_count);
										THROW(mmlr);
										if(mmlr > 0)
											ext_code_exists = 1;
										SXml::WNode w_p(_doc, "ain:Position");
										w_p.PutInner("ain:Identity", EncText((temp_buf = 0).Cat(r_ti.RByBill)));
										THROW(WriteProductInfo(_doc, "ain:Product", r_ti.GoodsID, lot_id, 0, 0));
										{
											//
											// �������� ����������, ��������������� �� ������.
											// ���� � ������������� � ����� ���� ���� �����, �� � �������� ���������� ��������� ���������� ���� �����.
											// � ��������� ������ ����� ���������� �� ������ ��������� (������� �� ���� �� ������ ������������ ���������).
											//
											const long org_qtty = (long)R0(fabs(r_ti.Qtty()));
											const long sqtty = /*ext_code_count*/ext_code_exists ? (long)ext_code_count : org_qtty;
											w_p.PutInner("ain:Quantity", EncText((temp_buf = 0).Cat(sqtty)));
											if(/*ext_code_count*/ext_code_exists && sqtty != org_qtty) {
												(temp_buf = 0).CatEq("LotID", lot_id).Space().CatEq("Qtty", org_qtty).Space().CatEq("Sended Qtty", sqtty);
												LogTextWithAddendum(PPTXT_EGAIS_MARKQTYNEQLOTQTY, temp_buf);
											}
										}
										{
											const ObjTagItem * p_infaid_tag = tag_list.GetItem(PPTAG_LOT_FSRARINFA);
											const ObjTagItem * p_infbid_tag = tag_list.GetItem(PPTAG_LOT_FSRARINFB);
											SXml::WNode n_infab(_doc, "ain:InformAB");
											if(p_infaid_tag && p_infbid_tag && p_infaid_tag->GetStr(infa_ident) && infa_ident.NotEmptyS() &&
												p_infbid_tag->GetStr(infb_ident) && infb_ident.NotEmptyS()) {
												SXml::WNode n_infabkey(_doc, "ain:InformABKey");
												n_infabkey.PutInner("ain:FormA", EncText(infa_ident));
												n_infabkey.PutInner("ain:LastFormB", EncText(infb_ident));
											}
											else if(tag_list.GetItemStr(PPTAG_LOT_EGIASINFAREG, temp_buf) > 0) {
												InformAReg iar;
												if(iar.FromStr(temp_buf)) {
													SXml::WNode n_infabreg(_doc, "ain:InformABReg");
													{
														SXml::WNode n_infa(_doc, "ain:InformA");
														n_infa.PutInner("iab:Quantity", EncText((temp_buf = 0).Cat(iar.Qtty)));
														if(checkdate(iar.ManufDate, 0))
															n_infa.PutInner("iab:BottlingDate", EncText((temp_buf = 0).Cat(iar.ManufDate, DATF_ISO8601|DATF_CENTURY)));
														n_infa.PutInnerSkipEmpty("iab:TTNNumber", EncText(temp_buf = iar.TTNCode));
														if(checkdate(iar.TTNDate, 0))
															n_infa.PutInner("iab:TTNDate", EncText((temp_buf = 0).Cat(iar.TTNDate, DATF_ISO8601|DATF_CENTURY)));
														n_infa.PutInnerSkipEmpty("iab:EGAISFixNumber", EncText(temp_buf = iar.EGAISCode));
														if(checkdate(iar.EGAISDate, 0))
															n_infa.PutInner("iab:EGAISFixDate", EncText((temp_buf = 0).Cat(iar.EGAISDate, DATF_ISO8601|DATF_CENTURY)));
													}
												}
												else
													LogLastError();
											}
										}
										if(ext_code_count) {
											SXml::WNode n_makr(_doc, "ain:MarkCodeInfo");
											for(uint ssp = 0; ss_ext_codes.get(&ssp, temp_buf);)
												n_makr.PutInner("ain:MarkCode", EncText(temp_buf));
										}
									}
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTWRITEOFF) {
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						n_dt.PutInner("awr:Identity", (temp_buf = 0).Cat(p_bp->Rec.ID));
						{
							SXml::WNode n_h(_doc, "awr:Header");
							n_h.PutInner("awr:ActNumber", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("awr:ActDate", EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
							if(p_bp->BTagL.GetItemStr(PPTAG_BILL_FORMALREASON, temp_buf) <= 0)
								PPLoadString("losses", temp_buf); // ������
							n_h.PutInner("awr:TypeWriteOff", EncText(temp_buf));
							// @v8.9.10 (�� ����� ���������� ���� ���������� � �����) n_h.PutInnerSkipEmpty("awr:Note", EncText(temp_buf = p_bp->Rec.Memo)); // ����������
						}
						{
							SXml::WNode n_c(_doc, "awr:Content");
							SString infa_ident;
							SString infb_ident;
							StringSet ss_ext_codes;
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								if(IsAlcGoods(r_ti.GoodsID) && PreprocessGoodsItem(r_ti.GoodsID, r_ti.LotID, 0, 0, agi) > 0) {
									SXml::WNode w_p(_doc, "awr:Position");
									w_p.PutInner("awr:Identity", EncText((temp_buf = 0).Cat(r_ti.RByBill)));
									{
										double qtty = fabs(r_ti.Qtty());
										if(agi.UnpackedVolume > 0.0) {
											const double mult = agi.UnpackedVolume / 10.0;
											qtty = (qtty * mult); // ������������� ��������� ���������� � ����������
										}
										w_p.PutInner("awr:Quantity", EncText((temp_buf = 0).Cat(qtty, MKSFMTD(0, 0, NMBF_NOTRAILZ))));
									}
									{
										MEMSZERO(lot_rec);
										P_BObj->trfr->Rcpt.Search(r_ti.LotID, &lot_rec);
										P_BObj->MakeLotText(&lot_rec, PPObjBill::ltfGoodsName, temp_buf);
										(lot_text = 0).CatChar('[').Cat(r_ti.RByBill).CatChar(']').Space().Cat(temp_buf);
										//
										THROW_PP_S(p_bp->LTagL.GetTagStr(tidx, PPTAG_LOT_FSRARINFB, temp_buf) > 0, PPERR_EGAIS_NOINFBIDINLOT, lot_text);
										{
											SXml::WNode w_refb(_doc, "awr:InformB");
											w_refb.PutInner("pref:BRegId", temp_buf);
										}
									}
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_TRANSFERTOSHOP) {
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						n_dt.PutInner("tts:Identity", (temp_buf = 0).Cat(p_bp->Rec.ID));
						{
							SXml::WNode n_h(_doc, "tts:Header");
							n_h.PutInner("tts:TransferNumber", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("tts:TransferDate",   EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
						}
						{
							SXml::WNode n_c(_doc, "tts:Content");
							SString infb_ident;
							SString rar_product_ident;
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								if(r_ti.Quantity_ > 0.0 && IsAlcGoods(r_ti.GoodsID) && PreprocessGoodsItem(r_ti.GoodsID, 0, 0, 0, agi) > 0) {
									const double qtty = fabs(r_ti.Quantity_);
									const ObjTagList * p_ti_tag_list = p_bp->LTagL.Get(tidx);
									infb_ident = 0;
									rar_product_ident = 0;
									if(p_ti_tag_list) {
										p_ti_tag_list->GetItemStr(PPTAG_LOT_FSRARINFB, infb_ident);
										p_ti_tag_list->GetItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, rar_product_ident);
									}
									if(rar_product_ident.Empty())
										rar_product_ident = agi.EgaisCode;
									if(rar_product_ident.NotEmpty() && infb_ident.NotEmpty()) {
										SXml::WNode w_p(_doc, "tts:Position");
										w_p.PutInner("tts:Identity",    EncText((temp_buf = 0).Cat(r_ti.RByBill)));
										w_p.PutInner("tts:ProductCode", EncText(temp_buf = rar_product_ident));
										w_p.PutInner("tts:Quantity", EncText((temp_buf = 0).Cat(qtty, MKSFMTD(0, 0, NMBF_NOTRAILZ))));
										{
											SXml::WNode w_refb(_doc, "tts:InformF2");
											w_refb.PutInner("pref:F2RegId", EncText(temp_buf = infb_ident));
										}
									}
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_TRANSFERFROMSHOP) {
						const PPBillPacket * p_bp = (const PPBillPacket *)rPack.P_Data;
						n_dt.PutInner("tfs:Identity", (temp_buf = 0).Cat(p_bp->Rec.ID));
						{
							SXml::WNode n_h(_doc, "tfs:Header");
							n_h.PutInner("tfs:TransferNumber", EncText(temp_buf = p_bp->Rec.Code));
							n_h.PutInner("tfs:TransferDate",   EncText((temp_buf = 0).Cat(p_bp->Rec.Dt, DATF_ISO8601|DATF_CENTURY)));
						}
						{
							SXml::WNode n_c(_doc, "tfs:Content");
							SString infb_ident;
							SString rar_product_ident;
							for(uint tidx = 0; tidx < p_bp->GetTCount(); tidx++) {
								const PPTransferItem & r_ti = p_bp->ConstTI(tidx);
								if(r_ti.Quantity_ < 0.0 && IsAlcGoods(r_ti.GoodsID) && PreprocessGoodsItem(r_ti.GoodsID, 0, 0, 0, agi) > 0) {
									const double qtty = fabs(r_ti.Quantity_);
									const ObjTagList * p_ti_tag_list = p_bp->LTagL.Get(tidx);
									infb_ident = 0;
									rar_product_ident = 0;
									if(p_ti_tag_list) {
										p_ti_tag_list->GetItemStr(PPTAG_LOT_FSRARINFB, infb_ident);
										p_ti_tag_list->GetItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, rar_product_ident);
									}
									if(rar_product_ident.Empty())
										rar_product_ident = agi.EgaisCode;
									if(rar_product_ident.NotEmpty() && infb_ident.NotEmpty()) {
										SXml::WNode w_p(_doc, "tfs:Position");
										w_p.PutInner("tfs:Identity",    EncText((temp_buf = 0).Cat(r_ti.RByBill)));
										w_p.PutInner("tfs:ProductCode", EncText(temp_buf = rar_product_ident));
										w_p.PutInner("tfs:Quantity", EncText((temp_buf = 0).Cat(qtty, MKSFMTD(0, 0, NMBF_NOTRAILZ))));
										{
											SXml::WNode w_refb(_doc, "tfs:InformF2");
											w_refb.PutInner("pref:F2RegId", EncText(temp_buf = infb_ident));
										}
									}
								}
							}
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_REQUESTREPEALWB) {
						const RepealWb * p_rwb = (const RepealWb *)rPack.P_Data;
						/*
						<qp:ClientId>030000194005</qp:ClientId>
						<qp:RequestNumber>011</qp:RequestNumber>
						<qp:RequestDate>2016-05-06T13:00:00</qp:RequestDate>
						<qp:WBRegId>TTN-0021795603</qp:WBRegId>
						*/
						n_dt.PutInner("qp:ClientId", EncText(fsrar_ident));
						n_dt.PutInner("qp:RequestNumber", EncText((temp_buf = 0).Cat(p_rwb->ReqNumber)));
						n_dt.PutInner("qp:RequestDate", (temp_buf = 0).Cat(getcurdatetime_(), DATF_ISO8601|DATF_CENTURY, 0));
						n_dt.PutInner("qp:WBRegId", EncText((temp_buf = 0).Cat(p_rwb->TTNCode)));
					}
					else if(doc_type == PPEDIOP_EGAIS_CONFIRMREPEALWB) {
						const RepealWb * p_rwb = (const RepealWb *)rPack.P_Data;
						/*
							<wt:Header>
							<wt:IsConfirm>Accepted</wt:IsConfirm>
							<wt:ConfirmNumber>0001</wt:ConfirmNumber>
							<wt:ConfirmDate>2014-12-17</wt:ConfirmDate>
							<wt:WBRegId>TTN-0021795603</wt:WBRegId>
							<wt:Note>������������ ������ ���������� ���</wt:Note>
							</wt:Header>
						*/
						SXml::WNode n_c(_doc, "wt:Header");
						n_c.PutInner("wt:IsConfirm", p_rwb->Confirm ? "Accepted" : "Rejected");
						n_c.PutInner("wt:ConfirmNumber", EncText((temp_buf = 0).Cat(p_rwb->ReqNumber)));
						n_c.PutInner("wt:ConfirmDate", (temp_buf = 0).Cat(getcurdate_(), DATF_ISO8601|DATF_CENTURY));
						n_c.PutInner("wt:WBRegId", EncText((temp_buf = 0).Cat(p_rwb->TTNCode)));
						n_c.PutInnerSkipEmpty("wt:Note", EncText(p_rwb->Memo));
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTUSING) {
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTREJECT) {
					}
					else {
					}
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

struct EgaisGoodsNameReplacement {
	const char * P_Pattern;
	const char * P_Subst;
};

static EgaisGoodsNameReplacement _GoodsNameReplacement[] = {
	{ "�������", "�����" },
	{ "���������", "�������" },
	{ "�����������", "��������" },
	{ "����������� ������������ ����� �������������", " "},
	{ "����������� ��������������� ��������", " " },
	{ "� ���������� �������������� ���������", " " },
	{ "����������� ��������������� ��������", " " },
	{ "��������������� ��������", " " },
	{ "� ���������� ������������� ����� �������������", " " },
	{ "������� �����������������, ������������� �� ������ ����", "������ ������� �����������������" },
	{ "�������, ������������� �� ������ ����", "������ �������" },
	{ "������������� �� ������ ����", "�� ������ ����" },
	{ "�������� ���� ��������� ������", " " },
	{ "� ����������� ������ ������������", "�����" },
	{ "���� ��������� ����������������� ������������", "���� ��������� �������� ���" },
	{ "������� ������ ( blazer strong)", " ������� ������ " },
	{ "<�������> (<blazer>)", "�������" },
	{ "<����� ���> (<marty ray>)", "����� ���" },
    { "�� ������", "����" },
	{ "���������������", "��������" },
	{ "�� ���������������������", "����" },
	{ "���������������", "��������" },
	{ "���������������", "������" },
	{ "hefe-weissier naturtrub (�������� ����-�������� ���������)", "hefe-weissier naturtrub" },
	{ "\"", " " },
	{ " ,", "," },
	{ "  ", " " }
};

SString & FASTCALL PPEgaisProcessor::PreprocessGoodsName(SString & rName) const
{
	rName.Strip().ToLower1251();
	for(uint i = 0; i < SIZEOFARRAY(_GoodsNameReplacement); i++)
        rName.ReplaceStr(_GoodsNameReplacement[i].P_Pattern, _GoodsNameReplacement[i].P_Subst, 0);
	return rName.Strip();
}

int SLAPI PPEgaisProcessor::Read_ProductInfo(xmlNode * pFirstNode, PPGoodsPacket * pPack,
	PrcssrAlcReport::GoodsItem * pExt, PrcssrAlcReport::RefCollection * pRefC, SFile * pOutFile)
{
	int    ok = 1;
	PPID   goods_id = 0;
	PrcssrAlcReport::GoodsItem alc_goods_item;
	SString temp_buf;
	SString full_name, short_name;
	//SString rar_ident;
	//SString ap_code; // ��� ����������� ��������� (�� ������ � rar_ident - ����� ����������� ������������)
	const PPID manuf_tag_id = Cfg.LotManufTagList.getCount() ? Cfg.LotManufTagList.get(0) : 0;
	PPID   manuf_psn_kind_id = PPPRK_MANUF;
	int    manuf_refc_pos = -1;
	int    imp_refc_pos = -1;
	PPPersonPacket psn_manuf;
	PPPersonPacket psn_imp;
	// @v9.3.5 {
	if(pRefC)
		pRefC->LastProductP = -1;
	// } @v9.3.5
	if(manuf_tag_id) {
		PPObjectTag tag_rec;
        if(TagObj.Fetch(manuf_tag_id, &tag_rec) > 0) {
			if(tag_rec.TagDataType == OTTYP_OBJLINK && tag_rec.TagEnumID == PPOBJ_PERSON && tag_rec.LinkObjGrp)
				manuf_psn_kind_id = tag_rec.LinkObjGrp;
        }
	}
	for(xmlNode * p_n = pFirstNode; p_n; p_n = p_n->next) {
		if(SXml::IsName(p_n, "Identity"))
			;
		else if(SXml::IsName(p_n, "Type"))
			; // �� ���������� - ����� ������ "��"
		else if(SXml::GetContentByName(p_n, "FullName", full_name))
			full_name.Utf8ToChar();
		else if(SXml::GetContentByName(p_n, "ShortName", short_name))
			short_name.Utf8ToChar();
		else if(SXml::GetContentByName(p_n, "AlcCode", alc_goods_item.EgaisCode))
			;
		else if(SXml::GetContentByName(p_n, "ProductVCode", alc_goods_item.CategoryCode))
			;
		else if(SXml::GetContentByName(p_n, "Capacity", temp_buf))
			alc_goods_item.Volume = temp_buf.ToReal();
		else if(SXml::GetContentByName(p_n, "AlcVolume", temp_buf))
			alc_goods_item.Proof = temp_buf.ToReal();
		else if(SXml::IsName(p_n, "Producer")) {
			Read_OrgInfo(p_n->children, manuf_psn_kind_id, EgaisPersonCore::rolefManuf, pPack ? &psn_manuf : 0, pRefC, 0);
			if(pRefC && pRefC->LastPersonP >= 0) // @v9.4.5 @fix (>0)--(>=0)
				manuf_refc_pos = pRefC->LastPersonP;
		}
		else if(SXml::IsName(p_n, "Importer")) {
			Read_OrgInfo(p_n->children, manuf_psn_kind_id, EgaisPersonCore::rolefImporter, pPack ? &psn_imp : 0, pRefC, 0);
			if(pRefC && pRefC->LastPersonP >= 0) // @v9.4.5 @fix (>0)--(>=0)
				imp_refc_pos = pRefC->LastPersonP;
		}
	}
	if(pOutFile) {
		SString out_line_buf;
        out_line_buf.Cat(alc_goods_item.EgaisCode).Tab().Cat(alc_goods_item.CategoryCode).Tab().Cat(alc_goods_item.Volume, MKSFMTD(0, 3, 0)).Tab().
			Cat(alc_goods_item.Proof, MKSFMTD(0, 1, 0)).Tab().Cat(full_name).Tab().Cat(short_name).CR();
		pOutFile->WriteLine(out_line_buf);
	}
	{
		EgaisProductCore::Item pi;
		int16   country_code = 0;
		STRNSCPY(pi.AlcoCode, alc_goods_item.EgaisCode);
		STRNSCPY(pi.CategoryCode, alc_goods_item.CategoryCode);
		pi.Proof = alc_goods_item.Proof;
		pi.Volume = alc_goods_item.Volume;
		pi.Flags  = EgaisProductCore::fVerified; // @v9.2.14
		(pi.Name = short_name).Transf(CTRANSF_OUTER_TO_INNER);
		(pi.FullName = full_name).Transf(CTRANSF_OUTER_TO_INNER);
		if(pRefC) {
			if(manuf_refc_pos >= 0) {
				const EgaisPersonCore::Item * p_person = pRefC->PersonList.at(manuf_refc_pos);
				if(p_person) {
					STRNSCPY(pi.ManufRarIdent, p_person->RarIdent);
					country_code = p_person->CountryCode;
				}
			}
			if(imp_refc_pos >= 0) {
				const EgaisPersonCore::Item * p_person = pRefC->PersonList.at(imp_refc_pos);
				if(p_person)
					STRNSCPY(pi.ImporterRarIdent, p_person->RarIdent);
			}
			THROW(pRefC->SetProduct(pi)); // 0178140000002304670
		}
		if(pExt) {
			if(psn_imp.Rec.ID)
				pExt->MnfOrImpPsnID = psn_imp.Rec.ID;
			else if(psn_manuf.Rec.ID)
				pExt->MnfOrImpPsnID = psn_manuf.Rec.ID;
			pExt->EgaisCode = alc_goods_item.EgaisCode;
			pExt->CategoryCode = pi.CategoryCode;
			pExt->RefcManufCode = pi.ManufRarIdent;
			pExt->RefcImporterCode = pi.ImporterRarIdent;
			pExt->Proof = pi.Proof;
			pExt->Volume = pi.Volume;
			pExt->CountryCode = country_code;
		}
		if(pPack) {
			pPack->destroy();
			if(alc_goods_item.EgaisCode.NotEmptyS()) {
				int    found_by_rarcode = 0;
				SString goods_name;
				{
					//
					// ��������� ������������ ������
					//
					if(full_name.NotEmptyS())
						goods_name = full_name;
					else if(short_name.NotEmptyS())
						goods_name = short_name;
					PreprocessGoodsName(goods_name);
					if(alc_goods_item.Proof > 0.0) {
						(temp_buf = 0).Cat(alc_goods_item.Proof, MKSFMTD(0, 1, NMBF_NOTRAILZ)).CatChar('%');
						if(!goods_name.Search(temp_buf, 0, 1, 0))
							goods_name.Space().Cat(temp_buf);
					}
					if(alc_goods_item.Volume > 0.0) {
						(temp_buf = 0).Cat(alc_goods_item.Volume, MKSFMTD(0, 3, NMBF_NOTRAILZ)).Cat("�");
						if(!goods_name.Search(temp_buf, 0, 1, 0))
							goods_name.Space().Cat(temp_buf);
					}
					{
						temp_buf = alc_goods_item.EgaisCode;
						if(!goods_name.Search(temp_buf, 0, 1, 0))
							goods_name.Space().Cat(temp_buf);
					}
					goods_name.Transf(CTRANSF_OUTER_TO_INNER);
				}
				if(SearchGoodsByRarCode(alc_goods_item.EgaisCode, &goods_id) > 0) {
					THROW(GObj.GetPacket(goods_id, pPack, 0) > 0);
					found_by_rarcode = 1;
				}
				else if(ACfg.Hdr.Flags & ACfg.Hdr.fSkipBillWithUnresolvedItems) {
					LogTextWithAddendum(PPTXT_EGAIS_GOODSREJECTEDCFG, goods_name);
					ok = -1;
				}
				else {
					goods_id = 0; // @paranoic
					pPack->Rec.Kind = PPGDSK_GOODS;
					STRNSCPY(pPack->Rec.Name, goods_name);
					STRNSCPY(pPack->Rec.Abbr, goods_name);
					pPack->Rec.Flags |= GF_AUTOCREATE;
				}
				if(ok > 0) {
					SETIFZ(pPack->Rec.GdsClsID, Cfg.E.AlcGoodsClsID);
					if(pPack->Rec.GdsClsID) {
						PPGdsClsPacket gc_pack;
						THROW(GcObj.Fetch(pPack->Rec.GdsClsID, &gc_pack) > 0);
						SETIFZ(pPack->Rec.ParentID, gc_pack.Rec.DefGrpID);
						SETIFZ(pPack->Rec.UnitID, gc_pack.Rec.DefUnitID);
						if(Cfg.VolumeClsDim && alc_goods_item.Volume > 0.0) {
							double org_val = 0.0;
							gc_pack.GetExtDim(&pPack->ExtRec, Cfg.VolumeClsDim, &org_val);
							if(org_val <= 0.0)
								gc_pack.RealToExtDim(alc_goods_item.Volume, Cfg.VolumeClsDim, pPack->ExtRec);
						}
						if(Cfg.E.ProofClsDim && alc_goods_item.Proof > 0.0) {
							double org_val = 0.0;
							gc_pack.GetExtDim(&pPack->ExtRec, Cfg.E.ProofClsDim, &org_val);
							if(org_val <= 0.0)
								gc_pack.RealToExtDim(alc_goods_item.Proof, Cfg.E.ProofClsDim, pPack->ExtRec);
						}
						if(Cfg.CategoryClsDim && alc_goods_item.CategoryCode.ToLong() > 0) {
							if(oneof4(Cfg.CategoryClsDim, PPGdsCls::eX, PPGdsCls::eY, PPGdsCls::eZ, PPGdsCls::eW)) {
								double org_val = 0.0;
								gc_pack.GetExtDim(&pPack->ExtRec, Cfg.CategoryClsDim, &org_val);
								if(org_val <= 0.0)
									gc_pack.RealToExtDim(alc_goods_item.CategoryCode.ToLong(), Cfg.CategoryClsDim, pPack->ExtRec);
							}
						}
					}
					if(Cfg.CategoryTagID && alc_goods_item.CategoryCode.NotEmptyS()) {
						const ObjTagItem * p_tag_item = pPack->TagL.GetItem(Cfg.CategoryTagID);
						if(!p_tag_item) {
							THROW(pPack->TagL.PutItemStr(Cfg.CategoryTagID, alc_goods_item.CategoryCode));
						}
					}
					if(!found_by_rarcode) {
						THROW(pPack->AddCode(alc_goods_item.EgaisCode, 0, 1.0));
					}
					THROW(GObj.PutPacket(&goods_id, pPack, 1));
					if(!found_by_rarcode)
						LogTextWithAddendum(PPTXT_EGAIS_GOODSCREATED, goods_name);
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Read_ActInventoryInformBReg(xmlNode * pFirstNode, PPEgaisProcessor::Packet * pPack)
{
	int   ok = 1;
	SString temp_buf;
	ActInformItem * p_item = 0;
	if(pPack && pPack->P_Data) {
		PPEgaisProcessor::ActInform * p_pack = (PPEgaisProcessor::ActInform *)pPack->P_Data;
		for(xmlNode * p_n = pFirstNode; p_n; p_n = p_n->next) {
			if(SXml::IsName(p_n, "Header")) {
				for(xmlNode * p_a = p_n->children; p_a; p_a = p_a->next) {
					if(SXml::GetContentByName(p_a, "ActRegId", temp_buf))
						(p_pack->ActRegId = temp_buf).Transf(CTRANSF_UTF8_TO_INNER);
					else if(SXml::GetContentByName(p_a, "Number", temp_buf))
						(p_pack->ActNumber = temp_buf).Transf(CTRANSF_UTF8_TO_INNER);
				}
			}
			else if(SXml::IsName(p_n, "Content")) {
				for(xmlNode * p_a = p_n->children; p_a; p_a = p_a->next) {
					if(SXml::IsName(p_a, "Position")) {
						THROW_MEM(p_item = new ActInformItem);
						for(xmlNode * p_p = p_a->children; p_p; p_p = p_p->next) {
							if(SXml::GetContentByName(p_p, "Identity", temp_buf))
								p_item->P = temp_buf.ToLong();
							else if(SXml::GetContentByName(p_p, "InformARegId", temp_buf))
								STRNSCPY(p_item->AIdent, temp_buf.Transf(CTRANSF_UTF8_TO_INNER));
							else if(SXml::IsName(p_p, "InformB")) {
								for(xmlNode * p_b = p_p->children; p_b; p_b = p_b->next) {
									if(SXml::IsName(p_b, "InformBItem")) {
										InformBItem bitem;
										for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
											if(SXml::GetContentByName(p_i, "Identity", temp_buf))
												bitem.P = temp_buf.ToLong();
											else if(SXml::GetContentByName(p_i, "BRegId", temp_buf))
												STRNSCPY(bitem.Ident, temp_buf.Transf(CTRANSF_UTF8_TO_INNER));
										}
										THROW_SL(p_item->BItems.insert(&bitem));
									}
								}
							}
						}
						p_pack->Items.insert(p_item);
						p_item = 0;
					}
				}
			}
		}
	}
	CATCHZOK
	delete p_item;
	return ok;
}

int SLAPI PPEgaisProcessor::AssignManufTypeToPersonPacket(PPPersonPacket & rPack, int roleFlag)
{
	int    ok = -1;
	int    manuf_type = -1;
	if(roleFlag & EgaisPersonCore::rolefManuf)
		manuf_type = 1;
	else if(roleFlag & EgaisPersonCore::rolefImporter)
		manuf_type = 2;
	if(oneof2(manuf_type, 1, 2)) {
		SString temp_buf;
		PPObjTag tag_obj;
		PPObjectTag tag_rec;
        if(Cfg.ManufImpTagID && tag_obj.Fetch(Cfg.ManufImpTagID, &tag_rec) > 0) {
			const ObjTagItem * p_item = rPack.TagL.GetItem(Cfg.ManufImpTagID);
			if(!p_item) {
				ObjTagItem new_tag_item;
				int    is_new_tag_inited = 0;
				if(tag_rec.TagDataType == OTTYP_ENUM) {
					PPTagEnumList enl(tag_rec.TagEnumID);
					if(enl.Read(tag_rec.TagEnumID)) {
                        for(uint i = 0; !is_new_tag_inited && i < enl.getCount(); i++) {
							StrAssocArray::Item item = enl.at_WithoutParent(i);
							temp_buf = item.Txt;
							long   temp_val = temp_buf.ToLong();
							if(temp_val == 1 && manuf_type == 1) {
								if(new_tag_item.SetInt(Cfg.ManufImpTagID, item.Id))
									is_new_tag_inited = 1;
							}
							else if(temp_val == 2 && manuf_type == 2) {
								if(new_tag_item.SetInt(Cfg.ManufImpTagID, item.Id))
									is_new_tag_inited = 1;
							}
                        }
					}
				}
				else if(tag_rec.TagDataType == OTTYP_INT) {
					if(new_tag_item.SetInt(Cfg.ManufImpTagID, manuf_type))
						is_new_tag_inited = 1;
				}
				else if(tag_rec.TagDataType == OTTYP_STRING) {
					if(new_tag_item.SetStr(Cfg.ManufImpTagID, (temp_buf = 0).Cat(manuf_type)))
						is_new_tag_inited = 1;
				}
				if(is_new_tag_inited) {
					THROW(rPack.TagL.PutItem(Cfg.ManufImpTagID, &new_tag_item));
					ok = 1;
				}
			}
        }
        if(ok < 0 && Cfg.E.ImporterPersonKindID) {
			if(manuf_type == 2) {
				rPack.Kinds.addUnique(Cfg.E.ImporterPersonKindID);
				ok = 1;
			}
			else if(manuf_type == 1) {
				rPack.Kinds.removeByID(Cfg.E.ImporterPersonKindID);
				ok = 1;
			}
        }
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Read_OrgInfo(xmlNode * pFirstNode, PPID personKindID, int roleFlag, PPPersonPacket * pPack,
	PrcssrAlcReport::RefCollection * pRefC, SFile * pOutFile)
{
	int   ok = 1;
	Reference * p_ref = PPRef;
	PPID  psn_id = 0;
	PPID  loc_id = 0;
	SString temp_buf;
	SString rar_ident;
	SString inn, kpp, unp, rnn;
	SString full_name, short_name;
	SString addr_descr;
	SString city;
	int    country_code = 0;
	int    region_code = 0;
	ObjTagItem tag_item;
	SString zip;
	// @v9.3.5 {
	if(pRefC)
		pRefC->LastPersonP = -1;
	// } @v9.3.5
	int    j_status = 0;
	for(xmlNode * p_n = pFirstNode; p_n; p_n = p_n->next) {
		//
		// ��� 2-� ������ ��������� ���� �������������� ����������� ���� ���������������� ������������ ������ ����������
		//
		int    is_v2_js = 0;
		if(!j_status) {
			if(SXml::IsName(p_n, "UL")) {
				j_status = 1;
				is_v2_js = 1;
			}
			else if(SXml::IsName(p_n, "FL")) {
				j_status = 2;
				is_v2_js = 1;
			}
			else if(SXml::IsName(p_n, "TS")) {
				j_status = 4;
				is_v2_js = 1;
			}
			else if(SXml::IsName(p_n, "FO")) {
				j_status = 3;
				is_v2_js = 1;
			}
			if(is_v2_js)
				p_n = p_n->children; // ������������� �� ���������� ������� - ��� ��������� � 1 � 2 ������� ���������
		}
		//
		if(SXml::IsName(p_n, "Identity")) {
		}
		else if(SXml::GetContentByName(p_n, "ClientRegId", rar_ident))
			;
		else if(SXml::GetContentByName(p_n, "FullName", full_name))
			full_name.Utf8ToChar();
		else if(SXml::GetContentByName(p_n, "ShortName", short_name))
			short_name.Utf8ToChar();
		else if(SXml::GetContentByName(p_n, "INN", inn))
			;
		else if(SXml::GetContentByName(p_n, "KPP", kpp))
			;
		else if(SXml::GetContentByName(p_n, "UNP", unp))
			;
		else if(SXml::GetContentByName(p_n, "RNN", rnn))
			;
		else if(SXml::IsName(p_n, "address")) {
			for(xmlNode * p_a = p_n->children; p_a; p_a = p_a->next) {
				if(SXml::GetContentByName(p_a, "Country", temp_buf))
					country_code = temp_buf.ToLong();
				else if(SXml::GetContentByName(p_a, "Index", zip))
					;
				else if(SXml::GetContentByName(p_a, "RegionCode", temp_buf))
					region_code = temp_buf.ToLong();
				else if(SXml::IsName(p_a, "area"))
					;
				else if(SXml::GetContentByName(p_a, "city", city))
					city.Utf8ToChar();
				else if(SXml::IsName(p_a, "place"))
					;
				else if(SXml::IsName(p_a, "street"))
					;
				else if(SXml::IsName(p_a, "house"))
					;
				else if(SXml::IsName(p_a, "building"))
					;
				else if(SXml::IsName(p_a, "liter"))
					;
				else if(SXml::GetContentByName(p_a, "description", addr_descr))
					addr_descr.Utf8ToChar();
			}
		}
	}
	if(pOutFile) {
        SString out_line_buf;
        out_line_buf.Cat(rar_ident).Tab().Cat(inn).Tab().Cat(kpp).Tab().Cat(unp).Tab().Cat(rnn).Cat(full_name).Tab().Cat(short_name).Tab().Cat(country_code).Tab().Cat(city).CR();
        pOutFile->WriteLine(out_line_buf);
	}
	if(rar_ident.NotEmptyS()) {
		if(pRefC) {
			EgaisPersonCore::Item pi;
			STRNSCPY(pi.RarIdent, rar_ident);
			STRNSCPY(pi.INN, inn);
			STRNSCPY(pi.KPP, kpp);
			STRNSCPY(pi.UNP, unp);
			STRNSCPY(pi.RNN, rnn);
			pi.CountryCode = (int16)country_code;
			pi.RegionCode = (int16)region_code;
			(pi.Name = short_name).Transf(CTRANSF_OUTER_TO_INNER);
			(pi.FullName = full_name).Transf(CTRANSF_OUTER_TO_INNER);
			(pi.AddressDescr = addr_descr).Transf(CTRANSF_OUTER_TO_INNER);
			pi.Flags = roleFlag;
			THROW(pRefC->SetPerson(pi));
		}
		if(pPack) {
			int    pack_extracted = 0; // ���� !0 �� ����� pPacket ��������������� (� ��� ��� ����
				// ������� �������������) � �������� �� ���� ������.
			int    rarid_assigned = 0; // 1 - �� ��� �������� ������ ����������, 2 - �� ��� �������� ������ �� �������
			PPLocationPacket loc_pack;
			if(SearchPersonByRarCode(rar_ident, &psn_id, &loc_id) > 0) {
				THROW(PsnObj.GetPacket(psn_id, pPack, 0) > 0);
				pack_extracted = 1;
				rarid_assigned = 1;
				if(loc_id) {
					for(uint dlp = 0; pPack->EnumDlvrLoc(&dlp, &loc_pack) > 0;) {
						if(loc_pack.ID == loc_id) {
							pPack->SelectedLocPos = dlp;
							break;
						}
					}
				}
			}
			else {
				kpp.Strip();
				PersonTbl::Rec psn_rec;
				PPIDArray by_inn_psn_list;
				if(inn.NotEmptyS()) {
					PPIDArray id_list;
					PsnObj.GetListByRegNumber(PPREGT_TPID, 0, inn, id_list);
					for(uint i = 0; i < id_list.getCount(); i++) {
						const PPID _id = id_list.get(i);
						if(PsnObj.Fetch(_id, &psn_rec) > 0)
							by_inn_psn_list.add(_id);
					}
				}
				if(kpp.NotEmptyS()) {
					PPIDArray by_kpp_psn_list;
					LAssocArray by_kpp_loc_list;
					RegisterArray kpp_reg_list;
					for(uint i = 0; i < by_inn_psn_list.getCount(); i++) {
						const PPID _id = by_inn_psn_list.get(i);
						PPPersonPacket _pack;
						if(PsnObj.GetPacket(_id, &_pack, 0) > 0) {
							if(_pack.Regs.GetListByType(PPREGT_KPP, ZERODATE, &kpp_reg_list) > 0) {
								for(uint j = 0; j < kpp_reg_list.getCount(); j++) {
									(temp_buf = kpp_reg_list.at(j).Num).Strip();
									if(kpp == temp_buf) {
										by_kpp_psn_list.add(_id);
										break;
									}
								}
							}
							if(!(roleFlag & (EgaisPersonCore::rolefManuf|EgaisPersonCore::rolefImporter))) { // ��� �������������� �� ������������ ��� � �������� ��������
								for(uint dlp = 0; _pack.EnumDlvrLoc(&dlp, &loc_pack) > 0;) {
									if(loc_pack.Regs.GetListByType(PPREGT_KPP, ZERODATE, &kpp_reg_list) > 0) {
										for(uint j = 0; j < kpp_reg_list.getCount(); j++) {
											(temp_buf = kpp_reg_list.at(j).Num).Strip();
											if(kpp == temp_buf) {
												by_kpp_loc_list.Add(_id, loc_pack.ID, 0);
												break;
											}
										}
									}
								}
							}
						}
					}
					if(by_kpp_loc_list.getCount()) {
						if(by_kpp_loc_list.getCount() > 1) {
							// @todo log ��������������� � ���������� ������ �� ���
							// ������ ����� ������������� �� �������, � ������� �� ��������� �����-��
							PPIDArray wo_rarid_loc_list;
							for(uint i = 0; i < by_kpp_loc_list.getCount(); i++) {
								const PPID loc_id = by_kpp_loc_list.at(i).Val;
								if(loc_id && p_ref->Ot.GetTagStr(PPOBJ_LOCATION, loc_id, PPTAG_LOC_FSRARID, temp_buf) <= 0)
									wo_rarid_loc_list.add(loc_id);
							}
							if(wo_rarid_loc_list.getCount()) {
								wo_rarid_loc_list.sortAndUndup();
								uint j = by_kpp_loc_list.getCount();
								do {
									PPID loc_id = by_kpp_loc_list.at(--j).Val;
									if(!wo_rarid_loc_list.bsearch(loc_id))
										by_kpp_loc_list.atFree(j);
								} while(j);
							}
						}
						const LAssoc & r_pl = by_kpp_loc_list.at(0);
						THROW(PsnObj.GetPacket(r_pl.Key, pPack, 0) > 0);
						pack_extracted = 1;
						for(uint dlp = 0; pPack->EnumDlvrLoc(&dlp, &loc_pack) > 0;) {
							if(loc_pack.ID == r_pl.Val) {
								pPack->SelectedLocPos = dlp;
								break;
							}
						}
					}
					else if(by_kpp_psn_list.getCount()) {
						if(by_kpp_psn_list.getCount() > 1) {
							PPIDArray wo_rarid_psn_list;
							// @todo log ��������������� � ���������� ���������� �� ���
							// ������ ����� ������������� �� ����������, � ������� �� ��������� �����-��
							for(uint i = 0; i < by_kpp_psn_list.getCount(); i++) {
								const PPID psn_id = by_kpp_psn_list.get(i);
								if(p_ref->Ot.GetTagStr(PPOBJ_PERSON, psn_id, PPTAG_PERSON_FSRARID, temp_buf) <= 0)
									wo_rarid_psn_list.add(psn_id);
							}
							if(wo_rarid_psn_list.getCount())
								by_kpp_psn_list = wo_rarid_psn_list;
						}
						THROW(PsnObj.GetPacket(by_kpp_psn_list.get(0), pPack, 0) > 0);
						pack_extracted = 1;
					}
				}
				//
				// �� ������ ������� (manufType < 0):
				// ��� �������������� ������ ��� - ����� ����������
				//
				if(!pack_extracted && by_inn_psn_list.getCount() && !(roleFlag & (EgaisPersonCore::rolefManuf|EgaisPersonCore::rolefImporter))) {
					if(by_inn_psn_list.getCount() > 1) {
						// @todo log ��������������� � ���������� ���������� �� ���
						// ������ ����� ������������� �� ����������, � ������� �� ��������� �����-��
						PPIDArray wo_rarid_psn_list;
						for(uint i = 0; i < by_inn_psn_list.getCount(); i++) {
							const PPID psn_id = by_inn_psn_list.get(i);
							if(p_ref->Ot.GetTagStr(PPOBJ_PERSON, psn_id, PPTAG_PERSON_FSRARID, temp_buf) <= 0)
								wo_rarid_psn_list.add(psn_id);
						}
						if(wo_rarid_psn_list.getCount())
							by_inn_psn_list = wo_rarid_psn_list;
					}
					THROW(PsnObj.GetPacket(by_inn_psn_list.get(0), pPack, 0) > 0);
					pack_extracted = 1;
					if(pPack->Regs.GetListByType(PPREGT_KPP, ZERODATE, 0) > 0) {
						//
						// ��� � ���������� ����, �� ��������� ��� �� ����� �� ���������� ��������� �� �������:
						//     ������� ����� ����� �������� � ��������� �� ��� ����������
						//
						loc_pack.destroy();
						loc_pack.Type = LOCTYP_ADDRESS;
						PsnObj.LocObj.InitCode(&loc_pack);
						if(zip.NotEmptyS())
							LocationCore::SetExField(&loc_pack, LOCEXSTR_ZIP, zip);
						if(addr_descr.NotEmptyS()) {
							LocationCore::SetExField(&loc_pack, LOCEXSTR_FULLADDR, (temp_buf = addr_descr).Transf(CTRANSF_OUTER_TO_INNER));
							loc_pack.Flags |= LOCF_MANUALADDR;
						}
						{
							RegisterTbl::Rec new_kpp_reg;
							MEMSZERO(new_kpp_reg);
							new_kpp_reg.RegTypeID = PPREGT_KPP;
							STRNSCPY(new_kpp_reg.Num, kpp);
							new_kpp_reg.ObjType = PPOBJ_LOCATION;
							loc_pack.Regs.insert(&new_kpp_reg);
						}
						{
							const ObjTagItem * p_tag_item = loc_pack.TagL.GetItem(PPTAG_LOC_FSRARID);
							if(!p_tag_item || (p_tag_item->GetStr(temp_buf) <= 0 || temp_buf.Empty())) {
								loc_pack.TagL.PutItemStr(PPTAG_LOC_FSRARID, rar_ident);
								rarid_assigned = 2;
							}
						}
						pPack->AddDlvrLoc(loc_pack);
					}
					else {
						RegisterTbl::Rec new_kpp_reg;
						MEMSZERO(new_kpp_reg);
						new_kpp_reg.RegTypeID = PPREGT_KPP;
						STRNSCPY(new_kpp_reg.Num, kpp);
						new_kpp_reg.ObjType = PPOBJ_PERSON;
						pPack->Regs.insert(&new_kpp_reg);
					}
				}
			}
			if(pack_extracted) {
				psn_id = pPack->Rec.ID;
				assert(psn_id != 0);
				if(personKindID)
					pPack->Kinds.addUnique(personKindID);
				AssignManufTypeToPersonPacket(*pPack, roleFlag);
				// @v8.9.0 {
				if(pPack->SelectedLocPos > 0) {
					uint   dlp = pPack->SelectedLocPos-1;
					PPLocationPacket loc_pack;
					if(pPack->EnumDlvrLoc(&dlp, &loc_pack) > 0) {
						const ObjTagItem * p_tag_item = loc_pack.TagL.GetItem(PPTAG_LOC_FSRARID);
						if(!p_tag_item || (p_tag_item->GetStr(temp_buf) <= 0 || temp_buf.Empty())) {
							loc_pack.TagL.PutItemStr(PPTAG_LOC_FSRARID, rar_ident);
							rarid_assigned = 2;
						}
					}
				}
				// } @v8.9.0
			}
			else {
				pPack->destroy();
				pPack->Rec.Status = PPPRS_LEGAL;
				pPack->Kinds.addUnique(personKindID);
				AssignManufTypeToPersonPacket(*pPack, roleFlag);
				temp_buf = 0;
				if(short_name.NotEmptyS())
					temp_buf = short_name;
				else if(full_name.NotEmptyS())
					temp_buf = full_name;
				temp_buf.Transf(CTRANSF_OUTER_TO_INNER);
				STRNSCPY(pPack->Rec.Name, temp_buf);
				if(inn.NotEmptyS()) {
					RegisterTbl::Rec new_inn_reg;
					MEMSZERO(new_inn_reg);
					new_inn_reg.RegTypeID = PPREGT_TPID;
					STRNSCPY(new_inn_reg.Num, inn);
					new_inn_reg.ObjType = PPOBJ_PERSON;
					pPack->Regs.insert(&new_inn_reg);
				}
				if(kpp.NotEmptyS()) {
					RegisterTbl::Rec new_kpp_reg;
					MEMSZERO(new_kpp_reg);
					new_kpp_reg.RegTypeID = PPREGT_KPP;
					STRNSCPY(new_kpp_reg.Num, kpp);
					new_kpp_reg.ObjType = PPOBJ_PERSON;
					pPack->Regs.insert(&new_kpp_reg);
				}
				{
					pPack->Loc.destroy();
					pPack->Loc.Type = LOCTYP_ADDRESS;
					PsnObj.LocObj.InitCode(&pPack->Loc);
					if(zip.NotEmptyS())
						LocationCore::SetExField(&pPack->Loc, LOCEXSTR_ZIP, zip);
					if(addr_descr.NotEmptyS()) {
						LocationCore::SetExField(&pPack->Loc, LOCEXSTR_FULLADDR, (temp_buf = addr_descr).Transf(CTRANSF_OUTER_TO_INNER));
						pPack->Loc.Flags |= LOCF_MANUALADDR;
					}
				}
			}
			if(!rarid_assigned) {
				//
				// �� �������� ��� ������������� �����-��
				//
				const ObjTagItem * p_tag_item = pPack->TagL.GetItem(PPTAG_PERSON_FSRARID);
				if(!p_tag_item || (p_tag_item->GetStr(temp_buf) <= 0 || temp_buf.Empty())) {
					pPack->TagL.PutItemStr(PPTAG_PERSON_FSRARID, rar_ident);
					rarid_assigned = 1;
				}
			}
			THROW(PsnObj.PutPacket(&psn_id, pPack, 1));
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Read_TicketResult(xmlNode * pFirstNode, int ticketType, PPEgaisProcessor::Ticket::Result & rResult)
{
	assert(oneof2(ticketType, 1, 2));
	int    ok = -1;
	SString temp_buf;
	rResult.Type = ticketType;
	for(xmlNode * p_r = pFirstNode; p_r; p_r = p_r->next) {
		if(ticketType == 1) {
			if(SXml::GetContentByName(p_r, "Conclusion", temp_buf)) {
				if(temp_buf.CmpNC("Accepted") == 0) {
					rResult.Conclusion = 1;
					ok = 1;
				}
				else if(temp_buf.CmpNC("Rejected") == 0) {
					rResult.Conclusion = 0;
					ok = 1;
				}
				else
					rResult.Conclusion = -1;
			}
			else if(SXml::GetContentByName(p_r, "ConclusionDate", temp_buf))
				strtodatetime(temp_buf,  &rResult.Time, DATF_ISO8601, TIMF_HMS);
			else if(SXml::GetContentByName(p_r, "Comments", temp_buf))
				rResult.Comment = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		else if(ticketType == 2) {
			if(SXml::GetContentByName(p_r, "OperationResult", temp_buf)) {
				if(temp_buf.CmpNC("Accepted") == 0) {
					rResult.Conclusion = 1;
					ok = 1;
				}
				else if(temp_buf.CmpNC("Rejected") == 0) {
					rResult.Conclusion = 0;
					ok = 1;
				}
				else
					rResult.Conclusion = -1;
			}
			else if(SXml::GetContentByName(p_r, "OperationName", temp_buf))
				rResult.OpName = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			else if(SXml::GetContentByName(p_r, "OperationDate", temp_buf))
				strtodatetime(temp_buf,  &rResult.Time, DATF_ISO8601, TIMF_HMS);
			else if(SXml::GetContentByName(p_r, "OperationComment", temp_buf))
				rResult.Comment = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
	}
	return ok;
}

int SLAPI PPEgaisProcessor::Read_Ticket(xmlNode * pFirstNode, Packet * pPack)
{
	int    ok = 1;
	SString temp_buf;
	if(pPack && pPack->P_Data) {
		Ticket * p_ticket = (Ticket *)pPack->P_Data;
		for(xmlNode * p_n = pFirstNode; p_n; p_n = p_n->next) {
			if(SXml::IsName(p_n, "Identity"))
				;
			else if(SXml::GetContentByName(p_n, "TicketDate", temp_buf))
				strtodatetime(temp_buf,  &p_ticket->TicketTime, DATF_ISO8601, TIMF_HMS);
			else if(SXml::GetContentByName(p_n, "DocId", temp_buf))
				p_ticket->DocUUID.FromStr(temp_buf);
			else if(SXml::GetContentByName(p_n, "TransportId", temp_buf))
				p_ticket->TranspUUID.FromStr(temp_buf);
			else if(SXml::GetContentByName(p_n, "RegID", temp_buf))
				p_ticket->RegIdent = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			else if(SXml::GetContentByName(p_n, "DocType", temp_buf)) {
				p_ticket->DocType = PPEgaisProcessor::RecognizeDocTypeTag(temp_buf);
				if(!p_ticket->DocType) {
					if(temp_buf.CmpNC("WayBillTicket") == 0) {
						p_ticket->DocType = PPEDIOP_EGAIS_CONFIRMTICKET;
					}
				}
			}
			else if(SXml::IsName(p_n, "Result"))
				Read_TicketResult(p_n->children, 1, p_ticket->R);
			else if(SXml::IsName(p_n, "OperationResult"))
				Read_TicketResult(p_n->children, 2, p_ticket->OpR);
		}
	}
	//CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Read_IformA(xmlNode * pFirstNode, Packet * pPack, PrcssrAlcReport::RefCollection * pRefC)
{
    int    ok = 1;
    SString temp_buf;
    EgaisRefATbl::Rec * p_data = (EgaisRefATbl::Rec *)pPack->P_Data;
    for(xmlNode * p_n = pFirstNode; p_n; p_n = p_n->next) {
		if(SXml::GetContentByName(p_n, "InformARegId", temp_buf)) {
			temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			STRNSCPY(p_data->RefACode, temp_buf);
		}
		else if(SXml::IsName(p_n, "Product")) {
			PrcssrAlcReport::GoodsItem agi;
			Read_ProductInfo(p_n->children, 0, &agi, pRefC, 0);
			STRNSCPY(p_data->AlcCode, agi.EgaisCode);
			STRNSCPY(p_data->ManufRarIdent, agi.RefcManufCode);
			STRNSCPY(p_data->ImporterRarIdent, agi.RefcImporterCode);
			p_data->Volume = (long)(agi.Volume * 100000); // @v9.2.12 @fix 1000000-->100000
			p_data->CountryCode = agi.CountryCode;
			p_data->Flags |= EgaisRefACore::fVerified;
		}
		else if(SXml::GetContentByName(p_n, "BottlingDate", temp_buf)) {
			strtodate(temp_buf, DATF_ISO8601, &p_data->BottlingDate);
		}
		else if(SXml::GetContentByName(p_n, "EGAISDate", temp_buf)) {
			strtodate(temp_buf, DATF_ISO8601, &p_data->ActualDate);
		}
    }
    if(pRefC) {
		pRefC->SetRefA(*p_data);
    }
    return ok;
}

int SLAPI PPEgaisProcessor::Read_TTNIformBReg(xmlNode * pFirstNode, Packet * pPack)
{
    int    ok = 1;
    SString temp_buf;
    SString bill_ident;
    InformB * p_data = (InformB *)pPack->P_Data;
    for(xmlNode * p_n = pFirstNode; p_n; p_n = p_n->next) {
		if(SXml::IsName(p_n, "Header")) {
            for(xmlNode * p_h = p_n->children; p_h; p_h = p_h->next) {
				/*
            	if(SXml::GetContentByName(p_h, "Identity", temp_buf))
					p_data->Id = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
				*/
            	if(SXml::GetContentByName(p_h, "Identity", bill_ident)) {
					bill_ident.Transf(CTRANSF_UTF8_TO_INNER);
            	}
            	else if(SXml::GetContentByName(p_h, "WBRegId", temp_buf))
            		p_data->WBRegId = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
            	else if(SXml::GetContentByName(p_h, "EGAISFixNumber", temp_buf))
            		p_data->FixNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
            	else if(SXml::GetContentByName(p_h, "EGAISFixDate", temp_buf))
            		strtodate(temp_buf, DATF_ISO8601, &p_data->FixDate);
            	else if(SXml::GetContentByName(p_h, "WBNUMBER", temp_buf))
					p_data->OuterCode = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
            	else if(SXml::GetContentByName(p_h, "WBDate", temp_buf))
					strtodate(temp_buf, DATF_ISO8601, &p_data->OuterDate);
				/*
            	else if(SXml::IsName(p_h, "Shipper"))
					;
            	else if(SXml::IsName(p_h, "Consignee"))
					;
            	else if(SXml::IsName(p_h, "Supplier"))
					;
				*/
            }
        }
		else {
			if(SXml::IsName(p_n, "Content")) {
				for(xmlNode * p_c = p_n->children; p_c; p_c = p_c->next) {
                    InformBItem item;
					if(SXml::IsName(p_c, "Position")) {
						for(xmlNode * p_pos = p_c->children; p_pos; p_pos = p_pos->next) {
							if(SXml::GetContentByName(p_pos, "Identity", temp_buf)) {
								if(temp_buf == "0")
									item.P = 10000;
								else
									item.P = temp_buf.ToLong();
							}
							else if(SXml::GetContentByName(p_pos, "InformBRegId", temp_buf))
                                STRNSCPY(item.Ident, temp_buf.Transf(CTRANSF_UTF8_TO_INNER));
						}
						p_data->Items.insert(&item);
					}
				}
			}
        }
    }
    if(!bill_ident.NotEmptyS())
		bill_ident = p_data->OuterCode;
	p_data->Id = bill_ident;
    return ok;
}

struct WayBillActRecadvItem {
	WayBillActRecadvItem()
	{
		THISZERO();
	}
    long   P;
    char   InformBIdent[24];
    double RealQtty;
};

int SLAPI PPEgaisProcessor::Read_WayBillAct(xmlNode * pFirstNode, PPID locID, Packet * pPack)
{
    int    ok = 1;
    int    was_header_accepted = 0;
    int    is_pack_inited = 0;
    int    wb_type = 0;
    int    is_accepted = -1;
    LDATE  act_date = ZERODATE;
    SString act_code;
    PPBillPacket * p_bp = (PPBillPacket *)pPack->P_Data;
    SString bill_ident;
    SString memo_note; // <note>
    SString temp_buf;
    PPObjOprKind op_obj;
    BillTbl::Rec bhdr;
    MEMSZERO(bhdr);
    TSArray <WayBillActRecadvItem> items;
    for(xmlNode * p_n = pFirstNode; ok > 0 && p_n; p_n = p_n->next) {
		if(SXml::IsName(p_n, "Header")) {
			was_header_accepted = 1;
            for(xmlNode * p_h = p_n->children; ok > 0 && p_h; p_h = p_h->next) {
				if(SXml::GetContentByName(p_h, "IsAccept", temp_buf)) {
					if(temp_buf.CmpNC("Accepted") == 0)
						is_accepted = 1;
					else if(temp_buf.CmpNC("Rejected") == 0)
						is_accepted = 0;
				}
                else if(SXml::GetContentByName(p_h, "ACTNUMBER", temp_buf)) {
					(act_code = temp_buf).Transf(CTRANSF_UTF8_TO_INNER);
                }
                else if(SXml::GetContentByName(p_h, "ActDate", temp_buf)) {
                    strtodate(temp_buf, DATF_ISO8601, &act_date);
                }
                else if(SXml::GetContentByName(p_h, "WBRegId", temp_buf)) {
					(bill_ident = temp_buf).Transf(CTRANSF_UTF8_TO_INNER);
                }
                else if(SXml::GetContentByName(p_h, "Note", temp_buf)) {
					(memo_note = temp_buf).Utf8ToChar();
                }
            }
        }
		else if(SXml::IsName(p_n, "Content")) {
			for(xmlNode * p_c = p_n->children; ok > 0 && p_c; p_c = p_c->next) {
				WayBillActRecadvItem item;
				if(SXml::IsName(p_c, "Position")) {
					for(xmlNode * p_pos = p_c->children; ok > 0 && p_pos; p_pos = p_pos->next) {
						if(SXml::GetContentByName(p_pos, "Identity", temp_buf)) {
							item.P = temp_buf.ToLong();
						}
						else if(SXml::GetContentByName(p_pos, "InformBRegId", temp_buf)) {
							temp_buf.Transf(CTRANSF_UTF8_TO_INNER).CopyTo(item.InformBIdent, sizeof(item.InformBIdent));
						}
						else if(SXml::GetContentByName(p_pos, "RealQuantity", temp_buf)) {
							item.RealQtty = temp_buf.ToReal();
						}
					}
					THROW_SL(items.insert(&item));
				}
			}
        }
    }
    {
		SString bill_text;
		if(ok < 0) {
			PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
			LogTextWithAddendum(PPTXT_EGAIS_BILLREJECTEDUNR, bill_text);
		}
		else {
			int    do_skip = 0;
			//int    is_valuation_modif = 0;
			if(bill_ident.NotEmptyS()) {
				PPIDArray ex_bill_id_list;
				PPRef->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_EDIIDENT, bill_ident, &ex_bill_id_list);
				for(uint i = 0; !do_skip && i < ex_bill_id_list.getCount(); i++) {
					const PPID ex_bill_id = ex_bill_id_list.get(i);
					BillTbl::Rec ex_bill_rec;
					if(P_BObj->Fetch(ex_bill_id, &ex_bill_rec) > 0) {
                        PPBillPacket bp;
						PPBillPacket link_pack;
                        int    do_turn_link_pack = 0;
                        int    recadv_status = PPEDI_RECADV_STATUS_UNDEF;
						THROW(P_BObj->ExtractPacket(ex_bill_id, &bp) > 0);
                        if(is_accepted != 0) { // @v8.9.5 (is_accepted > 0)-->(is_accepted != 0)
							if(items.getCount() == 0 && is_accepted > 0) { // @v8.9.5 (&& is_accepted > 0)
								recadv_status = PPEDI_RECADV_STATUS_ACCEPT;
							}
							else {
                                PPID   op_id = 0;
                                THROW(op_obj.GetEdiRecadvOp(&op_id, 1));
                                {
									int    edirecadv_found = 0;
                                	BillTbl::Rec link_rec;
									for(DateIter di; !do_skip && P_BObj->P_Tbl->EnumLinks(ex_bill_id, &di, BLNK_EDIRECADV, &link_rec) > 0;) {
										do_skip = 1;
										edirecadv_found = 1;
									}
									recadv_status = PPEDI_RECADV_STATUS_ACCEPT;
									if(edirecadv_found) {
										for(uint k = 0; k < bp.GetTCount(); k++) {
											const PPTransferItem & r_ti = bp.ConstTI(k);
											uint item_idx = 0;
											if(items.lsearch(&r_ti.RByBill, &item_idx, CMPF_LONG)) {
												const double real_qtty = fabs(items.at(item_idx).RealQtty);
												if(!feqeps(real_qtty, fabs(r_ti.Quantity_), 1e-06))
													recadv_status = PPEDI_RECADV_STATUS_PARTACCEPT;
											}
										}
									}
									else if(!do_skip) {
										THROW(link_pack.CreateBlank_WithoutCode(op_id, ex_bill_id, bp.Rec.LocID, 1));
										STRNSCPY(link_pack.Rec.Code, act_code);
										link_pack.Rec.Dt = checkdate(act_date, 0) ? act_date : getcurdate_();
										link_pack.Rec.EdiOp = PPEDIOP_RECADV;
										for(uint k = 0; k < bp.GetTCount(); k++) {
											const PPTransferItem & r_ti = bp.ConstTI(k);
											PPTransferItem ti;
											uint item_idx = 0;
											THROW(ti.Init(&link_pack.Rec));
											ti.GoodsID = r_ti.GoodsID;
											if(items.lsearch(&r_ti.RByBill, &item_idx, CMPF_LONG)) {
												const double real_qtty = fabs(items.at(item_idx).RealQtty);
												ti.Quantity_ = real_qtty;
												if(!feqeps(real_qtty, fabs(r_ti.Quantity_), 1e-06))
													recadv_status = PPEDI_RECADV_STATUS_PARTACCEPT;
											}
											else
												ti.Quantity_ = r_ti.Quantity_;
											// @v8.9.10 {
											{
												GoodsItem _agi;
												PreprocessGoodsItem(ti.GoodsID, 0, 0, 0, _agi);
												if(_agi.UnpackedVolume > 0.0) {
													const double mult = _agi.UnpackedVolume / 10.0;
													ti.Quantity_ = (ti.Quantity_ / mult); // ������������� ��������� ���������� � ����������
												}
											}
											// } @v8.9.10
											ti.Cost = r_ti.Cost;
											ti.Price = r_ti.NetPrice();
											THROW(link_pack.LoadTItem(&ti, 0, 0));
										}
										do_turn_link_pack = 1;
									}
                                }
							}
                        }
						else if(is_accepted == 0)
							recadv_status = PPEDI_RECADV_STATUS_REJECT;
						{
							const int srsr = BillCore::SetRecadvStatus(recadv_status, bp.Rec);
							if(do_turn_link_pack || srsr > 0) {
								PPTransaction tra(1);
								THROW(tra);
								if(do_turn_link_pack) {
									link_pack.InitAmounts();
									THROW(P_BObj->TurnPacket(&link_pack, 0));
								}
								if(srsr > 0) {
									THROW(SearchByID_ForUpdate(P_BObj->P_Tbl, PPOBJ_BILL, bp.Rec.ID, &ex_bill_rec) > 0);
									ex_bill_rec.Flags2 = bp.Rec.Flags2;
									THROW_DB(P_BObj->P_Tbl->updateRecBuf(&ex_bill_rec)); // @sfu
								}
								THROW(tra.Commit());
								{
									PPObjBill::MakeCodeString(&ex_bill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
									LogTextWithAddendum(PPTXT_EDI_BILLRECADVACCEPTED, bill_text);
								}
							}
						}
						break;
					}
				}
			}
			pPack->Flags |= Packet::fDoDelete; // @v9.0.0
		}
    }
	CATCHZOK
    return ok;
}

struct EgaisWayBillRowTags {
	int     RByBill;
	char    GoodsCode[24];
	char    InformA[24];
	char    InformB[24];
	char    Serial[24];
	PPID    ManufID;
};

int SLAPI PPEgaisProcessor::Read_WayBill(xmlNode * pFirstNode, PPID locID, const DateRange * pPeriod, Packet * pPack, PrcssrAlcReport::RefCollection * pRefC)
{
	//
	// Descr: ������� ������ �� ������ ���������
	//
	enum RejectReason {
		rrNone = 0,
		rrUnresolvedGoods,    // � ��������� ���� ������������� ������
		rrDateOutOfRange,     // ���� ��������� ������� �� ������� ��������� �������
		rrIntrBillNotFound,   // �� ������ ��������������� �������� ����������� �����������
		rrIntrBillIdUnrecogn, // �������� �������� ����� ���������� � ��������� ����������� �����������, �� �� ������� ���������� �������������
		rrRByBillDup          // ������������ ��������������� ����� � ���������
	};
    int    ok = 1;
    RejectReason rr = rrNone;
    int    was_header_accepted = 0;
    int    is_pack_inited = 0;
    int    wb_type = 0;
    int    unpacked = 0; // ������� ����, ��� �������� �������� ������������� ��������� (���������� � ����������)
    // PPID   op_id = 0;
    PPID   intr_expend_bill_id = 0; // ����� ��������� ���������� ��������, �������� �������� �� �������� �� ����� (���������� � ���� Note)
	// (�������� �� pPack->IntrBillID) PPID   local_intr_bill_id = 0; // ��������� ��� ����� �� ������������� ��������� ���������� ��������, ��������������� intr_expend_bill_id
    const  PPID loc_id = NZOR(locID, LConfig.Location);
    PPPersonPacket psn_shipper;
    PPPersonPacket psn_suppl;
    PPPersonPacket psn_consignee;
	PPLocationPacket loc_pack;
	PPBillPacket * p_bp = pPack ? (PPBillPacket *)pPack->P_Data : 0;
    PPFreight freight;
    SString bill_ident;
    SString memo_base; // <base>
    SString memo_note; // <note>
    SString temp_buf;
    BillTbl::Rec bhdr;
    MEMSZERO(bhdr);
	TSArray <EgaisWayBillRowTags> row_tags;
	const PPID manuf_tag_id = Cfg.LotManufTagList.getCount() ? Cfg.LotManufTagList.get(0) : 0;
	if(pPack)
		pPack->Flags |= Packet::fFaultObj; // @v9.2.8 ������������� ����. ����� ��������, ��� �������� OK, ���� ������.
    for(xmlNode * p_n = pFirstNode; ok > 0 && p_n; p_n = p_n->next) {
        if(SXml::GetContentByName(p_n, "Identity", temp_buf)) {
			bill_ident = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		else if(SXml::IsName(p_n, "Header")) {
			was_header_accepted = 1;
            for(xmlNode * p_h = p_n->children; ok > 0 && p_h; p_h = p_h->next) {
                if(SXml::GetContentByName(p_h, "NUMBER", temp_buf)) {
					temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
                    STRNSCPY(bhdr.Code, temp_buf);
                }
                else if(SXml::GetContentByName(p_h, "Date", temp_buf)) {
                    strtodate(temp_buf, DATF_ISO8601, &bhdr.Dt);
                }
                else if(SXml::GetContentByName(p_h, "ShippingDate", temp_buf)) {
					LDATE shp_dt;
					strtodate(temp_buf, DATF_ISO8601, &shp_dt);
					if(checkdate(shp_dt, 0))
						freight.IssueDate = shp_dt;
                }
                else if(SXml::GetContentByName(p_h, "Type", temp_buf)) {
					wb_type = RecognizeWayBillTypeText(temp_buf.Utf8ToChar());
                }
                else if(SXml::GetContentByName(p_h, "UnitType", temp_buf)) {
					if(temp_buf.CmpNC("Unpacked") == 0)
						unpacked = 1;
                }
				else if(SXml::IsName(p_h, "Shipper")) {
					// pRefC-->0: ��������� �� ��������� ������ � ���������� �� ������ �� ���������� �� (��� ����� ���� �� ����������)
					Read_OrgInfo(p_h->children, PPPRK_SUPPL, EgaisPersonCore::rolefShipper, p_bp ? &psn_shipper : 0, /*pRefC*/0, 0);
				}
                else if(SXml::IsName(p_h, "Consignee")) {
                	PPID   psn_kind = 0;
					if(oneof2(wb_type, wbtInvcFromMe, wbtRetFromMe)) // �� ���������� �� ���
						psn_kind = PPPRK_MAIN;
					else
						psn_kind = PPPRK_SUPPL; // ����������, �� ����� �� ������ ��������� ��� ������������� ������ ��������� ToMe.
					// @todo � ���� ����� ���������� ��������� ���������������� �����, �� ������� ������ ������� ��������
					// pRefC-->0: ��������� �� ��������� ������ � ���������� �� ������ �� ���������� �� (��� ����� ���� �� ����������)
					Read_OrgInfo(p_h->children, psn_kind, EgaisPersonCore::rolefConsignee, p_bp ? &psn_consignee : 0, /*pRefC*/0, 0);
                }
				else if(SXml::IsName(p_h, "Supplier")) {
					// pRefC-->0: ��������� �� ��������� ������ � ���������� �� ������ �� ���������� �� (��� ����� ���� �� ����������)
					Read_OrgInfo(p_h->children, PPPRK_SUPPL, EgaisPersonCore::rolefSupplier, p_bp ? &psn_suppl : 0, /*pRefC*/0, 0);
				}
                else if(SXml::IsName(p_h, "Transport"))
					;
                else if(SXml::GetContentByName(p_h, "Base", memo_base))
					memo_base.Transf(CTRANSF_UTF8_TO_INNER);
                else if(SXml::GetContentByName(p_h, "Note", memo_note)) {
                	memo_note.Transf(CTRANSF_UTF8_TO_INNER);
                	if(memo_note.CmpPrefix(P_IntrExpndNotePrefix, 1) == 0) {
						(temp_buf = memo_note).ShiftLeft(strlen(P_IntrExpndNotePrefix)).Strip().ShiftLeftChr('=').Strip();
						intr_expend_bill_id = temp_buf.ToLong();
						SETIFZ(intr_expend_bill_id, -1);
                	}
                }
            }
        }
		else if(was_header_accepted && !is_pack_inited) {
			PPID   ar_id = 0;
			PPID   dlvr_loc_id = 0;
			if(!pPack) {
				; // @skip next block
			}
			else if(intr_expend_bill_id > 0) {
				BillTbl::Rec intr_bill_rec;
				if(P_BObj->Fetch(intr_expend_bill_id, &intr_bill_rec) > 0 && IsIntrExpndOp(intr_bill_rec.OpID)) {
					if(BillCore::GetCode(temp_buf = intr_bill_rec.Code) == bhdr.Code)
						pPack->IntrBillID = intr_expend_bill_id;
				}
				if(!pPack->IntrBillID) {
					ObjSyncCore * p_osc = DS.GetTLA().P_ObjSync;
					PPIDArray private_id_list;
					if(p_osc && p_osc->GetPrivateObjectsByForeignID(PPOBJ_BILL, intr_expend_bill_id, &private_id_list) > 0) {
						for(uint i = 0; !pPack->IntrBillID && i < private_id_list.getCount(); i++) {
							const PPID test_bill_id = private_id_list.get(i);
							if(P_BObj->Fetch(test_bill_id, &intr_bill_rec) > 0) {
								const int intr = IsIntrOp(intr_bill_rec.OpID);
								if(oneof2(intr, INTREXPND, INTRRCPT) && BillCore::GetCode(temp_buf = intr_bill_rec.Code) == bhdr.Code)
									pPack->IntrBillID = test_bill_id;
							}
						}
					}
				}
				if(pPack->IntrBillID > 0) {
					//PPTXT_EGAIS_INTRBILLFOUNT
				}
				else {
					pPack->IntrBillID = -1;
					rr = rrIntrBillNotFound;
					ok = -1;
				}
			}
			else if(intr_expend_bill_id < 0) {
				pPack->IntrBillID = -1;
				rr = rrIntrBillIdUnrecogn;
				ok = -1;
			}
			if(wb_type == wbtInvcFromMe) { // �� ���������� �� ���
				if(p_bp) {
					const PPID op_id = ACfg.Hdr.EgaisRcptOpID;
					PPOprKind op_rec;
					PPBillPacket::SetupObjectBlock sob;
					THROW_PP(op_id, PPERR_EGAIS_RCPTOPUNDEF);
					GetOpData(op_id, &op_rec);
					THROW(p_bp->CreateBlank_WithoutCode(op_id, 0, loc_id, 1));
					p_bp->Rec.Dt = bhdr.Dt;
					STRNSCPY(p_bp->Rec.Code, bhdr.Code);
					if(memo_note.NotEmptyS())
						STRNSCPY(p_bp->Rec.Memo, memo_note);
					else if(memo_base.NotEmptyS())
						STRNSCPY(p_bp->Rec.Memo, memo_base);
					if(psn_suppl.Rec.ID) {
						ArObj.P_Tbl->PersonToArticle(psn_suppl.Rec.ID, op_rec.AccSheetID, &ar_id);
					}
					else if(psn_shipper.Rec.ID) {
						ArObj.P_Tbl->PersonToArticle(psn_shipper.Rec.ID, op_rec.AccSheetID, &ar_id);
					}
					if(ar_id)
						p_bp->SetupObject(ar_id, sob);
					if(op_rec.AccSheet2ID && P_UtmEntry && P_UtmEntry->MainOrgID) {
						PPAccSheet acs_rec;
						if(AcsObj.Fetch(op_rec.AccSheet2ID, &acs_rec) > 0 && acs_rec.Assoc == PPOBJ_PERSON && acs_rec.ObjGroup == PPPRK_MAIN) {
							PPID   ar2_id = 0;
							ArObj.P_Tbl->PersonToArticle(P_UtmEntry->MainOrgID, op_rec.AccSheet2ID, &ar2_id);
							p_bp->SetupObject2(ar2_id);
						}
					}
					is_pack_inited = 1;
				}
			}
			else if(wb_type == wbtRetFromMe) { // ������� �� ���������� �� ���
				if(p_bp && ACfg.Hdr.EgaisRetOpID) {
					const PPID op_id = ACfg.Hdr.EgaisRetOpID;
					PPOprKind op_rec;
					PPBillPacket::SetupObjectBlock sob;
					THROW_PP(op_id, PPERR_EGAIS_RCPTOPUNDEF);
					GetOpData(op_id, &op_rec);
					THROW(p_bp->CreateBlank_WithoutCode(op_id, 0, loc_id, 1));
					p_bp->Rec.Dt = bhdr.Dt;
					STRNSCPY(p_bp->Rec.Code, bhdr.Code);
					if(memo_note.NotEmptyS())
						STRNSCPY(p_bp->Rec.Memo, memo_note);
					else if(memo_base.NotEmptyS())
						STRNSCPY(p_bp->Rec.Memo, memo_base);
					//
					// @v9.2.10 ������� ��������� ������������� �����������: ������� �� ����������������, ����� - �� ����������,
					// ��������� ��������� ���������� ��������� ������� � ��������� ���������� ��� ����������
					//
					if(psn_shipper.Rec.ID) {
						ArObj.P_Tbl->PersonToArticle(psn_shipper.Rec.ID, op_rec.AccSheetID, &ar_id);
						if(ar_id && psn_shipper.SelectedLocPos > 0) {
							uint   dlp = psn_shipper.SelectedLocPos-1;
							if(psn_shipper.EnumDlvrLoc(&dlp, &loc_pack) > 0)
								dlvr_loc_id = loc_pack.ID;
						}
					}
					else if(psn_suppl.Rec.ID) {
						ArObj.P_Tbl->PersonToArticle(psn_suppl.Rec.ID, op_rec.AccSheetID, &ar_id);
						if(ar_id && psn_suppl.SelectedLocPos > 0) {
							uint   dlp = psn_suppl.SelectedLocPos-1;
							if(psn_suppl.EnumDlvrLoc(&dlp, &loc_pack) > 0)
								dlvr_loc_id = loc_pack.ID;
						}
					}
					if(ar_id) {
						p_bp->SetupObject(ar_id, sob);
						freight.DlvrAddrID = dlvr_loc_id;
					}
					if(op_rec.AccSheet2ID && P_UtmEntry && P_UtmEntry->MainOrgID) {
						PPAccSheet acs_rec;
						if(AcsObj.Fetch(op_rec.AccSheet2ID, &acs_rec) > 0 && acs_rec.Assoc == PPOBJ_PERSON && acs_rec.ObjGroup == PPPRK_MAIN) {
							PPID   ar2_id = 0;
							ArObj.P_Tbl->PersonToArticle(P_UtmEntry->MainOrgID, op_rec.AccSheet2ID, &ar2_id);
							p_bp->SetupObject2(ar2_id);
						}
					}
					is_pack_inited = 1;
				}
			}
		}
		if(SXml::IsName(p_n, "Content")) {
			if(p_bp && pPeriod && !pPeriod->CheckDate(p_bp->Rec.Dt)) {
				rr = rrDateOutOfRange;
				ok = -1;
			}
			else {
				SString serial;
				for(xmlNode * p_c = p_n->children; p_c; p_c = p_c->next) {
					PPTransferItem ti;
					serial = 0;
					if(p_bp) {
						THROW(ti.Init(&p_bp->Rec, 0, 0));
					}
					if(SXml::IsName(p_c, "Position")) {
						GoodsItem alc_ext;
						int    product_refc_pos = -1;
						double _src_qtty = 0.0;
						double _src_cost = 0.0;
						for(xmlNode * p_pos = p_c->children; p_pos; p_pos = p_pos->next) {
							if(SXml::GetContentByName(p_pos, "Identity", temp_buf)) {
								if(temp_buf == "0")
									ti.RByBill = 10000; // @v9.2.9
								else
									ti.RByBill = (int16)temp_buf.ToLong(); // @v8.8.6
							}
							else if(SXml::IsName(p_pos, "Product")) {
								int   rs = 0;
								PPGoodsPacket goods_pack;
								// pRefC-->0: ��������� �� ��������� ������ � ������ �� ������ �� ���������� �� (��� ����� ���� �� ����������)
								THROW(rs = Read_ProductInfo(p_pos->children, p_bp ? &goods_pack : 0, &alc_ext, /*pRefC*/0, 0));
								if(pRefC && pRefC->LastProductP > 0) {
                                    product_refc_pos = pRefC->LastProductP;
								}
								if(p_bp) {
									if(rs > 0) {
										THROW(ti.SetupGoods(goods_pack.Rec.ID, 0));
									}
									else {
										rr = rrUnresolvedGoods;
										ok = -1; // ����� ������������� ����� ������������ ��� ��������� � ������ �� �������� ������ ���������
									}
								}
							}
							else if(SXml::GetContentByName(p_pos, "Quantity", temp_buf))
								_src_qtty = fabs(temp_buf.ToReal());
							else if(SXml::GetContentByName(p_pos, "Price", temp_buf))
								_src_cost = fabs(temp_buf.ToReal());
							else if(SXml::IsName(p_pos, "Pack_ID")) {
							}
							else if(SXml::GetContentByName(p_pos, "Party", serial))
								serial.Strip().Transf(CTRANSF_UTF8_TO_INNER);
							else if(SXml::IsName(p_pos, "InformA")) {
								for(xmlNode * p_inf = p_pos->children; p_inf; p_inf = p_inf->next) {
									if(SXml::GetContentByName(p_inf, "RegId", alc_ext.InformA)) {
										alc_ext.InformA.Strip().Transf(CTRANSF_UTF8_TO_INNER);
										break;
									}
								}
							}
							else if(SXml::IsName(p_pos, "InformB")) {
								for(xmlNode * p_inf = p_pos->children; p_inf; p_inf = p_inf->next) {
									if(SXml::IsName(p_inf, "InformBItem")) {
										for(xmlNode * p_inf_item = p_inf->children; p_inf_item; p_inf_item = p_inf_item->next) {
											if(SXml::GetContentByName(p_inf_item, "BRegId", alc_ext.InformB))
												alc_ext.InformB.Strip().Transf(CTRANSF_UTF8_TO_INNER);
										}
									}
								}
							}
						}
						if(pRefC) {
							EgaisRefATbl::Rec refai;
							MEMSZERO(refai);
							STRNSCPY(refai.RefACode, alc_ext.InformA);
							if(product_refc_pos >= 0) {
								const EgaisProductCore::Item * p_product = pRefC->ProductList.at(product_refc_pos);
								if(p_product) {
									STRNSCPY(refai.AlcCode, p_product->AlcoCode);
									STRNSCPY(refai.ManufRarIdent, p_product->ManufRarIdent);
									STRNSCPY(refai.ImporterRarIdent, p_product->ImporterRarIdent);
									refai.Volume = (int32)(p_product->Volume * 100000.0);
									THROW(pRefC->SetRefA(refai));
								}
							}
						}
						if(is_pack_inited && ti.GoodsID && _src_qtty != 0.0) {
							if(unpacked) {
								GoodsItem _agi;
								PreprocessGoodsItem(ti.GoodsID, 0, 0, 0, _agi);
								if(_agi.UnpackedVolume > 0.0) {
									const double mult = _agi.UnpackedVolume / 10.0;
									ti.Quantity_ = R6(_src_qtty / mult);
									ti.Cost = R5(_src_cost * mult);
								}
								else {
									ti.Quantity_ = _src_qtty;
									ti.Cost = _src_cost;
								}
							}
							else {
								ti.Quantity_ = _src_qtty;
								ti.Cost = _src_cost;
							}
							uint   new_pos = p_bp->GetTCount();
							THROW(p_bp->LoadTItem(&ti, 0, 0/*serial*/));
							{
								EgaisWayBillRowTags rt;
								MEMSZERO(rt);
								rt.RByBill = ti.RByBill;
								if(alc_ext.EgaisCode.NotEmptyS())
									STRNSCPY(rt.GoodsCode, alc_ext.EgaisCode);
								if(alc_ext.InformA.NotEmptyS())
									STRNSCPY(rt.InformA, alc_ext.InformA);
								if(alc_ext.InformB.NotEmptyS())
									STRNSCPY(rt.InformB, alc_ext.InformB);
								rt.ManufID = alc_ext.MnfOrImpPsnID;
								STRNSCPY(rt.Serial, serial.Strip());
								row_tags.insert(&rt);
							}
						}
					}
				}
			}
		}
    }
    if(is_pack_inited) {
		SString bill_text;
		PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
		p_bp->SortTI(); // ����������� ���������� ������������� ������ �� RByBill (����� ������ � ������������ �������)
		{
			//
			// �������� �� ������� ���������� � ������� ����� ���������
			//
			LongArray rbb_list;
			for(uint i = 0; ok > 0 && i < p_bp->GetTCount(); i++) {
				const PPTransferItem & r_ti = p_bp->ConstTI(i);
				if(r_ti.RByBill && rbb_list.lsearch(r_ti.RByBill)) {
					rr = rrRByBillDup;
					ok = -1;
				}
			}
		}
		if(ok < 0) {
			int    msg_id = 0;
			if(rr == rrUnresolvedGoods)
				msg_id = PPTXT_EGAIS_BILLREJECTEDUNR;
			else if(rr == rrDateOutOfRange) {
				// (��������� ������) msg_id = PPTXT_EGAIS_BILLREJECTOOD;
			}
			else if(rr == rrIntrBillIdUnrecogn) {
			}
			else if(rr == rrIntrBillNotFound)
                msg_id = PPTXT_EGAIS_INTRBILLNFOUNT;
			else if(rr == rrRByBillDup)
				msg_id = PPTXT_EGAIS_RBYBILLDUP;
			else
				msg_id = PPTXT_EGAIS_BILLREJECT;
			if(msg_id)
				LogTextWithAddendum(msg_id, bill_text);
		}
		else {
			int    do_skip = 0;
			if(!bill_ident.NotEmptyS())
				bill_ident = p_bp->Rec.Code;
			{
				ObjTagItem tag_item;
				for(uint i = 0; i < p_bp->GetTCount(); i++) {
					const PPTransferItem & r_ti = p_bp->ConstTI(i);
					for(uint j = 0; j < row_tags.getCount(); j++) {
						const EgaisWayBillRowTags & r_rt = row_tags.at(j);
						if(r_rt.RByBill == r_ti.RByBill) {
							ObjTagList tag_list;
							if(r_rt.GoodsCode[0]) {
								tag_list.PutItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, r_rt.GoodsCode);
							}
							if(r_rt.InformA[0]) {
								tag_list.PutItemStr(PPTAG_LOT_FSRARINFA, r_rt.InformA);
							}
							if(r_rt.InformB[0]) {
								tag_list.PutItemStr(PPTAG_LOT_FSRARINFB, r_rt.InformB);
							}
							if(r_rt.ManufID && manuf_tag_id) {
								tag_item.SetInt(manuf_tag_id, r_rt.ManufID);
								tag_list.PutItem(manuf_tag_id, &tag_item);
							}
							THROW(p_bp->LTagL.Set(i, &tag_list));
						}
					}
				}
				p_bp->ProcessFlags |= PPBillPacket::pfForceRByBill; // @v8.8.6
				p_bp->Rec.EdiOp = pPack->DocType;
				if(!freight.IsEmpty())
					p_bp->SetFreight(&freight);
				if(bill_ident.NotEmptyS()) {
					p_bp->BTagL.PutItemStr(PPTAG_BILL_OUTERCODE, bill_ident);
				}
				{
					int    is_valuation_modif = 0;
					if(CheckOpFlags(p_bp->Rec.OpID, OPKF_NEEDVALUATION))
						P_BObj->AutoCalcPrices(p_bp, 0, &is_valuation_modif);
					if(!is_valuation_modif)
						p_bp->InitAmounts();
				}
			}
			pPack->Flags &= ~Packet::fFaultObj; // @v9.2.8 ����� ��������� ����� � �������
		}
    }
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::Helper_AreArticlesEq(PPID ar1ID, PPID ar2ID)
{
	int    yes = 0;
	if(ar1ID == ar2ID)
		yes = 1;
	else if(ar1ID && ar2ID) {
		PPID   psn1_id = ObjectToPerson(ar1ID);
		PPID   psn2_id = ObjectToPerson(ar2ID);
		Reference * p_ref = PPRef;
		if(psn1_id && psn2_id) {
			SString code1, code2;
			p_ref->Ot.GetTagStr(PPOBJ_PERSON, psn1_id, PPTAG_PERSON_FSRARID, code1);
			p_ref->Ot.GetTagStr(PPOBJ_PERSON, psn2_id, PPTAG_PERSON_FSRARID, code2);
			if(code1.NotEmptyS() && code2.NotEmptyS() && code1 == code2)
				yes = 1;
			else {
				PsnObj.GetRegNumber(psn1_id, PPREGT_TPID, getcurdate_(), code1);
				PsnObj.GetRegNumber(psn2_id, PPREGT_TPID, getcurdate_(), code2);
				if(code1.NotEmptyS() && code2.NotEmptyS() && code1 == code2)
					yes = 1;
			}
		}
	}
	return yes;
}

int SLAPI PPEgaisProcessor::Helper_AcceptBillPacket(Packet * pPack, TSCollection <PPEgaisProcessor::Packet> * pPackList, uint packIdx)
{
	int    ok = -1;
	PPBillPacket * p_bp = pPack ? (PPBillPacket *)pPack->P_Data : 0;
	if(p_bp && !(pPack->Flags & (Packet::fAcceptedBill|Packet::fFaultObj))) {
		const  int use_dt_in_bill_analog = BIN(ACfg.Hdr.Flags & PPAlbatrosCfgHdr::fUseDateInBillAnalog);
		int    do_skip = 0;
		SString temp_buf;
		SString msg_buf, fmt_buf;
		SString bill_text;
		SString bill_ident;
		p_bp->BTagL.GetItemStr(PPTAG_BILL_OUTERCODE, bill_ident);
		if(!bill_ident.NotEmptyS())
			bill_ident = p_bp->Rec.Code;
		if(bill_ident.NotEmptyS()) {
			uint   last_analog_pos = 0; // idx+1
			if(pPackList) {
				for(uint pi = packIdx+1; pi < pPackList->getCount(); pi++) {
                    const Packet * p_pack = pPackList->at(pi);
                    if(p_pack && p_pack->P_Data && p_pack->DocType == PPEDIOP_EGAIS_WAYBILL && !(p_pack->Flags & Packet::fFaultObj)) {
						PPBillPacket * p_other_bp = (PPBillPacket *)p_pack->P_Data;
						if((!use_dt_in_bill_analog || p_other_bp->Rec.Dt == p_bp->Rec.Dt) && sstreq(p_other_bp->Rec.Code, p_bp->Rec.Code)) {
                            if(p_other_bp->BTagL.GetItemStr(PPTAG_BILL_OUTERCODE, temp_buf) > 0 && temp_buf == bill_ident) {
								if(Helper_AreArticlesEq(p_other_bp->Rec.Object, p_bp->Rec.Object)) {
									last_analog_pos = pi+1;
								}
                            }
                        }
                    }
				}
			}
			if(last_analog_pos) {
				do_skip = 1;
				PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
				PPLoadText(PPTXT_EGAIS_BILLFORWARDFOUND, fmt_buf);
				Log(msg_buf.Printf(fmt_buf, (const char *)bill_text));
			}
			else {
				PPIDArray ex_bill_id_list;
				PPRef->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_OUTERCODE, bill_ident, &ex_bill_id_list);
				for(uint i = 0; !do_skip && i < ex_bill_id_list.getCount(); i++) {
					const PPID ex_bill_id = ex_bill_id_list.get(i);
					BillTbl::Rec ex_bill_rec;
					if(P_BObj->Fetch(ex_bill_id, &ex_bill_rec) > 0 && ex_bill_rec.OpID == p_bp->Rec.OpID) {
						if(!use_dt_in_bill_analog || ex_bill_rec.Dt == p_bp->Rec.Dt) { // @v9.1.9
							BillCore::GetCode(temp_buf = ex_bill_rec.Code);
							if(temp_buf.CmpNC(p_bp->Rec.Code) == 0 && Helper_AreArticlesEq(ex_bill_rec.Object, p_bp->Rec.Object)) {
								do_skip = 1;
							}
							if(do_skip) {
								/* ���������� ��������� - ������
								PPObjBill::MakeCodeString(&ex_bill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
								PPLoadText(PPTXT_EGAIS_BILLTOOACCEPTED, fmt_buf);
								Log(msg_buf.Printf(fmt_buf, (const char *)bill_text));
								*/
								// @v9.0.0 {
								if(diffdate(getcurdate_(), ex_bill_rec.Dt) > 14)
									pPack->Flags |= Packet::fDoDelete;
								// } @v9.0.0
							}
						}
					}
				}
			}
		}
		if(!do_skip) {
			THROW(P_BObj->TurnPacket(p_bp, 1));
			pPack->Flags |= Packet::fAcceptedBill;
			ok = 1;
			//
			PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
			LogTextWithAddendum(PPTXT_EGAIS_BILLACCEPTED, bill_text);
			if(pPack->IntrBillID > 0) {
				LogTextWithAddendum(PPTXT_EGAIS_INTRBILLFOUNT, bill_text);
				PPBillPacket wroff_pack;
				THROW(P_BObj->ExtractPacket(pPack->IntrBillID, &wroff_pack) > 0);
				{
					const int intr = IsIntrOp(wroff_pack.Rec.OpID);
					int   do_update_wroff_pack = 0;
					if(wroff_pack.Rec.LinkBillID != p_bp->Rec.ID) {
						wroff_pack.Rec.LinkBillID = p_bp->Rec.ID;
						do_update_wroff_pack = 1;
					}
					for(uint i = 0; i < wroff_pack.GetTCount(); i++) {
						PPTransferItem & r_wo_ti = wroff_pack.TI(i);
						for(uint j = 0; j < p_bp->GetTCount(); j++) {
							const PPTransferItem & r_ti = p_bp->ConstTI(j);
							if(r_ti.RByBill == r_wo_ti.RByBill) {
								const ObjTagList * p_src_tag_list = p_bp->LTagL.Get(j);
								if(p_src_tag_list) {
									const ObjTagItem * p_infa_tag = p_src_tag_list->GetItem(PPTAG_LOT_FSRARINFA);
									const ObjTagItem * p_infb_tag = p_src_tag_list->GetItem(PPTAG_LOT_FSRARINFB);
									if(p_infa_tag || p_infb_tag) {
										ObjTagList dest_tag_list;
                                        if(intr == INTREXPND) {
											THROW_MEM(SETIFZ(wroff_pack.P_MirrorLTagL, new PPLotTagContainer));
											{
												ObjTagList * p_dest_tag_list = wroff_pack.P_MirrorLTagL->Get(i);
												RVALUEPTR(dest_tag_list, p_dest_tag_list);
												if(dest_tag_list.PutItemNZ(p_infa_tag, 0) > 0)
													do_update_wroff_pack = 1;
												if(dest_tag_list.PutItemNZ(p_infb_tag, 0) > 0)
													do_update_wroff_pack = 1;
												wroff_pack.P_MirrorLTagL->Set(i, &dest_tag_list);
											}
                                        }
                                        else if(intr == INTRRCPT) {
											ObjTagList * p_dest_tag_list = wroff_pack.LTagL.Get(i);
											RVALUEPTR(dest_tag_list, p_dest_tag_list);
											if(dest_tag_list.PutItemNZ(p_infa_tag, 0) > 0)
												do_update_wroff_pack = 1;
											if(dest_tag_list.PutItemNZ(p_infb_tag, 0) > 0)
												do_update_wroff_pack = 1;
											wroff_pack.LTagL.Set(i, &dest_tag_list);
                                        }
									}
								}
							}
						}
					}
					if(do_update_wroff_pack) {
						PPTransaction tra(1);
						THROW(tra);
                        THROW(P_BObj->UpdatePacket(&wroff_pack, 0));
						THROW(P_BObj->P_Tbl->SetRecFlag(p_bp->Rec.ID, BILLF_WRITEDOFF, 1, 0));
						THROW(tra.Commit());
					}
				}
			}
		}
	}
	CATCHZOK
    return ok;
}

struct EgaisRestItem {
	EgaisRestItem()
	{
		THISZERO();
	}
	PPID   GoodsID;
	PPID   MnfOrImpPsnID;
	char   InformAIdent[24];
    char   InformBIdent[24];
    char   EgaisCode[24];
    double Rest;
};

struct EgaisSettledTransferItem {
    char   InformBIdent[24];
	PPID   GoodsID;
	PPID   BillID;
    int    RowId;
    double Qtty;
};

IMPL_CMPCFUNC(EgaisSettledTransferItem, p1, p2)
{
	const EgaisSettledTransferItem * i1 = (const EgaisSettledTransferItem *)p1;
	const EgaisSettledTransferItem * i2 = (const EgaisSettledTransferItem *)p2;
    return strcmp(i1->InformBIdent, i2->InformBIdent);
}

int SLAPI PPEgaisProcessor::Helper_CreateWriteOffShop(const PPBillPacket * pCurrentRestPack, const DateRange * pPeriod)
{
	int    ok = 1;
	const  PPID loc_id = pCurrentRestPack ? pCurrentRestPack->Rec.LocID : 0;
	PPID   op_id = 0;
	SString temp_buf;
	SString fmt_buf;
	SString bill_text;
	PPObjOprKind op_obj;
	ObjTagItem tag_item;
	PPBillPacket * p_wroff_bp = 0;
	//
	// ��������� �������� �������� � �������� ��� (������� 2) �����, �������
	// ��� � ������ ����� �� �������� � �������� ����.
	//
	const LDATE _cur_date = getcurdate_();
    PPIDArray alco_goods_list;
    PPID   wos_op_id = 0;
    if(oneof3(Cfg.E.WrOffShopWay, Cfg.woswBalanceWithLots, Cfg.woswByBills, Cfg.woswByCChecks)) {
		if(!op_obj.GetEdiWrOffShopOp(&wos_op_id, 1))
			LogLastError();
		else if(wos_op_id) {
			PPID   ex_today_bill_id = 0;
			{
				DateRange test_period;
				test_period.SetDate(_cur_date);
				BillTbl::Rec sbill_rec;
				SString edi_ack;
				for(SEnum en = P_BObj->P_Tbl->EnumByOp(wos_op_id, &test_period, 0); !ex_today_bill_id && en.Next(&sbill_rec) > 0;) {
					if(sbill_rec.LocID == loc_id) {
						ex_today_bill_id = sbill_rec.ID;
						PPObjBill::MakeCodeString(&sbill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
						//PPRef->Ot.GetTagStr(PPOBJ_BILL, sbill_rec.ID, PPTAG_BILL_EDIACK, edi_ack = 0);
						//PPRef->Ot.GetTagStr(PPOBJ_BILL, sbill_rec.ID, PPTAG_BILL_EDIIDENT, temp_buf = 0);
					}
				}
			}
			if(ex_today_bill_id) {
				LogTextWithAddendum(PPTXT_EGAIS_TODAYYETWROFFR2, bill_text);
			}
			else if(GetAlcGoodsList(alco_goods_list) > 0) {
				SString egais_code;
				ReceiptTbl::Rec lot_rec;
				PPIDArray lot_id_list;
				if(Cfg.E.WrOffShopWay == Cfg.woswBalanceWithLots) {
					for(uint i = 0; i < pCurrentRestPack->GetTCount(); i++) {
						const PPTransferItem & r_ti = pCurrentRestPack->ConstTI(i);
						if(r_ti.Quantity_ > 0.0 && pCurrentRestPack->LTagL.GetTagStr(i, PPTAG_LOT_FSRARLOTGOODSCODE, egais_code) > 0) {
							assert(egais_code.NotEmpty());
							PPRef->Ot.SearchObjectsByStrExactly(PPOBJ_LOT, PPTAG_LOT_FSRARLOTGOODSCODE, egais_code, &lot_id_list);
							double current_lot_rest = 0.0;
							for(uint j = 0; j < lot_id_list.getCount(); j++) {
								const PPID lot_id = lot_id_list.get(j);
								if(P_BObj->trfr->Rcpt.Search(lot_id, &lot_rec) > 0 && lot_rec.LocID == loc_id) {
									double _rest = 0.0;
									P_BObj->trfr->GetRest(lot_id, _cur_date, MAXLONG, &_rest, 0);
									current_lot_rest += _rest;
								}
							}
							double wroff_qtty = (r_ti.Quantity_ - current_lot_rest);
							if(wroff_qtty >= 1.0) {
								if(!p_wroff_bp) {
									THROW_MEM(p_wroff_bp = new PPBillPacket);
									THROW(p_wroff_bp->CreateBlank2(wos_op_id, _cur_date, loc_id, 1));
								}
								{
									PPTransferItem ti;
									uint   new_pos = p_wroff_bp->GetTCount();
									THROW(ti.Init(&p_wroff_bp->Rec, 1));
									THROW(ti.SetupGoods(r_ti.GoodsID, 0));
									ti.Quantity_ = wroff_qtty;
									THROW(p_wroff_bp->LoadTItem(&ti, 0, 0));
									{
										ObjTagList tag_list;
										tag_list.PutItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
										THROW(p_wroff_bp->LTagL.Set(new_pos, &tag_list));
									}
								}
							}
						}
					}
				}
				else if(Cfg.E.WrOffShopWay == Cfg.woswByCChecks) {
					int    skip = 0;
					CCheckFilt cc_filt;
					if(!Cfg.P_CcFilt) {
						LogTextWithAddendum(PPTXT_EGAIS_WROFFR2CCFUNDEF, temp_buf = 0);
						skip = 1;
					}
					else {
						cc_filt = *Cfg.P_CcFilt;
						if(cc_filt.Period.IsZero()) {
							if(pPeriod && !pPeriod->IsZero())
								cc_filt.Period = *pPeriod;
							else
								cc_filt.Period.SetDate(plusdate(getcurdate_(), -1));
						}
						if(cc_filt.NodeList.GetCount() == 0 && loc_id) {
							PPObjCashNode cn_obj;
							PPCashNode cn_rec;
							skip = 1;
							for(SEnum en = cn_obj.ref->Enum(PPOBJ_CASHNODE, 0); en.Next(&cn_rec) > 0;) {
								if(cn_rec.LocID == loc_id) {
									cc_filt.NodeList.Add(cn_rec.ID);
									skip = 0;
								}
							}
							if(skip) {
								LogTextWithAddendum(PPTXT_EGAIS_WROFFR2CCFNONODES, temp_buf = 0);
							}
						}
					}
					if(!skip) {
						PPViewCCheck cc_view;
						CCheckViewItem cc_item;
						CCheckCore * p_cc = cc_view.GetCc();
						THROW(cc_view.Init_(&cc_filt));
						if(p_cc) {
							CCheckPacket cc_pack;
							EgaisMarkBlock emb;
							BarcodeArray bc_list;
							LongArray crest_row_list;
							LongArray ex_row_list;
							PPIDArray cc_list;
							for(cc_view.InitIteration(0); cc_view.NextIteration(&cc_item) > 0;) {
								cc_list.add(cc_item.ID);
							}
							cc_list.sortAndUndup();
							for(uint ccidx = 0; ccidx < cc_list.getCount(); ccidx++) {
								const PPID cc_id = cc_list.get(ccidx);
								cc_pack.Init();
								p_cc->LoadPacket(cc_id, 0, &cc_pack);
								for(uint clidx = 0; clidx < cc_pack.GetCount(); clidx++) {
									const CCheckLineTbl::Rec & r_ccl = cc_pack.GetLine(clidx);
									const PPID goods_id = labs(r_ccl.GoodsID);
									if(alco_goods_list.bsearch(goods_id)) {
										cc_pack.GetLineTextExt(clidx, CCheckPacket::lnextEgaisMark, temp_buf);
										bc_list.clear();
										if(temp_buf.NotEmpty() && ParseEgaisMark(temp_buf, emb) > 0) {
											egais_code = emb.EgaisCode;
											bc_list.Add(egais_code, 0, 1.0);
										}
										else {
											GetEgaisCodeList(goods_id, bc_list);
										}
										const double ccl_qtty = r_ccl.Quantity;
										double ccl_rest = ccl_qtty;
										for(uint bci = 0; bci < bc_list.getCount(); bci++) {
											egais_code = bc_list.at(bci).Code;
											double ex_row_qtty = 0.0;
											if(p_wroff_bp) {
												p_wroff_bp->LTagL.SearchString(egais_code, PPTAG_LOT_FSRARLOTGOODSCODE, 0, ex_row_list);
												for(uint eridx = 0; eridx < ex_row_list.getCount(); eridx++) {
													const PPTransferItem & r_ex_ti = p_wroff_bp->TI(ex_row_list.get(eridx));
													ex_row_qtty += r_ex_ti.Quantity_;
												}
											}
											else
												ex_row_list.clear();
											//
											double crest = 0.0;
											{
												pCurrentRestPack->LTagL.SearchString(egais_code, PPTAG_LOT_FSRARLOTGOODSCODE, 0, crest_row_list);
												for(uint cri = 0; cri < crest_row_list.getCount(); cri++) {
													const PPTransferItem & r_cr_ti = pCurrentRestPack->ConstTI(crest_row_list.get(cri));
													crest += r_cr_ti.Quantity_;
												}
												crest += ex_row_qtty;
											}
											if(ccl_rest != 0.0 && fsign(ccl_rest) == fsign(ccl_qtty)) {
												double wroff_qtty = 0.0;
												if(ccl_rest < 0.0)
													wroff_qtty = ccl_rest;
												/* ����� �� ��������� �������� � �����
												else if(bci == (bc_list.getCount()-1))
													wroff_qtty = ccl_rest;
												*/
												else if(crest > 0.0)
													wroff_qtty = MIN(ccl_rest, crest);
												if(wroff_qtty != 0.0) {
													if(!p_wroff_bp) {
														THROW_MEM(p_wroff_bp = new PPBillPacket);
														THROW(p_wroff_bp->CreateBlank2(wos_op_id, _cur_date, loc_id, 1));
													}
													if(ex_row_list.getCount()) {
														PPTransferItem & r_ex_ti = p_wroff_bp->TI(ex_row_list.get(0));
														r_ex_ti.Quantity_ -= wroff_qtty;
													}
													else {
														PPTransferItem ti;
														uint   new_pos = p_wroff_bp->GetTCount();
														THROW(ti.Init(&p_wroff_bp->Rec, 1));
														THROW(ti.SetupGoods(goods_id, 0));
														ti.Quantity_ = -wroff_qtty;
														THROW(p_wroff_bp->LoadTItem(&ti, 0, 0));
														{
															ObjTagList tag_list;
															tag_list.PutItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
															THROW(p_wroff_bp->LTagL.Set(new_pos, &tag_list));
														}
													}
													ccl_rest -= wroff_qtty;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				else {

				}
				if(p_wroff_bp && p_wroff_bp->GetTCount()) {
					p_wroff_bp->InitAmounts();
					THROW(P_BObj->TurnPacket(p_wroff_bp, 1));
					PPObjBill::MakeCodeString(&p_wroff_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
					LogTextWithAddendum(PPTXT_EGAIS_REG2WBCREATED, bill_text);
				}
			}
		}
	}
	CATCHZOK
	ZDELETE(p_wroff_bp);
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_CreateTransferToShop(const PPBillPacket * pCurrentRestPack)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	const  PPID loc_id = pCurrentRestPack ? pCurrentRestPack->Rec.LocID : 0;
	PPID   op_id = 0;
	SString temp_buf;
	SString fmt_buf;
	SString bill_text;
	PPObjOprKind op_obj;
	ObjTagItem tag_item;
	PPBillPacket * p_shop_rest_bp = 0;
	//
	// ��������� �������� �������� � �������� ��� (������� 2) �����, �������
	// ��� � ������ ����� �� �������� � �������� ����.
	//
	const LDATE _cur_date = getcurdate_();
    PPIDArray alco_goods_list;
    PPID   sco_op_id = 0;
    PPBillPacket current_wh_rest_bp;
    if(!op_obj.GetEdiShopChargeOnOp(&sco_op_id, 1))
		LogLastError();
	else if(!op_id && !op_obj.GetEdiStockOp(&op_id, 1))
		LogLastError();
	else if(op_id) {
		BillTbl::Rec bill_rec;
		DateRange period;
		period.Set(plusdate(_cur_date, -2), plusdate(_cur_date, +1));
		PPID   last_wh_rest_bill_id = 0;
		for(DateIter di(&period); P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
			if(bill_rec.EdiOp == PPEDIOP_EGAIS_REPLYRESTS)
				last_wh_rest_bill_id = bill_rec.ID;
		}
		if(last_wh_rest_bill_id) {
			if(P_BObj->ExtractPacket(last_wh_rest_bill_id, &current_wh_rest_bp, BPLD_FORCESERIALS) > 0) {
				//PPTXT_EGAIS_LASTSTOCKBILLFOUND "��������� �������� ����������� ��������� �������� � @{brand_egais}: '%s'"
				PPObjBill::MakeCodeString(&current_wh_rest_bp.Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, temp_buf);
				LogTextWithAddendum(PPTXT_EGAIS_LASTSTOCKBILLFOUND, temp_buf);
			}
			else {
				current_wh_rest_bp.Rec.ID = 0;
				LogLastError();
			}
		}
		else {
			(temp_buf = 0).Cat(period, 1);
			LogTextWithAddendum(PPTXT_EGAIS_STOCKBILLNFOUNT, temp_buf);
		}
	}
    if(current_wh_rest_bp.Rec.ID && sco_op_id && GetAlcGoodsList(alco_goods_list) > 0) {
		int    dont_create_transfer = 0; // ���� ���� ��� �� �������� ����� �������� ��������, �� ����� �������� �� �������
		PPIDArray settled_bill_list; // ������ ���������� ��������, �������� � �����
		{
			DateRange settled_period;
			settled_period.Set(encodedate(1, 9, 2016), ZERODATE);
			BillTbl::Rec sbill_rec;
			SString settled_edi_ack;
			for(SEnum en = P_BObj->P_Tbl->EnumByOp(sco_op_id, &settled_period, 0); en.Next(&sbill_rec) > 0;) {
				if(p_ref->Ot.GetTagStr(PPOBJ_BILL, sbill_rec.ID, PPTAG_BILL_EDIACK, settled_edi_ack = 0) > 0) {
					if(p_ref->Ot.GetTagStr(PPOBJ_BILL, sbill_rec.ID, PPTAG_BILL_EDIIDENT, temp_buf) > 0) {
						THROW_SL(settled_bill_list.add(sbill_rec.ID));
					}
					else {
						PPObjBill::MakeCodeString(&sbill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, temp_buf);
						LogTextWithAddendum(PPTXT_EGAIS_UNSETTLEDSHOPTRFR, temp_buf);
						dont_create_transfer = 1;
					}
				}
			}
		}
		if(!dont_create_transfer) {
			settled_bill_list.sortAndUndup();
			SString ref_a, ref_b, egais_code;
			SString goods_name;
			LongArray row_idx_list;
			{
				PPIDArray ref_b_lot_list; // ������ ����� �� �������� � ��� ��, ��� � � ������ ������� �������� �� ������
				LongArray processed_csr_rows; // ������ �������� ����� pCurrentRestPack, ������� ���� ��������� ��������� ����������
				PrcssrAlcReport::GoodsItem agi;
				long   checked_counter = 0;
				StrAssocArray checked_list; // ������ �����-�����, ������� ��� �����������
				RAssocArray checked_qtty_list; // ������ �������� �������� �� ����� �����
				//
				PPTransaction tra(1);
				THROW(tra);
				for(uint cwridx = 0; cwridx < current_wh_rest_bp.GetTCount(); cwridx++) {
					const PPTransferItem & r_cwr_item = current_wh_rest_bp.ConstTI(cwridx);
					current_wh_rest_bp.LTagL.GetTagStr(cwridx, PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
					current_wh_rest_bp.LTagL.GetTagStr(cwridx, PPTAG_LOT_FSRARINFB, ref_b);
					if(egais_code.NotEmpty() && ref_b.NotEmpty() && PreprocessGoodsItem(labs(r_cwr_item.GoodsID), 0, 0, 0, agi) > 0) {
						const double cwr_rest = r_cwr_item.Quantity_;
						double shop_rest = 0.0;

						PPID   lot_id = 0;
						ReceiptTbl::Rec lot_rec;
						ref_b_lot_list.clear();
						p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_LOT, PPTAG_LOT_FSRARINFB, ref_b, &ref_b_lot_list);
						for(uint llidx = 0; llidx < ref_b_lot_list.getCount(); llidx++) {
							const PPID temp_lot_id = ref_b_lot_list.get(llidx);
							if(P_BObj->trfr->Rcpt.Search(temp_lot_id, &lot_rec) > 0 && lot_rec.LocID == loc_id) {
								if(IsAlcGoods(lot_rec.GoodsID)) { // @v9.4.7
									lot_id = lot_rec.ID;
									break;
								}
							}
						}
						//
						{
							LongArray csr_row_idx_list;
							int    cscr = pCurrentRestPack->LTagL.SearchString(egais_code, PPTAG_LOT_FSRARLOTGOODSCODE, 0, csr_row_idx_list);
							if(cscr > 0) {
								for(uint s = 0; s < csr_row_idx_list.getCount(); s++) {
									const long csr_idx = csr_row_idx_list.get(s);
									if(csr_idx >= 0 && csr_idx < (long)pCurrentRestPack->GetTCount()) {
										const PPTransferItem & r_csr_ti = pCurrentRestPack->ConstTI(csr_idx);
										THROW_SL(processed_csr_rows.add(csr_idx));
										shop_rest += r_csr_ti.Quantity_;
									}
								}
							}
							{
								uint   clp = 0;
								if(checked_list.SearchByText(egais_code, PTR_CMPFUNC(PcharNoCase), &clp)) {
									const long clid = checked_list.at(clp).Id;
									double checked_qtty = checked_qtty_list.Get(clid, 0);
									shop_rest += checked_qtty;
								}
							}
						}
						//
						if(lot_id == 0) {
							// PPTXT_EGAIS_NOLOTSFORWHREST    "��� ������� �� ������ ����� '%s' �� ������� �� ������ ������������ � �����"
							(temp_buf = 0).Cat(egais_code).CatDiv('-', 1).Cat(ref_b).Space().CatChar('=').Cat(cwr_rest, MKSFMTD(0, 1, 0));
							LogTextWithAddendum(PPTXT_EGAIS_NOLOTSFORWHREST, temp_buf);
						}
						else if(cwr_rest > 0.0) {
							double transfer_qtty = 0.0;
							if(Cfg.E.Flags & PrcssrAlcReport::Config::fWhToReg2ByLacks) {
								if(shop_rest < 0.0)
									transfer_qtty = MIN(-shop_rest, cwr_rest);
							}
							else
								transfer_qtty = cwr_rest;
							if(transfer_qtty >= 1.0) {
								if(p_shop_rest_bp && p_shop_rest_bp->GetTCount() >= 100) {
									p_shop_rest_bp->InitAmounts();
									THROW(P_BObj->TurnPacket(p_shop_rest_bp, 0));
									PPObjBill::MakeCodeString(&p_shop_rest_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
									LogTextWithAddendum(PPTXT_EGAIS_REG2TBCREATED, bill_text);
									ZDELETE(p_shop_rest_bp);
								}
								if(!p_shop_rest_bp) {
									THROW_MEM(p_shop_rest_bp = new PPBillPacket);
									THROW(p_shop_rest_bp->CreateBlank2(sco_op_id, _cur_date, loc_id, 0));
								}
								{
									PPTransferItem ti;
									uint   new_pos = p_shop_rest_bp->GetTCount();
									THROW(ti.Init(&p_shop_rest_bp->Rec, 1));
									THROW(ti.SetupGoods(lot_rec.GoodsID, 0));
									ti.Quantity_ = transfer_qtty;
									ti.Cost = lot_rec.Cost;
									ti.Price = lot_rec.Price;
									THROW(p_shop_rest_bp->LoadTItem(&ti, 0, 0));
									{
										uint   clp = 0;
                                        if(!checked_list.SearchByText(egais_code, PTR_CMPFUNC(PcharNoCase), &clp)) {
											checked_counter++;
											THROW_SL(checked_list.Add(checked_counter, egais_code, 0));
											clp = 0;
											THROW_SL(checked_list.SearchByText(egais_code, PTR_CMPFUNC(PcharNoCase), &clp));
                                        }
                                        const long clid = checked_list.at(clp).Id;
                                        THROW_SL(checked_qtty_list.Add(clid, transfer_qtty, 1, 0));
									}
									{
										ObjTagList tag_list;
										if(egais_code.NotEmpty()) {
											tag_list.PutItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
										}
										assert(ref_b.NotEmpty()); // @paranoic
										if(ref_b.NotEmpty()) {
											tag_list.PutItemStr(PPTAG_LOT_FSRARINFB, ref_b);
										}
										if(ref_a.NotEmpty()) {
											tag_list.PutItemStr(PPTAG_LOT_FSRARINFA, ref_a);
										}
										THROW(p_shop_rest_bp->LTagL.Set(new_pos, &tag_list));
									}
								}
							}
						}
						else if(shop_rest < 0.0) {
							//
							// PPTXT_EGAIS_NOWHRESTFORR2DFCT  "��� �������������� ������� �� �������� 2 @{brand_egais} '%s' ��� ���������� ������� �� ������ @{brand_egais}"
							(temp_buf = 0).Cat(egais_code).CatDiv('-', 1).Cat(ref_b).Space().CatChar('=').Cat(shop_rest, MKSFMTD(0, 1, 0));
							LogTextWithAddendum(PPTXT_EGAIS_NOWHRESTFORR2DFCT, temp_buf);
						}
					}
				}
				for(uint csridx = 0; csridx < pCurrentRestPack->GetTCount(); csridx++) {
					if(!processed_csr_rows.lsearch((long)csridx)) {
						const PPTransferItem & r_csr_ti = pCurrentRestPack->ConstTI(csridx);
						if(r_csr_ti.Quantity_ < 0.0) {
							pCurrentRestPack->LTagL.GetTagStr(csridx, PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
							// PPTXT_EGAIS_NOWHRESTFORR2DFCT  "��� �������������� ������� �� �������� 2 @{brand_egais} '%s' ��� ���������� ������� �� ������ @{brand_egais}"
							(temp_buf = 0).Cat(egais_code).Space().CatChar('=').Cat(r_csr_ti.Quantity_, MKSFMTD(0, 1, 0));
							LogTextWithAddendum(PPTXT_EGAIS_NOWHRESTFORR2DFCT, temp_buf);
						}
					}
				}
				if(p_shop_rest_bp && p_shop_rest_bp->GetTCount()) {
					p_shop_rest_bp->InitAmounts();
					THROW(P_BObj->TurnPacket(p_shop_rest_bp, 0));
					PPObjBill::MakeCodeString(&p_shop_rest_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
					LogTextWithAddendum(PPTXT_EGAIS_REG2TBCREATED, bill_text);
				}
				THROW(tra.Commit());
			}
		}
	}
	CATCHZOK
	ZDELETE(p_shop_rest_bp);
	return ok;
}

int SLAPI PPEgaisProcessor::Read_Rests(xmlNode * pFirstNode, PPID locID, const DateRange * pPeriod, Packet * pPack, PrcssrAlcReport::RefCollection * pRefC)
{
    int    ok = 1;
    int    was_header_accepted = 0;
    int    is_pack_inited = 0;
    int    wb_type = 0;
    PPID   op_id = 0;
    const  PPID   loc_id = NZOR(locID, LConfig.Location);
	DateRange period;
	BillTbl::Rec bill_rec;
    LDATETIME rest_dtm = ZERODATETIME;
	PPBillPacket * p_bp = pPack ? (PPBillPacket *)pPack->P_Data : 0;
	PPBillPacket * p_shop_rest_bp = 0;
    SString temp_buf;
    BillTbl::Rec bhdr;
    MEMSZERO(bhdr);
    TSArray <EgaisRestItem> items;
	const PPID manuf_tag_id = Cfg.LotManufTagList.getCount() ? Cfg.LotManufTagList.get(0) : 0;
    for(xmlNode * p_n = pFirstNode; ok > 0 && p_n; p_n = p_n->next) {
        if(SXml::GetContentByName(p_n, "RestsDate", temp_buf)) {
			strtodatetime(temp_buf, &rest_dtm, DATF_ISO8601, TIMF_HMS);
		}
		else if(SXml::IsName(p_n, "Products")) {
			for(xmlNode * p_c = p_n->children; ok > 0 && p_c; p_c = p_c->next) {
				if(SXml::IsName(p_c, "StockPosition") || SXml::IsName(p_c, "ShopPosition")) {
					GoodsItem alc_ext;
					EgaisRestItem rest_item;
					int    product_refc_pos = -1;
					for(xmlNode * p_pos = p_c->children; ok > 0 && p_pos; p_pos = p_pos->next) {
						if(SXml::GetContentByName(p_pos, "Quantity", temp_buf)) {
							rest_item.Rest = temp_buf.ToReal();
						}
						else if(SXml::GetContentByName(p_pos, "InformARegId", temp_buf)) {
							temp_buf.Strip().Transf(CTRANSF_UTF8_TO_INNER).CopyTo(rest_item.InformAIdent, sizeof(rest_item.InformAIdent));
						}
						else if(SXml::GetContentByName(p_pos, "InformBRegId", temp_buf)) {
							temp_buf.Strip().Transf(CTRANSF_UTF8_TO_INNER).CopyTo(rest_item.InformBIdent, sizeof(rest_item.InformBIdent));
						}
						else if(SXml::IsName(p_pos, "Product")) {
							int   rs = 0;
							PPGoodsPacket goods_pack;
							THROW(rs = Read_ProductInfo(p_pos->children, p_bp ? &goods_pack : 0, &alc_ext, pRefC, 0));
							if(pRefC && pRefC->LastProductP > 0) {
								product_refc_pos = pRefC->LastProductP;
							}
							if(rs > 0) {
								rest_item.GoodsID = goods_pack.Rec.ID;
								rest_item.MnfOrImpPsnID = alc_ext.MnfOrImpPsnID;
								STRNSCPY(rest_item.EgaisCode, alc_ext.EgaisCode);
							}
						}
					}
					if(pRefC) {
						EgaisRefATbl::Rec refai;
						MEMSZERO(refai);
						STRNSCPY(refai.RefACode, rest_item.InformAIdent);
						if(product_refc_pos >= 0) {
							const EgaisProductCore::Item * p_product = pRefC->ProductList.at(product_refc_pos);
							if(p_product) {
								STRNSCPY(refai.AlcCode, p_product->AlcoCode);
								STRNSCPY(refai.ManufRarIdent, p_product->ManufRarIdent);
								STRNSCPY(refai.ImporterRarIdent, p_product->ImporterRarIdent);
								refai.Volume = (int32)(p_product->Volume * 100000.0);
								THROW(pRefC->SetRefA(refai));
							}
						}
					}
					THROW_SL(items.insert(&rest_item));
				}
			}
		}
    }
    if(p_bp && (items.getCount() || pPack->DocType == PPEDIOP_EGAIS_REPLYRESTSSHOP) && checkdate(rest_dtm.d, 0)) {
		SString bill_text;
		int    do_skip = 0;
		PPID   op_id = 0;
		PPObjOprKind op_obj;
		THROW(op_obj.GetEdiStockOp(&op_id, 1));
        {
			period.SetDate(rest_dtm.d);
        	for(DateIter di(&period); !do_skip && P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
				ObjTagItem tag_item;
				if(PPRef->Ot.GetTag(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_CREATEDTM, &tag_item) > 0) {
					LDATETIME ts;
                    if(tag_item.GetTimestamp(&ts) > 0 && cmp(ts, rest_dtm) >= 0) {
                        do_skip = 1;
					}
				}
			}
        }
        if(!do_skip) {
			ObjTagItem tag_item;
			{
				PPTransaction tra(1);
				THROW(tra);
				THROW(p_bp->CreateBlank2(op_id, rest_dtm.d, loc_id, 0));
				p_bp->Rec.EdiOp = pPack->DocType;
				if(pPack->DocType == PPEDIOP_EGAIS_REPLYRESTSSHOP) {
					temp_buf = p_bp->Rec.Code;
					temp_buf.CatChar('-').Cat("SHOP");
					STRNSCPY(p_bp->Rec.Code, temp_buf);
				}
				{
					tag_item.SetTimestamp(PPTAG_BILL_CREATEDTM, rest_dtm);
					p_bp->BTagL.PutItem(PPTAG_BILL_CREATEDTM, &tag_item);
				}
				for(uint i = 0; i < items.getCount(); i++) {
					const EgaisRestItem & r_item = items.at(i);
					if(r_item.GoodsID) {
						PPTransferItem ti;
						uint   new_pos = p_bp->GetTCount();
						THROW(ti.Init(&p_bp->Rec, 1));
						THROW(ti.SetupGoods(r_item.GoodsID, 0));
						// @v9.4.9 ti.Quantity_ = r_item.Rest;
						// @v9.4.9 {
						{
							GoodsItem _agi;
							PreprocessGoodsItem(ti.GoodsID, 0, 0, 0, _agi);
							if(_agi.UnpackedVolume > 0.0) {
								const double mult = _agi.UnpackedVolume / 10.0;
								ti.Quantity_ = R6(r_item.Rest / mult);
							}
							else {
								ti.Quantity_ = r_item.Rest;
							}
						}
						// } @v9.4.9
						THROW(p_bp->LoadTItem(&ti, 0, 0));
						{
							ObjTagList tag_list;
							if(r_item.EgaisCode[0]) {
								tag_list.PutItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, r_item.EgaisCode);
							}
							if(r_item.InformAIdent[0]) {
								tag_list.PutItemStr(PPTAG_LOT_FSRARINFA, r_item.InformAIdent);
							}
							if(r_item.InformBIdent[0]) {
								tag_list.PutItemStr(PPTAG_LOT_FSRARINFB, r_item.InformBIdent);
							}
							if(r_item.MnfOrImpPsnID && manuf_tag_id) {
								tag_item.SetInt(manuf_tag_id, r_item.MnfOrImpPsnID);
								tag_list.PutItem(manuf_tag_id, &tag_item);
							}
							THROW(p_bp->LTagL.Set(new_pos, &tag_list));
						}
					}
				}
				p_bp->InitAmounts();
				THROW(P_BObj->TurnPacket(p_bp, 0));
				pPack->Flags |= Packet::fAcceptedBill;
				THROW(tra.Commit());
				PPObjBill::MakeCodeString(&p_bp->Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
				LogTextWithAddendum(PPTXT_EGAIS_RESTSACCEPTED, bill_text);
			}
			//
			// @v9.2.11 {
			/* @v9.4.0
			if(pPack->DocType == PPEDIOP_EGAIS_REPLYRESTSSHOP) {
				THROW(Helper_CreateTransferToShop(p_bp));
				//THROW(Helper_CreateWriteOffShop(p_bp, pPeriod));
			}
			*/
			// } @v9.2.11
		}
    }
	CATCHZOK
	ZDELETE(p_shop_rest_bp);
    return ok;
}

int SLAPI PPEgaisProcessor::Write(Packet & rPack, PPID locID, const char * pFileName)
{
	int    ok = -1;
    xmlTextWriter * p_x = xmlNewTextWriterFilename(pFileName, 0);
    THROW(p_x);
    THROW(ok = Helper_Write(rPack, locID, p_x));
    CATCHZOK
    xmlFreeTextWriter(p_x);
    return ok;
}

int SLAPI PPEgaisProcessor::Write(Packet & rPack, PPID locID, SBuffer & rBuffer)
{
	int    ok = -1;
	xmlTextWriter * p_x = 0;
	xmlBuffer * p_x_buf = xmlBufferCreate();
	THROW(p_x_buf);
    THROW(p_x = xmlNewTextWriterMemory(p_x_buf, 0));
    THROW(ok = Helper_Write(rPack, locID, p_x));
	xmlTextWriterFlush(p_x);
	THROW_SL(rBuffer.Write((char *)p_x_buf->content, p_x_buf->use));
    CATCHZOK
    xmlFreeTextWriter(p_x);
    xmlBufferFree(p_x_buf);
    return ok;
}

int SLAPI PPEgaisProcessor::AcceptDoc(PPEgaisProcessor::Reply & rR, const char * pFileName)
{
    int    ok = 1;
    SString file_name;
    if(!isempty(pFileName))
		file_name = pFileName;
	else {
		THROW(GetTempFileName(0, 0, "EQD", file_name));
	}
	{
		const  int max_try = 3;
		int    try_no = 0;
		int    try_ok = 0;
		do {
			if(try_no)
				delay(1000);
			SFile wr_stream(file_name, SFile::mWrite);
			ScURL c;
			try_ok = c.HttpGet(rR.Url, 0, &wr_stream);
			try_no++;
		} while(!try_ok && (try_no < max_try));
		THROW_SL(try_ok);
	}
	rR.AcceptedFileName = file_name;
    CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::DeleteDoc(PPEgaisProcessor::Reply & rR)
{
    int    ok = 1;
    if(!(rR.Status & Reply::stOffline)) {
		SBuffer ack_buf;
		SFile wr_stream(ack_buf, SFile::mWrite);
		ScURL c;
		THROW_SL(c.HttpDelete(rR.Url, 0, &wr_stream));
    }
	rR.Status |= Reply::stDeleted;
	// @v9.2.10 (������ ��������� - ������) LogTextWithAddendum(PPTXT_EGAIS_DOCDELETED, rR.Url);
    CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::Helper_InitNewPack(const int docType, TSCollection <PPEgaisProcessor::Packet> * pPackList,
	PPEgaisProcessor::Packet ** ppPack)
{
	int    ok = 1;
	PPEgaisProcessor::Packet * p_new_pack = 0;
	if(pPackList) {
		THROW_MEM(p_new_pack = new Packet(docType));
	}
	ASSIGN_PTR(ppPack, p_new_pack);
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_FinalizeNewPack(PPEgaisProcessor::Packet ** ppNewPack, uint srcReplyPos, TSCollection <PPEgaisProcessor::Packet> * pPackList)
{
	int    ok = 1;
	if(pPackList && ppNewPack && *ppNewPack) {
		(*ppNewPack)->SrcReplyPos = srcReplyPos;
		THROW_SL(pPackList->insert(*ppNewPack));
		(*ppNewPack) = 0;
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_Read(void * pCtx, const char * pFileName, long flags,
	PPID locID, const DateRange * pPeriod, uint srcReplyPos, TSCollection <PPEgaisProcessor::Packet> * pPackList,
	PrcssrAlcReport::RefCollection * pRefC)
{
	long   file_no = 0;
	{
		SPathStruc ps;
		ps.Split(pFileName);
		file_no = ps.Nam.ToLong();
	}
	int    ok = -1;
	xmlParserCtxt * p_ctx = (xmlParserCtxt *)pCtx;
	xmlDoc * p_doc = 0;
	xmlNode * p_root = 0;
	Packet * p_new_pack = 0;
	SString line_buf;
	SString out_file_name;
	SString temp_buf;
	SString fsrar_id;
	GetFSRARID(locID, fsrar_id, 0);
	THROW_SL(fileExists(pFileName));
	THROW_LXML((p_doc = xmlCtxtReadFile(p_ctx, pFileName, 0, XML_PARSE_NOENT)), p_ctx);
	THROW(p_root = xmlDocGetRootElement(p_doc));
	if(SXml::IsName(p_root, "Documents")) {
		for(xmlNode * p_n = p_root->children; p_n; p_n = p_n->next) {
			if(SXml::IsName(p_n, "Owner")) {

			}
			else if(SXml::IsName(p_n, "Document")) {
				for(xmlNode * p_nd = p_n->children; p_nd; p_nd = p_nd->next) {
					const int doc_type = p_nd ? RecognizeDocTypeTag((const char *)p_nd->name) : 0;
					int    rs = 0;
					if(doc_type == PPEDIOP_EGAIS_WAYBILL) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(rs = Read_WayBill(p_nd->children, locID, pPeriod, p_new_pack, pRefC));
						if(rs > 0) {
							THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							ok = 1;
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_WAYBILLACT) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(rs = Read_WayBillAct(p_nd->children, locID, p_new_pack));
						if(rs > 0) {
							THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							ok = 1;
						}
					}
					else if(oneof2(doc_type, PPEDIOP_EGAIS_REPLYRESTS, PPEDIOP_EGAIS_REPLYRESTSSHOP)) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(rs = Read_Rests(p_nd->children, locID, pPeriod, p_new_pack, pRefC));
						if(rs > 0) {
							THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							ok = 1;
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(rs = Read_ActInventoryInformBReg(p_nd->children, p_new_pack));
						if(rs > 0) {
							THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							ok = 1;
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_TTNINFORMBREG) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(rs = Read_TTNIformBReg(p_nd->children, p_new_pack));
						if(rs > 0) {
							const InformB * p_data = (const InformB *)p_new_pack->P_Data;
							if(!pPeriod || pPeriod->CheckDate(p_data->OuterDate)) {
								THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							}
							else {
								ZDELETE(p_new_pack);
							}
							ok = 1;
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_TICKET) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(rs = Read_Ticket(p_nd->children, p_new_pack));
						if(rs > 0) {
							THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							ok = 1;
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_REPLYFORMA) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						THROW(Read_IformA(p_nd->children, p_new_pack, pRefC));
						THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
					}
					else if(doc_type == PPEDIOP_EGAIS_REPLYCLIENT) {
						const PPID sell_pk_id = GetSellPersonKind();
						if(sell_pk_id) {
							THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
							{
								for(xmlNode * p_n = p_nd->children; p_n; p_n = p_n->next) {
									if(SXml::IsName(p_n, "Clients")) {
										PPGetFilePath(PPPATH_OUT, "egais-clients", out_file_name);
										SFile out_file(out_file_name, SFile::mAppend);
										for(xmlNode * p_c = p_n->children; p_c; p_c = p_c->next) {
											if(SXml::IsName(p_c, "Client")) {
												PPPersonPacket * p_new_psn_pack = 0;
												if(p_new_pack) {
													THROW_MEM(p_new_psn_pack = new PPPersonPacket);
												}
												Read_OrgInfo(p_c->children, sell_pk_id, EgaisPersonCore::rolefConsignee|EgaisPersonCore::rolefVerified,
													p_new_psn_pack, pRefC, out_file.IsValid() ? &out_file : 0);
												if(p_new_psn_pack)
													((TSCollection <PPPersonPacket> *)p_new_pack->P_Data)->insert(p_new_psn_pack);
											}
										}
									}
								}
							}
							THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
							ok = 1;
						}
					}
					else if(doc_type == PPEDIOP_EGAIS_REPLYAP) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
						{
							for(xmlNode * p_n = p_nd->children; p_n; p_n = p_n->next) {
								if(SXml::IsName(p_n, "Products")) {
									PPGetFilePath(PPPATH_OUT, "egais-products", out_file_name);
									SFile out_file(out_file_name, SFile::mAppend);
									for(xmlNode * p_c = p_n->children; p_c; p_c = p_c->next) {
										if(SXml::IsName(p_c, "Product")) {
											PrcssrAlcReport::GoodsItem ext;
											PPGoodsPacket * p_new_goods_pack = 0;
											if(p_new_pack) {
												THROW_MEM(p_new_goods_pack = new PPGoodsPacket);
											}
											Read_ProductInfo(p_c->children, p_new_goods_pack, &ext, pRefC, out_file.IsValid() ? &out_file : 0);
											if(p_new_goods_pack)
												((TSCollection <PPGoodsPacket> *)p_new_pack->P_Data)->insert(p_new_goods_pack);
										}
									}
								}
							}
						}
						THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
						ok = 1;
					}
					else if(doc_type == PPEDIOP_EGAIS_REPLYBARCODE) {
						THROW(Helper_InitNewPack(doc_type, pPackList, &p_new_pack));
                        {
                        	TSCollection <QueryBarcode> * p_qbl = (TSCollection <QueryBarcode> *)p_new_pack->P_Data;
                        	for(xmlNode * p_n = p_nd->children; p_n; p_n = p_n->next) {
                        		if(SXml::IsName(p_n, "Marks")) {
									PPGetFilePath(PPPATH_OUT, "egais-marks", out_file_name);
									const int is_out_empty = fileExists(out_file_name) ? 0 : 1;
									SFile out_file(out_file_name, SFile::mAppend);
									if(is_out_empty) {
										(line_buf = 0).Cat("Type").Tab().Cat("Rank").Tab().Cat("Number").Tab().Cat("Barcode").CR();
										out_file.WriteLine(line_buf);
									}
                        			for(xmlNode * p_c = p_n->children; p_c; p_c = p_c->next) {
										if(SXml::IsName(p_c, "Mark")) {
											QueryBarcode * p_qb = p_qbl->CreateNewItem(0);
											THROW_SL(p_qb);
											for(xmlNode * p_m = p_c->children; p_m; p_m = p_m->next) {
												if(SXml::GetContentByName(p_m, "Type", temp_buf)) {
													p_qb->CodeType = temp_buf.ToLong();
												}
												else if(SXml::GetContentByName(p_m, "Rank", temp_buf)) {
													p_qb->Rank = temp_buf;
												}
												else if(SXml::GetContentByName(p_m, "Number", temp_buf)) {
													p_qb->Number = temp_buf;
												}
												else if(SXml::GetContentByName(p_m, "Barcode", temp_buf)) {
													p_qb->Result = temp_buf;
												}
											}
											(line_buf = 0).Cat(p_qb->CodeType).Tab().Cat(p_qb->Rank).Tab().Cat(p_qb->Number).Tab().Cat(p_qb->Result).CR();
											out_file.WriteLine(line_buf);
											{
                                                PPGetFilePath(PPPATH_OUT, p_qb->Result, temp_buf);
												PPBarcode::CreateImage(p_qb->Result, BARCSTD_PDF417, SFileFormat::Png, temp_buf);
											}
										}
                        			}
                        		}
                        	}
                        }
						THROW(Helper_FinalizeNewPack(&p_new_pack, srcReplyPos, pPackList));
						ok = 1;
					}
				}
			}
		}
	}
	CATCHZOK
	xmlFreeDoc(p_doc);
	delete p_new_pack;
	return ok;
}

int SLAPI PPEgaisProcessor::GetDebugPath(PPID locID, SString & rPath)
{
	rPath = 0;
    int    ok = 1;
    SString temp_buf;
    SString temp_path;
	SString url;
	THROW(GetURL(locID, url));
	THROW(GetFSRARID(locID, temp_buf, 0));
    PPGetPath(PPPATH_TEMP, temp_path);
    temp_path.SetLastSlash().Cat("EGAIS").CatChar('-').Cat(temp_buf);
    THROW_SL(::createDir(temp_path));
	rPath = temp_path;
    CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::MakeOutputFileName(const Reply * pReply, const SString & rTempPath, SString & rFileName)
{
	rFileName = 0;
	int    ok = 1;
	SPathStruc ps;
	SString temp_buf;
	InetUrl _up(pReply->Url);
	_up.GetComponent(InetUrl::cPath, temp_buf);
	(rFileName = rTempPath).SetLastSlash().Cat(temp_buf);
	ps.Split(rFileName);
	(temp_buf = ps.Nam).CatChar('.').Cat("xml");
	ps.Nam = 0;
	ps.Ext = 0;
	ps.Merge(rFileName);
	THROW_SL(::createDir(rFileName));
	rFileName.SetLastSlash().Cat(temp_buf);
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::DebugReadInput(PPID locID)
{
    int    ok = 1;
    xmlParserCtxt * p_ctx = 0;
    SString temp_buf;
    SString temp_path;
	SString line_buf;
	SString url;
	THROW(GetURL(locID, url));
	Log((line_buf = "URL").CatDiv(':', 2).Cat(url));
	THROW(GetDebugPath(locID, temp_path));
    {
		TSCollection <PPEgaisProcessor::Reply> reply_list;
		THROW(p_ctx = xmlNewParserCtxt());
		THROW(GetReplyList(p_ctx, locID, +1, reply_list));
		for(uint i = 0; i < reply_list.getCount(); i++) {
			Reply * p_reply = reply_list.at(i);
			if(p_reply) {
				p_reply->Id.ToStr(S_GUID::fmtIDL, temp_buf);
				(line_buf = "Reply").CatDiv(':', 2).Cat(p_reply->Url).CatChar(' ').Cat(temp_buf);
				Log(line_buf);
				{
					THROW(MakeOutputFileName(p_reply, temp_path, line_buf));
					if(!AcceptDoc(*p_reply, line_buf))
						LogLastError();
					else
						Log((temp_buf = "Accepted").CatDiv(':', 2).Cat(line_buf));
				}
			}
		}
    }
	CATCH
		LogLastError();
		ok = 0;
	ENDCATCH
	xmlFreeParserCtxt(p_ctx);
	CALLPTRMEMB(P_Logger, Save(PPFILNAM_EGAIS_LOG, 0));
    return ok;
}

void SLAPI PPEgaisProcessor::LogTicketResult(const Ticket * pTicket, const BillTbl::Rec * pBillRec)
{
	SString temp_buf = GetDocTypeTag(pTicket->DocType);
	if(pBillRec) {
		SString bill_text;
		PPObjBill::MakeCodeString(pBillRec, PPObjBill::mcsAddOpName, bill_text);
		temp_buf.Space().Cat("bill").Space().CatChar('\'').Cat(bill_text).CatChar('\'');
	}
	if(pTicket->R.Type == 1) {
		temp_buf.Space().Cat("RESULT");
		if(pTicket->R.Conclusion == 1)
			temp_buf.CatDiv(':', 2).Cat("Accepted");
		else if(pTicket->R.Conclusion == 0)
			temp_buf.CatDiv(':', 2).Cat("Rejected");
		if(pTicket->R.Comment.NotEmpty())
			temp_buf.CatDiv('-', 1).Cat(pTicket->R.Comment);
	}
	if(pTicket->OpR.Type == 2) {
		temp_buf.Space().Cat("OPERATION-RESULT");
		if(pTicket->OpR.Conclusion == 1)
			temp_buf.CatDiv(':', 2).Cat("Accepted");
		else if(pTicket->OpR.Conclusion == 0)
			temp_buf.CatDiv(':', 2).Cat("Rejected");
		if(pTicket->OpR.Comment.NotEmpty())
			temp_buf.CatDiv('-', 1).Cat(pTicket->OpR.Comment);
	}
	LogTextWithAddendum(PPTXT_EGAIS_TICKETREGARRVD, temp_buf);
}

int SLAPI PPEgaisProcessor::Helper_FinishBillProcessingByTicket(int ticketType, const BillTbl::Rec & rRec,
	const SString & rBillText, const PPEgaisProcessor::Ticket * pT, int conclusion, int use_ta)
{
	int    ok = -1;
	Reference * p_ref = PPRef;
	int    selfreject_ticket = 0;
	if(ticketType > 1000) {
		ticketType = ticketType - 1000;
		selfreject_ticket = 1000;
	}
	const  PPID bill_id = rRec.ID;
	SString temp_buf;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(pT->RegIdent.NotEmpty()) {
			if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_id, PPTAG_BILL_EDIIDENT, temp_buf) > 0) {
				if(temp_buf.CmpNC(pT->RegIdent) != 0)
					LogTextWithAddendum(PPTXT_EGAIS_BILLEDIIDNTICKET, rBillText);
			}
			else {
				if(conclusion != 0 && !selfreject_ticket) { // @v8.9.5
					if(ticketType == 2) { // @v8.9.9
						ObjTagItem tag_item;
						THROW(tag_item.SetStr(PPTAG_BILL_EDIIDENT, pT->RegIdent));
						THROW(p_ref->Ot.PutTag(PPOBJ_BILL, bill_id, &tag_item, 0));
					}
				}
			}
		}
		if(conclusion == 1) {
			int    msg_id = (ticketType == 2) ? PPTXT_EGAIS_BILLFPROCESSED_S : PPTXT_EGAIS_BILLFPROCESSED_U;
			THROW(P_BObj->P_Tbl->SetRecFlag2(bill_id, BILLF2_ACKPENDING, 0, 0));
			LogTextWithAddendum(msg_id, rBillText);
			ok = 1;
		}
		else if(conclusion == 0) {
			int    msg_id = (ticketType == 2) ? PPTXT_EGAIS_BILLFUNPROCESSED_S : PPTXT_EGAIS_BILLFUNPROCESSED_U;
			THROW(P_BObj->P_Tbl->SetRecFlag2(bill_id, BILLF2_ACKPENDING, 0, 0));
			if(!(State & stDontRemoveTags)) {
				if(!pT->TranspUUID.IsZero()) {
					const PPID uuid_tag_id = selfreject_ticket ? PPTAG_BILL_EDIREJECTACK : PPTAG_BILL_EDIACK;
					ObjTagItem uuid_tag;
					if(p_ref->Ot.GetTag(PPOBJ_BILL, bill_id, uuid_tag_id, &uuid_tag) > 0) {
						S_GUID ex_uuid;
						uuid_tag.GetGuid(&ex_uuid);
						if(ex_uuid == pT->TranspUUID) {
							THROW(p_ref->Ot.RemoveTag(PPOBJ_BILL, bill_id, uuid_tag_id, 0));
						}
					}
				}
				// @v8.9.5 {
				if((rRec.EdiOp != PPEDIOP_EGAIS_WAYBILL && !selfreject_ticket) && (rRec.EdiOp || pT->DocType == PPEDIOP_EGAIS_WAYBILL)) {
					if(pT->RegIdent.NotEmpty()) {
						SString ex_edi_ident;
                        if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_id, PPTAG_BILL_EDIIDENT, ex_edi_ident) > 0) {
							if(ex_edi_ident.CmpNC(pT->RegIdent) == 0)
								THROW(p_ref->Ot.RemoveTag(PPOBJ_BILL, bill_id, PPTAG_BILL_EDIIDENT, 0));
                        }
					}
				}
				// } @v8.9.5
			}
			LogTextWithAddendum(msg_id, rBillText);
			ok = 1;
		}
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_FinishConfirmProcessingByTicket(const BillTbl::Rec & rRec, const SString & rBillText, const S_GUID & rUuid, int conclusion, int use_ta)
{
	int    ok = -1;
	const  PPID bill_id = rRec.ID;
	SString temp_buf;
	{
		Reference * p_ref = PPRef;
		PPTransaction tra(1);
		THROW(tra);
		if(conclusion == 1) {
			ObjTagItem tag_item;
			THROW(tag_item.SetGuid(PPTAG_BILL_EDIRECADVCONFIRM, &rUuid));
			THROW(p_ref->Ot.PutTag(PPOBJ_BILL, rRec.ID, &tag_item, 0));
			LogTextWithAddendum(PPTXT_EGAIS_RECADVCFMACCEPTED, rBillText);
			ok = 1;
		}
		else if(conclusion == 0) {
			THROW(p_ref->Ot.RemoveTag(PPOBJ_BILL, rRec.ID, PPTAG_BILL_EDIRECADVCONFIRM, 0));
			LogTextWithAddendum(PPTXT_EGAIS_RECADVCFMREJECTED, rBillText);
			ok = 1;
		}
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::FinishBillProcessingByTicket(const PPEgaisProcessor::Ticket * pT, int use_ta)
{
	int    ok = -1;
	if(pT) {
		Reference * p_ref = PPRef;
		SString fmt_buf, bill_text, temp_buf;
		PPIDArray bill_id_list;
		ObjTagItem tag_item;
		if(pT->DocType == PPEDIOP_EGAIS_CONFIRMTICKET) {
			temp_buf = pT->RegIdent;
			p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_EDIIDENT, temp_buf, &bill_id_list);
			bill_id_list.sortAndUndup();
			for(uint j = 0; j < bill_id_list.getCount(); j++) {
				const PPID bill_id = bill_id_list.get(j);
				BillTbl::Rec bill_rec;
				if(P_BObj->Search(bill_id, &bill_rec) > 0) {
					PPObjBill::MakeCodeString(&bill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
					if(pT->R.Type == 1) {
						THROW(ok = Helper_FinishConfirmProcessingByTicket(bill_rec, bill_text, pT->TranspUUID, pT->R.Conclusion, use_ta));
						LogTicketResult(pT, &bill_rec);
					}
					if(pT->OpR.Type == 2) {
						THROW(ok = Helper_FinishConfirmProcessingByTicket(bill_rec, bill_text, pT->TranspUUID, pT->OpR.Conclusion, use_ta));
						LogTicketResult(pT, &bill_rec);
					}
					break;
				}
			}
		}
		else if(oneof9(pT->DocType, PPEDIOP_EGAIS_WAYBILLACT, PPEDIOP_EGAIS_WAYBILL, PPEDIOP_EGAIS_ACTCHARGEON, PPEDIOP_EGAIS_ACTCHARGEON_V2,
			PPEDIOP_EGAIS_ACTWRITEOFF, PPEDIOP_EGAIS_TRANSFERTOSHOP, PPEDIOP_EGAIS_TRANSFERFROMSHOP, PPEDIOP_EGAIS_ACTCHARGEONSHOP,
			PPEDIOP_EGAIS_ACTWRITEOFFSHOP)) {
			int    selfreject_ticket = 0;
			SString guid;
			if(!pT->TranspUUID.IsZero())
				pT->TranspUUID.ToStr(S_GUID::fmtIDL, guid);
			if(pT->DocType == PPEDIOP_EGAIS_WAYBILLACT) {
				p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_EDIREJECTACK, guid, &bill_id_list);
				if(bill_id_list.getCount())
					selfreject_ticket = 1000;
			}
			if(!selfreject_ticket) {
				if(pT->RegIdent.NotEmpty()) {
					p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_EDIIDENT, pT->RegIdent, &bill_id_list);
				}
				if(!bill_id_list.getCount() && guid.NotEmpty()) {
					p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_EDIACK, guid, &bill_id_list);
				}
			}
			bill_id_list.sortAndUndup();
			for(uint j = 0; j < bill_id_list.getCount(); j++) {
				const PPID bill_id = bill_id_list.get(j);
				BillTbl::Rec bill_rec;
				if(P_BObj->Search(bill_id, &bill_rec) > 0) {
					PPObjBill::MakeCodeString(&bill_rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
					if(pT->R.Type == 1) {
						THROW(ok = Helper_FinishBillProcessingByTicket(1+selfreject_ticket, bill_rec, bill_text, pT, pT->R.Conclusion, use_ta));
						LogTicketResult(pT, &bill_rec);
					}
					if(pT->OpR.Type == 2) {
						THROW(ok = Helper_FinishBillProcessingByTicket(2+selfreject_ticket, bill_rec, bill_text, pT, pT->OpR.Conclusion, use_ta));
						LogTicketResult(pT, &bill_rec);
					}
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_ReadFilesOffline(const char * pPath, TSCollection <PPEgaisProcessor::Reply> & rList)
{
    int    ok = 1;
	SString temp_buf, code_buf;
	SPathStruc ps;
	(temp_buf = pPath).SetLastSlash().Cat("*.*");
	SDirEntry de;
	for(SDirec direc(temp_buf); direc.Next(&de) > 0;) {
		if(de.IsFolder()) {
			if(!de.IsSelf() && !de.IsUpFolder() && stricmp(de.FileName, P_TempOutputDirName) != 0) {
				(temp_buf = pPath).SetLastSlash().Cat(de.FileName);
				THROW(Helper_ReadFilesOffline(temp_buf, rList)); // @recursion
			}
		}
		else {
			(temp_buf = pPath).SetLastSlash().Cat(de.FileName);
			ps.Split(temp_buf);
			if(ps.Ext.CmpNC("xml") == 0) {
				PPEgaisProcessor::Reply * p_new_reply = new PPEgaisProcessor::Reply;
				THROW_MEM(p_new_reply);
				p_new_reply->Status |= Reply::stOffline;
				p_new_reply->AcceptedFileName = temp_buf;
				SPathStruc::NormalizePath(pPath, SPathStruc::npfSlash, p_new_reply->Url);
				THROW_SL(rList.insert(p_new_reply));
			}
		}
	}
	CATCHZOK
    return ok;
}

int SLAPI PPEgaisProcessor::SearchActChargeByActInform(const PPEgaisProcessor::ActInform & rInf, PPID * pBillID)
{
	int    ok = -1;
	PPID   bill_id = 0;
    if(rInf.ActNumber.NotEmpty()) {
		PPID   op_id = 0;
		SString temp_buf;
		BillTbl::Rec bill_rec;
		THROW(GetActChargeOnOp(&op_id, PPEDIOP_EGAIS_ACTCHARGEON, 1));
		for(DateIter di(encodedate(1, 12, 2015)); ok < 0 && P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
			BillCore::GetCode(temp_buf = bill_rec.Code);
			if(temp_buf.CmpNC(rInf.ActNumber) == 0) {
				bill_id = bill_rec.ID;
				ok = 1;
			}
		}
    }
    CATCHZOK
    ASSIGN_PTR(pBillID, bill_id);
	return ok;
}

int SLAPI PPEgaisProcessor::DeleteSrcPacket(const Packet * pPack, TSCollection <PPEgaisProcessor::Reply> & rReplyList)
{
	int    ok = -1;
	if(pPack && pPack->SrcReplyPos > 0 && pPack->SrcReplyPos <= rReplyList.getCount()) {
		Reply * p_reply = rReplyList.at(pPack->SrcReplyPos-1);
		THROW(DeleteDoc(*p_reply));
		ok = 1;
	}
	CATCH
		LogLastError();
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_CollectRefs(void * pCtx, TSCollection <PPEgaisProcessor::Reply> & rReplyList, RefCollection & rRefC)
{
	int    ok = 1;
	xmlParserCtxt * p_ctx = (xmlParserCtxt *)pCtx;
	SString temp_buf;
	SString temp_path;
	StringSet ss_url_tok;
    for(uint i = 0; i < rReplyList.getCount(); i++) {
		PPEgaisProcessor::Reply * p_reply = rReplyList.at(i);
		int    doc_type = 0;
		temp_buf = p_reply->Url;
		temp_buf.Strip().RmvLastSlash();
		ss_url_tok.clear();
		temp_buf.Tokenize("/", ss_url_tok);
		ss_url_tok.reverse();
		for(uint sp = 0; !doc_type && ss_url_tok.get(&sp, temp_buf);) {
			if(temp_buf.NotEmptyS()) {
				doc_type = PPEgaisProcessor::RecognizeDocTypeTag(temp_buf);
				if(!doc_type) {
					if(temp_buf.CmpNC("FORMBREGINFO") == 0)
						doc_type = PPEDIOP_EGAIS_TTNINFORMBREG;
					else if(temp_buf.CmpNC("ReplyPartner") == 0)
						doc_type = PPEDIOP_EGAIS_REPLYCLIENT;
					else if(temp_buf.CmpNC("ReplyAP") == 0)
						doc_type = PPEDIOP_EGAIS_REPLYAP;
					else if(temp_buf.CmpNC("INVENTORYREGINFO") == 0)
						doc_type = PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG;
					else if(temp_buf.CmpNC("WayBillTicket") == 0)
						doc_type = PPEDIOP_EGAIS_CONFIRMTICKET;
				}
			}
		}
		if(oneof5(doc_type, PPEDIOP_EGAIS_WAYBILL, PPEDIOP_EGAIS_REPLYCLIENT, PPEDIOP_EGAIS_REPLYRESTS,
			PPEDIOP_EGAIS_REPLYRESTSSHOP, PPEDIOP_EGAIS_REPLYAP)) {
			int    adr = 1; // AcceptDoc result
			if(!(p_reply->Status & Reply::stOffline)) {
				THROW(MakeOutputFileName(p_reply, temp_path, temp_buf));
				adr = AcceptDoc(*p_reply, temp_buf);
			}
			if(!adr || !Helper_Read(p_ctx, p_reply->AcceptedFileName, 0, 0, 0, (i+1), 0, &rRefC))
				LogLastError();
			ok = 1;
		}
    }
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::CollectRefs()
{
	int    ok = 1;
	xmlParserCtxt * p_ctx = 0;
	{
		SString temp_buf;
		SString temp_path;
		SString url;
		StringSet ss_egais_paths;
		PPGetPath(PPPATH_TEMP, temp_path);
		SPathStruc ps;
		(temp_buf = temp_path).SetLastSlash().Cat("*.*");
		SDirEntry de;
		for(SDirec direc(temp_buf, 1); direc.Next(&de) > 0;) {
			temp_buf = de.FileName;
			if(temp_buf.Len() == 18 && temp_buf.CmpPrefix("EGAIS-", 1) == 0) {
				(temp_buf = temp_path).SetLastSlash().Cat(de.FileName);
				ss_egais_paths.add(temp_buf);
			}
		}
		if(ss_egais_paths.getCount()) {
			THROW_MEM(SETIFZ(P_RefC, new RefCollection));
			THROW(p_ctx = xmlNewParserCtxt());
			for(uint pp = 0; ss_egais_paths.get(&pp, temp_buf);) {
				TSCollection <PPEgaisProcessor::Reply> reply_list;
				if(Helper_ReadFilesOffline(temp_buf, reply_list)) {
					Helper_CollectRefs(p_ctx, reply_list, *P_RefC);
				}
			}
			THROW(P_RefC->Store(1));
		}
	}
	CATCH
		LogLastError();
		ok = 0;
	ENDCATCH
	xmlFreeParserCtxt(p_ctx);
	return ok;
}

int SLAPI PPEgaisProcessor::RemoveOutputMessages(PPID locID, int debugMode)
{
	int    ok = 1;
	xmlParserCtxt * p_ctx = 0;
	TSCollection <PPEgaisProcessor::Reply> reply_list;
	THROW(p_ctx = xmlNewParserCtxt());
	THROW(GetReplyList(p_ctx, locID, -1 /* ! -1 - ��������� ��������� */, reply_list));
	const uint reply_count = reply_list.getCount();
	if(reply_count) {
		SString msg_buf;
		PPLoadText(PPTXT_EGAIS_REMOVEOUTPUT, msg_buf);
		for(uint i = 0; i < reply_count; i++) {
			Reply * p_reply = reply_list.at(i);
			if(p_reply) {
				if(debugMode) {
					Log(p_reply->Url);
				}
				else {
					THROW(DeleteDoc(*p_reply));
				}
			}
			PPWaitPercent(i+1, reply_count, msg_buf);
		}
	}
	CATCH
		ok = 0;
		LogLastError();
	ENDCATCH
	xmlFreeParserCtxt(p_ctx);
	return ok;
}

int SLAPI PPEgaisProcessor::ReadInput(PPID locID, const DateRange * pPeriod, long flags)
{
	int    ok = -1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	SString temp_path;
	xmlParserCtxt * p_ctx = 0;
	TSCollection <PPEgaisProcessor::Reply> reply_list;
	TSCollection <PPEgaisProcessor::Packet> pack_list;
	StringSet ss_url_tok;
	PPWaitMsg(PPSTR_TEXT, PPTXT_EGAIS_READINGDATA, 0);
	THROW(p_ctx = xmlNewParserCtxt());
	THROW(GetDebugPath(locID, temp_path));
	if(flags & rifOffline) {
		THROW(Helper_ReadFilesOffline(temp_path, reply_list));
	}
	else {
		THROW(GetReplyList(p_ctx, locID, +1, reply_list));
	}
	const uint reply_count = reply_list.getCount();
	if(reply_count) {
		LongArray ediop_list;
		ediop_list.addzlist(PPEDIOP_EGAIS_WAYBILL, PPEDIOP_EGAIS_TTNINFORMBREG, PPEDIOP_EGAIS_TICKET,
			PPEDIOP_EGAIS_REPLYCLIENT, PPEDIOP_EGAIS_REPLYRESTS, PPEDIOP_EGAIS_REPLYRESTSSHOP,
			PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG, PPEDIOP_EGAIS_WAYBILLACT, PPEDIOP_EGAIS_REPLYAP,
			PPEDIOP_EGAIS_REPLYFORMA, PPEDIOP_EGAIS_REPLYRESTSSHOP, PPEDIOP_EGAIS_REPLYBARCODE, 0);
		ediop_list.sortAndUndup();
		THROW_MEM(SETIFZ(P_RefC, new RefCollection));
		for(uint i = 0; i < reply_count; i++) {
			PPEgaisProcessor::Reply * p_reply = reply_list.at(i);
			PPWaitPercent(i+1, reply_count, p_reply->Url);
			int    doc_type = 0;
			temp_buf = p_reply->Url;
			temp_buf.Strip().RmvLastSlash();
			ss_url_tok.clear();
			temp_buf.Tokenize("/", ss_url_tok);
			ss_url_tok.reverse();
			for(uint sp = 0; !doc_type && ss_url_tok.get(&sp, temp_buf);) {
				if(temp_buf.NotEmptyS()) {
					doc_type = PPEgaisProcessor::RecognizeDocTypeTag(temp_buf);
					if(!doc_type) {
						if(temp_buf.CmpNC("FORMBREGINFO") == 0)
							doc_type = PPEDIOP_EGAIS_TTNINFORMBREG;
						else if(temp_buf.CmpNC("ReplyPartner") == 0)
							doc_type = PPEDIOP_EGAIS_REPLYCLIENT;
						else if(temp_buf.CmpNC("ReplyAP") == 0)
							doc_type = PPEDIOP_EGAIS_REPLYAP;
						else if(temp_buf.CmpNC("INVENTORYREGINFO") == 0)
							doc_type = PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG;
						else if(temp_buf.CmpNC("WayBillTicket") == 0)
							doc_type = PPEDIOP_EGAIS_CONFIRMTICKET;
					}
				}
			}
			if(ediop_list.bsearch(doc_type)) {
				int    adr = 1; // AcceptDoc result
				if(!(p_reply->Status & Reply::stOffline)) {
					THROW(MakeOutputFileName(p_reply, temp_path, temp_buf));
					adr = AcceptDoc(*p_reply, temp_buf);
				}
				if(adr && Helper_Read(p_ctx, p_reply->AcceptedFileName, 0, locID, pPeriod, (i+1), &pack_list, P_RefC)) {
					if(oneof4(doc_type, PPEDIOP_EGAIS_REPLYCLIENT, PPEDIOP_EGAIS_REPLYAP, PPEDIOP_EGAIS_REPLYRESTS, PPEDIOP_EGAIS_REPLYFORMA)) {
						if(!DeleteDoc(*p_reply))
							LogLastError();
					}
					else {
						uint plc = pack_list.getCount();
						if(plc) do {
							const Packet * p_pack = pack_list.at(--plc);
							if(p_pack && p_pack->SrcReplyPos == (i+1)) {
								if(p_pack->Flags & Packet::fDoDelete) {
									if(!DeleteDoc(*p_reply))
										LogLastError();
									break;
								}
							}
							else
								break;
						} while(plc);
					}
				}
				else
					LogLastError();
				ok = 1;
			}
			else {
				// @v8.9.5 {
				if(!(p_reply->Status & Reply::stOffline)) {
					if(MakeOutputFileName(p_reply, temp_path, temp_buf))
						AcceptDoc(*p_reply, temp_buf);
					else
						LogLastError();
				}
				// } @v8.9.5
				temp_buf = GetDocTypeTag(doc_type);
				if(temp_buf.Empty())
					temp_buf.Cat(doc_type);
				LogTextWithAddendum(PPTXT_EGAIS_GOTUNPROCESSEDDOC, temp_buf);
			}
			PPWaitPercent(i+1, reply_count); // @v9.4.0
		}
		// @v9.0.4 {
		if(!P_RefC->Store(1))
			LogLastError();
		// } @v9.0.4
	}
	{
		PPIDArray bill_id_list;
		SString edi_ident;
		SString bill_text;
		const LDATETIME _curdtm = getcurdatetime_();
		LongArray skip_packidx_list;
		int    last_rest_pos = -1;
		int    last_restshop_pos = -1;
		DateIter last_rest_di;
		DateIter last_restshop_di;
		for(uint packidx = 0; packidx < pack_list.getCount(); packidx++) {
			int    do_skip = 0;
			if(skip_packidx_list.lsearch((long)packidx))
				do_skip = 1;
			else {
				Packet * p_pack = pack_list.at(packidx);
				if(p_pack && p_pack->P_Data) {
					if(p_pack->DocType == PPEDIOP_EGAIS_REPLYRESTS) {
						const PPBillPacket * p_rbp = (const PPBillPacket *)p_pack->P_Data;
						if(p_rbp && p_rbp->Rec.ID) {
							DateIter tdi;
							tdi.dt = p_rbp->Rec.Dt;
							tdi.oprno = p_rbp->Rec.BillNo;
							if(tdi.Cmp(last_rest_di) > 0) {
								last_rest_di = tdi;
								last_rest_pos = (int)packidx;
							}
						}
					}
					else if(p_pack->DocType == PPEDIOP_EGAIS_REPLYRESTSSHOP) {
						const PPBillPacket * p_rbp = (const PPBillPacket *)p_pack->P_Data;
						if(p_rbp && p_rbp->Rec.ID) {
							DateIter tdi;
							tdi.dt = p_rbp->Rec.Dt;
							tdi.oprno = p_rbp->Rec.BillNo;
							if(tdi.Cmp(last_restshop_di) > 0) {
								last_restshop_di = tdi;
								last_restshop_pos = (int)packidx;
							}
						}
					}
					else if(p_pack->DocType == PPEDIOP_EGAIS_WAYBILL) {
						if(!Helper_AcceptBillPacket(p_pack, &pack_list, packidx))
							LogLastError();
					}
					else if(p_pack->DocType == PPEDIOP_EGAIS_TTNINFORMBREG) {
						const InformB * p_inf = (const InformB *)p_pack->P_Data;
						//
						// �������� �������� ���������� ������� ���� � ����� PPEDIOP_EGAIS_TTNINFORMBREG
						// ����������� � ������ ��������� (������ ������ �������). ��� ����� ������ ��������� ������.
						//
						for(uint fwpackidx = packidx+1; fwpackidx < pack_list.getCount(); fwpackidx++) {
							Packet * p_fw_pack = pack_list.at(fwpackidx);
							if(p_fw_pack && p_fw_pack->P_Data && p_fw_pack->DocType == p_pack->DocType) {
								const InformB * p_fw_inf = (const InformB *)p_fw_pack->P_Data;
								if(p_fw_inf->Id.CmpNC(p_inf->Id) == 0 && p_fw_inf->OuterCode.CmpNC(p_inf->OuterCode) == 0 && p_fw_inf->OuterDate == p_inf->OuterDate) {
									if(p_fw_inf->FixDate > p_inf->FixDate) {
                                        do_skip = 1;
									}
									else if(p_fw_inf->FixDate < p_inf->FixDate) {
										skip_packidx_list.add((long)fwpackidx);
									}
									else {
                                        int64 this_fix_number = 0;
                                        int64 fw_fix_number = 0;
										temp_buf = p_inf->FixNumber;
										while(temp_buf.Len() && !isdec(temp_buf.C(0)))
                                            temp_buf.ShiftLeft();
										this_fix_number = temp_buf.ToInt64();
										temp_buf = p_fw_inf->FixNumber;
										while(temp_buf.Len() && !isdec(temp_buf.C(0)))
                                            temp_buf.ShiftLeft();
										fw_fix_number = temp_buf.ToInt64();
										if(fw_fix_number > this_fix_number) {
											do_skip = 1;
										}
										else if(fw_fix_number < this_fix_number) {
											skip_packidx_list.add((long)fwpackidx);
										}
									}
								}
							}
						}
						if(!do_skip) {
							edi_ident = p_inf->WBRegId;
							BillTbl::Rec ex_bill_rec;
							bill_id_list.clear();
							p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_BILL, PPTAG_BILL_OUTERCODE, p_inf->Id, &bill_id_list);
							if(p_inf->Id.IsDigit()) {
								int64 id_64 = p_inf->Id.ToInt64();
								if(id_64 > 0 && id_64 < LONG_MAX) {
									PPID   id_32 = (PPID)id_64;
									if(!bill_id_list.lsearch(id_32) && P_BObj->Fetch(id_32, &ex_bill_rec) > 0) {
										if(IsOpBelongTo(ex_bill_rec.OpID, Cfg.ExpndOpID) || IsOpBelongTo(ex_bill_rec.OpID, Cfg.IntrExpndOpID)) {
											BillCore::GetCode(temp_buf = ex_bill_rec.Code);
											if(temp_buf.NotEmptyS() && temp_buf.CmpNC(p_inf->OuterCode) == 0) {
												long    _dd = diffdate(p_inf->FixDate, ex_bill_rec.Dt);
												if(_dd >= 0 && _dd <= 30) { // @v8.9.5 7-->30
													bill_id_list.add(id_32);
												}
											}
										}
									}
								}
							}
							int    _bill_found = 0;
							int    _err = 0; // ������� ����, ��� ��������� ������. ���� !0, �� ������� �������� �� ��� ������
							for(uint j = 0; !_bill_found && j < bill_id_list.getCount(); j++) {
								const PPID bill_id = bill_id_list.get(j);
								BillTbl::Rec ex_bill_rec;
								PPBillPacket bp, _link_bp;
								PPBillPacket * p_intr_link_bp = 0;
								int   do_update = 0;
								int   do_update_wroff_pack = 0;
								//
								// �������� do_process:
								// 0 - �� ������������,
								// 1 - ������ �� ����������,
								// 2 - ������� �� ����������
								// 3 - ��������� �������� (�� ��� ������ ������ �� ������, �� �������� ��������� ���)
								int   do_process = 0;
								if(P_BObj->Fetch(bill_id, &ex_bill_rec) > 0) {
									if(IsOpBelongTo(ex_bill_rec.OpID, ACfg.Hdr.EgaisRcptOpID))
										do_process = 1;
									else if(IsOpBelongTo(ex_bill_rec.OpID, ACfg.Hdr.EgaisRetOpID))
										do_process = 2;
									if(IsOpBelongTo(ex_bill_rec.OpID, Cfg.ExpndOpID) || IsOpBelongTo(ex_bill_rec.OpID, Cfg.IntrExpndOpID))
										do_process = 3;
								}
								if(do_process == 3) {
									/* @v9.0.4 ������ �� ������ �� ������� � ��� ������������ ���������
									if(P_BObj->ExtractPacket(bill_id, &bp) > 0) {
										_bill_found = 1;
										PPObjBill::MakeCodeString(&bp.Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
										ObjTagItem tag_item;
										const ObjTagItem * p_ex_tag_item = bp.BTagL.GetItem(PPTAG_BILL_EDIIDENT);
										if(!p_ex_tag_item) {
											if(tag_item.SetStr(PPTAG_BILL_EDIIDENT, edi_ident)) {
												bp.BTagL.PutItem(PPTAG_BILL_EDIIDENT, &tag_item);
												LogTextWithAddendum(PPTXT_EGAIS_BILLEDIASSGNINFB, bill_text);
												do_update = 1;
											}
										}
										else {
											p_ex_tag_item->GetStr(temp_buf);
											if(temp_buf.CmpNC(edi_ident) != 0) {
												//LogTextWithAddendum(PPTXT_EGAIS_BILLEDIIDNINFB, bill_text);
												_bill_found = 0;
											}
										}
									}
									*/
								}
								else if(oneof2(do_process, 1, 2)) {
									BillCore::GetCode(temp_buf = ex_bill_rec.Code);
									// @v9.0.1 (temp_buf.NotEmptyS() && temp_buf.CmpNC(p_inf->OuterCode) == 0)
									if(temp_buf.NotEmptyS() && temp_buf.CmpNC(p_inf->OuterCode) == 0 && P_BObj->ExtractPacket(bill_id, &bp) > 0) {
										_bill_found = 1;
										PPObjBill::MakeCodeString(&bp.Rec, PPObjBill::mcsAddOpName|PPObjBill::mcsAddLocName, bill_text);
										ObjTagItem tag_item;
										int    intr = 0;
										const  ObjTagItem * p_ex_tag_item = bp.BTagL.GetItem(PPTAG_BILL_EDIIDENT);
										if(!p_ex_tag_item) {
											if(tag_item.SetStr(PPTAG_BILL_EDIIDENT, edi_ident)) {
												bp.BTagL.PutItem(PPTAG_BILL_EDIIDENT, &tag_item);
												LogTextWithAddendum(PPTXT_EGAIS_BILLEDIASSGNINFB, bill_text);
												do_update = 1;
											}
										}
										else {
											p_ex_tag_item->GetStr(temp_buf);
											if(temp_buf.CmpNC(edi_ident) != 0) {
												//LogTextWithAddendum(PPTXT_EGAIS_BILLEDIIDNINFB, bill_text);
												_bill_found = 0;
											}
										}
										if(_bill_found) {
											for(uint bi = 0; bi < p_inf->Items.getCount(); bi++) {
												const InformBItem & r_bitem = p_inf->Items.at(bi);
												if(r_bitem.P && r_bitem.Ident[0]) {
													for(uint t = 0; t < bp.GetTCount(); t++) {
														PPTransferItem & r_ti = bp.TI(t);
														if(r_ti.RByBill == r_bitem.P) {
															const ObjTagList * p_ex_tag_list = bp.LTagL.Get(t);
															ObjTagList tag_list;
															int    do_update_informb = 1;
															if(p_ex_tag_list) {
																p_ex_tag_item = p_ex_tag_list->GetItem(PPTAG_LOT_FSRARINFB);
																if(p_ex_tag_item && p_ex_tag_item->GetStr(temp_buf) > 0 && temp_buf.CmpNC(r_bitem.Ident) == 0)
																	do_update_informb = 0;
																else
																	tag_list = *p_ex_tag_list;
															}
															if(do_update_informb) {
																tag_list.PutItemStr(PPTAG_LOT_FSRARINFB, r_bitem.Ident);
																THROW(bp.LTagL.Set(t, &tag_list));
																do_update = 1;
															}
															break;
														}
													}
												}
											}
											{
												//
												// ���� �����-�������� ��� ����� ����������� �������� ����������� �����������,
												// �� ��� ����������� ����� ����� ������������ ��������� ��� ���������
												// � ��� ������� � � �.
												//
												PPIDArray wroff_bill_list;
												BillTbl::Rec wroff_bill_rec;
												for(DateIter diter; P_BObj->P_Tbl->EnumLinks(bp.Rec.ID, &diter, BLNK_WROFFDRAFT, &wroff_bill_rec) > 0;) {
													const int local_intr = IsIntrOp(wroff_bill_rec.OpID);
													if(oneof2(local_intr, INTREXPND, INTRRCPT))
														wroff_bill_list.add(wroff_bill_rec.ID);
												}
												if(wroff_bill_list.getCount() > 1)
													LogTextWithAddendum(PPTXT_EGAIS_BILLWROFFCONFL, bill_text);
												else if(wroff_bill_list.getCount() == 1) {
													THROW(P_BObj->ExtractPacket(wroff_bill_list.get(0), &_link_bp) > 0);
													intr = IsIntrOp(_link_bp.Rec.OpID);
													if(oneof2(intr, INTREXPND, INTRRCPT)) {
														p_intr_link_bp = &_link_bp;
														for(uint lbi = 0; lbi < _link_bp.GetTCount(); lbi++) {
															PPTransferItem & r_wo_ti = _link_bp.TI(lbi);
															for(uint t = 0; t < bp.GetTCount(); t++) {
																const PPTransferItem & r_ti = bp.ConstTI(t);
																if(r_ti.RByBill == r_wo_ti.RByBill) {
																	const ObjTagList * p_src_tag_list = bp.LTagL.Get(t);
																	if(p_src_tag_list) {
																		const ObjTagItem * p_infa_tag = p_src_tag_list->GetItem(PPTAG_LOT_FSRARINFA);
																		const ObjTagItem * p_infb_tag = p_src_tag_list->GetItem(PPTAG_LOT_FSRARINFB);
																		if(p_infa_tag || p_infb_tag) {
																			ObjTagList dest_tag_list;
																			if(intr == INTREXPND) {
																				THROW_MEM(SETIFZ(_link_bp.P_MirrorLTagL, new PPLotTagContainer));
																				const ObjTagList * p_dest_tag_list = _link_bp.P_MirrorLTagL->Get(lbi);
																				RVALUEPTR(dest_tag_list, p_dest_tag_list);
																				if(dest_tag_list.PutItemNZ(p_infa_tag, 0) > 0)
																					do_update_wroff_pack = 1;
																				if(dest_tag_list.PutItemNZ(p_infb_tag, 0) > 0)
																					do_update_wroff_pack = 1;
																				_link_bp.P_MirrorLTagL->Set(lbi, &dest_tag_list);
																			}
																			else if(intr == INTRRCPT) {
																				const ObjTagList * p_dest_tag_list = _link_bp.LTagL.Get(lbi);
																				RVALUEPTR(dest_tag_list, p_dest_tag_list);
																				if(dest_tag_list.PutItemNZ(p_infa_tag, 0) > 0)
																					do_update_wroff_pack = 1;
																				if(dest_tag_list.PutItemNZ(p_infb_tag, 0) > 0)
																					do_update_wroff_pack = 1;
																				_link_bp.LTagL.Set(lbi, &dest_tag_list);
																			}
																		}
																	}
																	break;
																}
															}
														}
													}
												}
											}
										}
									}
								}
								if(do_update) {
									if(!P_BObj->UpdatePacket(&bp, 1)) {
										_err = 1;
										LogLastError();
									}
								}
								if(do_update_wroff_pack && p_intr_link_bp) {
									if(!P_BObj->UpdatePacket(p_intr_link_bp, 1)) {
										_err = 1;
										LogLastError();
									}
								}
							}
							if(!(flags & rifOffline) && _bill_found && !_err && diffdate(_curdtm.d, p_inf->OuterDate) >= 14)
								DeleteSrcPacket(p_pack, reply_list);
						}
					}
					else if(p_pack->DocType == PPEDIOP_EGAIS_ACTINVENTORYINFORMBREG) {
						const ActInform * p_inf = (const ActInform *)p_pack->P_Data;
						PPID   bill_id = 0;
						if(flags & rifRepairInventoryMark) {
							SString ref_b;
							PPTransaction tra(1);
							THROW(tra);
							for(uint j = 0; j < p_inf->Items.getCount(); j++) {
								const ActInformItem * p_item = p_inf->Items.at(j);
								if(p_item) {
									ref_b = 0;
									if(p_item->BItems.getCount()) {
										for(uint n = 0; n < p_item->BItems.getCount(); n++) {
											const InformBItem & r_bitem = p_item->BItems.at(n);
											if(r_bitem.P == p_item->P && r_bitem.Ident[0]) {
												(ref_b = r_bitem.Ident).Strip();
												break;
											}
										}
									}
									if(p_item->AIdent[0] && ref_b.NotEmpty()) {
										PPIDArray lot_A_list;
										PPIDArray lot_B_list;
										p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_LOT, PPTAG_LOT_FSRARINFA, p_item->AIdent, &lot_A_list);
										p_ref->Ot.SearchObjectsByStrExactly(PPOBJ_LOT, PPTAG_LOT_FSRARINFB, ref_b, &lot_B_list);
										if(lot_A_list.getCount() == 0 && lot_B_list.getCount() == 1) {
											PPID   lot_id = lot_B_list.get(0);
											ObjTagItem tag_item;
											THROW(tag_item.SetStr(PPTAG_LOT_FSRARINFA, p_item->AIdent));
											THROW(p_ref->Ot.PutTag(PPOBJ_LOT, lot_id, &tag_item, 0));
										}
									}
								}
							}
							THROW(tra.Commit());
						}
						else if(SearchActChargeByActInform(*p_inf, &bill_id) > 0) {
							PPBillPacket bp;
							THROW(P_BObj->ExtractPacket(bill_id, &bp) > 0);
							{
								PPTransaction tra(1);
								THROW(tra);
								for(uint i = 0; i < bp.GetTCount(); i++) {
									const long   row_id = (long)(i+1);
									for(uint j = 0; j < p_inf->Items.getCount(); j++) {
										const ActInformItem * p_item = p_inf->Items.at(j);
										if(p_item && p_item->P == row_id) {
											const PPTransferItem & r_ti = bp.ConstTI(i);
											const PPID lot_id = (PPID)r_ti.QuotPrice;
											if(lot_id) {
												ObjTagItem tag_item;
												if(p_item->AIdent[0]) {
													THROW(tag_item.SetStr(PPTAG_LOT_FSRARINFA, p_item->AIdent));
													THROW(p_ref->Ot.PutTag(PPOBJ_LOT, lot_id, &tag_item, 0));
												}
												if(p_item->BItems.getCount()) {
													for(uint n = 0; n < p_item->BItems.getCount(); n++) {
														const InformBItem & r_bitem = p_item->BItems.at(n);
														if(r_bitem.P == p_item->P && r_bitem.Ident[0]) {
															THROW(tag_item.SetStr(PPTAG_LOT_FSRARINFB, r_bitem.Ident));
															THROW(p_ref->Ot.PutTag(PPOBJ_LOT, lot_id, &tag_item, 0));
															break;
														}
													}
												}
											}
											break;
										}
									}
								}
								THROW(tra.Commit());
							}
						}
					}
					else if(p_pack->DocType == PPEDIOP_EGAIS_TICKET) {
						Ticket * p_tick = (Ticket *)p_pack->P_Data;
						//
						// Note: ��������� ����� ���������� ���������� � ����� @1, ������ ����
						// ��������� �� ����� ������������ �������� ����� ������������ ����
						// ��������� ��������� �� �������� �� �������������� �������� ������ ��
						// ������������ � ��������� ������ �������� (DGQ).
						//
						THROW(FinishBillProcessingByTicket(p_tick, 1));
						{
							DGQTbl::Rec dgq_rec;
							THROW_MEM(SETIFZ(P_Dgq, new DGQCore));
							if(P_Dgq->SearchQuery(p_tick->TranspUUID, &dgq_rec) > 0) {
								if(!dgq_rec.Ready) {
									temp_buf = GetDocTypeTag(p_tick->DocType);
									temp_buf.Space().Cat(dgq_rec.QDt, DATF_DMY).Space().Cat(dgq_rec.QTm, TIMF_HMS);
									if(p_tick->R.Conclusion == 1)
										temp_buf.Space().Cat("Accepted");
									else if(p_tick->R.Conclusion == 0)
										temp_buf.Space().Cat("Rejected");
									if(p_tick->R.Comment.NotEmptyS())
										temp_buf.CatDiv('-', 1).Cat(p_tick->R.Comment);
									LogTextWithAddendum(PPTXT_EGAIS_TICKETREGARRVD, temp_buf);
									// THROW(FinishBillProcessingByTicket(p_tick)); // @1
									THROW(P_Dgq->CommitQuery(p_tick->TranspUUID, 1));
								}
							}
						}
						if(!(flags & rifOffline) && diffdate(_curdtm.d, p_tick->TicketTime.d) > 1)
							DeleteSrcPacket(p_pack, reply_list);
					}
					else if(p_pack->DocType == PPEDIOP_EGAIS_REPLYFORMA) {
						const EgaisRefATbl::Rec * p_ref_a = (const EgaisRefATbl::Rec *)p_pack->P_Data;
						if(p_ref_a && P_RefC) {
							EgaisRefATbl::Rec refai;
							MEMSZERO(refai);
							STRNSCPY(refai.RefACode, p_ref_a->RefACode);
							STRNSCPY(refai.AlcCode,  p_ref_a->AlcCode);
							STRNSCPY(refai.ManufRarIdent, p_ref_a->ManufRarIdent);
							STRNSCPY(refai.ImporterRarIdent, p_ref_a->ImporterRarIdent);
							refai.Volume = p_ref_a->Volume;
							refai.ActualDate = p_ref_a->ActualDate;
							refai.BottlingDate = p_ref_a->BottlingDate;
							THROW(P_RefC->SetRefA(refai));
						}
					}
				}
			}
		}
		{
			if(last_rest_pos >= 0) {
			}
			if(last_restshop_pos >= 0 && last_restshop_pos < (int)pack_list.getCount()) {
				Packet * p_pack = pack_list.at(last_restshop_pos);
				PPBillPacket * p_bp = p_pack ? (PPBillPacket *)p_pack->P_Data : 0;
				THROW(Helper_CreateTransferToShop(p_bp));
				THROW(Helper_CreateWriteOffShop(p_bp, pPeriod));
			}
		}
	}
	CATCH
		LogLastError();
		ok = 0; // ��������� �� ������ ������ ���������� ������, ������� �������� PPEgaisProcessor::ReadInput
	ENDCATCH
	xmlFreeParserCtxt(p_ctx);
	CALLPTRMEMB(P_Logger, Save(PPFILNAM_EGAIS_LOG, 0));
	return ok;
}

int SLAPI PPEgaisProcessor::GetActChargeOnOp(PPID * pID, int egaisOp, int use_ta)
{
	int    ok = 1;

	PPID   reserved_id = 0;
	PPID   op_type_id = 0;
	long   op_flags = 0;
	uint   name_id = 0;
	const  char * p_symb = 0;
	const  char * p_cntr_template = 0;

	PPID   op_id = 0;
	SString temp_buf;
	PPObjOprKind op_obj;
	PPOprKind op_rec;
	if(egaisOp == PPEDIOP_EGAIS_ACTCHARGEON) {
		reserved_id = PPOPK_EDI_ACTCHARGEON;
		name_id = PPTXT_OPK_EGAIS_ACTCHARGEON;
		p_symb = "EGAISACTCHARGEON";
		op_type_id = PPOPT_GOODSEXPEND;
		op_flags = (OPKF_NOUPDLOTREST|OPKF_PASSIVE);
		p_cntr_template = "EGACT%05";
	}
	else if(egaisOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
		reserved_id = PPOPK_EDI_ACTCHARGEONSHOP;
		name_id = PPTXT_OPK_EGAIS_ACTCHARGEONSHOP;
		p_symb = "EGAISACTCHARGEON-R2";
		op_type_id = PPOPT_DRAFTRECEIPT;
		op_flags = OPKF_PASSIVE;
		p_cntr_template = "EGACTS%05";
	}
	assert(reserved_id);
	THROW_PP(reserved_id, PPERR_INVPARAM);
	if(op_obj.Search(reserved_id, &op_rec) > 0) {
		op_id = op_rec.ID;
	}
	else {
		PPOprKindPacket op_pack;
        MEMSZERO(op_rec);
		op_pack.Rec.ID = reserved_id;
        PPLoadText(name_id, temp_buf);
        THROW(temp_buf.NotEmptyS());
        STRNSCPY(op_pack.Rec.Name, temp_buf);
        STRNSCPY(op_pack.Rec.Symb, p_symb);
        op_pack.Rec.OpTypeID = op_type_id;
		op_pack.Rec.Flags |= op_flags;
		op_pack.OpCntrPack.Init(0);
		STRNSCPY(op_pack.OpCntrPack.Head.CodeTemplate, p_cntr_template);
		op_pack.OpCntrPack.Head.Counter = 0;
		THROW(op_obj.PutPacket(&op_id, &op_pack, use_ta));
	}
	CATCHZOK
	ASSIGN_PTR(pID, op_id);
	return ok;
}

int SLAPI PPEgaisProcessor::CreateActChargeOnBill(PPID * pBillID, int ediOp, PPID locID, LDATE restDate, const PPIDArray & rLotList, int use_ta)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	PPID   op_id = 0;
	PPID   bill_id = 0;
	SString ref_a, ref_b, egais_code;
	PrcssrAlcReport::GoodsItem agi;
	PPBillPacket pack;
	PPIDArray ex_lot_list;
	const LDATE rest_date = NZOR(restDate, getcurdate_());
	assert(oneof2(ediOp, PPEDIOP_EGAIS_ACTCHARGEON, PPEDIOP_EGAIS_ACTCHARGEONSHOP));
	THROW_PP(oneof2(ediOp, PPEDIOP_EGAIS_ACTCHARGEON, PPEDIOP_EGAIS_ACTCHARGEONSHOP), PPERR_INVPARAM);
	THROW(GetActChargeOnOp(&op_id, ediOp, 1));
	if(ediOp == PPEDIOP_EGAIS_ACTCHARGEON) {
		THROW_PP(CheckOpFlags(op_id, OPKF_NOUPDLOTREST, 0), PPERR_EGAIS_INVACTOP);
	}
	else if(ediOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
		THROW_PP(GetOpType(op_id) == PPOPT_DRAFTRECEIPT, PPERR_EGAIS_INVACTOP);
	}
	{
		BillTbl::Rec bill_rec;
		for(DateIter di(ZERODATE); P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
			PPTransferItem ti;
			if(ediOp == PPEDIOP_EGAIS_ACTCHARGEON) {
				for(int rbb = 0; P_BObj->trfr->EnumItems(bill_rec.ID, &rbb, &ti) > 0;) {
					ex_lot_list.addnz(ti.LotID);
				}
			}
			else if(ediOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
				PPBillPacket ex_bp;
				if(P_BObj->ExtractPacket(bill_rec.ID, &ex_bp, BPLD_FORCESERIALS) > 0) {
					for(uint i = 0; i < ex_bp.GetTCount(); i++) {
						const PPTransferItem & r_ex_ti = ex_bp.ConstTI(i);
						if(r_ex_ti.Quantity_ > 0.0)
							ex_lot_list.addnz(r_ex_ti.OrdLotID);
					}
				}
			}
		}
		ex_lot_list.sortAndUndup();
	}
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		THROW(pack.CreateBlank2(op_id, rest_date, locID, 0));
        for(uint i = 0; i < rLotList.getCount(); i++) {
			const PPID lot_id = rLotList.get(i);
			if(!ex_lot_list.bsearch(lot_id)) {
				ReceiptTbl::Rec lot_rec;
				if(P_BObj->trfr->Rcpt.Search(lot_id, &lot_rec) > 0) {
					const PPID goods_id = lot_rec.GoodsID;
					if(goods_id > 0 && IsAlcGoods(goods_id) && PreprocessGoodsItem(goods_id, 0, 0, 0, agi) > 0) { // ��������� ��������� ���� ������
						PPTransferItem ti;
						int   skip = 0;
						double rest = 0.0;
						p_ref->Ot.GetTagStr(PPOBJ_LOT, lot_id, PPTAG_LOT_FSRARINFA, ref_a);
						p_ref->Ot.GetTagStr(PPOBJ_LOT, lot_id, PPTAG_LOT_FSRARINFB, ref_b);
						p_ref->Ot.GetTagStr(PPOBJ_LOT, lot_id, PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
						if(ediOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
							if(ref_b.NotEmptyS() || egais_code.Empty())
								skip = 1;
						}
						if(!skip) {
							// @v9.3.3 {
							if(ediOp == PPEDIOP_EGAIS_ACTCHARGEON) {
								P_BObj->trfr->GetRest(lot_id, pack.Rec.Dt, MAXLONG, &rest);
							}
							else if(ediOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
								P_BObj->trfr->GetRest(lot_id, pack.Rec.Dt, encodedate(30, 9, 2016), &rest); // @v9.3.9
								// @v9.3.9 rest = lot_rec.Rest;
								if(agi.UnpackedVolume > 0.0) {
									const double mult = agi.UnpackedVolume / 10.0;
									rest = (rest * mult); // ������������� ��������� ���������� � ����������
								}
							}
							// } @v9.3.3
							if(rest > 0.0) {
								THROW(ti.Init(&pack.Rec));
								THROW(ti.SetupGoods(goods_id));
								THROW(ti.SetupLot(lot_id, &lot_rec, 0));
								ti.QuotPrice = (double)lot_id;
								ti.Cost = 0.0;
								ti.Price = 0.0;
								ti.Discount = 0.0;
								if(ediOp == PPEDIOP_EGAIS_ACTCHARGEON) {
									ti.Quantity_ = -rest;
								}
								else if(ediOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
									ti.Quantity_ = rest;
									ti.OrdLotID = lot_id; // @v9.3.1
								}
								{
									const uint tipos = pack.GetTCount();
									THROW(pack.LoadTItem(&ti, 0, 0));
									if(ediOp == PPEDIOP_EGAIS_ACTCHARGEONSHOP) {
										ObjTagList tag_list;
										ObjTagItem tag_item;
										if(egais_code.NotEmpty()) {
											tag_list.PutItemStr(PPTAG_LOT_FSRARLOTGOODSCODE, egais_code);
										}
										if(ref_a.NotEmpty()) {
											tag_list.PutItemStr(PPTAG_LOT_FSRARINFA, ref_a);
										}
										if(ref_b.NotEmpty()) {
											tag_list.PutItemStr(PPTAG_LOT_FSRARINFB, ref_b);
										}
										THROW(pack.LTagL.Set(tipos, &tag_list));
									}
								}
							}
						}
					}
				}
			}
        }
        if(pack.GetTCount()) {
			THROW(P_BObj->__TurnPacket(&pack, 0, 1, 0));
			bill_id = pack.Rec.ID;
        }
		else
			ok = -1;
		THROW(tra.Commit());
	}
    CATCHZOK
	ASSIGN_PTR(pBillID, bill_id);
	return ok;
}

int SLAPI PPEgaisProcessor::ExpandBaseOpList(const PPIDArray & rBaseOpList, PPIDArray & rResultList)
{
	int    ok = -1;
	rResultList.clear();
	PPObjOprKind op_obj;
	ObjRestrictArray or_list;
	for(uint i = 0; i < rBaseOpList.getCount(); i++) {
		const PPID base_op_id = rBaseOpList.get(i);
		if(base_op_id) {
			if(IsGenericOp(base_op_id) > 0) {
				or_list.clear();
				ObjRestrictItem * p_or_item;
				op_obj.GetGenericList(base_op_id, &or_list);
				for(uint oppos = 0; or_list.enumItems(&oppos, (void **)&p_or_item);)
					rResultList.add(p_or_item->ObjID);
			}
			else
				rResultList.add(base_op_id);
		}
	}
	rResultList.sortAndUndup();
	if(rResultList.getCount())
		ok = 1;
	return ok;
}

int SLAPI PPEgaisProcessor::GetAcceptedBillList(const SendBillsParam & rP, long flags, PPIDArray & rList)
{
	rList.clear();
	int    ok = -1;
	Reference * p_ref = PPRef;
	uint   i;
	SString temp_buf;
	PPObjOprKind op_obj;
	PPIDArray temp_bill_list;
	PPIDArray base_op_list;
	PPIDArray op_list;
	BillTbl::Rec bill_rec;
	ObjTagItem tag_item;
	base_op_list.add(ACfg.Hdr.EgaisRcptOpID);
	if(!(flags & bilstfRepeal))
		base_op_list.add(ACfg.Hdr.EgaisRetOpID);
	ExpandBaseOpList(base_op_list, op_list);
	for(i = 0; i < op_list.getCount(); i++) {
		const PPID op_id = op_list.get(i);
		if(op_id) {
			PPOprKind op_rec;
			GetOpData(op_id, &op_rec);
			if(rP.IdList.getCount()) {
				for(uint j = 0; j < rP.IdList.getCount(); j++) {
					const PPID bill_id = rP.IdList.get(j);
                    if(P_BObj->Search(bill_id, &bill_rec) > 0 && bill_rec.OpID == op_id) {
						if(bill_rec.EdiOp == PPEDIOP_EGAIS_WAYBILL && (!rP.LocID || bill_rec.LocID == rP.LocID)) {
							if(!(flags & bilstfWritedOff) || (bill_rec.Flags & BILLF_WRITEDOFF)) {
								if(CheckBillForMainOrgID(bill_rec, op_rec)) {
									temp_bill_list.add(bill_rec.ID);
								}
							}
						}
                    }
				}
			}
			else {
				for(DateIter di(&rP.Period); P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
					if(bill_rec.EdiOp == PPEDIOP_EGAIS_WAYBILL && (!rP.LocID || bill_rec.LocID == rP.LocID)) {
						if(!(flags & bilstfWritedOff) || (bill_rec.Flags & BILLF_WRITEDOFF)) {
							if(CheckBillForMainOrgID(bill_rec, op_rec)) {
								temp_bill_list.add(bill_rec.ID);
							}
						}
					}
				}
			}
		}
	}
	temp_bill_list.sortAndUndup();
	for(uint k = 0; k < temp_bill_list.getCount(); k++) {
		const PPID bill_id = temp_bill_list.get(k);
		if(P_BObj->Search(bill_id, &bill_rec) > 0) {
			int    suited = 1;
			if(flags & bilstfReadyForAck) {
				if(!P_BObj->CheckStatusFlag(bill_rec.StatusID, BILSTF_READYFOREDIACK)) {
					int    wroff_bill_is_ready = -1;
					BillTbl::Rec wroff_bill_rec;
					for(DateIter diter; P_BObj->P_Tbl->EnumLinks(bill_rec.ID, &diter, BLNK_WROFFDRAFT, &wroff_bill_rec) > 0;) {
						if(!P_BObj->CheckStatusFlag(wroff_bill_rec.StatusID, BILSTF_READYFOREDIACK)) {
							wroff_bill_is_ready = 0;
							break;
						}
						else
							wroff_bill_is_ready = 1;
					}
					if(wroff_bill_is_ready <= 0)
						suited = 0;
				}
			}
			if(suited && p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIACK, temp_buf) > 0) {
				if(flags & bilstfRepeal) {
					if(temp_buf.C(0) != '!')
						suited = 0;
					else if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIREPEALACK, temp_buf) > 0) {
						//
						// ������ �� ������ ���������� ��� ���������
						//
						suited = 0;
					}
				}
				else {
					//
					// �� ���������� � ������������� ����� PPTAG_BILL_EDIACK ��� ���������� �������������
					//
					suited = 0;
				}
			}
			if(suited) {
				rList.add(bill_rec.ID);
				ok = 1;
			}
		}
	}
	return ok;
}

int SLAPI PPEgaisProcessor::SendBillActs(const SendBillsParam & rP)
{
	int    ok = -1;
	PPIDArray accepted_bill_list;
	GetAcceptedBillList(rP, PPEgaisProcessor::bilstfReadyForAck, accepted_bill_list);
	for(uint i = 0; i < accepted_bill_list.getCount(); i++) {
		const PPID bill_id = accepted_bill_list.get(i);
		PPEgaisProcessor::Packet pack(PPEDIOP_EGAIS_WAYBILLACT);
		if(P_BObj->ExtractPacket(bill_id, (PPBillPacket *)pack.P_Data) > 0) {
            Ack ack;
            const int r = PutQuery(pack, rP.LocID, "WayBillAct", ack);
			if(r > 0) {
				ok = 1;
			}
			else if(r == 0)
				LogLastError();
		}
	}
	//CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::SendBillRepeals(const SendBillsParam & rP)
{
	int    ok = -1;
	PPIDArray accepted_bill_list;
	GetAcceptedBillList(rP, PPEgaisProcessor::bilstfReadyForAck|PPEgaisProcessor::bilstfRepeal, accepted_bill_list);
	for(uint i = 0; i < accepted_bill_list.getCount(); i++) {
		const PPID bill_id = accepted_bill_list.get(i);
		PPEgaisProcessor::Packet pack(PPEDIOP_EGAIS_REQUESTREPEALWB);
		PPBillPacket bp;
		if(P_BObj->ExtractPacket(bill_id, &bp) > 0) {
            Ack ack;
			RepealWb * p_rwb = (RepealWb *)pack.P_Data;
			p_rwb->BillID = bp.Rec.ID;
			BillCore::GetCode(p_rwb->ReqNumber = bp.Rec.Code);
			p_rwb->ReqNumber.CatChar('-').Cat("repeal");
			if(bp.BTagL.GetItemStr(PPTAG_BILL_EDIIDENT, p_rwb->TTNCode) > 0) {
				const int r = PutQuery(pack, rP.LocID, "RequestRepealWB", ack);
				if(r > 0) {
					ok = 1;
				}
				else if(r == 0)
					LogLastError();
			}
		}
	}
	//CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::GetBillListForTransmission(const SendBillsParam & rP, long flags, PPIDArray & rList, PPIDArray * pRejectList)
{
	rList.clear();
	int    ok = -1;
	Reference * p_ref = PPRef;
	uint   i;
	SString edi_ack;
	SString edi_rejectack;
	SString edi_ident;
	ObjTagItem tag_item;
	PPIDArray op_list;
	// @v9.0.10 PPIDArray alc_goods_list;
	PPObjOprKind op_obj;
	BillTbl::Rec bill_rec;
	PPIDArray temp_bill_list;
	{
		PPIDArray base_op_list;
		if(flags & bilstfChargeOn)
			base_op_list.add(PPOPK_EDI_ACTCHARGEON);
		// @v9.2.11 {
		if(flags & bilstfChargeOnShop)
			base_op_list.add(PPOPK_EDI_ACTCHARGEONSHOP);
		if(flags & (bilstfTransferToShop|bilstfTransferFromShop))
			base_op_list.add(PPOPK_EDI_SHOPCHARGEON);
		// } @v9.2.11
		// @v9.4.0 {
		if(flags & bilstfWriteOffShop)
			base_op_list.add(PPOPK_EDI_WRITEOFFSHOP);
		// } @v9.4.0
		if(flags & bilstfExpend)
			base_op_list.add(Cfg.ExpndOpID);
		if(flags & bilstfIntrExpend)
			base_op_list.add(Cfg.IntrExpndOpID);
		if(flags & bilstfReturnToSuppl)
			base_op_list.add(Cfg.SupplRetOpID);
		if(flags & bilstfLosses)
			base_op_list.add(Cfg.ExpndEtcOpID);
		ExpandBaseOpList(base_op_list, op_list);
	}
	// @v9.0.10 GoodsIterator::GetListByGroup(Cfg.AlcGoodsGrpID, &alc_goods_list);
    for(i = 0; i < op_list.getCount(); i++) {
		const PPID op_id = op_list.get(i);
		PPOprKind op_rec;
		GetOpData(op_id, &op_rec);
		if(rP.IdList.getCount()) {
			for(uint j = 0; j < rP.IdList.getCount(); j++) {
				const PPID bill_id = rP.IdList.get(j);
				if(P_BObj->Search(bill_id, &bill_rec) > 0 && bill_rec.OpID == op_id) {
					if(!rP.LocID || bill_rec.LocID == rP.LocID) {
						int    suited = 1;
						if(flags & bilstfReadyForAck && !P_BObj->CheckStatusFlag(bill_rec.StatusID, BILSTF_READYFOREDIACK))
							suited = 0;
						else if(CheckBillForMainOrgID(bill_rec, op_rec))
							temp_bill_list.add(bill_rec.ID);
					}
				}
			}
		}
		else {
			for(DateIter di(&rP.Period); P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
				if(!rP.LocID || bill_rec.LocID == rP.LocID) {
					int    suited = 1;
					if(flags & bilstfReadyForAck && !P_BObj->CheckStatusFlag(bill_rec.StatusID, BILSTF_READYFOREDIACK))
						suited = 0;
					else if(CheckBillForMainOrgID(bill_rec, op_rec))
						temp_bill_list.add(bill_rec.ID);
				}
			}
		}
    }
	temp_bill_list.sortAndUndup();
	for(uint k = 0; k < temp_bill_list.getCount(); k++) {
		const PPID bill_id = temp_bill_list.get(k);
		if(P_BObj->Search(bill_id, &bill_rec) > 0) {
			int    suited = 1;
			int    for_reject = 0;
			p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_id, PPTAG_BILL_EDIACK, edi_ack);
			p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_id, PPTAG_BILL_EDIIDENT, edi_ident);
			p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_id, PPTAG_BILL_EDIREJECTACK, edi_rejectack);
			if(edi_ack.NotEmpty() && edi_ident.NotEmpty() && bill_rec.Flags2 & BILLF2_DECLINED && edi_rejectack.Empty()) {
				for_reject = 1; // �� ��������� ���������� ��������� ����� �� ���������� //
			}
			else {
				if(suited && edi_ack.NotEmpty()) {
					//
					// �� ���������� � ������������� ����� PPTAG_BILL_EDIACK ��� ���������� �������������
					//
					suited = 0;
				}
				if(suited && (flags & (bilstfExpend|bilstfIntrExpend)) && edi_ident.NotEmpty()) {
					//
					// �������� ��� ���������
					//
					suited = 0;
				}
			}
			if(suited) {
				PPBillPacket pack;
				suited = 0;
				if(P_BObj->ExtractPacket(bill_id, &pack) > 0) {
					int    has_plus = 0;
					int    has_minus = 0;
					for(uint tidx = 0; !suited && tidx < pack.GetTCount(); tidx++) {
						const PPTransferItem & r_ti = pack.ConstTI(tidx);
						// @v9.0.10 if(alc_goods_list.bsearch(r_ti.GoodsID))
						if(IsAlcGoods(r_ti.GoodsID)) { // @v9.0.10
							if(r_ti.Quantity_ > 0.0)
								has_plus = 1;
							else if(r_ti.Quantity_ < 0.0)
								has_minus = 1;
							suited = 1;

						}
					}
					if(suited) {
						if((flags & bilstfTransferToShop) && !has_plus)
							suited = 0;
						else if((flags & bilstfTransferFromShop) && !has_minus)
							suited = 0;
					}
				}
				if(suited) {
					if(for_reject) {
						CALLPTRMEMB(pRejectList, add(bill_rec.ID));
					}
					else
						rList.add(bill_id);
					ok = 1;
				}
			}
		}
	}
	return ok;
}

int SLAPI PPEgaisProcessor::CheckBillForMainOrgID(const BillTbl::Rec & rRec, const PPOprKind & rOpRec)
{
	int    ok = 1;
	const  PPID utm_main_org_id = P_UtmEntry ? P_UtmEntry->MainOrgID : 0;
	if(utm_main_org_id && rOpRec.AccSheet2ID) {
		PPAccSheet acs_rec;
		if(AcsObj.Fetch(rOpRec.AccSheet2ID, &acs_rec) > 0 && acs_rec.Assoc == PPOBJ_PERSON && acs_rec.ObjGroup == PPPRK_MAIN) {
			if(rRec.Object2 && ObjectToPerson(rRec.Object2, 0) != utm_main_org_id)
				ok = 0;
		}
	}
	return ok;
}

int SLAPI PPEgaisProcessor::GetBillListForConfirmTicket(const SendBillsParam & rP, long flags, PPIDArray & rList)
{
	rList.clear();
	int    ok = -1;
	Reference * p_ref = PPRef;
	uint   i;
	BillTbl::Rec bill_rec;
	SString temp_buf;
	ObjTagItem tag_item;
	PPIDArray op_list;
	PPObjOprKind op_obj;
	{
		PPIDArray base_op_list;
		if(flags & bilstfExpend)
			base_op_list.add(Cfg.ExpndOpID);
		if(flags & bilstfIntrExpend)
			base_op_list.add(Cfg.IntrExpndOpID);
		if(flags & bilstfReturnToSuppl)
			base_op_list.add(Cfg.SupplRetOpID);
		ExpandBaseOpList(base_op_list, op_list);
	}
    for(i = 0; i < op_list.getCount(); i++) {
		const PPID op_id = op_list.get(i);
		PPOprKind op_rec;
		GetOpData(op_id, &op_rec);
		if(rP.IdList.getCount()) {
			for(uint j = 0; j < rP.IdList.getCount(); j++) {
				const PPID bill_id = rP.IdList.get(j);
				if(P_BObj->Search(bill_id, &bill_rec) > 0 && bill_rec.OpID == op_id) {
					if(!rP.LocID || bill_rec.LocID == rP.LocID) {
						int    suited = 1;
						const  int recadv_status = BillCore::GetRecadvStatus(bill_rec);
						const  int recadv_conf_status = BillCore::GetRecadvConfStatus(bill_rec);
						if(!recadv_status && !recadv_conf_status)
							suited = 0;
						else if(flags & bilstfReadyForAck && !P_BObj->CheckStatusFlag(bill_rec.StatusID, BILSTF_READYFOREDIACK))
							suited = 0;
						else if(!CheckBillForMainOrgID(bill_rec, op_rec))
							suited = 0;
						else if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIIDENT, temp_buf) <= 0)
							suited = 0;
						else if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIACK, temp_buf) <= 0)
							suited = 0;
						else if(p_ref->Ot.GetTag(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIRECADVCONFIRM, &tag_item) > 0) {
							S_GUID uuid;
							if(tag_item.GetGuid(&uuid) && !uuid.IsZero())
								suited = 0;
						}
						if(suited) {
							rList.add(bill_rec.ID);
							ok = 1;
						}
					}
				}
			}
		}
		else {
			for(DateIter di(&rP.Period); P_BObj->P_Tbl->EnumByOpr(op_id, &di, &bill_rec) > 0;) {
				if(!rP.LocID || bill_rec.LocID == rP.LocID) {
					int    suited = 1;
					const  int recadv_status = BillCore::GetRecadvStatus(bill_rec);
					const  int recadv_conf_status = BillCore::GetRecadvConfStatus(bill_rec);
					if(!recadv_status && !recadv_conf_status)
						suited = 0;
					else if(flags & bilstfReadyForAck && !P_BObj->CheckStatusFlag(bill_rec.StatusID, BILSTF_READYFOREDIACK))
						suited = 0;
					else if(!CheckBillForMainOrgID(bill_rec, op_rec))
						suited = 0;
					else if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIIDENT, temp_buf) <= 0)
						suited = 0;
					else if(p_ref->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIACK, temp_buf) <= 0)
						suited = 0;
					else if(p_ref->Ot.GetTag(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIRECADVCONFIRM, &tag_item) > 0) {
						S_GUID uuid;
						if(tag_item.GetGuid(&uuid) && !uuid.IsZero())
							suited = 0;
					}
					if(suited) {
						rList.add(bill_rec.ID);
						ok = 1;
					}
				}
			}
		}
    }
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_SendBills(PPID billID, int ediOp, PPID locID, const char * pUrlSuffix)
{
	int    ok = -1;
	PPEgaisProcessor::Packet pack(ediOp);
	if(P_BObj->ExtractPacket(billID, (PPBillPacket *)pack.P_Data, BPLD_FORCESERIALS) > 0) {
		Ack ack;
		const int r = PutQuery(pack, locID, pUrlSuffix, ack);
		if(r > 0)
			ok = 1;
		else if(r == 0)
			LogLastError();
	}
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_SendBillsByPattern(const SendBillsParam & rP, const PPEgaisProcessor::BillTransmissionPattern & rPattern)
{
	int    ok = -1;
	SString file_name;
	SString temp_buf;
	PPIDArray totransm_bill_list;
	GetBillListForTransmission(rP, rPattern.Flags, totransm_bill_list, 0);
	if(totransm_bill_list.getCount()) {
		if(rPattern.EdiOp == PPEDIOP_EGAIS_ACTCHARGEON) {
			ExclChrgOnMarks.clear();
			PPGetFilePath(PPPATH_IN, "egais-exclude-chargeon-marks.txt", file_name);
			if(fileExists(file_name)) {
				SFile f_in(file_name, SFile::mRead);
				while(f_in.ReadLine(temp_buf)) {
					if(PrcssrAlcReport::IsEgaisMark(temp_buf.Chomp(), 0))
						ExclChrgOnMarks.add(temp_buf);
				}
			}
		}
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			if(Helper_SendBills(totransm_bill_list.get(i), rPattern.EdiOp, rP.LocID, rPattern.P_UrlSuffix) > 0)
				ok = 1;
		}
	}
	return ok;
}

int SLAPI PPEgaisProcessor::SendBills(const SendBillsParam & rP)
{
	int    ok = -1;
	SString file_name;
	SString temp_buf;
	PPIDArray totransm_bill_list, reject_bill_list;
	// @v9.4.1 {
	{
		const PPEgaisProcessor::BillTransmissionPattern _BillTransmPatterns[] = {
			{ bilstfReadyForAck|bilstfChargeOn,         PPEDIOP_EGAIS_ACTCHARGEON,       "ActChargeOn" },
			{ bilstfReadyForAck|bilstfChargeOnShop,     PPEDIOP_EGAIS_ACTCHARGEONSHOP,   "ActChargeOnShop_v2" },
			{ bilstfReadyForAck|bilstfWriteOffShop,     PPEDIOP_EGAIS_ACTWRITEOFFSHOP,   "ActWriteOffShop_v2" },
			{ bilstfReadyForAck|bilstfTransferToShop,   PPEDIOP_EGAIS_TRANSFERTOSHOP,    "TransferToShop" },
			{ bilstfReadyForAck|bilstfTransferFromShop, PPEDIOP_EGAIS_TRANSFERFROMSHOP,  "TransferFromShop" },
			{ bilstfReadyForAck|bilstfLosses,           PPEDIOP_EGAIS_ACTWRITEOFF,       "ActWriteOff" }
		};
		for(uint i = 0; i < SIZEOFARRAY(_BillTransmPatterns); i++) {
			if(Helper_SendBillsByPattern(rP, _BillTransmPatterns[i]) > 0)
				ok = 1;
		}
	}
	// } @v9.4.1
#if 0 // @v9.4.1 {
	{
		//
		// �������� ���������� ���������� �������� �� ������
		//
		totransm_bill_list.clear();
		GetBillListForTransmission(rP, bilstfReadyForAck|bilstfChargeOn, totransm_bill_list, 0);
		if(totransm_bill_list.getCount()) {
			{
				ExclChrgOnMarks.clear();
                PPGetFilePath(PPPATH_IN, "egais-exclude-chargeon-marks.txt", file_name);
				if(fileExists(file_name)) {
					SFile f_in(file_name, SFile::mRead);
					while(f_in.ReadLine(temp_buf)) {
						if(PrcssrAlcReport::IsEgaisMark(temp_buf.Chomp(), 0))
                            ExclChrgOnMarks.add(temp_buf);
					}
				}
			}
			for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
				if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_ACTCHARGEON, rP.LocID, "ActChargeOn") > 0)
					ok = 1;
			}
		}
	}
	// @v9.2.11 {
	{
		//
		// �������� ���������� ���������� �������� �� ������ � �������� ���� (������� 2)
		//
		totransm_bill_list.clear();
		GetBillListForTransmission(rP, bilstfReadyForAck|bilstfChargeOnShop, totransm_bill_list, 0);
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_ACTCHARGEONSHOP, rP.LocID, "ActChargeOnShop_v2") > 0)
				ok = 1;
		}
	}
	// } @v9.2.11
	// @v9.4.0 {
	{
		//
		// �������� ���������� �������� �������� � ������� � �������� ���� (������� 2)
		//
		totransm_bill_list.clear();
		GetBillListForTransmission(rP, bilstfReadyForAck|bilstfWriteOffShop, totransm_bill_list, 0);
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_ACTWRITEOFFSHOP, rP.LocID, "ActWriteOffShop_v2") > 0)
				ok = 1;
		}
	}
	// } @v9.4.0
	// @v9.2.11 {
	{
		//
		// �������� ���������� ����������� ������� � �������� ��� (������� 2)
		//
		totransm_bill_list.clear();
		GetBillListForTransmission(rP, bilstfReadyForAck|bilstfTransferToShop, totransm_bill_list, 0);
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_TRANSFERTOSHOP, rP.LocID, "TransferToShop") > 0)
				ok = 1;
		}
	}
	{
		//
		// �������� ���������� ����������� ������� �� ��������� ���� (������� 2) �� �����
		//
		totransm_bill_list.clear();
		GetBillListForTransmission(rP, bilstfReadyForAck|bilstfTransferFromShop, totransm_bill_list, 0);
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_TRANSFERFROMSHOP, rP.LocID, "TransferFromShop") > 0)
				ok = 1;
		}
	}
	// } @v9.2.11
	{
		//
		// �������� ���������� ������� ������� (��� ������)
		//
		totransm_bill_list.clear();
		GetBillListForTransmission(/*locID, period*/rP, bilstfReadyForAck|bilstfLosses, totransm_bill_list, 0);
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_ACTWRITEOFF, rP.LocID, "ActWriteOff") > 0)
				ok = 1;
		}
	}
#endif // } 0 @v9.4.1
	{
		//
		// �������� ���������� ��������
		//
		totransm_bill_list.clear();
		reject_bill_list.clear();
		GetBillListForTransmission(/*locID, period*/rP, bilstfReadyForAck|bilstfExpend|bilstfIntrExpend, totransm_bill_list, &reject_bill_list);
		{
			for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
				if(Helper_SendBills(totransm_bill_list.get(i), PPEDIOP_EGAIS_WAYBILL, rP.LocID, "WayBill") > 0)
					ok = 1;
			}
		}
		{
			for(uint i = 0; i < reject_bill_list.getCount(); i++) {
				const PPID bill_id = reject_bill_list.get(i);
				PPEgaisProcessor::Packet pack(PPEDIOP_EGAIS_WAYBILLACT);
				PPBillPacket * p_bp = (PPBillPacket *)pack.P_Data;
				if(P_BObj->ExtractPacket(bill_id, p_bp, BPLD_FORCESERIALS) > 0) {
					if(p_bp->Rec.Flags2 & BILLF2_DECLINED && p_bp->BTagL.GetItemStr(PPTAG_BILL_EDIIDENT, temp_buf) > 0) {
						Ack ack;
						const int r = PutQuery(pack, rP.LocID, "WayBillAct", ack);
						if(r > 0)
							ok = 1;
						else if(r == 0)
							LogLastError();
					}
				}
			}
		}
	}
	{
		//
		// �������� ���������� �������� ����������
		//
		totransm_bill_list.clear();
		reject_bill_list.clear();
		GetBillListForTransmission(/*locID, period*/rP, bilstfReadyForAck|bilstfReturnToSuppl, totransm_bill_list, &reject_bill_list);
		{
			for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
				const PPID bill_id = totransm_bill_list.get(i);
				PPEgaisProcessor::Packet pack(PPEDIOP_EGAIS_WAYBILL);
				pack.Flags |= PPEgaisProcessor::Packet::fReturnBill;
				PPBillPacket * p_bp = (PPBillPacket *)pack.P_Data;
				if(P_BObj->ExtractPacket(bill_id, p_bp, BPLD_FORCESERIALS) > 0) {
					Ack ack;
					const int r = PutQuery(pack, rP.LocID, "WayBill", ack);
					if(r > 0)
						ok = 1;
					else if(r == 0)
						LogLastError();
				}
			}
		}
		{
			for(uint i = 0; i < reject_bill_list.getCount(); i++) {
				const PPID bill_id = reject_bill_list.get(i);
				PPEgaisProcessor::Packet pack(PPEDIOP_EGAIS_WAYBILLACT);
				pack.Flags |= PPEgaisProcessor::Packet::fReturnBill;
				PPBillPacket * p_bp = (PPBillPacket *)pack.P_Data;
				if(P_BObj->ExtractPacket(bill_id, p_bp, BPLD_FORCESERIALS) > 0) {
					if(p_bp->Rec.Flags2 & BILLF2_DECLINED && p_bp->BTagL.GetItemStr(PPTAG_BILL_EDIIDENT, temp_buf) > 0) {
						Ack ack;
						const int r = PutQuery(pack, rP.LocID, "WayBillAct", ack);
						if(r > 0)
							ok = 1;
						else if(r == 0)
							LogLastError();
					}
				}
			}
		}
	}
	{
		//
		// ���������� �����������
		//
		totransm_bill_list.clear();
		GetBillListForConfirmTicket(rP, bilstfReadyForAck|bilstfExpend|bilstfIntrExpend, totransm_bill_list);
		ObjTagItem tag_item;
		SString reg_ident;
		for(uint i = 0; i < totransm_bill_list.getCount(); i++) {
			const PPID bill_id = totransm_bill_list.get(i);
			BillTbl::Rec bill_rec;
			if(P_BObj->Search(bill_id, &bill_rec) > 0) {
				if(PPRef->Ot.GetTagStr(PPOBJ_BILL, bill_rec.ID, PPTAG_BILL_EDIIDENT, reg_ident) > 0) {
					PPEgaisProcessor::Packet pack(PPEDIOP_EGAIS_CONFIRMTICKET);
					ConfirmTicket * p_ticket = (ConfirmTicket *)pack.P_Data;
					p_ticket->BillID = bill_rec.ID;
					p_ticket->Conclusion = -1;
					const int recadv_conf_status = BillCore::GetRecadvConfStatus(bill_rec);
					if(recadv_conf_status == PPEDI_RECADVCONF_STATUS_ACCEPT)
						p_ticket->Conclusion = 1;
					else if(recadv_conf_status == PPEDI_RECADVCONF_STATUS_REJECT)
						p_ticket->Conclusion = 0;
					if(oneof2(p_ticket->Conclusion, 0, 1)) {
						Ack ack;
						p_ticket->Code = bill_rec.Code;
                        BillCore::GetCode(p_ticket->Code);
                        p_ticket->Code.CatChar('-').Cat("RECADVCFM");
						p_ticket->Date = getcurdate_();
						p_ticket->RegIdent = reg_ident;
						const int r = PutQuery(pack, rP.LocID, "WayBillTicket", ack);
						if(r > 0)
							ok = 1;
						else if(r == 0)
							LogLastError();
					}
				}
			}
		}
	}
	//CATCHZOK
	return ok;
}

SLAPI PPEgaisProcessor::QueryParam::QueryParam()
{
	DocType = 0;
	DbActualizeFlags = 0;
	Flags = 0;
	MainOrgID = 0;
	LocID = 0;
}

int SLAPI PPEgaisProcessor::QueryParam::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pSCtx)
{
	int    ok = 1;
	THROW(pSCtx->Serialize(dir, DocType, rBuf));
	THROW(pSCtx->Serialize(dir, DbActualizeFlags, rBuf));
	THROW(pSCtx->Serialize(dir, Flags, rBuf));
	THROW(pSCtx->Serialize(dir, MainOrgID, rBuf));
	THROW(pSCtx->Serialize(dir, LocID, rBuf));
	THROW(pSCtx->Serialize(dir, ParamString, rBuf));
	CATCHZOK
	return ok;
}

int SLAPI PPEgaisProcessor::EditQueryParam(PPEgaisProcessor::QueryParam * pData)
{
	class EgaisQDialog : public TDialog {
	public:
		EgaisQDialog(PPEgaisProcessor * pPrc) : TDialog(DLG_EGAISQ)
		{
			P_Prc = pPrc;
			Prev_afClearInnerEgaisDb_State = 0;
		}
        void   DisplayInfo(const char * pInfo)
        {
			SString msg_buf;
			if(StartUpInfo.NotEmpty())
				msg_buf.Cat(StartUpInfo).CR();
			msg_buf.Cat(pInfo);
			setStaticText(CTL_EGAISQ_INFO, msg_buf);
        }
        void   SetupStartUpInfo()
        {
        	StartUpInfo = 0;
			if(P_Prc) {
				PPID   main_org_id = 0;
				PPID   loc_id = 0;
				SString url, temp_buf;
				getCtrlData(CTLSEL_EGAISQ_MAINORG, &main_org_id);
				getCtrlData(CTLSEL_EGAISQ_LOC, &loc_id);
				{
					PPID   preserve_main_org_id = 0;
					GetMainOrgID(&preserve_main_org_id);
					if(main_org_id)
						DS.SetMainOrgID(main_org_id, 0);
					if(P_Prc->GetFSRARID(loc_id, temp_buf = 0, 0))
						StartUpInfo.Cat(temp_buf);
					else
						StartUpInfo.Cat("fasrarid-undef");
					StartUpInfo.Space();
					if(P_Prc->GetURL(loc_id, url = 0))
						StartUpInfo.Cat(url);
					else
						StartUpInfo.Cat("url-undef");
					DS.SetMainOrgID(preserve_main_org_id, 0);
				}
			}
        }
		int    setDTS(const PPEgaisProcessor::QueryParam * pData)
		{
			int    ok = 1;
			RVALUEPTR(Data, pData);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  0, PPEDIOP_EGAIS_QUERYCLIENTS);
			AddClusterAssoc(CTL_EGAISQ_WHAT, -1, PPEDIOP_EGAIS_QUERYCLIENTS);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  1, PPEDIOP_EGAIS_QUERYCLIENTS+1000);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  2, PPEDIOP_EGAIS_QUERYAP);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  3, PPEDIOP_EGAIS_QUERYFORMA);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  4, PPEDIOP_EGAIS_QUERYFORMB);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  5, PPEDIOP_EGAIS_QUERYRESTS);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  6, PPEDIOP_EGAIS_QUERYRESTSSHOP);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  7, PPEDIOP_EGAIS_QUERYBARCODE);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  8, PPEDIOP_EGAIS_QUERYCLIENTS+2000);
			AddClusterAssoc(CTL_EGAISQ_WHAT,  9, PPEDIOP_EGAIS_QUERYCLIENTS+3000);
			SetClusterData(CTL_EGAISQ_WHAT, Data.DocType);
			setCtrlString(CTL_EGAISQ_QADD, Data.ParamString);
			SetupPersonCombo(this, CTLSEL_EGAISQ_MAINORG, Data.MainOrgID, 0, PPPRK_MAIN, 1);
			SetupLocationCombo(this, CTLSEL_EGAISQ_LOC, Data.LocID, 0, LOCTYP_WAREHOUSE, 0);

			AddClusterAssoc(CTL_EGAISQ_ACTLZVAR, 0, Data._afQueryRefA);
			AddClusterAssoc(CTL_EGAISQ_ACTLZVAR, 1, Data._afQueryPerson);
			AddClusterAssoc(CTL_EGAISQ_ACTLZVAR, 2, Data._afQueryGoods);
			AddClusterAssoc(CTL_EGAISQ_ACTLZVAR, 3, Data._afQueryByChargeOn);
			AddClusterAssoc(CTL_EGAISQ_ACTLZVAR, 4, Data._afClearInnerEgaisDb); // @v9.3.4
			SetClusterData(CTL_EGAISQ_ACTLZVAR, Data.DbActualizeFlags);
			Prev_afClearInnerEgaisDb_State = BIN(Data.DbActualizeFlags & Data._afClearInnerEgaisDb);

			AddClusterAssoc(CTL_EGAISQ_FLAGS, 0, stTestSendingMode);
			SetClusterData(CTL_EGAISQ_FLAGS, Data.Flags);

			setStaticText(CTL_EGAISQ_INFO, Data.InfoText);

			disableCtrl(CTL_EGAISQ_ACTLZVAR, Data.DocType != (PPEDIOP_EGAIS_QUERYCLIENTS+2000));
			SetupStartUpInfo();
			DisplayInfo(0);
			return ok;
		}
		int    getDTS(PPEgaisProcessor::QueryParam * pData)
		{
			int    ok = 1;
			Data.DocType = GetClusterData(CTL_EGAISQ_WHAT);
			getCtrlString(CTL_EGAISQ_QADD, Data.ParamString);
			getCtrlData(CTLSEL_EGAISQ_MAINORG, &Data.MainOrgID);
			getCtrlData(CTLSEL_EGAISQ_LOC, &Data.LocID);
			GetClusterData(CTL_EGAISQ_FLAGS, &Data.Flags);
			GetClusterData(CTL_EGAISQ_ACTLZVAR, &Data.DbActualizeFlags);
			ASSIGN_PTR(pData, Data);
			return ok;
		}
	private:
        DECL_HANDLE_EVENT
        {
        	TDialog::handleEvent(event);
			if(event.isClusterClk(CTL_EGAISQ_WHAT)) {
				long doc_type = GetClusterData(CTL_EGAISQ_WHAT);
				SString msg_buf;
				if(doc_type == (PPEDIOP_EGAIS_QUERYCLIENTS+2000)) {
					disableCtrl(CTL_EGAISQ_ACTLZVAR, 0);
					DisplayInfo(0);
				}
				else {
					disableCtrl(CTL_EGAISQ_ACTLZVAR, 1);
					if(doc_type == PPEDIOP_EGAIS_QUERYBARCODE) {
						PPLoadText(PPTXT_HINT_EGAIS_QBARCODE, msg_buf);
						DisplayInfo(msg_buf);
					}
					else if(doc_type == PPEDIOP_EGAIS_QUERYCLIENTS) {
						PPLoadText(PPTXT_HINT_EGAIS_QCLIENTS, msg_buf);
						DisplayInfo(msg_buf);
					}
					else if(doc_type == PPEDIOP_EGAIS_QUERYAP) {
						PPLoadText(PPTXT_HINT_EGAIS_QPRODUCTS, msg_buf);
						DisplayInfo(msg_buf);
					}
					else if(doc_type == PPEDIOP_EGAIS_QUERYRESTS) {
						PPLoadText(PPTXT_HINT_EGAIS_QRESTS, msg_buf);
						DisplayInfo(msg_buf);
					}
					else if(doc_type == PPEDIOP_EGAIS_QUERYRESTSSHOP) {
						PPLoadText(PPTXT_HINT_EGAIS_QRESTSSHOP, msg_buf);
						DisplayInfo(msg_buf);
					}
					else {
						DisplayInfo(0);
					}
				}
			}
			else if(event.isClusterClk(CTL_EGAISQ_ACTLZVAR)) {
				long _af = 0;
				GetClusterData(CTL_EGAISQ_ACTLZVAR, &_af);
				if(!(_af & PPEgaisProcessor::QueryParam::_afClearInnerEgaisDb)) {
					Prev_afClearInnerEgaisDb_State = 0;
				}
				else if(!Prev_afClearInnerEgaisDb_State) {
					if(CONFIRM(PPCFM_CLEARINNEREGAISDB)) {
						Prev_afClearInnerEgaisDb_State = 1;
					}
					else {
						_af &= ~PPEgaisProcessor::QueryParam::_afClearInnerEgaisDb;
						SetClusterData(CTL_EGAISQ_ACTLZVAR, _af);
					}
				}
			}
			else if(event.isCbSelected(CTLSEL_EGAISQ_LOC) || event.isCbSelected(CTLSEL_EGAISQ_MAINORG)) {
				SetupStartUpInfo();
				DisplayInfo(0);
			}
			else
				return;
			clearEvent(event);
        }
        SString StartUpInfo;
        PPEgaisProcessor * P_Prc;
		PPEgaisProcessor::QueryParam Data;
		long   Prev_afClearInnerEgaisDb_State;
	};
	int    ok = -1;
	QueryParam _param;
	RVALUEPTR(_param, pData);
	EgaisQDialog * dlg = new EgaisQDialog(this);
	if(CheckDialogPtr(&dlg, 1)) {
		dlg->setDTS(&_param);
		if(ExecView(dlg) == cmOK) {
			dlg->getDTS(&_param);
			ASSIGN_PTR(pData, _param);
            ok = 1;
		}
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

int SLAPI PPEgaisProcessor::Helper_ExtractGoodsCodesFromBills(PPID opID, StringSet & rSs)
{
	int    ok = -1;
	SString temp_buf;
	BillTbl::Rec cobill_rec;
	DateRange coperiod;
	coperiod.Set(encodedate(1, 9, 2016), ZERODATE);
	for(SEnum en = P_BObj->P_Tbl->EnumByOp(opID, &coperiod, 0); en.Next(&cobill_rec) > 0;) {
		const PPID sobill_id = cobill_rec.ID;
		PPBillPacket sobill_pack;
		if(P_BObj->ExtractPacket(sobill_id, &sobill_pack, BPLD_FORCESERIALS) > 0) {
			for(uint k = 0; k < sobill_pack.GetTCount(); k++) {
				const PPTransferItem & r_so_ti = sobill_pack.ConstTI(k);
				if(sobill_pack.LTagL.GetTagStr(k, PPTAG_LOT_FSRARLOTGOODSCODE, temp_buf) > 0) {
					if(temp_buf.Len() > 8 && P_RefC->PrC.IsVerifiedCode(temp_buf) <= 0) {
						rSs.add(temp_buf);
						ok = 1;
					}
				}
			}
		}
	}
	return ok;
}

int SLAPI PPEgaisProcessor::ImplementQuery(PPEgaisProcessor::QueryParam & rParam)
{
	int    ok = 1;
	int    query_sended = 0;
	int    do_report_error = 0;
	SString mark_buf;
	SString temp_buf;
	PPID preserve_main_org_id = 0;
	GetMainOrgID(&preserve_main_org_id);
	if(rParam.MainOrgID)
		DS.SetMainOrgID(rParam.MainOrgID, 0);
	//
	if(PrcssrAlcReport::IsEgaisMark(rParam.ParamString, &mark_buf)) {
		PrcssrAlcReport::EgaisMarkBlock emb;
		PrcssrAlcReport::ParseEgaisMark(mark_buf, emb);
		(rParam.InfoText = 0).Cat(emb.EgaisCode);
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYCLIENTS) {
		if(QueryClients(rParam.LocID, querybyINN, rParam.ParamString))
			query_sended = 1;
		else
			do_report_error = 1;
	}
	else if(rParam.DocType == (PPEDIOP_EGAIS_QUERYCLIENTS+1000)) {
		if(Cfg.AlcLicRegTypeID) {
			const LDATE _cd = getcurdate_();
			StringSet inn_list;
			SString temp_buf;
			PPViewPerson psn_view;
			PersonFilt psn_filt;
			PersonViewItem psn_item;
			psn_filt.AttribType = PPPSNATTR_REGISTER;
			psn_filt.RegTypeID = Cfg.AlcLicRegTypeID;
			psn_filt.EmptyAttrib = EA_NOEMPTY;
			PPWait(1);
			THROW(psn_view.Init_(&psn_filt));
			for(psn_view.InitIteration(); psn_view.NextIteration(&psn_item) > 0;) {
				RegisterTbl::Rec reg_rec;
				if(PsnObj.GetRegister(psn_item.ID, PPREGT_TPID, _cd, &reg_rec) > 0) {
					(temp_buf = reg_rec.Num).Strip().ToUpper();
					if(!inn_list.search(temp_buf, 0, 0)) {
						inn_list.add(temp_buf);
					}
				}
				PPWaitPercent(psn_view.GetCounter(), "Get contragent INN");
			}
			{
				const uint innc = inn_list.getCount();
				uint   inni = 0;
				for(uint innp = 0; inn_list.get(&innp, temp_buf);) {
					if(QueryClients(rParam.LocID, querybyINN, temp_buf)) {
						query_sended = 1;
					}
					inni++;
					PPWaitPercent(inni, innc, "Query EGAIS contragent");
				}
			}
			PPWait(0);
		}
	}
	else if(rParam.DocType == (PPEDIOP_EGAIS_QUERYCLIENTS+2000)) {
		THROW_MEM(SETIFZ(P_RefC, new RefCollection));
		{
			//
			// ������ ����������� �� ��������������: "���"
			// ������ ������ �� ���� �����: "���"
			//
			const uint _delay_ms = 100;
			int   db_was_cleared = 0;

			SString msg_buf;
			StringSet refa_codes;
			StringSet manuf_codes_for_products; // ��������������
			StringSet codes_for_products;
			StringSet codes_for_contragents;
			StringSet inn_for_contragents;
			PPWait(1);
			if(rParam.DbActualizeFlags & rParam._afClearInnerEgaisDb) {
				PPTransaction tra(1);
				THROW(tra);
				THROW(P_RefC->PrC.Clear(0))
				THROW(P_RefC->PsC.Clear(0));
				THROW(P_RefC->RaC.Clear(0));
				THROW(tra.Commit());
				db_was_cleared = 1;
			}
			PPWaitMsg(PPSTR_TEXT, PPTXT_EGAIS_PREPAREQFORIDB, 0);
			if(rParam.DbActualizeFlags & rParam._afQueryByChargeOn) {
				Helper_ExtractGoodsCodesFromBills(PPOPK_EDI_ACTCHARGEONSHOP, codes_for_products);
				Helper_ExtractGoodsCodesFromBills(PPOPK_EDI_SHOPCHARGEON, codes_for_products);
				Helper_ExtractGoodsCodesFromBills(PPOPK_EDI_WRITEOFFSHOP, codes_for_products);
			}
			if(!db_was_cleared && rParam.DbActualizeFlags & (rParam._afQueryRefA|rParam._afQueryPerson|rParam._afQueryGoods)) {
				BExtQuery q(&P_RefC->RaC, 1);
				EgaisRefATbl::Key1 k1;
				MEMSZERO(k1);
				q.selectAll();
				for(q.initIteration(0, &k1, spFirst); q.nextIteration() > 0;) {
					EgaisRefATbl::Rec item;
					P_RefC->RaC.copyBufTo(&item);
					if(item.RefACode[0]) {
						if(rParam.DbActualizeFlags & rParam._afQueryRefA) {
							if(!(item.Flags & EgaisRefACore::fVerified))
								refa_codes.add(item.RefACode);
						}
						if(rParam.DbActualizeFlags & rParam._afQueryPerson) {
							int    is_importer = 0;
							if(strlen(item.ImporterRarIdent) >= 8) { // ��� ����������� 12 ��������, �� ���������� ��������� 8
								is_importer = 1;
								if(P_RefC->PsC.IsVerifiedCode(item.ImporterRarIdent) <= 0) {
									codes_for_contragents.add(item.ImporterRarIdent);
									manuf_codes_for_products.add(item.ImporterRarIdent);
								}
							}
							if(strlen(item.ManufRarIdent) >= 8) {
								if(P_RefC->PsC.IsVerifiedCode(item.ManufRarIdent) <= 0) {
									codes_for_contragents.add(item.ManufRarIdent);
									if(!is_importer)
										manuf_codes_for_products.add(item.ManufRarIdent);
								}
							}
						}
						if(rParam.DbActualizeFlags & rParam._afQueryGoods) {
							if(strlen(item.AlcCode) > 8) {
								if(P_RefC->PrC.IsVerifiedCode(item.AlcCode) <= 0)
									codes_for_products.add(item.AlcCode);
							}
						}
					}
				}
			}
			refa_codes.sortAndUndup();
			codes_for_contragents.sortAndUndup();
			manuf_codes_for_products.sortAndUndup();
			codes_for_products.sortAndUndup(); // @v9.4.3
			/*
			{
				BExtQuery q(&P_RefC->PsC, 1);
				EgaisPersonTbl::Key1 k1;
				MEMSZERO(k1);
				q.selectAll();
				EgaisPersonCore::Item item;
				for(q.initIteration(0, &k1, spFirst); q.nextIteration() > 0;) {
					if(P_RefC->PsC.RecToItem(P_RefC->PsC.data, item)) {
						if(strlen(item.INN) >= 8)
							inn_for_contragents.add(item.INN);
					}
				}
				inn_for_contragents.sortAndUndup();
			}
			*/
			PPLoadText(PPTXT_EGAIS_REQUESTFORIDB, msg_buf);
			PPWaitMsg(msg_buf);
			{
				long   done_q = 0;
				const  long total_q = refa_codes.getCount() + codes_for_contragents.getCount() + codes_for_products.getCount();
				uint  qc = 0;
				for(uint ssp = 0; refa_codes.get(&ssp, temp_buf);) {
					if(qc)
						delay(_delay_ms);
					if(QueryInfA(rParam.LocID, temp_buf))
						qc++;
					else
						LogLastError();
					done_q++;
					PPWaitPercent(done_q, total_q, msg_buf);
				}
				if(codes_for_contragents.getCount()) {
					for(uint ssp = 0; codes_for_contragents.get(&ssp, temp_buf);) {
						if(qc)
							delay(_delay_ms);
						if(QueryClients(rParam.LocID, querybyCode, temp_buf))
							qc++;
						else
							LogLastError();
						done_q++;
						PPWaitPercent(done_q, total_q, msg_buf);
					}
				}
				if(codes_for_products.getCount()) {
					for(uint ssp = 0; codes_for_products.get(&ssp, temp_buf);) {
						if(qc)
							delay(_delay_ms);
						if(QueryProducts(rParam.LocID, querybyCode, temp_buf))
							qc++;
						else
							LogLastError();
						done_q++;
						PPWaitPercent(done_q, total_q, msg_buf);
					}
				}
			}
			PPWait(0);
		}
	}
	else if(rParam.DocType == (PPEDIOP_EGAIS_QUERYCLIENTS+3000)) {
		const int rmv_debug_mode = (rParam.ParamString.CmpNC("yes") == 0) ? 0 : 1;
		TSArray <PPEgaisProcessor::UtmEntry> utm_list;
		THROW(GetUtmList(rParam.LocID, utm_list));
		PPWait(1);
		for(uint i = 0; i < utm_list.getCount(); i++) {
			SetUtmEntry(rParam.LocID, &utm_list.at(i), 0);
			RemoveOutputMessages(rParam.LocID, rmv_debug_mode);
		}
		PPWait(0);
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYFORMA) {
		if(QueryInfA(rParam.LocID, rParam.ParamString))
			query_sended = 1;
		else
			do_report_error = 1;
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYFORMB) {
		if(QueryInfB(rParam.LocID, rParam.ParamString))
			query_sended = 1;
		else
			do_report_error = 1;
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYAP) {
		if(QueryProducts(rParam.LocID, querybyINN, rParam.ParamString))
			query_sended = 1;
		else
			do_report_error = 1;
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYRESTS) {
		if(QueryRests(rParam.LocID, 0))
			query_sended = 1;
		else
			do_report_error = 1;
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYRESTSSHOP) {
		//
		// @v9.3.1 ��������������� ����� �������� �������� �� �������� 2
		// ����������� ������� �� ������. ��� ����, ����� ��� ��������
		// ��������������� ��������� �������� �� ������ �� ������� 2
		// ��������� ������� �������� ������� � �� ������.
		//
		if(QueryRests(rParam.LocID, 0)) {
			query_sended = 1;
			delay(100);
			if(QueryRestsShop(rParam.LocID, 0))
				query_sended = 1;
			else
				do_report_error = 1;
		}
		else
			do_report_error = 1;
	}
	else if(rParam.DocType == PPEDIOP_EGAIS_QUERYBARCODE) {
		if(rParam.ParamString.NotEmptyS()) {
			//SPathStruc ps;
			//ps.Split()
			StringSet ss(' ', rParam.ParamString);
			Packet qp(PPEDIOP_EGAIS_QUERYBARCODE);
			TSCollection <QueryBarcode> * p_qbl = (TSCollection <QueryBarcode> *)qp.P_Data;
			if(p_qbl) {
				if(ss.getCount() == 3) {
					QueryBarcode qb;
					uint   ssp = 0;
					ss.get(&ssp, temp_buf = 0);
					qb.CodeType = temp_buf.ToLong();
					ss.get(&ssp, temp_buf = 0);
					qb.Rank = temp_buf;
					ss.get(&ssp, temp_buf = 0);
					qb.Number = temp_buf;
					QueryBarcode * p_qb = p_qbl->CreateNewItem(0);
					THROW_SL(p_qb);
					*p_qb = qb;
				}
				else {
					SPathStruc ps;
					ps.Split(rParam.ParamString);
					if(ps.Drv.Empty() && ps.Dir.Empty()) {
						PPGetFilePath(PPPATH_IN, rParam.ParamString, temp_buf);
					}
					else
						temp_buf = rParam.ParamString;
					if(fileExists(temp_buf)) {
						SFile f_in(temp_buf, SFile::mRead);
						if(f_in.IsValid()) {
							while(f_in.ReadLine(temp_buf)) {
								temp_buf.Chomp();
								ss.setBuf(temp_buf);
								if(ss.getCount() == 3) {
									QueryBarcode qb;
									uint   ssp = 0;
									ss.get(&ssp, temp_buf = 0);
									qb.CodeType = temp_buf.ToLong();
									ss.get(&ssp, temp_buf = 0);
									qb.Rank = temp_buf;
									ss.get(&ssp, temp_buf = 0);
									qb.Number = temp_buf;
									QueryBarcode * p_qb = p_qbl->CreateNewItem(0);
									THROW_SL(p_qb);
									*p_qb = qb;
								}
							}
						}
					}
				}
				if(p_qbl->getCount()) {
					Ack    ack;
					if(!PutQuery(qp, rParam.LocID, "QueryBarcode", ack))
						do_report_error = 1;
				}
			}
		}
	}
	else {

	}
	if(do_report_error) {
		PPGetMessage(mfError, PPErrCode, 0, 1, temp_buf);
		rParam.InfoText = temp_buf;
	}
	else if(query_sended) {

	}
	CATCHZOK
	DS.SetMainOrgID(preserve_main_org_id, 0);
	return ok;
}

int SLAPI PPEgaisProcessor::InteractiveQuery()
{
	int    ok = -1;
	QueryParam _param;
	SETFLAGBYSAMPLE(_param.Flags, stTestSendingMode, State);
	_param.LocID = LConfig.Location;
	while(EditQueryParam(&_param) > 0) {
		THROW(ImplementQuery(_param));
		ok = 1;
	}
	CATCHZOKPPERR
	return ok;
}

// static
int SLAPI PPEgaisProcessor::EditInformAReg(InformAReg & rData)
{
    int    ok = -1;
    uint   sel = 0;
    TDialog * dlg = new TDialog(DLG_EGAISREGA);
    THROW(CheckDialogPtr(&dlg, 0));
    dlg->setCtrlLong(CTL_EGAISREGA_QTTY, rData.Qtty);
    dlg->setCtrlDate(CTL_EGAISREGA_MANUFDATE, rData.ManufDate);
    dlg->setCtrlDate(CTL_EGAISREGA_TTNDATE, rData.TTNDate);
    dlg->setCtrlString(CTL_EGAISREGA_TTNCODE, rData.TTNCode);
    dlg->setCtrlDate(CTL_EGAISREGA_EGAISD, rData.EGAISDate);
    dlg->setCtrlString(CTL_EGAISREGA_EGAISN, rData.EGAISCode);
    while(ok < 0 && ExecView(dlg) == cmOK) {
		rData.Qtty = dlg->getCtrlLong(sel = CTL_EGAISREGA_QTTY);
		if(rData.Qtty <= 0 || rData.Qtty > 1000000)
			PPErrorByDialog(dlg, sel, PPERR_USERINPUT);
		else {
			rData.ManufDate = dlg->getCtrlDate(sel = CTL_EGAISREGA_MANUFDATE);
			if(!checkdate(rData.ManufDate, 0))
				PPErrorByDialog(dlg, sel, PPERR_SLIB);
			else {
				rData.TTNDate = dlg->getCtrlDate(sel = CTL_EGAISREGA_TTNDATE);
				if(!checkdate(rData.TTNDate, 0))
					PPErrorByDialog(dlg, sel, PPERR_SLIB);
				else {
					dlg->getCtrlString(sel = CTL_EGAISREGA_TTNCODE, rData.TTNCode);
					rData.EGAISDate = dlg->getCtrlDate(sel = CTL_EGAISREGA_EGAISD);
					if(!checkdate(rData.EGAISDate, 1))
						PPErrorByDialog(dlg, sel, PPERR_SLIB);
					else {
						dlg->getCtrlString(sel = CTL_EGAISREGA_EGAISN, rData.EGAISCode);
						ok = 1;
					}
				}
			}
		}
    }
    CATCHZOKPPERR
    return ok;
}

// static
int SLAPI PPEgaisProcessor::InputMark(PrcssrAlcReport::GoodsItem * pAgi, SString & rMark)
{
	class EgaisMakrDialog : public TDialog {
	public:
		EgaisMakrDialog(const PrcssrAlcReport::GoodsItem * pAgi) : TDialog(DLG_EGAISMARK)
		{
			P_Agi = pAgi;
		}
	private:
		DECL_HANDLE_EVENT
		{
			TDialog::handleEvent(event);
			if(event.isCmd(cmInputUpdated) && event.isCtlEvent(CTL_EGAISMARK_INPUT)) {
				getCtrlString(CTL_EGAISMARK_INPUT, CodeBuf = 0);
				SString msg_buf, mark_buf;
				if(PrcssrAlcReport::IsEgaisMark(CodeBuf, &mark_buf) && PrcssrAlcReport::ParseEgaisMark(mark_buf, Mb) > 0) {
					PPLoadText(PPTXT_EGAISMARKVALID, msg_buf);
					msg_buf.CR().Cat(Mb.EgaisCode);
				}
				else {
					PPLoadError(PPERR_TEXTISNTEGAISMARK, msg_buf, CodeBuf);
				}
				setStaticText(CTL_EGAISMARK_INFO, msg_buf);
			}
		}
		SString CodeBuf;
		const PrcssrAlcReport::GoodsItem * P_Agi;
		PrcssrAlcReport::EgaisMarkBlock Mb;
	};

	rMark = 0;

    int    ok = -1;
	SString temp_buf;
    PrcssrAlcReport::EgaisMarkBlock mb;
    EgaisMakrDialog * dlg = new EgaisMakrDialog(pAgi);
    THROW(CheckDialogPtr(&dlg, 0));
	if(pAgi) {
		SString line_buf;
		GetGoodsName(pAgi->GoodsID, temp_buf);
		line_buf = temp_buf;
		if(pAgi->CategoryCode.NotEmpty())
			line_buf.CatChar('\n').Cat(pAgi->CategoryCode);
		if(pAgi->CategoryName.NotEmpty()) {
			(temp_buf = pAgi->CategoryName).Transf(CTRANSF_OUTER_TO_INNER);
			line_buf.CatChar('\n').Cat(temp_buf);
		}
		dlg->setStaticText(CTL_EGAISMARK_AGI, line_buf);
	}
    while(ok < 0 && ExecView(dlg) == cmOK) {
		dlg->getCtrlString(CTL_EGAISMARK_INPUT, temp_buf);
		if(PrcssrAlcReport::IsEgaisMark(temp_buf, &rMark) && PrcssrAlcReport::ParseEgaisMark(rMark, mb) > 0) {
			ok = 1;
		}
		else {
			PPSetError(PPERR_TEXTISNTEGAISMARK, temp_buf);
			PPErrorByDialog(dlg, CTL_EGAISMARK_INPUT, -1);
			TInputLine * p_il = (TInputLine *)dlg->getCtrlView(CTL_EGAISMARK_INPUT);
			CALLPTRMEMB(p_il, selectAll(1));
			rMark = 0;
		}
    }
    CATCHZOKPPERR
    delete dlg;
    return ok;
}

int SLAPI TestEGAIS(PPEgaisProcessor::TestParam * pParam)
{
	int    ok = 1;
	TDialog * dlg = 0;
	SString temp_buf, file_name;
	PPEgaisProcessor::TestParam param;
	if(!RVALUEPTR(param, pParam)) {
		MEMSZERO(param);
		dlg = new TDialog(DLG_TESTEGAIS);
		THROW(CheckDialogPtr(&dlg, 0));
		param.Flags |= PPEgaisProcessor::TestParam::fReceiveDataToTempCatalog;
		dlg->AddClusterAssoc(CTL_TESTEGAIS_FLAGS, 0, PPEgaisProcessor::TestParam::fReceiveDataToTempCatalog);
		dlg->AddClusterAssoc(CTL_TESTEGAIS_FLAGS, 1, PPEgaisProcessor::TestParam::fAcceptDataFromTempCatalog);
		dlg->SetClusterData(CTL_TESTEGAIS_FLAGS, param.Flags);
		SetupLocationCombo(dlg, CTLSEL_TESTEGAIS_LOC, param.LocID, 0, LOCTYP_WAREHOUSE, 0);
		if(ExecView(dlg) == cmOK) {
			dlg->getCtrlData(CTLSEL_TESTEGAIS_LOC, &param.LocID);
			dlg->GetClusterData(CTL_TESTEGAIS_FLAGS, &param.Flags);
		}
		else
			ok = -1;
		ZDELETE(dlg);
	}
	if(ok > 0) {
		PPEgaisProcessor ep(0, 0);
		THROW(ep);
		PPWait(1);
		{
			TSArray <PPEgaisProcessor::UtmEntry> utm_list;
			THROW(ep.GetUtmList(param.LocID, utm_list));
			for(uint i = 0; i < utm_list.getCount(); i++) {
				ep.SetUtmEntry(param.LocID, &utm_list.at(i), 0);
				if(param.Flags & PPEgaisProcessor::TestParam::fReceiveDataToTempCatalog) {
					THROW(ep.DebugReadInput(param.LocID));
				}
				if(param.Flags & PPEgaisProcessor::TestParam::fAcceptDataFromTempCatalog) {
					THROW(ep.ReadInput(param.LocID, 0, PPEgaisProcessor::rifOffline));
				}
				ep.SetUtmEntry(0, 0, 0);
			}
		}
		if(!(param.Flags & (PPEgaisProcessor::TestParam::fAcceptDataFromTempCatalog|PPEgaisProcessor::TestParam::fReceiveDataToTempCatalog))) {
			ep.CollectRefs();
		}
		PPWait(0);
	}
	CATCHZOKPPERR
	delete dlg;
	return ok;
}

SLAPI EgaisPersonCore::Item::Item()
{
	Clear();
}

void SLAPI EgaisPersonCore::Item::Clear()
{
	ID = 0;
	RarIdent[0] = 0;
	INN[0] = 0;
	KPP[0] = 0;
	UNP[0] = 0;
	RNN[0] = 0;
	CountryCode = 0;
	RegionCode = 0;
	Name = 0;
	FullName = 0;
	AddressDescr = 0;
	Flags = 0;
	ActualDate = ZERODATE;
}

int SLAPI EgaisPersonCore::Item::IsEqual(const EgaisPersonCore::Item & rS, long cmpflags) const
{
    int    yes = 1;
    if(!(cmpflags & cmpfExceptID) && ID != rS.ID)
		yes = 0;
	else if(!sstreq(RarIdent, rS.RarIdent))
		yes = 0;
	else if(!sstreq(INN, rS.INN))
		yes = 0;
	else if(!sstreq(KPP, rS.KPP))
		yes = 0;
	else if(!sstreq(UNP, rS.UNP))
		yes = 0;
	else if(!sstreq(RNN, rS.RNN))
		yes = 0;
	else if(CountryCode != rS.CountryCode)
		yes = 0;
	else if(RegionCode != rS.RegionCode)
		yes = 0;
	else if(Name != rS.Name)
		yes = 0;
	else if(FullName != rS.FullName)
		yes = 0;
	else if(AddressDescr != rS.AddressDescr)
		yes = 0;
	else if(!(cmpflags & cmpfExceptFlags) && Flags != rS.Flags)
		yes = 0;
	else if(ActualDate != rS.ActualDate)
		yes = 0;
	return yes;
}

SLAPI EgaisPersonCore::EgaisPersonCore() : EgaisPersonTbl()
{
}

int SLAPI EgaisPersonCore::RecToItem(const EgaisPersonTbl::Rec & rRec, EgaisPersonCore::Item & rItem)
{
	int    ok = 1;
	SString temp_buf;
	rItem.ID = rRec.ID;
	STRNSCPY(rItem.RarIdent, rRec.RarIdent);
	STRNSCPY(rItem.INN, rRec.INN);
	STRNSCPY(rItem.KPP, rRec.KPP);
	STRNSCPY(rItem.UNP, rRec.UNP);
	rItem.CountryCode = rRec.CountryCode;
	rItem.RegionCode = rRec.RegionCode;
	rItem.Flags = rRec.Flags;
	rItem.ActualDate = rRec.ActualDate;
	{
		Reference * p_ref = PPRef;
		THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPERSON, rRec.ID, PPTRPROP_NAME), rItem.Name));
		rItem.Name.Transf(CTRANSF_UTF8_TO_INNER);
		THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPERSON, rRec.ID, PPTRPROP_LONGNAME), rItem.FullName));
		rItem.FullName.Transf(CTRANSF_UTF8_TO_INNER);
		THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPERSON, rRec.ID, txtprpAddressDescr), rItem.AddressDescr));
		rItem.AddressDescr.Transf(CTRANSF_UTF8_TO_INNER);
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisPersonCore::Search(PPID id, EgaisPersonCore::Item & rItem)
{
	rItem.Clear();
	int    ok = -1;
	EgaisPersonTbl::Rec rec;
	int    r = SearchByID(this, PPOBJ_EGAISPERSON, id, &rec);
	if(r > 0) {
		RecToItem(rec, rItem);
		ok = 1;
	}
	return ok;
}

int SLAPI EgaisPersonCore::IsVerifiedCode(const char * pRarCode)
{
	int    ok = -1;
	TSArray <EgaisPersonTbl::Rec> list;
	int    ap = SearchByCode(pRarCode, list);
	if(ap > 0) {
		for(uint i = 0; ok < 0 && i < list.getCount(); i++)
			if(list.at(i).Flags & rolefVerified)
				ok = 1;
	}
	else if(ap == 0)
		ok = 0;
	return ok;
}

int SLAPI EgaisPersonCore::SearchByCode(const char * pRarCode, TSArray <EgaisPersonTbl::Rec> & rList)
{
	rList.clear();
	int    ok = -1;
	EgaisPersonTbl::Key1 k1;
	MEMSZERO(k1);
	STRNSCPY(k1.RarIdent, pRarCode);
	if(search(1, &k1, spEq)) do {
		THROW_SL(rList.insert(&data));
		ok = 1;
	} while(search(1, &k1, spNext) && sstreq(data.RarIdent, pRarCode));
	if(ok > 0) {
		const uint _c = rList.getCount();
		if(_c == 1)
			ok = 1;
		else {
			uint   actual_pos = 0; // +1
			LDATE  last_dt = ZERODATE;
			for(uint i = 0; i < _c; i++) {
				const EgaisPersonTbl::Rec & r_item = rList.at(i);
				if(r_item.ActualDate > last_dt)
					actual_pos = i+1;
			}
			ok = (int)actual_pos;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisPersonCore::Put(PPID * pID, EgaisPersonCore::Item * pItem, long * pConflictFlags, int use_ta)
{
	int    ok = 1;
	long   conflict_flags = 0;
	SString temp_buf;
	{
		Reference * p_ref = PPRef;
		PPTransaction tra(use_ta);
		THROW(tra);
		if(*pID) {
			Item org_item;
			THROW(Search(*pID, org_item) > 0);
			if(!pItem) {
				THROW_DB(rereadForUpdate(0, 0));
				THROW_DB(deleteRec());
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, PPTRPROP_NAME), (const wchar_t *)0, 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, PPTRPROP_LONGNAME), (const wchar_t *)0, 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, txtprpAddressDescr), (const wchar_t *)0, 0));
			}
			else if(pItem->IsEqual(org_item, 0)) {
				ok = -1;
			}
			else {
				EgaisPersonTbl::Rec rec;
				MEMSZERO(rec);
				rec.ID = *pID;
				STRNSCPY(rec.RarIdent, pItem->RarIdent);
				STRNSCPY(rec.INN, pItem->INN);
				STRNSCPY(rec.KPP, pItem->KPP);
				STRNSCPY(rec.UNP, pItem->UNP);
				STRNSCPY(rec.RNN, pItem->RNN);
				rec.CountryCode = pItem->CountryCode;
				rec.RegionCode = pItem->RegionCode;
				rec.Flags = pItem->Flags;
				rec.ActualDate = pItem->ActualDate;
				THROW_DB(rereadForUpdate(0, 0));
				THROW_DB(updateRecBuf(&rec));
				//
				// ������ ��������� ���� �� ������ �� ������, ���� � �� ���� �� ������ �����
				//
				if(pItem->Name.NotEmptyS()) {
					(temp_buf = pItem->Name).Transf(CTRANSF_INNER_TO_UTF8);
					THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, PPTRPROP_NAME), temp_buf, 0));
				}
				if(pItem->FullName.NotEmptyS()) {
					(temp_buf = pItem->FullName).Transf(CTRANSF_INNER_TO_UTF8);
					THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, PPTRPROP_LONGNAME), temp_buf, 0));
				}
				if(pItem->AddressDescr.NotEmptyS()) {
					(temp_buf = pItem->AddressDescr).Transf(CTRANSF_INNER_TO_UTF8);
					THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, txtprpAddressDescr), temp_buf, 0));
				}
			}
		}
		else {
			EgaisPersonTbl::Rec rec;
			EgaisPersonTbl::Key1 k1;
			MEMSZERO(k1);
			STRNSCPY(k1.RarIdent, pItem->RarIdent);
			if(search(1, &k1, spEq)) {
				ok = -1;
                int   do_update = 0;
                copyBufTo(&rec);
                if(pItem->INN[0]) {
					if(rec.INN[0] == 0) {
						STRNSCPY(rec.INN, pItem->INN);
						do_update = 1;
					}
					else if(!sstreq(rec.INN, pItem->INN)) {
						if(pItem->Flags & EgaisPersonCore::rolefVerified) {
							STRNSCPY(rec.INN, pItem->INN);
							do_update = 1;
						}
						conflict_flags |= 0x0001;
					}
                }
                if(pItem->KPP[0]) {
					if(rec.KPP[0] == 0) {
						STRNSCPY(rec.KPP, pItem->KPP);
						do_update = 1;
					}
					else if(!sstreq(rec.KPP, pItem->KPP)) {
						if(pItem->Flags & EgaisPersonCore::rolefVerified) {
							STRNSCPY(rec.KPP, pItem->KPP);
							do_update = 1;
						}
						conflict_flags |= 0x0002;
					}
                }
                if(pItem->UNP[0]) {
					if(rec.UNP[0] == 0) {
						STRNSCPY(rec.UNP, pItem->UNP);
						do_update = 1;
					}
					else if(!sstreq(rec.UNP, pItem->UNP)) {
						if(pItem->Flags & EgaisPersonCore::rolefVerified) {
							STRNSCPY(rec.UNP, pItem->UNP);
							do_update = 1;
						}
						conflict_flags |= 0x0004;
					}
                }
                if(pItem->RNN[0]) {
					if(rec.RNN[0] == 0) {
						STRNSCPY(rec.RNN, pItem->RNN);
						do_update = 1;
					}
					else if(!sstreq(rec.RNN, pItem->RNN)) {
						if(pItem->Flags & EgaisPersonCore::rolefVerified) {
							STRNSCPY(rec.RNN, pItem->RNN);
							do_update = 1;
						}
						conflict_flags |= 0x0008;
					}
                }
                if(pItem->CountryCode) {
					if(!rec.CountryCode) {
						rec.CountryCode = pItem->CountryCode;
						do_update = 1;
					}
					else if(rec.CountryCode != pItem->CountryCode) {
						if(pItem->Flags & EgaisPersonCore::rolefVerified) {
							rec.CountryCode = pItem->CountryCode;
							do_update = 1;
						}
						conflict_flags |= 0x0010;
					}
                }
                if(pItem->RegionCode) {
					if(!rec.RegionCode) {
						rec.RegionCode = pItem->RegionCode;
						do_update = 1;
					}
					else if(rec.RegionCode != pItem->RegionCode) {
						if(pItem->Flags & EgaisPersonCore::rolefVerified) {
							rec.RegionCode = pItem->RegionCode;
							do_update = 1;
						}
						conflict_flags |= 0x0020;
					}
                }
				if(rec.Flags != pItem->Flags) {
					rec.Flags |= pItem->Flags;
					do_update = 1;
				}
				if(pItem->ActualDate > rec.ActualDate) {
					rec.ActualDate = pItem->ActualDate;
					do_update = 1;
				}
				{
					SString name, full_name, address_descr;
					THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPERSON, rec.ID, PPTRPROP_NAME), name));
					name.Transf(CTRANSF_UTF8_TO_INNER);
					THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPERSON, rec.ID, PPTRPROP_LONGNAME), full_name));
					full_name.Transf(CTRANSF_UTF8_TO_INNER);
					THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPERSON, rec.ID, txtprpAddressDescr), address_descr));
					address_descr.Transf(CTRANSF_UTF8_TO_INNER);
					if(pItem->Name.Len() > name.Len()) {
						THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, rec.ID, PPTRPROP_NAME), (temp_buf = pItem->Name).Transf(CTRANSF_INNER_TO_UTF8), 0));
						ok = 1;
					}
					if(pItem->FullName.Len() > full_name.Len()) {
						THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, rec.ID, PPTRPROP_LONGNAME), (temp_buf = pItem->FullName).Transf(CTRANSF_INNER_TO_UTF8), 0));
						ok = 1;
					}
					if(pItem->AddressDescr.Len() > address_descr.Len()) {
						THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, rec.ID, txtprpAddressDescr), (temp_buf = pItem->AddressDescr).Transf(CTRANSF_INNER_TO_UTF8), 0));
						ok = 1;
					}
				}
				if(do_update) {
					THROW_DB(rereadForUpdate(0, 0));
					THROW_DB(updateRecBuf(&rec));
					ok = 1;
				}
			}
			else {
				MEMSZERO(rec);
				STRNSCPY(rec.RarIdent, pItem->RarIdent);
				STRNSCPY(rec.INN, pItem->INN);
				STRNSCPY(rec.KPP, pItem->KPP);
				STRNSCPY(rec.UNP, pItem->UNP);
				STRNSCPY(rec.RNN, pItem->RNN);
				rec.CountryCode = pItem->CountryCode;
				rec.RegionCode = pItem->RegionCode;
				rec.Flags = pItem->Flags;
				rec.ActualDate = pItem->ActualDate;
				THROW_DB(insertRecBuf(&rec, 0, pID));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, PPTRPROP_NAME), (temp_buf = pItem->Name).Transf(CTRANSF_INNER_TO_UTF8), 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, PPTRPROP_LONGNAME), (temp_buf = pItem->FullName).Transf(CTRANSF_INNER_TO_UTF8), 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, *pID, txtprpAddressDescr), (temp_buf = pItem->AddressDescr).Transf(CTRANSF_INNER_TO_UTF8), 0));
			}
		}
		THROW(tra.Commit());
	}
    CATCHZOK
	ASSIGN_PTR(pConflictFlags, conflict_flags);
    return ok;
}

int SLAPI EgaisPersonCore::Clear(int use_ta)
{
	int    ok = -1;
	Reference * p_ref = PPRef;
	IterCounter cntr;
	SString msg_buf;
	EgaisPersonTbl::Key0 k0;
	MEMSZERO(k0);
	cntr.Init(this);
	PPLoadText(PPTXT_EGAIS_CLEARINNEREGAISDB, msg_buf);
	msg_buf.CatDiv(':', 2).Cat(this->tableName);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(search(0, &k0, spFirst)) do {
			const PPID id = data.ID;
			THROW_DB(rereadForUpdate(0, 0));
			THROW_DB(deleteRec());
			THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, id, PPTRPROP_NAME), (const wchar_t *)0, 0));
			THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, id, PPTRPROP_LONGNAME), (const wchar_t *)0, 0));
			THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPERSON, id, txtprpAddressDescr), (const wchar_t *)0, 0));
			PPWaitPercent(cntr.Increment(), msg_buf);
			ok = 1;
		} while(search(0, &k0, spNext));
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisPersonCore::Export(long fmt, const char * pFileName)
{
	int    ok = 1;
	SFile  f_out(pFileName, SFile::mWrite);
	THROW_SL(f_out.IsValid());
	{
		ulong  rec_no = 0;
		SString line_buf;
		BExtQuery q(this, 1);
		EgaisPersonTbl::Key1 k1;
		MEMSZERO(k1);
		q.selectAll();
		Item item;
		for(q.initIteration(0, &k1, spFirst); q.nextIteration() > 0;) {
			if(RecToItem(data, item)) {
				if(rec_no == 0) {
					(line_buf = 0).Cat("ID").Tab().Cat("RarIdent").Tab().
						Cat("INN").Tab().Cat("KPP").Tab().Cat("UNP").Tab().Cat("RNN").Tab().
						Cat("CountryCode").Tab().Cat("RegionCode").Tab().Cat("Flags").Tab().
						Cat("Name").Tab().Cat("FullName").Tab().Cat("AddrDescr").CR();
					f_out.WriteLine(line_buf);
				}
				line_buf = 0;
				line_buf.Cat(item.ID).Tab().Cat(item.RarIdent).Tab().Cat(item.INN).Tab().Cat(item.KPP).Tab().
					Cat(item.UNP).Tab().Cat(item.RNN).Tab().Cat(item.CountryCode).Tab().Cat(item.RegionCode).Tab().
					CatHex(item.Flags).Tab().Cat(item.Name.Transf(CTRANSF_INNER_TO_OUTER)).Tab().Cat(item.FullName.Transf(CTRANSF_INNER_TO_OUTER)).Tab().Cat(item.AddressDescr.Transf(CTRANSF_INNER_TO_OUTER)).CR();
				f_out.WriteLine(line_buf);
				rec_no++;
			}
		}
	}
    CATCHZOK
	return ok;
}

SLAPI EgaisProductCore::Item::Item()
{
	Clear();
}

void SLAPI EgaisProductCore::Item::Clear()
{
	ID = 0;
	Flags = 0;
	AlcoCode[0] = 0;
	ManufRarIdent[0] = 0;
	ImporterRarIdent[0] = 0;
	CategoryCode[0] = 0;
	Proof = 0.0;
	Volume = 0.0;
	Name = 0;
	FullName = 0;
	ActualDate = ZERODATE;
}

int SLAPI EgaisProductCore::Item::IsEqual(const Item & rS, long cmpflags) const
{
	int    yes = 1;
	if(!(cmpflags & cmpfExceptID) && ID != rS.ID)
		yes = 0;
	else if(Flags != rS.Flags)
		yes = 0;
	else if(Proof != rS.Proof)
		yes = 0;
	else if(Volume != rS.Volume)
		yes = 0;
	else if(!sstreq(AlcoCode, rS.AlcoCode))
		yes = 0;
	else if(!sstreq(ManufRarIdent, rS.ManufRarIdent))
		yes = 0;
	else if(!sstreq(ImporterRarIdent, rS.ImporterRarIdent))
		yes = 0;
	else if(!sstreq(CategoryCode, rS.CategoryCode))
		yes = 0;
	else if(Name != rS.Name)
		yes = 0;
	else if(FullName != rS.FullName)
		yes = 0;
	else if(ActualDate != rS.ActualDate)
		yes = 0;
	return yes;
}

SLAPI EgaisProductCore::EgaisProductCore() : EgaisProductTbl()
{
}

int SLAPI EgaisProductCore::RecToItem(const EgaisProductTbl::Rec & rRec, EgaisProductCore::Item & rItem)
{
	int    ok = 1;
	rItem.ID = rRec.ID;
	rItem.Proof = ((double)rRec.Proof) / 1000.0;
	rItem.Volume = ((double)rRec.Volume) / 100000.0;
	rItem.ActualDate = rRec.ActualDate;
	rItem.Flags = rRec.Flags; // @v9.2.14
	STRNSCPY(rItem.AlcoCode, rRec.AlcCode);
	STRNSCPY(rItem.ManufRarIdent, rRec.ManufRarIdent);
	STRNSCPY(rItem.ImporterRarIdent, rRec.ImporterRarIdent);
	STRNSCPY(rItem.CategoryCode, rRec.CategoryCode);
	{
		Reference * p_ref = PPRef;
		THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPRODUCT, rRec.ID, PPTRPROP_NAME), rItem.Name));
		rItem.Name.Transf(CTRANSF_UTF8_TO_INNER);
		THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPRODUCT, rRec.ID, PPTRPROP_LONGNAME), rItem.FullName));
		rItem.FullName.Transf(CTRANSF_UTF8_TO_INNER);
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisProductCore::Search(PPID id, EgaisProductCore::Item & rItem)
{
	rItem.Clear();
	int    ok = -1;
	EgaisProductTbl::Rec rec;
	int    r = SearchByID(this, PPOBJ_EGAISPRODUCT, id, &rec);
	if(r > 0) {
		RecToItem(rec, rItem);
		ok = 1;
	}
	return ok;
}

int SLAPI EgaisProductCore::IsVerifiedCode(const char * pAlcoCode)
{
	int    ok = -1;
	TSArray <EgaisProductTbl::Rec> list;
	int    ap = SearchByCode(pAlcoCode, list);
	if(ap > 0) {
		for(uint i = 0; ok < 0 && i < list.getCount(); i++) {
			const EgaisProductTbl::Rec & r_rec = list.at(i);
			if(r_rec.Flags & fVerified)
				ok = 1;
		}
	}
	else if(ap == 0)
		ok = 0;
	return ok;
}

int SLAPI EgaisProductCore::SearchByCode(const char * pAlcoCode, TSArray <EgaisProductTbl::Rec> & rList)
{
	rList.clear();
	int    ok = -1;
	EgaisProductTbl::Key1 k1;
	MEMSZERO(k1);
	STRNSCPY(k1.AlcCode, pAlcoCode);
	if(search(1, &k1, spEq)) do {
		THROW_SL(rList.insert(&data));
		ok = 1;
	} while(search(1, &k1, spNext) && sstreq(data.AlcCode, pAlcoCode));
	if(ok > 0) {
		const uint _c = rList.getCount();
		if(_c == 1)
			ok = 1;
		else {
			uint   actual_pos = 0; // +1
			LDATE  last_dt = ZERODATE;
			for(uint i = 0; i < _c; i++) {
				const EgaisProductTbl::Rec & r_item = rList.at(i);
				if(r_item.ActualDate > last_dt)
					actual_pos = i+1;
			}
			ok = (int)actual_pos;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisProductCore::Put(PPID * pID, EgaisProductCore::Item * pItem, long * pConflictFlags, int use_ta)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	long   conflict_flags = 0;
	EgaisProductTbl::Rec rec;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(*pID) {
			Item org_item;
			THROW(Search(*pID, org_item) > 0);
			if(!pItem) {
				THROW_DB(rereadForUpdate(0, 0));
				THROW_DB(deleteRec());
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, *pID, PPTRPROP_NAME), (const wchar_t *)0, 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, *pID, PPTRPROP_LONGNAME), (const wchar_t *)0, 0));
			}
			else if(pItem->IsEqual(org_item, 0)) {
				ok = -1;
			}
			else {
				MEMSZERO(rec);
				rec.ID = *pID;
				rec.Proof = (long)(pItem->Proof * 1000.0);
				rec.Volume = (long)(pItem->Volume * 100000.0);
				STRNSCPY(rec.AlcCode, pItem->AlcoCode);
				STRNSCPY(rec.ManufRarIdent, pItem->ManufRarIdent);
				STRNSCPY(rec.ImporterRarIdent, pItem->ImporterRarIdent);
				STRNSCPY(rec.CategoryCode, pItem->CategoryCode);
				rec.ActualDate = pItem->ActualDate;
				rec.Flags = pItem->Flags; // @v9.2.14
				THROW_DB(rereadForUpdate(0, 0));
				THROW_DB(updateRecBuf(&rec));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, *pID, PPTRPROP_NAME), (temp_buf = pItem->Name).Transf(CTRANSF_INNER_TO_UTF8), 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, *pID, PPTRPROP_LONGNAME), (temp_buf = pItem->FullName).Transf(CTRANSF_INNER_TO_UTF8), 0));
			}
		}
		else {
			const int32 new_proof  = (long)(pItem->Proof * 1000.0);
			const int32 new_volume = (long)(pItem->Volume * 100000.0);

			EgaisProductTbl::Key1 k1;
			MEMSZERO(k1);
			STRNSCPY(k1.AlcCode, pItem->AlcoCode);
			if(search(1, &k1, spEq)) {
				ok = -1;
				int    do_update = 0;
				copyBufTo(&rec);
				if(pItem->AlcoCode[0]) {
					if(rec.AlcCode[0] == 0) {
						STRNSCPY(rec.AlcCode, pItem->AlcoCode);
						do_update = 1;
					}
					else if(!sstreq(rec.AlcCode, pItem->AlcoCode)) {
						if(pItem->Flags & EgaisProductCore::fVerified) {
							STRNSCPY(rec.AlcCode, pItem->AlcoCode);
							do_update = 1;
						}
						conflict_flags |= 0x0001;
					}
				}
				if(pItem->CategoryCode[0]) {
					if(rec.CategoryCode[0] == 0) {
						STRNSCPY(rec.CategoryCode, pItem->CategoryCode);
						do_update = 1;
					}
					else if(!sstreq(rec.CategoryCode, pItem->CategoryCode)) {
						if(pItem->Flags & EgaisProductCore::fVerified) {
							STRNSCPY(rec.CategoryCode, pItem->CategoryCode);
							do_update = 1;
						}
						conflict_flags |= 0x0002;
					}
				}
				if(pItem->ManufRarIdent[0]) {
					if(rec.ManufRarIdent[0] == 0) {
						STRNSCPY(rec.ManufRarIdent, pItem->ManufRarIdent);
						do_update = 1;
					}
					else if(!sstreq(rec.ManufRarIdent, pItem->ManufRarIdent)) {
						if(pItem->Flags & EgaisProductCore::fVerified) {
							STRNSCPY(rec.ManufRarIdent, pItem->ManufRarIdent);
							do_update = 1;
						}
						conflict_flags |= 0x0004;
					}
				}
				if(pItem->ImporterRarIdent[0]) {
					if(rec.ImporterRarIdent[0] == 0) {
						STRNSCPY(rec.ImporterRarIdent, pItem->ImporterRarIdent);
						do_update = 1;
					}
					else if(!sstreq(rec.ImporterRarIdent, pItem->ImporterRarIdent)) {
						if(pItem->Flags & EgaisProductCore::fVerified) {
							STRNSCPY(rec.ImporterRarIdent, pItem->ImporterRarIdent);
							do_update = 1;
						}
						conflict_flags |= 0x0008;
					}
				}
				if(new_proof > 0) {
					if(rec.Proof <= 0) {
						rec.Proof = new_proof;
						do_update = 1;
					}
					else if(new_proof != rec.Proof) {
						if(pItem->Flags & EgaisProductCore::fVerified) {
							rec.Proof = new_proof;
							do_update = 1;
						}
						conflict_flags |= 0x0010;
					}
				}
				if(new_volume > 0) {
					if(rec.Volume <= 0) {
						rec.Volume = new_volume;
						do_update = 1;
					}
					else if(new_volume != rec.Volume) {
						if(pItem->Flags & EgaisProductCore::fVerified) {
							rec.Volume = new_volume;
							do_update = 1;
						}
						conflict_flags |= 0x0020;
					}
				}
				{
					SString name, full_name;
					THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPRODUCT, rec.ID, PPTRPROP_NAME), name));
					name.Transf(CTRANSF_UTF8_TO_INNER);
					THROW(p_ref->UtrC.GetText(TextRefIdent(PPOBJ_EGAISPRODUCT, rec.ID, PPTRPROP_LONGNAME), full_name));
					full_name.Transf(CTRANSF_UTF8_TO_INNER);
					if(pItem->Name.Len() > name.Len()) {
						THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, rec.ID, PPTRPROP_NAME), (temp_buf = pItem->Name).Transf(CTRANSF_INNER_TO_UTF8), 0));
						ok = 1;
					}
					if(pItem->FullName.Len() > full_name.Len()) {
						THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, rec.ID, PPTRPROP_LONGNAME), (temp_buf = pItem->FullName).Transf(CTRANSF_INNER_TO_UTF8), 0));
						ok = 1;
					}
				}
				if(pItem->Flags & EgaisProductCore::fVerified && !(rec.Flags & EgaisProductCore::fVerified)) {
					rec.Flags |= EgaisProductCore::fVerified;
					do_update = 1;
				}
				if(do_update) {
					THROW_DB(rereadForUpdate(0, 0));
					THROW_DB(updateRecBuf(&rec));
					ok = 1;
				}
			}
			else {
				MEMSZERO(rec);
				rec.Proof = new_proof;
				rec.Volume = new_volume;
				STRNSCPY(rec.AlcCode, pItem->AlcoCode);
				STRNSCPY(rec.ManufRarIdent, pItem->ManufRarIdent);
				STRNSCPY(rec.ImporterRarIdent, pItem->ImporterRarIdent);
				STRNSCPY(rec.CategoryCode, pItem->CategoryCode);
				rec.ActualDate = pItem->ActualDate;
				rec.Flags = pItem->Flags; // @v9.2.14
				THROW_DB(insertRecBuf(&rec, 0, pID));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, *pID, PPTRPROP_NAME), (temp_buf = pItem->Name).Transf(CTRANSF_INNER_TO_UTF8), 0));
				THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, *pID, PPTRPROP_LONGNAME), (temp_buf = pItem->FullName).Transf(CTRANSF_INNER_TO_UTF8), 0));
			}
		}
		THROW(tra.Commit());
	}
    CATCHZOK
    ASSIGN_PTR(pConflictFlags, conflict_flags);
    return ok;
}

int SLAPI EgaisProductCore::Clear(int use_ta)
{
	int    ok = -1;
	Reference * p_ref = PPRef;
	IterCounter cntr;
	SString msg_buf;
	EgaisProductTbl::Key0 k0;
	MEMSZERO(k0);
	cntr.Init(this);
	PPLoadText(PPTXT_EGAIS_CLEARINNEREGAISDB, msg_buf);
	msg_buf.CatDiv(':', 2).Cat(this->tableName);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(search(0, &k0, spFirst)) do {
			const PPID id = data.ID;
			THROW_DB(rereadForUpdate(0, 0));
			THROW_DB(deleteRec());
			THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, id, PPTRPROP_NAME), (const wchar_t *)0, 0));
			THROW(p_ref->UtrC.SetText(TextRefIdent(PPOBJ_EGAISPRODUCT, id, PPTRPROP_LONGNAME), (const wchar_t *)0, 0));
			PPWaitPercent(cntr.Increment(), msg_buf);
			ok = 1;
		} while(search(0, &k0, spNext));
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisProductCore::Export(long fmt, const char * pFileName)
{
	int    ok = 1;
	SFile  f_out(pFileName, SFile::mWrite);
	THROW_SL(f_out.IsValid());
	{
		ulong  rec_no = 0;
		SString line_buf;
		BExtQuery q(this, 1);
		EgaisProductTbl::Key1 k1;
		MEMSZERO(k1);
		q.selectAll();
		Item item;
		for(q.initIteration(0, &k1, spFirst); q.nextIteration() > 0;) {
			if(RecToItem(data, item)) {
				if(rec_no == 0) {
					(line_buf = 0).Cat("ID").Tab().Cat("AlcoCode").Tab().
						Cat("ManufRarIdent").Tab().Cat("ImporterRarIdent").Tab().Cat("CategoryCode").Tab().Cat("Proof").Tab().
						Cat("Volume").Tab().Cat("Name").Tab().Cat("FullName").CR();
					f_out.WriteLine(line_buf);
				}
				line_buf = 0;
				line_buf.Cat(item.ID).Tab().Cat(item.AlcoCode).Tab().Cat(item.ManufRarIdent).Tab().Cat(item.ImporterRarIdent).Tab().
					Cat(item.CategoryCode).Tab().Cat(item.Proof, MKSFMTD(0, 2, 0)).Tab().Cat(item.Volume, MKSFMTD(0, 3, 0)).Tab().
					Cat(item.Name.Transf(CTRANSF_INNER_TO_OUTER)).Tab().Cat(item.FullName.Transf(CTRANSF_INNER_TO_OUTER)).CR();
				f_out.WriteLine(line_buf);
				rec_no++;
			}
		}
	}
    CATCHZOK
	return ok;
}

//static
int FASTCALL EgaisRefACore::IsRecEq(const EgaisRefATbl::Rec & rR1, const EgaisRefATbl::Rec & rR2)
{
#define CMP_MEMB(m)  if(rR1.m != rR2.m) return 0;
#define CMP_MEMBS(m) if(!sstreq(rR1.m, rR2.m)) return 0;
	CMP_MEMB(ID);
	CMP_MEMBS(RefACode);
	CMP_MEMBS(AlcCode);
	CMP_MEMBS(ManufRarIdent);
	CMP_MEMBS(ImporterRarIdent);
	CMP_MEMB(CountryCode);
	CMP_MEMB(Volume);
	CMP_MEMB(BottlingDate);
	CMP_MEMB(ActualDate);
#undef CMP_MEMBS
#undef CMP_MEMB
	return 1;
}

SLAPI EgaisRefACore::EgaisRefACore() : EgaisRefATbl()
{
}

int SLAPI EgaisRefACore::Search(PPID id, EgaisRefATbl::Rec * pRec)
{
	return SearchByID(this, PPOBJ_EGAISREFA, id, pRec);
}

int SLAPI EgaisRefACore::SearchByCode(const char * pRefACode, TSArray <EgaisRefATbl::Rec> & rList)
{
	rList.clear();
	int    ok = -1;
	EgaisRefATbl::Key1 k1;
	MEMSZERO(k1);
	STRNSCPY(k1.RefACode, pRefACode);
	if(search(1, &k1, spEq)) do {
		THROW_SL(rList.insert(&data));
		ok = 1;
	} while(search(1, &k1, spNext) && sstreq(data.RefACode, pRefACode));
	if(ok > 0) {
		const uint _c = rList.getCount();
		if(_c == 1)
			ok = 1;
		else {
			uint   actual_pos = 0; // +1
			LDATE  last_dt = ZERODATE;
			for(uint i = 0; i < _c; i++) {
				const EgaisRefATbl::Rec & r_item = rList.at(i);
				if(r_item.ActualDate > last_dt)
					actual_pos = i+1;
			}
			ok = (int)actual_pos;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisRefACore::SearchByProductCode(const char * pAlcoCode, TSArray <EgaisRefATbl::Rec> & rList)
{
	rList.clear();
	int    ok = -1;
	EgaisRefATbl::Key2 k2;
	MEMSZERO(k2);
	STRNSCPY(k2.AlcCode, pAlcoCode);
	if(search(2, &k2, spEq)) do {
		THROW_SL(rList.insert(&data));
		ok = 1;
	} while(search(2, &k2, spNext) && sstreq(data.AlcCode, pAlcoCode));
	CATCHZOK
	return ok;
}

int SLAPI EgaisRefACore::Put(PPID * pID, EgaisRefATbl::Rec * pRec, long * pConflictFlags, int use_ta)
{
	int    ok = 1;
	long   conflict_flags = 0;
	EgaisRefATbl::Rec rec;
    if(*pID) {
		EgaisRefATbl::Rec org_rec;
		THROW(Search(*pID, &org_rec) > 0);
		if(!pRec) {
			THROW_DB(rereadForUpdate(0, 0));
			THROW_DB(deleteRec());
		}
		else if(IsRecEq(*pRec, org_rec)) {
			ok = -1;
		}
		else {
			MEMSZERO(rec);
			rec.ID = *pID;
			STRNSCPY(rec.RefACode, pRec->RefACode);
			STRNSCPY(rec.AlcCode, pRec->AlcCode);
			STRNSCPY(rec.ManufRarIdent, pRec->ManufRarIdent);
			STRNSCPY(rec.ImporterRarIdent, pRec->ImporterRarIdent);
			rec.CountryCode = pRec->CountryCode;
			rec.Volume = pRec->Volume;
			rec.BottlingDate = pRec->BottlingDate;
			rec.Flags = pRec->Flags;
			THROW_DB(rereadForUpdate(0, 0));
			THROW_DB(updateRecBuf(&rec));
		}
    }
    else {
		EgaisRefATbl::Key1 k1;
		MEMSZERO(k1);
		STRNSCPY(k1.RefACode, pRec->RefACode);
		if(search(1, &k1, spEq)) {
			ok = -1;
			int    do_update = 0;
			copyBufTo(&rec);
			if(pRec->AlcCode[0]) {
				if(rec.AlcCode[0] == 0) {
					STRNSCPY(rec.AlcCode, pRec->AlcCode);
					do_update = 1;
				}
				else if(!sstreq(rec.AlcCode, pRec->AlcCode)) {
					if(pRec->Flags & EgaisRefACore::fVerified) {
						STRNSCPY(rec.AlcCode, pRec->AlcCode);
						do_update = 1;
					}
					conflict_flags |= 0x0001;
				}
			}
			if(pRec->ManufRarIdent[0]) {
				if(rec.ManufRarIdent[0] == 0) {
					STRNSCPY(rec.ManufRarIdent, pRec->ManufRarIdent);
					do_update = 1;
				}
				else if(!sstreq(rec.ManufRarIdent, pRec->ManufRarIdent)) {
					if(pRec->Flags & EgaisRefACore::fVerified) {
						STRNSCPY(rec.ManufRarIdent, pRec->ManufRarIdent);
						do_update = 1;
					}
					conflict_flags |= 0x0002;
				}
			}
			if(pRec->ImporterRarIdent[0]) {
				if(rec.ImporterRarIdent[0] == 0) {
					STRNSCPY(rec.ImporterRarIdent, pRec->ImporterRarIdent);
					do_update = 1;
				}
				else if(!sstreq(rec.ImporterRarIdent, pRec->ImporterRarIdent)) {
					if(pRec->Flags & EgaisRefACore::fVerified) {
						STRNSCPY(rec.ImporterRarIdent, pRec->ImporterRarIdent);
						do_update = 1;
					}
					conflict_flags |= 0x0004;
				}
			}
			if(pRec->Volume > 0) {
                if(rec.Volume <= 0) {
					rec.Volume = pRec->Volume;
					do_update = 1;
                }
                else if(rec.Volume != pRec->Volume) {
					if(pRec->Flags & EgaisRefACore::fVerified) {
						rec.Volume = pRec->Volume;
						do_update = 1;
					}
					conflict_flags |= 0x0008;
				}
			}
			if(checkdate(pRec->BottlingDate, 0)) {
				if(!checkdate(rec.BottlingDate, 0)) {
					rec.BottlingDate = pRec->BottlingDate;
					do_update = 1;
				}
				else if(pRec->BottlingDate != rec.BottlingDate) {
					if(pRec->Flags & EgaisRefACore::fVerified) {
						rec.BottlingDate = pRec->BottlingDate;
						do_update = 1;
					}
					conflict_flags |= 0x0010;
				}
			}
			if(pRec->CountryCode > 0) {
				if(rec.CountryCode <= 0) {
					rec.CountryCode = pRec->CountryCode;
					do_update = 1;
				}
				else if(pRec->CountryCode != rec.CountryCode) {
					if(pRec->Flags & EgaisRefACore::fVerified) {
						rec.CountryCode = pRec->CountryCode;
						do_update = 1;
					}
					conflict_flags |= 0x0020;
				}
			}
			if(pRec->Flags & EgaisRefACore::fVerified && !(rec.Flags & EgaisRefACore::fVerified)) {
				rec.Flags |= EgaisRefACore::fVerified;
				do_update = 1;
			}
			if(do_update) {
				THROW_DB(rereadForUpdate(0, 0));
				THROW_DB(updateRecBuf(&rec));
			}
		}
		else {
			MEMSZERO(rec);
			STRNSCPY(rec.RefACode, pRec->RefACode);
			STRNSCPY(rec.AlcCode, pRec->AlcCode);
			STRNSCPY(rec.ManufRarIdent, pRec->ManufRarIdent);
			STRNSCPY(rec.ImporterRarIdent, pRec->ImporterRarIdent);
			rec.CountryCode = pRec->CountryCode;
			rec.Volume = pRec->Volume;
			rec.BottlingDate = pRec->BottlingDate;
			rec.Flags = pRec->Flags;
			THROW_DB(insertRecBuf(&rec, 0, pID));
		}
    }
    CATCHZOK
    ASSIGN_PTR(pConflictFlags, conflict_flags);
	return ok;
}

int SLAPI EgaisRefACore::Clear(int use_ta)
{
	int    ok = -1;
	IterCounter cntr;
	SString msg_buf;
	EgaisRefATbl::Key0 k0;
	MEMSZERO(k0);
	cntr.Init(this);
	PPLoadText(PPTXT_EGAIS_CLEARINNEREGAISDB, msg_buf);
	msg_buf.CatDiv(':', 2).Cat(this->tableName);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(search(0, &k0, spFirst)) do {
			const PPID id = data.ID;
			THROW_DB(rereadForUpdate(0, 0));
			THROW_DB(deleteRec());
			PPWaitPercent(cntr.Increment(), msg_buf);
			ok = 1;
		} while(search(0, &k0, spNext));
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI EgaisRefACore::Export(long fmt, const char * pFileName)
{
	int    ok = 1;
	SFile  f_out(pFileName, SFile::mWrite);
	THROW_SL(f_out.IsValid());
	{
		ulong  rec_no = 0;
		SString line_buf;
		BExtQuery q(this, 1);
		EgaisRefATbl::Key1 k1;
		MEMSZERO(k1);
		q.selectAll();
		for(q.initIteration(0, &k1, spFirst); q.nextIteration() > 0;) {
			EgaisRefATbl::Rec item;
			copyBufTo(&item);
			if(rec_no == 0) {
				(line_buf = 0).Cat("ID").Tab().Cat("RefACode").Tab().
					Cat("AlcCode").Tab().Cat("ManufRarIdent").Tab().Cat("ImporterRarIdent").Tab().Cat("CountryCode").Tab().
					Cat("Volume").Tab().Cat("BottlingDate").CR();
				f_out.WriteLine(line_buf);
			}
			line_buf = 0;
			line_buf.Cat(item.ID).Tab().Cat(item.RefACode).Tab().Cat(item.AlcCode).Tab().Cat(item.ManufRarIdent).Tab().
				Cat(item.ImporterRarIdent).Tab().Cat(item.CountryCode).Tab().Cat(((double)item.Volume)/100000.0, MKSFMTD(0, 3, 0)).Tab().
				Cat(item.BottlingDate, DATF_DMY|DATF_CENTURY).CR();
			f_out.WriteLine(line_buf);
			rec_no++;
		}
	}
    CATCHZOK
	return ok;
}
//
//
//
SLAPI PrcssrAlcReport::RefCollection::RefCollection()
{
	LastPersonP = -1;
	LastProductP = -1;
	LastRefAP = -1;
}

int FASTCALL PrcssrAlcReport::RefCollection::SetPerson(EgaisPersonCore::Item & rItem)
{
	int    ok = 1;
	int    result_pos = -1;
	for(uint pos = 0; result_pos < 0 && PersonList.lsearch(rItem.RarIdent, &pos, PTR_CMPFUNC(Pchar), offsetof(EgaisPersonCore::Item, RarIdent));) {
		EgaisPersonCore::Item * p_item = PersonList.at(pos);
		if(p_item && p_item->IsEqual(rItem, EgaisPersonCore::Item::cmpfExceptFlags)) {
			p_item->Flags |= rItem.Flags;
			result_pos = (int)pos;
		}
		pos++;
	}
	if(result_pos < 0) {
		EgaisPersonCore::Item * p_new_item = new EgaisPersonCore::Item(rItem);
		THROW_MEM(p_new_item);
		THROW_SL(PersonList.insert(p_new_item));
		result_pos = PersonList.getCount()-1;
	}
	LastPersonP = result_pos;
	CATCH
		ok = 0;
		LastPersonP = -1;
	ENDCATCH
	return ok;
}

int FASTCALL PrcssrAlcReport::RefCollection::SetProduct(EgaisProductCore::Item & rItem)
{
	int    ok = 1;
	int    result_pos = -1;
	for(uint pos = 0; result_pos < 0 && ProductList.lsearch(rItem.AlcoCode, &pos, PTR_CMPFUNC(Pchar), offsetof(EgaisProductCore::Item, AlcoCode));) {
		const EgaisProductCore::Item * p_item = ProductList.at(pos);
		if(p_item && p_item->IsEqual(rItem, 0)) {
			result_pos = (int)pos;
		}
		pos++;
	}
	if(result_pos < 0) {
		EgaisProductCore::Item * p_new_item = new EgaisProductCore::Item(rItem);
		THROW_MEM(p_new_item);
		THROW_SL(ProductList.insert(p_new_item));
		result_pos = ProductList.getCount()-1;
	}
	LastProductP = result_pos;
	CATCH
		ok = 0;
		LastProductP = -1;
	ENDCATCH
	return ok;
}

int FASTCALL PrcssrAlcReport::RefCollection::SetRefA(EgaisRefATbl::Rec & rItem)
{
	int    ok = 1;
	int    result_pos = -1;
	for(uint pos = 0; result_pos < 0 && RefAList.lsearch(rItem.RefACode, &pos, PTR_CMPFUNC(Pchar), offsetof(EgaisRefATbl::Rec, RefACode));) {
		const EgaisRefATbl::Rec & r_item = RefAList.at(pos);
		if(EgaisRefACore::IsRecEq(r_item, rItem)) {
			result_pos = (int)pos;
		}
		pos++;
	}
	if(result_pos < 0) {
		THROW_SL(RefAList.insert(&rItem));
		result_pos = RefAList.getCount()-1;
	}
	LastRefAP = result_pos;
	CATCH
		ok = 0;
		LastRefAP = -1;
	ENDCATCH
	return ok;
}

int SLAPI PrcssrAlcReport::RefCollection::Store(int use_ta)
{
	int    ok = 1;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		{
			for(uint i = 0; i < PersonList.getCount(); i++) {
				EgaisPersonCore::Item * p_item = PersonList.at(i);
				if(p_item) {
					long conflict_flags = 0;
					THROW(PsC.Put(&p_item->ID, p_item, &conflict_flags, 0));
				}
			}
		}
		{
			for(uint i = 0; i < ProductList.getCount(); i++) {
				EgaisProductCore::Item * p_item = ProductList.at(i);
				if(p_item) {
					long conflict_flags = 0;
					THROW(PrC.Put(&p_item->ID, p_item, &conflict_flags, 0));
				}
			}
		}
		{
			for(uint i = 0; i < RefAList.getCount(); i++) {
				EgaisRefATbl::Rec & r_item = RefAList.at(i);
				long   conflict_flags = 0;
				THROW(RaC.Put(&r_item.ID, &r_item, &conflict_flags, 0));
			}
		}
		THROW(tra.Commit());
	}
	{
        SString dump_filename;
        PPGetFilePath(PPPATH_OUT, PPFILNAM_EGAIS_PERSON_TXT, dump_filename);
        THROW(PsC.Export(0, dump_filename));
        PPGetFilePath(PPPATH_OUT, PPFILNAM_EGAIS_PRODUCT_TXT, dump_filename);
        THROW(PrC.Export(0, dump_filename));
        PPGetFilePath(PPPATH_OUT, PPFILNAM_EGAIS_REFA_TXT, dump_filename);
        THROW(RaC.Export(0, dump_filename));
	}
    CATCHZOK
	return ok;
}