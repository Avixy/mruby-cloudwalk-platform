/*
** network.c - Network module
**
** Network.ping
**   NetPing();
**
*/

#include "mruby.h"
#include <stddef.h>
#include <string.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#if MRUBY_RELEASE_NO < 10000
  #include "error.h"
#else
  #include "mruby/error.h"
#endif


int connectUsingDHCP = 1;

static mrb_value mrb_network__ping(mrb_state *mrb, mrb_value klass)
{
	mrb_value ip;
	mrb_int timeout, ret;
	char sIp[16] =
	{ 0x00 };
	int status;
	char cmdPing[32];

	mrb_get_args(mrb, "Si", &ip, &timeout);

	ret = -1;

	strncpy(&sIp, RSTRING_PTR(ip), RSTRING_LEN(ip) );

	snprintf(cmdPing, sizeof(cmdPing), "ping %s", sIp);
	status = system(cmdPing);


	if (-1 != status)
	{
		printf("Ping result: %d", ret);
	}
	else
	{
		printf("Error on Ping!\n");
	}

	return mrb_fixnum_value(ret);
}

static mrb_value
mrb_wifi_dhcp_client_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int net, ret;
  mrb_get_args(mrb, "i", &net);

  connectUsingDHCP = 1;
  ret = 0;

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_wifi_dhcp_client_check(mrb_state *mrb, mrb_value klass)
{
  mrb_int net, ret;
  mrb_get_args(mrb, "i", &net);

  /*TODO: Ver com o Thiago como é o processo deles, pois o nosso DHCP é direto nas conexões.*/


  return mrb_fixnum_value(0);
}

void
mrb_init_network(mrb_state* mrb)
{
  struct RClass *network;

  network = mrb_define_class(mrb, "Network", mrb->object_class);

  mrb_define_class_method(mrb, network, "_ping", mrb_network__ping, MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb, network, "_dhcp_client_start", mrb_wifi_dhcp_client_start, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, network, "_dhcp_client_check", mrb_wifi_dhcp_client_check, MRB_ARGS_REQ(1));
}

