#include <idc.idc>
#include <functions.idc>

// ----------------------------
// CAPCOM MUSIC FORMAT PARSER
// ----------------------------

static capcom_snd_opcode_name(cmd) {
	auto ret = "";
	if(cmd == 0x0E) {
		ret = "loopA end";
	} else if(cmd == 0x0F) {
		ret = "loopB end";
	} else if(cmd == 0x10) {
		ret = "loopC end";
	} else if(cmd == 0x11) {
		ret = "loopD end";
	} else if(cmd == 0x12) {
		ret = "loopA start";
	} else if(cmd == 0x13) {
		ret = "loopB start";
	} else if(cmd == 0x14) {
		ret = "loopC start";
	} else if(cmd == 0x15) {
		ret = "loopD start";
	} else if(cmd == 0x16) {
		ret = "jump";
	} else if(cmd == 0x17) {
		ret = "stop";
	} else
		ret = form("cmd_%02x", cmd);
	return ret;
}

static capcom_snd_parse(void) {
	auto ea = ScreenEA();
	ea = FindUnexplored(ea - 1, SEARCH_DOWN);
	auto cmd,jmp_ptr, base, addr;
	auto seg = GetReg(ea,"ds") - 1;
	cmd = Byte(ea);
	Message("cmd = %02x at 0x%08x\n",cmd,ea);
	if(cmd >= 0x20) { 			// data
		ea = make_data_array(ea,1,"data");
	} else if(((cmd >=0) && (cmd <=3))||(cmd == 0x17)) {	// no args
		ea = make_data_array(ea,1,capcom_snd_opcode_name(cmd));
	} else if((cmd == 4)||((cmd >= 6)&&(cmd <= 0xD))||(cmd == 0x18)) {	// 1 byte arg
		ea = make_data_array(ea,2,capcom_snd_opcode_name(cmd));
	} else if(cmd == 5) {	// 2 byte arg
		ea = make_data_array(ea,3,capcom_snd_opcode_name(cmd));
	} else if((cmd >=0x0E) && (cmd <=0x15)) {	// loops
		ea = make_data_array(ea,2,capcom_snd_opcode_name(cmd));
		MakeUnknown(ea, 2, DOUNK_SIMPLE);
		MakeData(ea, FF_BYTE, 1, 0);
		SetArrayFormat(ea, AP_IDXHEX, 1, 0);
		MakeData(ea + 1, FF_BYTE, 1, 0);
		SetArrayFormat(ea + 1, AP_IDXHEX, 1, 0);
		addr = (Byte(ea) << 8) | Byte(ea + 1);
		base = MK_FP(AskSelector(seg + 1), 0);
		OpOffEx(ea + 1, 0, REF_LOW8, base + addr, base, 0);
		OpOffEx(ea, 0, REF_HIGH8, base + addr, base, 0);
		ea=ea + 2;
	} else if(cmd == 0x16) {	// jump
		ea = make_data_array(ea,1,capcom_snd_opcode_name(cmd));
		MakeUnknown(ea, 2, DOUNK_SIMPLE);
		MakeData(ea, FF_BYTE, 1, 0);
		SetArrayFormat(ea, AP_IDXHEX, 1, 0);
		MakeData(ea + 1, FF_BYTE, 1, 0);
		SetArrayFormat(ea + 1, AP_IDXHEX, 1, 0);
		addr = (Byte(ea) << 8) | Byte(ea + 1);
		base = MK_FP(AskSelector(seg + 1), 0);
		OpOffEx(ea + 1, 0, REF_LOW8, base + addr, base, 0);
		OpOffEx(ea, 0, REF_HIGH8, base + addr, base, 0);
		ea=ea + 2;
	} else {
		Message(" unk_cmd at 0x%08x\n",ea);
	}
	Jump(ea);
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
