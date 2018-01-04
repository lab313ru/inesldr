#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// CHIP AND DALE 2 NES SCRIPTS
// ---------------------------------

static make_offset_custom(ea, exec) {
	auto ofs = Word(ea), bank = GetReg(ea,"ds")-1;
	if(ofs<0x800)
		ea = make_offset_ex(ea, 0, exec, 1);
	else if(ofs>=0xC000) {
		ea = make_offset_ex(ea, 7, exec, 1);
	} else {
		if((bank==7)&&(ofs<0xC000)) {
			ea = make_offset_ex(ea, AskLong(0, "Enter Bank Nunber"), exec, 1);
		} else
			ea = make_offset_ex(ea, bank, exec, 1);
	}
	return ea;
}

static make_offset_custom_b(ea, bank, exec) {
	auto ofs = Word(ea);
	if(ofs<0x800)
		ea = make_offset_ex(ea, 0, exec, 1);
	else if(ofs>=0xC000) {
		ea = make_offset_ex(ea, 7, exec, 1);
	} else {
		ea = make_offset_ex(ea, bank, exec, 1);
	}
	return ea;
}

static cd2_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "exit";
	} else if(cmd == 0x1) {
		ret = "flush";
	} else if(cmd == 0x2) {
		ret = "delay";
	} else if(cmd == 0x3) {
		ret = "ejmp";
	} else if(cmd == 0x4) {
		ret = "start";
	} else if(cmd == 0x5) {
		ret = "ejsr";
	} else if(cmd == 0x6) {
		ret = "jmp";
	} else if(cmd == 0x7) {
		ret = "jsr";
	} else if(cmd == 0x8) {
		ret = "ret";
	} else if(cmd == 0x9) {
		ret = "loopA";
	} else if(cmd == 0xA) {
		ret = "loopB";
	} else if(cmd == 0xB) {
		ret = "beq";
	} else if(cmd == 0xC) {
		ret = "bcs";
	} else if(cmd == 0xD) {
		ret = "bmi";
	} else if(cmd == 0xE) {
		ret = "bne";
	} else if(cmd == 0xF) {
		ret = "bcc";
	} else if(cmd == 0x10) {
		ret = "bpl";
	} else if(cmd == 0x11) {
		ret = "test_pads";
	} else if(cmd == 0x12) {
		ret = "switch";
	} else if(cmd == 0x13) {
		ret = "sta";
	} else if(cmd == 0x14) {
		ret = "chr_bank";
	} else if(cmd == 0x15) {
		ret = "loopA_reset";
	} else if(cmd == 0x1B) {
		ret = "loopB_reset";
	} else if(cmd == 0x41) {
		ret = "stack_ofs_skip";
	} else if(cmd == 0x42) {
		ret = "default_pc";
	} else if(cmd == 0x43) {
		ret = "param_40_set";
	} else {
		ret = "unk";
	}
	return ret;
}

static cd2_opcode(ea) {
	auto cmd_opc = "", cmd, bank, tmp0, tmp1, tmp2;
	bank = GetReg(ea,"ds");

//	if(isCode(ea)) ea = BADADDR;

	if(ea != BADADDR) {
//		Message(" start at ea = 0x%08x!\n", ea);
		cmd = Byte(ea);
		cmd_opc = cd2_opcode_name(cmd);

		if((cmd == 0x00)||(cmd == 0x08)) {	// ret/exit/stop
			ea = make_data_array(ea, 1, cmd_opc);
			// stop here
		} else if(cmd==0x12) {	// switch
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_custom(ea, 0);	// no exec
			make_offset_custom(ea, 0);	// no exec
			ea = Word(ea)+MK_FP(AskSelector(GetReg(ea,"ds")), 0);
			// stop here
		} else if((cmd == 0x01)||(cmd==0x15)||(cmd==0x16)||(cmd==0x18)||(cmd==0x19)||(cmd==0x1A)||(cmd==0x1B)||(cmd==0x1C)||(cmd==0x1F)||(cmd==0x20)||(cmd==0x21)||(cmd==0x22)||(cmd==0x23)||(cmd==0x24)||(cmd==0x2B)||(cmd==0x2C)||(cmd==0x2D)||(cmd==0x2E)||(cmd==0x40)||(cmd==0x43)||(cmd==0x44)) {	// no args
			ea = make_data_array(ea, 1, cmd_opc);
		} else if((cmd==2)||(cmd==0x17)||(cmd==0x1D)||(cmd==0x1E)||(cmd==0x27)||(cmd==0x28)||(cmd==0x29)||(cmd==0x2A)||(cmd==0x2F)||(cmd==0x30)||(cmd==0x31)||(cmd==0x33)||(cmd==0x34)||(cmd==0x35)||(cmd==0x37)) {	// 1b arg
			ea = make_data_array(ea, 2, cmd_opc);
		} else if((cmd==0x14)||(cmd==0x25)||(cmd==0x26)||(cmd==0x32)||(cmd==0x38)||(cmd==0x39)||(cmd==0x3C)||(cmd==0x3D)) {	// 2b arg
			ea = make_data_array(ea, 3, cmd_opc);
		} else if(cmd==0x3F) {	// ofs, 1b arg
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_custom_b(ea, 2, 0); // no exec
			ea = make_data_array(ea, 1, "");
		} else if(cmd==0x3E) {	// 4b arg
			ea = make_data_array(ea, 5, cmd_opc);
		} else if(cmd==0x11) {	// 2b args, ofs
			ea = make_data_array(ea, 3, cmd_opc);
			ea = make_offset_custom(ea, 0);	// no exec
		} else if((cmd==0x04)||(cmd==0x09)||(cmd==0x0A)||(cmd==0x13)) {	// 1b args, ofs
			ea = make_data_array(ea, 2, cmd_opc);
			ea = make_offset_custom(ea, 0);	// no exec
		} else if((cmd==0x03)||(cmd==0x05)) {	// ofs
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_custom(ea, 1);	// exec
		} else if((cmd==0x06)||(cmd==0x07)||(cmd==0x0B)||(cmd==0x0C)||(cmd==0x0D)||(cmd==0x0E)||(cmd==0x0F)||(cmd==0x10)||(cmd==0x3A)||(cmd==0x3B)||(cmd==0x42)||(cmd==0x41)) {	// ofs
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_custom(ea, 0);	// no exec
		} else {
			Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
		}
		return ea;
	} else
		return ea;
}

static cd2_apu_name(cmd) {
	auto ret = "";
	if(cmd == 0xFF) {
		ret = "STOP";
	} else if(cmd == 0xF0) {
		ret = "LOOP_START";
	} else if(cmd == 0xF1) {
		ret = "DO_LOOP";
	} else if(cmd == 0xF2) {
		ret = "JSR";
	} else if(cmd == 0xF3) {
		ret = "RET";
	} else if(cmd == 0xF8) {
		ret = "JMP";
	} else {
		ret = "unk";
	}
	return ret;
}

static cd2_apu(ea) {
	auto cmd_opc = "", cmd, bank, tmp0, tmp1, tmp2, stop = 0;
	bank = GetReg(ea,"ds");
	tmp2 = ea;
	do {
		if(ea != BADADDR) {
			tmp0 = 0;
			tmp1 = ea;
			while((cmd = Byte(ea)) < 0xD0) {
				tmp0++;
				ea++;
			}
			if(tmp0>0) {
				ea = make_data_array(tmp1, tmp0, "DATA");
				cmd = Byte(ea);
			}
			Message(" cmd == %04x\n", cmd);
			cmd_opc = cd2_apu_name(cmd);
			if((cmd == 0xF3)||(cmd == 0xFF)) {	// RET/STOP
				make_data_array(ea, 1, cmd_opc);
				stop = 1;
				// stop here
			} else if(((cmd >= 0xD0)&&(cmd<=0xD8))||(cmd==0xE8)||(cmd==0xF1)) {	// no args
				ea = make_data_array(ea, 1, cmd_opc);
			} else if(((cmd >= 0xD9)&&(cmd<=0xE2))||((cmd >= 0xE9)&&(cmd<=0xED))||(cmd==0xF0)) {	// 1b arg
				ea = make_data_array(ea, 2, cmd_opc);
			} else if(cmd==0xF2) {	// ofs
				ea = make_data_array(ea, 1, cmd_opc);
				ea = make_offset_custom_b(ea, 6, 0); // no exec
			} else if(cmd==0xF8) {	// ofs
				make_data_array(ea, 1, cmd_opc);
				make_offset_custom_b(ea+1, 6, 0); // no exec
				stop = 1;
				// stop here
			} else {
				Message(" unknown apu opcode at 0x%08x = %02x\n", ea, cmd);
			}
//			return ea;
		}
	} while (!stop);
	return tmp2;
}

static cd2_script_selection(void) {
	Jump(cd2_opcode(ScreenEA()));
}

static cd2_apu_selection(void) {
	Jump(cd2_apu(ScreenEA()));
}

static main(void) {
	AddHotkey("Shift-O", "cd2_script_selection");
//	AddHotkey("Shift-I", "cd2_apu_selection");
}
