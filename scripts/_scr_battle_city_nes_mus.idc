#include <idc.idc>
#include <functions.idc>

// ----------------------------
// BATTLE CITY SE SCRIPTS
// ----------------------------

static bcity_snd_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0xE8) {			//0
		ret = "STOP";
	} else if(cmd == 0xE9) {	//1
		ret = "LD_1A   ";
	} else if(cmd == 0xEA) {	//1
		ret = "LD_1B   ";
	} else if(cmd == 0xEB) {	//1
		ret = "LD_1C   ";
	} else if(cmd == 0xEC) {	//1
		ret = "LD_2    ";
	} else if(cmd == 0xED) {	//1
		ret = "LD_4    ";
	} else if(cmd == 0xEE) {	//1
		ret = "LD_1D   ";
	} else if(cmd == 0xEF) {	//0
		ret = "LOOP_BEGIN";
	} else if(cmd == 0xF0) {	//2
		ret = "DO_LOOPA";
	} else if(cmd == 0xF1) {	//2
		ret = "DO_LOOPB";
	} else if(cmd == 0xF2) {	//2
		ret = "DO_LOOPC";
	} else if((cmd >= 0xF3)&&(cmd<=0xF8)) {	//0
		ret = "NOP";
	} else if(cmd == 0xF9) {	//1
		ret = "JMP";
	} else
		ret = form("UNK_%02X", cmd);
	return ret;
}

static bcity_snd_parse(void) {
	auto ea = ScreenEA();
//	ea = FindUnexplored(ea - 1, SEARCH_DOWN);
	auto cmd,jmp_ptr,base,addr,tmp;
	auto seg = GetReg(ea,"ds") - 1;
	cmd = Byte(ea);
//	Message("cmd = %02x at 0x%08x\n",cmd,ea);
	if(cmd < 0x60) { 			// data
		ea = make_data_array(ea,1,form("NOTE    %02X,%02X",cmd&0xF8,cmd&7));
	} else if(cmd == 0x60) { 			// data
		ea = make_data_array(ea,1,"MOVE_6_TO_7");
	} else if((cmd > 0x60)&&(cmd < 0xE8)) {		// data
		ea = make_data_array(ea,1,form("LD_6    %02X",cmd-0x60));
	} else if(((cmd >=0xF3) && (cmd <=0xF8))||(cmd == 0xE8)||(cmd == 0xEF)) {	// no args
		ea = make_data_array(ea,1,bcity_snd_opcode_name(cmd));
	} else if((cmd >= 0xE9)&&(cmd <= 0xEE)) {	// 1 byte arg
		tmp = Byte(ea+1);
		ea = make_data_array(ea,2,form("%s%02X",bcity_snd_opcode_name(cmd),tmp));
	} else if((cmd >=0xF0) && (cmd <=0xF2)) {	// loops
		tmp = Byte(ea+1);
		addr = Byte(ea+2);
		ea = make_data_array(ea,2,form("%s %02X",bcity_snd_opcode_name(cmd),tmp));
		base = AskLong(0, "Enter Base Offset");
		ea = make_data_array(ea,1,form("OFS %02X",addr));
		OpOffEx(ea-1, 0, REF_OFF16, -1, 0, -base);
	} else if(cmd == 0xF9) {	// jump
		addr = Byte(ea+1);
		ea = make_data_array(ea,1,bcity_snd_opcode_name(cmd));
		base = AskLong(0, "Enter Base Offset");
		ea = make_data_array(ea,1,form("OFS %02X",addr));
		OpOffEx(ea-1, 0, REF_OFF16, -1, 0, -base);
	} else {
		Message(" unk_cmd at 0x%08x\n",ea);
	}
	Jump(ea);
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
	AddHotkey("Shift-I", "bcity_snd_parse");
}
