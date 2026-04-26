#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "simulator.h"

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [--dry-run] <A.conf|path/to/A.conf>\n", prog);
}

static int has_conf_suffix(const char *path) {
    size_t len = strlen(path);
    return len >= 5 && strcmp(path + len - 5, ".conf") == 0;
}

static int parse_config_argument(
    const char *config_arg,
    char *topology_dir,
    size_t topology_dir_size,
    char *node_name
) {
    const char *slash = strrchr(config_arg, '/');
    const char *basename = slash ? slash + 1 : config_arg;

    if (!has_conf_suffix(basename) || basename[0] == '\0') {
        return -1;
    }

    *node_name = (char)toupper((unsigned char)basename[0]);

    if (slash) {
        size_t dir_len = (size_t)(slash - config_arg);
        if (dir_len == 0 || dir_len >= topology_dir_size) {
            return -1;
        }
        memcpy(topology_dir, config_arg, dir_len);
        topology_dir[dir_len] = '\0';
        return 0;
    }

    if (access(config_arg, F_OK) == 0) {
        if (snprintf(topology_dir, topology_dir_size, ".") >= (int)topology_dir_size) {
            return -1;
        }
        return 0;
    }

    if (snprintf(topology_dir, topology_dir_size, "topology") >= (int)topology_dir_size) {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    char topology_dir[256];
    char node_name = '\0';
    const char *config_arg = NULL;
    int dry_run = 0;

    if (argc == 3 && strcmp(argv[1], "--dry-run") == 0) {
        dry_run = 1;
        config_arg = argv[2];
    } else if (argc == 2) {
        config_arg = argv[1];
    } else {
        usage(argv[0]);
        return 1;
    }

    if (parse_config_argument(config_arg, topology_dir, sizeof(topology_dir), &node_name) != 0) {
        fprintf(stderr, "Invalid config path: %s\n", config_arg);
        usage(argv[0]);
        return 1;
    }

    if (dry_run) {
        if (preview_node_process(topology_dir, node_name) != 0) {
            fprintf(stderr, "Node config preview failed for %c using topology dir %s\n", node_name, topology_dir);
            return 1;
        }
        return 0;
    }

    if (run_node_process(topology_dir, node_name) != 0) {
        fprintf(stderr, "Node process failed for %c using topology dir %s\n", node_name, topology_dir);
        return 1;
    }

    return 0;
}
