#include <stdlib.h>
#include <stdio.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#ifdef AVIXY_DEVICE
#include "avixy/gfx.h"
#include "avixy/gfx/menu.h"
#include "avixy/gfx/forms.h"
#include "avixy/gfx/input.h"
#include "avixy/gfx/lcd.h"
#include "avixy/bitmap.h"
#include "avixy/sb.h"
#endif

mrb_value
mrb_display_s_clear(mrb_state *mrb, mrb_value self)
{
	lcdClear();

  return mrb_nil_value();
}

mrb_value
mrb_display_s_clear_line(mrb_state *mrb, mrb_value self)
{
  mrb_int line;

  mrb_get_args(mrb, "i", &line);

  lcdSetCursorLC(line, 0);
  lcdClearEOL(-1);

  return mrb_nil_value();
}

mrb_value
mrb_display_s_print_bitmap(mrb_state *mrb, mrb_value self)
{
  mrb_value path;
  mrb_int x, y;

  mrb_get_args(mrb, "oii", &path, &y, &x);

  printf("display bitmap: x=%d, y=%d, bitmap=%s", x,y, (char *) RSTRING_PTR(path));

  lcdBitmapLoadFromFileToScreen((const char *) RSTRING_PTR(path), x, y);

  return mrb_nil_value();
}

mrb_value
mrb_display_s_print_line(mrb_state *mrb, mrb_value self)
{
  mrb_value buf;
  mrb_int x, y;

  mrb_get_args(mrb, "oii", &buf, &y, &x);

  printf("display print line: x=%d, y=%d, type=%d, text=%s\n", x,y, mrb_type(buf), (char *) RSTRING_PTR(buf));

  lcdPrint(y, x, (char *) RSTRING_PTR(buf));

  return mrb_nil_value();
}

mrb_value
mrb_display_s_print_status_bar(mrb_state *mrb, mrb_value self)
{
  mrb_value path;
  mrb_int slot = 0;

  mrb_get_args(mrb, "io", &slot, &path);

  if (mrb_string_p(path)) {
    if (sbSetStatusbarIcon(slot, RSTRING_PTR(path)) == 0)
    {
      return mrb_true_value();
    }
    else
    {
      return mrb_nil_value();
    }
  } else {
    if (sbSetStatusbarIcon(slot, NULL) == 0)
    {
      return mrb_true_value();
    }
    else
    {
      return mrb_nil_value();
    }
  }
  return mrb_nil_value();
}

mrb_value
mrb_display_s_status_bar_slots_available(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(sbGetStatubarSlotsAvailableCount());
}

void
mrb_display_init(mrb_state* mrb)
{
  struct RClass *platform;
  struct RClass *display;

  platform = mrb_class_get(mrb, "Platform");
  display = mrb_define_class_under(mrb, platform, "Display", mrb->object_class);

  mrb_define_class_method(mrb , display , "clear"                      , mrb_display_s_clear                      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , display , "clear_line"                 , mrb_display_s_clear_line                 , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , display , "print_bitmap"               , mrb_display_s_print_bitmap               , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , display , "print_line"                 , mrb_display_s_print_line                 , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , display , "print_status_bar"           , mrb_display_s_print_status_bar           , MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb , display , "status_bar_slots_available" , mrb_display_s_status_bar_slots_available , MRB_ARGS_NONE());
}
