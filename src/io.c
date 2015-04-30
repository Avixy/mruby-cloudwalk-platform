#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#ifdef AVIXY_DEVICE
#include "avixy/gfx.h"
#include "avixy/bitmap.h"
#include "avixy/gfx/input.h"
#endif

enum input_mode
{
	INPUT_NUMBERS = 36,
	INPUT_LETTERS = 20,
	INPUT_SECRET  = 28
};

static mrb_value
mrb_platform_io_s__getc(mrb_state *mrb, mrb_value self)
{    
  mrb_int timeout=0, key;

  mrb_get_args(mrb, "i", &timeout);

  printf("i will get a char with timeout = %d", timeout);

#ifdef AVIXY_DEVICE
  key = kbdWaitKey(timeout);
#endif

  return mrb_fixnum_value(key);
}

static mrb_value mrb_platform_io_s__gets(mrb_state *mrb, mrb_value self)
{
	int ret = 666;
	char sValue[128], mask[128];
	mrb_int min, max, mode, x, y;

	memset(sValue, '\0', sizeof(sValue));
	memset(mask, '\0', sizeof(sValue));

	mrb_get_args(mrb, "iiiii", &min, &max, &mode, &y, &x);
	printf("-------------> min = %d, max = %d, mode = %d, y = %d, x = %d\n",
			min, max, mode, y, x);

#ifdef AVIXY_DEVICE
	inputConfig(INPUT_CFG_DEFAULT, 0);
	inputConfig(INPUT_CFG_TIMEOUT, (void *) 60000);
	inputConfig(INPUT_CFG_DRAW_BORDER, 0);

	if (mode == INPUT_NUMBERS)
	{
		memset(mask, '9', max);

	}
	else // INPUT_LETTERS / default
	{
		memset(mask, 'x', max);
	}

	lcdSetCursorLC(y, x);


	if (mode == INPUT_SECRET)
	{
		ret = inputGetFormated(va("!*[%02d]%s", min, mask), sValue);
	}
	else
	{
		ret = inputGetFormated(va("![%02d]%s", min, mask), sValue);
	}
	printf("ret = %d, sValue = %s\n", ret, sValue);
	//lcdPrint(y, x, sValue);
#endif

	return mrb_fixnum_value(0);
}

void
mrb_io_init(mrb_state* mrb)
{
  struct RClass *io;

  io = mrb_define_class(mrb, "IO", mrb->object_class);

  mrb_define_class_method(mrb , io , "_getc" , mrb_platform_io_s__getc , MRB_ARGS_REQ(1));

  mrb_define_class_method(mrb , io , "_gets" , mrb_platform_io_s__gets , MRB_ARGS_REQ(5));

#ifdef AVIXY_DEVICE
  inputConfig(INPUT_CFG_SET_ESCAPE, (void *)'\b');
#endif  
}

