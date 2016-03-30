#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

// Avixy includes
#include "core/device_core.h"
#include "avixy/avixy.h"
#include "sys/param.h"
#include <sys/reboot.h>

static mrb_value
mrb_system_s__serial(mrb_state *mrb, mrb_value self)
{
  char serial[128];
  uint32_t serial_number;

  memset(&serial, 0, sizeof(serial));

#ifdef AVIXY_DEVICE
  serial_number = avxGetSerialNumber();
  sprintf(serial, "%d", serial_number);
#endif

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

#ifdef AVIXY_DEVICE
  avxSetBacklight(mode, mode);
#endif

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

#ifdef AVIXY_DEVICE
  struct avx_charger batInfo;
  int capacity = 0;
#endif

  memset(&battery, 0, sizeof(battery));

#ifdef AVIXY_DEVICE
  memset(&batInfo, 0, sizeof(struct avx_charger));

  if ( avxGetBatteryInfo(&batInfo) < 0 )
  {
	  //Report error.
	  printf("Error on getBatteryInfo!\n");
  } else {
    capacity = ((batInfo.charge  * 100) / batInfo.capacity);
    
    int psu_present = 0;
#if AVX_MODEL == 4000
    psu_present = batInfo.plug_ac;
#else
    psu_present = batInfo.power_supply_present;
#endif

    if (psu_present){
		  capacity = (capacity >= 95 ? 6 : 5);
	  } else {
		  capacity = capacity / 5;
		  capacity = MIN(capacity,4);
	  }
  }

  sprintf(battery, "%d", capacity);
#endif

  return mrb_str_new_cstr(mrb, battery);
}

static mrb_value
mrb_audio_s__beep(mrb_state *mrb, mrb_value self)
{
  mrb_int tone, milliseconds;

  mrb_get_args(mrb, "ii", &tone, &milliseconds);

#ifdef AVIXY_DEVICE
  avxBuzzer(tone, milliseconds, 1);
#endif

  return mrb_fixnum_value(0);
}

static mrb_value
mrb_system_s_reboot(mrb_state *mrb, mrb_value self)
{
  mrb_int ret=0;

  avxResetSystem();
  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_system_s_hwclock(mrb_state *mrb, mrb_value self)
{
  /*ST_TIME t;*/
  mrb_value timezone;
  mrb_int year, month, day, hour, minute, second;
  mrb_get_args(mrb, "iiiiiio", &year, &month, &day, &hour, &minute, &second, &timezone);

  struct tm tm;

  tm.tm_mday = day;
  tm.tm_mon = month - 1;
  tm.tm_year = year - 1900;
  tm.tm_hour = hour; 
  tm.tm_min = minute;
  tm.tm_sec = second;

  if(mrb_string_p(timezone))  {
    char tzone[10];
    sscanf(tzone, "TZ=%s", RSTRING_PTR(timezone));
    dtmSetTimeZone(tzone); 
  }

  dtmSetTimetm(&tm);

 return mrb_fixnum_value(0);
}

static mrb_value
mrb_system_s_model(mrb_state *mrb, mrb_value self)
{
  char version[32]="\0";
  memset(&version, 0, sizeof(version));
  sprintf(version, "%s\0", DEVICE_MODEL);
  printf("==========>>>>> device model: %s", DEVICE_MODEL);

  return mrb_str_new_cstr(mrb, version);
}

static mrb_value
mrb_system_s_brand(mrb_state *mrb, mrb_value self)
{
  char brand[32]="\0";
  memset(&brand, 0, sizeof(brand));
  sprintf(brand, "%s\0", DEVICE_BRAND);

  return mrb_str_new_cstr(mrb, brand);
}

static mrb_value
mrb_system_s_os_version(mrb_state *mrb, mrb_value self)
{
  avxCoreInit();
  int mngFd = avxGetMngFd();
  struct avx_firmware_version version = { };
  char os_version[32]="\0";
  memset(&os_version, 0, sizeof(os_version));

  int res = ioctl(mngFd, AVXSYS_GET_FIRMWARE_VERSION, &version );
  
  if(!res) // No Errors
  {
    // For Avixy the Os Version is the same as the Kernel version
    sprintf(os_version, "%d.%d.%d\0", version.kernel.major, 
      version.kernel.minor, version.kernel.change);

  }
  printf("==========>>>>> os version: %s", os_version);

  return mrb_str_new_cstr(mrb, os_version);
}

static mrb_value
mrb_system_s_sdk_version(mrb_state *mrb, mrb_value self)
{
  char version[32]="\0";
  memset(&version, 0, sizeof(version));
  sprintf(version, "%s", AVIXY_SDK_VERSION);
  printf("==========>>>>> version: %s", version);

  return mrb_str_new_cstr(mrb, version);
}

static mrb_value
mrb_system_s_pinpad_version(mrb_state *mrb, mrb_value self)
{
  char pinpad_version[32]="\0";
  memset(&pinpad_version, 0, sizeof(pinpad_version));
  sprintf(pinpad_version, "%s\0", AVIXY_HW_VERSION);
  printf("==========>>>>> pinpad version: %s", pinpad_version);

  return mrb_str_new_cstr(mrb, pinpad_version);
}

void
mrb_system_init(mrb_state* mrb)
{
  struct RClass *platform;
  struct RClass *audio;
  struct RClass *system;

  platform = mrb_define_class(mrb, "Platform", mrb->object_class);
  audio    = mrb_define_class_under(mrb, platform, "Audio", mrb->object_class);
  system   = mrb_define_class_under(mrb, platform, "System", mrb->object_class);

  mrb_define_class_method(mrb , system , "serial"         , mrb_system_s__serial        , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "backlight="     , mrb_system_s__set_backlight , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , system , "battery"        , mrb_system_s__battery       , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , audio  , "beep"           , mrb_audio_s__beep           , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , system , "reboot"         , mrb_system_s_reboot         , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "hwclock"        , mrb_system_s_hwclock        , MRB_ARGS_REQ(7));
  mrb_define_class_method(mrb , system , "model"          , mrb_system_s_model          , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "brand"          , mrb_system_s_brand          , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "os_version"     , mrb_system_s_os_version     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "sdk_version"    , mrb_system_s_sdk_version    , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , system , "pinpad_version" , mrb_system_s_pinpad_version , MRB_ARGS_NONE());
}

