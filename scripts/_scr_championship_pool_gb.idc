#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// CHAMPIONSHIP POOL GB SCRIPT CONVERT
// ---------------------------------

static cpool_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "exit";
	} else if(cmd == 0x01) {
		ret = "call";
//	} else if(cmd == 0x02) {
//		ret = "unk";
	} else if(cmd == 0x03) {
		ret = "set8";
	} else if(cmd == 0x05) {
		ret = "set16";
	} else if(cmd == 0x07) {
		ret = "move";
	} else if(cmd == 0x08) {
		ret = "tlm_move";
	} else if(cmd == 0x09) {
		ret = "chr_move";
	} else if(cmd == 0x0E) {
		ret = "tlm_rle_prg3";
	} else if(cmd == 0x10) {
		ret = "tlm_raw_prg3";
//	} else if(cmd == 0x12) {
//		ret = "unk";
	} else if(cmd == 0x13) {
		ret = "nmi_on";
	} else if(cmd == 0x14) {
		ret = "nmi_off";
//	} else if(cmd == 0x15) {
//		ret = "unk";
//	} else if(cmd == 0x18) {
//		ret = "unk";
//	} else if(cmd == 0x19) {
//		ret = "unk";
//	} else if(cmd == 0x1D) {
//		ret = "unk";
	} else if(cmd == 0x1C) {
		ret = "mem_set";
//	} else if(cmd == 0x1E) {
//		ret = "unk";
	} else {
		ret = form("cmd%02X",cmd);
	}
	return ret;
}

static cpool_opcode(ea) {
	auto cmd_opc = "", cmd, tmp0, tmp1;
	if(ea != BADADDR) {
//		Message(" start at ea = 0x%08x!\n", ea);
		cmd = Byte(ea);
		cmd_opc = cpool_opcode_name(cmd);

		if(((cmd>=0x13)&&(cmd<=0x17))||(cmd==0)||(cmd==0x1D)||(cmd==0x1E)) {							// no args
			ea = make_data_array(ea, 1, cmd_opc);
		} else if((cmd==0x12)||(cmd==0x18)||(cmd==0x19)||(cmd==0x02)) {	// 1 arg
			ea = make_data_array(ea, 2, cmd_opc);
		} else if((cmd==0x1C)) {														// 2 arg
			ea = make_data_array(ea, 3, cmd_opc);
		} else if((cmd==1)||(cmd==8)||(cmd==9)) {										// ptr, prg
			ea = make_data_array(ea, 1, cmd_opc);
			tmp0 = Byte(ea+2);
			ea = make_offset_gb(ea, tmp0);
			ea = make_data_array(ea, 1, "");
		} else if((cmd==0xE)||(cmd==0x10)) {											// ptr, val8 [prg3]
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_gb(ea, 3);
			ea = make_data_array(ea, 1, "");
		} else if(cmd==3) {																// ptr, val8
			ea = make_data_array(ea, 1, cmd_opc);
			tmp1 = Word(ea);
			if(tmp1>=0xD000)
				ea = make_offset_gb(ea, 0xD);
			else if(tmp1>=0xC000)
				ea = make_offset_gb(ea, 0xC);
			ea = make_data_array(ea, 1, "");
		} else if(cmd==5) {																// ptr, val16
			ea = make_data_array(ea, 1, cmd_opc);
			tmp1 = Word(ea);
			if(tmp1>=0xD000)
				ea = make_offset_gb(ea, 0xD);
			else if(tmp1>=0xC000)
				ea = make_offset_gb(ea, 0xC);
			tmp1 = Word(ea);
			if(tmp1>=0x4000)
				ea = make_offset_gb(ea, 5);
			else
				ea = make_data_array(ea, 2, "");
		} else if(cmd==7) {																// ptr, dst, prg
			ea = make_data_array(ea, 1, cmd_opc);
			tmp0 = Byte(ea+4);
			ea = make_offset_gb(ea, tmp0);
			tmp1 = Word(ea);
			if(tmp1>=0xD000)
				ea = make_offset_gb(ea, 0xD);
			else if(tmp1>=0xC000)
				ea = make_offset_gb(ea, 0xC);
			else if(tmp1>=0x9C00)
				ea = make_offset_gb(ea, 0xA);
			else if(tmp1>=0x9800)
				ea = make_offset_gb(ea, 0x9);
			else if(tmp1>=0x8000)
				ea = make_offset_gb(ea, 0x8);
			ea = make_data_array(ea, 1, "");
		} else {
			Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
		}
		return ea;
	}
}

static cpool_script_selection(void) {
	Jump(cpool_opcode(ScreenEA()));
}

static main(void) {
	AddHotkey("Shift-X", "cpool_script_selection");
}
