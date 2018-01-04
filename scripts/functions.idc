
// SCRIPT PARSE HELPERS
// -------------------

static make_auto_ofs16_gb(ea) {
	auto word0, bank0, base;
	MakeWord(ea);
	word0 = Word(ea);
	bank0 = GetReg(ea,"ds");
	if(bank0>0) {
		if(word0<0x4000)
			base = MK_FP(AskSelector(0), 0);
		else
			base = MK_FP(AskSelector(bank0), 0);
	}
	else {
		if(word0<0x4000)
			base = MK_FP(AskSelector(0), 0);
		else
			base = -1;
	}
	if(base!=-1)
		OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
}

static make_auto_ofs16_nes(ea) {
	auto ofs = Word(ea), bank = GetReg(ea,"ds")-1;
	if(ofs<0x800)
		ea = make_offset_ex(ea, 0, 0, 1);
	else if(ofs>=0xC000) {
		ea = make_offset_ex(ea, 7, 0, 1);
	} else {
		if((bank==7)&&(ofs<0xC000)) {
			ea = make_offset_ex(ea, AskLong(0, "Enter Bank Nunber"), 0, 1);
		} else
			ea = make_offset_ex(ea, bank, 0, 1);
	}
	return ea;
}

static auto_far_ptr() {
	auto ea = ScreenEA();
	auto_far_ptr_ex(ea,4,0);
}

static auto_far_ptr_ex(ea,mode,arg) {
	auto bofs, wofs, base, size;
	auto bank0 = GetReg(ea,"ds");

//	Message("far ptr at 0x%08x, mode %d, arg %d\n",ea,mode,arg);

	if(mode==11) {
		auto cnt0, num_lines = Byte(ea), cnt1, startea = ea+1;
		MakeUnknown(ea, 1, DOUNK_SIMPLE);
		MakeByte(ea);
		for(cnt0=0; cnt0<num_lines; cnt0++) {
			cnt1=0;
			while((Byte(startea+cnt1)!=0)&&(cnt1<16))cnt1++;
			if(cnt1<16)cnt1++;
			MakeUnknown(startea, cnt1, DOUNK_SIMPLE);
			MakeStr(startea,startea+cnt1);
			MakeNameEx(startea,"",SN_CHECK|SN_NOWARN);
			startea=startea+cnt1;
		}
	} else if(mode==10) {
		MakeUnknown(ea, 38, DOUNK_SIMPLE);
		make_data_array(ea,3,"");
		MakeWord(ea+3);
		far_ptr(ea+3, -2, 0, 0, 0);
		MakeWord(ea+5);
		far_ptr(ea+5, -3, 0, 0, 0);
		MakeWord(ea+7);
		far_ptr(ea+7, -7, 0, 0, 0);
		MakeWord(ea+9);
		far_ptr(ea+9, -9, 0, 0, 0);
		MakeWord(ea+11);
		far_ptr(ea+11, -11, 0, 0, 0);
		MakeWord(ea+13);
		far_ptr(ea+13, -13, 0, 0, 0);
		MakeWord(ea+15);
		far_ptr(ea+15, -15, 0, 0, 0);
		MakeWord(ea+17);
		far_ptr(ea+17, -17, 0, 0, 0);
		MakeWord(ea+19);
		far_ptr_code(ea+19, -19, 0, 0, 0);
		MakeWord(ea+21);
		far_ptr(ea+21, -21, 0, 0, 0);
		MakeWord(ea+23);
		MakeWord(ea+25);
		MakeWord(ea+27);
		MakeWord(ea+29);
		far_ptr(ea+29, -29, 0, 0, 0);
		MakeWord(ea+31);
		far_ptr(ea+31, -31, 0, 0, 0);
		MakeWord(ea+33);
		far_ptr(ea+33, -33, 0, 0, 0);
		MakeWord(ea+35);
		far_ptr(ea+35, -35, 0, 0, 0);
		MakeByte(ea+37);
//		Jump(ea+38);
	} else if(mode==9) {
		auto cnt;
//		ea=make_data_array(ea,2,"");
//		while (Byte(ea)&0x30) {
//		Message("start from 0x%08x\n",ea);
	    for(cnt=0;cnt<arg;cnt++) {
			MakeUnknown(ea, 15, DOUNK_SIMPLE);
			ea=make_data_array(ea,10,"");
			MakeWord(ea);
			wofs=Word(ea);
			if(wofs<0x4000)
				base = MK_FP(AskSelector(0), 0);
			else
				base = MK_FP(AskSelector(bank0), 0);
			if(wofs!=0) {
				OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
				MakeCode(base + wofs);
				AutoMark(base + wofs, AU_CODE);
			}
			MakeWord(ea+2);
			wofs=Word(ea+2);
			if(wofs<0x4000)
				base = MK_FP(AskSelector(0), 0);
			else
				base = MK_FP(AskSelector(bank0), 0);
			if(wofs!=0) {
				OpOffEx(ea + 2, 0, REF_OFF16, -1, base, 0);
				MakeCode(base + wofs);
				AutoMark(base + wofs, AU_CODE);
			}
			MakeByte(ea+4);
			ea=ea+5;
		}
//			ea=make_data_array(ea,1,"");
//			if(Word(ea)<0xA000)
//				ea=make_offset_ex(ea,0,0,1);
//			else if(Word(ea)<0xC000)
//				ea=make_offset_ex(ea,1,0,1);
//			else
//				ea=make_offset_ex(ea,2,0,1);
//		}
//		ea=make_data_array(ea,1,"");
//		if(Word(ea)<0xA000)
//				ea=make_offset_ex(ea,0,0,1);
//		else if(Word(ea)<0xC000)
//				ea=make_offset_ex(ea,1,0,1);
//		else
//				ea=make_offset_ex(ea,2,0,1);
//		Jump(ea);
	} else if(mode==8) {
		ea=FindBinary(ea+1, SEARCH_DOWN|SEARCH_CASE, "FA FF 7F 67 2E ?? E5 75 C3 ?? ??");
		if(ea!=BADADDR) {
			MakeUnknown(ea, 11, DOUNK_SIMPLE);
			MakeCode(ea);
			AutoMark(ea, AU_CODE);
			Wait();
			far_ptr_code(ea, 5, 9, 1, -1);
			Jump(ea+10);
		}
	} else if(mode==7) {
		auto bt;
		while ((bt=Byte(ea))!=0xFF) {
			MakeUnknown(ea, 7, DOUNK_SIMPLE);
			MakeByte(ea+0);
			MakeWord(ea+1);
			MakeWord(ea+3);
			MakeWord(ea+5);
			far_ptr(ea+5, -5, 0, 0, 0);
			ea=ea+7;
		}
		MakeUnknown(ea, 3, DOUNK_SIMPLE);
		MakeByte(ea+0);
		MakeWord(ea+1);
		far_ptr(ea+1, -1, 0, 0, 0);
		Jump(ea+3);
	} else if(mode==6) {
		size = 9;
		MakeUnknown(ea, size, DOUNK_SIMPLE);
		MakeByte(ea+0);
		MakeWord(ea+1);
		far_ptr(ea+1, 2, 0, 0, 0);
		MakeByte(ea+3);
		MakeWord(ea+4);
		far_ptr(ea+4, 2, 0, 0, 0);
		MakeByte(ea+6);
		MakeWord(ea+7);
		far_ptr(ea+6, 0, 1, 0, 0);
		Wait();
		Jump(ea+size);
	// --
	} else if(mode == 5) {
		auto loofs = 2;
		auto hiofs = 1;
		size = 3;
		MakeUnknown(ea, size, DOUNK_SIMPLE);
		MakeByte(ea);
		MakeByte(ea+1);
		MakeByte(ea+2);
		auto addr = Byte(ea+loofs)|(Byte(ea+hiofs)<<8);
		base = MK_FP(AskSelector(Byte(ea)), 0);
		OpOffEx(ea + loofs, 0, REF_LOW8, base + addr, base, 0);
		OpOffEx(ea + hiofs, 0, REF_HIGH8, base + addr, base, 0);
		Wait();
		Jump(ea + size);
	} else if(mode == 4) {
	// simple far link
		auto base0, ofs0, name;
		ea = -1;
		while ((ea=FindBinary(ea+1, SEARCH_DOWN, "21 ?? ?? DF"))!=BADADDR) {
			if(isCode(GetFlags(ea)))
				MakeUnknown(ea, 3, DOUNK_SIMPLE);
			MakeCode(ea);
			AutoMark(ea, AU_CODE);
			Wait();
//			Message("ea=0x%08x ", ea);
			base0 = MK_FP(AskSelector(Byte(ea+2)), 0);
//			Message("base=0x%08x ", base0);
			ofs0 = base0 + 0x4001 + (Byte(ea+1)<<1);
//			Message("ofs0=0x%08x ", ofs0);
			ofs0 = base0 + Word(ofs0);
//			Message("ofs1=0x%08x\n", ofs0);
			MakeCode(ofs0);
			AutoMark(ofs0, AU_CODE);
			Wait();
			name = NameEx(BADADDR, ofs0);
			if(name == "") {
				MakeComm(ea, "");
				MakeComm(ea, form("0x%08x",ofs0));
			} else {
				MakeComm(ea, "");
				MakeComm(ea, form("%s", name));
			}
			AddCodeXref(ea, ofs0, fl_CF);
		}
	} else if(mode == 3) {
		bofs = 0;
		wofs = 2;
		size = 4;
		MakeUnknown(ea, size, DOUNK_SIMPLE);
		MakeWord(ea + wofs);
		MakeWord(ea + bofs);

		if(Word(ea + wofs)!=0) {
			far_ptr(ea, bofs, wofs, 0, 0);

			auto bb = Byte(ea + bofs);
			auto ww = Word(ea + wofs);
			base = MK_FP(AskSelector(bb), 0);
			auto ptr = base + ww;
			auto cc;

			while ((cc=Byte(ptr))!=0) {
				auto tmp1 = Word(ptr+1);
				MakeUnknown(ptr, 3, DOUNK_SIMPLE);
				MakeByte(ptr);
				MakeWord(ptr+1);
				OpOffEx(ptr+1, 0, REF_OFF16, -1, base, 0);
				Message(" %d sub at 0x%08x\n", cc, base + tmp1);
				MakeCode(base + tmp1);
				AutoMark(base + tmp1, AU_CODE);
				Wait();
				ptr=ptr+3;
			}
		}

		Jump(ea + size);
	} else if(mode == 2) {
	// custom far resources	bank/ofs/dst/size
		size = 10;
		MakeUnknown(ea, size, DOUNK_SIMPLE);
		MakeByte(ea + 0);
		MakeByte(ea + 1);
		MakeWord(ea + 2);
		far_ptr(ea, 1, 2, 0, 0);
		MakeWord(ea + 4);
		MakeWord(ea + 6);
		far_ptr(ea, 1, 6, 0, 0);
		MakeWord(ea + 8);
		Wait();
		Jump(ea + size);
	// --
	} else if(mode == 1) {
	// custom far resources	bank/ofs/dst/size
		auto i,j,k;
		i=Byte(ea);
		MakeWord(ea);
		ea=ea+2;
		j=0;
		while(j<i) {
			size = 6;
			MakeUnknown(ea, size, DOUNK_SIMPLE);
			MakeWord(ea + 0);
			MakeWord(ea + 2);
			MakeWord(ea + 4);
			far_ptr(ea, 3, 4, 0, 0);
			ea = ea + size;
			j++;
		}
		Jump(ea);
	// --
	} else {
	// simple far link
		bofs = 2;
		wofs = 0;
//		if(Word(ea)!=0) {
		size = 3;
		MakeUnknown(ea, size, DOUNK_SIMPLE);
//		MakeData(ea, FF_BYTE, 10, 0);
//		SetArrayFormat(ea, AP_IDXHEX, 10, 0);
//		MakeWord(ea + 0);
//		MakeWord(ea + 2);
		MakeByte(ea + bofs);
//		MakeWord(ea + 1);
		MakeWord(ea + wofs);
//		MakeWord(ea + wofs + 2);
//		MakeWord(ea + wofs + 4);
		far_ptr(ea, bofs, wofs, 0, 0);
//		far_ptr_code(ea, bofs, wofs + 2, 0, 0);
//		MakeWord(ea + 3);
//		MakeWord(ea + 5);
//		MakeByte(ea + 7);
//		Wait();
//		Message("mode 0\n");
		Jump(ea + size);
//		} else {
//			MakeUnknown(ea, 2, DOUNK_SIMPLE);
//			MakeWord(ea + 0);
//			Jump(ea + 2);
//		}
	// --
	}
}

static make_data_array(ea, len, cmt) {
	MakeUnknown(ea, len, DOUNK_SIMPLE);
	MakeData(ea, FF_BYTE, len, 0);
	SetArrayFormat(ea, AP_IDXHEX, 16 /*len*/, 0);
	MakeComm(ea, "");
	MakeComm(ea, cmt);
	return ea + len;
}

static make_offset(ea, bank) {
	return make_offset_ex(ea, bank, 0, 0);
}

static make_code(ea) {
	auto len = 2;
	while((MakeCode(ea)==0)&&(len<4)) {
		MakeUnknown(ea, len, DOUNK_SIMPLE);
		len++;
	}
	AutoMark(ea, AU_CODE);
	Wait();
}

static make_offset_ex(ea, bank, execute, nomsg) {
	auto base, ptr = Word(ea);
	MakeUnknown(ea, 2, DOUNK_SIMPLE);
	MakeData(ea, FF_WORD, 1, 0);
	if(ptr < 0x8000)
		base = MK_FP(AskSelector(0), 0);
	else
		base = MK_FP(AskSelector(bank + 1), 0);
	OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
	if(execute) {
		make_code(base + ptr);
		Wait();
	}
	if(!nomsg) {
		MakeComm(ea, "");
		MakeComm(ea,  form("ofs 0x%08x", base + ptr));
	}
	return ea + 2;
}

static make_offset_gb(ea, bank) {
	auto base, ptr = Word(ea);
	MakeUnknown(ea, 2, DOUNK_SIMPLE);
	MakeData(ea, FF_WORD, 1, 0);
	base = MK_FP(AskSelector(bank), 0);
	OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
//	MakeComm(ea, "");
//	MakeComm(ea,  form("ofs 0x%08x", base + ptr));
	return ea + 2;
}

static make_offset_gb_code(ea, bank) {
	auto base, ptr = Word(ea);
	MakeUnknown(ea, 2, DOUNK_SIMPLE);
	MakeData(ea, FF_WORD, 1, 0);
	base = MK_FP(AskSelector(bank), 0);
	OpOffEx(ea, 0, REF_OFF16, -1, base, 0);
	make_code(base + ptr);
//	MakeCode(base + ptr);
//	AutoMark(base + ptr, AU_CODE);
	Wait();
//	MakeComm(ea, "");
//	MakeComm(ea,  form("ofs 0x%08x", base + ptr));
	return ea + 2;
}

static make_name(ea, bank, name) {
	auto base, ptr = Word(ea);
	if(ptr < 0x8000)
		base = MK_FP(AskSelector(0), 0);
	else
		base = MK_FP(AskSelector(bank + 1), 0);
	MakeName(base + ptr, "");
	MakeName(base + ptr, form("%s%X", name, base + ptr));
}



// BASE SYSTEM FUNCTIONS!

static getSegSize(seg) {
	auto j, segstart, segea;
	for (j=0x8000; j<0x10000; j=j+0x1000) {
		segea = MK_FP(AskSelector(seg),j);
		segstart = SegStart(segea);
		if(segstart != BADADDR) {
			return SegEnd(segea)-segstart;
		}
	}
	segea = MK_FP(AskSelector(seg),0x6000);
	segstart = SegStart(segea);
	if(segstart != BADADDR) {
		return SegEnd(segea)-segstart;
	}
	return -1;
}

static getSegOrg(seg) {
	auto k, segstart, segea;
	for (k=0x8000; k<0x10000; k=k+0x1000) {
		segea = MK_FP(AskSelector(seg),k);
		if(SegStart(segea) != BADADDR) {
			return k;
		}
	}
	segea = MK_FP(AskSelector(seg),0x6000);
	if(SegStart(segea) != BADADDR) {
		return 0x6000;
	}
	return -1;
}

// PARAMETRIC FUNCTIONS

static parametric_fixsize(opcode, size) {
	auto ea = -1;
	Message(" Fixsize search for \"%s\" start!\n", opcode);
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN|SEARCH_CASE, opcode);
		if(ea != BADADDR) {
			Message(" >found one at 0x%08x\n", ea);
			MakeUnknown(ea, size + 3 + 3, DOUNK_SIMPLE);
			if(size == 2)
				MakeWord(ea + 3);
			else
				MakeData(ea + 3, FF_BYTE, size, 0);
			MakeCode(ea);
			AutoMark(ea + size + 3, AU_CODE);
		}
	} while (ea != BADADDR);
	Wait();
	Message(" Done!\n");
}

static parametric_farcall(opcode, size, segmask, bankop, ofsop, delta, shift, dobankbyte) {
	auto ea = -1;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN|SEARCH_CASE, opcode);
		if(ea != BADADDR) {
			auto bank, base, offs;
//			Message("Pattern found at %08x\n", ea);
			MakeUnknown(ea, size + 3, DOUNK_SIMPLE);

			offs = Word(ea + ofsop);
//			if(offs < 0xA000)
				bank = ((Byte(ea + bankop) << shift) & segmask) + 1;
//			else
//				bank = ((Byte(ea + bankop) << shift) & segmask) + 2;
			if(dobankbyte)
				MakeByte(ea + bankop);
			base = MK_FP(AskSelector(bank), 0);
			MakeWord(ea + ofsop);
			OpOffEx(ea + ofsop, 0, REF_OFF16, -1, base, delta);

			MakeCode(ea);
			AutoMark(ea + size + 3, AU_CODE);
			MakeCode(ea + size + 3);
			AutoMark(base + offs, AU_CODE);
			MakeCode(base + offs);
			Wait();
		}
	} while (ea != BADADDR);
//	Message("Done\n");
}

static parametric_stopbytes(opcode, stopbytes) {
	auto size, db, ea = -1;
//	Message("Start at %08x stopbyte %s %s\n", ea, opcode, stopbytes);
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN|SEARCH_CASE, opcode);
		if(ea != BADADDR) {
//			Message("Pattern found at %08x\n", ea);
			size = FindBinary(ea + 3, SEARCH_DOWN|SEARCH_CASE, stopbytes);
			MakeUnknown(ea, size - ea + 3, DOUNK_SIMPLE);
			MakeData(ea + 3, FF_BYTE, size - ea - 2, 0);
			MakeCode(ea);
			//AutoMark(size + 3, AU_CODE);
			Wait();
		}
	} while (ea != BADADDR);
//	Message("Done\n");
}

static parametric_switch(opcode) {
	auto ea = -1;
//	Message("Start at %08x switch for\n", ea);
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN|SEARCH_CASE, opcode);
		if(ea != BADADDR) {
			make_16bit_near_tbl(ea, 0);
			MakeUnknown(ea, 3, DOUNK_SIMPLE);
			MakeCode(ea);
			AutoMark(ea, AU_CODE);
			Wait();
		}
	} while (ea != BADADDR);
//	Message("Done\n");
}

// MAKE TABLES HELPERS

static make_16bit_near_tbl(ea, ofs) {
	auto base, minea, curea, maxea, size, dw;
	base = MK_FP(AskSelector(GetSegmentAttr(ea, SEGATTR_SEL)), 0);
	maxea = base + 0xFFFF;
	size = 0;
	do {
		dw = Word(ea + size + 3);
		curea = base + dw;
		if(curea < maxea) maxea = curea;
		MakeUnknown(ea + size + 3, 3, DOUNK_SIMPLE);
		MakeWord(ea + size + 3);
//		OpOffEx(ea + size + 3, 0, REF_OFF16, curea - ofs, base, ofs);
		OpOffEx(ea + size + 3, 0, REF_OFF16, -1, base, ofs);
		MakeUnknown(curea - ofs, 3, DOUNK_SIMPLE);
		MakeCode(curea - ofs);
		AutoMark(curea - ofs, AU_CODE);
		size = size + 2;
	} while (ea + size + 3 < (maxea - ofs));
	Wait();
	MakeCode(ea + size + 3);
	AutoMark(ea + size + 3, AU_CODE);
	MakeCode(maxea - ofs);
	AutoMark(maxea - ofs, AU_CODE);
	Wait();
}

static make_8bit_ofs(ea, loofs, hiofs, ofs) {
	auto base = BADADDR, addr = Byte(ea + loofs + 1) | (Byte(ea + hiofs + 1) << 8);
	if(addr >= 0x8000)
		base = MK_FP(AskSelector(GetSegmentAttr(ea, SEGATTR_SEL)), 0);
	else if((addr >= 0x6000)||(addr < 0x800))
		base = MK_FP(AskSelector(GetSegmentAttr(addr, SEGATTR_SEL)), 0);
	if(base != BADADDR) {
		OpOffEx(ea + loofs, 0, REF_LOW8, base + addr - ofs, base, ofs);
		OpOffEx(ea + hiofs, 0, REF_HIGH8, base + addr - ofs, base, ofs);
	} else
		Message(" 8bit offset in bad range!\n");
}

static make_8bit_far_ofs(ea, loofs, hiofs, ofs, bank) {
	auto base, addr = Byte(ea + loofs + 1) | (Byte(ea + hiofs + 1) << 8);
	base = MK_FP(AskSelector(bank), 0);
	OpOffEx(ea + loofs, 0, REF_LOW8, base + addr - ofs, base, ofs);
	OpOffEx(ea + hiofs, 0, REF_HIGH8, base + addr - ofs, base, ofs);
}

static make_8bit_near_tbl(loaddr, hiaddr, ofs, code) {
	auto i, size;
	if(hiaddr > loaddr)
		size = hiaddr - loaddr;
	else
		size = loaddr - hiaddr;
	MakeUnknown(loaddr, size, DOUNK_SIMPLE);
	MakeUnknown(hiaddr, size, DOUNK_SIMPLE);
	for(i=0; i<size; i++) {
		auto lptr, hptr, base, bank, refptr;
		lptr = Byte(loaddr + i);
		hptr = Byte(hiaddr + i);
		refptr = lptr | (hptr << 8);
//		if(refptr < 0x6000)
//			base = MK_FP(AskSelector(GetSegmentAttr(0, SEGATTR_SEL)), 0);
//		else
		if(((loaddr>0x4000)&&(refptr>0x4000))||((loaddr<0x4000)&&(refptr<0x4000)))
			base = MK_FP(AskSelector(GetSegmentAttr(loaddr, SEGATTR_SEL)), 0);
		else if((loaddr>0x4000)&&(refptr<0x4000))
			base = MK_FP(AskSelector(0), 0);
		else if((loaddr<0x4000)&&(refptr>0x4000))
			base = MK_FP(AskSelector(1), 0);
		MakeByte(loaddr + i);
		OpOffEx(loaddr + i, 0, REF_LOW8, base + refptr - ofs, base, ofs);
		MakeByte(hiaddr + i);
		OpOffEx(hiaddr + i, 0, REF_HIGH8, base + refptr - ofs, base, ofs);
		if (code)
			AutoMark(base + refptr - ofs, AU_CODE);
	}
}

static make_8bit_far_tbl(loaddr, hiaddr, bankaddr, nsize, delta) {
	auto i, size, bank; // = bankaddr;
	if(nsize == 0)
		size = hiaddr - loaddr;
	else
		size = nsize;

	MakeUnknown(loaddr, size, DOUNK_SIMPLE);
	MakeUnknown(hiaddr, size, DOUNK_SIMPLE);
	MakeUnknown(bankaddr, size, DOUNK_SIMPLE);
	for(i=0; i<size; i++) {
		auto lptr, hptr, base, refptr, bankn;
		lptr = Byte(loaddr + i);
		hptr = Byte(hiaddr + i);
		refptr = lptr | (hptr << 8);

//		bank = bankaddr;
//		bank = (Byte(bankaddr + i) >> 5);
//		bank = (bank >> 5) + 1;

		bank = Byte(bankaddr + i);
//		if(!(i&1))
//			bank = bank >> 4;
//		bank = bank & 15;

		if(refptr < 0xC000)
			base = MK_FP(AskSelector(bank + 1), 0);
		else
			base = MK_FP(AskSelector(8), 0);

		MakeByte(loaddr + i);
		OpOffEx(loaddr + i, 0, REF_LOW8, base + refptr + delta, base, delta);
		MakeByte(hiaddr + i);
		OpOffEx(hiaddr + i, 0, REF_HIGH8, base + refptr + delta, base, delta);
		MakeByte(bankaddr + (i >> 1));
//		AutoMark(base + refptr, AU_CODE);
/*
	MakeUnknown(loaddr, size, DOUNK_SIMPLE);
	MakeUnknown(hiaddr, size, DOUNK_SIMPLE);
	for(i=0; i<size; i++) {
		auto lptr, hptr, base, refptr, bankn;
		lptr = Byte(loaddr + i);
		hptr = Byte(hiaddr + i);
		refptr = (lptr | ((hptr & 0x3F) << 8))|0x8000;
		bank = (hptr >> 6)&3;
		base = MK_FP(AskSelector(bank + 1), 0);
		MakeByte(loaddr + i);
		OpOffEx(loaddr + i, 0, REF_LOW8, base + refptr, base, 0);
		MakeByte(hiaddr + i);
		OpOffEx(hiaddr + i, 0, REF_HIGH8, base + refptr, base, (0x8000-((hptr & 0xC0)<< 8)) - base);
*/
	}
}

// HOTKEY FUNCTIONS

static make_8bit_ofs_selection2() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 0, 2, 0);
}

static make_8bit_far_ofs_selection2() {
	auto ea = ScreenEA();
	auto bank = AskLong(0, "Enter Bank Nunber");
	if(ea != BADADDR)
		make_8bit_far_ofs(ea, 0, 2, 0, bank + 1);
}

static make_8bit_ofs_selection2_i() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 2, 0, 0);
}

static make_8bit_far_ofs_selection2_i() {
	auto ea = ScreenEA();
	auto bank = AskLong(0, "Enter Bank Nunber");
	if(ea != BADADDR)
		make_8bit_far_ofs(ea, 2, 0, 0, bank + 1);
}

static make_8bit_ofs_selection3() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 0, 3, 0);
}

static make_8bit_ofs_selection3_i() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 3, 0, 0);
}

static make_8bit_far_ofs_selection3_i() {
	auto ea = ScreenEA();
	auto bank = AskLong(0, "Enter Bank Nunber");
	if(ea != BADADDR)
		make_8bit_far_ofs(ea, 3, 0, 0, bank + 1);
}

static make_8bit_ofs_selection4() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 0, 4, 0);
}

static make_8bit_far_ofs_selection4() {
	auto ea = ScreenEA();
	auto bank = AskLong(0, "Enter Bank Nunber");
	if(ea != BADADDR)
		make_8bit_far_ofs(ea, 0, 4, 0, bank + 1);
}

static make_8bit_ofs_selection4_i() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 4, 0, 0);
}

static make_8bit_ofs_selection5() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 0, 5, 0);
}

static make_8bit_far_ofs_selection5() {
	auto ea = ScreenEA();
	auto bank = AskLong(0, "Enter Bank Nunber");
	if(ea != BADADDR)
		make_8bit_far_ofs(ea, 0, 5, 0, bank + 1);
}

static make_8bit_ofs_selection5_i() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 5, 0, 0);
}

static make_8bit_ofs_selection6() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 0, 6, 0);
}

static make_8bit_ofs_selection6_i() {
	auto ea = ScreenEA();
	if(ea != BADADDR)
		make_8bit_ofs(ea, 6, 0, 0);
}

static make_8bit_far_ofs_selection6() {
	auto ea = ScreenEA();
	auto bank = AskLong(0, "Enter Bank Nunber");
	if(ea != BADADDR)
		make_8bit_far_ofs(ea, 0, 6, 0, bank + 1);
}

static make_8bit_near_selection() {
	auto sea = SelStart();
	auto eea = SelEnd();
	if((sea != BADADDR) && (eea != BADADDR)) {
		auto mea = (sea + eea) / 2;
		make_8bit_near_tbl(sea, mea, 0, 0);
	}
}

static make_8bit_near_selection_d() {
	auto sea = SelStart();
	auto eea = SelEnd();
	if((sea != BADADDR) && (eea != BADADDR)) {
		auto mea = (sea + eea) / 2;
		make_8bit_near_tbl(sea, mea, -1, 1);
	}
}

static make_8bit_near_selection_i() {
	auto sea = SelStart();
	auto eea = SelEnd();
	auto mea;
	if((sea == BADADDR) || (eea = BADADDR)) {
		sea = ScreenEA();
/* FAR NES!!
		auto addr = Byte(sea + 1) | (Byte(sea) << 8);
		auto bank = AskLong(0, "Enter Bank Nunber");
		auto base = MK_FP(AskSelector(bank + 1), 0);
		MakeUnknown(sea, 2, DOUNK_SIMPLE);
		MakeByte(sea);
		MakeByte(sea + 1);
		OpOffEx(sea + 0, 0, REF_HIGH8, base + addr, base, 0);
		OpOffEx(sea + 1, 0, REF_LOW8, base + addr, base, 0);
// NEAR
/*/
		mea = sea + 1;
		make_8bit_near_tbl(mea, sea, 0, 0);
//*/
		Jump(sea + 2);
	} else {
		mea = (sea + eea) / 2;
		make_8bit_near_tbl(mea, sea, 0, 0);
	}
}

static make_8bit_near_selection_d_i() {
	auto sea = SelStart();
	auto eea = SelEnd();
	if((sea != BADADDR) && (eea != BADADDR)) {
		auto mea = (sea + eea) / 2;
		make_8bit_near_tbl(mea, sea, -1, 1);
	}
}

static make_8bit_ofs_gb_common(ea,loofs,hiofs) {
	auto ofs = 0;
	auto base, addr = Byte(ea + loofs + 1) | (Byte(ea + hiofs + 1) << 8);
	if(addr < 0x4000)
		base = MK_FP(AskSelector(GetSegmentAttr(0, SEGATTR_SEL)), 0);
	else if((addr>=0xC000)&&(addr<0xD000))
		base = MK_FP(AskSelector(SegByName("RAM0")),0);
	else if((addr>=0xD000)&&(addr<0xE000))
		base = MK_FP(AskSelector(SegByName("RAMB")),0);
	else {
		if(ea < 0x4000) {
			base = MK_FP(AskSelector(AskLong(0, "Enter Bank Nunber")), 0);
		} else {
			base = MK_FP(AskSelector(GetSegmentAttr(ea, SEGATTR_SEL)), 0);
		}
	}
//	Message("addr = %08x, base = %08x\n",addr,base);
	OpOffEx(ea + loofs, 1, REF_LOW8, base + addr - ofs, base, ofs);
	OpOffEx(ea + hiofs, 1, REF_HIGH8, base + addr - ofs, base, ofs);
}

static make_8bit_ofs_gb_2() {
	make_8bit_ofs_gb_common(ScreenEA(),0,2);
}

static make_8bit_ofs_gb_2_i() {
	make_8bit_ofs_gb_common(ScreenEA(),2,0);
}

static make_8bit_ofs_gb_3() {
	make_8bit_ofs_gb_common(ScreenEA(),0,3);
}

static make_8bit_ofs_gb_3_i() {
	make_8bit_ofs_gb_common(ScreenEA(),3,0);
}

static make_8bit_ofs_gb_4() {
	make_8bit_ofs_gb_common(ScreenEA(),0,4);
}

static make_8bit_ofs_gb_4_i() {
	make_8bit_ofs_gb_common(ScreenEA(),4,0);
}

static make_8bit_ofs_gb_5() {
	make_8bit_ofs_gb_common(ScreenEA(),0,5);
}

static make_8bit_ofs_gb_5_i() {
	make_8bit_ofs_gb_common(ScreenEA(),5,0);
}

static make_8bit_ofs_gb_auto() {
	auto loofs = 0;
	auto sofs = 2, hiofs = -1;
	auto ofs = 0;
	auto ea = ScreenEA();
	auto base, addr;
	while(sofs < 8) {
		if(GetOpType(ea + sofs,0)==1) {
			if(Byte(ea + sofs)==0xCE)
				hiofs = sofs;
		}
		sofs++;
	}
	if(hiofs != -1) {
		make_8bit_ofs_gb_common(ea,loofs,hiofs);
	}
}

static find_near_tbls() {
	auto b0, b1, base, loaddr, hiaddr, ea, screa = ScreenEA();

	// DO SWITCHES JMP ($XX) TYPE
	// LDA $XXXX/+1,X/Y
	// STA $YY
	// LDA $XXXX/+1,X/Y
	// STA $YY+1
	// ..
	// JMP ($YY)
	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "?? ?? ?? 85 ?? ?? ?? ?? 85");
		if(ea != BADADDR) {
			auto first_byte_ofs = FindBinary(ea + 10, SEARCH_DOWN | SEARCH_CASE, "6C ?? 00");
			if(first_byte_ofs != BADADDR) {
				first_byte_ofs = first_byte_ofs - ea;
				if(first_byte_ofs < 20) {
					b0 = Byte(ea + 0);
					b1 = Byte(ea + 5);
					if((b0 == b1) && ((b0 == 0xBD) || (b0 == 0xB9))) {
						auto ptr_ofs0, ptr_ofs1, ptr_first_byte;
						MakeUnknown(ea, 12, DOUNK_SIMPLE);
						AutoMark(ea, AU_CODE);
						Wait();
						base = MK_FP(AskSelector(GetSegmentAttr(ea, SEGATTR_SEL)), 0);
						ptr_first_byte = Byte(ea + first_byte_ofs + 1);
						ptr_ofs0 = Byte(ea + 4);
						ptr_ofs1 = Byte(ea + 9);
						if(ptr_ofs0 == ptr_first_byte) {
							loaddr = base + Word(ea + 1);
							hiaddr = base + Word(ea + 6);
						} else if(ptr_ofs1 == ptr_first_byte) {
							loaddr = base + Word(ea + 6);
							hiaddr = base + Word(ea + 1);
						}
						if((hiaddr-loaddr)>1) {
							Message("8-bit near indirect jump found at 0x%08x base = %08x lo %08x hi %08x sz %04x\n", ea, base, loaddr, hiaddr, hiaddr-loaddr);
							make_8bit_near_tbl(loaddr, hiaddr, 0, 1);
						} else {
							Message("16-bit near indirect jump found at 0x%08x base = %08x ea %08x\n", ea, base, loaddr);
							make_16bit_near_tbl(loaddr-3, 0);
						}
					}
				}
			}
		}
	} while (ea != BADADDR);

	// DO SWITCHES RET TYPE
	// LDA $XXXX+1,X/Y
	// PHA
	// LDA $XXXX,X/Y
	// PHA
	// RET
	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "?? ?? ?? 48 ?? ?? ?? 48 60");
		if(ea != BADADDR) {
			b0 = Byte(ea + 0);
			b1 = Byte(ea + 4);
			if((b0 == b1) && ((b0 == 0xBD) || (b0 == 0xB9))) {
				MakeUnknown(ea, 9, DOUNK_SIMPLE);
				AutoMark(ea, AU_CODE);
				Wait();
				base = MK_FP(AskSelector(GetSegmentAttr(ea, SEGATTR_SEL)), 0);
				loaddr = base + Word(ea + 5);
				hiaddr = base + Word(ea + 1);
				if((hiaddr-loaddr)>1) {
					Message("8-bit RET indirect jump found at 0x%08x base = %08x lo %08x hi %08x sz %04x\n", ea, base, loaddr, hiaddr, hiaddr-loaddr);
					make_8bit_near_tbl(loaddr, hiaddr, -1, 1);
				} else {
					Message("16-bit RET indirect jump found at 0x%08x base = %08x ea %08x\n", ea, base, loaddr);
					make_16bit_near_tbl(loaddr-3, -1);
				}
			}
		}
	} while (ea != BADADDR);

	// DO STACK JUMP
	// LDA $HI
	// PHA
	// LDA $LO
	// PHA
	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "A9 ?? 48 A9 ?? 48");
		if(ea != BADADDR) {
			Message("test 0x%08x\n", ea);
			make_8bit_ofs(ea, 3, 0, -1);
		}
	} while (ea != BADADDR);

	// DO OFFSET
	// LDA #$XX
	// STA $YY
	// LDA #$ZZ
	// STA $YY+1
	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "A9 ?? 85 ?? A9 ?? 85 ??");
		if(ea != BADADDR) {
			if((Byte(ea + 7) - Byte(ea + 3)) == 1) {
				MakeUnknown(ea, 8, DOUNK_SIMPLE);
				AutoMark(ea, AU_CODE);
				Wait();
				make_8bit_ofs(ea, 0, 4, 0);
				ea = ea + 8;
			}
		}
	} while (ea != BADADDR);

	// DO OFFSET
	// LDX #$XX
	// LDY #$YY
	// JSR $ZZZZ / STX + STY
	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "A2 ?? A0 ?? 20");
		if(ea != BADADDR) {
			MakeUnknown(ea, 4, DOUNK_SIMPLE);
			AutoMark(ea, AU_CODE);
			Wait();
			make_8bit_ofs(ea, 0, 2, 0);
		}
	} while (ea != BADADDR);

	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "A2 ?? A0 ?? 8E ?? ?? 8C ?? ??");
		if(ea != BADADDR) {
			MakeUnknown(ea, 4, DOUNK_SIMPLE);
			AutoMark(ea, AU_CODE);
			Wait();
			make_8bit_ofs(ea, 0, 2, 0);
		}
	} while (ea != BADADDR);

	ea = screa;
	do {
		ea = FindBinary(ea + 1, SEARCH_DOWN | SEARCH_CASE, "A2 ?? A0 ?? 84 ?? 86");
		if(ea != BADADDR) {
			MakeUnknown(ea, 4, DOUNK_SIMPLE);
			AutoMark(ea, AU_CODE);
			Wait();
			make_8bit_ofs(ea, 0, 2, 0);
		}
	} while (ea != BADADDR);

	// TODO
	// BD C5 A5 LDA off_3E5C5,X
	// 85 00    STA word_0
	// E8       INX
	// BD C5 A5 LDA off_3E5C5,X
	// 85 01    STA word_0+1
	// 6C 00 00 JMP (word_0)


	Message("Done\n");
}

static auto_array() {
	auto ea = ScreenEA();
	auto i = 1;
	ea = FindUnexplored(ea - 1, SEARCH_DOWN);
//	Message("start ea %08x", ea);
	do {
		auto flags = GetFlags(ea + i);
		auto stop = isUnknown(flags) && !isRef(flags);
		i++;
	} while ((flags != 0) && (stop));
	i=i-1;
//	Message(" stop ea %08x, size = %d\n", ea + i - 1, i);
	if(MakeData(ea, FF_BYTE, i, 0)) {
		if(SetArrayFormat(ea, AP_IDXHEX, 0x10, 0))
			Jump(ea);
		else
			Message("can't set array format!\n");

	} else
		Message("can't make data!\n");
}


static ptr_table(ea, docode) {
	auto i = 0, w0, stop = 0;
	auto base = MK_FP(AskSelector(GetSegmentAttr(ea, SEGATTR_SEL)), 0);
	do {
		if((i&1)==0){
			MakeUnknown(ea+i, 2, DOUNK_SIMPLE);
			MakeWord(ea+i);
			w0=Word(ea+i);
			OpOffEx(ea+i, 0, REF_OFF16, -1, base, 0);
			if(docode) {
//				Message("try to do code at 0x%08x\n",base+w0);
//				MakeUnknown(base+w0, 3, DOUNK_SIMPLE);
				make_code(base+w0);
//				MakeCode(base+w0);
//				AutoMark(base+w0, AU_CODE);
				Wait();
			}
			Wait();
		}
		i++;
		if((i&1)==0) {
			auto tmpp=GetFlags(ea + i);
			stop=(isRef(tmpp));//||isCode(tmpp));
		}
//		Message("stop = %d\n",stop);
	} while (!stop && (i < 0x10000));
}

static ptr_table_far(ea, farbank, docode) {
	auto i = 0, w0, stop = 0;
	auto base, eabank = GetSegmentAttr(ea, SEGATTR_SEL);
	do {
		if((i&1)==0){
			MakeUnknown(ea+i, 2, DOUNK_SIMPLE);
			MakeWord(ea+i);
			w0=Word(ea+i);
			if(w0<0x4000) {
				base = MK_FP(AskSelector(0), 0);
			} else if(w0<0x8000) {
				if((eabank>0)||(farbank==-1))
					base = MK_FP(AskSelector(eabank), 0);
				else
					base = MK_FP(AskSelector(farbank), 0);
			}
			OpOffEx(ea+i, 0, REF_OFF16, -1, base, 0);
			if(docode) {
//				Message("try to do code at 0x%08x\n",base+w0);
//				MakeUnknown(base+w0, 3, DOUNK_SIMPLE);

//				make_code(base+w0);	// too many broken code if table detect failed
				MakeCode(base+w0);
				AutoMark(base+w0, AU_CODE);
				Wait();
			}
			Wait();
		}
		i++;
		if((i&1)==0) {
			auto tmpp=GetFlags(ea + i);
			stop=(isRef(tmpp));//||isCode(tmpp));
		}
//		Message("stop = %d\n",stop);
	} while (!stop && (i < 0x10000));
}

static auto_ptr_table() {
	ptr_table(ScreenEA(),0);
}

static far_ptr(ea, bofs, wofs, op, opofs) {
	auto base, bank_delta=0;
	if(SegByName(form("ROM%X",0))==BADADDR)
		bank_delta=1;
	if (bank_delta==0) {
		auto ofs = Word(ea + wofs);
		if(ofs<0x4000)
			base = MK_FP(AskSelector(0), 0);
		else {
			if(bofs==-1)
				base = MK_FP(AskSelector(GetReg(ea,"ds")), 0);
			else
				base = MK_FP(AskSelector(Byte(ea + bofs)), 0);
		}
	} else
		base = MK_FP(AskSelector(Byte(ea + bofs) + bank_delta), 0);
	OpOffEx(ea + wofs + opofs, op, REF_OFF16, -1, base, 0);
}

static far_ptr_code(ea, bofs, wofs, op, opofs) {
	auto base;
	auto ofs = Word(ea + wofs);
	if(ofs<0x4000)
		base = MK_FP(AskSelector(0), 0);
	else {
		if(bofs==-1)
			base = MK_FP(AskSelector(GetReg(ea,"ds")), 0);
		else
			base = MK_FP(AskSelector(Byte(ea + bofs)), 0);
	}
	OpOffEx(ea + wofs + opofs, op, REF_OFF16, -1, base, 0);
	make_code(base + ofs);
	Wait();
}

// duplicate code marker
static duplicate_marker() {

	auto max_treshold = 0x10;

	auto i, j, result, treshold, starti, zero_cnt, one_cnt;
	auto numbank_src, bankea_src, banksize_src, byte_src, res_src_idx;
	auto numbank_dst, bankea_dst, banksize_dst, byte_dst, res_dst_idx;
	auto numbank_delta;

	if(SegByName(form("ROM%X",0)) == BADADDR) {
		numbank_src = 1;
		numbank_delta = 1;
	} else {
		numbank_src = 0;
		numbank_delta = 0;
	}
	while ((bankea_src = SegByBase(numbank_src)) != BADADDR) {
		banksize_src = getSegSize(numbank_src);
		res_src_idx = form("|%1x",numbank_src-numbank_delta);
		Message("bank %02x found: ea = %04x, size = %04x\n  testing:", numbank_src-numbank_delta, bankea_src, banksize_src);
		numbank_dst = numbank_src + 1;
		while ((bankea_dst = SegByBase(numbank_dst)) != BADADDR) {
//			if(numbank_src != numbank_dst)
			{
				banksize_dst = getSegSize(numbank_dst);
				/* compare here */
				if(banksize_src == banksize_dst) {
					Message(" %02x", numbank_dst-numbank_delta);
					res_dst_idx = form("|%1x",numbank_dst-numbank_delta);
					treshold = 0;
					zero_cnt = 0;
					one_cnt = 0;
					for(i = 0; i < banksize_src; i++) {
						byte_src = Byte(bankea_src + i);
						byte_dst = Byte(bankea_dst + i);
						if(byte_src == byte_dst) {
							if(treshold == 0) {
								starti = i;
							} else if(treshold >= max_treshold) {
								if((zero_cnt + one_cnt) != treshold) {
									if(treshold == max_treshold) {
										Message("\n  match found at 0x%08x\n testing:",starti + bankea_src);
										for(j=0;j<max_treshold;j++) {
											result = CommentEx(bankea_src + starti + j, 0) + res_dst_idx;
											MakeComm(bankea_src + starti + j, "");
											MakeComm(bankea_src + starti + j, result);
											result = CommentEx(bankea_dst + starti + j, 0) + res_src_idx;
											MakeComm(bankea_dst + starti + j, "");
											MakeComm(bankea_dst + starti + j, result);
										}
									}
									result = CommentEx(bankea_src + i, 0) + res_dst_idx;
									MakeComm(bankea_src + i, "");
									MakeComm(bankea_src + i, result);
									result = CommentEx(bankea_dst + i, 0) + res_src_idx;
									MakeComm(bankea_dst + i, "");
									MakeComm(bankea_dst + i, result);
								}
							}
							treshold++;
							if(byte_src == 0)
								zero_cnt++;
							else if(byte_src == 0xff)
								one_cnt++;
						} else {
							treshold = 0;
							zero_cnt = 0;
							one_cnt = 0;
						}
					}
				}
				/* compare here */
			}
			numbank_dst++;
		}
		Message("\n");
		numbank_src++;
	}
}


// autorename ptrs
static auto_rename_ptrs() {
	auto sea = SelStart();
	auto eea = SelEnd();
	auto bank = GetReg(sea,"ds");
	auto extbank = -1, extbase = -1;
	auto base = MK_FP(AskSelector(bank), 0);
	auto name_mask = AskStr("", "Enter Name Mask");
	auto i = 0;
	while(sea < eea) {
		auto ofs = Word(sea);
		auto nname = form("%s%02X",name_mask,i);
		if(SegByName(form("ROM%X",0))!=BADADDR) {	// GB mode
			if(bank>0){
				if(ofs<0x4000) {
					MakeNameEx(0+ofs,nname,SN_CHECK|SN_NOWARN);
				} else if(ofs<0x8000) {
					MakeNameEx(base+ofs,nname,SN_CHECK|SN_NOWARN);
				}
			} else {
				if(ofs<0x4000) {
					MakeNameEx(0+ofs,nname,SN_CHECK|SN_NOWARN);
				} else if(ofs<0x8000){
					if(extbank==-1) {
						extbank = AskLong(1, "Enter External Bank");
						extbase = MK_FP(AskSelector(extbank), 0);
					}
					MakeNameEx(extbase+ofs,nname,SN_CHECK|SN_NOWARN);
				}
			}
		} else {									// NES mode (MMC1, 128K)
			if(bank<8){
				if(ofs<0xC000) {
					MakeNameEx(base+ofs,nname,SN_CHECK|SN_NOWARN);
				} else {
					MakeNameEx(MK_FP(AskSelector(8), 0)+ofs,nname,SN_CHECK|SN_NOWARN);
				}
			} else {
				if(ofs<0xC000) {
					if(extbank==-1) {
						extbank = AskLong(1, "Enter External Bank");
						extbase = MK_FP(AskSelector(extbank), 0);
					}
					MakeNameEx(extbase+ofs,nname,SN_CHECK|SN_NOWARN);
				} else {
					MakeNameEx(MK_FP(AskSelector(8), 0)+ofs,nname,SN_CHECK|SN_NOWARN);
				}
			}
		}
		sea=sea+2;
		i++;
	}
}
