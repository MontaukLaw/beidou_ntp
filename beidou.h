#ifndef _BEI_DOU_H__
#define _BEI_DOU_H__

#define RESONABLE_YEAR_MAX 2025
#define RESONABLE_YEAR_MIN 2023
#define NTP_PORT 123

// ntp的包大小
#define NTP_PACKET_SIZE 48
#define REV_FULL_SIZE 48

// unix时钟是从1970年开始的, ntp时钟是从1900年开始, 中间差了2208988800秒
#define NTP_UNIX_OFFSET 2208988800UL


void beidou_process(void *ptr);

#endif
