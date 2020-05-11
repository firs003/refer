#ifndef __VIS_UTIL_H__
#define __VIS_UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

int vis_uptime_str(char *str);
void print_in_hex(void *buf, size_t len, char *pre, char *end);

#ifdef __cplusplus
}
#endif

#endif //__VIS_UTIL__
