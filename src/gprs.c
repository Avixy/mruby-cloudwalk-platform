

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
#include "network/network.h"
#include "network/modem.h"
#include "gprs/gprs_interface.h"
#endif


/*TODO: Combinar com o Thiago métodos para:
 * Set SIM Pin. Existem Operadoras com Simcards protegidos por PIN. Observar que isso deve ser feito antes do start.
 * Get ID Do simcard.
 */


static int simSlotSelected = 0;
static char *prvSimPIN="";
static char prvIdDoSimCard[32];

static char strGPRSApn[32];
static char strGPRSUser[16];
static char strGPRSPasswd[16];

#ifdef AVIXY_DEVICE
int fillAPN(int operadora, char *nomeOperadora, struct avxmodem_access_point *apn, char *nomePadraoDaOperadora, char *simID)
{
	int indOperadora = OP_INDEFINIDA;

	strncpy(apn->apn, strGPRSApn, sizeof(apn->apn));
	strncpy(apn->login, strGPRSUser, sizeof(apn->login));
	strncpy(apn->password, strGPRSPasswd, sizeof(apn->password));

	strcpy(nomePadraoDaOperadora, nomeOperadora);

	return(indOperadora);
}
#endif

/*Start the hardware*/
static mrb_value
mrb_gprs_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=0;

#ifdef AVIXY_DEVICE
  avxnmInterfacePriority(AVXNM_NETIF_PRIORITIES_MODEM_ETHERNET_WIFI);
  gprsComSetSimSlot(simSlotSelected);
  gprsComSetModoOperacaoDesejado(GPRS_MODO_CONECTADO);
  ret = gprsComInit(prvSimPIN, prvIdDoSimCard, NULL, fillAPN);
#endif

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
	  gprsComSetModoOperacaoDesejado(GPRS_MODO_CONECTADO);
  }else{
	  gprsComSetModoOperacaoDesejado(GPRS_MODO_DESLIGADO);
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
  int keep_alive=300000, timeout=0, ret=0;

  apn   = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@apn"));
  sAPN  = mrb_str_to_cstr(mrb, apn);

  user  = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@user"));
  sUser = mrb_str_to_cstr(mrb, user);

  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
  sPass    = mrb_str_to_cstr(mrb, password);

  strncpy(strGPRSApn, sAPN, sizeof(strGPRSApn));
  strncpy(strGPRSUser, sUser, sizeof(strGPRSUser));
  strncpy(strGPRSPasswd, sPass, sizeof(strGPRSPasswd));

#ifdef AVIXY_DEVICE  
  ret = gprsComSetModoOperacaoDesejado(GPRS_MODO_CONECTADO);
#endif

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
  state = gprsComGetCurrentState();

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
  mrb_int ret=0;

#ifdef AVIXY_DEVICE
  gprsComSetModoOperacaoDesejado(GPRS_MODO_REGISTRADO);
#endif

  return mrb_true_value();
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
}

