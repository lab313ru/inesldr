
#include <idc.idc>

static makestring(ea) {
	if(ea != BADADDR) {
		auto gstring, stringlen, curlen, stringsize = Dword(ea + 8);
		MakeUnknown(ea, (stringsize * 2) + 12, DOUNK_SIMPLE);
//		Message("string found at %08x, size %08x \"", ea, stringsize);
		MakeDword(ea + 0);
		MakeDword(ea + 4);
		MakeDword(ea + 8);
		curlen = 0;
		do {
			MakeStr(ea + (curlen * 2) + 12, BADADDR);
			gstring = GetString(ea + (curlen * 2) + 12, -1, ASCSTR_UNICODE);
//			Message("%s", gstring);
			stringlen = strlen(gstring);
			curlen = curlen + stringlen;
		} while ((gstring != "") && (curlen != stringsize));
//		Message("\"\n", curlen);
	} else {
		Message("No more strings found!\n");
	}
}

static main(void) {
	auto screenea = ScreenEA();
	auto nextstringea = FindBinary(screenea, SEARCH_DOWN, "B0 04 02 00 FF FF FF FF");
	SetLongPrm (INF_STRTYPE, ASCSTR_UNICODE); // ASCSTR_ULEN4);

	Message("Convert Start at %08x\n", screenea);
	makestring(nextstringea);	

	do {
		nextstringea = FindBinary(nextstringea + 1, SEARCH_DOWN | SEARCH_NEXT, "B0 04 02 00 FF FF FF FF");
		makestring(nextstringea);
	} while (nextstringea != BADADDR);

	Message("Convert Over\n");
}
