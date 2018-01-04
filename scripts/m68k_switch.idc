
#include <idc.idc>

static main(void) {
	auto ea = FirstSeg(), ea_end, count, i, base;
	ea_end = SegEnd(ea);
	Message("Start\n");
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "30 3B 08 06 4E FB 00 02");
		base = ea + 8;
//		Message("Pattern at 0x%08x\n", ea);
		if(ea != BADADDR) {
			count = Word(base) >> 1;
//			Message(" size = %08x\n", count);
			for(i = 0; i < count; i++) {
				auto idx = base + (i * 2);
				auto ofs = Word(idx);
//				Message("  ofs = 0x%08x\n", base + ofs);
				MakeUnknown(idx, 2, DOUNK_SIMPLE);
				MakeWord(idx);
				OpOffEx(idx, 0, REF_OFF16, base + ofs, base, 0);
				AutoMark(base + ofs, AU_CODE);
				Wait();
			}
		}
	} while (ea != BADADDR);
	Message("Stop\n");
}
