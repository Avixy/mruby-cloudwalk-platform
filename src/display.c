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
#endif

/* Dúvidas
 *
 *  1 - Como inicializa o display?
 *  2 - Como a app sabe o tamanho da tela?
 *  3 - Como seleciona Fonte?
 *  	Tudo isso é feito na app que inicializa o mruby.
 *
 *  4 - Como tratar transparência no bitmap?
 *  	O Thiago disse que usam RGBA. Nosso SDk não suporta o alfa, mas o Ananias ficou de ver como fazer.
 */

mrb_value
mrb_display_s_clear(mrb_state *mrb, mrb_value self)
{

#ifdef AVIXY_DEVICE
	printf("LCD Clear!");
	lcdClear();
#endif

  return mrb_nil_value();
}

mrb_value
mrb_display_s_clear_line(mrb_state *mrb, mrb_value self)
{
  mrb_int line;

  mrb_get_args(mrb, "i", &line);

#ifdef AVIXY_DEVICE
  lcdSetCursorLC(line, 0);
  lcdClearEOL(-1);
#endif

  return mrb_nil_value();
}

mrb_value
mrb_display_s_print_bitmap(mrb_state *mrb, mrb_value self)
{
  mrb_value path;
  mrb_int x, y;

  mrb_get_args(mrb, "oii", &path, &y, &x);

#ifdef AVIXY_DEVICE
  printf("display bitmap: x=%d, y=%d, bitmap=%s", x,y, (char *) mrb_ptr(path));

  lcdBitmapLoadFromFileToScreen((const char *) mrb_ptr(path), x, y);
#endif

  return mrb_nil_value();
}

mrb_value
mrb_display_s_print_line(mrb_state *mrb, mrb_value self)
{
  mrb_value buf;
  mrb_int x, y;

  mrb_get_args(mrb, "oii", &buf, &y, &x);

#ifdef AVIXY_DEVICE  
  printf("display bitmap: x=%d, y=%d, bitmap=%s", x,y, (char *) mrb_ptr(buf));

  lcdPrint(y, x, (char *) mrb_ptr(buf));
#endif
  
  return mrb_nil_value();
}

void
mrb_display_init(mrb_state* mrb)
{
  struct RClass *platform;
  struct RClass *display;

  platform = mrb_class_get(mrb, "Platform");
  display = mrb_define_class_under(mrb, platform, "Display", mrb->object_class);

  mrb_define_class_method(mrb , display , "clear"        , mrb_display_s_clear       , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , display , "clear_line"   , mrb_display_s_clear_line  , MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb , display , "print_bitmap" , mrb_display_s_print_bitmap , MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb , display , "print_line"   , mrb_display_s_print_line   , MRB_ARGS_REQ(3));
}
