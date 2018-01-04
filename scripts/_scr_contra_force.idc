#include <idc.idc>
#include <functions.idc>

// ----------------------------
// CONTRA FORCE snd script
// ----------------------------

static cf_snd_opcode_name(cmd) {
	auto ret = "";
	if((cmd >= 0xF6)&&(cmd <= 0xFB)) {
		ret = "begin loopA";
	} else if(cmd == 0xFC) {
		ret = "begin loopB";
	} else
		ret = form("cmd_%02x", cmd);
	return ret;
}

static cf_snd_parse(ea, chan) {
//	auto ea = ScreenEA();
//	ea = FindUnexplored(ea - 1, SEARCH_DOWN);
	auto cmd, sp = 0, stop = 0, byteseq = 0;
	auto jmp_ptr, jmp_base;
	auto adjseg, seg = GetReg(ea,"ds") - 1;
	if (Byte(ea) & 0x80) {
		// MODE 0 (music)
		do {
			cmd = Byte(ea);
			if(cmd < 0xDF) { 			// data
				ea = make_data_array(ea,1,"data1");
			} else if((cmd >= 0xED)&&(cmd <= 0xFA)) {// loop no args
				ea = make_data_array(ea,1,cf_snd_opcode_name(cmd));
			} else if((cmd == 0xE1)||(cmd == 0xE2)||(cmd == 0xE4)||(cmd == 0xE5)||(cmd == 0xE6)||(cmd == 0xE8)||(cmd == 0xE9)||(cmd == 0xEA)) {	// 1 byte arg
				ea = make_data_array(ea,2,cf_snd_opcode_name(cmd));
			} else if((cmd == 0xE3)||(cmd == 0xE7)) {	// 2 byte arg
				ea = make_data_array(ea,3,cf_snd_opcode_name(cmd));
			} else if(cmd == 0xE0) {					// ch0/1 - 4 bytes arg, ch2 - 2 bytes arg, ch3 - 1 byte arg
				if(chan < 2)
					ea = make_data_array(ea,5,cf_snd_opcode_name(cmd));
				else if (chan == 2)
					ea = make_data_array(ea,3,cf_snd_opcode_name(cmd));
				else if (chan == 3)
					ea = make_data_array(ea,2,cf_snd_opcode_name(cmd));
			} else if((cmd == 0xEB)||(cmd == 0xEC)) {	// 1/3 bytes arg
				if(Byte(ea+1) == 0) {
					ea = make_data_array(ea,2,cf_snd_opcode_name(cmd));
				} else {
					ea = make_data_array(ea,4,cf_snd_opcode_name(cmd));
				}
			} else { 					// cmds
				if((cmd == 0xFB)||(cmd == 0xFC)) {	// 1 byte arg
					ea = make_data_array(ea,1,cf_snd_opcode_name(cmd));
				} else if(cmd == 0xFD) {					// 2 byte jsr
					ea = make_data_array(ea,1,"jsr");
//					if(((ea - 1) & 0x1FFF) == 0x1FFF) {
//						seg++;
//						ea = NextSeg(ea);
//					}
					adjseg = seg;
//					if(!(Word(ea) & 0x2000) && ((seg & 1) == 1)) {
//						adjseg--;
//					} else if((Word(ea) & 0x2000) && ((seg & 1) == 0)) {
//						adjseg++;
//					}
					jmp_ptr = Word(ea) + MK_FP(AskSelector(adjseg + 1), 0);
					ea = make_offset(ea,adjseg);
					if(sp == 0) {
						sp = ea;
						ea = jmp_ptr;
					} else {
						Message("can't jump inside subroutine at 0x%08x!\n",ea);
						stop = 1;
					}
				} else if(cmd == 0xFE) {					// loop 1/3 byte arg
					if(Byte(ea+1)==0xFF){
						ea = make_data_array(ea,2,"jmp");
						adjseg = seg;
//						if((!(Word(ea) & 0x2000)) && (seg & 1)) {
//							adjseg--;
//						} else if((Word(ea) & 0x2000) && !(seg & 1)) {
//							adjseg++;
//						}
						ea = make_offset(ea,adjseg);
						stop = 1;
					} else {
						ea = make_data_array(ea,2,"do loop");
					}
				} else if(cmd == 0xFF) {					// ret / stop
					if(sp != 0) {
						ea = make_data_array(ea,1,"ret");
						ea = sp;
						sp = 0;
					} else {
						ea = make_data_array(ea,1,"stop");
						stop = 1;
					}
				} else {
					Message(" unk_cmd at 0x%08x\n",ea);
					stop = 1;
				}
			}
		} while (!stop);
		// END MODE 0 (music)
	} else {
		// MODE 1 (sounds)
		do {
			cmd = Byte(ea);
			if(cmd == 0) { 			// data
				ea = make_data_array(ea,1,"data1");
				byteseq = 0;
			} else if(cmd < 0x10) { 			// data
				if(chan == 5)
					ea = make_data_array(ea,1,"data1");
				else
					ea = make_data_array(ea,3,"data3");
				byteseq = 0;
			} else if(cmd < 0xE8) { 	// data
				if(byteseq || ((chan == 5) && (cmd != 0x10)))
					ea = make_data_array(ea,1,"data1");
				else
					ea = make_data_array(ea,2,"data2");
			} else if(((cmd >= 0xE8)&&(cmd <= 0xEB))||(cmd == 0xED)) { 	// cmd1
				if(chan == 5)
					ea = make_data_array(ea,1,"data1");
				else
					ea = make_data_array(ea,2,cf_snd_opcode_name(cmd));
			} else if(cmd <= 0xEF) { 	// cmd1
				if(chan == 5)
					ea = make_data_array(ea,1,"data1");
				else {
					ea = make_data_array(ea,1,cf_snd_opcode_name(cmd));
					if(cmd == 0xEC)
						byteseq = 1;
				}
			} else if((cmd >= 0xF0)&&(cmd <= 0xFA)) { 	// data
				if(byteseq | (chan == 5))
					ea = make_data_array(ea,1,"data1");
				else
					ea = make_data_array(ea,2,"data2");
			} else { 					// cmds
				byteseq = 0;
				if((cmd == 0xFB)||(cmd == 0xFC)) {	// 1 byte arg
					ea = make_data_array(ea,1,cf_snd_opcode_name(cmd));
				} else if(cmd == 0xFD) {					// 2 byte jsr
					ea = make_data_array(ea,1,cf_snd_opcode_name(cmd));
					adjseg = seg;
// TMNT III не хочет такой аджуст
//					if(!(Word(ea) & 0x2000) && ((seg & 1) == 1)) {
//						adjseg--;
//					} else if((Word(ea) & 0x2000) && ((seg & 1) == 0)) {
//						adjseg++;
//					}
					jmp_ptr = Word(ea) + MK_FP(AskSelector(adjseg + 1), 0);
					ea = make_offset(ea,adjseg);
					if(sp == 0) {
						sp = ea;
						ea = jmp_ptr;
					} else {
						Message("can't jump inside subroutine at 0x%08x!\n",ea);
						stop = 1;
					}
				} else if(cmd == 0xFE) {					// loop 1/3 byte arg
					if(Byte(ea+1)==0xFF){
						ea = make_data_array(ea,2,"jmp");
						adjseg = seg;
//						if((!(Word(ea) & 0x2000)) && (seg & 1)) {
//							adjseg--;
//						} else if((Word(ea) & 0x2000) && !(seg & 1)) {
//							adjseg++;
//						}
						ea = make_offset(ea,adjseg);
						stop = 1;
					} else {
						ea = make_data_array(ea,2,"do loop");
					}
				} else if(cmd == 0xFF) {					// ret / stop
					if(sp != 0) {
						ea = make_data_array(ea,1,"ret");
						ea = sp;
						sp = 0;
					} else {
						ea = make_data_array(ea,1,"stop");
						stop = 1;
					}
				} else {
					Message(" unk_cmd at 0x%08x\n",ea);
					stop = 1;
				}
			}
		} while (!stop);
		// END MODE 1 (sounds)
	}
}

static cf_snd_lib_parse(void) {
	auto ea = ScreenEA();
	auto chan = Byte(ea) & 0xF;
	auto seg = GetReg(ea,"ds") - 1, adjseg;
	auto ofs = Word(ea + 1);

// CF
//	if(ofs < 0xA000)
//		adjseg = seg;
//	else if(ofs < 0xC000)
//		adjseg = seg + 1;
//	else if(ofs < 0xE000)
//		adjseg = 14;

// TMNT III
	if(ofs < 0xC000)
		adjseg = seg;
	else
		adjseg = seg + 1;

	ea = make_data_array(ea,1,"");
	ea = make_offset(ea, adjseg);

	auto snd_ptr = ofs + MK_FP(AskSelector(adjseg + 1), 0);
	cf_snd_parse(snd_ptr, chan);

	Jump(ea);
}

static tmnt3_mus_parse(void) {
	auto ea = ScreenEA();
	auto seg = GetReg(ea,"ds") - 1, adjseg;

	auto chan = 0;
	do {
		auto ofs = Word(ea);
//		if(ofs < 0xC000)
//			adjseg = seg;
//		else
//			adjseg = seg + 1;
		adjseg = 0x1A;
		auto snd_ptr = ofs + MK_FP(AskSelector(adjseg + 1), 0);
		cf_snd_parse(snd_ptr, chan);
		ea = ea + 2;
		chan = chan + 1;
	} while (chan < 4);
	
	Jump(ea);
}

static cf_spr_format(void) {
	auto ea = SelStart();
	auto eea = SelEnd();
	auto seg = GetReg(ea,"ds") - 1, segadj;
	Message("CF spr format start\n");
	if((ea != BADADDR) && (eea != BADADDR)) {
		do {
			auto ofs = Word(ea);
			if(((ofs & 0x2000) == 0x2000) && ((seg & 1) == 0))
				segadj = seg + 1;
			else if(((ofs & 0x2000) == 0x0000) && ((seg & 1) == 1))
				segadj = seg -1;
			else
				segadj = seg;
			auto spr_ptr = ofs + MK_FP(AskSelector(segadj + 1), 0);
			Message(" CF spr 0x%08x parse\n",spr_ptr);
			do {
				auto spr_cmd = Byte(spr_ptr);
				if(spr_cmd != 0xFF) {
					Message("  CF spr cmd %02x make\n",spr_cmd);
					spr_ptr = make_data_array(spr_ptr, ((spr_cmd & 0x7F) * 3) + 2, "");
				} else {
					Message("  CF spr link\n");
					spr_cmd = 0;
				}
			} while (spr_cmd & 0x80);
			ea = ea + 2;
		} while (ea < eea);
		Message("CF spr format end\n");
	} else
		Message("CF spr format error\n");
}

static main(void) {
//	AddHotkey("Shift-X", "cf_snd_lib_parse");
	AddHotkey("Shift-X", "tmnt3_mus_parse");
}
