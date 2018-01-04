#include <idc.idc>
#include <functions.idc>

// ----------------------------
// BUCKY'O HARE cutscene script
// ----------------------------

static bucky_script_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0xF) {
		ret = "stop";
	} else if(cmd == 0x14) {
		ret = "jmp";
	} else if(cmd == 0xB) {
		ret = "loop";
	} else
		ret = form("cmd_%02x", cmd);
	return ret;
}

static bucky_script_parse_selection(void) {
	auto ea = ScreenEA();
	auto cmd = Byte(ea);
	if(cmd & 0x80) { 	// hi cmd
		if(cmd >= 0xC0) {
			ea = make_data_array(ea,7,bucky_script_opcode_name(cmd));
		} else if(cmd >= 0xB0) {
			ea = make_data_array(ea,1,bucky_script_opcode_name(cmd));
		} else if(cmd >= 0x90) {
			ea = make_data_array(ea,3,bucky_script_opcode_name(cmd));
		} else {
			ea = make_data_array(ea,6,bucky_script_opcode_name(cmd));
		}
	} else {		 	// lo cmd
		if((cmd == 1)||(cmd == 8)||(cmd == 9)||(cmd == 0xA)||(cmd == 0xF)||(cmd == 0x12)||(cmd == 0x15)||(cmd == 0x16)||(cmd == 0x18)) {// no args
			ea = make_data_array(ea,1,bucky_script_opcode_name(cmd));
		} else if(((cmd >= 2)&&(cmd <= 7))||(cmd == 0xC)||(cmd == 0xD)||(cmd == 0xE)||(cmd == 0x11)||(cmd == 0x13)) {		// 1 byte arg
			ea = make_data_array(ea,2,bucky_script_opcode_name(cmd));
		} else if(cmd == 0x14) {	// 2 byte arg jmp
			ea = make_data_array(ea,1,bucky_script_opcode_name(cmd));
			ea = make_offset(ea,5);
		} else if((cmd == 0xB)||(cmd == 0x10)) {		// 2 byte arg loop
			ea = make_data_array(ea,3,bucky_script_opcode_name(cmd));
		} else if(cmd == 0) {		// 4 byte arg
			ea = make_data_array(ea,5,bucky_script_opcode_name(cmd));
		} else if(cmd == 0x17) {		// 8 byte arg
			ea = make_data_array(ea,9,bucky_script_opcode_name(cmd));
		} else {
			Message(" unk_cmd!\n");
		}
	}
	Jump(ea);
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
