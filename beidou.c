#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "beidou.h"
#include <stdint.h>
#include <time.h>

// extern uint32_t ref_time;
extern uint32_t ref_time_int;
extern uint32_t ref_time_fract;

void set_linux_sys_time(char *newDateTime)
{
    // 设置新的日期和时间，格式为"月/日/年 时:分:秒"
    // = "10/20/23 14:30:00";
    // 2023-10-15 14:30:00

    // 构建要执行的命令字符串
    char command[100];
    snprintf(command, sizeof(command), "date -s \"%s\"", newDateTime);

    // 使用系统命令来修改日期和时间
    int status = system(command);
    if (status == 0)
    {
        printf("date changed: %s\n", newDateTime);
    }
    else
    {
        printf("change date failed\n");
    }

    return 0;
}

uint32_t get_current_fractional_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint32_t)(ts.tv_nsec);
    // // struct timespec ts;
    // // clock_gettime(CLOCK_REALTIME, &ts);
    // // return (uint32_t)(ts.tv_nsec * 4294.967296);
    // struct timeval tv;
    // srand(time(NULL));
    // uint32_t random_number = rand() % 1000;
    // unsigned long long nseconds = tv.tv_usec * 1000 + (unsigned long long)random_number;
    // // printf("ns: %llu\n", nseconds);
    // // float ntp_double = (float)nseconds / 1000000;
    // // printf("ntp_double: %f\n", ntp_double);
    // // unsigned long long sec = (unsigned long long)tv.tv_sec;
    // // printf("sec: %llu\n", sec);
    // return (uint32_t)nseconds;
}

char *new_getNthSegment(const char *str, int n)
{
    const char *start = str;
    char *end = NULL;
    int count = 0;

    while (count < n && *str != '\0')
    {
        if (*str == ',')
        {
            count++;
            if (count < n)
            {
                start = str + 1; // 移动到下一个字符
            }
        }
        str++;
    }

    if (count == n)
    {
        end = str;
        while (*str != ',' && *str != '\0')
        {
            str++;
        }
    }

    if (end != NULL)
    {
        int length = end - start;
        char *segment = (char *)malloc(length + 1);
        if (segment != NULL)
        {
            strncpy(segment, start, length);
            segment[length] = '\0';
            return segment;
        }
    }

    return NULL;
}

// 函数用于获取字符串中第n个部分（使用逗号分隔）
char *getNthSegment(const char *str, int n)
{
    char *token;
    char *copy = strdup(str); // 复制输入字符串以避免修改原始字符串
    char *segment = NULL;

    token = strtok(copy, ",");

    // 使用循环遍历分隔后的部分，找到第n个部分
    for (int i = 1; i < n && token != NULL; i++)
    {
        token = strtok(NULL, ",");
    }

    if (token != NULL)
    {
        segment = strdup(token);
    }

    free(copy); // 释放复制的字符串

    return segment;
}

int countOccurrences(const char *str, char symbol)
{
    int count = 0;

    while (*str)
    {
        if (*str == symbol)
        {
            count++;
        }
        str++;
    }
    return count;
}

void parseGNRMCTimeDate(const char *nmeaSentence, char *timeBuffer, char *dateBuffer)
{
    // $GNRMC,023240.00,V,,,,,,,221023,,,N,V*1E
    // $GNRMC,025237.00,A,2241.56736,N,11413.27346,E,0.106,,221023,,,A,V*14
    // $GNRMC,025237.00,A,2241.56736,N,11413.27346,E,0.106,,221023,,,A,V*14
    // $GNRMC,025236.00,A,2241.56734,N,11413.27344,E,0.539,136.54,221023,,,A,V*06
    // 检查语句是否以$GNRMC开头
    // printf("total:%d\n", countOccurrences(nmeaSentence, ','));
    int commaCounter = countOccurrences(nmeaSentence, ',');
    if (commaCounter != 13)
    {
        return;
    }
    if (memcmp(new_getNthSegment(nmeaSentence, 1), "$GNRMC", 6) == 0)
    {

        if (memcmp(new_getNthSegment(nmeaSentence, 3), "A", 1) != 0)
        {
            printf("gps status is not ready\n");
            return;
        }
        printf("nmeaSentence: %s\n", nmeaSentence);
        timeBuffer = new_getNthSegment(nmeaSentence, 2);
        // printf("timeBuffer: %s\n", timeBuffer);
        dateBuffer = new_getNthSegment(nmeaSentence, 10);
        // printf("dateBuffer: %s\n", dateBuffer);

        if (timeBuffer == NULL || dateBuffer == NULL)
        {
            return;
        }

        int year = 2000 + (dateBuffer[4] - '0') * 10 + (dateBuffer[5] - '0');
        int month = (dateBuffer[2] - '0') * 10 + (dateBuffer[3] - '0');
        int day = (dateBuffer[0] - '0') * 10 + (dateBuffer[1] - '0');
        int hour = (timeBuffer[0] - '0') * 10 + (timeBuffer[1] - '0');
        int minute = (timeBuffer[2] - '0') * 10 + (timeBuffer[3] - '0');
        int second = (timeBuffer[4] - '0') * 10 + (timeBuffer[5] - '0');

        if (year > RESONABLE_YEAR_MAX || year < RESONABLE_YEAR_MIN)
        {
            printf("Wrong time format: year: %d \n", year);
            return;
        }
        char newDateTime[20];
        snprintf(newDateTime, sizeof(newDateTime), "%d-%d-%d %d:%d:%d", year, month, day, hour, minute, second);
        printf("renew newDateTime by gps: %s\n", newDateTime);
        set_linux_sys_time(newDateTime);

        ref_time_int = time(NULL) + NTP_UNIX_OFFSET;
        ref_time_fract = get_current_fractional_time();
        printf("ref_time: %lu ref_time_fract :%lu\n", ref_time_int, ref_time_fract);
    }

    // printf("year: %d\n", year);
    // printf("month: %d\n", month);
    // printf("day: %d\n", day);
    // printf("hour: %d\n", hour);
    // printf("minute: %d\n", minute);
    // printf("second: %d\n", second);
}

void beidou_process(void *ptr)
{
    printf("beidou_process starting\n");
    // int serial_port_num = 4;
    char serial_port[15];
    sprintf(serial_port, "/dev/ttyS%d", 4);
    int serial_fd;
    // char serial_port = "/dev/ttyS4";
    printf("opening serial_port: %s\n", serial_port);
    serial_fd = open(serial_port, O_RDWR | O_NOCTTY);
    if (serial_fd == -1)
    {
        perror("Failed to open serial port");
        return;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(serial_fd, &tty) != 0)
    {
        perror("Error from tcgetattr");
        return;
    }

    cfsetospeed(&tty, B38400);
    cfsetispeed(&tty, B38400);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0)
    {
        perror("Error from tcsetattr");
        return;
    }
    char rx_buffer[2048];
    char timeBuffer[12]; // 存储时间信息的缓冲区
    char dateBuffer[7];  // 存储日期信息的缓冲区
    while (1)
    {
        usleep(500);
        int bytes_read = read(serial_fd, rx_buffer, sizeof(rx_buffer));
        if (bytes_read > 0)
        {
            rx_buffer[bytes_read] = '\0';
            // printf("\rrx_buffer: \n %s ", rx_buffer);
            parseGNRMCTimeDate(rx_buffer, timeBuffer, dateBuffer);
        }
        else
        {
            printf("No data received.\n");
        }
    }

    close(serial_fd);

    return;
}