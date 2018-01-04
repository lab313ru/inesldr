#include <idc.idc>
#include <functions.idc>

// DRAGON WARRIOR 3 MACROSES!!
static dw3_find_brk_calls() {
	auto ea = ScreenEA(), idx, cmd, bank, ofs, base, addr, name;
	auto idx_table = 0x1769ED;
	auto bank_table = 0x176917;
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
			cmd = Byte(ea + 2);
			if (((cmd & 0x0F) == 0x0F) && (((cmd & 0xF0) >= 0x40) && ((cmd & 0xF0) <= 0xE0))) { // direct calls
				bank = (cmd >> 4) | 0x10;
				base = MK_FP(AskSelector(bank + 1), 0);
				ofs = Word(base + 0x8000 + (idx << 1));
				addr = base + ofs;
//				Message(" bank %01x, base %08x, ofs %04x, addr %08x\n", bank, base, ofs, addr);
				name = NameEx(BADADDR, addr);
				if(name == "") {
					MakeComm(ea + 1, "");
					MakeComm(ea + 1, form("ofs 0x%08x, -> dst 0x%08x", base + 0x8000 + (idx << 1), addr));
//					Message(" direct ofs found from 0x%08x to 0x%08x\n", ea, addr);
				} else {
					MakeComm(ea + 1, "");
					MakeComm(ea + 1, form("%s", name));
//					Message(" direct ofs found from 0x%08x to %s\n", ea, name);
				}
				AddCodeXref(ea, addr, fl_CF);
				Wait();
			} else {	// lib calls
				cmd = ((cmd & 8) << 1) | (cmd >> 4);
				if(cmd < 2) { // library indexes
//					Message("idx=%02x, cmd=%02x, ",idx, cmd);
					ofs = idx | (cmd << 8);
//					Message("ofs=%03x, ",ofs);
					idx = Byte(idx_table + ofs);
//					Message("ofs_idx=%02x, ",idx);
					bank = (Byte(bank_table + (ofs >> 1)) >> (((ofs & 1) ^ 1) << 2)) & 0xF;
//					Message("bank_byte=%02x, nibble=%d\n",Byte(bank_table + (ofs >> 1)), (ofs & 1) ^ 1);

					base = MK_FP(AskSelector(bank + 1), 0);
					ofs = Word(base + 0x8000 + (idx << 1));
					addr = base + ofs;
//					Message(" bank %01x, base %08x, ofs %04x, addr %08x\n", bank, base, ofs, addr);
					name = NameEx(BADADDR, addr);
					if(name == "") {
						MakeComm(ea + 1, "");
						MakeComm(ea + 1, form("ofs 0x%08x, -> dst 0x%08x", base + 0x8000 + (idx << 1), addr));
//						Message(" lib ofs found from 0x%08x to 0x%08x\n", ea, addr);
					} else {
						MakeComm(ea + 1, "");
						MakeComm(ea + 1, form("%s", name));
//						Message(" lib ofs found from 0x%08x to %s\n", ea, name);
					}
//					Message(" define xref from 0x%08x to 0x%08x\n",ea,addr);
					AddCodeXref(ea, addr, fl_CF);
				} else { // other syscalls
					ofs = Word(0x1767C0 + (cmd << 1));
					if (ofs < 0xC000) {
						if(cmd < 0x0E)
							addr = 0x138000 + ofs;
						else
							addr = 0x060000 + ofs;
					} else {
						addr = 0x168000 + ofs;
					}
					name = NameEx(BADADDR, addr);
					if(name == "") {
						MakeComm(ea + 1, "");
						MakeComm(ea + 1, form("ofs 0x%08x", addr));
//						Message(" lib syscall from 0x%08x to 0x%08x\n", ea, addr);
					} else {
						MakeComm(ea + 1, "");
						MakeComm(ea + 1, form("%s,$%02X", name, idx));
//						Message(" direct ofs to %s\n", name);
					}
					AddCodeXref(ea, addr, fl_CF);
				}
			}
			Jump(ea + 3);
		}
//		ea = FindText(ea + 2, SEARCH_DOWN | SEARCH_CASE, 0, 0, "BRK");
//	} while (ea != BADADDR);
//	Message("BRK search done!\n");
}

static make_ofs_set() {
	auto bank_ea = 0x39100;
	auto lib_ea = 0x39112;
	auto i, j = 0;
	auto data = 0, bit, bank, addr, ofs, base;
	for(i = 0; i < 139; i++) {
		if(j == 0) {
			data = Byte(bank_ea + (i >> 3));
		}
		bit = data & 0x80;
		data = data << 1;
		j = j + 1;
		j = j & 7;
		if(bit)
			bank = 3;
		else
			bank = 2;
		base = MK_FP(AskSelector(bank + 1), 0);
		addr = lib_ea + (i * 5) + 1;
		ofs = Word(addr);
		Message("bank %d, base %08x, addr %08x, ofs %04x, dest 0x%08x\n",bank,base,addr,ofs,base+ofs);
		OpOffEx(addr, 0, REF_OFF16, -1, base, 0);
	}
}

static make_ofs_set2() {
	auto ea = ScreenEA(), bank, base, tileset;
	tileset = Byte(ea + 2);
	MakeUnknown(ea, 3, DOUNK_SIMPLE);
	MakeData(ea, FF_WORD, 1, 0);
	MakeData(ea + 2, FF_BYTE, 1, 0);
	if (tileset >= 0xC)
		bank = 7;
	else
		bank = 6;
	base = MK_FP(AskSelector(bank + 1), 0);
	OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
	Jump(ea + 3);
}

static make_ofs_set3() {
	auto ofsea = 0x14342B, ea = 0x14342D, size = 127, bank = 0xD, base = 0x138000, i;
	auto addr = Word(ofsea) + base, name, prev_name = NameEx(BADADDR, addr), prev_addr = addr;
	for(i = 0; i < size; i++) {
		addr = addr + Byte(ea + i);
		MakeComm(ea + i, "");
		name = NameEx(BADADDR, addr);
		if(name == "") {
			MakeComm(ea + i,  form("ofs: 0x%08x", addr));
		} else {
			MakeComm(ea + i,  form(".BYTE %s-%s", name, prev_name));
		}
		prev_name = name;
		prev_addr = addr;
		AddCodeXref(ea + 1, addr, fl_CF);
	}
}

static make_ofs_set4() {
	auto idx_table = 0x1769ED;
	auto bank_table = 0x176917;
	auto size = 426, bank, base, i, idx, ofs, addr, name;

	for(i = 0; i < size; i++) {
		idx = Byte(idx_table + i);
		MakeData(idx_table + i, FF_BYTE, 1, 0);
		bank = (Byte(bank_table + (i >> 1)) >> (((i & 1) ^ 1) << 2)) & 0xF;
		base = MK_FP(AskSelector(bank + 1), 0);
		ofs = Word(base + 0x8000 + (idx << 1));
		if(ofs >=0xC000) {
			base = MK_FP(AskSelector(bank + 2), 0);
		}
		if(ofs != 0) {
			addr = base + ofs;
			name = NameEx(BADADDR, addr);
			if(name == "") {
				MakeComm(idx_table + i, "");
				MakeComm(idx_table + i, form("bank %02x, idx %02x", bank, idx, addr));
			} else {
				MakeComm(idx_table + i, "");
				MakeComm(idx_table + i, form("FAR $%02X,$%d7 (%s)", i & 0xFF, (i & 0x100)?1:0, name));
			}
		} else {
			MakeComm(idx_table + i,  form("FAR $%02X,$%d7 (bank %02x, idx %02x, empty)", i & 0xFF, (i & 0x100)?1:0, bank, idx));
		}
	}
}

static make_npc_lists() {
	auto ea = 0x14019A, idx, len, i;

	for(i = 0; i < 0xFA; i++) {
		MakeName(ea, form("_npcl%02X", i));
		while ((idx = Byte(ea)) != 0) {
			len = idx & 7;
			if(len == 0)
				len = 5;
			else if((len == 1)||(len == 2))
				len = 6;
			else if((len == 3)||(len == 4))
				len = 7;
			else if((len == 5)||(len == 6))
				len = 8;
			else if(len == 7)
				len = 9;
			MakeUnknown(ea, len, DOUNK_SIMPLE);
			MakeData(ea, FF_BYTE, len, 0);
			SetArrayFormat(ea, AP_IDXHEX, len, 0);
			ea = ea + len;
		}
		MakeUnknown(ea, 1, DOUNK_SIMPLE);
		MakeData(ea, FF_BYTE, 1, 0);
		ea = ea + 1;
	}
}

static main(void) {
//	test();
//	find_brk_calls();
//	make_ofs_set();
//	make_ofs_set3();
//	make_ofs_set4();
//	make_npc_lists();
}

// END OF DW3
