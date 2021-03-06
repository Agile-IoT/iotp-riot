/*
 * main.c
 *
 *  Created on: June, 18 2018
 *      Authors: Eduard Brehm
 *               Emmanuel Baccelli
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eid.h"
#include <xtimer.h>
#include "shell.h"
#include "periph/flashpage.h"

#define LINE_LEN            (16)

int IK_SIZE=32;
int TS_SIZE=8;
char dummy_data[1]="0";

/**
 * @brief   Allocate space for 1 flash page in RAM
 */
static uint8_t page_mem[FLASHPAGE_SIZE] = {0};

#ifdef MODULE_PERIPH_FLASHPAGE_RAW
/*
 * @brief   Allocate an aligned buffer for raw writings
 */
static char raw_buf[64] __attribute__ ((aligned (FLASHPAGE_RAW_ALIGNMENT)));

static uint32_t getaddr(const char *str)
{
    uint32_t addr = strtol(str, NULL, 16);

    return addr;
}
#endif


static void dumpchar(uint8_t mem)
{
    if (mem >= ' ' && mem <= '~') {
        printf("%c", mem);
    }
    else {
        printf("??");
    }
}


static void memdump(void *addr, size_t len)
{
    unsigned pos = 0;
    uint8_t *mem = (uint8_t *)addr;

    while (pos < (unsigned)len) {
        for (unsigned i = 0; i < len; i++) {
            dumpchar(mem[pos++]);
        }
        puts("");
    }
}

uint16_t read_counter(void) {
    uint16_t counter = 0;
    uint8_t high8 = page_mem[IK_SIZE];
    uint8_t low8 = page_mem[IK_SIZE+1];
    counter = high8 << 8;
    counter |= low8;
    return counter;    
}

static void dump_ik(void)
{
    puts("Key read in internal memory:");
    memdump(page_mem, IK_SIZE);
    puts("Counter read in internal memory: ");
    printf("%d \n", read_counter());
}

static int getpage(const char *str)
{
    int page = atoi(str);
    if ((page >= (int)FLASHPAGE_NUMOF) || (page < 0)) {
        printf("error: page %i is invalid\n", page);
        return -1;
    }
    return page;
}

int read_ik(int argc, char **argv) {
    if(!(argc > 1)) {
    puts("Input error: too few arguments");
    return -1;
    }
    int page = getpage(argv[1]);
    if (page < 0) {
        return 1;
    }

    flashpage_read(page, page_mem);
    printf("Read flash page %i into local page buffer\n", page);
    dump_ik();

    return 0;

}

int write_counter(uint16_t value, int page) {
    uint8_t high8 = (value >> 8) & 0x00FF;
    uint8_t low8 = value & 0x00FF;
    page_mem[IK_SIZE] = high8;
    page_mem[IK_SIZE+1] = low8;
    if (flashpage_write_and_verify(page, page_mem) != FLASHPAGE_OK) {
        printf("error: writing counter in page %i failed\n", page);
   	return 1;
      }
    return 0;
}

int write_ik(int argc, char **argv) {
    if(!(argc > 2)) {
    puts("Input error: too few arguments");
    return -1;
    }
    uint16_t *counter;
    void *data = argv[2];
    int data_len = strlen(argv[2]);
    if (data_len != IK_SIZE) {
    puts("Input error: key length is wrong");
    return -1;
    }
    int page = getpage(argv[1]);
    if ((page < 0) ) {
        return 1;
    }

    flashpage_write(page, NULL);
    memcpy(&page_mem[0], argv[2], data_len);
    page_mem[data_len]= 0x00;
    page_mem[data_len+1]= 0x00;
    for (unsigned i = data_len+2; i < FLASHPAGE_SIZE; i++) {
        memcpy(&page_mem[i], dummy_data, 1);
        }
    if (flashpage_write_and_verify(page, page_mem) != FLASHPAGE_OK) {
        printf("error: writing identify key in page %i failed\n", page);
   	return 1;
      }

    return 0;

}

int genEID(int argc, char **argv)
{
    (void) argc;
    if(argc > 2) {
      char *ikString = argv[1];
      int scaler = 0;
      int beacon_time_seconds = atoi(argv[2]);
      uint8_t eid[16] = {0};
      generateEID(ikString, scaler, beacon_time_seconds, eid);
      printf("Generated OTP: ");
      printHex(eid, 8);
    } else {
       printf("Input error: too few arguments\n");
    }
    return 0;
}

static int cmd_info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("Flash start addr:\t0x%08x\n", (int)CPU_FLASH_BASE);
    printf("Page size:\t\t%i\n", (int)FLASHPAGE_SIZE);
    printf("Number of pages:\t%i\n", (int)FLASHPAGE_NUMOF);

    return 0;
}


#ifdef MODULE_PERIPH_FLASHPAGE_RAW
static int cmd_write_raw(int argc, char **argv)
{
    uint32_t addr;

    if (argc < 3) {
        printf("usage: %s <addr> <data>\n", argv[0]);
        return 1;
    }

    addr = getaddr(argv[1]);

    /* try to align */
    memcpy(raw_buf, argv[2], strlen(argv[2]));

    flashpage_write_raw((void*)addr, raw_buf, strlen(raw_buf));

    printf("wrote local data to flash address %#lx of len %u\n",
           addr, strlen(raw_buf));
    return 0;
}
#endif

static int cmd_erase(int argc, char **argv)
{
    int page;

    if (argc < 2) {
        printf("usage: %s <page>\n", argv[0]);
        return 1;
    }

    page = getpage(argv[1]);
    if (page < 0) {
        return 1;
    }
    flashpage_write(page, NULL);

    printf("successfully erased page %i (addr %p)\n",
           page, flashpage_addr(page));
    return 0;
}

static int run_otp(int argc, char **argv)
{

   if(argc > 1) {
    char cnt[8 + 1] = {0};
    char ik[32 +1] = {0};
    uint16_t counter = 0;
    
    cnt[TS_SIZE] = '\0';
    ik[IK_SIZE] = '\0';
    
    while(true) {
        int page = getpage(argv[1]);
        if (page < 0) {
            return 1;
        }
        flashpage_read(page, page_mem);
        dump_ik();
        for (int i = 0; i < IK_SIZE; i++) {
            sprintf(&ik[i],"%d", page_mem[i]);
        }
        
        counter = read_counter();
        sprintf(&cnt[0],"%d",counter);
        char *gen[3] = {"genEID", ik, cnt};
        genEID(3, gen);
        counter++;
        flashpage_write(page, NULL);
        int res = write_counter(counter, page);

        xtimer_sleep(5);
    }
    }
    else {
    puts("Input error: too few arguments");
    return -1;
    }
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "info", "Show information about pages", cmd_info },
    { "write_ik", "Write key to the given page", write_ik },
    { "read_ik", "Read key from the given page", read_ik },
    { "geneid", "Generates an EID", genEID },
    { "run_otp", "Runs periodic periodic token generation", run_otp },
#ifdef MODULE_PERIPH_FLASHPAGE_RAW
    { "write_raw", "Write (ASCII, max 64B) data to the given address", cmd_write_raw },
#endif
    { "erase", "Erase the given page buffer", cmd_erase },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("ROM flash read write test\n");
    puts("Please refer to the README.md for further information\n");

    cmd_info(0, NULL);

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
