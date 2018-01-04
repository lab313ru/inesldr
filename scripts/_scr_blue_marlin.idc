#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// BLUE MARLIN SCRIPT CONVERT
// ---------------------------------

static bluemar_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "exit";
	} else if(cmd == 0x01) {
		ret = "ppu_off";
	} else if(cmd == 0x02) {
		ret = "ppu_on";
	} else if(cmd == 0x03) {
		ret = "cls";
	} else if(cmd == 0x06) {
		ret = "call_engine_manager";
	} else if(cmd == 0x0A) {
		ret = "fade_in";
	} else if(cmd == 0x0B) {
		ret = "fade_out";
	} else if(cmd == 0x0D) {
		ret = "mirr0";
	} else if(cmd == 0x0E) {
		ret = "mirr1";
	} else if(cmd == 0x0F) {
		ret = "slow_fade_in";
	} else if(cmd == 0x2E) {
		ret = "call_engine_manager_until_apu";
	} else if(cmd == 0x2F) {
		ret = "exec";
	} else if(cmd == 0x3B) {
		ret = "for_end";
	} else if(cmd == 0x41) {
		ret = "rts";
	} else if(cmd == 0x42) {
		ret = "bcs";
	} else if(cmd == 0x43) {
		ret = "bcc";
	} else if(cmd == 0x4A) {
		ret = "tlm_unk2";
	} else if(cmd == 0x04) {
		ret = "set_engine_manager";
	} else if(cmd == 0x05) {
		ret = "set_irq";
	} else if(cmd == 0x07) {
		ret = "do_engine_manager_loop";
	} else if(cmd == 0x08) {
		ret = "set_scrollY";
	} else if(cmd == 0x09) {
		ret = "set_scrollX";
	} else if(cmd == 0x0C) {
		ret = "call_engine_manager_pad_skip";
	} else if(cmd == 0x10) {
		ret = "load_chr4";
	} else if(cmd == 0x11) {
		ret = "load_chr5";
	} else if(cmd == 0x12) {
		ret = "load_chr6";
	} else if(cmd == 0x13) {
		ret = "load_chr7";
	} else if(cmd == 0x14) {
		ret = "load_chr01";
	} else if(cmd == 0x15) {
		ret = "load_chr23";
	} else if(cmd == 0x20) {
		ret = "set_bg_pal";
	} else if(cmd == 0x21) {
		ret = "set_spr_pal";
	} else if(cmd == 0x22) {
		ret = "res_send";
	} else if(cmd == 0x23) {
		ret = "res_lib_send";
	} else if(cmd == 0x24) {
		ret = "set_music";
	} else if(cmd == 0x25) {
		ret = "set_bg_lib_pal";
	} else if(cmd == 0x26) {
		ret = "set_bg_spr_pal";
	} else if(cmd == 0x27) {
		ret = "msg";
	} else if(cmd == 0x28) {
		ret = "obj_insert";
	} else if(cmd == 0x29) {
		ret = "obj_show";
	} else if(cmd == 0x2A) {
		ret = "obj_hide";
	} else if(cmd == 0x2B) {
		ret = "obj_unk0";
	} else if(cmd == 0x2C) {
		ret = "obj_unk1";
	} else if(cmd == 0x2D) {
		ret = "obj_move";
	} else if(cmd == 0x30) {
		ret = "zram_write_byte";
	} else if(cmd == 0x31) {
		ret = "zram_write_word";
	} else if(cmd == 0x32) {
		ret = "ram_write_byte";
	} else if(cmd == 0x33) {
		ret = "ram_write_word";
	} else if(cmd == 0x34) {
		ret = "ram_inc_byte";
	} else if(cmd == 0x35) {
		ret = "ram_dec_byte";
	} else if(cmd == 0x36) {
		ret = "ram_inc_word";
	} else if(cmd == 0x37) {
		ret = "ram_dec_word";
	} else if(cmd == 0x38) {
		ret = "exec_ptr";
	} else if(cmd == 0x39) {
		ret = "jmp";
	} else if(cmd == 0x3A) {
		ret = "for_begin";
	} else if(cmd == 0x3C) {
		ret = "press_test_bne";
	} else if(cmd == 0x3D) {
		ret = "press_test_beq";
	} else if(cmd == 0x3E) {
		ret = "held_test_bne";
	} else if(cmd == 0x3F) {
		ret = "held_test_beq";
	} else if(cmd == 0x40) {
		ret = "jsr";
	} else if(cmd == 0x44) {
		ret = "switch";
	} else if(cmd == 0x45) {
		ret = "ram_move_byte";
	} else if(cmd == 0x46) {
		ret = "ram_move_word";
	} else if(cmd == 0x47) {
		ret = "beq";
	} else if(cmd == 0x48) {
		ret = "tlm_unk0";
	} else if(cmd == 0x49) {
		ret = "tlm_unk1";
	}
	return ret;
}

static bluemar_opcode(ea) {
	auto cmd_len, cmd_opc = "", cmd, jmp_ptr, jmp_base;
	auto _sp = 0;
	if(ea != BADADDR) {
//		Message(" start at ea = 0x%08x!\n", ea)
		cmd = Byte(ea);
		cmd_opc = bluemar_opcode_name(cmd);

		// no args
		if((cmd==0)||(cmd==0x41)||(cmd==0x2F)) {
			make_data_array(ea, 1, cmd_opc);
		} else if(((cmd>=1)&&(cmd<=3))||(cmd==6)||(cmd==0xA)||(cmd==0xB)||((cmd>=0xD)&&(cmd<=0xF))||(cmd==0x2E)||(cmd==0x3B)||(cmd==0x4A)) {
			ea = make_data_array(ea, 1, cmd_opc);
		} else if((cmd==7)||(cmd==0xC)||((cmd>=0x10)&&(cmd<=0x15))||(cmd==0x24)||(cmd==0x29)||(cmd==0x2A)||(cmd==0x2C)||(cmd==0x3A)||(cmd==0x49)) {	// 1 arg
			ea = make_data_array(ea, 2, cmd_opc);
		} else if((cmd==8)||(cmd==9)||(cmd==0x2B)||(cmd==0x30)) {	// 2 arg
			ea = make_data_array(ea, 3, cmd_opc);
		} else if((cmd==0x31)||(cmd==0x2D)) {	// 3 arg
			ea = make_data_array(ea, 4, cmd_opc);
		} else if(cmd==0x48) {	// 5 args
			ea = make_data_array(ea, 6, cmd_opc);
		} else if(cmd==0x28) {	// 8 arg
			ea = make_data_array(ea, 9, cmd_opc);
		} else if(cmd==0x32) {	// ofs0 byte
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_ex(ea, 0, 0, 1);
			ea = make_data_array(ea, 1, "");
		} else if(cmd==0x33) {	// ofs0 word
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_ex(ea, 0, 0, 1);
			ea = make_data_array(ea, 2, "");
		} else if((cmd>=0x34)&&(cmd<=0x37)) {	// ofs0
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_ex(ea, 0, 0, 1);
		} else if(cmd==0x47) {	// ofs67 ofs67
			ea = make_data_array(ea, 1, cmd_opc);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
			if(Word(ea)>=0xC000) {
				make_name(ea, 7, "beq_");
				ea = make_offset_ex(ea, 7, 0, 1);
			} else {
				make_name(ea, 6, "beq_");
				ea = make_offset_ex(ea, 6, 0, 1);
			}
		} else if((cmd==0x45)||(cmd==0x46)) {	// ofs67 ofs67 ofs67
			ea = make_data_array(ea, 1, cmd_opc);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
		} else if(cmd==0x22) {	// ofs2
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_ex(ea, 2, 0, 1);
		} else if(cmd==0x23) {	// ofs2 ofs67
			ea = make_data_array(ea, 1, cmd_opc);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
		} else if((cmd==0x20)||(cmd==0x21)) {	// ofs3
			ea = make_data_array(ea, 1, cmd_opc);
			ea = make_offset_ex(ea, 3, 0, 1);
		} else if((cmd==0x25)||(cmd==0x26)) {	// ofs3 ofs67
			ea = make_data_array(ea, 1, cmd_opc);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
		} else if((cmd==4)||(cmd==5)||(cmd==0x27)||(cmd==0x39)||(cmd==0x40)||(cmd==0x42)||(cmd==0x43)) {	// ofs67
			ea = make_data_array(ea, 1, cmd_opc);
			if(cmd==0x27) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "msg_"); else make_name(ea, 6, "msg_");
			} else if(cmd==0x39) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "jmp_"); else make_name(ea, 6, "jmp_");
			} else if(cmd==0x40) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "jsr_"); else make_name(ea, 6, "jsr_");
			} else if(cmd==0x42) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "bcs_"); else make_name(ea, 6, "bcs_");
			} else if(cmd==0x43) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "bcc_"); else make_name(ea, 6, "bcc_");
			}
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
		} else if(cmd==0x38) {	// ofs67 external exec
			ea = make_data_array(ea, 1, cmd_opc);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 1, 1); else ea = make_offset_ex(ea, 6, 1, 1);
		} else if((cmd>=0x3C)&&(cmd<=0x3F)) {	// 1 arg ofs 67
			ea = make_data_array(ea, 2, cmd_opc);
			if((cmd==0x3C)||(cmd==0x3E)) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "bne_"); else make_name(ea, 6, "bne_");
			} else if((cmd==0x3D)||(cmd==0x3F)) {
				if(Word(ea)>=0xC000) make_name(ea, 7, "beq_"); else make_name(ea, 6, "beq_");
			}
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
		} else if(cmd==0x44) {	// ofs 67 ofs67 list
			ea = make_data_array(ea, 1, cmd_opc);
			if(Word(ea)>=0xC000) ea = make_offset_ex(ea, 7, 0, 1); else ea = make_offset_ex(ea, 6, 0, 1);
			auto tmp = Word(ea) & 0x7FFF, cnt = 0;
			while((ea & 0x7FFF) < tmp) {
				if(Word(ea)>=0xC000) {
					make_name(ea, 7, form("case%X_",cnt));
					ea = make_offset_ex(ea, 7, 0, 1);
				} else {
					make_name(ea, 6, form("case%X_",cnt));
					ea = make_offset_ex(ea, 6, 0, 1);
				}
				cnt++;
				auto tmp1 = Word(ea) & 0x7FFF;
				if(tmp1 < tmp) tmp = tmp1;
			}
		} else {
			Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
		}
		return ea;
	}
}

static bluemar_script_selection(void) {
	Jump(bluemar_opcode(ScreenEA()));
}

static main(void) {
	AddHotkey("Shift-X", "bluemar_script_selection");
}
