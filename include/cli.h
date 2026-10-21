
#include <exception>
#include <unistd.h>
#define CLI_HAVE_ISATTY 1
#include "mmu.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

static const int CLI_LINE_SZ = 256;
static const int CLI_MAX_ARGS = 8;

struct sim {
    physical_mem &mem;
    frame_allocator &frames;
    page_table &pt;
    mmu_ &mmu;
};

static int tokenize(char *line, char *argv[], int max_args);

static bool parse_u64(const char *s, int base, uint64_t *out);

static bool parse_perm(const char *s, uint8_t *out);

static void print_help();

static void cmd_map(sim &s, int argc, char *argv[]);

static void cmd_unmap(sim &s, int argc, char *argv[]);

static void cmd_translate(sim &s, int argc, char *argv[]);

static void cmd_read(sim &s, int argc, char *argv[]);

static void cmd_write(sim &s, int argc, char *argv[]);

static void cmd_stats(sim &s);
