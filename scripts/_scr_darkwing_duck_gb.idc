#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// DARKWING DUCK GB SCRIPTS
// ---------------------------------

// GetReg(ea,"ds")

static dd_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "stop";
	} else if(cmd == 0x1) {
		ret = "flush_pc";
	} else if(cmd == 0x2) {
		ret = "delay";
	} else if(cmd == 0x3) {
		ret = "ejsrA";
	} else if(cmd == 0x4) {
		ret = "set_main_sub";
	} else if(cmd == 0x5) {
		ret = "new_thread";
	} else if(cmd == 0x6) {
		ret = "ejsrB";
	} else if(cmd == 0x7) {
		ret = "jmp";
	} else if(cmd == 0x8) {
		ret = "jsr";
	} else if(cmd == 0x9) {
		ret = "ret";
	} else if(cmd == 0xA) {
		ret = "break";
	} else if(cmd == 0xB) {
		ret = "loopA_init";
	} else if(cmd == 0xC) {
		ret = "loopB_init";
	} else if(cmd == 0xD) {
		ret = "do_loopA";
	} else if(cmd == 0xE) {
		ret = "do_loopB";
	} else if(cmd == 0xF) {
		ret = "beq";
	} else if(cmd == 0x10) {
		ret = "bcs";
	} else if(cmd == 0x11) {
		ret = "jmp_if_pad_test";
	} else if(cmd == 0x12) {
		ret = "switch";
	} else if(cmd == 0x13) {
		ret = "set_default_branch";
	} else if(cmd == 0x14) {
		ret = "do_default_branch";
	} else if(cmd == 0x15) {
		ret = "memset";
	} else if(cmd == 0x1B) {
		ret = "thread_on";
	} else if(cmd == 0x1C) {
		ret = "thread_off";
	} else if(cmd == 0x39) {
		ret = "printf";
	} else if(cmd == 0x3A) {
		ret = "rect";
	} else if(cmd == 0x3C) {
		ret = "cursor_display";
	} else if(cmd == 0x3E) {
		ret = "pad_read_LR";
	} else if(cmd == 0x3F) {
		ret = "pad_read_UD";
	} else if(cmd == 0x40) {
		ret = "pad_test_AB";
	} else if(cmd == 0x43) {
		ret = "nop";
	} else if(cmd == 0x44) {
		ret = "nop";
	} else if(cmd == 0x45) {
		ret = "nop";
	} else if(cmd == 0x48) {
		ret = "type_set";
	} else if(cmd == 0x49) {
		ret = "type_wait";
	} else {
		ret = "unk";
	}
	return ret;
}

static make_offset_gb_custom(ea, bank, exec) {
	auto tmp1 = Word(ea), ret;
	if(exec) {
		if(tmp1>=0xFF80)
			ret = make_offset_gb_code(ea, 0x10);
		else if(tmp1>=0xFF00)
			ret = make_offset_gb(ea, 0xF);
		else if(tmp1>=0xD000)
			ret = make_offset_gb_code(ea, 0xD);
		else if(tmp1>=0xC000)
			ret = make_offset_gb_code(ea, 0xC);
		else if(tmp1>=0x4000)
			ret = make_offset_gb_code(ea, bank);
		else
			ret = make_offset_gb_code(ea, 0);
	}
	else {
		if(tmp1>=0xFF80)
			ret = make_offset_gb(ea, 0x10);
		else if(tmp1>=0xFF00)
			ret = make_offset_gb(ea, 0xF);
		else if(tmp1>=0xD000)
			ret = make_offset_gb(ea, 0xD);
		else if(tmp1>=0xC000)
			ret = make_offset_gb(ea, 0xC);
		else if(tmp1>=0x4000)
			ret = make_offset_gb(ea, bank);
		else
			ret = make_offset_gb(ea, 0);
	}
	return ret;
}

static make_txt_res(ea, bank) {
	auto ofs = Word(ea), cmd, base, ptr, len;
	if(ofs<0x4000) {
		base = 0;
		ptr = ofs;
	} else {
		base = MK_FP(AskSelector(bank), 0);
		ptr = base+ofs;
	}
	while((cmd=Byte(ptr))!=0xFF) {
		len = Byte(ptr+2);
		ptr = make_data_array(ptr, 3+len, "");
	}
	make_data_array(ptr,1,"");
}

static make_rect_res(ea, bank) {
	auto ofs = Word(ea), cmd, base, ptr, w_, h_;
	if(ofs<0x4000) {
		base = 0;
		ptr = ofs;
	} else {
		base = MK_FP(AskSelector(bank), 0);
		ptr = base+ofs;
	}
	w_ = Byte(ptr);
	h_ = Byte(ptr+1);
	ptr = make_data_array(ptr, 2, "");
	if(w_&0x80) w_=1;
	if(h_&0x80) {
		ptr = make_data_array(ptr, w_, "");
	} else {
		while(h_!=0) {
			ptr = make_data_array(ptr, w_, "");
			h_ = h_ - 1;
		}
	}
}

static make_type_res(ea, bank) {
	auto ofs = Word(ea), cmd, base, ptr, i;
	if(ofs<0x4000) {
		base = 0;
		ptr = ofs;
	} else {
		base = MK_FP(AskSelector(bank), 0);
		ptr = base+ofs;
	}
	i = 0;
	do {
		cmd = Byte(ptr+i);
		i=i+1;
		if(cmd==0x7E) {
			ptr = make_data_array(ptr, i, "");
			i=0;
		}
	} while(cmd!=0x7F);
	ptr = make_data_array(ptr, i, "");
}

static dd_opcode(ea) {
	auto cmd_opc = "", cmd, bank, tmp0, tmp1, tmp2;
	bank = GetReg(ea,"ds");
	if(ea != BADADDR) {
//		Message(" start at ea = 0x%08x!\n", ea);
		cmd = Byte(ea);

// FOR DUCK TALES 2
		if(cmd>=0x14)
			cmd++;
// ----------------

		cmd_opc = dd_opcode_name(cmd);
		
		if((cmd == 0)||(cmd == 0x9)) {	// ret/exit
			make_data_array(ea, 1, cmd_opc);
			// stop here
		} else if((cmd==1)||(cmd==0xB)||(cmd==0xC)||(cmd==0x14)||(cmd==0x18)||(cmd==0x19)||(cmd==0x1A)||(cmd==0x1B)||(cmd==0x1C)||(cmd==0x22)||(cmd==0x23)||(cmd==0x24)||(cmd==0x25)||(cmd==0x26)||(cmd==0x27)||(cmd==0x2E)||(cmd==0x2F)||(cmd==0x30)||(cmd==0x31)||(cmd==0x40)||(cmd==0x43)||(cmd==0x44)||(cmd==0x45)||(cmd==0x49)) {	// no args
			ea = make_data_array(ea, 1, cmd_opc);
		} else if((cmd==2)||(cmd==0x17)||(cmd==0x1F)||(cmd==0x20)||(cmd==0x21)||(cmd==0x2A)||(cmd==0x2B)||(cmd==0x2C)||(cmd==0x2D)||(cmd==0x32)||(cmd==0x34)||(cmd==0x36)||(cmd==0x37)||(cmd==0x3E)||(cmd==0x3F)) {	// 1b arg
			ea = make_data_array(ea, 2, cmd_opc);
		} else if((cmd==3)||(cmd==6)) {	// call extarnal ofs
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_gb_custom(ea, bank, 1);	// with exec
		} else if((cmd==4)) {	// main sub switch
			make_data_array(ea, 2, cmd_opc);
			// stop here
		} else if((cmd==7)||(cmd==0x8)||(cmd==0xA)||(cmd==0xF)||(cmd==0x10)||(cmd==0x13)||(cmd==0x3C)) {	// ofs
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_gb_custom(ea, bank, 0);	// no exec
		} else if((cmd==0x12)) {	// switch
			ea = make_data_array(ea, 1, cmd_opc);
			make_offset_gb_custom(ea, bank, 0);	// no exec
			// stop here
		} else if((cmd==0xD)||(cmd==0xE)||(cmd==0x5)) {	// 2b args, ofs
			ea = make_data_array(ea, 2, cmd_opc);
			ea = make_offset_gb_custom(ea, bank, 0);	// no exec
		} else if((cmd==0x15)||(cmd==0x11)) {	// 3b args, ofs
			ea = make_data_array(ea, 3, cmd_opc);
			ea = make_offset_gb_custom(ea, bank, 0);	// no exec
		} else if((cmd==0x39)) {	// printf arg ofs
			ea = make_data_array(ea, 1, cmd_opc);
			make_txt_res(ea, bank);
			ea = make_offset_gb_custom(ea, bank, 0);	// no exec
		} else if((cmd==0x3A)) {	// regt ofs ofs
			ea = make_data_array(ea, 3, cmd_opc);
			make_rect_res(ea, bank);
			ea = make_offset_gb_custom(ea, bank, 0);	// no exec
		} else if((cmd==0x47)||(cmd==0x28)||(cmd==0x29)||(cmd==0x35)||(cmd==0x4B)) {	// 2b arg
			ea = make_data_array(ea, 3, cmd_opc);
		} else if((cmd==0x48)) {	// type setup
			ea = make_data_array(ea, 4, cmd_opc);
			make_type_res(ea, bank);
			ea = make_offset_gb_custom(ea, bank, 0);	// no exec
		} else {
			Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
		}
		return ea;
	}
}

static dd_script_selection(void) {
	Jump(dd_opcode(ScreenEA()));
}

static main(void) {
	AddHotkey("Shift-O", "dd_script_selection");
}
