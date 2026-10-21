#include <exception>
#define CLI_HAVE_ISATTY 1
#include "../include/cli.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

static int tokenize(char *line, char *argv[], int max_args) {

    int argc = 0;
    char *tok = std::strtok(line, " \t\r\n");
    while (tok != nullptr) {
        if (argc == max_args)
            return -1;
        argv[argc++] = tok;
        tok = std::strtok(nullptr, " \t\r\n");
    }
    return argc;
}

static bool parse_u64(const char *s, int base, uint64_t *out) {

    if (s[0] == '-')
        return false;
    char *end = nullptr;
    errno = 0;
    uint64_t v = std::strtoull(s, &end, base);

    if (end == s || *end != '\0' || errno == ERANGE)
        return false;
    *out = (uint64_t)v;
    return true;
}

static bool parse_perm(const char *s, uint8_t *out) {
    uint8_t perm = 0;
    if (s[0] == '\0')
        return false;
    for (const char *p = s; *p != '\0'; p++) {
        if (*p == 'r')
            perm |= static_cast<uint8_t>(perms::read);
        else if (*p == 'w')
            perm |= static_cast<uint8_t>(perms::write);
        else if (*p == 'x')
            perm |= static_cast<uint8_t>(perms::execute);
        else
            return false;
    }
    *out = perm;
    return true;
}

static void print_help() {
    std::puts(
        "commands (addresses are hex, e.g. 1000 or 0x1000; max 39 bits):\n"
        "  map <va> <perm>        map the page containing va; perm = r, w, x, "
        "rw, rwx ...\n"
        "  unmap <va>             unmap that page and free its frame\n"
        "  translate <va> [r|w]   show the physical address (default: r)\n"
        "  read <va>              read one byte through the MMU\n"
        "  write <va> <value>     write one byte (value 0-255, decimal or "
        "0x..)\n"
        "  dump                   print the page-table tree contents\n"
        "  stats                  show frame allocator usage\n"
        "  help                   this text\n"
        "  quit                   exit");
}

static void cmd_map(sim &s, int argc, char *argv[]) {
    uint64_t va;
    uint8_t perm;
    if (argc != 3) {
        std::puts("usage: map <va> <perm>");
        return;
    }
    if (!parse_u64(argv[1], 16, &va)) {
        std::puts("error: bad address");
        return;
    }
    if (!parse_perm(argv[2], &perm)) {
        std::puts("error: perm must be a combination of r, w, x");
        return;
    }

    s.pt.map(va, perm);

    struct walk_result w = s.pt.walk(va);

    // The frame allocator hands back frames as-is. Zero it here (the CLI
    // plays the role of the OS) so a page that is unmapped and mapped
    // again never exposes its previous contents.
    s.mem.zero_range(w.frame_number * FRAME_SZ, FRAME_SZ);

    std::printf("mapped va 0x%llx -> frame %llu (paddr 0x%llx)\n",
                (unsigned long long)(va & ~0xFFFull),
                (unsigned long long)w.frame_number,
                (unsigned long long)(w.frame_number * FRAME_SZ));
}

static void cmd_unmap(sim &s, int argc, char *argv[]) {
    uint64_t va;
    if (argc != 2) {
        std::puts("usage: unmap <va>");
        return;
    }
    if (!parse_u64(argv[1], 16, &va)) {
        std::puts("error: bad address");
        return;
    }

    struct walk_result w = s.pt.walk(va);
    if (!w.present) {
        std::puts("not mapped, nothing to do");
        return;
    }
    s.pt.unmap(va);
    std::printf("unmapped va 0x%llx (frame %llu freed)\n",
                (unsigned long long)(va & ~0xFFFull),
                (unsigned long long)w.frame_number);
}

static void cmd_translate(sim &s, int argc, char *argv[]) {
    uint64_t va;
    bool is_write = false;
    if (argc != 2 && argc != 3) {
        std::puts("usage: translate <va> [r|w]");
        return;
    }
    if (!parse_u64(argv[1], 16, &va)) {
        std::puts("error: bad address");
        return;
    }
    if (argc == 3) {
        if (std::strcmp(argv[2], "w") == 0)
            is_write = true;
        else if (std::strcmp(argv[2], "r") == 0)
            is_write = false;
        else {
            std::puts("usage: translate <va> [r|w]");
            return;
        }
    }

    uint64_t pa = s.mmu.translate(va, is_write);
    std::printf("va 0x%llx -> pa 0x%llx\n", (unsigned long long)va,
                (unsigned long long)pa);
}

static void cmd_read(sim &s, int argc, char *argv[]) {
    uint64_t va;
    if (argc != 2) {
        std::puts("usage: read <va>");
        return;
    }
    if (!parse_u64(argv[1], 16, &va)) {
        std::puts("error: bad address");
        return;
    }

    uint64_t pa = s.mmu.translate(va, /*is_write=*/false);
    uint8_t value = s.mem.read_byte(pa);
    std::printf("va 0x%llx -> pa 0x%llx : 0x%02x (%u)\n",
                (unsigned long long)va, (unsigned long long)pa, (unsigned)value,
                (unsigned)value);
}

static void cmd_write(sim &s, int argc, char *argv[]) {
    uint64_t va, value;
    if (argc != 3) {
        std::puts("usage: write <va> <value>");
        return;
    }
    if (!parse_u64(argv[1], 16, &va)) {
        std::puts("error: bad address");
        return;
    }
    if (!parse_u64(argv[2], 0, &value) || value > 0xFF) {
        std::puts("error: value must be 0-255");
        return;
    }

    uint64_t pa = s.mmu.translate(va, /*is_write=*/true);
    s.mem.write_byte(pa, (uint8_t)value);
    std::printf("va 0x%llx -> pa 0x%llx : wrote 0x%02x\n",
                (unsigned long long)va, (unsigned long long)pa,
                (unsigned)value);
}

static void cmd_stats(sim &s) {
    unsigned long long total = (unsigned long long)s.frames.total_count();
    unsigned long long free_ = (unsigned long long)s.frames.free_count();
    std::printf(
        "frames: %llu total, %llu used, %llu free (%llu KiB physical memory)\n",
        total, total - free_, free_,
        (unsigned long long)(physical_mem::phys_mem_sz() / 1024));
}

int main() {
    physical_mem mem;
    frame_allocator frames;
    page_table pt(frames);
    mmu_ mmu(pt, mem);

    struct sim sim_ = {mem, frames, pt, mmu};
    bool interactive = isatty(0) != 0;

    if (interactive)
        std::puts(
            "pagetable-walker simulator (3-level). type 'help' for commands. ");

    char line[CLI_LINE_SZ];
    while (true) {
        if (interactive) {
            std::fputs("ptw> ", stdout);
            std::fflush(stdout);
        }
        if (std::fgets(line, sizeof line, stdin) == nullptr)
            break;

        if (std::strchr(line, '\n') == nullptr && !std::feof(stdin)) {
            int c;
            while ((c = std::fgetc(stdin)) != '\n' && c != EOF) {
            }
            std::puts("error! line too long");
            continue;
        }

        line[std::strcspn(line, "\r\n")] = '\0';
        if (!interactive && line[0] != '\0')
            std::printf("ptw > %s\n", line);

        char *argv[CLI_MAX_ARGS];
        int argc = tokenize(line, argv, CLI_MAX_ARGS);
        if (argc == 0)
            continue;
        if (argc < 0) {
            std::puts("error: too many arguments");
            continue;
        }

        const char *cmd = argv[0];
        try {

            if (std::strcmp(cmd, "map") == 0)
                cmd_map(sim_, argc, argv);
            else if (std::strcmp(cmd, "unmap") == 0)
                cmd_unmap(sim_, argc, argv);
            else if (std::strcmp(cmd, "translate") == 0)
                cmd_translate(sim_, argc, argv);
            else if (std::strcmp(cmd, "read") == 0)
                cmd_read(sim_, argc, argv);
            else if (std::strcmp(cmd, "write") == 0)
                cmd_write(sim_, argc, argv);
            else if (std::strcmp(cmd, "dump") == 0)
                pt.dump();
            else if (std::strcmp(cmd, "stats") == 0)
                cmd_stats(sim_);
            else if (std::strcmp(cmd, "help") == 0)
                print_help();
            else if (std::strcmp(cmd, "quit") == 0 ||
                     std::strcmp(cmd, "exit") == 0)
                break;
            else
                std::printf("unknown command: %s (try 'help')\n", cmd);
        } catch (page_fault &f) {
            std::printf("FAULT (%s):  %s\n",
                        f.kind == fault_kind::NOT_PRESENT ? "not-present"
                                                          : "protection",
                        f.what());
        } catch (const std::exception &e) {
            std::printf("error: %s\n", e.what());
        }
    }
}
