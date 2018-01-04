#include <idc.idc>
#include <functions.idc>

// HIRYU NO KEN SPECIAL
static hnks_find_brk_calls() {
	auto ea = ScreenEA(), idx, ptr, bank, ofs, base, addr, name;
	ea = FindText(ea, SEARCH_DOWN | SEARCH_CASE, 0, 0, "BRK");
//	Message("BRK search start!\n");
//	do {
	if(ea != BADADDR) {
		MakeUnknown(ea + 1, 2, DOUNK_SIMPLE);
		MakeData(ea + 1, FF_BYTE, 2, 0);
		MakeCode(ea + 3);
		AutoMark(ea + 3, AU_CODE);
		Wait();
		idx = Byte(ea + 1);
		bank = Byte(ea + 2);
		base = MK_FP(AskSelector(bank + 1), 0);
		ptr = base + 0x8000 + (idx << 1);
		ofs = Word(ptr);
		MakeUnknown(ptr, 2, DOUNK_SIMPLE);
		MakeData(ptr, FF_WORD, 1, 0);
		OpOff(ptr,0,base);
		addr = base + ofs;
//		Message(" bank %01x, base %08x, ofs %04x, addr %08x\n", bank, base, ofs, addr);
		MakeCode(addr);
		AutoMark(ea + 3, AU_CODE);
		Wait();
		name = NameEx(BADADDR, addr);
		if(name == "") {
			MakeComm(ea + 1, "");
			MakeComm(ea + 1, form("0x%08x", addr));
//			Message(" direct ofs found from 0x%08x to 0x%08x\n", ea, addr);
		} else {
			MakeComm(ea + 1, "");
			MakeComm(ea + 1, form("%s", name));
//			Message(" direct ofs found from 0x%08x to %s\n", ea, name);
		}
		AddCodeXref(ea, addr, fl_CF);
		Wait();
	}
	Jump(ea + 3);
//	Message("BRK search done!\n");
}
//

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
