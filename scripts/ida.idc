//
//      This file is automatically executed when IDA is started.
//      You can define your own IDC functions and assign hotkeys to them.
//
//      You may add your frequently used functions here and they will
//      be always available.
//
//
#include <idc.idc>
#include <functions.idc>
#include <cdl_autorun.idc>

//-----------------------------------------------------------------------
// A singleton class for managing breakpoints
class BreakpointManager
{
  // Returns the breakpoints quantity
  Count()
  {
    return GetBptQty();
  }

  // Returns a breakpoint object
  Get(index)
  {
    auto count = this.Count();
    if ( index >= count )
      throw form("Invalid breakpoint index %d (0..%d expected).", index, count);
    return Breakpoint(index);
  }

  // Adds a breakpoint
  Add(bpt)
  {
    return bpt._add();
  }

  // Deletes a breakpoint
  Delete(bpt)
  {
    return bpt._delete();
  }

  // Updates a breakpoint
  Update(bpt)
  {
    return bpt._update();
  }
}

// Breakpoint manager class instance
extern Breakpoints;

//-----------------------------------------------------------------------
// Get name of the current processor
static get_processor_name(void)
{
  auto i;

  auto procname = "";
  for ( i=0; i < 8; i++ )
  {
    auto chr = GetCharPrm(INF_PROCNAME+i);
    if ( chr == 0 )
      break;
    procname = procname + chr;
  }
  return procname;
}

//-----------------------------------------------------------------------
// Get signed extended 16-bit value
static SWord(ea)
{
  auto v = Word(ea);
  if ( v & 0x8000 )
    v = v | ~0xFFFF;
  return v;
}

//-----------------------------------------------------------------------
static main(void)
{
  //
  //      This function is executed when IDA is started.
  //
  //      Add statements to fine-tune your IDA here.
  //

  // Instantiate the breakpoints singleton object
  Breakpoints = BreakpointManager();

  // uncomment this line to remove full paths in the debugger process options:
  // SetCharPrm(INF_LFLAGS, LFLG_DBG_NOPATH|GetCharPrm(INF_LFLAGS));

  // MyHotkeys

	DelHotkey("Shift-F1");
	AddHotkey("Shift-F1", "cdl_script_execute");			//

	AddHotkey("Shift-Q", "make_8bit_near_selection");		// 8-bit ptr table, selection
	AddHotkey("Shift-A", "make_8bit_near_selection_i");		// --//-- hi/lo inverse order

	AddHotkey("Shift-W", "make_8bit_near_selection_d");		// 8-bit ptr table with -1 displacement, selection
	AddHotkey("Shift-S", "make_8bit_near_selection_d_i");	// --//-- hi/lo inverse order

	AddHotkey("Shift-E", "make_8bit_ofs_selection2");		// 8-bit single ptr 2 byte gap, current cursor
	AddHotkey("Shift-D", "make_8bit_far_ofs_selection2");	// --//-- far manual bank request
	AddHotkey("Shift-C", "make_8bit_far_ofs_selection2_i");	// --//-- hi/lo inverse order

	AddHotkey("Shift-R", "make_8bit_ofs_selection3");		// --//-- 3 byte gap
//	AddHotkey("Shift-F", "make_8bit_ofs_selection3_i");		// --//-- hi/lo inverse order
	AddHotkey("Shift-F", "make_8bit_far_ofs_selection3_i");	//

	AddHotkey("Shift-T", "make_8bit_ofs_selection4");		// --//-- 4 byte gap
	AddHotkey("Shift-G", "make_8bit_ofs_selection4_i");		// --//-- hi/lo inverse order
	AddHotkey("Shift-B", "make_8bit_far_ofs_selection4");	// --//-- far manual bank request

	AddHotkey("Shift-Y", "make_8bit_ofs_selection5");		// --//-- 5 bytes gap
	AddHotkey("Shift-H", "make_8bit_ofs_selection5_i");		// --//-- hi/lo inverse order
	AddHotkey("Shift-N", "make_8bit_far_ofs_selection5");	// --//-- far manual bank request

	AddHotkey("Shift-U", "make_8bit_ofs_selection6");		// --//-- 6 bytes gap
	AddHotkey("Shift-J", "make_8bit_ofs_selection6_i");		// --//-- hi/lo inverse order
	AddHotkey("Shift-M", "make_8bit_far_ofs_selection6");	// --//-- far manual bank request

	AddHotkey("Shift-Z", "auto_array");						// auto convert unexplored area to byte array, auto search of next area from cursor
	AddHotkey("Shift-X", "auto_ptr_table");
	AddHotkey("Shift-V", "auto_far_ptr");
	AddHotkey("Shift-P", "find_near_tbls");					// auto detect generic code patterns for jump tables and offsets
	AddHotkey("Shift-0", "duplicate_marker");				// mark copies of data from other banks
	AddHotkey("Shift-9", "auto_rename_ptrs");

	DelHotkey("Shift-1");
	AddHotkey("Shift-1", "make_8bit_ofs_gb_auto");
	DelHotkey("Shift-2");
	AddHotkey("Shift-2", "make_8bit_ofs_gb_2");
	DelHotkey("Shift-3");
	AddHotkey("Shift-3", "make_8bit_ofs_gb_3");				// default IDA's shortcut for OpNumber is '#', which is converted to Shift-3 automatically, be careful dammit!
	DelHotkey("Shift-4");
	AddHotkey("Shift-4", "make_8bit_ofs_gb_4");
	DelHotkey("Shift-5");
	AddHotkey("Shift-5", "make_8bit_ofs_gb_5");
	DelHotkey("Shift-F2");
	AddHotkey("Shift-F2", "make_8bit_ofs_gb_2_i");
	DelHotkey("Shift-F3");
	AddHotkey("Shift-F3", "make_8bit_ofs_gb_3_i");				// default IDA's shortcut for OpNumber is '#', which is converted to Shift-3 automatically, be careful dammit!
	DelHotkey("Shift-F4");
	AddHotkey("Shift-F4", "make_8bit_ofs_gb_4_i");
	DelHotkey("Shift-F5");
	AddHotkey("Shift-F5", "make_8bit_ofs_gb_5_i");
}
