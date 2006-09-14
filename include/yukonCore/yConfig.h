
#ifndef __YOKON_CORE_CONFIG_H__
#define __YOKON_CORE_CONFIG_H__

void yConfigServer(char server[256]);
void yConfigInterval(double *v);
void yConfigScale(char scale[64]);
void yConfigInsets(uint64_t v[4]);
void yConfigHotkey(char hotkey[64]);

#endif /* __YOKON_CORE_CONFIG_H__ */
