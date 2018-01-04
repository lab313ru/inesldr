#include <idc.idc>
#include <functions.idc>

// ---------------------------------
// THUNDERBIRDS US SCRIPT PARSER
// ---------------------------------

static thunder_us_get_norm_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x00) {
		ret = "ppu_nt_fill";
	} else if(cmd == 0x05) {
		ret = "game_level_start";
	} else if(cmd == 0x06) {
		ret = "raw_cut_send";
	} else if(cmd == 0x07) {
		ret = "pak_cut_send";
	} else if(cmd == 0x08) {
		ret = "msg_dlg_disp";
	} else if(cmd == 0x0A) {
		ret = "tlm_unp_imm";
	} else if(cmd == 0x0B) {
		ret = "tlm_unp_ofs";
	} else if(cmd == 0x0F) {
		ret = "set_pal_imm";
	} else if(cmd == 0x10) {
		ret = "set_pal_ofs";
	} else if(cmd == 0x18) {
		ret = "nop";
	} else if(cmd == 0x19) {
		ret = "chr0_set";
	} else if(cmd == 0x1A) {
		ret = "chr1_set";
	} else if(cmd == 0x1C) {
		ret = "mus_set_imm";
	} else if(cmd == 0x1D) {
		ret = "snd_set_imm";
	} else if(cmd == 0x1E) {
		ret = "mus_set_ofs";
	} else if(cmd == 0x1F) {
		ret = "snd_set_ofs";
	} else if(cmd == 0x20) {
		ret = "write_imm";
	} else if(cmd == 0x21) {
		ret = "add_imm";
	} else if(cmd == 0x22) {
		ret = "move_mem";
	} else if(cmd == 0x23) {
		ret = "or_mem";
	} else if(cmd == 0x24) {
		ret = "or_imm";
	} else if(cmd == 0x25) {
		ret = "and_imm";
	} else if(cmd == 0x26) {
		ret = "delay";
	} else if(cmd == 0x27) {
		ret = "wait_for_key";
	} else if(cmd == 0x28) {
		ret = "wait_for_key_with_cur";
	} else if(cmd == 0x29) {
		ret = "vblank";
	} else if(cmd == 0x2A) {
		ret = "call";
	} else if(cmd == 0x2B) {
		ret = "ret";
	} else if(cmd == 0x2C) {
		ret = "break";
	} else if(cmd == 0x2D) {
		ret = "map_briefing";
	} else
		ret = form("cmd_%02x", cmd);
	return ret;
}

static thunder_us_get_F2_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0xF8) {
		ret = "block_end";
	} else if(cmd == 0xF9) {
		ret = "block_bne";
	} else if(cmd == 0xFA) {
		ret = "block_equ";
	} else if(cmd == 0xFB) {
		ret = "loop_end";
	} else if(cmd == 0xFC) {
		ret = "cmp";
	} else if(cmd == 0xFD) {
		ret = "test";
	} else if(cmd == 0xFF) {
		ret = "loop_start";
	} else
		ret = form("cmd_%F202x", cmd);
	return ret;
}

static thunder_script(ea) {
	auto cmd_len, cmd_opc = "", cmd_branch = 0, cmd, stop = 0, jmp_ptr, jmp_base;
	auto _sp = 0;
//	if((ea != BADADDR) && (eea != BADADDR)) {
	if(ea != BADADDR) {
		Message(" start at ea = 0x%08x!\n", ea);
		do {
			cmd = Byte(ea);
			if(cmd == 0xF2) {	// case/loop operands
				cmd = Byte(ea + 1);
				cmd_opc = thunder_us_get_F2_opcode_name(cmd);
				if((cmd == 0xF9)||(cmd == 0xFA)) { // branch start
					ea = make_data_array(ea, 2, cmd_opc);
					cmd_branch++;
				} else if((cmd == 0xF8)) {	// branch stop
					ea = make_data_array(ea, 2, cmd_opc);
					cmd_branch--;
				} else if((cmd == 0xFC)||(cmd == 0xFD)||(cmd == 0xFE)) {	// compare
					ea = make_data_array(ea, 2, cmd_opc);
					ea = make_offset(ea, 6);
					ea = make_data_array(ea, 1, "");
				} else if(cmd == 0xFF) {	// loop start
					ea = make_data_array(ea, 3, cmd_opc);
				} else if((cmd == 0xFB)) {	// loop end
					ea = make_data_array(ea, 2, cmd_opc);
				} else {
					Message(" unknown F2 opcode at 0x%08x = %02x\n", ea, cmd);
					stop = 1;
				}
		 	} else {			// normal operands
				cmd_opc = thunder_us_get_norm_opcode_name(cmd);
				if((cmd==0x03)||(cmd==0x04)||(cmd==0x0D)||(cmd==0x0E)||(cmd==0x14)||(cmd==0x15)||(cmd==0x18)||(cmd==0x27)||(cmd==0x28)||(cmd==0x29)||(cmd==0x2D)) {			// no args
					ea = make_data_array(ea, 1, cmd_opc);
				} else if((cmd==0x2C)) {	// script break
					ea = make_data_array(ea, 1, cmd_opc);
					if(!cmd_branch)
						stop = 1;
				} else if((cmd==0x01)||(cmd==0x02)||(cmd==0x05)||(cmd==0x07)||(cmd==0x08)||(cmd==0x0A)||(cmd==0x0F)||(cmd==0x11)||(cmd==0x17)||(cmd==0x19)||(cmd==0x1A)||(cmd==0x1C)||(cmd==0x1D)||(cmd==0x26)) {	// 1b arg
					ea = make_data_array(ea, 2, cmd_opc);
				} else if((cmd==0x00)||(cmd==0x06)) {	// 2b args
					ea = make_data_array(ea, 3, cmd_opc);
				} else if((cmd==0x0C)) {	// 2b+ofs
					ea = make_data_array(ea, 3, cmd_opc);
					ea = make_offset(ea, 6);
				} else if((cmd==0x12)||(cmd==0x13)) {	// 3b
					ea = make_data_array(ea, 4, cmd_opc);
				} else if((cmd==0x09)) {	// 4b
					ea = make_data_array(ea, 5, cmd_opc);
				} else if((cmd==0x1B)) {	// 5b
					ea = make_data_array(ea, 6, cmd_opc);
				} else if((cmd==0x16)) {	// 6b
					ea = make_data_array(ea, 7, cmd_opc);
				} else if((cmd==0x0B)||(cmd==0x10)||(cmd==0x1E)||(cmd==0x1F)) {	// ofs
					ea = make_data_array(ea, 1, cmd_opc);
					ea = make_offset(ea, 6);
				} else if((cmd==0x2A)) {	// call
					ea = make_data_array(ea, 1, cmd_opc);
					jmp_ptr = Word(ea);
					jmp_base = MK_FP(AskSelector(6 + 1), 0);
					ea = make_offset(ea, 6);
					if(!_sp) {
						_sp = ea;
						ea = jmp_base + jmp_ptr;
					} else {
						Message(" out of stack at 0x%08x\n", ea);
						stop = 1;
					}
				} else if((cmd==0x2B)) {	// ret
					ea = make_data_array(ea, 1, cmd_opc);
					if(_sp) {
						ea = _sp;
						_sp = 0;
					} else {
						Message(" can't ret at 0x%08x\n", ea);
						stop = 1;
					}
				} else if((cmd==0x20)||(cmd==0x21)||(cmd==0x24)||(cmd==0x25)) {	// ofs+1b
					ea = make_data_array(ea, 1, cmd_opc);
					ea = make_offset(ea, 6);
					ea = make_data_array(ea, 1, "");
				} else if((cmd==0x22)||(cmd==0x23)) {	// ofs+ofs
					ea = make_data_array(ea, 1, cmd_opc);
					ea = make_offset(ea, 6);
					ea = make_offset(ea, 6);
				} else {
					Message(" unknown opcode at 0x%08x = %02x\n", ea, cmd);
					stop = 1;
				}
			}
//			if((!_sp) && (ea > eea))
//				stop = 1;
		} while (!stop);
		Message(" stop at ea = 0x%08x!\n", ea);
	} else
		Message(" no selection!\n");
}

static thunder_script_selection(void) {
	auto ea = ScreenEA();
	thunder_script(ea);
}

static thunder_script_auto(void) {
	auto eastart = 0x98000;
	auto eastop = 0x98056;
	Message(" Start\n");
	do {
		auto ptr = Word(eastart);
		auto base = MK_FP(AskSelector(6 + 1), 0);
		thunder_script(base + ptr);
		eastart = eastart + 2;
	} while (eastart < eastop);

	Message("Done\n");
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
