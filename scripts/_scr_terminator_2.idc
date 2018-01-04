#include <idc.idc>
#include <functions.idc>

// -----------------------------
// TERMINATOR 2 GRAPHICS SCRIPTS
// -----------------------------
static term2_scripts_autosearch(void) {
	auto ea = 0;
//	auto scr_jsr = "20 56 CA";	// release
	auto scr_jsr = "20 A3 C8";	// proto

	Message(" Start\n");
	do {
		Message(" Search from 0x%08x\n", ea);
		ea = FindBinary(ea, SEARCH_DOWN, scr_jsr);

		if(ea != BADADDR) {
			Message(" Found at 0x%08x\n", ea);
//			ea = term2_release_script_parse(ea + 3);
			ea = term2_proto_script_parse(ea + 3);
		}
	} while (ea != BADADDR);

	Message("Done\n");
}

static term2_release_script_parse(ea) {
	auto data_len = 0;
	auto data_ea = 0;
	auto script_break = 0;
	auto scr_len_list_ea = 0xB4B5D;
	do {
		auto cmd = Byte(ea);
		auto cmd_len;
		if(cmd & 0x80) { 				// cmd
			if(data_len) {
				Message(" +data array size=%0x\n", data_len);
				MakeUnknown(data_ea, data_len, DOUNK_SIMPLE);
				MakeData(data_ea, FF_BYTE, data_len, 0);
				SetArrayFormat(data_ea, AP_IDXHEX, data_len, 0);
				data_ea = 0;
				data_len = 0;
			}
			if(cmd == 0x88)
				cmd_len = 3;
			else
				cmd_len = Byte(scr_len_list_ea + (cmd & 0x7F)) + 1;
			if(cmd == 0x83) {			// break
				script_break = 1;
				Message(" break at 0x%08x\n", ea);
				MakeUnknown(ea, 1, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, 1, 0);
				SetArrayFormat(ea, AP_IDXHEX, 1, 0);
				ea = ea + 1;
			} else if (cmd == 0x8B) {	// tlm ofs
				Message(" cmd_tlm\n");
				MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
				MakeByte(ea);
				MakeWord(ea+1);
				auto base = MK_FP(AskSelector(3), 0);
				OpOffEx(ea+1, 0, REF_OFF16, -1, base, 0);
				SetArrayFormat(ea+1, AP_IDXHEX, 1, 0);
				MakeWord(ea+3);
				SetArrayFormat(ea+3, AP_IDXHEX, 1, 0);
				ea = ea + 5;
			} else {
				Message(" cmd_%02x, size=%02x\n",cmd,cmd_len);
				MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, cmd_len, 0);
				SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
				ea = ea + cmd_len;
			}
		} else {		 // raw data
			if(!data_len)
				data_ea = ea;
			data_len = data_len + 1;
			ea = ea + 1;
		}
	} while (!script_break);
	return ea;
}

static term2_proto_script_parse(ea) {
	auto data_len = 0;
	auto data_ea = 0;
	auto script_break = 0;
	do {
		auto cmd = Byte(ea);
		if(cmd & 0x80) { 				// cmd
			if(data_len) {
				Message(" +data array size=%0x\n", data_len);
				MakeUnknown(data_ea, data_len, DOUNK_SIMPLE);
				MakeData(data_ea, FF_BYTE, data_len, 0);
				SetArrayFormat(data_ea, AP_IDXHEX, data_len, 0);
				data_ea = 0;
				data_len = 0;
			}
			if(cmd == 0x83) {
				script_break = 1;
				Message(" break at 0x%08x\n", ea);
				MakeUnknown(ea, 1, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, 1, 0);
				SetArrayFormat(ea, AP_IDXHEX, 1, 0);
				ea = ea + 1;
			} else if((cmd == 0x80)||(cmd == 0x82)||(cmd == 0x85)||(cmd == 0x86)||(cmd == 0x89)||(cmd == 0x8D)) {
				Message(" cmd_%02x, size=2\n",cmd);
				MakeUnknown(ea, 2, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, 2, 0);
				SetArrayFormat(ea, AP_IDXHEX, 2, 0);
				ea = ea + 2;
			} else if((cmd == 0x81)||(cmd == 0x84)||(cmd == 0x87)||(cmd == 0x88)||(cmd == 0x8A)||(cmd == 0x8C)) {
				Message(" cmd_%02x, size=3\n",cmd);
				MakeUnknown(ea, 3, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, 3, 0);
				SetArrayFormat(ea, AP_IDXHEX, 3, 0);
				ea = ea + 3;
			} else if(cmd == 0x8B) {
				Message(" cmd_tlm\n");
				MakeUnknown(ea, 5, DOUNK_SIMPLE);
				MakeByte(ea);
				MakeWord(ea+1);
				auto base = MK_FP(AskSelector(4), 0);
				OpOffEx(ea+1, 0, REF_OFF16, -1, base, 0);
				SetArrayFormat(ea+1, AP_IDXHEX, 1, 0);
				MakeWord(ea+3);
				SetArrayFormat(ea+3, AP_IDXHEX, 1, 0);
				ea = ea + 5;
			} else {
				Message(" unk_cmd!\n");
				script_break = 1;
			}
		} else {		 // raw data
			if(!data_len)
				data_ea = ea;
			data_len = data_len + 1;
			ea = ea + 1;
		}
	} while (!script_break);
	return ea;
}

// -------------------------------
// TERMINATOR 2 MUSIC DATA PARSER
// -------------------------------
static term2_proto_apu_ch0() {
	auto cmd_82 = 0;
	auto cmd_len, cmd, stop = 0;
	auto _sp = 0, _s0, _s1, _s2;
	auto ea = SelStart();
	auto eea = SelEnd();
	auto base, ptr, i;
	if((ea != BADADDR) && (eea != BADADDR)) {
//		Message(" run from %08x to %08x!\n", ea, eea);
		do {
			cmd = Byte(ea);
			if(cmd & 0x80) {
				if((cmd == 0x81)||(cmd == 0x83)||(cmd == 0x85)||(cmd == 0x90)) {
					cmd_len = 1;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					if(cmd == 0x83) {
						MakeComm(ea, "");
						MakeComm(ea, "stop");
//						Message(" stop\n");
						stop = 1;
					} else if(cmd == 0x85) {
						MakeComm(ea, "");
						MakeComm(ea, "ret");
//						Message(" ret\n");
						if(_sp == 3) {
							ea = _s2;
						} else if(_sp == 2) {
							ea = _s1;
						} else if(_sp == 1) {
							ea = _s0;
						} else {
							Message(" can't do ret!\n");
							stop = 1;
						}
						_sp--;
					} else {
						MakeComm(ea, "");
						MakeComm(ea, form("cmd_%02x", cmd));
//						Message(" cmd_%02x\n", cmd);
						ea = ea + cmd_len;
					}
				} else if((cmd == 0x80)||(cmd == 0x82)||(cmd == 0x86)||(cmd == 0x87)||(cmd == 0x8B)||(cmd == 0x8D)||(cmd == 0x8E)||(cmd == 0x93)||(cmd == 0x94)||(cmd == 0x95)) {
					cmd_len = 2;
					MakeComm(ea, "");
					if(cmd == 0x82) {
						cmd_82 = Byte(ea + 1);
						if(cmd_82) {
//							Message(" mode8\n");
							MakeComm(ea, "mode8");
						} else {
//							Message(" mode16\n");
							MakeComm(ea, "mode16");
						}
					} else {
//						Message(" cmd_%02x\n", cmd);
						MakeComm(ea, form("cmd_%02x", cmd));
					}
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					ea = ea + cmd_len;
				} else if((cmd == 0x84)||(cmd == 0x88)) {
					ptr = Word(ea + 1);
					cmd_len = 3;
					MakeUnknown(ea, 3, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, 1, 0);
					MakeData(ea + 1, FF_WORD, 1, 0);
					if(ptr < 0xA000)
						base = MK_FP(AskSelector(0x9), 0);
					else
						base = MK_FP(AskSelector(0xA), 0);
					OpOffEx(ea + 1, 0, REF_OFF16, -1, base, 0);
					MakeComm(ea, "");
					MakeComm(ea + 1, "");
					if(cmd == 0x88) {
						stop = 1;
//						Message(" jmp to 0x%08x\n", base + ptr);
						MakeComm(ea, form("jmp 0x%08x", base + ptr));
					} else {
//						Message(" jsr to 0x%08x\n", base + ptr);
						MakeComm(ea, form("jsr 0x%08x", base + ptr));
						if(_sp == 0) {
							_s0 = ea + cmd_len;
						} else if(_sp == 1) {
							_s1 = ea + cmd_len;
						} else if(_sp == 2) {
							_s2 = ea + cmd_len;
						} else {
							Message(" out of stack!\n");
							stop = 1;
						}
						ea = base + ptr;
						_sp++;
					}
				} else if((cmd == 0x8A)||(cmd == 0x91)||(cmd == 0x92)) {
					cmd_len = 4;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					if(cmd == 0x92) {
						MakeData(ea, FF_BYTE, 2, 0);
						SetArrayFormat(ea, AP_IDXHEX, 2, 0);
						ptr = Word(ea + 2);
						MakeData(ea + 2, FF_WORD, 1, 0);
						if(ptr < 0xA000)
							base = MK_FP(AskSelector(0x9), 0);
						else
							base = MK_FP(AskSelector(0xA), 0);
						OpOffEx(ea + 2, 0, REF_OFF16, -1, base, 0);
						MakeComm(ea, "");
						MakeComm(ea + 2, "");
						MakeComm(ea, form("cmd_%02x arg_ofs 0x%08x", cmd, base + ptr));
						i = 0;
						while (Byte(base + ptr + i) != 0xFF) i++;
						MakeUnknown(base + ptr, i + 1, DOUNK_SIMPLE);
						MakeData(base + ptr, FF_BYTE, i + 1, 0);
						SetArrayFormat(base + ptr, AP_IDXHEX, i + 1, 0);
					} else {
						MakeData(ea, FF_BYTE, cmd_len, 0);
						SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
						MakeComm(ea, "");
						MakeComm(ea, form("cmd_%02x", cmd));
					}
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else if(cmd == 0x8C) {
					if(Byte(ea + 1))
						cmd_len = 4;
					else
						cmd_len = 2;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					MakeComm(ea, "");
					MakeComm(ea, form("cmd_%02x", cmd));
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else if(cmd == 0x8F) {
					if(Byte(ea + 1))
						cmd_len = 5;
					else
						cmd_len = 2;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					MakeComm(ea, "");
					MakeComm(ea, form("cmd_%02x", cmd));
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else if(cmd == 0x89) {
					i = 1;
					while(!(Byte(ea + i)&0x80)) i++;
					cmd_len = i + 2;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					MakeComm(ea, "");
					MakeComm(ea, form("cmd_%02x", cmd));
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else {
					Message(" unk_cmd at ea = 0x%08x!\n", ea);
					if(_sp == 3) {
						ea = _s2;
						_sp--;
					} else if(_sp == 2) {
						ea = _s1;
						_sp--;
					} else if(_sp == 1) {
						ea = _s0;
						_sp--;
					} else {
						stop = 1;
					}
				}
			} else {
				if(cmd_82)
					cmd_len = 1;
				else
					cmd_len = 2;
//				Message(" data len %d\n", cmd_len);
				MakeComm(ea, "");
				MakeComm(ea, "data");
				MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, cmd_len, 0);
				SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
				ea = ea + cmd_len;
			}
			if((!_sp) && (ea > eea))
				stop = 1;
		} while (!stop);
		Message(" stop at ea = 0x%08x!\n", ea);
	} else
		Message(" no selection!\n");
}

static term2_proto_apu_ch3() {
	auto cmd_82 = 0;
	auto cmd_len, cmd, stop = 0;
	auto _sp = 0, _s0, _s1, _s2;
	auto ea = SelStart();
	auto eea = SelEnd();
	auto base, ptr, i;
	if((ea != BADADDR) && (eea != BADADDR)) {
//		Message(" run from %08x to %08x!\n", ea, eea);
		do {
			cmd = Byte(ea);
			if(cmd & 0x80) {
				if((cmd == 0x81)||(cmd == 0x83)||(cmd == 0x85)||(cmd == 0x90)||(cmd == 0x86)||(cmd == 0x87)) {
					cmd_len = 1;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					if(cmd == 0x83) {
						MakeComm(ea, "");
						MakeComm(ea, "stop");
//						Message(" stop\n");
						stop = 1;
					} else if(cmd == 0x85) {
						MakeComm(ea, "");
						MakeComm(ea, "ret");
//						Message(" ret\n");
						if(_sp == 3) {
							ea = _s2;
						} else if(_sp == 2) {
							ea = _s1;
						} else if(_sp == 1) {
							ea = _s0;
						} else {
							Message(" can't do ret!\n");
							stop = 1;
						}
						_sp--;
					} else {
						MakeComm(ea, "");
						MakeComm(ea, form("cmd_%02x", cmd));
//						Message(" cmd_%02x\n", cmd);
						ea = ea + cmd_len;
					}
				} else if((cmd == 0x80)||(cmd == 0x82)||(cmd == 0x8B)||(cmd == 0x8D)||(cmd == 0x8E)||(cmd == 0x93)||(cmd == 0x94)||(cmd == 0x95)) {
					cmd_len = 2;
					MakeComm(ea, "");
					if(cmd == 0x82) {
						cmd_82 = Byte(ea + 1);
						if(cmd_82) {
//							Message(" mode8\n");
							MakeComm(ea, "mode8");
						} else {
//							Message(" mode16\n");
							MakeComm(ea, "mode16");
						}
					} else {
//						Message(" cmd_%02x\n", cmd);
						MakeComm(ea, form("cmd_%02x", cmd));
					}
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					ea = ea + cmd_len;
				} else if((cmd == 0x84)||(cmd == 0x88)||(cmd == 0x91)) {
					cmd_len = 3;
					MakeUnknown(ea, 3, DOUNK_SIMPLE);
					ptr = Word(ea + 1);
					MakeData(ea, FF_BYTE, 1, 0);
					MakeData(ea + 1, FF_WORD, 1, 0);
					if(ptr < 0xA000)
						base = MK_FP(AskSelector(0x9), 0);
					else
						base = MK_FP(AskSelector(0xA), 0);
					OpOffEx(ea + 1, 0, REF_OFF16, -1, base, 0);
					MakeComm(ea, "");
					MakeComm(ea + 1, "");
					if(cmd == 0x88) {
						stop = 1;
//						Message(" jmp to 0x%08x\n", base + ptr);
						MakeComm(ea, form("jmp 0x%08x", base + ptr));
					} else if(cmd == 0x84) {
//						Message(" jsr to 0x%08x\n", base + ptr);
						MakeComm(ea, form("jsr 0x%08x", base + ptr));
						if(_sp == 0) {
							_s0 = ea + cmd_len;
						} else if(_sp == 1) {
							_s1 = ea + cmd_len;
						} else if(_sp == 2) {
							_s2 = ea + cmd_len;
						} else {
							Message(" out of stack!\n");
							stop = 1;
						}
						ea = base + ptr;
						_sp++;
					} else {
						MakeComm(ea, "");
						MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
						MakeData(ea, FF_BYTE, cmd_len, 0);
						SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
						ea = ea + cmd_len;
					}
				} else if((cmd == 0x8A)||(cmd == 0x92)) {
					cmd_len = 4;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					if(cmd == 0x92) {
						MakeData(ea, FF_BYTE, 2, 0);
						SetArrayFormat(ea, AP_IDXHEX, 2, 0);
						ptr = Word(ea + 2);
						MakeData(ea + 2, FF_WORD, 1, 0);
						if(ptr < 0xA000)
							base = MK_FP(AskSelector(0x9), 0);
						else
							base = MK_FP(AskSelector(0xA), 0);
						OpOffEx(ea + 2, 0, REF_OFF16, -1, base, 0);
						MakeComm(ea, "");
						MakeComm(ea + 2, "");
						MakeComm(ea, form("cmd_%02x arg_ofs 0x%08x", cmd, base + ptr));
						i = 0;
						while (Byte(base + ptr + i) != 0xFF) i++;
						MakeUnknown(base + ptr, i + 1, DOUNK_SIMPLE);
						MakeData(base + ptr, FF_BYTE, i + 1, 0);
						SetArrayFormat(base + ptr, AP_IDXHEX, i + 1, 0);
					} else {
						MakeData(ea, FF_BYTE, cmd_len, 0);
						SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
						MakeComm(ea, "");
						MakeComm(ea, form("cmd_%02x", cmd));
					}
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else if(cmd == 0x8C) {
					if(Byte(ea + 1))
						cmd_len = 4;
					else
						cmd_len = 2;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					MakeComm(ea, "");
					MakeComm(ea, form("cmd_%02x", cmd));
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else if(cmd == 0x8F) {
					if(Byte(ea + 1))
						cmd_len = 5;
					else
						cmd_len = 2;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					MakeComm(ea, "");
					MakeComm(ea, form("cmd_%02x", cmd));
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else if(cmd == 0x89) {
					i = 1;
					while(!(Byte(ea + i)&0x80)) i++;
					cmd_len = i + 2;
					MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
					MakeData(ea, FF_BYTE, cmd_len, 0);
					SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
					MakeComm(ea, "");
					MakeComm(ea, form("cmd_%02x", cmd));
//					Message(" cmd_%02x\n", cmd);
					ea = ea + cmd_len;
				} else {
					Message(" unk_cmd at ea = 0x%08x!\n", ea);
					if(_sp == 3) {
						ea = _s2;
						_sp--;
					} else if(_sp == 2) {
						ea = _s1;
						_sp--;
					} else if(_sp == 1) {
						ea = _s0;
						_sp--;
					} else {
						stop = 1;
					}
				}
			} else {
				if(cmd_82)
					cmd_len = 1;
				else
					cmd_len = 2;
//				Message(" data len %d\n", cmd_len);
				MakeComm(ea, "");
				MakeComm(ea, "data");
				MakeUnknown(ea, cmd_len, DOUNK_SIMPLE);
				MakeData(ea, FF_BYTE, cmd_len, 0);
				SetArrayFormat(ea, AP_IDXHEX, cmd_len, 0);
				ea = ea + cmd_len;
			}
			if((!_sp) && (ea > eea))
				stop = 1;
		} while (!stop);
		Message(" stop at ea = 0x%08x!\n", ea);
	} else
		Message(" no selection!\n");
}

static main(void) {
//	AddHotkey("Shift-X", "script_convert0");
//	AddHotkey("Shift-C", "script_convert1");
}
