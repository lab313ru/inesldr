
#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// KOEI CROSSPLATFORM GAME SCRIPTS
// ---------------------------------

//#define MANUAL

#ifdef MANUAL
//	#define AUTODETECT
//	#define MMC 1
 	#define MMC 5
	#define LAST_BANK	0x1F
//	#define LAST_BANK	0x3F
#endif

static make_data_array_cust(ea, len, cmt) {
	MakeUnknown(ea, len, DOUNK_SIMPLE);
	if(len==2) {
		MakeWord(ea);
	} else {
		MakeData(ea, FF_BYTE, len, 0);
		SetArrayFormat(ea, AP_IDXHEX, 16 /*len*/, 0);
	}
	cmt = substr(CommentEx(ea, 0), 0, 3) + cmt;
	MakeComm(ea, "");
	MakeComm(ea, cmt);
	return ea + len;
}

static make_offset_custom(ea, exec, mmc, last_bank, defb1, defb2) {
	auto ofs = Word(ea), bank = GetReg(ea,"ds")-1;
	if(mmc == 1) {
		if(ofs<0x8000)
			ea = make_offset_ex(ea, -1, exec, 1);
		else if(ofs>=0xC000)
			ea = make_offset_ex(ea, last_bank, exec, 1);
		else
			ea = make_offset_ex(ea, bank, exec, 1);
	} else if (mmc == 5) {
		if(ofs<0x8000)
			ea = make_offset_ex(ea, -1, exec, 1);
		else if(ofs>=0xE000)
			ea = make_offset_ex(ea, last_bank, exec, 1);
		else if((ofs>=0xC000)&&(ofs<0xE000))
			ea = make_offset_ex(ea, last_bank-1, exec, 1);
		else if((defb2!=-1)&&((ofs>=0xA000)&&(ofs<0xC000))&&(exec==1))
			ea = make_offset_ex(ea, defb2, exec, 1);
		else if((defb1!=-1)&&((ofs>=0x8000)&&(ofs<0xA000))&&(exec==1))
			ea = make_offset_ex(ea, defb1, exec, 1);
		else
			ea = make_offset_ex(ea, bank, exec, 1);
	}
	return ea;
}

static get_offset_custom(ea, exec, mmc, last_bank, defb1, defb2) {
	auto ofs = Word(ea), bank = GetReg(ea,"ds")-1;
	if(mmc == 1) {
		if(ofs<0x8000)
			ea = MK_FP(AskSelector(0), 0) + ofs;
		else if(ofs>=0xC000)
			ea = MK_FP(AskSelector(last_bank + 1), 0) + ofs;
		else
			ea = MK_FP(AskSelector(bank + 1), 0) + ofs;
	} else if (mmc==5) {
		if(ofs<0x8000)
			ea = MK_FP(AskSelector(0), 0) + ofs;
		else if(ofs>=0xE000)
			ea = MK_FP(AskSelector(last_bank + 1), 0) + ofs;
		else if((ofs>=0xC000)&&(ofs<0xE000))
			ea = MK_FP(AskSelector(last_bank - 1 + 1), 0) + ofs;
		else if((defb2!=-1)&&((ofs>=0xA000)&&(ofs<0xC000))&&(exec==1))
			ea = MK_FP(AskSelector(defb2 + 1), 0) + ofs;
		else if((defb1!=-1)&&((ofs>=0x8000)&&(ofs<0xA000))&&(exec==1))
			ea = MK_FP(AskSelector(defb1 + 1), 0) + ofs;
		else
			ea = MK_FP(AskSelector(bank + 1), 0) + ofs;
	}
	return ea;
}

static get_name_offset_custom(ea,exec,mmc, last_bank, defb1, defb2) {
	auto ofs_ea = get_offset_custom(ea,exec, mmc, last_bank, defb1, defb2);
	auto ofs_name = NameEx(BADADDR, ofs_ea);
	if(ofs_name == "") {
		return form("0x%08x", ofs_ea);
	} else {
		return ofs_name;
	}
}

static set_name_offset_custom(ea,ofs_name,exec,mmc, last_bank, defb1, defb2) {
	auto ofs_ea = get_offset_custom(ea,exec, mmc, last_bank, defb1, defb2);
	ofs_name = form("_%s_%X",ofs_name,ofs_ea);
	MakeNameEx(ofs_ea,ofs_name,SN_CHECK|SN_NOWARN);
}

static native_calls_search(opcode,mmc, last_bank, defb1, defb2) {
	auto ea = -1, tmp0, tmp1;
	auto bank = GetReg(ea,"ds")-1;
	Message(" Native call search start (%s)...\n", opcode);
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN|SEARCH_CASE, opcode);
		if(ea != BADADDR) {
			auto base, offs;
			Message(" >found one at 0x%08x\n", ea);
			tmp0 = get_name_offset_custom(ea + 3,1,mmc,last_bank,defb1,defb2);
			tmp0 = substr(tmp0, 0, 3);
			if((tmp0 == "loc")||(tmp0 == "sub"))
				set_name_offset_custom(ea + 3,"procedure",1,mmc,last_bank,defb1,defb2);
			make_offset_custom(ea + 3, 1, mmc, last_bank, defb1, defb2);	// exec
			tmp0 = Byte(ea + 5);
			tmp1 = tmp0&0x80?"-":"";
			if(tmp0&0x80)
				tmp0 = 0x100-tmp0;
			make_data_array_cust(ea + 5, 1, "");
			MakeCode(ea);
			AutoMark(ea, AU_CODE);
			MakeCode(ea + 6);
			AutoMark(ea + 6, AU_CODE);
			Wait();
		}
	} while (ea != BADADDR);
	Message(" Done!\n");
}

static koei_opcode(cmd) {
	auto ret = "unk";
	if((cmd >= 0x00)&&(cmd <= 0x0B))	  ret = form("MOV       A, [HP-var%d]",(0xC-cmd&0xF));
	else if((cmd >= 0x0C)&&(cmd <= 0x0F)) ret = form("MOV       A, [HP+arg%d]",(cmd&0xF)-0xC);
	else if((cmd >= 0x10)&&(cmd <= 0x1B)) ret = form("MOV       B, [HP-var%d]",(0xC-cmd&0xF));
	else if((cmd >= 0x1C)&&(cmd <= 0x1F)) ret = form("MOV       B, [HP+arg%d]",(cmd&0xF)-0xC);
	else if((cmd >= 0x20)&&(cmd <= 0x2B)) ret = form("MOV       [HP-var%d], A",(0xC-cmd&0xF));
	else if((cmd >= 0x2C)&&(cmd <= 0x2F)) ret = form("MOV       [HP+arg%d],A",(cmd&0xF)-0xC);
	else if((cmd >= 0x30)&&(cmd <= 0x3B)) ret = form("PUSH      [HP-var%d]",(0xC-cmd&0xF));
	else if((cmd >= 0x3C)&&(cmd <= 0x3F)) ret = form("PUSH      [HP+arg%d]",(cmd&0xF)-0xC);
	else if((cmd >= 0x40)&&(cmd <= 0x4F)) ret = form("MOVI      A, %d",cmd&0xF);
	else if((cmd >= 0x50)&&(cmd <= 0x5F)) ret = form("MOVI      B, %d",cmd&0xF);
	else if((cmd >= 0x60)&&(cmd <= 0x6F)) ret = form("PUSHI     %d",cmd&0xF);
	else if((cmd >= 0x70)&&(cmd <= 0x7F)) ret = form("ADDI      A, %d",cmd&0xF);
	else if(cmd == 0x81) ret = "MOV       A, [HP%s%d]";
	else if(cmd == 0x82) ret = "MOV       A, [HP%s%d]";
	else if(cmd == 0x83) ret = "MOV       B, [HP%s%d]";
	else if(cmd == 0x84) ret = "MOV       B, [HP%s%d]";
	else if(cmd == 0x85) ret = "MOV       [HP%s%d], A";
	else if(cmd == 0x86) ret = "MOV       [HP%s%d], A";
	else if(cmd == 0x87) ret = "PUSH      [HP%s%d]";
	else if(cmd == 0x88) ret = "PUSH      [HP%s%d]";
	else if(cmd == 0x89) ret = "MOVS.B    A, $%02X";
	else if(cmd == 0x8A) ret = "MOV       A, %s";
	else if(cmd == 0x8B) ret = "MOVS.B    B, $%02X";
	else if(cmd == 0x8C) ret = "MOV       B, %s";
	else if(cmd == 0x8D) ret = "PUSHS.B   $%02X";
	else if(cmd == 0x8E) ret = "PUSH      %s";
	else if(cmd == 0x8F) ret = "ADDS.B    A, $%02X";
	else if(cmd == 0x90) ret = "ADD       A, %s%d";
	else if(cmd == 0xA0) ret = "MOVU.B    A, [HP%s%d]";
	else if(cmd == 0xA1) ret = "MOVU.B    B, [HP%s%d]";
	else if(cmd == 0xA2) ret = "MOVU.B    [HP%s%d], A";
	else if(cmd == 0xA3) ret = "PUSHS.B   [HP%s%d]";
	else if(cmd == 0xA4) ret = "MOV       A, [%s]";
	else if(cmd == 0xA5) ret = "MOVU.B    A, [%s]";
	else if(cmd == 0xA6) ret = "MOV       B, [%s]";
	else if(cmd == 0xA7) ret = "MOVU.B    B, [%s]";
	else if(cmd == 0xA8) ret = "MOV       [%s], A";
	else if(cmd == 0xA9) ret = "MOVU.B    [%s], A";
	else if(cmd == 0xAA) ret = "PUSH      [%s]";
	else if(cmd == 0xAB) ret = "PUSHU.B   [%s]";
	else if(cmd == 0xAC) ret = "EJSR      %s";
	else if(cmd == 0xAD) ret = "COPY      [B], [A], $%04X";
	else if(cmd == 0xAE) ret = "ADDU.B    SP, %02X";
	else if(cmd == 0xAF) ret = "ADD       SP, %s%d";
	else if(cmd == 0xB0) ret = "MOV       A, [A]";
	else if(cmd == 0xB1) ret = "MOV       [POP B], A";
	else if(cmd == 0xB2) ret = "NOP";
	else if(cmd == 0xB3) ret = "PUSH      A";
	else if(cmd == 0xB4) ret = "POP       B";
	else if(cmd == 0xB5) ret = "MUL       A, B";
	else if(cmd == 0xB6) ret = "DIVS      A, B";
	else if(cmd == 0xB8) ret = "DIVU      A, B";
	else if(cmd == 0xB9) ret = "MODS      A, B";	// AWJ
	else if(cmd == 0xBA) ret = "MODU      A, B";	// AWJ
	else if(cmd == 0xBB) ret = "ADD       A, B";
	else if(cmd == 0xBC) ret = "SUB       A, B";
	else if(cmd == 0xBD) ret = "ASL       A, B";
	else if(cmd == 0xBE) ret = "LSR       A, B";
	else if(cmd == 0xBF) ret = "ASR       A, B";
	else if(cmd == 0xC0) ret = "CMPS.EQ   A, B";	//  A == B AWJ
	else if(cmd == 0xC1) ret = "CMPS.NE   A, B";	//  A != B AWJ
	else if(cmd == 0xC2) ret = "CMPS.LT   A, B";	// sA <  B AWJ
	else if(cmd == 0xC3) ret = "CMPS.LE   A, B";	// sA <= B AWJ
	else if(cmd == 0xC4) ret = "CMPS.GT   A, B";	// sA >  B AWJ
	else if(cmd == 0xC5) ret = "CMPS.GE   A, B";	// sA >= B AWJ
	else if(cmd == 0xC6) ret = "CMPU.LT   A, B";	// uA <  B AWJ
	else if(cmd == 0xC7) ret = "CMPU.LE   A, B";	// uA <= B AWJ
	else if(cmd == 0xC8) ret = "CMPU.GT   A, B";	// uA >  B AWJ
	else if(cmd == 0xC9) ret = "CMPU.GE   A, B";	// uA >= B AWJ
	else if(cmd == 0xCA) ret = "NOT       A";		// if A>0, A=0, else A=1 fix AWJ
	else if(cmd == 0xCB) ret = "NEG       A";		// A = -A
	else if(cmd == 0xCC) ret = "COMP      A";		// A = ~A
	else if(cmd == 0xCD) ret = "SWAP      A, B";
	else if(cmd == 0xCF) ret = "RET";
	else if(cmd == 0xD0) ret = "INC       A";
	else if(cmd == 0xD1) ret = "DEC       A";
	else if(cmd == 0xD2) ret = "ASL       A";
	else if(cmd == 0xD3) ret = "MOVU.B    A, [A]";
	else if(cmd == 0xD4) ret = "MOVU.B    [POP B], A";
	else if(cmd == 0xD5) ret = "SWITCH    %s%d, %d";
	else if(cmd == 0xD6) ret = "JMP       %s";
	else if(cmd == 0xD7) ret = "BRKT      A, %s";
	else if(cmd == 0xD8) ret = "BRKF      A, %s";
	else if(cmd == 0xD9) ret = "SWITCH.C  %d";
	else if(cmd == 0xDA) ret = "AND       A, B";
	else if(cmd == 0xDB) ret = "OR        A, B";
	else if(cmd == 0xDC) ret = "XOR       A, B";
	else if(cmd == 0xDD) ret = "EJSR      A";
	else if(cmd == 0xDE) ret = "MOV       A, HP%s%d";
	else if(cmd == 0xDF) ret = "MOV       B, HP%s%d";
	else if(cmd == 0xE0) ret = "MOVBF.S   A, [A], %d, %d";	// sA = bit[A][%d-%d] AWJ
	else if(cmd == 0xE1) ret = "MOVBF.U   A, [A], %d, %d";	// uA = bit[A][%d-%d] AWJ
	else if(cmd == 0xE2) ret = "MOVBF     [B], A, %d, %d";	// [B] = bitA[%d-%d]  AWJ
	else if(cmd == 0xE3) ret = "JMP.B     %s [PC-%d]";
	else if(cmd == 0xE4) ret = "BRKT.B    %s [PC-%d]";
	else if(cmd == 0xE5) ret = "BRKF.B    %s [PC-%d]";
	else if(cmd == 0xE6) ret = "JMP.B     %s [PC+%d]";
	else if(cmd == 0xE7) ret = "BRKT.B    %s [PC+%d]";
	else if(cmd == 0xE8) ret = "BRKF.B    %s [PC+%d]";
	else if(cmd == 0xE9) ret = "EJSR      %s, SP+%d";
	else if(cmd == 0xEA) ret = "EJSR      A, SP+%d";
	return ret;
}

static koei_disasm(ea, stopea, mmc, last_bank, defb1, defb2) {
	auto cmd_opc = "", cmd, tmp0, tmp1, tmp2, i, do_stop = 0;
	auto bank = GetReg(ea,"ds"), BIOS_SUB_PTR;

	// detect the bytecode execution routine offset
	tmp0 = FindBinary(0, SEARCH_DOWN|SEARCH_CASE, "68 85 08 68 85 09 68 85 00 68 85 01 A0 07 38");
	if(mmc == 1)
		BIOS_SUB_PTR = (tmp0&0x3FFF)|0xC000;
	else if(mmc == 5) {
		if(((GetReg(tmp0,"ds")-1)&1)==0)
			BIOS_SUB_PTR = (tmp0&0x1FFF)|0xC000;
		else
			BIOS_SUB_PTR = (tmp0&0x1FFF)|0xE000;
	}

#ifdef MANUAL
//	Message("> START AT 0x%08X\n",ea);

	while(1) {	// manual exit							// NO INDENTION!!
#else
	while(ea<=stopea) {
#endif

	if(isCode(GetFlags(ea))) {
		if(Byte(ea)==0x20) {
			// check if we at the bytecode execution routine
			tmp0=Word(ea+1);
			if(tmp0==BIOS_SUB_PTR) {
				tmp0 = Word(ea+3);
				tmp1 = tmp0&0x8000?"-":"";
				if(tmp0&0x8000)
					tmp0 = 0x10000-tmp0;
				// write down some info about variables buffer in stack
				ea = make_data_array_cust(ea+3, 2, form("LOCAL VARS ARRAY SIZE: %s%d",tmp1,tmp0));
#ifdef MANUAL
			} else {
				// stop if any other native 6502 opcode, obviously some error here
				Message(" 6502 opcode at 0x%08x, silly!\n",ea);
				return ea;
#endif
			}
#ifdef MANUAL
		} else {
			Message(" 6502 opcode at 0x%08x, silly!\n",ea);
			return ea;
#endif
		}
	}

	if(ea != BADADDR) {
		// get opcode
		cmd = Byte(ea);
		// get opcode name
		cmd_opc = koei_opcode(cmd);

//		Message("->cur ea = 0x%08X\n",ea);
//		Message("->cur cmd = %02X (%s)\n",cmd,cmd_opc);

		if(cmd==0xCF) {									// ret/exit/stop
			ea = make_data_array_cust(ea, 1, cmd_opc);
#ifdef MANUAL
//			Message("> STOP AT 0x%08X\n",ea);
			return ea;
#endif
		} else if(cmd==0xD5) {							// switch
			tmp0 = Word(ea + 1);
			tmp1 = Word(ea + 3);
			tmp2 = tmp0&0x8000?"-":"";
			if(tmp0&0x8000)
				tmp0 = 0x10000-tmp0;
			ea = make_data_array_cust(ea, 5, form(cmd_opc,tmp2,tmp0,tmp1));
			tmp2 = substr(CommentEx(ea, 0), 0, 3) + "DEFAULT";
			MakeComm(ea, "");
			MakeComm(ea, tmp2);
			set_name_offset_custom(ea,"case",0,mmc,last_bank,defb1,defb2);
			ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);				// no exec, default case
			for(i=0;i<tmp1;i++) {
				tmp2 = substr(CommentEx(ea, 0), 0, 3) + form("CASE: %d",i);
				MakeComm(ea, "");
				MakeComm(ea, tmp2);
				set_name_offset_custom(ea,"case",0,mmc,last_bank,defb1,defb2);
				ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);			// no exec, switch case
			}
		} else if(cmd==0xD9) {							// switch compare
			tmp0 = Word(ea + 1);
			ea = make_data_array_cust(ea, 3, form(cmd_opc,tmp0));
			for(i=0;i<tmp0;i++) {
				ea = make_data_array_cust(ea, 2, form("CASE: %d",i));
				set_name_offset_custom(ea,"case",0,mmc,last_bank,defb1,defb2);
				ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);			// no exec, switch case
			}
			tmp2 = substr(CommentEx(ea, 0), 0, 3) + "DEFAULT";
			MakeComm(ea, "");
			MakeComm(ea, tmp2);
			set_name_offset_custom(ea,"defult_case",0,mmc,last_bank,defb1,defb2);
			ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);				// no exec, default case
		} else if(((cmd>=0x00)&&(cmd<=0x7F))||((cmd>=0xB0)&&(cmd<=0xB6))||((cmd>=0xB8)&&(cmd<=0xCD))||((cmd>=0xD0)&&(cmd<=0xD4))||((cmd>=0xDA)&&(cmd<=0xDD))) {			// no args
			ea = make_data_array_cust(ea, 1, cmd_opc);
//		} else if((cmd == 0x80)||((cmd>=0x91)&&(cmd<=0x9F))||(cmd == 0xCE)||((cmd>=0xEB)&&(cmd<=0xFF))) {
														// BRK/EMPTY opcodes
//			ea = make_data_array_cust(ea, 1, cmd_opc);
		} else if(((cmd&0xF1)==0x81)||(cmd==0xAE)) {	// 1b arg
			if(cmd<=0x88) {
				tmp0 = Byte(ea + 1);
				tmp1 = tmp0&0x80?"-":"+";
				if(tmp0&0x80)
					tmp0 = 0x100-tmp0;
				ea = make_data_array_cust(ea, 2, form(cmd_opc,tmp1,tmp0));
			} else {
				tmp0 = Byte(ea + 1);
				ea = make_data_array_cust(ea, 2, form(cmd_opc,tmp0));
			}
		} else if((cmd==0x82)||(cmd==0x84)||(cmd==0x86)||(cmd==0x88)||(cmd==0x90)||((cmd>=0xA0)&&(cmd<=0xA3))||(cmd==0xAD)||(cmd==0xAF)||(cmd==0xDE)||(cmd==0xDF)) {
														// 2b arg
			tmp0 = Word(ea + 1);
			tmp1 = tmp0&0x8000?"-":"+";
			if(tmp0&0x8000)
				tmp0 = 0x10000-tmp0;
			ea = make_data_array_cust(ea, 3, form(cmd_opc,tmp1,tmp0));
		} else if((cmd>=0xE0)&&(cmd<=0xE2)) {			// 1b + 1b arg
			tmp0 = Byte(ea + 1);
			tmp1 = Byte(ea + 2);
			ea = make_data_array_cust(ea, 3, form(cmd_opc,tmp0,tmp1));
		} else if(((cmd>=0xA4)&&(cmd<=0xAB))||(cmd==0x8A)||(cmd==0x8C)||(cmd==0x8E)) {
														// ofs arg, no exec
			tmp1 = get_name_offset_custom(ea + 1,0,mmc,last_bank,defb1,defb2);
			ea = make_data_array_cust(ea, 1, form(cmd_opc,tmp1));
			ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);
		} else if((cmd>=0xD6)&&(cmd<=0xD8)) {			// opcode local ofs with name, no exec
			tmp1 = get_name_offset_custom(ea + 1,0,mmc,last_bank,defb1,defb2);
			ea = make_data_array_cust(ea, 1, form(cmd_opc,tmp1));
			set_name_offset_custom(ea,"loc",0,mmc,last_bank,defb1,defb2);
			ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);
		} else if((cmd==0xAC)) {						// ofs arg, exec
			tmp1 = get_name_offset_custom(ea + 1,1,mmc,last_bank,defb1,defb2);
			ea = make_data_array_cust(ea, 1, form(cmd_opc,tmp1));
			tmp1 = substr(tmp1, 0, 3);
			if((tmp1 == "loc")||(tmp1 == "sub"))
				set_name_offset_custom(ea,"procedure",1,mmc,last_bank,defb1,defb2);
			ea = make_offset_custom(ea, 1, mmc, last_bank, defb1, defb2);
		} else if(cmd==0xE9) {							// ofs arg, byte arg, exec
			tmp0 = Word(ea + 1);
			tmp1 = Byte(ea + 3);
			tmp2 = get_name_offset_custom(ea + 1,1,mmc,last_bank,defb1,defb2);
			ea = make_data_array_cust(ea, 1, form(cmd_opc,tmp2,tmp1));
			tmp2 = substr(tmp2, 0, 3);
			if((tmp2 == "loc")||(tmp2 == "sub"))
				set_name_offset_custom(ea,"procedure",1,mmc,last_bank,defb1,defb2);
			ea = make_offset_custom(ea, 1, mmc, last_bank, defb1, defb2);
			ea = make_data_array_cust(ea, 1, "");
		} else if(cmd==0xEA) {							// special indirect EJSR
			tmp0 = Byte(ea + 1);
			ea = make_data_array_cust(ea, 2, form(cmd_opc,tmp0));
		} else if((cmd>=0xE3)&&(cmd<=0xE5)) {			// special relative branches (never used, may be redundant)
			tmp0 = Byte(ea + 1);
			tmp1 = NameEx(BADADDR, ea - tmp0 + 1);
			if(tmp1 == "")
				tmp1 = form("0x%08x", ea - tmp0 + 1);
			ea = make_data_array_cust(ea, 2, form(cmd_opc,tmp1,tmp0));
		} else if((cmd>=0xE6)&&(cmd<=0xE8)) {			// special relative branches
			tmp0 = Byte(ea + 1);
			tmp1 = NameEx(BADADDR, ea + tmp0 + 1);
			if(tmp1 == "")
				tmp1 = form("0x%08x", ea + tmp0 + 1);
			ea = make_data_array_cust(ea, 2, form(cmd_opc,tmp1,tmp0));
		} else if(cmd==0xB7) {							// aithm32 opcodes
			tmp0 = Byte(ea + 1);
			if(tmp0 == 0x01)      cmd_opc = "MUL.D     A, B";
			else if(tmp0 == 0x02) cmd_opc = "DIVS.D    A, B";
			else if(tmp0 == 0x03) cmd_opc = "ADD.D     A, B";
			else if(tmp0 == 0x04) cmd_opc = "SUB.D     A, B";
			else if(tmp0 == 0x05) cmd_opc = "NEG.D     A";
			else if(tmp0 == 0x06) cmd_opc = "CMP.D.EQ  A, B";
			else if(tmp0 == 0x07) cmd_opc = "CMP.D.NE  A, B";
			else if(tmp0 == 0x08) cmd_opc = "CMPS.D.LT A, B";
			else if(tmp0 == 0x09) cmd_opc = "CMPS.D.LE A, B";
			else if(tmp0 == 0x0A) cmd_opc = "CMPS.D.GT A, B";
			else if(tmp0 == 0x0B) cmd_opc = "CMPS.D.GE A, B";
			else if(tmp0 == 0x0C) cmd_opc = "MOV.D     A, [HP%s%d]";
			else if(tmp0 == 0x0D) cmd_opc = "MOV.D     B, [HP%s%d]";
			else if(tmp0 == 0x0E) cmd_opc = "MOV.D     [HP%s%d], A";
			else if(tmp0 == 0x0F) cmd_opc = "PUSH.D    [HP%s%d]";
			else if(tmp0 == 0x10) cmd_opc = "MOV.D     A, [%s]";
			else if(tmp0 == 0x11) cmd_opc = "MOV.D     B, [%s]";
			else if(tmp0 == 0x12) cmd_opc = "MOV.D     [%s], A";
			else if(tmp0 == 0x13) cmd_opc = "PUSH.D    [%s]";
			else if(tmp0 == 0x14) cmd_opc = "PUSH.D    A";				// AWJ
			else if(tmp0 == 0x15) cmd_opc = "POP.D     B";
			else if(tmp0 == 0x16) cmd_opc = "MOV.D     A, [A]";
			else if(tmp0 == 0x17) cmd_opc = "MOV.D     [pop B], A";
			else if(tmp0 == 0x18) cmd_opc = "MOV.D     A, %d";
			else if(tmp0 == 0x19) cmd_opc = "MOV.D     B, %d";
			else if(tmp0 == 0x1A) cmd_opc = "SWAP.D    A, B";
			else if(tmp0 == 0x1B) cmd_opc = "INC.D     A";
			else if(tmp0 == 0x1C) cmd_opc = "DEC.D     A";
			else if(tmp0 == 0x1D) cmd_opc = "BOOL.D    A";				// A = bool(A != 0)
			else if(tmp0 == 0x1E) cmd_opc = "MODS.D    A, B";
			else if(tmp0 == 0x1F) cmd_opc = "ASL.D     A, B";
			else if(tmp0 == 0x20) cmd_opc = "ASR.D     A, B";
			else if(tmp0 == 0x21) cmd_opc = "COMP.D    A";
			else if(tmp0 == 0x22) cmd_opc = "AND.D     A, B";
			else if(tmp0 == 0x23) cmd_opc = "OR.D      A, B";
			else if(tmp0 == 0x24) cmd_opc = "XOR.D     A, B";
			else if(tmp0 == 0x25) cmd_opc = "EXTS.D    A";
			else if(tmp0 == 0x26) cmd_opc = "EXTU.D    A";
			else if(tmp0 == 0x27) cmd_opc = "NOP.D";
			else if(tmp0 == 0x28) cmd_opc = "NOT.D     A";				// should be if A>0, A=0, else A=1 but never saves the result!
			else if(tmp0 == 0x29) cmd_opc = "CMP.D.LT  A, B";
			else if(tmp0 == 0x2A) cmd_opc = "CMP.D.LE  A, B";
			else if(tmp0 == 0x2B) cmd_opc = "CMP.D.GT  A, B";
			else if(tmp0 == 0x2C) cmd_opc = "CMP.D.GE  A, B";
			else if(tmp0 == 0x2D) cmd_opc = "LSR.D     A, B";
			else if(tmp0 == 0x2E) cmd_opc = "DIVU.D    A, B";
			else if(tmp0 == 0x2F) cmd_opc = "MODU.D    A, B";
			else cmd_opc = "(illegal)";
			if(((tmp0>=0x01)&&(tmp0<=0x0B))||((tmp0>=0x14)&&(tmp0<=0x17))||((tmp0>=0x1A)&&(tmp0<=0x2F))) {
														// no args
				ea = make_data_array_cust(ea, 2, cmd_opc);
			} else if((tmp0>=0x0C)&&(tmp0<=0x0F)) {		// word args
				tmp1 = Word(ea + 2);
				tmp2 = tmp1&0x8000?"-":"+";
				if(tmp1&0x8000)
					tmp1 = 0x10000-tmp1;
				ea = make_data_array_cust(ea, 4, form(cmd_opc,tmp2,tmp1));
			} else if((tmp0>=0x10)&&(tmp0<=0x13)) {		// ofs arg, no exec
				tmp1 = get_name_offset_custom(ea + 2,0,mmc,last_bank,defb1,defb2);
				ea = make_data_array_cust(ea, 2, form(cmd_opc,tmp1));
				ea = make_offset_custom(ea, 0, mmc, last_bank, defb1, defb2);
			} else if((tmp0==0x18)||(tmp0==0x19)) {		// dword arg
				tmp1 = Dword(ea + 2);
				ea = make_data_array_cust(ea, 6, form(cmd_opc,tmp1));
			} else {
				Message(" unknown 32-bit arithmetic opcode at 0x%08x = %02x\n", ea, tmp0);
				return ea;
			}
		} else {
			Message(" unknown script opcode at 0x%08x = %02x\n", ea, cmd);
			return ea;
		}
	} else {
		Message(" bad address.\n",ea);
		return ea;
	}
	}			// WHILE END, NO INDENTION!!
	return ea;
}

#ifdef MANUAL
static koei_script_selection(void) {
	Jump(
		koei_disasm(ScreenEA(),BADADDR, MMC, LAST_BANK, -1, -1)//;
	);
}
#endif

static makevarnotlove(ea,name,len) {
	if(len) {
		MakeUnknown(ea, len, DOUNK_SIMPLE);
		MakeWord(ea);
		MakeData(ea, FF_BYTE, len, 0);
		SetArrayFormat(ea, AP_IDXHEX|AP_ALLOWDUPS, 0x10, 0);
	}
	MakeNameEx(ea,name,SN_CHECK|SN_NOWARN);
}

static ea_to_pattern(ea,mmc,last_bank) {
	auto tmp0, bank = GetReg(ea,"ds")-1;
	if(mmc == 1) {
		if(bank == last_bank)
			tmp0 = (ea&0x3FFF)|0xC000;
		else
			tmp0 = (ea&0x3FFF)|0x8000;
	} if(mmc == 5) {
		if(bank < (last_bank-1)) {
			if(SegStart(MK_FP(AskSelector(bank+1),0x8000)) != BADADDR)
				tmp0 = (ea&0x1FFF)|0x8000;
			else if(SegStart(MK_FP(AskSelector(bank+1),0xA000)) != BADADDR)
				tmp0 = (ea&0x1FFF)|0xA000;
		} else if((bank&1)==0)
			tmp0 = (ea&0x1FFF)|0xC000;
		else
			tmp0 = (ea&0x1FFF)|0xE000;
	}
	return form("20 %02X %02X", tmp0&0xFF, tmp0>>8);	// JSR opcode pattern
}

static auto_search_pattern(pattern, name,mmc,last_bank) {
	auto tmp0;
	tmp0 = FindBinary(0, SEARCH_DOWN|SEARCH_CASE, pattern);
	if(tmp0 != BADADDR) {
		makevarnotlove(tmp0,name,0);
		Message("Found \"%s\" at 0x%08X.\n",name,tmp0);
		return ea_to_pattern(tmp0,mmc,last_bank);
	} else {
		Message("Can't find \"%s\".\n",name);
		return -1;
	}
}

static main(void) {
	auto tmp0, tmp1;

// A0 17 B1 02 99 4E 00 88 C0 01 D0 F6 A9 ?? 48 A9 ?? 48 08 - bios_call_args
// 85 50 A9 ?? 85 51 A9 ?? 48 A9 16 48 08 6C ?? ?? EA 60	- bios_call_fast

#ifdef MANUAL
	makevarnotlove(0,"_reg0",2);
	makevarnotlove(2,"_regSP",2);
	makevarnotlove(4,"_regHP",2);
	makevarnotlove(6,"_regPC",2);
	makevarnotlove(8,"_regA",4);
	makevarnotlove(0xC,"_regB",4);
	makevarnotlove(0x10,"_reg32",4);
	makevarnotlove(0x50,"_bios_args",22);
	makevarnotlove(0x66,"_bios_result",2);
	makevarnotlove(0x100,"_cpu_stack",256);
	makevarnotlove(0x200,"_scr_stack",1024);
	makevarnotlove(0x600,"_spr_buf",256);

	if(MMC == 5) {
		makevarnotlove(0x5100,"_MMC5_PRG_SIZE",1);
		makevarnotlove(0x5101,"_MMC5_CHR_SIZE",1);
		makevarnotlove(0x5102,"_MMC5_WRAM0_ENABLE",1);
		makevarnotlove(0x5103,"_MMC5_WRAM1_ENABLE",1);
		makevarnotlove(0x5104,"_MMC5_CHR_MODE",1);
		makevarnotlove(0x5105,"_MMC5_NT_MODE",1);
		makevarnotlove(0x5106,"_MMC5_NT_FILL",1);
		makevarnotlove(0x5107,"_MMC5_AT_FILL",12);
		makevarnotlove(0x5113,"_MMC5_PRG_PAGES",5);
		makevarnotlove(0x5120,"_MMC5_CHR_BANKSA",8);
		makevarnotlove(0x5128,"_MMC5_CHR_BANKSB",8);
		makevarnotlove(0x5200,"_MMC5_SP_MODE",1);
		makevarnotlove(0x5201,"_MMC5_SP_SCROLL",1);
		makevarnotlove(0x5202,"_MMC5_SP_PAGE",2);
		makevarnotlove(0x5204,"_MMC5_IRQ_ENABLE",1);
		makevarnotlove(0x5205,"_MMC5_MUL0",1);
		makevarnotlove(0x5206,"_MMC5_MUL1",1);
		makevarnotlove(0x5C00,"_MMC5_EXRAM",1024);
	}

	AddHotkey("Shift-O", "koei_script_selection");

	#ifdef AUTODETECT
	// search for native version of "EJSR ofs, sp+N" opcode
	tmp0 = auto_search_pattern("68 85 00 68 85 01 38 A0 03 A5 02 F1 00 85 02 B0 02 C6 03", "_native_procedure_call_param",MMC,LAST_BANK);
	if(tmp0 != -1) native_calls_search(tmp0,MMC,LAST_BANK,-1,-1);

	// search for native version of bytecode procedure execution routine (prepares stack, do bytecode stack backup)
	tmp0 = auto_search_pattern("68 85 08 68 85 09 18 68 69 01 85 00 68 69 00 85 01 A0 07", "_native_procedure_exec_param",MMC,LAST_BANK);
	if(tmp0 != -1) parametric_fixsize(tmp0, 3);

	// search for all bytecode procedure execution routines
	tmp0 = FindBinary(0, SEARCH_DOWN|SEARCH_CASE, "68 85 08 68 85 09 68 85 00 68 85 01 A0 07 38");
	if(tmp0 != BADADDR)
		makevarnotlove(tmp0,"_procedure_exec_param",0);
		Message("Found \"_procedure_exec_param\" at 0x%08X.\n",tmp0);
		tmp1 = ea_to_pattern(tmp0,MMC,LAST_BANK);
		tmp0 = -1;
		parametric_stopbytes(tmp1, "CF");	// first pass, protects from auto code
		Message(" Procedure auto search start.\n");
		while ((tmp0 = FindBinary(tmp0 + 1, SEARCH_DOWN|SEARCH_CASE, tmp1)) != BADADDR) {
			Message(" >found one at 0x%08X.\n",tmp0);
			make_data_array_cust(tmp0 + 3, 2, "");
			MakeCode(tmp0);
			AutoMark(tmp0, AU_CODE);
			Wait();
			koei_disasm(tmp0,BADADDR,MMC,LAST_BANK,-1,-1);
		}
		Message(" Done!\n");
	#endif

#else

	auto md5 = GetInputMD5();

	if (md5=="42C744D52C85EB55974586E470842DC1") {
		Message(" Start Gemfire decompilation.\n");
		koei_disasm(0x008003, 0x00978C, 5, 0x1F, -1, -1);
		koei_disasm(0x020003, 0x020824, 5, 0x1F, -1, -1);
		koei_disasm(0x038142, 0x039E77, 5, 0x1F, -1, -1);
		koei_disasm(0x06A003, 0x06B700, 5, 0x1F, -1, -1);
		koei_disasm(0x084003, 0x084D83, 5, 0x1F, -1, -1);
		koei_disasm(0x09E003, 0x09ED41, 5, 0x1F, -1, -1);
		koei_disasm(0x0B6003, 0x0B7472, 5, 0x1F, -1, -1);
		koei_disasm(0x0CE003, 0x0CEECB, 5, 0x1F, -1, -1);
		koei_disasm(0x0E6003, 0x0E7710, 5, 0x1F, -1, -1);
		koei_disasm(0x0E7A16, 0x0E7C6D, 5, 0x1F, -1, -1);
		koei_disasm(0x102003, 0x102E93, 5, 0x1F, -1, -1);
		koei_disasm(0x11A003, 0x11AA70, 5, 0x1F, -1, -1);
		koei_disasm(0x132003, 0x133FFE, 5, 0x1F, -1, -1);
		koei_disasm(0x14C000, 0x14C325, 5, 0x1F, -1, -1);
		koei_disasm(0x164003, 0x1659DD, 5, 0x1F, -1, -1);
		koei_disasm(0x196003, 0x1978F2, 5, 0x1F, -1, -1);
		koei_disasm(0x2F6003, 0x2F618F, 5, 0x1F, -1, -1);
		koei_disasm(0x2F622D, 0x2F7E53, 5, 0x1F, -1, -1);
		koei_disasm(0x2F7F3B, 0x2F7FA9, 5, 0x1F, -1, -1);
	} else if (md5=="23FE0C32B5480F063110A90313735C71") {
		Message(" Start Royal Blood decompilation.\n");
		koei_disasm(0x008003, 0x009607, 5, 0x1F, -1, -1);
		koei_disasm(0x020003, 0x020578, 5, 0x1F, -1, -1);
		koei_disasm(0x038003, 0x039E88, 5, 0x1F, -1, -1);
		koei_disasm(0x06A003, 0x06B698, 5, 0x1F, -1, -1);
		koei_disasm(0x084003, 0x084DDB, 5, 0x1F, -1, -1);
		koei_disasm(0x09E003, 0x09ECE2, 5, 0x1F, -1, -1);
		koei_disasm(0x0B8003, 0x0B9309, 5, 0x1F, -1, -1);
		koei_disasm(0x0D0003, 0x0D0EA6, 5, 0x1F, -1, -1);
		koei_disasm(0x0E8003, 0x0E9725, 5, 0x1F, -1, -1);
		koei_disasm(0x0E99B4, 0x0E9BFB, 5, 0x1F, -1, -1);
		koei_disasm(0x104003, 0x1052E0, 5, 0x1F, -1, -1);
		koei_disasm(0x11C003, 0x11CA70, 5, 0x1F, -1, -1);
		koei_disasm(0x134003, 0x135FFE, 5, 0x1F, -1, -1);
		koei_disasm(0x14E000, 0x14E530, 5, 0x1F, -1, -1);
		koei_disasm(0x166003, 0x16796A, 5, 0x1F, -1, -1);
		koei_disasm(0x198003, 0x1998F2, 5, 0x1F, -1, -1);
		koei_disasm(0x300003, 0x30018F, 5, 0x1F, -1, -1);
		koei_disasm(0x30022D, 0x301DA1, 5, 0x1F, -1, -1);
		koei_disasm(0x301E89, 0x301ED8, 5, 0x1F, -1, -1);
	} else if (md5=="9C77BD2874CB7F80AA8A05919203300C") {
		Message(" Start Aoki Ookami 1 decompilation.\n");
		koei_disasm(0x008003, 0x00B11A, 1, 0xF, -1, -1);
		koei_disasm(0x020003, 0x02311F, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03B3E7, 1, 0xF, -1, -1);
		koei_disasm(0x050003, 0x0518DA, 1, 0xF, -1, -1);
		koei_disasm(0x068003, 0x069700, 1, 0xF, -1, -1);
		koei_disasm(0x080003, 0x083B4A, 1, 0xF, -1, -1);
		koei_disasm(0x098003, 0x0980AA, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F8232, 1, 0xF, -1, -1);
		koei_disasm(0x140003, 0x14028B, 1, 0xF, -1, -1);
		koei_disasm(0x174A03, 0x176D30, 1, 0xF, -1, -1);
	} else if (md5=="C7F80A7E1A2A5335AF114CAC7B424ACF") {
		Message(" Start Genghis Khan 1 decompilation.\n");
		koei_disasm(0x008003, 0x00B21B, 1, 0xF, -1, -1);
		koei_disasm(0x020003, 0x022FB0, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03ADE3, 1, 0xF, -1, -1);
		koei_disasm(0x050003, 0x051BC2, 1, 0xF, -1, -1);
		koei_disasm(0x068003, 0x069768, 1, 0xF, -1, -1);
		koei_disasm(0x080003, 0x083BB1, 1, 0xF, -1, -1);
		koei_disasm(0x098003, 0x0980AA, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F8232, 1, 0xF, -1, -1);
		koei_disasm(0x140003, 0x14028B, 1, 0xF, -1, -1);
		koei_disasm(0x174B03, 0x176DC2, 1, 0xF, -1, -1);
	} else if (md5=="5EF1BA9461974F34B0633EAF063490DD") {
		Message(" Start Bandit Kings decompilation.\n");
		koei_disasm(0x008003, 0x008C25, 5, 0x1F, -1, -1);
		koei_disasm(0x008CE9, 0x009366, 5, 0x1F, -1, -1);
		koei_disasm(0x020003, 0x021B3A, 5, 0x1F, -1, -1);
		koei_disasm(0x03A003, 0x03A7DD, 5, 0x1F, -1, -1);
		koei_disasm(0x03B003, 0x03BD91, 5, 0x1F, -1, -1);
		koei_disasm(0x054003, 0x055AF5, 5, 0x1F, -1, -1);
		koei_disasm(0x06E003, 0x06F7A2, 5, 0x1F, -1, -1);
		koei_disasm(0x088003, 0x08924A, 5, 0x1F, -1, -1);
		koei_disasm(0x0A2003, 0x0A32B3, 5, 0x1F, -1, -1);
		koei_disasm(0x0A3357, 0x0A3609, 5, 0x1F, -1, -1);
		koei_disasm(0x0BC003, 0x0BD9F5, 5, 0x1F, -1, -1);
		koei_disasm(0x0D6003, 0x0D71DB, 5, 0x1F, -1, -1);
		koei_disasm(0x0EE003, 0x0EFBD7, 5, 0x1F, -1, -1);
		koei_disasm(0x108003, 0x1096B8, 5, 0x1F, -1, -1);
		koei_disasm(0x122003, 0x12395B, 5, 0x1F, -1, -1);
		koei_disasm(0x13A003, 0x13BFFF, 5, 0x1F, -1, -1);
		koei_disasm(0x154000, 0x155286, 5, 0x1F, -1, -1);
		koei_disasm(0x16C003, 0x16DFFF, 5, 0x1F, -1, -1);
		koei_disasm(0x186000, 0x187038, 5, 0x1F, -1, -1);
		koei_disasm(0x1A0003, 0x1A140A, 5, 0x1F, -1, -1);
		koei_disasm(0x29C003, 0x29C3D4, 5, 0x1F, -1, -1);
		koei_disasm(0x304190, 0x305D5B, 5, 0x1F, -1, -1);
		koei_disasm(0x322003, 0x3223BE, 5, 0x1F, -1, -1);
	} else if (md5=="862DFE1D1345A8903819DFB0614E3D88") {
		Message(" Start Suikoden decompilation.\n");
		koei_disasm(0x008003, 0x008BF8, 5, 0x1F, -1, -1);
		koei_disasm(0x008CBC, 0x0098E3, 5, 0x1F, -1, -1);
		koei_disasm(0x020003, 0x021B63, 5, 0x1F, -1, -1);
		koei_disasm(0x03A003, 0x03A81B, 5, 0x1F, 1, -1);	// first half of bank uses 1 as library
		koei_disasm(0x03B003, 0x03BDD5, 5, 0x1F, 9, -1);	// second half uses 9 instead
		koei_disasm(0x054003, 0x055AEB, 5, 0x1F, 1, -1);
		koei_disasm(0x06E003, 0x06F991, 5, 0x1F, 1, -1);
		koei_disasm(0x086003, 0x087307, 5, 0x1F, 1, -1);
		koei_disasm(0x0A0003, 0x0A1130, 5, 0x1F, 1, -1);
		koei_disasm(0x0A11D4, 0x0A1486, 5, 0x1F, 1, -1);
		koei_disasm(0x0BA003, 0x0BBA02, 5, 0x1F, 1, -1);
		koei_disasm(0x0D4003, 0x0D51CB, 5, 0x1F, 1, -1);
		koei_disasm(0x0EC003, 0x0EDBBC, 5, 0x1F, -1, -1);
		koei_disasm(0x106003, 0x1076E5, 5, 0x1F, 9, -1);
		koei_disasm(0x120003, 0x121958, 5, 0x1F, 9, -1);
		koei_disasm(0x138003, 0x139FFF, 5, 0x1F, -1, 0xD);	// paired banks, goes together always
		koei_disasm(0x152000, 0x153244, 5, 0x1F, 0xC, -1);
		koei_disasm(0x16A003, 0x16BFFF, 5, 0x1F, -1, 0xF);
		koei_disasm(0x184000, 0x185064, 5, 0x1F, 0xE, -1);
		koei_disasm(0x19E003, 0x19F452, 5, 0x1F, -1, -1);
		koei_disasm(0x302190, 0x303C76, 5, 0x1F, -1, -1);
		koei_disasm(0x320003, 0x3203B8, 5, 0x1F, -1, -1);
	} else if (md5=="60F52E9EF4B97E52AB6D47BE73DA815F") {
		Message(" Start L'Empereur usa decompilation.\n");
		koei_disasm(0x00A003, 0x00B4E4, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x0230A3, 5, 0x1F, -1, -1);
		koei_disasm(0x03C003, 0x03D03B, 5, 0x1F, -1, -1);
		koei_disasm(0x056003, 0x057E1F, 5, 0x1F, -1, -1);
		koei_disasm(0x070003, 0x071A38, 5, 0x1F, -1, -1);
		koei_disasm(0x08A003, 0x08BCFF, 5, 0x1F, -1, -1);
		koei_disasm(0x0A4003, 0x0A5A1F, 5, 0x1F, -1, -1);
		koei_disasm(0x0BE003, 0x0BFED3, 5, 0x1F, -1, -1);
		koei_disasm(0x0D8003, 0x0D9455, 5, 0x1F, -1, -1);
		koei_disasm(0x0F2003, 0x0F3AD2, 5, 0x1F, -1, -1);
		koei_disasm(0x10A003, 0x10B8E6, 5, 0x1F, -1, -1);
		koei_disasm(0x124003, 0x125C84, 5, 0x1F, -1, -1);
		koei_disasm(0x13E003, 0x13EF11, 5, 0x1F, -1, -1);
		koei_disasm(0x158194, 0x159BFC, 5, 0x1F, -1, -1);
		koei_disasm(0x172003, 0x1731BE, 5, 0x1F, -1, -1);
		koei_disasm(0x18A003, 0x18A781, 5, 0x1F, -1, -1);
		koei_disasm(0x1A2003, 0x1A314F, 5, 0x1F, -1, -1);
		koei_disasm(0x1BA003, 0x1BBFFD, 5, 0x1F, -1, -1);
		koei_disasm(0x1D4000, 0x1D4CDF, 5, 0x1F, -1, -1);
		koei_disasm(0x304003, 0x30530F, 5, 0x1F, -1, -1);
		koei_disasm(0x322003, 0x3221C0, 5, 0x1F, -1, -1);
	} else if (md5=="0F7EB49332C0ACF1D97FC63A08C485C3") {
		Message(" Start L'Empereur japan decompilation.\n");
		koei_disasm(0x00A003, 0x00B499, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x0230B8, 5, 0x1F, -1, -1);
		koei_disasm(0x03C003, 0x03D022, 5, 0x1F, -1, -1);
		koei_disasm(0x056003, 0x057E1B, 5, 0x1F, -1, -1);
		koei_disasm(0x070003, 0x071A3D, 5, 0x1F, -1, -1);
		koei_disasm(0x08A003, 0x08BCF4, 5, 0x1F, -1, -1);
		koei_disasm(0x0A4003, 0x0A5A11, 5, 0x1F, -1, -1);
		koei_disasm(0x0BE003, 0x0BFEBD, 5, 0x1F, -1, -1);
		koei_disasm(0x0D8003, 0x0D9455, 5, 0x1F, -1, -1);
		koei_disasm(0x0F2003, 0x0F3AE6, 5, 0x1F, -1, -1);
		koei_disasm(0x10A003, 0x10B8A6, 5, 0x1F, -1, -1);
		koei_disasm(0x124003, 0x125C5F, 5, 0x1F, -1, -1);
		koei_disasm(0x13E003, 0x13EF0C, 5, 0x1F, -1, -1);
		koei_disasm(0x158194, 0x159BDC, 5, 0x1F, -1, -1);
		koei_disasm(0x172003, 0x1731BE, 5, 0x1F, -1, -1);
		koei_disasm(0x18A003, 0x18A6FE, 5, 0x1F, -1, -1);
		koei_disasm(0x1A2003, 0x1A312C, 5, 0x1F, -1, -1);
		koei_disasm(0x1BA003, 0x1BBFFF, 5, 0x1F, -1, -1);
		koei_disasm(0x1D4001, 0x1D4C95, 5, 0x1F, -1, -1);
		koei_disasm(0x302003, 0x30335E, 5, 0x1F, -1, -1);
		koei_disasm(0x320003, 0x3201A9, 5, 0x1F, -1, -1);
	} else if (md5=="B35C3CBB57EF285A24EA35426BEB8D3C") {
		Message(" Start Romance of Three Kingdoms decompilation.\n");
		koei_disasm(0x008003, 0x00A26C, 1, 0xF, -1, -1);
		koei_disasm(0x020089, 0x023A4C, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03A6FC, 1, 0xF, -1, -1);
		koei_disasm(0x050003, 0x052553, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F82DA, 1, 0xF, -1, -1);
		koei_disasm(0x129603, 0x12A6FC, 1, 0xF, -1, -1);
		koei_disasm(0x1749C3, 0x176EB3, 1, 0xF, -1, -1);
	} else if (md5=="B2A5321E8625D74F9CCC9BD475472FDC") {
		Message(" Start Sangokushi decompilation.\n");
		koei_disasm(0x008003, 0x00A23E, 1, 0xF, -1, -1);
		koei_disasm(0x020089, 0x023A5B, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03ABB3, 1, 0xF, -1, -1);
		koei_disasm(0x050003, 0x05251F, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F82DA, 1, 0xF, -1, -1);
		koei_disasm(0x129603, 0x12A703, 1, 0xF, -1, -1);
		koei_disasm(0x1749C3, 0x177062, 1, 0xF, -1, -1);
	} else if (md5=="A185F95B3AE2209644C2D9885DFA1A3B") {
		Message(" Start Nobunaga's Ambitions decompilation.\n");
		koei_disasm(0x008003, 0x00A849, 1, 0xF, -1, -1);
		koei_disasm(0x020003, 0x0239B1, 1, 0xF, -1, -1);
		koei_disasm(0x038086, 0x03B0B8, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F808E, 1, 0xF, -1, -1);
		koei_disasm(0x158003, 0x158225, 1, 0xF, -1, -1);
		koei_disasm(0x174A03, 0x174BCC, 1, 0xF, -1, -1);
		koei_disasm(0x174C35, 0x176822, 1, 0xF, -1, -1);
	} else if (md5=="44B65B5E88CF5A329F7A08C8A2D9F672") {
		Message(" Start Nobunaga no Yabou decompilation.\n");
		koei_disasm(0x008003, 0x00A83A, 1, 0xF, -1, -1);
		koei_disasm(0x020003, 0x023980, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03AFF6, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F808E, 1, 0xF, -1, -1);
		koei_disasm(0x158003, 0x158225, 1, 0xF, -1, -1);
		koei_disasm(0x174A03, 0x174BCC, 1, 0xF, -1, -1);
		koei_disasm(0x174C35, 0x1767EE, 1, 0xF, -1, -1);
	} else if (md5=="967FB047B19FB85DDA51B88A3FF72B94") {
		Message(" Start Nobunaga no Yabou revA decompilation.\n");
		koei_disasm(0x008003, 0x00A83A, 1, 0xF, -1, -1);
		koei_disasm(0x020003, 0x0239AB, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03B016, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0F808E, 1, 0xF, -1, -1);
		koei_disasm(0x158003, 0x158225, 1, 0xF, -1, -1);
		koei_disasm(0x174A03, 0x174BCC, 1, 0xF, -1, -1);
		koei_disasm(0x174C35, 0x176817, 1, 0xF, -1, -1);
	} else if (md5=="A33DFE9E4ADC67321877042F51877687") {
		Message(" Start Nobunaga's Ambition 2 decompilation.\n");
		koei_disasm(0x00A003, 0x00BA8C, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x022ED0, 5, 0x1F, -1, -1);
		koei_disasm(0x03A003, 0x03BA95, 5, 0x1F, -1, -1);
		koei_disasm(0x054003, 0x0554AD, 5, 0x1F, -1, -1);
		koei_disasm(0x06E003, 0x06F9DB, 5, 0x1F, -1, -1);
		koei_disasm(0x088003, 0x08994B, 5, 0x1F, -1, -1);
		koei_disasm(0x0A2003, 0x0A3843, 5, 0x1F, -1, -1);
		koei_disasm(0x0BC003, 0x0BD32C, 5, 0x1F, -1, -1);
		koei_disasm(0x0D6003, 0x0D6C3D, 5, 0x1F, -1, -1);
		koei_disasm(0x0EE003, 0x0EFA90, 5, 0x1F, -1, -1);
		koei_disasm(0x108003, 0x109B96, 5, 0x1F, -1, -1);
		koei_disasm(0x122003, 0x1236F0, 5, 0x1F, -1, -1);
		koei_disasm(0x13A003, 0x13BCDA, 5, 0x1F, -1, -1);
		koei_disasm(0x16C003, 0x16DFFE, 5, 0x1F, -1, -1);
		koei_disasm(0x186001, 0x186F97, 5, 0x1F, -1, -1);
		koei_disasm(0x19E003, 0x19F688, 5, 0x1F, -1, -1);
		koei_disasm(0x300003, 0x30158C, 5, 0x1F, -1, -1);
		koei_disasm(0x31E003, 0x31E21F, 5, 0x1F, -1, -1);
	} else if (md5=="C2240AAEBE0911C1684B656B7B5B8B0C") {
		Message(" Start Nobunaga no Yabou 2 decompilation.\n");
		koei_disasm(0x00A003, 0x00B99E, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x022EF4, 5, 0x1F, -1, -1);
		koei_disasm(0x03A003, 0x03BA98, 5, 0x1F, -1, -1);
		koei_disasm(0x054003, 0x0552D7, 5, 0x1F, -1, -1);
		koei_disasm(0x06E003, 0x06F9DE, 5, 0x1F, -1, -1);
		koei_disasm(0x088003, 0x089AB5, 5, 0x1F, -1, -1);
		koei_disasm(0x0A2003, 0x0A385F, 5, 0x1F, -1, -1);
		koei_disasm(0x0BC003, 0x0BD301, 5, 0x1F, -1, -1);
		koei_disasm(0x0D6003, 0x0D6C3C, 5, 0x1F, -1, -1);
		koei_disasm(0x0EE003, 0x0EFABA, 5, 0x1F, -1, -1);
		koei_disasm(0x108003, 0x109BBE, 5, 0x1F, -1, -1);
		koei_disasm(0x122003, 0x1236F0, 5, 0x1F, -1, -1);
		koei_disasm(0x13A003, 0x13BCA5, 5, 0x1F, -1, -1);
		koei_disasm(0x16C003, 0x16DFFF, 5, 0x1F, -1, -1);
		koei_disasm(0x186000, 0x18700A, 5, 0x1F, -1, -1);
		koei_disasm(0x19E003, 0x19F5B7, 5, 0x1F, -1, -1);
		koei_disasm(0x300003, 0x3014F7, 5, 0x1F, -1, -1);
		koei_disasm(0x31E003, 0x31E21F, 5, 0x1F, -1, -1);
	} else if (md5=="A53911F154FF89CE1BAF305126F6949F") {
		#define UNCHARTED
		Message(" Start Uncharted Waters decompilation.\n");
		koei_disasm(0x008003, 0x009909, 5, 0x3F, -1, 1);
		koei_disasm(0x022003, 0x022691, 5, 0x3F, -1, 1);
		koei_disasm(0x022B2C, 0x023671, 5, 0x3F, -1, 1);
		koei_disasm(0x0237CE, 0x023A8B, 5, 0x3F, -1, 1);
		koei_disasm(0x03A003, 0x03AFC1, 5, 0x3F, -1, 1);
		koei_disasm(0x082003, 0x082C74, 5, 0x3F, -1, 1);
		koei_disasm(0x0CB8CB, 0x0CBBE3, 5, 0x3F, -1, 1);
		koei_disasm(0x0E2003, 0x0E2061, 5, 0x3F, -1, 1);
		koei_disasm(0x112003, 0x112370, 5, 0x3F, -1, 1);
		koei_disasm(0x12C003, 0x12DCDC, 5, 0x3F, -1, 1);
		koei_disasm(0x15E003, 0x15F5F7, 5, 0x3F, -1, 1);
		koei_disasm(0x176003, 0x176D96, 5, 0x3F, -1, 1);
		koei_disasm(0x190003, 0x1914E9, 5, 0x3F, -1, 1);
		koei_disasm(0x1A8003, 0x1A874D, 5, 0x3F, -1, 1);
		koei_disasm(0x1A8DC3, 0x1A971F, 5, 0x3F, -1, 1);
		koei_disasm(0x1C0003, 0x1C12B0, 5, 0x3F, -1, 1);
		koei_disasm(0x1D8003, 0x1D94D5, 5, 0x3F, -1, 1);
		koei_disasm(0x1F0003, 0x1F14C9, 5, 0x3F, -1, 1);
		koei_disasm(0x208003, 0x209257, 5, 0x3F, -1, 1);
		koei_disasm(0x220003, 0x22105C, 5, 0x3F, -1, 1);
		koei_disasm(0x238003, 0x23917E, 5, 0x3F, -1, 1);
		koei_disasm(0x250003, 0x2512B1, 5, 0x3F, -1, 1);
		koei_disasm(0x268003, 0x269066, 5, 0x3F, -1, 1);
		koei_disasm(0x280003, 0x280912, 5, 0x3F, -1, 1);
		koei_disasm(0x298003, 0x299A68, 5, 0x3F, -1, 1);
		koei_disasm(0x2B0003, 0x2B0B8D, 5, 0x3F, -1, 1);
		koei_disasm(0x2C8003, 0x2C88CC, 5, 0x3F, -1, 1);
		koei_disasm(0x2E0003, 0x2E0B9A, 5, 0x3F, -1, 1);
		koei_disasm(0x35C003, 0x35C2B3, 5, 0x3F, -1, 1);
		koei_disasm(0x374003, 0x374225, 5, 0x3F, -1, 1);
		koei_disasm(0x38C003, 0x38C498, 5, 0x3F, -1, 1);
		koei_disasm(0x3A6003, 0x3A673B, 5, 0x3F, -1, 1);
		koei_disasm(0x60047B, 0x600EAC, 5, 0x3F, -1, 1);
		koei_disasm(0x6014A5, 0x601CEB, 5, 0x3F, -1, 1);
		koei_disasm(0x61E003, 0x61E0FF, 5, 0x3F, -1, 1);
	} else if (md5=="F8B22A4C325C32071090492AC8174D0A") {
		#define DAIKOKUJIDAI
		Message(" Start Daikoukai Jidai decompilation.\n");
		koei_disasm(0x008003, 0x0099C5, 5, 0x3F, -1, 1);
		koei_disasm(0x022003, 0x022689, 5, 0x3F, -1, 1);
		koei_disasm(0x022B24, 0x0235C5, 5, 0x3F, -1, 1);
		koei_disasm(0x023722, 0x0239DF, 5, 0x3F, -1, 1);
		koei_disasm(0x03A003, 0x03AE6A, 5, 0x3F, -1, 1);
		koei_disasm(0x082003, 0x082C74, 5, 0x3F, -1, 1);
		koei_disasm(0x0CB82D, 0x0CBB48, 5, 0x3F, -1, 1);
		koei_disasm(0x0E2003, 0x0E2061, 5, 0x3F, -1, 1);
		koei_disasm(0x112003, 0x112370, 5, 0x3F, -1, 1);
		koei_disasm(0x12C003, 0x12DD18, 5, 0x3F, -1, 1);
		koei_disasm(0x15E003, 0x15F5CA, 5, 0x3F, -1, 1);
		koei_disasm(0x176003, 0x176D8B, 5, 0x3F, -1, 1);
		koei_disasm(0x190003, 0x1914A0, 5, 0x3F, -1, 1);
		koei_disasm(0x1A8003, 0x1A872C, 5, 0x3F, -1, 1);
		koei_disasm(0x1A8DA2, 0x1A96A3, 5, 0x3F, -1, 1);
		koei_disasm(0x1C2003, 0x1C32B7, 5, 0x3F, -1, 1);
		koei_disasm(0x1DA003, 0x1DB562, 5, 0x3F, -1, 1);
		koei_disasm(0x1F2003, 0x1F3496, 5, 0x3F, -1, 1);
		koei_disasm(0x20A003, 0x20B23A, 5, 0x3F, -1, 1);
		koei_disasm(0x222003, 0x22302D, 5, 0x3F, -1, 1);
		koei_disasm(0x23A003, 0x23B1DD, 5, 0x3F, -1, 1);
		koei_disasm(0x252003, 0x253307, 5, 0x3F, -1, 1);
		koei_disasm(0x26A003, 0x26B0A8, 5, 0x3F, -1, 1);
		koei_disasm(0x282003, 0x2828E0, 5, 0x3F, -1, 1);
		koei_disasm(0x29A003, 0x29BA32, 5, 0x3F, -1, 1);
		koei_disasm(0x2B4003, 0x2B4B8D, 5, 0x3F, -1, 1);
		koei_disasm(0x2CC003, 0x2CC8CF, 5, 0x3F, -1, 1);
		koei_disasm(0x2E4003, 0x2E4B78, 5, 0x3F, -1, 1);
		koei_disasm(0x360003, 0x3602B3, 5, 0x3F, -1, 1);
		koei_disasm(0x378003, 0x378225, 5, 0x3F, -1, 1);
		koei_disasm(0x390003, 0x390167, 5, 0x3F, -1, 1);
		koei_disasm(0x3A8003, 0x3A89C0, 5, 0x3F, -1, 1);
		koei_disasm(0x60247B, 0x602EAC, 5, 0x3F, -1, 1);
		koei_disasm(0x6034A5, 0x603CEB, 5, 0x3F, -1, 1);
		koei_disasm(0x620003, 0x6200FF, 5, 0x3F, -1, 1);
	} else if (md5=="13B9EE48FEABFB16851016E0EFC245C7") {
		Message(" Start Famicom Top Management decompilation.\n");
		koei_disasm(0x008003, 0x00B5F3, 1, 0xF, -1, -1);
		koei_disasm(0x020003, 0x023159, 1, 0xF, -1, -1);
		koei_disasm(0x038003, 0x03B8EC, 1, 0xF, -1, -1);
		koei_disasm(0x050003, 0x052486, 1, 0xF, -1, -1);
		koei_disasm(0x068003, 0x06B1B8, 1, 0xF, -1, -1);
		koei_disasm(0x080003, 0x082E44, 1, 0xF, -1, -1);
		koei_disasm(0x0F8003, 0x0FA187, 1, 0xF, -1, -1);
		koei_disasm(0x128003, 0x129798, 1, 0xF, -1, -1);
		koei_disasm(0x140003, 0x143BB1, 1, 0xF, -1, -1);
		koei_disasm(0x174983, 0x17694E, 1, 0xF, -1, -1);
	} else if (md5=="78D9B15C629CA983AC7D681685A67625") {
		Message(" Start Ishin no Arashi decompilation.\n");
		koei_disasm(0x008003, 0x008E0E, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x023D24, 5, 0x1F, -1, -1);
		koei_disasm(0x03A003, 0x03B7AC, 5, 0x1F, -1, -1);
		koei_disasm(0x06C003, 0x06D418, 5, 0x1F, -1, -1);
		koei_disasm(0x086003, 0x087429, 5, 0x1F, -1, -1);
		koei_disasm(0x0A0003, 0x0A0E06, 5, 0x1F, -1, -1);
		koei_disasm(0x0B8003, 0x0B9A93, 5, 0x1F, -1, -1);
		koei_disasm(0x0D2003, 0x0D2A47, 5, 0x1F, -1, -1);
		koei_disasm(0x0D2F77, 0x0D38DB, 5, 0x1F, -1, -1);
		koei_disasm(0x0EC003, 0x0ED639, 5, 0x1F, -1, -1);
		koei_disasm(0x106003, 0x106FE5, 5, 0x1F, -1, -1);
		koei_disasm(0x11E003, 0x11F114, 5, 0x1F, -1, -1);
		koei_disasm(0x138003, 0x139145, 5, 0x1F, -1, -1);
		koei_disasm(0x150003, 0x1511E6, 5, 0x1F, -1, -1);
		koei_disasm(0x168003, 0x169FFF, 5, 0x1F, -1, -1);
		koei_disasm(0x182003, 0x183037, 5, 0x1F, -1, -1);
		koei_disasm(0x2FC003, 0x2FD958, 5, 0x1F, -1, -1);
		koei_disasm(0x31A003, 0x31A055, 5, 0x1F, -1, -1);
	} else if (md5=="F47EF59EA3453BEA7C41385839FF9CA8") {
		Message(" Start Romance of The Three Kingdoms 2 decompilation.\n");
		koei_disasm(0x00A003, 0x00BA88, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x023D31, 5, 0x1F, -1, -1);
		koei_disasm(0x03C003, 0x03DB7D, 5, 0x1F, -1, -1);
		koei_disasm(0x056003, 0x057BCE, 5, 0x1F, -1, -1);
		koei_disasm(0x070003, 0x071DC6, 5, 0x1F, -1, -1);
		koei_disasm(0x08A003, 0x08B813, 5, 0x1F, -1, -1);
		koei_disasm(0x0A4003, 0x0A591B, 5, 0x1F, -1, -1);
		koei_disasm(0x0BE003, 0x0BF8BC, 5, 0x1F, -1, -1);
		koei_disasm(0x0D6003, 0x0D75FE, 5, 0x1F, -1, -1);
		koei_disasm(0x0F0003, 0x0F1C39, 5, 0x1F, 8, -1);
		koei_disasm(0x0F1E03, 0x0F1F0B, 5, 0x1F, 8, -1);
		koei_disasm(0x10A003, 0x10BCDA, 5, 0x1F, 8, -1);
		koei_disasm(0x124003, 0x125B6C, 5, 0x1F, 8, -1);
		koei_disasm(0x13E003, 0x13FAA8, 5, 0x1F, 8, -1);
		koei_disasm(0x156003, 0x157DD5, 5, 0x1F, -1, 0xE);
		koei_disasm(0x170C03, 0x171B99, 5, 0x1F, 0xD, -1);
		koei_disasm(0x188003, 0x189FFE, 5, 0x1F, -1, 0x10);
		koei_disasm(0x1A2000, 0x1A2543, 5, 0x1F, 0xF, -1);
		koei_disasm(0x1A2E12, 0x1A3329, 5, 0x1F, 0xF, -1);
		koei_disasm(0x1BA003, 0x1BB846, 5, 0x1F, -1, -1);
		koei_disasm(0x1D2003, 0x1D2CA6, 5, 0x1F, -1, -1);
		koei_disasm(0x206103, 0x2067E7, 5, 0x1F, -1, -1);
		koei_disasm(0x207D03, 0x207F6B, 5, 0x1F, -1, -1);
		koei_disasm(0x220003, 0x2207FA, 5, 0x1F, -1, -1);
		koei_disasm(0x220A83, 0x220C25, 5, 0x1F, -1, -1);
		koei_disasm(0x220D83, 0x220ECD, 5, 0x1F, -1, -1);
		koei_disasm(0x304003, 0x3055D0, 5, 0x1F, -1, -1);
		koei_disasm(0x322003, 0x322257, 5, 0x1F, -1, -1);
	} else if (md5=="0694CF9EBCDC9318EF67B45D0A44E2DF") {
		Message(" Start Sangokushi 2 PRG0 decompilation.\n");
		koei_disasm(0x00A003, 0x00BA44, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x023D35, 5, 0x1F, -1, -1);
		koei_disasm(0x03C003, 0x03DBF9, 5, 0x1F, 1, -1);	// bank 1 visible from banks 2-7
		koei_disasm(0x056003, 0x057D71, 5, 0x1F, 1, -1);
		koei_disasm(0x070003, 0x071F12, 5, 0x1F, 1, -1);
		koei_disasm(0x08A003, 0x08BAF4, 5, 0x1F, 1, -1);
		koei_disasm(0x0A4003, 0x0A5AE0, 5, 0x1F, 1, -1);
		koei_disasm(0x0BE003, 0x0BFA78, 5, 0x1F, 1, -1);
		koei_disasm(0x0D6003, 0x0D75D1, 5, 0x1F, -1, -1);
		koei_disasm(0x0F0003, 0x0F1C2B, 5, 0x1F, 8, -1);
		koei_disasm(0x10A003, 0x10BC3E, 5, 0x1F, 8, -1);
		koei_disasm(0x124003, 0x125AB7, 5, 0x1F, 8, -1);
		koei_disasm(0x13E003, 0x13FADE, 5, 0x1F, 8, -1);
		koei_disasm(0x156003, 0x157DB6, 5, 0x1F, -1, 0xE);
		koei_disasm(0x170A03, 0x171B50, 5, 0x1F, 0xD, -1);
		koei_disasm(0x188003, 0x189FFE, 5, 0x1F, -1, 0x10);
		koei_disasm(0x1A2000, 0x1A24F5, 5, 0x1F, 0xF, -1);
		koei_disasm(0x1A2D12, 0x1A31A7, 5, 0x1F, 0xF, -1);
		koei_disasm(0x1BA003, 0x1BB87D, 5, 0x1F, -1, -1);
		koei_disasm(0x1D2003, 0x1D2A8B, 5, 0x1F, -1, -1);
		koei_disasm(0x206103, 0x2067E9, 5, 0x1F, -1, -1);
		koei_disasm(0x207D03, 0x207F6B, 5, 0x1F, -1, -1);
		koei_disasm(0x304003, 0x30557A, 5, 0x1F, -1, -1);
		koei_disasm(0x322003, 0x322204, 5, 0x1F, -1, -1);
	} else if (md5=="872387896E3851B3C3DB69DCF8818F30") {
		Message(" Start Sangokushi 2 PRG1 decompilation.\n");
		koei_disasm(0x00A003, 0x00BA49, 5, 0x1F, -1, -1);
		koei_disasm(0x022003, 0x023D35, 5, 0x1F, -1, -1);
		koei_disasm(0x03C003, 0x03DC04, 5, 0x1F, 1, -1);	// bank 1 visible from banks 2-7
		koei_disasm(0x056003, 0x057D71, 5, 0x1F, 1, -1);
		koei_disasm(0x070003, 0x071F12, 5, 0x1F, 1, -1);
		koei_disasm(0x08A003, 0x08BAF4, 5, 0x1F, 1, -1);
		koei_disasm(0x0A4003, 0x0A5AE0, 5, 0x1F, 1, -1);
		koei_disasm(0x0BE003, 0x0BFA83, 5, 0x1F, 1, -1);
		koei_disasm(0x0D6003, 0x0D75F7, 5, 0x1F, -1, -1);
		koei_disasm(0x0F0003, 0x0F1C2B, 5, 0x1F, 8, -1);
		koei_disasm(0x10A003, 0x10BC3E, 5, 0x1F, 8, -1);
		koei_disasm(0x124003, 0x125B3A, 5, 0x1F, 8, -1);
		koei_disasm(0x13E003, 0x13FAE3, 5, 0x1F, 8, -1);
		koei_disasm(0x156003, 0x157DB6, 5, 0x1F, -1, 0xE);
		koei_disasm(0x170A03, 0x171B50, 5, 0x1F, 0xD, -1);
		koei_disasm(0x188003, 0x189FFE, 5, 0x1F, -1, 0x10);
		koei_disasm(0x1A2000, 0x1A24F9, 5, 0x1F, 0xF, -1);
		koei_disasm(0x1A2D12, 0x1A31A7, 5, 0x1F, 0xF, -1);
		koei_disasm(0x1BA003, 0x1BB87D, 5, 0x1F, -1, -1);
		koei_disasm(0x1D2003, 0x1D2A8B, 5, 0x1F, -1, -1);
		koei_disasm(0x206103, 0x2067E9, 5, 0x1F, -1, -1);
		koei_disasm(0x207D03, 0x207F6B, 5, 0x1F, -1, -1);
		koei_disasm(0x304003, 0x305582, 5, 0x1F, -1, -1);
		koei_disasm(0x322003, 0x32220F, 5, 0x1F, -1, -1);
	} else if (md5=="E0359CFF7EDFFF4C8BDF4C61D8451305") {
		Message(" Start Nobunaga no Yabou 3 decompilation.\n");
		koei_disasm(0x00A003, 0x00BC6A, 5, 0x3F, -1, -1);
		koei_disasm(0x024003, 0x024B0C, 5, 0x3F, -1, -1);
		koei_disasm(0x03C003, 0x03CFD4, 5, 0x3F, -1, -1);
		koei_disasm(0x054003, 0x054208, 5, 0x3F, -1, -1);
		koei_disasm(0x06C003, 0x06CE54, 5, 0x3F, -1, -1);
		koei_disasm(0x086003, 0x087DAD, 5, 0x3F, -1, -1);
		koei_disasm(0x0A0003, 0x0A0DC6, 5, 0x3F, -1, -1);
		koei_disasm(0x0B8003, 0x0B9CC9, 5, 0x3F, -1, -1);
		koei_disasm(0x0D2003, 0x0D361D, 5, 0x3F, -1, -1);
		koei_disasm(0x0EC003, 0x0EDCC9, 5, 0x3F, -1, -1);
		koei_disasm(0x106003, 0x107B14, 5, 0x3F, -1, -1);
		koei_disasm(0x120003, 0x121845, 5, 0x3F, -1, -1);
		koei_disasm(0x13A003, 0x13BF66, 5, 0x3F, -1, -1);
		koei_disasm(0x154003, 0x155E9A, 5, 0x3F, -1, -1);
		koei_disasm(0x16C003, 0x16CFED, 5, 0x3F, -1, -1);
		koei_disasm(0x184003, 0x1855B3, 5, 0x3F, -1, -1);
		koei_disasm(0x19C003, 0x19D383, 5, 0x3F, -1, -1);
		koei_disasm(0x1B6003, 0x1B7AEC, 5, 0x3F, -1, -1);
		koei_disasm(0x1D0003, 0x1D19CB, 5, 0x3F, -1, -1);
		koei_disasm(0x1EA003, 0x1EB4DF, 5, 0x3F, -1, -1);
		koei_disasm(0x204003, 0x204C42, 5, 0x3F, -1, -1);
		koei_disasm(0x21C003, 0x21C809, 5, 0x3F, -1, -1);
		koei_disasm(0x236003, 0x236BE8, 5, 0x3F, -1, -1);
		koei_disasm(0x250003, 0x25042E, 5, 0x3F, -1, -1);
		koei_disasm(0x26A003, 0x26AC38, 5, 0x3F, -1, -1);
		koei_disasm(0x285D08, 0x285E52, 5, 0x3F, -1, -1);
		koei_disasm(0x29E003, 0x29FBEC, 5, 0x3F, -1, -1);
		koei_disasm(0x2B86A2, 0x2B9C1B, 5, 0x3F, -1, -1);
		koei_disasm(0x2D2003, 0x2D2190, 5, 0x3F, -1, -1);
		koei_disasm(0x2D2959, 0x2D3BAC, 5, 0x3F, -1, -1);
		koei_disasm(0x2EC003, 0x2ED509, 5, 0x3F, -1, -1);
		koei_disasm(0x306003, 0x306BAF, 5, 0x3F, -1, -1);
		koei_disasm(0x5C4003, 0x5C44DD, 5, 0x3F, -1, -1);
		koei_disasm(0x62A003, 0x62A185, 5, 0x3F, -1, -1);
		koei_disasm(0x62A552, 0x62B553, 5, 0x3F, -1, -1);
		koei_disasm(0x62B7F9, 0x62BF47, 5, 0x3F, -1, -1);
	} else if(md5 == "2C9B111B2CC753657EDE3D48E4253355") {
		Message(" Start Aoki Ookami 2 decompilation.\n");
		koei_disasm(0x06C003, 0x06CEE8, 5, 0x3F, -1, -1);
		koei_disasm(0x086003, 0x087ADB, 5, 0x3F, -1, -1);
		koei_disasm(0x0A0003, 0x0A10B8, 5, 0x3F, -1, -1);
		koei_disasm(0x0BA2F2, 0x0BBE05, 5, 0x3F, -1, -1);
		koei_disasm(0x0EC003, 0x0EC6EA, 5, 0x3F, -1, -1);
		koei_disasm(0x10652E, 0x106DD9, 5, 0x3F, -1, -1);
		koei_disasm(0x120003, 0x121130, 5, 0x3F, -1, -1);
		koei_disasm(0x13A003, 0x13ACB7, 5, 0x3F, -1, -1);
		koei_disasm(0x1549D9, 0x155F10, 5, 0x3F, -1, -1);
		koei_disasm(0x16E003, 0x16FAF7, 5, 0x3F, -1, -1);
		koei_disasm(0x1A0003, 0x1A1368, 5, 0x3F, -1, -1);
		koei_disasm(0x1BA003, 0x1BAF96, 5, 0x3F, -1, -1);
		koei_disasm(0x1D4003, 0x1D5212, 5, 0x3F, -1, -1);
		koei_disasm(0x1EE003, 0x1EEEAA, 5, 0x3F, -1, -1);
		koei_disasm(0x208003, 0x208E99, 5, 0x3F, -1, -1);
		koei_disasm(0x222003, 0x2236E6, 5, 0x3F, -1, -1);
		koei_disasm(0x23C003, 0x23D391, 5, 0x3F, -1, -1);
		koei_disasm(0x256003, 0x2579F6, 5, 0x3F, -1, -1);
		koei_disasm(0x270003, 0x270EA5, 5, 0x3F, -1, -1);
		koei_disasm(0x28A003, 0x28BCA0, 5, 0x3F, -1, -1);
		koei_disasm(0x2A4003, 0x2A55F7, 5, 0x3F, -1, -1);
		koei_disasm(0x2BE003, 0x2BF8B7, 5, 0x3F, -1, -1);
		koei_disasm(0x2D8003, 0x2D96E4, 5, 0x3F, -1, -1);
		koei_disasm(0x2F0003, 0x2F1645, 5, 0x3F, -1, -1);
		koei_disasm(0x30A003, 0x30AE1D, 5, 0x3F, -1, -1);
		koei_disasm(0x4CE003, 0x4CF45A, 5, 0x3F, -1, -1);
		koei_disasm(0x4E6008, 0x4E7A98, 5, 0x3F, -1, -1);
		koei_disasm(0x500003, 0x50181B, 5, 0x3F, -1, -1);
		koei_disasm(0x51A003, 0x51B8E6, 5, 0x3F, -1, -1);
		koei_disasm(0x534003, 0x5352B6, 5, 0x3F, -1, -1);
		koei_disasm(0x568003, 0x568C3E, 5, 0x3F, -1, -1);
		koei_disasm(0x5694D8, 0x569C45, 5, 0x3F, -1, -1);
		koei_disasm(0x582003, 0x58269E, 5, 0x3F, -1, -1);
		koei_disasm(0x632003, 0x632FC9, 5, 0x3F, -1, -1);
		koei_disasm(0x63322F, 0x633662, 5, 0x3F, -1, -1);
	}
	Message(" Done.\n");
#endif
}