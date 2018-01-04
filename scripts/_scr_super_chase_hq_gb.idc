#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// SUPER CHASE HQ GB SCRIPT CONVERT
// ---------------------------------

static schasehq_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "stop";
	} else if(cmd == 0x02) {
		ret = "res_lib_load";
	} else if(cmd == 0x04) {
		ret = "res_lib_ofs_load";
	} else if(cmd == 0x06) {
		ret = "delay_no_skip";
	} else if(cmd == 0x08) {
		ret = "delay_with_skip";
	} else if(cmd == 0x0A) {
		ret = "call";
	} else if(cmd == 0x0C) {
		ret = "main_sub_set";
	} else if(cmd == 0x0E) {
		ret = "fade_in";
	} else if(cmd == 0x10) {
		ret = "unk";
	} else if(cmd == 0x12) {
		ret = "unk";
	} else if(cmd == 0x14) {
		ret = "num_display";
	} else if(cmd == 0x16) {
		ret = "unk";
	} else if(cmd == 0x18) {
		ret = "unk";
	} else if(cmd == 0x1A) {
		ret = "unk";
	} else if(cmd == 0x1C) {
		ret = "set";
	} else if(cmd == 0x1E) {
		ret = "jmp";
	} else if(cmd == 0x20) {
		ret = "switch";
	} else if(cmd == 0x22) {
		ret = "beq";
	} else if(cmd == 0x24) {
		ret = "unk";
	} else if(cmd == 0x26) {
		ret = "unk";
	} else if(cmd == 0x28) {
		ret = "unk";
	} else if(cmd == 0x2A) {
		ret = "unk";
	}
	return ret;
}

static schasehq_opcode(ea) {
	auto cmd_opc = "", cmd, tmp0, tmp1, tmp2;
	if(ea != BADADDR) {
//		Message(" start at ea = 0x%08x!\n", ea);
		cmd = Byte(ea);
		cmd_opc = schasehq_opcode_name(cmd);

		if(cmd == 0) {																	// break, no args
			ea = make_data_array(ea, 1, cmd_opc);
		} else if((cmd==0x02)||(cmd==0x06)||(cmd==0x26)) {								// 1b
			ea = make_data_array(ea, 2, cmd_opc);
		} else if((cmd==0x24)||(cmd==0x10)) {											// 7b
			ea = make_data_array(ea, 8, cmd_opc);
		} else if((cmd==0x1C)) {														// 1b, 1ptr ram
			tmp0 = Byte(ea+1)|(Byte(ea+5)<<8);
			tmp1 = Word(ea+2);
			tmp2 = Word(ea+6);
			if(((tmp2-tmp1)==1)&&(tmp0>=0x4000)&&(tmp0<0x8000)&&((tmp1&1)==0)&&(tmp1>=0xC1A0)&&(tmp1<0xC1B4)){
				auto base = MK_FP(AskSelector(1), 0);
				ea = make_data_array(ea, 1, cmd_opc);
				make_data_array(ea, 1, "");
				if(base != BADADDR) OpOffEx(ea, 0, REF_LOW8, base+tmp0, base, 0);
				ea++;
				ea = make_offset_gb(ea, 0xC);
				ea = make_data_array(ea, 1, cmd_opc);
				make_data_array(ea, 1, "");
				if(base != BADADDR)	OpOffEx(ea, 0, REF_HIGH8, base+tmp0, base, 0);
				ea++;
				ea = make_offset_gb(ea, 0xC);
			} else {
				ea = make_data_array(ea, 2, cmd_opc);
				if(tmp1>=0xFF80)
					ea = make_offset_gb(ea, 0x10);
				else if(tmp1>=0xFF00)
					ea = make_offset_gb(ea, 0xF);
				else if(tmp1>=0xD000)
					ea = make_offset_gb(ea, 0xD);
				else if(tmp1>=0xC000)
					ea = make_offset_gb(ea, 0xC);
			}
		} else if((cmd==4)) {															// 3b
			ea = make_data_array(ea, 4, cmd_opc);
		} else if((cmd==0xA)||(cmd==0xC)||(cmd==0x1E)) {								// 1ptr code
			ea = make_data_array(ea, 1, cmd_opc);
			tmp1 = Word(ea);
			if(tmp1<0x4000)
				ea = make_offset_gb(ea, 0);
			else if(tmp1<0x8000)
				ea = make_offset_gb(ea, 1);
//			if((cmd==0x1E)||(cmd==0xC))
//				ea = ea - 3;
		} else if(cmd==8) {																// 3b
			ea = make_data_array(ea, 3, cmd_opc);
		} else if(cmd==0x20) {															// switch 1ptr ram, 1ptr code
			ea = make_data_array(ea, 1, cmd_opc);
			tmp1 = Word(ea);
			if(tmp1>=0xFF80)
				ea = make_offset_gb(ea, 0x10);
			else if(tmp1>=0xD000)
				ea = make_offset_gb(ea, 0xD);
			else if(tmp1>=0xC000)
				ea = make_offset_gb(ea, 0xC);
			ea = make_offset_gb(ea, 1);
//			ea = ea - 5;
		} else if(cmd==0x12) {															// 1w, zero term string
			ea = make_data_array(ea, 3, cmd_opc);
			tmp0 = 0;
			while(Byte(ea+tmp0)!=0)tmp0++;
			MakeStr(ea,ea+tmp0+1);
			ea=ea+tmp0+1;
		} else if((cmd==0x22)) {														// 1b, 1ptr ram, 1ptr code
			ea = make_data_array(ea, 2, cmd_opc);
			tmp1 = Word(ea);
			if(tmp1>=0xFF80)
				ea = make_offset_gb(ea, 0x10);
			else if(tmp1>=0xD000)
				ea = make_offset_gb(ea, 0xD);
			else if(tmp1>=0xC000)
				ea = make_offset_gb(ea, 0xC);
			tmp1 = Word(ea);
			ea = make_offset_gb(ea, 1);
		} else if((cmd==0x14)) {														// 2b, 1ptr ram, 1b
			ea = make_data_array(ea, 3, cmd_opc);
			tmp1 = Word(ea);
			if(tmp1>=0xFF80)
				ea = make_offset_gb(ea, 0x10);
			else if(tmp1>=0xD000)
				ea = make_offset_gb(ea, 0xD);
			else if(tmp1>=0xC000)
				ea = make_offset_gb(ea, 0xC);
			ea = make_data_array(ea, 1, "");
		} else {
			Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
		}
		return ea;
	}
}

static schasehq_script_selection(void) {
	Jump(schasehq_opcode(ScreenEA()));
}

static main(void) {
	AddHotkey("Shift-X", "schasehq_script_selection");
}
