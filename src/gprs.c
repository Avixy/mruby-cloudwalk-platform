

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#include "mruby.h"
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

#ifdef AVIXY_DEVICE
  #include "core/device_core.h"
  #include "core/trace.h"
  #include "network/network.h"
  #include "network/modem.h"
  #include "gprs/gprs_interface.h"
#endif


static int simSlotSelected = 0;
static char prvIdDoSimCard[32];

static char strGPRSApn[32];
static char strGPRSUser[16];
static char strGPRSPasswd[16];

#ifdef AVIXY_DEVICE
int fillAPN(int operadora, char *nomeOperadora, struct avxmodem_access_point *apn, char *nomePadraoDaOperadora, char *simID)
{
	printf("entrando em %s\n", __FUNCTION__);

	int indOperadora = OP_UNDEFINED;

	strncpy(apn->apn, strGPRSApn, sizeof(apn->apn));
	strncpy(apn->login, strGPRSUser, sizeof(apn->login));
	strncpy(apn->password, strGPRSPasswd, sizeof(apn->password));

	strcpy(nomePadraoDaOperadora, nomeOperadora);

	return(indOperadora);
}
#endif


static void fxProgresscallback(void)
{
#ifdef AVIXY_DEVICE  
	printf("gprs state: %d\n", gprsCommGetCurrentStateToApplication());
#endif  
}

/*Start the hardware*/
static mrb_value
mrb_gprs_start(mrb_state *mrb, mrb_value klass)
{
  printf("entrando no mrb_gprs_start..\n");

  mrb_int ret=0;

#ifdef AVIXY_DEVICE
  avxnmInterfacePriority(AVXNM_NETIF_PRIORITIES_MODEM_ETHERNET_WIFI);

  gprsCommSetSimSlot(simSlotSelected);

  gprsCommSetDesiredOperationMode(GPRS_REGISTERED_MODE);
#endif

  printf("saindo mrb_gprs_start ..\n");

  return mrb_fixnum_value(ret);
}

/*Turn on/off the power 1 - on 0 - off*/
static mrb_value
mrb_gprs_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int on;
  mrb_get_args(mrb, "i", &on);

#ifdef AVIXY_DEVICE
  if (on){
	  gprsCommSetDesiredOperationMode(GPRS_CONNECTED_MODE);
  }else{
	  gprsCommSetDesiredOperationMode(GPRS_SHUTDOWN_MODE);
  }
#endif

  return mrb_true_value();
}


/*Start GPRS dial*/
/*should be unblocking if timeout 0*/
static mrb_value
mrb_gprs_connect(mrb_state *mrb, mrb_value klass)
{
  mrb_value apn, user, password;
  const char *sAPN, *sUser, *sPass;
  int ret=0;

  printf("entrando... mrb_gprs_connect\n");

  apn   = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@apn"));
  printf("ping\n");

  sAPN  = mrb_str_to_cstr(mrb, apn);

  user  = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@user"));
  sUser = mrb_str_to_cstr(mrb, user);

  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
  sPass    = mrb_str_to_cstr(mrb, password);

  printf("sAPN=%s, sUser=%s, sPass=%s\n", sAPN, sUser, sPass);

  strncpy(strGPRSApn, sAPN, sizeof(strGPRSApn));
  strncpy(strGPRSUser, sUser, sizeof(strGPRSUser));
  strncpy(strGPRSPasswd, sPass, sizeof(strGPRSPasswd));

#ifdef AVIXY_DEVICE  
  printf("vou entrar no gprsCommInit ..\n");

  ret = gprsCommInit("8486", prvIdDoSimCard, fxProgresscallback, fillAPN);

  printf("sai no gprsCommInit ..\n");

  ret = gprsCommSetDesiredOperationMode(GPRS_CONNECTED_MODE);
#endif

  printf("saindo do mrb_gprs_connect\n");

  return mrb_fixnum_value(ret);
}

/*Check if dial has ended*/
/*   0 -> Sucess*/
/*   1 -> In Progress*/
/* < 0 -> Fail*/
static mrb_value
mrb_gprs_connected_m(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=0;
  int state;

#ifdef AVIXY_DEVICE
  state = gprsCommGetCurrentStateToApplication();

  switch (state)
	{
  	  case G_PPP_CONNECTED:
  		ret = 0;
  		break;
	case G_INITIALIZING:
	case G_READ_SIM_ID:
	case G_CONNECTING:
	case G_SEARCHING_GPRS:
	case G_PPP_CONNECTING:
	case G_CONNECTED:
		ret = 1;
		break;
	case G_REGISTRATION_REFUSED:
	case G_PPP_DISCONNECTING:
	case G_SHUTTING_DOWN:
	case G_SHUTDOWN:
		ret = -state;
		break;
	default:
		ret=-99;
		break;
	}
#endif

  return mrb_fixnum_value(ret);
}

static mrb_value
mrb_gprs_disconnect(mrb_state *mrb, mrb_value klass)
{

#ifdef AVIXY_DEVICE
  gprsCommSetDesiredOperationMode(GPRS_REGISTERED_MODE);
#endif

  return mrb_true_value();
}

/*Must to return the signal value between 1 and 5*/
static mrb_value
mrb_gprs_signal(mrb_state *mrb, mrb_value klass)
{
  /*TODO Implement*/
  /*mrb_int signal = OsWlGetSignal();*/
  mrb_int signal = 0;
  return mrb_fixnum_value(signal);
}

/*Must to return the signal value between 1 and 5*/
static mrb_value
mrb_gprs_sim_id(mrb_state *mrb, mrb_value klass)
{
  return mrb_str_new_cstr(mrb, prvIdDoSimCard);
}

void
mrb_gprs_init(mrb_state* mrb)
{
  struct RClass *platform, *network, *gprs;

  platform = mrb_class_get(mrb, "Platform");
  network  = mrb_class_get_under(mrb, platform, "Network");
  gprs     = mrb_define_class_under(mrb, network, "Gprs", mrb->object_class);

  mrb_define_class_method(mrb, gprs, "start", mrb_gprs_start, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, gprs, "power", mrb_gprs_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, gprs, "connect", mrb_gprs_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, gprs, "connected?", mrb_gprs_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, gprs, "disconnect", mrb_gprs_disconnect, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, gprs, "signal", mrb_gprs_signal, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, gprs, "sim_id", mrb_gprs_sim_id, MRB_ARGS_NONE());
}

