 
#include <iwlib.h>

int connect_wifi()
{
  struct iwreq wrq;
  char essid[IW_ESSID_MAX_SIZE + 1];
  unsigned char key[IW_ENCODING_TOKEN_MAX];
  int32_t keylen = 0;

  int wlsock = iw_sockets_open();

  const char *itfName = "wlan0";
  const char *networkName = "linksys";
  const char *com_key = "9F1C3EE11CBA230B27BF1C1B6F";
  if(wlsock < 0)
    return -1;

  memset(&wrq,0,sizeof(struct iwreq));

  keylen = iw_in_key_full(wlsock, itfName, com_key, key, &wrq.u.data.flags);
  if(keylen <= 0)
    return -1;

  wrq.u.data.length   = keylen;
  wrq.u.data.pointer  = (caddr_t) key;
  wrq.u.data.flags |= IW_ENCODE_RESTRICTED;

  if(iw_set_ext(wlsock, itfName, SIOCSIWENCODE, &wrq) < 0)
    return -1;

  memset(&wrq,0,sizeof(struct iwreq));

  wrq.u.mode = IW_MODE_INFRA;

  if(iw_set_ext( wlsock, itfName, SIOCSIWMODE, &wrq) < 0)
    return -1;

  if(strlen(networkName) > IW_ESSID_MAX_SIZE)
    return -1;

  memset(essid,0,IW_ESSID_MAX_SIZE + 1);
  memset(&wrq,0,sizeof(struct iwreq));

  strncpy(essid,networkName,strlen(networkName));
  wrq.u.essid.flags = 1;
  wrq.u.essid.pointer = (caddr_t) essid;
  wrq.u.essid.length = strlen(essid) + 1;

  if(iw_set_ext( wlsock, itfName, SIOCSIWESSID, &wrq) < 0)
    return -1;

  iw_sockets_close(wlsock);

  return 0;

}

