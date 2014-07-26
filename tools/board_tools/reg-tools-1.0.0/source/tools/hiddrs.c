#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>

#include "memmap.h"
#include "hi.h"
#include "strfunc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int errno;

#define OPEN_FILE "/dev/mem"
#define DDR_STAT_SIG SIGUSR1

#define PAGE_SIZE_MASK 0xfffff000

#define TIMER_INTERVAL 1
#define DDRC_ADDR 0x20110000
#define DDRC_ADDR2 0x20120000
#define DDRC_MAP_LENGTH 0x1000
#define DDRC_TEST7 0x260
#define DDRC_TEST8 0x264
#define DDRC_TEST9 0x268

#define DDRC_VALUE 0xd47868c0
#define ONE_SECOND 0x47868c0
#define MAX_SECOND 3

#define reg_read(addr) (*(volatile unsigned int *)(addr))
#define reg_write(addr, val) (*(volatile unsigned int *)(addr) = (val))

static unsigned int *ddrc_addr;
static unsigned int *ddrc_addr2;
static int fd;
static struct sigaction sa;
static timer_t tt;
static unsigned int timer_interval = TIMER_INTERVAL;
static unsigned int reg_value;
static unsigned int ddrc_num;

#define IO_ADDRESS(addr) ((unsigned char *)(ddrc_addr) + addr)
#define IO_ADDRESS2(addr) ((unsigned char *)(ddrc_addr2) + addr)

static int ddrc_remap(unsigned int ddrc_num);
static void ddrc_unmap(unsigned int ddrc_num);

static void ddr_statistic(int n)
{
	unsigned int ddr_read = 0;
	unsigned int ddr_write = 0;
	unsigned int tmp = 0;
	double tmp2 = 0.0;
	float ddr_rate = 0.0f;
	float ddr_rate2 = 0.0f;
	tmp = reg_read(IO_ADDRESS(DDRC_TEST7));

	while (tmp & (0x1 << 30))
		tmp = reg_read(IO_ADDRESS(DDRC_TEST7));

	ddr_read = reg_read(IO_ADDRESS(DDRC_TEST8));
	ddr_write = reg_read(IO_ADDRESS(DDRC_TEST9));
	tmp = reg_value & 0xfffffff;
	tmp *= 2;
	/*overflow maybe*/
	tmp2 = ddr_read + ddr_write;
	ddr_rate = (tmp2 / tmp) * 100;

	/*statistic ddrc1*/
	if (ddrc_num == 2) {
		tmp = reg_read(IO_ADDRESS2(DDRC_TEST7));
		while (tmp & (0x1 << 30))
			tmp = reg_read(IO_ADDRESS2(DDRC_TEST7));

		ddr_read = reg_read(IO_ADDRESS2(DDRC_TEST8));
		ddr_write = reg_read(IO_ADDRESS2(DDRC_TEST9));
		tmp = reg_value & 0xfffffff;
		tmp *= 2;
		tmp2 = ddr_read + ddr_write;
		ddr_rate2 = (tmp2 / tmp) * 100;
	}

	if (ddrc_num != 2) {
		printf("%0.2f%%\n", ddr_rate);
		reg_write(IO_ADDRESS(DDRC_TEST7), reg_value);
	} else {
		printf("%0.2f%% %0.2f%%\n", ddr_rate, ddr_rate2);
		reg_write(IO_ADDRESS(DDRC_TEST7), reg_value);
		reg_write(IO_ADDRESS2(DDRC_TEST7), reg_value);
	}
	return;
}

static void ddr_ctrl_c(int n)
{
	timer_delete(tt);
	ddrc_unmap(ddrc_num);
	close(fd);
	_exit(0);
}

static int ddrc_remap(unsigned int ddrc_num)
{
	unsigned int phy_addr_in_page;
	unsigned int ddrc_addr_tmp = DDRC_ADDR;
	if (ddrc_num == 0)
		ddrc_addr_tmp = DDRC_ADDR;
	else if (ddrc_num == 1)
		ddrc_addr_tmp = DDRC_ADDR2;

	phy_addr_in_page = ddrc_addr_tmp & PAGE_SIZE_MASK;
	ddrc_addr = (unsigned int *)mmap(NULL, DDRC_MAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
	if (ddrc_addr == MAP_FAILED) {
		printf("ddr%d statistic mmap failed.\n", ddrc_num);
		goto ERR1;
	}

	if (ddrc_num == 2) {
		ddrc_addr_tmp = DDRC_ADDR2;
		phy_addr_in_page = ddrc_addr_tmp & PAGE_SIZE_MASK;
		ddrc_addr2 = (unsigned int *)mmap(NULL, DDRC_MAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);	
		if (ddrc_addr2 == MAP_FAILED) {
			printf("ddr%d statistic mmap failed.\n", ddrc_num - 1);
			goto ERR2;
		}
	}
	printf("0x%p, 0x%p\n", ddrc_addr, ddrc_addr2);
	return 0;
ERR2:
	munmap(ddrc_addr, DDRC_MAP_LENGTH);
ERR1:
	return -1;
}

static void ddrc_unmap(unsigned int ddrc_num)
{
	if (ddrc_num == 0 || ddrc_num == 1)
		munmap(ddrc_addr, DDRC_MAP_LENGTH);
	else if (ddrc_num == 2) {
		munmap(ddrc_addr, DDRC_MAP_LENGTH);
		munmap(ddrc_addr2, DDRC_MAP_LENGTH);
	}
}

//int main(int argc, char *argv[])
HI_RET hiddrs(int argc , char* argv[])
{
	int ret = 0;
	struct itimerspec tspec;
	struct sigevent timer_event;
	unsigned int second = 0;

	if (argc == 3) {
		ddrc_num = atoi(argv[1]);
		if (ddrc_num < 0 && ddrc_num > 2)
			goto ERR;

		second = atoi(argv[2]);
		if (second > MAX_SECOND)
			goto ERR;
	} else if (argc == 2) {
		ddrc_num = atoi(argv[1]);
		if (ddrc_num < 0 && ddrc_num > 2)
			goto ERR;
		second = TIMER_INTERVAL;
	} else {
ERR:
		printf("ddr statistic usage like this:\n");
		printf("\tddrs [second=1,2,3]\n");
		printf("\teg. ddrs 2\n");
		return -1;
	}

	timer_interval = second;

	fd = open(OPEN_FILE, O_RDWR | O_SYNC);
	if (-1 == fd) {
		perror("ddr statistic open failed:");
		goto ERR0;
	}
	
	ret = ddrc_remap(ddrc_num);
	if (ret)
		goto ERR1;

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = ddr_statistic;

	sigemptyset(&sa.sa_mask);

	if (sigaction(DDR_STAT_SIG, &sa, NULL)) {
		printf("ddr statistic install signal failed.\n");
		goto ERR1;
	}

	timer_event.sigev_notify = SIGEV_SIGNAL;
	timer_event.sigev_signo = DDR_STAT_SIG;
	timer_event.sigev_value.sival_ptr = (void *)&tt;

	if (timer_create(CLOCK_REALTIME, &timer_event, &tt) < 0) {
		printf("ddr statistic timer create failed.\n");
		goto ERR2;
	}

	tspec.it_value.tv_sec = timer_interval;
	tspec.it_value.tv_nsec = 0;
	tspec.it_interval.tv_sec = timer_interval;
	tspec.it_interval.tv_nsec = 0;

	reg_value = timer_interval * ONE_SECOND;
	reg_value &= 0xfffffff;
	reg_value |= 0xd0000000;
	reg_write(IO_ADDRESS(DDRC_TEST7), reg_value);
	if (ddrc_num == 2)
		reg_write(IO_ADDRESS2(DDRC_TEST7), reg_value);

	if (timer_settime(tt, TIMER_ABSTIME, &tspec, NULL) < 0) {
		printf("ddr statistic set timer failed.\n");
		goto ERR3;
	}

	printf("===== ddr statistic =====\n");

	struct sigaction sa_ctrl_c;
	sa_ctrl_c.sa_flags = SA_RESTART;
	sa_ctrl_c.sa_handler = ddr_ctrl_c;
	sigemptyset(&sa_ctrl_c.sa_mask);

	if (sigaction(SIGINT, &sa_ctrl_c, NULL)) {
		printf("ddr statistic catch SIGINT signal failed.\n");
		goto ERR3;
	}

	while (1)
		sleep(1);
	return 0;

ERR3:
	timer_delete(tt);
ERR2:
	ddrc_unmap(ddrc_num);
ERR1:
	close(fd);
ERR0:
	return -1;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
