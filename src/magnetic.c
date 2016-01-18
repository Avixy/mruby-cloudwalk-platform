#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/string.h"
#include "mruby/hash.h"

#include "ioctl/mag.h"
#include "magcard/magcard.h"

mrb_value
mrb_magnetic_s_open(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

  readersEnableMag(1);

  ret = 0;

  return mrb_fixnum_value(ret);
}

/* TODO: Ver o que ele quer com esse read
 *
 */
mrb_value
mrb_magnetic_s_read(mrb_state *mrb, mrb_value self)
{
  mrb_int ret;

  ret = readersMagCheckResult(0);

  return mrb_fixnum_value(ret);
}

mrb_value
mrb_magnetic_s_close(mrb_state *mrb, mrb_value self)
{
	readersEnableMag(0);

  return mrb_nil_value();
}

/*{:track1 => "", :track2 => "", :track3 => ""}*/
mrb_value
mrb_magnetic_s_tracks(mrb_state *mrb, mrb_value self)
{
  mrb_value hash;

  hash = mrb_hash_new(mrb);

  struct mag_stripe magTracksInfo;

  if (readersMagGetInfo(&magTracksInfo) > 0) {  		

  		char buffer[128];
  		
		if (magTracksInfo.tracks[0].status == 0
				&& magTracksInfo.tracks[0].CharCount > 0)
		{			
			memset(buffer, 0x00, 128);
			readersMagGetTrack(buffer, 0);
			mrb_value track = mrb_str_new_cstr(mrb, buffer);
			mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track1")), track);
		}		


		if (magTracksInfo.tracks[1].status == 0
				&& magTracksInfo.tracks[1].CharCount > 0)
		{
			memset(buffer, 0x00, 128);
			readersMagGetTrack(buffer, 1);
			mrb_value track = mrb_str_new_cstr(mrb, buffer);
			mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track2")), track);
		}		


		if (magTracksInfo.tracks[2].status == 0
				&& magTracksInfo.tracks[2].CharCount > 0)
		{
			memset(buffer, 0x00, 128);
			readersMagGetTrack(buffer, 2);
			mrb_value track = mrb_str_new_cstr(mrb, buffer);
			mrb_hash_set(mrb, hash, mrb_symbol_value(mrb_intern_cstr(mrb, "track3")), track);
		}		

	}

  return hash;
}

void
mrb_magnetic_init(mrb_state* mrb)
{
  struct RClass *platform;
  struct RClass *magnetic;

  platform = mrb_class_get(mrb, "Platform");
  magnetic = mrb_define_class_under(mrb, platform, "Magnetic", mrb->object_class);

  mrb_define_class_method(mrb , magnetic , "open"      , mrb_magnetic_s_open      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , magnetic , "read"      , mrb_magnetic_s_read      , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , magnetic , "close"     , mrb_magnetic_s_close     , MRB_ARGS_NONE());
  mrb_define_class_method(mrb , magnetic , "tracks"    , mrb_magnetic_s_tracks    , MRB_ARGS_REQ(1));
}

