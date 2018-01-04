
#include <idc.idc>
#include <functions.idc>

static main(void) {
	auto cdlFileName, cdlFile;
	cdlFileName = getCdlFilename();

	MakeNames();

	cdlFile = fopen(cdlFileName, "rb");
	if( cdlFile == 0 ) {
		cdlFileName = AskFile(0,"*.cdl","Choose a CDL-file Manually");
		cdlFile = fopen(cdlFileName, "rb");
	}

	if (cdlFile != 0) {
		auto cd, ncd, secondpass = 0;
		auto seg = 1, segea, segsize, segorg, segorgmask;
		auto codelog = 0, datalog = 0, pcmlog = 0, unusedlog = 0;
//		auto lastseg, lastsegorg, plastsegorg;
//		auto singleseg, singlesegorg;
//		auto s8, sA, sC, sE;

		segsize = getSegSize(seg);
		segorgmask = ~(segsize - 1);

		Message("CDL file \"%s\" opened succesfull, segment size = 0x%x\n", cdlFileName, segsize);

		Message("SEG analyze start\n");

		auto segcount = 1, onesingleseg = 0, onesingleorg = 0, miltisingleorg = 0;
		auto segs6 = 0, seg6 = 0;
		auto segs8 = 0, seg8 = 0;
		auto segsA = 0, segA = 0;
		auto segsC = 0, segC = 0;
		auto segsE = 0, segE = 0;
		auto segsmask = 0;

		while (SegByBase(segcount) != BADADDR) {
			auto sgo = getSegOrg(segcount);
			if(sgo == 0x6000) {
				segs6++;
				seg6 = segcount;
			} else if(sgo == 0x8000) {
				segs8++;
				seg8 = segcount;
			} else if(sgo == 0xA000) {
				segsA++;
				segA = segcount;
			} else if(sgo == 0xC000) {
				segsC++;
				segC = segcount;
			} else if(sgo == 0xE000) {
				segsE++;
				segE = segcount;
			}
			segcount++;
		}

		if(segs6 == 1) {
			miltisingleorg++;
			onesingleseg = seg6;
			onesingleorg = 0x6000;
			segsmask = segsmask | 1;
		}
		if(segs8 == 1) {
			miltisingleorg++;
			onesingleseg = seg8;
			onesingleorg = 0x8000;
			segsmask = segsmask | 2;
		}
		if(segsA == 1) {
			miltisingleorg++;
			onesingleseg = segA;
			onesingleorg = 0xA000;
			segsmask = segsmask | 4;
		}
		if(segsC == 1) {
			miltisingleorg++;
			onesingleseg = segC;
			onesingleorg = 0xC000;
			segsmask = segsmask | 8;
		}
		if(segsE == 1) {
			miltisingleorg++;
			onesingleseg = segE;
			onesingleorg = 0xE000;
			segsmask = segsmask | 16;
		}

		if(miltisingleorg == 0) {
			Message(" No single segments found!\n");
		} else if(miltisingleorg > 1) {
			Message(" Multiple single segments found: ");
			if(segs6 == 1) Message("segment %d ORG = 0x6000, ", seg6);
			if(segs8 == 1) Message("segment %d ORG = 0x8000, ", seg8);
			if(segsA == 1) Message("segment %d ORG = 0xA000, ", segA);
			if(segsC == 1) Message("segment %d ORG = 0xC000, ", segC);
			if(segsE == 1) Message("segment %d ORG = 0xE000", segE);
			Message("\n");
		} else {
			Message(" Single segment %d ORG = %04x!\n", onesingleseg, onesingleorg );
		}

		Message("SEG analyze is over\n");

		do {
			auto i, segeai, optype, opvalue, opsize = 0, lastop = 0, lastopaddr = 0, lastopaddrbit = 0, ofst, byteread = 0, firstoffset = 0xffff;
			segea = SegByBase(seg);
			segorg = getSegOrg(seg);
			i = 0;
			Message("Seg %08x %08x start pass %d\n", segea, segorg, secondpass);

			do {
				segeai = segea + i;

				if(secondpass == 0) {
					if(!ncd)
						cd = fgetc(cdlFile);
					else
						ncd = 0;
					if((cd & 3) == 2) {
						MakeComm(segeai, "");
						if(cd & 0x40) {
							MakeComm(segeai, "pcm data");
							MakeByte(segeai);
							pcmlog++;
							if(byteread > 0)
								byteread--;
						} else {
							if(cd & 0x80) {
								if (cd & 0x20)
									MakeComm(segeai, "ind data used at 6000");
								else
									MakeComm(segeai, "data used at 6000");
							} else {
								if (cd & 0x20)
									MakeComm(segeai, form("ind data used at %4x", ((cd & 0x0c) << 11) + 0x8000));
								else
									MakeComm(segeai, form("data used at %4x", ((cd & 0x0c) << 11) + 0x8000));
							}
							if(byteread == 0) {
								ofst = Byte(segeai);
								byteread++;
							}
							cd = fgetc(cdlFile);
							ncd = 1;
							ofst = ofst | (Byte(segeai + 1) << 8);
							byteread++;
							if(((cd & 3) == 2) && lastop && (lastopaddrbit == (i & 1)) && ((ofst & segorgmask) == segorg)) {
								if(isCode(GetFlags(segeai))) {
									MakeUnknown(segeai, 1, DOUNK_SIMPLE);
									MakeWord(segeai);
								} else if(!isData(GetFlags(segeai))) {
									MakeWord(segeai);
								}
								if((lastopaddr == i) || ((ofst & (segsize - 1)) < firstoffset))
									firstoffset = ofst & (segsize - 1);
								OpOff(segeai,0,MK_FP(AskSelector(seg),0));
								ncd = 0;
								i++;
								datalog++;
								byteread--;
								byteread--;
							} else {
								if(isCode(GetFlags(segeai))) {
									MakeUnknown(segeai, 1, DOUNK_SIMPLE);
									MakeByte(segeai);
								} else if(!isData(GetFlags(segeai))) {
									MakeByte(segeai);
								}
								byteread--;
								ofst = (ofst >> 8) & 0xFF;
							}
						}
						datalog++;
					} else if((cd & 3) == 0) {
						MakeComm(segeai, "");
						if(lastop && (lastopaddrbit == (i & 1)) && (i < firstoffset) && (!isCode(GetFlags(segeai)))) {
							if(byteread == 0) {
								ofst = Byte(segeai);
								byteread++;
							}
							cd = fgetc(cdlFile);
							ncd = 1;
							if((cd & 3) == 0) {
								ofst = ofst | (Byte(segeai + 1) << 8);
								byteread++;
								if((ofst & segorgmask) == segorg) {
									MakeUnknown(segeai, 2, DOUNK_SIMPLE);
									MakeWord(segeai);
									if((lastopaddr == i) || ((ofst & (segsize - 1)) < firstoffset))
										firstoffset = (ofst & (segsize - 1));
									OpOff(segeai,0,MK_FP(AskSelector(seg),0));
								}
								ncd = 0;
								i++;
								unusedlog++;
								byteread--;
								byteread--;
							}
						} else {
							if(byteread > 0)
								byteread--;
						}
						unusedlog++;
					} else {
						if(byteread > 0)
							byteread--;
						if(opsize < 2) {
							if(isData(GetFlags(segeai)))
								MakeUnknown(segeai, 3, DOUNK_SIMPLE);
							if((opsize = MakeCode(segeai)) == 0) {
								MakeUnknown(segeai, 3, DOUNK_SIMPLE);
								opsize = MakeCode(segeai);
							}
							lastop = Byte(segeai);
							lastop = (lastop == 0x20); // || (lastop == 0x6C);
							lastopaddr = (i + 3);
							lastopaddrbit = ((i + 3) & 1);
							firstoffset = 0xffff;
						}
						else
							opsize--;
						MakeComm(segeai, "");
						if((cd & 3) == 1) {
							if(cd & 0x80) {
								if(cd & 0x10)
									MakeComm(segeai, "ind code used at 6000");
								else
									MakeComm(segeai, "code used at 6000");
							} else {
								if(cd & 0x10)
									MakeComm(segeai, form("ind code used at %4x", ((cd & 0x0c) << 11) + 0x8000));
								else
									MakeComm(segeai, form("code used at %4x", ((cd & 0x0c) << 11) + 0x8000));
							}

						} else {
							if(cd & 0x80) {
								if((cd & 0x30)==0x00) MakeComm(segeai, "code used as data at 6000");
								else if((cd & 0x30)==0x10) MakeComm(segeai, "ind code used as data at 6000");
								else if((cd & 0x30)==0x20) MakeComm(segeai, "code used as ind data at 6000");
								else if((cd & 0x30)==0x30) MakeComm(segeai, "ind code used as ind data at 6000");
							}
							else {
								if((cd & 0x30)==0x00) MakeComm(segeai, form("code used as data at %4x", ((cd & 0x0c) << 11) + 0x8000));
								else if((cd & 0x30)==0x10) MakeComm(segeai, form("ind code used as data at %4x", ((cd & 0x0c) << 11) + 0x8000));
								else if((cd & 0x30)==0x20) MakeComm(segeai, form("code used as ind data at %4x", ((cd & 0x0c) << 11) + 0x8000));
								else if((cd & 0x30)==0x30) MakeComm(segeai, form("ind code used as ind data at %4x", ((cd & 0x0c) << 11) + 0x8000));
							}
							datalog++;
						}
						codelog++;
					}
				} else {
					auto opmasked;
					optype = GetOpType(segeai,0);
					if( (optype == 2) || (optype == 3) || (optype == 4)) {
						opvalue = GetOperandValue(segeai,0);
						opmasked = (opvalue & segorgmask);
						if(opmasked < 0x6000)
							OpOff(segeai,0,0);
						else if(opmasked == segorg)
							OpOff(segeai,0,MK_FP(AskSelector(seg),0));
						else if(miltisingleorg > 1)
							makeOffsetMulti(opmasked, segeai, seg6, seg8, segA, segC, segE, segsmask);
						else if((miltisingleorg == 1)&&(opmasked == onesingleorg))
							OpOff(segeai,0,MK_FP(AskSelector(onesingleseg),0));
						else if((opmasked < 0x8000) && (seg6 == 0))
							OpOff(segeai,0,0);
					} else if(optype == 7) {
						auto opcode = Byte(segeai);
						if((opcode == 0x20) || (opcode == 0x4C)) {
							opvalue = Word(segeai + 1);
							opmasked = (opvalue & segorgmask);
							if(opmasked < 0x6000)
								OpOff(segeai,0,0);
							else if(opmasked == segorg)
								OpOff(segeai,0,MK_FP(AskSelector(seg),0));
							else if(miltisingleorg > 1)
								makeOffsetMulti(opmasked, segeai, seg6, seg8, segA, segC, segE, segsmask);
							else if((miltisingleorg == 1)&&(opmasked == onesingleorg))
								OpOff(segeai,0,MK_FP(AskSelector(onesingleseg),0));
							else if((opmasked < 0x8000) && (seg6 == 0))
								OpOff(segeai,0,0);
						}
					}
				}
				i++;
			} while (i < segsize);
			if(secondpass == 1) {
				secondpass = 0;
				seg++;
			} else {
				secondpass = 1;
				Wait();
			}
		} while (SegByBase(seg) != BADADDR);

		segea = SegByBase(seg - 1) + segsize;
		MakeUnknown(segea - 6, 6, DOUNK_SIMPLE);
		MakeWord(segea - 6);
		if(!OpOff(segea - 6,0,MK_FP(AskSelector(seg - 1),0)))
			OpOff(segea - 6,0,MK_FP(AskSelector(seg - 2),0));
		MakeWord(segea - 4);
		if(!OpOff(segea - 4,0,MK_FP(AskSelector(seg - 1),0)))
			OpOff(segea - 4,0,MK_FP(AskSelector(seg - 2),0));
		MakeWord(segea - 2);
		if(!OpOff(segea - 2,0,MK_FP(AskSelector(seg - 1),0)))
			OpOff(segea - 2,0,MK_FP(AskSelector(seg - 2),0));

		Message("CDL file has been processed successfully.\n");
		Message("	CODE bytes:   0x%x\n",codelog);
		Message("	DATA bytes:   0x%x (including PCM data bytes: 0x%x)\n",datalog,pcmlog);
		Message("	UNUSED bytes: 0x%x\n",unusedlog);
	} else {
		Message("Can't open CDL file: \"%s\"\n", cdlFileName);
		return -1;
	}
}

static getCdlFilename() {
	auto extPos, idbFileName = GetIdbPath();
	extPos = strstr(idbFileName, ".idb");
	return substr(idbFileName, 0, extPos) + ".cdl";
}

static MakeNames() {
	MakeName(0x2000,"_PPU_CTRL");
	MakeName(0x2001,"_PPU_MASK");
	MakeName(0x2002,"_PPU_STATUS");
	MakeName(0x2003,"_PPU_OAM_ADDR");
	MakeName(0x2004,"_PPU_OAM_DATA");
	MakeName(0x2005,"_PPU_SCROLL");
	MakeName(0x2006,"_PPU_ADDR");
	MakeName(0x2007,"_PPU_DATA");

	MakeName(0x4000,"_APU_PULSE1_ENV");
	MakeName(0x4001,"_APU_PULSE1_SWEEP");
	MakeName(0x4002,"_APU_PULSE1_TIME");
	MakeName(0x4003,"_APU_PULSE1_LEN");
	MakeName(0x4004,"_APU_PULSE2_ENV");
	MakeName(0x4005,"_APU_PULSE2_SWEEP");
	MakeName(0x4006,"_APU_PULSE2_TIME");
	MakeName(0x4007,"_APU_PULSE2_LEN");
	MakeName(0x4008,"_APU_TRI_COUNT");
	MakeName(0x4009,"_APU_TRI_UNUSED");
	MakeName(0x400a,"_APU_TRI_TIME");
	MakeName(0x400b,"_APU_TRI_LEN");
	MakeName(0x400c,"_APU_NOISE_ENV");
	MakeName(0x400d,"_APU_NOISE_UNUSED");
	MakeName(0x400e,"_APU_NOISE_LOOP");
	MakeName(0x400f,"_APU_NOISE_LEN");
	MakeName(0x4010,"_APU_DMC_FREQ");
	MakeName(0x4011,"_APU_DMC_LOAD");
	MakeName(0x4012,"_APU_DMC_ADDR");
	MakeName(0x4013,"_APU_DMC_LEN");
	MakeName(0x4014,"_PPU_OAM_DMA");
	MakeName(0x4015,"_APU_STATUS");
	MakeName(0x4016,"_PAD0_REG");
	MakeName(0x4017,"_PAD1_REG");
}

static makeOffsetMulti(opmasked, segeai, seg6, seg8, segA, segC, segE, segsmask) {
	if(opmasked < 0x6000)
		OpOff(segeai,0,0);
	else if((segsmask & 1) && (opmasked == 0x6000))
		OpOff(segeai,0,MK_FP(AskSelector(seg6),0));
	else if((segsmask & 2) && (opmasked == 0x8000))
		OpOff(segeai,0,MK_FP(AskSelector(seg8),0));
	else if((segsmask & 4) && (opmasked == 0xA000))
		OpOff(segeai,0,MK_FP(AskSelector(segA),0));
	else if((segsmask & 8) && (opmasked == 0xC000))
		OpOff(segeai,0,MK_FP(AskSelector(segC),0));
	else if((segsmask & 16) && (opmasked == 0xE000))
		OpOff(segeai,0,MK_FP(AskSelector(segE),0));
	else if((opmasked < 0x8000) && (seg6 == 0))
		OpOff(segeai,0,0);
}