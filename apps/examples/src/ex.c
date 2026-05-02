#ifdef MIA_OPTION_UNITYBUILD

#define O__ID 00
#include "o/unitybuild.h"
#include "ex_00_hello_world.c"
#undef O__ID

#define O__ID 01
#include "o/unitybuild.h"
#include "ex_01_pointers.c"
#undef O__ID

#define O__ID 02
#include "o/unitybuild.h"
#include "ex_02_animations.c"
#undef O__ID

#define O__ID 03
#include "o/unitybuild.h"
#include "ex_03_iui.c"
#undef O__ID

#define O__ID 04
#include "o/unitybuild.h"
#include "ex_04_widgets.c"
#undef O__ID

#define O__ID 05
#include "o/unitybuild.h"
#include "ex_05_windows.c"
#undef O__ID

#define O__ID 06
#include "o/unitybuild.h"
#include "ex_06_fonts.c"
#undef O__ID

#define O__ID 07
#include "o/unitybuild.h"
#include "ex_07_sound.c"
#undef O__ID

#define O__ID 08
#include "o/unitybuild.h"
#include "ex_08_xtras.c"
#undef O__ID

#define O__ID 09
#include "o/unitybuild.h"
#include "ex_09_fetching.c"
#undef O__ID

#define O__ID 10
#include "o/unitybuild.h"
#include "ex_10_upndownload.c"
#undef O__ID

#define O__ID 11
#include "o/unitybuild.h"
#include "ex_11_benchmark.c"
#undef O__ID

#define O__ID 12
#include "o/unitybuild.h"
#include "ex_12_drawing.c"
#undef O__ID

#define O__ID 13
#include "o/unitybuild.h"
#include "ex_13_mixer.c"
#undef O__ID

#define O__ID 14
#include "o/unitybuild.h"
#include "ex_14_webcam.c"
#undef O__ID

#define O__ID vc
#include "o/unitybuild.h"
#include "EXViewOverlay.c"
#undef O__ID

#define O__ID main
#include "o/unitybuild.h"
#include "main.c"
#undef O__ID

#define O__ID tea
#include "o/unitybuild.h"
#include "tea.c"
#undef O__ID

#define O__ID thn
#include "o/unitybuild.h"
#include "thunder.c"
#undef O__ID

#endif
typedef int avoid_empty_translation_unit;
