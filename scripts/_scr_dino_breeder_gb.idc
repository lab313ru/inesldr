#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// DINO BREEDER GB SCRIPTS
// ---------------------------------

// GetReg(ea,"ds")

// 0 init charset [byte][ofs]
// 1 init output0 [byte][byte]
// 2 init mask    [byte]
// 3 init output1 [byte][byte]
// 5 stop
// 6 wait flag    [byte]
// C delay		  [byte]
// 8 flush
// 9 sync
// B call		  [ofs]
// 5C

static dd_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "init_charset";
	} else if(cmd == 0x1) {
		ret = "init_output0";
	} else if(cmd == 0x2) {
		ret = "init_mask";
	} else if(cmd == 0x3) {
		ret = "init_output1";
	} else if(cmd == 0x5) {
		ret = "stop";
	} else if(cmd == 0x6) {
		ret = "wait_flag";
	} else if(cmd == 0x8) {
		ret = "flush";
	} else if(cmd == 0x9) {
		ret = "sync";
	} else if(cmd == 0xB) {
		ret = "call";
	} else if(cmd == 0xC) {
		ret = "delay";
	} else if(cmd == 0x5C) {
		ret = "cmd5C";
	} else {
		ret = "unk";
	}
	return ret;
}


static exec_recursive(ea, bank) {
	auto ptr = Word(ea);
	auto base = MK_FP(AskSelector(bank), 0);
	MakeUnknown(ea, 2, DOUNK_SIMPLE);
	MakeData(ea, FF_WORD, 1, 0);
	OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
	dd_opcode(base + ptr);
	return ea+2;
}

static make_offset_gb_custom(ea, bank, exec) {
	auto tmp1 = Word(ea), ret;
	if(exec) {
		if(tmp1>=0xFF80)
			ret = make_offset_gb(ea, SegByName("ZRAM"));
		else if(tmp1>=0xFF00)
			ret = make_offset_gb(ea, SegByName("HWREGS"));
		else if(tmp1>=0xD000)
			ret = make_offset_gb(ea, SegByName("RAMB"));
		else if(tmp1>=0xC000)
			ret = make_offset_gb(ea, SegByName("RAM0"));
		else if(tmp1>=0x4000)
			ret = exec_recursive(ea, bank);
		else
			ret = exec_recursive(ea, 0);
	}
	else {
		if(tmp1>=0xFF80)
			ret = make_offset_gb(ea, SegByName("ZRAM"));
		else if(tmp1>=0xFF00)
			ret = make_offset_gb(ea, SegByName("HWREGS"));
		else if(tmp1>=0xD000)
			ret = make_offset_gb(ea, SegByName("RAMB"));
		else if(tmp1>=0xC000)
			ret = make_offset_gb(ea, SegByName("RAM0"));
		else if(tmp1>=0x4000)
			ret = make_offset_gb(ea, bank);
		else
			ret = make_offset_gb(ea, 0);
	}
	return ret;
}

static dd_opcode(ea) {
	auto cmd_opc, cmd, bank, char_d, tmp0, tmp1, tmp2;
	bank = GetReg(ea,"ds");
	tmp1 = 0;
	do {
		if(ea != BADADDR) {
//			Message(" start at ea = 0x%08x!\n", ea);
			cmd = Byte(ea);
			cmd_opc = dd_opcode_name(cmd);
			if(cmd_opc!="unk") {
				if(tmp1) {
					ea = make_data_array(tmp2, tmp1, "");
					tmp1 = 0;
				}
				if((cmd == 0x00)) {	// init charset
					tmp0=Byte(ea+1);
					ea = make_data_array(ea, 2, cmd_opc);
					ea = make_offset_gb_custom(ea, tmp0, 0);	// no exec
				} else if((cmd==0x01)||(cmd==0x03)) {	// b,b
					ea = make_data_array(ea, 3, cmd_opc);
				} else if((cmd==0x05)) {	// stop
					ea = make_data_array(ea, 1, cmd_opc);
				} else if((cmd==0x09)) {
					ea = make_data_array(ea, 1, cmd_opc);
				} else if((cmd==0x02)||(cmd==0x06)||(cmd==0x08)||(cmd==0x0C)||(cmd==0x5C)) {	// 1b
					ea = make_data_array(ea, 2, cmd_opc);
				} else if((cmd==0x0B)) {	// call ofs
					ea = make_data_array(ea, 1, cmd_opc);
					ea = make_offset_gb_custom(ea, bank, 1);	// exec
				} else {
					Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
				}
			} else {
				if(tmp1==0)
					tmp2 = ea;
				tmp1++;
				ea++;
			}
		}
//		return ea;
	} while (cmd != 0x05);
	return ea;
}

static dd_script_selection(void) {
//	auto ea = -1, seg, base, ofs;
//	while ((ea=FindBinary(ea+1,SEARCH_DOWN,"21 ?? ?? CD 4D 0C")) != BADADDR) {	// 1
//	while ((ea=FindBinary(ea+1,SEARCH_DOWN,"21 ?? ?? CD A9 0C")) != BADADDR) {	// 2
//		seg = GetReg(ea,"ds");
//		ofs = Word(ea+1);
//		base = MK_FP(AskSelector(seg), 0);
//		dd_opcode(base+ofs);
//	}
	Jump(dd_opcode(ScreenEA()));
}

static main(void) {
	AddHotkey("Shift-O", "dd_script_selection");
}
