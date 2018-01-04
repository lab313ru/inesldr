#include <idc.idc>
#include <functions.idc>

// ----------------------------
// DOUBLE DRAGON snd script
// ----------------------------

static dd_snd_script_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0xC9) {
		ret = "sleep";
	} else if(cmd == 0xCD) {
		ret = "load";
	} else if(cmd == 0xCE) {
		ret = "loop1";
	} else if(cmd == 0xCF) {
		ret = "loop2";
	} else if(cmd == 0xD0) {
		ret = "jmp";
	} else if(cmd == 0xD1) {
		ret = "jsr";
	} else if(cmd == 0xD2) {
		ret = "ret";
	} else if(cmd == 0xD3) {
		ret = "stop";
	} else
		ret = form("cmd_%02x", cmd);
	return ret;
}

static dd_snd_script_parse(void) {
	auto ea = ScreenEA();
//	ea = FindUnexplored(ea - 1, SEARCH_DOWN);
	auto cmd, sp = 0, stop = 0;
	auto jmp_ptr, jmp_base;
	auto seg = GetReg(ea,"ds") - 1;
	do {
		cmd = Byte(ea);
		if(cmd < 0x40) { 	// 00-3F
			ea = make_data_array(ea,1,"data0");
		} else if(cmd < 0xC0) { 	// 40-BF
			if(cmd & 0x80)
				ea = make_data_array(ea,2,"data2");
			else
				ea = make_data_array(ea,1,"data1");
		} else {		 	// C0-D5
			if(((cmd >= 0xC9)&&(cmd <= 0xCC))||(cmd == 0xD3)||(cmd == 0xD5)) {// no args
				ea = make_data_array(ea,1,dd_snd_script_opcode_name(cmd));
			} else if(((cmd >= 0xC0)&&(cmd <= 0xC8))||(cmd == 0xD4)) {	// 1 byte arg
				ea = make_data_array(ea,2,dd_snd_script_opcode_name(cmd));
			} else if(cmd == 0xCD) {	// 2 byte arg load with ret
				ea = make_data_array(ea,1,dd_snd_script_opcode_name(cmd));
				jmp_ptr = Word(ea) + MK_FP(AskSelector(seg + 1), 0);
				ea = make_offset(ea,seg);
				auto tmp = Word(jmp_ptr);
				if((tmp >= 0x8000) && (tmp < 0xC000)) {
					jmp_ptr = make_offset(jmp_ptr,seg);
					jmp_ptr = make_data_array(jmp_ptr,2,"chnl cmd");
				} else {
					jmp_ptr = make_data_array(jmp_ptr,4,"chnl cmd");
				}
			} else if((cmd == 0xCE)||(cmd == 0xCF)) {	// 3 byte loop
				ea = make_data_array(ea,2,dd_snd_script_opcode_name(cmd));
				ea = make_offset(ea,seg);
			} else if(cmd == 0xD0) {		// 2 byte jmp
				ea = make_data_array(ea,1,dd_snd_script_opcode_name(cmd));
				ea = make_offset(ea,seg);
			} else if(cmd == 0xD1) {		// 2 byte jsr
				ea = make_data_array(ea,1,dd_snd_script_opcode_name(cmd));
				jmp_ptr = Word(ea) + MK_FP(AskSelector(seg + 1), 0);
				ea = make_offset(ea,seg);
				if(sp == 0) {
					sp = ea;
					ea = jmp_ptr;
				} else {
					Message("can't jump inside subroutine at 0x%08x!\n",ea);
					stop = 1;
				}
			} else if(cmd == 0xD2) {		// ret
				ea = make_data_array(ea,1,dd_snd_script_opcode_name(cmd));
				ea = sp;
				sp = 0;
			} else {
				Message(" unk_cmd at 0x%08x\n",ea);
				stop = 1;
			}
		}
	} while ((sp != 0)&&(stop == 0));
	Jump(ea);
}

static dd_snd_lib_parse(void) {
	auto ea = ScreenEA();
	auto i, ptr_ea, ptr_ofs;
	auto seg = GetReg(ea,"ds") - 1;
	if(ea != BADADDR) {
		ptr_ea = Word(ea) + MK_FP(AskSelector(seg + 1), 0);
		auto ptr_name = Name(ptr_ea);
		ea = make_offset(ea, seg);
		ptr_ea = make_data_array(ptr_ea,1,"");
		auto chnl = Byte(ptr_ea);
		while (chnl != 0xFF) {
			ptr_ea = make_data_array(ptr_ea,1,"");
			ptr_ofs = Word(ptr_ea) + MK_FP(AskSelector(seg + 1), 0);
			auto new_name = form("%s_ch%d",ptr_name,chnl);
			MakeName(ptr_ofs, new_name);
			ptr_ea = make_offset(ptr_ea,seg);
			chnl = Byte(ptr_ea);
		};
		ptr_ea = make_data_array(ptr_ea,1,"");
	}
	Jump(ea);
}

static dd_spr_lib_parse(void) {
	auto ea = ScreenEA();
	ea = FindUnexplored(ea - 1, SEARCH_DOWN);
	auto i, ptr_ea, ptr_ofs, sp = 0, stop = 0;
	auto seg = GetReg(ea,"ds") - 1;
	auto segbase = MK_FP(AskSelector(seg + 1), 0);
	if(ea != BADADDR) {
			auto ofs = Word(ea);
			ptr_ea = ofs + segbase;
			ea = make_offset(ea, seg);
		if(ofs != 0xC000) {
			auto cmd = Byte(ptr_ea);
			while ((cmd || sp) && !stop) {
				if(cmd == 0xFF) {
					ptr_ea = make_data_array(ptr_ea, 1, "jsr");
					ptr_ofs = Word(ptr_ea) + segbase;
					ptr_ea = make_offset(ptr_ea, seg);
					sp = ptr_ea;
					ptr_ea = ptr_ofs;
				} else if(cmd == 0xFE) {
					ptr_ea = make_data_array(ptr_ea, 2, "tbl0");
					ptr_ea = make_offset(ptr_ea, seg);
					ptr_ea = make_offset(ptr_ea, seg);
					ptr_ea = make_offset(ptr_ea, seg);
				} else if(cmd & 0x80) {
					ptr_ea = make_data_array(ptr_ea, 1, "cond jmp");
					ptr_ofs = Word(ptr_ea) + segbase;
					ptr_ea = make_offset(ptr_ea, seg);
				} else if(cmd == 0x00) {
					if(sp) {
						ptr_ea = make_data_array(ptr_ea, 1, "");
						ptr_ea = sp;
						sp = 0;
					}
				} else {
					ptr_ea = make_data_array(ptr_ea, 2, "tbl1");
					ptr_ea = make_offset(ptr_ea, seg);
					ptr_ea = make_offset(ptr_ea, seg);
				}
				cmd = Byte(ptr_ea);
			};
			ptr_ea = make_data_array(ptr_ea, 1, "");
		}
	}
	Jump(ea);
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
