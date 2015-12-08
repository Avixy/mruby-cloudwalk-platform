#include "mruby.h"
#include <stddef.h>
#include <string.h>

#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#ifdef AVIXY_DEVICE
#include "network/network.h"
#include "wifi/wifi_interface.h"
#endif

#if MRUBY_RELEASE_NO < 10000
  #include "error.h"
#else
  #include "mruby/error.h"
#endif

static mrb_value
mrb_wifi_start(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=0;

#ifdef AVIXY_DEVICE
  avxnmInterfacePriority(AVXNM_NETIF_PRIORITIES_WIFI_ETHERNET_MODEM);
#endif

  return mrb_fixnum_value(ret);
}

/*Turn on/off the power 1 - on 0 - off*/
static mrb_value
mrb_wifi_power(mrb_state *mrb, mrb_value klass)
{
  mrb_int state, ret;
  mrb_get_args(mrb, "i", &state);

#ifdef AVIXY_DEVICE
  ret=wifiComInit(NULL, NULL); // No wifi listeners are needed. Ruby does the pooling via connected state

  ret = wifiComSetModoOperacaoDesejado(WIFI_MODO_INICIALIZADO);
#endif

  return mrb_fixnum_value(ret);
}

#ifdef AVIXY_DEVICE
static enum avxwifi_encryptions getAvixyEncryptionFromAuthStrAndCypherStr(const char * sAuthentication, const char * sCypher){
	if (strcmp("open", sAuthentication) ==0 ) return WIFI_ENCRYPTION_NONE;
	if (strcmp("wep", sAuthentication) ==0 ){
		if (strcmp("wep64", sCypher) ==0 ) return WIFI_ENCRYPTION_WEP_64;
		if (strcmp("wep128", sCypher) ==0 ) return WIFI_ENCRYPTION_WEP_128;
		if (strcmp("wepx", sCypher) ==0 ) return WIFI_ENCRYPTION_WEP_256; // TODO: Veririficar se wepx = 256
	}
	if (strcmp("wepshared", sAuthentication) ==0 ){
			if (strcmp("wep64", sCypher) ==0 ) return WIFI_ENCRYPTION_WEP_64;
			if (strcmp("wep128", sCypher) ==0 ) return WIFI_ENCRYPTION_WEP_128;
			if (strcmp("wepx", sCypher) ==0 ) return WIFI_ENCRYPTION_WEP_256; // TODO: Veririficar se wepx = 256
		}
	if (strcmp("wpapsk", sAuthentication) ==0 ) return WIFI_ENCRYPTION_WPA_PSK;
	if (strcmp("wpa2psk", sAuthentication) ==0 ) return WIFI_ENCRYPTION_WPA2_PSK;
	if (strcmp("wpawpa2psk", sAuthentication) ==0 ) return WIFI_ENCRYPTION_WPA2_PSK; //TODO: acredito que esse seja wpa ou wpa2, portanto achei melhor escolher o 2.

	//Os outros modos não são suportados.
	return -1;
}
#endif

extern int connectUsingDHCP;

static mrb_value
mrb_wifi_connect(mrb_state *mrb, mrb_value klass)
{
  mrb_value password, essid, bssid, channel, mode, authentication, cipher;
  const char *sPassword, *sEssid, *sBssid, *sChannel, *sCipher, *sMode, *sAuthentication;
  int timeout=60000;
  mrb_int ret = 0;

  essid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@essid"));
  sEssid = mrb_str_to_cstr(mrb, essid);

  bssid = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@bssid"));
  sBssid = mrb_str_to_cstr(mrb, bssid);

  channel = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@channel"));
  sChannel = mrb_str_to_cstr(mrb, channel);

  mode = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@mode"));
  sMode = mrb_str_to_cstr(mrb, mode);

#ifdef AVIXY_DEVICE
  enum avxwifi_modes avx_mode;
  enum avxwifi_encryptions avx_wifi_encryption;
  
  if (strcmp("ibss",sMode)==0){
	  avx_mode = WIFI_MODE_ADHOC;
	  //TODO: Ver como fazer adhoc no 3400
  } else {
	  avx_mode = WIFI_MODE_INFRASTRUCTURE;
  }
#endif

  authentication = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@authentication"));
  sAuthentication = mrb_str_to_cstr(mrb, authentication);

  password = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@password"));
  sPassword = mrb_str_to_cstr(mrb, password);

  cipher = mrb_cv_get(mrb, klass, mrb_intern_lit(mrb, "@cipher"));
  sCipher = mrb_str_to_cstr(mrb, cipher);

#ifdef AVIXY_DEVICE
  avx_wifi_encryption = getAvixyEncryptionFromAuthStrAndCypherStr(sAuthentication, sCipher);

  if (avx_wifi_encryption == -1){
	  printf("Modo de conexão não suportado!\n");
	  //TODO: Reportar erro pra plataforma.
	  ret = -1;
  } else {

	  wifiComInicOpcoesRedeWifi();
	  wifiComAddOpcaoRedeWifi((char *)sEssid , (char *)sPassword, avx_wifi_encryption, 0); // forcing index 0

	  wifiComSetDHCP(connectUsingDHCP);
	  //wifiComSetIPFixo("192.168.1.248", "192.168.1.1", "192.168.1.1", "192.168.1.1", "255.255.255.0");

	  ret = wifiComSetModoOperacaoDesejado(WIFI_MODO_CONECTADO_COM_IP);
  }
#endif

  return mrb_fixnum_value(ret);
}

/*0   -> Sucess*/
/*1   -> In Progress*/
/*< 0 -> Fail*/
static mrb_value mrb_wifi_connected_m(mrb_state *mrb, mrb_value klass)
{
	char sEssid[32 + 1] = "                                \0";
	char sBssid[19 + 1] = "                   \0";
	mrb_int iRssi, ret;

#ifdef AVIXY_DEVICE
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
#endif

	/*TODO 	O que colocar em essid, bssid e iRssi?*/
	if (ret == 0) {
	/*mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@essid"), mrb_str_new_cstr(mrb, sEssid));*/
	/*mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@bssid"), mrb_str_new_cstr(mrb, sBssid));*/
	/*mrb_cv_set(mrb, klass, mrb_intern_lit(mrb, "@rssi"), mrb_fixnum_value(iRssi));*/
	}

	return mrb_fixnum_value(ret);
}

static mrb_value
mrb_wifi_disconnect(mrb_state *mrb, mrb_value klass)
{
  mrb_int ret=0;

#ifdef AVIXY_DEVICE
  ret = wifiComSetModoOperacaoDesejado(WIFI_MODO_INICIALIZADO);
#endif

  return mrb_fixnum_value(ret);
}

void
mrb_wifi_init(mrb_state *mrb)
{
  struct RClass *platform, *network, *wifi;

  platform = mrb_class_get(mrb, "Platform");
  network  = mrb_class_get_under(mrb, platform, "Network");
  wifi     = mrb_define_class_under(mrb, network, "Wifi", mrb->object_class);

  mrb_define_class_method(mrb, wifi, "start", mrb_wifi_start, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "power", mrb_wifi_power, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, wifi, "connect", mrb_wifi_connect, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, wifi, "connected?", mrb_wifi_connected_m, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, wifi, "disconnect", mrb_wifi_disconnect, MRB_ARGS_NONE());
}

