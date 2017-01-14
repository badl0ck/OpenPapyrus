// PPMSG.CPP
// Copyright (c) A.Sobolev 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2015, 2016
//
#include <pp.h>
#pragma hdrstop
#include <strstore.h>

static StringStore2 * _PPStrStore = 0; // @global @threadsafe

int SLAPI PPInitStrings(const char * pFileName)
{
	ENTER_CRITICAL_SECTION
	if(_PPStrStore == 0) {
		SString name;
		SString temp_buf;
		StrAssocArray file_lang_list;
		if(pFileName == 0)
			PPGetFilePath(PPPATH_BIN, "ppstr.bin", name);
		else
			name = pFileName;
        {
            SPathStruc ps;
            ps.Split(name);
			if(!ps.Nam.HasChr('-')) {
				const SString org_nam = ps.Nam;
				const SString org_ext = ps.Ext;
				(ps.Nam = org_nam).CatChar('-').CatChar('*');
				ps.Merge(temp_buf);
				{
					SString lang_symb;
					SDirEntry de;
					SPathStruc ps_lang;
					for(SDirec direc(temp_buf, 0); direc.Next(&de) > 0;) {
						if(!de.IsFolder()) {
							temp_buf = de.FileName;
							uint   hyphen_pos = 0;
							if(temp_buf.StrChr('-', &hyphen_pos)) {
								temp_buf.Sub(hyphen_pos+1, temp_buf.Len(), lang_symb = 0);
								int slang = RecognizeLinguaSymb(lang_symb, 0);
								if(slang > 0) {
									ps_lang.Split(de.FileName);
									ps.Nam = ps_lang.Nam;
									ps.Ext = ps_lang.Ext;
									ps.Merge(temp_buf);
									file_lang_list.Add(slang, temp_buf);
								}
							}
						}
					}
				}
			}
			file_lang_list.Add(0, name); // ������� ���� ������ ���� ��������� � ������ ������
        }
		_PPStrStore = new StringStore2();
		{
	#ifdef NDEBUG
			const int self_test = 0;
	#else
			const int self_test = 1;
	#endif
			int    _done = 0;
			for(uint i = 0; i < file_lang_list.getCount(); i++) {
				name = file_lang_list.at_WithoutParent(i).Txt;
				if(_PPStrStore->Load(name, self_test)) {
					_done = 1;
				}
				else {
					char err_msg[1024];
					sprintf(err_msg, "Unable load string resource '%s'", name.cptr());
					::MessageBox(0, err_msg, _T("Error"), MB_OK|MB_ICONERROR); // @unicodeproblem
					ZDELETE(_PPStrStore);
				}
			}
			if(self_test && _PPStrStore) {
				assert(_PPStrStore->GetString(PPSTR_TEXT, PPTXT_TESTSTRING, temp_buf));
				assert(temp_buf == "abc @def ghi");
				_PPStrStore->GetDescription(PPSTR_TEXT, PPTXT_TESTSTRING, temp_buf);
				assert(temp_buf == "description for teststring");
			}
			/* @v9.1.2 ���������� � PPSession::Init
			if(_done) {
				SLS.SetLoadStringFunc(PPLoadStringFunc);
				SLS.SetExpandStringFunc(PPExpandStringFunc); // @v9.0.11
			}
			*/
		}
	}
	return BIN(_PPStrStore);
	LEAVE_CRITICAL_SECTION
}

int SLAPI PPReleaseStrings()
{
	if(_PPStrStore)
		DO_CRITICAL(ZDELETE(_PPStrStore));
	return 1;
}

int SLAPI PPLoadString(int group, int code, SString & s)
{
	//
	// ��� ������� @threadsafe ��������� StrStore2::GetString �������� const-��������
	//
	int    ok = 1;
	s = 0;
	if(group && code) {
		PROFILE_START
		ok = _PPStrStore ? _PPStrStore->GetString(group, code, s) : 0;
		PROFILE_END
		s.Transf(CTRANSF_UTF8_TO_INNER);
		if(!ok)
			PPSetErrorSLib();
	}
	return ok;
}

SString & SLAPI PPLoadStringS(int group, int code, SString & s)
{
	PPLoadString(group, code, s);
	return s;
}

int FASTCALL PPExpandString(SString & rS, int ctransf)
{
	return _PPStrStore ? _PPStrStore->ExpandString(rS, ctransf) : 0;
}

int FASTCALL PPLoadString(const char * pSignature, SString & rBuf)
{
	//
	// ��� ������� @threadsafe ��������� StrStore2::GetString �������� const-��������
	//
	int    ok = 1;
	rBuf = 0;
	if(isempty(pSignature))
		ok = -1;
	else {
		PROFILE_START
		ok = _PPStrStore ? _PPStrStore->GetString(pSignature, rBuf) : 0;
		PROFILE_END
		rBuf.Transf(CTRANSF_UTF8_TO_INNER);
		if(!ok)
			PPSetErrorSLib();
	}
	return ok;
}

int SLAPI PPLoadString(int group, int code, char * buf, size_t bufLen)
{
	SString temp_buf;
	int    ok = PPLoadString(group, code, temp_buf);
	temp_buf.CopyTo(buf, bufLen);
	return ok;
}

int FASTCALL PPLoadText(int code, SString & s)
{
	return PPLoadString(PPSTR_TEXT, code, s);
}

SString & FASTCALL PPLoadTextS(int code, SString & s)
{
	s = 0;
	PPLoadString(PPSTR_TEXT, code, s);
	return s;
}

int FASTCALL PPLoadTextWin(int code, SString & s)
{
	int    ok = PPLoadString(PPSTR_TEXT, code, s);
	s.Transf(CTRANSF_INNER_TO_OUTER);
	return ok;
}

int FASTCALL PPLoadError(int code, SString & s, const char * pAddInfo)
{
	return PPGetMessage(mfError|mfOK, code, pAddInfo, DS.CheckExtFlag(ECF_SYSSERVICE), s);
}

int FASTCALL PPSetError(int errCode)
{
	PPThreadLocalArea & tla = DS.GetTLA();
	if(&tla && tla.IsConsistent()) {
		tla.LastErr = errCode;
		tla.AddedMsgString = 0;
	}
	return 0; // @v8.7.0 1-->0
}

int FASTCALL PPSetLibXmlError(const xmlParserCtxt * pCtx)
{
	PPThreadLocalArea & tla = DS.GetTLA();
	if(&tla && tla.IsConsistent()) {
		tla.LastErr = PPERR_LIBXML;
		tla.AddedMsgString = 0;
		if(pCtx) {
			if(pCtx->lastError.code)
				tla.AddedMsgString.CatDiv(' ', 0, 1).Cat(pCtx->lastError.code);
			if(!isempty(pCtx->lastError.message))
				tla.AddedMsgString.CatDiv(' ', 0, 1).Cat(pCtx->lastError.message);
			if(!isempty(pCtx->lastError.file))
				tla.AddedMsgString.CatDiv(' ', 0, 1).Cat(pCtx->lastError.file);
		}
		if(!tla.AddedMsgString.NotEmptyS())
			tla.AddedMsgString = "unknown";
	}
	return 0; // @v8.7.0 1-->0
}

int PPSetErrorNoMem() { return PPSetError(PPERR_NOMEM); } // @v8.5.10 (,0)
int PPSetErrorSLib() { return PPSetError(PPERR_SLIB); }
int PPSetErrorDB() { return PPSetError(PPERR_DBENGINE); }

int FASTCALL PPSetError(int errCode, const char * pAddedMsg)
{
	PPThreadLocalArea & tla = DS.GetTLA();
	if(&tla && tla.IsConsistent()) {
		tla.LastErr = errCode;
		(tla.AddedMsgString = 0).CatN(pAddedMsg, 256);
	}
	return 0; // @v8.7.0 1-->0
}

int FASTCALL PPSetError(int errCode, long val)
{
	PPThreadLocalArea & tla = DS.GetTLA();
	if(&tla && tla.IsConsistent()) {
		tla.LastErr = errCode;
		(tla.AddedMsgString = 0).Cat(val);
	}
	return 0; // @v8.7.0 1-->0
}

int SLAPI PPSetObjError(int errCode, PPID objType, PPID objID)
{
	PPThreadLocalArea & tla = DS.GetTLA();
	if(&tla && tla.IsConsistent()) {
		tla.LastErrObj.Obj = objType;
		tla.LastErrObj.Id  = objID;
		tla.LastErr = errCode;
	}
	return 0; // @v8.7.0 1-->0
}

void FASTCALL PPSetAddedMsgString(const char * pStr)
{
	DS.GetTLA().AddedMsgString = pStr;
}

void SLAPI PPSetAddedMsgObjName(PPID objType, PPID objID)
{
	SString obj_name;
	GetObjectName(objType, objID, obj_name);
	PPSetAddedMsgString(obj_name);
}

int SLAPI PPGetMessage(uint options, int msgcode, const char * pAddInfo, int rmvSpcChrs, SString & rBuf)
{
	char * p_tmp_buf  = 0;
	char * p_tmp_buf2 = 0;
	SString temp_buf;
	char   btr_err_code[16], fname[MAXPATH];
	int    group = 0;
	int    addcode = 0;
	int    is_win_msg = 0; // 1 as win32 msg, 2 as socket msg
	rBuf = 0;
	switch(options & 0x00ff) {
		case mfError:
			group = PPMSG_ERROR;
			switch(msgcode) {
				case PPERR_DBENGINE:
					{
						const int _btr_err_code = BtrError;
						if(_btr_err_code) {
							if(_btr_err_code == BE_SLIB) {
								; // @nobreak : ���������� ���������� ����� {case PPERR_SLIB}
							}
							else {
								group   = addcode = msgcode;
								msgcode = _btr_err_code;
								if(msgcode == BE_ORA_TEXT)
									pAddInfo = DBS.GetAddedMsgString();
								else if(!pAddInfo && DBTable::GetLastErrorFileName())
									pAddInfo = DBTable::GetLastErrorFileName();
								break;
							}
						}
						else
							break;
					}
				case PPERR_SLIB:
					{
						const int _sl_err_code = SLibError; // ��������� � SLibError �������� �������
						if(_sl_err_code) {
							if(_sl_err_code == SLERR_WINDOWS)
								is_win_msg = 1;
							else if(_sl_err_code == SLERR_SOCK_WINSOCK) {
								is_win_msg = 2;
								group   = addcode = msgcode;
								msgcode = _sl_err_code;
							}
							else if(_sl_err_code == SLERR_CURL) {
								const int ce = SLS.GetConstTLA().LastCurlErr;
								group = addcode = PPSTR_CURLERR;
								msgcode = ce;
							}
							else {
								group   = addcode = msgcode;
								msgcode = _sl_err_code;
							}
						}
					}
					break;
				case PPERR_DBLIB:
					{
						const int _db_err_code = DBErrCode;
						if(_db_err_code) {
							group = addcode = msgcode;
							if(_db_err_code == SDBERR_SLIB) {
								group = PPERR_SLIB;
								msgcode = SLibError;
							}
							else if(_db_err_code == SDBERR_BTRIEVE) {
								group = PPERR_DBENGINE;
								msgcode = BtrError;
							}
							else
								msgcode = _db_err_code;
						}
					}
					break;
				case PPERR_REFSEXISTS:
				case PPERR_REFISBUSY:
				case PPERR_OBJNFOUND:
				case PPERR_DUPSYMB:
				case PPERR_LOTRESTBOUND:
					//pAddInfo = InitObjAddInfo(fname, sizeof(fname));
					//static char * InitObjAddInfo(char * pBuf, size_t bufSize)
					{
						PPObjID last_err_obj = DS.GetConstTLA().LastErrObj;
						GetObjectTitle(last_err_obj.Obj, temp_buf = 0).CatCharN(' ', 2);
						ideqvalstr(last_err_obj.Id, temp_buf).Space().CatChar('(');
						GetObjectName(last_err_obj.Obj, last_err_obj.Id, temp_buf, 1);
						temp_buf.CatChar(')');
						pAddInfo = STRNSCPY(fname, temp_buf);
					}
					break;
				case PPERR_DBQUERY:
					break;
				case PPERR_CRYSTAL_REPORT:
					group = addcode = PPERR_CRYSTAL_REPORT;
					msgcode = CrwError;
					break;
				case PPERR_NORIGHTS:
					if(DS.GetTLA().AddedMsgStrNoRights.Empty()) {
						GetCurUserName(temp_buf = 0);
						STRNSCPY(fname, temp_buf);
					}
					else
						STRNSCPY(fname, DS.GetTLA().AddedMsgStrNoRights);
					pAddInfo = fname;
					break;
			}
			break;
		case mfWarn: group = PPMSG_WARNING;      break;
		case mfInfo: group = PPMSG_INFORMATION;  break;
		case mfConf: group = PPMSG_CONFIRMATION; break;
		case mfCritWarn: group = PPMSG_CONFIRMATION; break;
		default:
			return 0;
	}
	if(!(p_tmp_buf = new char[PP_MSGLEN+1])) {
		pAddInfo = "Not enough memory";
		msgcode = 0;
	}
	else
		*p_tmp_buf = 3; // ��������� ����� �������������� //
	if(!pAddInfo) {
		if(oneof2(group, PPERR_DBENGINE, PPERR_DBLIB))
			pAddInfo = DBS.GetConstTLA().AddedMsgString;
		else if(group == PPERR_SLIB)
			pAddInfo = SLS.GetConstTLA().AddedMsgString;
		else
			pAddInfo = DS.GetConstTLA().AddedMsgString;
	}
	if(is_win_msg) {
		int c = (is_win_msg == 2) ? SLS.GetConstTLA().LastSockErr : SLS.GetOsError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, c,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)p_tmp_buf, PP_MSGLEN, NULL);
		chomp(SCharToOem(p_tmp_buf));
		/* @todo
		if(pAddInfo) {
			p_tmp_buf
		}
		*/
	}
	else if(msgcode) {
__loadstring:
		if(!PPLoadString(group, msgcode, p_tmp_buf+1, PP_MSGLEN)) {
			if(SLibError == SLERR_NOFOUND && addcode) {
				if(addcode == PPERR_DBENGINE) {
					group   = PPMSG_ERROR;
					msgcode = PPERR_DBENGINE;
					addcode = 0;
					pAddInfo = itoa(BtrError, btr_err_code, 10);
				}
				else {
					group   = PPMSG_ERROR;
					msgcode = addcode;
					addcode = 0;
				}
				goto __loadstring;
			}
			else {
				// @v9.0.4 sprintf(p_tmp_buf+1, "���������� ����㧨�� ᮮ�饭��: (%d)%d", group, msgcode);
				// @v9.0.4 {
				PPLoadString(PPMSG_ERROR, PPERR_TEXTLOADINGFAULT, temp_buf);
				temp_buf.CatDiv(':', 2).CatParStr(group).Space().Cat(msgcode);
				temp_buf.CopyTo(p_tmp_buf+1, PP_MSGLEN-1);
				// } @v9.0.4
				pAddInfo = p_tmp_buf;
				msgcode = 0;
			}
		}
		else {
			const size_t tmp_buf_size = 1024;
			if(pAddInfo && (p_tmp_buf2 = new char[tmp_buf_size]) != 0) {
				_snprintf(p_tmp_buf2, tmp_buf_size-1, p_tmp_buf, pAddInfo); // @v7.9.9
				// @v7.9.9 sprintf(p_tmp_buf2, p_tmp_buf, pAddInfo);
				delete p_tmp_buf;
				p_tmp_buf = p_tmp_buf2;
			}
		}
	}
	rBuf = msgcode ? p_tmp_buf : pAddInfo;
	if(rmvSpcChrs)
		rBuf.ReplaceChar('\003', ' ').ReplaceChar('\n', ' ');
	delete p_tmp_buf;
	return 1;
}

int FASTCALL PPError(int errcode, const char * pAddInfo)
{
	int    r = 0;
	//
	// ��� ��� ������� PPMessage ����� �������� �������� ������, ��������� ���
	// ��� ������ � ������ ��������� ������������ ������
	//
	PPSaveErrContext();
	int    ok = PPMessage(mfError|mfOK, ((errcode >= 0) ? errcode : PPErrCode), pAddInfo);
	if(ok > 0) {
		if(!CS_SERVER) {
			PPRestoreErrContext();
			r = 1;
			PPLogMessage(PPFILNAM_ERRMSG_LOG, 0, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_LASTERR|LOGMSGF_DBINFO);
		}
	}
	if(!r)
		PPRestoreErrContext();
	return ok;
}

int SLAPI PPError()
{
	return PPError(-1, 0);
}

int FASTCALL PPErrorTooltip(int errcode, const char * pAddInfo)
{
	int    ok = 0, r = 0;
	//
	// ��� ��� ������� PPMessage ����� �������� �������� ������, ��������� ���
	// ��� ������ � ������ ��������� ������������ ������
	//
	PPSaveErrContext();
	ok = PPTooltipMessage(mfError|mfOK, ((errcode >= 0) ? errcode : PPErrCode), pAddInfo);
	if(ok > 0) {
		if(!CS_SERVER) {
			PPRestoreErrContext();
			r = 1;
			PPLogMessage(PPFILNAM_ERRMSG_LOG, 0, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_LASTERR|LOGMSGF_DBINFO);
		}
	}
	if(!r)
		PPRestoreErrContext();
	return ok;
}

int SLAPI PPErrorZ()
{
	return (PPError(-1, 0), 0);
}

int SLAPI PPDbSearchError()
{
	return (BTROKORNFOUND) /**/ ? -1 : PPSetErrorDB();
}

static int SLAPI PPCriticalWarning(SString & rMsg, uint /*options*/)
{
	if(!CS_SERVER) {
		int    yes = cmCancel;
		char   answ[32];
		rMsg.ReplaceChar('\003', ' ');
		TDialog * dlg = new TDialog(DLG_CRITWARN);
		if(CheckDialogPtr(&dlg, 1)){
			dlg->setStaticText(CTL_CRITWARN_HEAD, rMsg);
			if(ExecView(dlg) == cmOK) {
				dlg->getCtrlData(CTL_CRITWARN_ANSWER, answ);
				yes = (stricmp866(answ, "��") !=0) ? cmCancel : cmYes;
			}
		}
		else
			yes = 0;
		delete dlg;
		return yes;
	}
	else
		return cmYes;
}

int FASTCALL PPMessage(uint options, int msgcode, const char * pAddInfo)
{
	int    ok = 0;
	SString buf;
	if(PPGetMessage(options, msgcode, pAddInfo, DS.CheckExtFlag(ECF_SYSSERVICE), buf)) {
		PPWait(0);
		ok = ((options & mfCritWarn) == mfCritWarn) ? PPCriticalWarning(buf, options) : PPOutputMessage(buf, options);
	}
	return ok;
}

int SLAPI PPOutputMessage(const char * pMsg, uint options)
{
	if(!CS_SERVER) {
		if(SLS.CheckUiFlag(sluifUseLargeDialogs))
			options |= mfLargeBox;
		return messageBox(pMsg, options);
	}
	else {
		if((options & mfConf) != mfConf) // @v7.7.9 !(options & mfConf) --> ((options & mfConf) != mfConf)
			PPLogMessage((options & mfError) ? PPFILNAM_ERR_LOG : PPFILNAM_INFO_LOG, pMsg, LOGMSGF_TIME);
		return 1;
	}
}

int SLAPI PPTooltipMessage(const char * pMsg, const char * pImgPath, HWND parent, long timer, COLORREF color, long flags)
{
	int    ok = 0;
	if(!CS_SERVER) {
		if(pMsg || pImgPath) {
			SMessageWindow * p_win = new SMessageWindow;
			if(p_win) {
				SString buf;
				buf = pMsg;
				buf.ReplaceChar('\003', ' ').Strip();
				ok = p_win->Open(buf, pImgPath, parent, 0, timer, color, flags, 0);
			}
		}
	}
	return ok;
}

int SLAPI PPTooltipMessage(uint options, int msgcode, const char * pAddInfo)
{
	int    ok = 0;
	if(!CS_SERVER) {
		SString buf;
		if(PPGetMessage(options, msgcode, pAddInfo, DS.CheckExtFlag(ECF_SYSSERVICE), buf)) {
			SMessageWindow * p_win = new SMessageWindow;
			if(p_win) {
				buf.ReplaceChar('\003', ' ').Strip();
				COLORREF color = GetColorRef(SClrSteelblue);
				long   flags = SMessageWindow::fSizeByText|SMessageWindow::fOpaque|SMessageWindow::fPreserveFocus;
				if(options & mfError) {
					color = GetColorRef(SClrRed);
					flags |= SMessageWindow::fShowOnCenter;
				}
				ok = p_win->Open(buf, 0, 0, 0, 30000, color, flags, 0);
			}
		}
	}
	return ok;
}
//
//
// Prototype
void SLAPI AlignWaitDlg(HWND hw = 0);

SLAPI PPThreadLocalArea::WaitBlock::WaitBlock() : IdleTimer(500)
{
	State = stValid;
	PrevView = 0;
	WaitDlg = 0;
	WaitCur = LoadCursor(TProgram::GetInst(), MAKEINTRESOURCE(IDC_PPYWAIT));
	OrgCur = 0;
	hwndPB = 0;
	PrevPercent = -1;
}

SLAPI PPThreadLocalArea::WaitBlock::~WaitBlock()
{
	Stop();
	DestroyCursor(WaitCur);
}

int PPThreadLocalArea::WaitBlock::IsValid() const
{
	return BIN(State & stValid);
}

HWND PPThreadLocalArea::WaitBlock::GetWindowHandle() const
{
	return WaitDlg;
}

int SLAPI PPThreadLocalArea::WaitBlock::Start()
{
	int    ok = 1;
	State |= stValid;
	if(!WaitDlg) {
		PrevView = 0;
		WaitDlg = APPL->CreateDlg(DLG_WAIT, APPL->H_MainWnd, (DLGPROC)0L, 0);
		PrevPercent = -1;
		IdleTimer.Restart(1000);
		if(WaitDlg) {
			TView * p_cur = APPL->P_DeskTop->P_Current;
			PrevView = (p_cur && p_cur->IsConsistent()) ? p_cur : 0;
			AlignWaitDlg(WaitDlg);
			ShowWindow(WaitDlg, SW_SHOWNA);
			hwndPB = GetDlgItem(WaitDlg, 101);
			hwndST = GetDlgItem(WaitDlg, CTL_WAIT_TEXT);
			UpdateWindow(hwndST);
			if(WaitCur)
				OrgCur = SetCursor(WaitCur);
		}
		else {
			State &= stValid;
			ok = 0;
		}
	}
	else
		ok = -1;
	return ok;
}

int SLAPI PPThreadLocalArea::WaitBlock::Stop()
{
	int    ok = 1;
	if(WaitDlg) {
	   	uint32 save;
		if(PrevView && PrevView->IsConsistent()) {
			save = PrevView->options;
			PrevView->options |= ofSelectable;
		}
		DestroyWindow(WaitDlg);
		WaitDlg = 0;
		SetActiveWindow(APPL->H_TopOfStack);
		if(OrgCur)
			SetCursor(OrgCur);
		if(PrevView && PrevView->IsConsistent())
			PrevView->options = save;
	}
	else
		ok = -1;
	return ok;
}

int SLAPI PPThreadLocalArea::WaitBlock::Hide()
{
	int    ok = 0;
	if(WaitDlg) {
		if(!(State & stHide)) {
			ShowWindow(WaitDlg, SW_HIDE);
			State |= stHide;
			ok = 1;
		}
		else
			ok = -1;
	}
	return ok;
}

int SLAPI PPThreadLocalArea::WaitBlock::Show()
{
	int    ok = 0;
	if(WaitDlg) {
		if(State & stHide) {
			ShowWindow(WaitDlg, SW_SHOWNA);
			State &= ~stHide;
			ok = 1;
		}
		else
			ok = -1;
	}
	return ok;
}

int FASTCALL PPThreadLocalArea::WaitBlock::SetMessage(const char * pMsg)
{
	PROFILE_START
	if(IdleTimer.Check(0) && APPL && APPL->H_MainWnd)
		SendMessage(APPL->H_MainWnd, WM_ENTERIDLE, 0, 0);
	DS.SetThreadNotification(PPSession::stntMessage, pMsg);
	//
	PPAdviseList adv_list;
	if(DS.GetAdviseList(PPAdviseBlock::evWaitMsg, 0, adv_list) > 0) {
		PPNotifyEvent ev;
		PPAdviseBlock adv_blk;
		for(uint j = 0; adv_list.Enum(&j, &adv_blk);) {
			if(adv_blk.Proc) {
				ev.Clear();
				ev.ExtStr = pMsg;
				adv_blk.Proc(PPAdviseBlock::evWaitMsg, &ev, adv_blk.ProcExtPtr);
			}
		}
	}
	PROFILE_END
	if(hwndST) {
		if(pMsg) {
			if(Text.Cmp(pMsg, 0) != 0) {
				(Text = pMsg).Transf(CTRANSF_INNER_TO_OUTER);
				// @v9.1.5 ::SetWindowText(hwndST, (const char *)Text); // @unicodeproblem
				TView::SSetWindowText(hwndST, Text); // @v9.1.5
			}
		}
		else if(Text.NotEmpty()) {
			// @v9.1.5 ::SetWindowText(hwndST, (const char *)(Text = 0)); // @unicodeproblem
			TView::SSetWindowText(hwndST, Text = 0); // @v9.1.5
		}
		TProgram::IdlePaint();
	}
	return 1;
}

int FASTCALL PPThreadLocalArea::WaitBlock::SetPercent(ulong p, ulong t, const char * msg)
{
	int    result = 1;
	ulong  percent = (ulong)(t ? (100.0 * ((double)p / (double)t)) : 100.0);
	if(percent != PrevPercent || (msg && msg[0] && PrevMsg.Cmp(msg, 0) != 0)) {
		PrevPercent = percent;
		PrevMsg = msg;
		if(hwndPB) {
			ShowWindow(hwndPB, SW_SHOWNA);
			SendMessage(hwndPB, PBM_SETPOS, percent, 0);
		}
		char b[1024], * s;
		if(msg) {
			s = stpcpy(b, msg);
			*s++ = ' ';
		}
		else
			s = b;
		ultoa(percent, s, 10);
		s = b + strlen(b);
		*s++ = '%';
		*s = 0;
		result = SetMessage(b);
	}
	else
		result = SetMessage(Text);
	return result;
}

#define __WD DS.GetTLA().WD

void SLAPI AlignWaitDlg(HWND hw)
{
	SETIFZ(hw, __WD.GetWindowHandle());
	if(hw) {
		RECT   r1, r2;
		GetWindowRect(hw, &r1);
		GetWindowRect(APPL->H_MainWnd, &r2);
		r1.bottom -= r1.top;
		r1.right -= r1.left;
		r1.top = r2.top+80;
		r1.left = r2.left+40;
		::MoveWindow(hw, r1.left, r1.top, r1.right, r1.bottom, 1);
	}
}

int FASTCALL PPWait(int begin)
{
	int    ok = 1;
	if(begin != 1)
		DS.SetThreadNotification(PPSession::stntMessage, 0);
	if(!CS_SERVER) {
		if(begin == 2)
			__WD.Hide();
		else if(begin == 3)
			__WD.Show();
		else if(begin == 1) {
			__WD.Start();
		}
		else if(begin == 0) {
			__WD.Stop();
		}
	}
	return ok;
}

int FASTCALL PPWaitMsg(const char * pMsg)
{
	return __WD.SetMessage(pMsg);
}

int FASTCALL PPWaitPercent(ulong p, ulong t, const char * pMsg)
{
	return __WD.SetPercent(p, t, pMsg);
}

int FASTCALL PPWaitPercent(const IterCounter & cntr, const char * pMsg)
	{ return PPWaitPercent(cntr, cntr.GetTotal(), pMsg); }
int FASTCALL PPWaitPercent(ulong v, const char * pMsg)
	{ return PPWaitPercent(v, 100UL, pMsg); }

int FASTCALL PPWaitMsg(int msgGrpID, int msgID, const char * addInfo)
{
	char   buf[128], msg[80], * p;
	if(PPLoadString(msgGrpID, msgID, msg, sizeof(msg))) {
		if(addInfo)
			sprintf(p = buf, msg, addInfo);
		else
			p = msg;
		return PPWaitMsg(p);
	}
	return 0;
}

int FASTCALL PPWaitLong(long v)
{
	char b[32];
	return PPWaitMsg(ltoa(v, b, 10));
}

int FASTCALL PPWaitDate(LDATE dt)
{
	char   b[32];
	return PPWaitMsg(datefmt(&dt, DATF_DMY, b));
}

static int FASTCALL CheckEscKey(int cmd)
{
	MSG    msg;
	return PeekMessage(&msg, 0, WM_KEYDOWN, WM_KEYDOWN, cmd ? PM_NOREMOVE : PM_REMOVE) ? ((msg.wParam == VK_ESCAPE) ? 1 : 0) : 0;
}

int SLAPI PPCheckUserBreak()
{
	int    ok = 1;
	PROFILE_START
	if(SLS.CheckStopFlag()) {
		PPSetError(PPERR_PROCESSWASSTOPPED);
		ok = 0;
	}
	else if(DS.IsThreadStopped()) {
		PPSetError(PPERR_THREADWASSTOPPED);
		ok = 0;
	}
	else if(!CS_SERVER) {
		if(__WD.GetWindowHandle() && CheckEscKey(1)) {
			CheckEscKey(0);
			PPWait(0);
			if(PPMessage(mfConf|mfYesNo, PPCFM_USERBREAK, 0) == cmYes)
                ok = (PPErrCode = PPERR_USERBREAK, 0);
			else
				ok = (PPWait(1), -1);
		}
	}
	PROFILE_END
	return ok;
}