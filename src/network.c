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

int ping(char*, int);
char* toip(char* address);

static mrb_value mrb_network__ping(mrb_state *mrb, mrb_value klass)
{
	mrb_value ip;
	mrb_int timeout;
	char sIp[16] =
	{ 0x00 };
	char* parsedIp;

	int res = 0;

	mrb_get_args(mrb, "Si", &ip, &timeout);

	strncpy(&sIp, RSTRING_PTR(ip), RSTRING_LEN(ip) );

	parsedIp = toip(sIp);

	printf("will ping %s", sIp);
	printf(" (%s)\n", parsedIp);

	res = ping(parsedIp, timeout);

	printf("---> ping=%d\n", res);

	return mrb_fixnum_value(res==0);
}

static mrb_value
mrb_wifi_dhcp_client_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int net;
  mrb_get_args(mrb, "i", &net);

  connectUsingDHCP = 1;

  //no avx3400, foi combinado sempre retornar sucesso, dado que o dhcp é informado na hora da conexão e nao aqui, assincronamente.
  return mrb_fixnum_value(0);
}

static mrb_value
mrb_wifi_dhcp_client_check(mrb_state *mrb, mrb_value klass)
{
  mrb_int net;
  mrb_get_args(mrb, "i", &net);

  //no avx3400, foi combinado sempre retornar sucesso, dado que o dhcp é informado na hora da conexão e nao aqui, assincronamente.
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

