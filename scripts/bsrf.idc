
#include <idc.idc>

//TRY ME!
//void    AddCodeXref(long From,long To,long flowtype);

static main(void) {
	auto i, j, k, l, opcode, oeaddr, ops, lname, laddr, loffs, offs, reg, eaddr = FirstSeg(), maxaddr = SegEnd(eaddr);
	Message("Segment from %08x to %08x found.\n", eaddr, maxaddr);
	do {
		if(isCode(GetFlags(eaddr))) {
			opcode = Word(eaddr);
			if(((opcode & 0xF0FF) == 0x0003) || ((opcode & 0xF0FF) == 0x0023)) {	// bsrf, braf
				reg = opcode & 0x0F00;
				for(i = 0; i < 20; i++) {
					j = i << 1;
					oeaddr = eaddr - j;
					ops = Word(oeaddr);
					if((ops & 0xFF00) == (0xD000 | reg)) { // mov.l #, r#
						SetFlags(oeaddr, GetFlags(oeaddr) & 0xffafffff);
						SetManualInsn(eaddr, "");
						offs = Dword((oeaddr & 0xfffffffc) + ((ops & 0xff) << 2) + 4);
						laddr = eaddr + offs + 4;
						lname = NameEx(BADADDR, laddr);
						if(lname == "") {
							AutoMark(laddr, AU_CODE);
							AutoMark(laddr, AU_PROC);
							Wait();
							lname = NameEx(BADADDR, laddr);
							if(lname == "")
								SetManualInsn(eaddr, form("%s    r%d ; %08x", (opcode == 0x0003) ? "bsrf" : "braf", reg >> 8, laddr));
							else
								SetManualInsn(eaddr, form("%s    r%d ; %s", (opcode == 0x0003) ? "bsrf" : "braf", reg >> 8, lname));
						} else {
							SetManualInsn(eaddr, form("%s    r%d ; %s", (opcode == 0x0003) ? "bsrf" : "braf", reg >> 8, lname));
						}
						MakeComm(laddr, "");
						MakeComm(laddr, form("XREF: %08x", eaddr));
						break;
					}
				}
			} else if ((opcode & 0xFF00) == 0xc700) {	// mova #, r0
				offs = (eaddr & 0xfffffffc) + ((opcode & 0xff) << 2) + 4;
				for(i = 0; i < 20; i++) {
					j = i << 1;
					oeaddr = eaddr + j;
					ops = Word(oeaddr);
					if((ops & 0xF0FF) == 0x300C) { // add a0, r#
						reg = ops & 0x0F00;
						for(k = 0; k < 24; k++) {
							l = k << 1;
							laddr = oeaddr - l;
							ops = Word(laddr);
							if((ops & 0xFF00) == (0xD000 | reg)) { // mov.l #, r#
								SetFlags(laddr, GetFlags(laddr) & 0xffafffff);
								SetManualInsn(oeaddr, "");
								loffs = Dword((laddr & 0xfffffffc) + ((ops & 0xff) << 2) + 4);
								loffs = loffs + offs;
								lname = NameEx(BADADDR, loffs);
								if(lname == "") {
									SetManualInsn(oeaddr, form("add     r0, r%d ; %08x", reg >> 8, loffs));
								} else {
									SetManualInsn(oeaddr, form("add     r0, r%d ; %s",  reg >> 8, lname));
								}
								MakeComm(laddr, "");
								MakeComm(loffs, form("XREF: %08x", oeaddr));
								break;
							}
						}
						break;
					}
				}
			}
		}
		eaddr = NextHead(eaddr, maxaddr);
		if(eaddr == BADADDR) {
			eaddr = NextSeg(maxaddr);
			maxaddr = SegEnd(eaddr);
			if(eaddr != BADADDR)
				Message("Segment from %08x to %08x found.\n", eaddr, maxaddr);
		}
	} while (eaddr != BADADDR);
}
