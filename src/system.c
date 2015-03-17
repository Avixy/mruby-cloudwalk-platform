#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"
#include <sys/reboot.h>

#include "core.h"

static mrb_value
mrb_system_s__serial(mrb_state *mrb, mrb_value self)
{
  char serial[128];
  uint32_t serial_number;

  memset(&serial, 0, sizeof(serial));
  serial_number = avxGetSerialNumber();

  sprintf(serial, "%d", serial_number);

  return mrb_str_new_cstr(mrb, serial);
}

/**
# Set screen backlight (turn on automatically if there has actions like key-pressing, card-swiping or card-inserting).
    #  0 = Turn off backlight.
    #  1 = (D200): Keep backlight on for 30 seconds ( auto-shut-down after 30 seconds).
    #  1 = (Vx510): On.
    #  2 = (D200): Always on.
    # -1 = (Avixy 3400): Turn off automatically after xx seconds.
    #  n = (Evo/Telium 2): Percentage until 100.
*/
static mrb_value
mrb_system_s__set_backlight(mrb_state *mrb, mrb_value self)
{
  mrb_int mode;

  mrb_get_args(mrb, "i", &mode);

  avxSetBacklight(mode, mode);

  
  return mrb_fixnum_value(mode);
}

/**
 * # Read the battery level.
# 0 Battery voltage low and battery icon blinks. Suggested that do not process transaction, print and wireless communication etc. at this moment. You should recharge the battery immediately to avoid shut down and lost data.
# 1 Battery icon displays 1 grid
# 2 Battery icon displays 2 grids
# 3 Battery icon displays 3 grids
# 4 Battery icon displays 4 grids
# 5 Powered by external power supply and the battery in charging. Battery icon displays form empty to full cycle. The battery status indicator on the bottom of terminal is displaying red
# 6 Powered by external power supply and the battery charging 6 finished. Battery icon displays full grids. The battery status indicator on the bottom of terminal is displaying green.
 */

static mrb_value
mrb_system_s__battery(mrb_state *mrb, mrb_value self)
{
  char battery[128];
  struct avx_charger batInfo;
  int capacity = 0;

  memset(&battery, 0, sizeof(battery));

  memset(&batInfo, 0, sizeof(struct avx_charger));

  if ( avxGetBatteryInfo(&batInfo) < 0 )
  {
	  //Report error.
  } else {
	   capacity = ((batInfo.charge  * 100) / batInfo.capacity);
	  if (batInfo.power_supply_present){
		  capacity = (capacity >= 95 ? 6 : 5);
	  } else {
		  capacity = capacity / 5;
		  capacity = MIN(capacity,4);
	  }
  }

  sprintf(battery, "%d", capacity);

  return mrb_str_new_cstr(mrb, battery);
}

static mrb_value
mrb_audio_s__beep(mrb_state *mrb, mrb_value self)
{
  mrb_int tone, milliseconds;

  mrb_get_args(mrb, "ii", &tone, &milliseconds);

  avxBuzzer(tone, milliseconds, 1);

  return mrb_fixnum_value(tone);
}

static mrb_value
mrb_system_s_reboot(mrb_state *mrb, mrb_value self)
{
 return mrb_fixnum_value(reboot(RB_AUTOBOOT));
}

void
mrb_system_init(mrb_state* mrb)
{
  struct RClass *platform;
  struct RClass *audio;
  struct RClass *system;

  platform = mrb_define_class(mrb, "Platform", mrb->object_class);
  audio = mrb_define_class_under(mrb, platform, "Audio", mrb->object_class);
  system = mrb_define_class_under(mrb, platform, "System", mrb->object_class);

  mrb_define_class_method(mrb , system , "serial"     , mrb_system_s__serial        , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "backlight=" , mrb_system_s__set_backlight , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , system , "battery"    , mrb_system_s__battery       , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , audio  , "beep"       , mrb_audio_s__beep           , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , system , "reboot"     , mrb_system_s_reboot         , MRB_ARGS_NONE());
}

