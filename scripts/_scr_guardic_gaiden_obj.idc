#include <idc.idc>
#include <functions.idc>

// guardic lvl obj data list
static make_obj_list() {
	auto ea = 0x80A6E, cmd, len, i, j;
	for(i = 0; i < 24; i++) {
		for(j = 0; j < 24; j++) {
			cmd = Byte(ea);
			if((cmd & 0x60) == 0x20)
				Message(form("%02x",Byte(ea + 3)));
			else
				Message("--");
			len = (cmd & 0xF) + 1;
			MakeUnknown(ea, len, DOUNK_SIMPLE);
			MakeData(ea, FF_BYTE, len, 0);
			SetArrayFormat(ea, AP_IDXHEX, len, 0);
			ea = ea + len;
		}
		Message("\n");
	}
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
