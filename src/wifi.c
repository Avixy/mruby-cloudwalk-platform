#include "mruby.h"
#include <stddef.h>
#include <string.h>

#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

/* Avixy SDK Includes */
#include "network/network.h"
#include "wifiInterface.h"


#if MRUBY_RELEASE_NO < 10000
  #include "error.h"
#else
  #include "mruby/error.h"
#endif

static mrb_value
mrb_wifi_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=0;

  avxnmInterfacePriority(AVXNM_NETIF_PRIORITIES_WIFI_ETHERNET_MODEM);

  return mrb_fixnum_value(ret);
}

static void fxProgresscallback(void)
{
	printf("!STATUS: %d\n", wifiComGetCurrentState());
}

/*Turn on/off the power 1 - on 0 - off*/
static mrb_value
mrb_wifi_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int state, ret;
  mrb_get_args(mrb, "i", &state);

  ret = wifiComInit(fxProgresscallback);

  ret = wifiComSetModoOperacaoDesejado(WIFI_MODO_INICIALIZADO);

  return mrb_fixnum_value(ret);
}


static enum avxwifi_encryptions getAvixyEncryptionFromAuthStrAndCypherStr(const char * sAuthentication, const char * sCypher){

	if (strcmp("open", sAuthentication) ==0 )
		return WIFI_ENCRYPTION_NONE;

	if (strcmp("wep", sAuthentication) ==0 )
	{
		if (strcmp("wep64", sCypher) ==0 )
			return WIFI_ENCRYPTION_WEP_64;

		if (strcmp("wep128", sCypher) ==0 )
			return WIFI_ENCRYPTION_WEP_128;

		if (strcmp("wepx", sCypher) ==0 )
			return WIFI_ENCRYPTION_WEP_256;
	}

	if (strcmp("wepshared", sAuthentication) ==0 )
	{
			if (strcmp("wep64", sCypher) ==0 )
				return WIFI_ENCRYPTION_WEP_64;

			if (strcmp("wep128", sCypher) ==0 )
				return WIFI_ENCRYPTION_WEP_128;

			if (strcmp("wepx", sCypher) ==0 )
				return WIFI_ENCRYPTION_WEP_256;
	}

	if (strcmp("wpapsk", sAuthentication) ==0 )
		return WIFI_ENCRYPTION_WPA_PSK;

	if (strcmp("wpa2psk", sAuthentication) ==0 )
		return WIFI_ENCRYPTION_WPA2_PSK;

	if (strcmp("wpawpa2psk", sAuthentication) ==0 )
		return WIFI_ENCRYPTION_WPA2_PSK;

	//Os outros modos não são suportados.
	return -1;
}

extern int connectUsingDHCP;

static mrb_value mrb_wifi_connect(mrb_state *mrb, mrb_value klass)
{
	enum avxwifi_modes avx_mode;
	enum avxwifi_encryptions avx_wifi_encryption;

	mrb_int ret = 0;

	  mrb_value password, essid, bssid, channel, mode, authentication, cipher;
	  const char *sPassword, *sEssid, *sBssid, *sChannel, *sCipher, *sMode, *sAuthentication;

	  essid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@essid"));
	  sEssid = mrb_str_to_cstr(mrb, essid);

	  bssid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@bssid"));
	  sBssid = mrb_str_to_cstr(mrb, bssid);

	  channel = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@channel"));
	  sChannel = mrb_str_to_cstr(mrb, channel);

	  mode = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@mode"));
	  sMode = mrb_str_to_cstr(mrb, mode);

	  if (strcmp("ibss",sMode)==0)
	  {
		  avx_mode = WIFI_MODE_ADHOC;
	  }
	  else
	  {
		  avx_mode = WIFI_MODE_INFRASTRUCTURE;
	  }

	  authentication = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@authentication"));
	  sAuthentication = mrb_str_to_cstr(mrb, authentication);

	  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
	  sPassword = mrb_str_to_cstr(mrb, password);

	  cipher = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@cipher"));
	  sCipher = mrb_str_to_cstr(mrb, cipher);

	avx_wifi_encryption = getAvixyEncryptionFromAuthStrAndCypherStr(sAuthentication, sCipher);

	printf("sAuthentication: %s, sCipher: %s, sEssid: %s, sBssid: %s, sPassword: %s, avx_wifi_encryption: %d\n", sAuthentication, sCipher, sEssid, sBssid, sPassword, avx_wifi_encryption);

	if (avx_wifi_encryption == -1)
	{
		printf("Modo de conexão não suportado!\n");
		ret = -1;
	}
	else
	{
		wifiComInicOpcoesRedeWifi();
		wifiComAddOpcaoRedeWifi((char *) sEssid, (char *) sPassword, avx_wifi_encryption);

		wifiComSetDHCP(connectUsingDHCP);

		ret = wifiComSetModoOperacaoDesejado(WIFI_MODO_CONECTADO_COM_IP);
	}

	return mrb_fixnum_value(ret);
}

/*0   -> Sucess*/
/*1   -> In Progress*/
/*< 0 -> Fail*/
static mrb_value mrb_wifi_connected_m(mrb_state *mrb, mrb_value klass)
{
	mrb_int ret=0;

	int state = wifiComGetCurrentState();

	switch (state)
	{
	case W_NO_DEVICE:
	case W_NOT_INITIALIZED:
		ret = -state;
		break;
	case W_INITIALIZING:
	case W_INITIALIZED:
	case W_ACCESS_POINT_CONNECTING:
	case W_ACCESS_POINT_CONNECTED:
	case W_CONNECTING:
	case W_DISCONNECTING:
	case W_SHUTTING_DOWN:
	case W_ACCESS_POINT_DISCONNECTING:
		ret = 1;
		break;
	case W_CONNECTED:
		ret = 0;
		break;
	}

	return mrb_fixnum_value(ret);
}

static mrb_value
mrb_wifi_disconnect(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=0;

  ret = wifiComSetModoOperacaoDesejado(WIFI_MODO_INICIALIZADO);

  return mrb_fixnum_value(ret);
}

void
mrb_init_wifi(mrb_state *mrb)
{
  struct RClass *network, *wifi;

  network = mrb_class_get(mrb, "Network");
  wifi    = mrb_define_class_under(mrb, network, "Wifi", mrb->object_class);

  mrb_define_class_method(mrb, wifi, "start", mrb_wifi_start, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "power", mrb_wifi_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, wifi, "connect", mrb_wifi_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, wifi, "connected?", mrb_wifi_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "disconnect", mrb_wifi_disconnect, MRB_ARGS_NONE());
}

