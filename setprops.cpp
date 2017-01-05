#include <fcntl.h>
#include <string>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

int trigger_prop = 0;
static void load_properties_from_file(const char *, const char *);

// from https://github.com/android/platform_system_core/blob/nougat-mr1-release/init/property_service.cpp
static bool is_legal_property_name(const char* name, size_t namelen)
{
    size_t i;
    if (namelen >= PROP_NAME_MAX) return false;
    if (namelen < 1) return false;
    if (name[0] == '.') return false;
    if (name[namelen - 1] == '.') return false;

    /* Only allow alphanumeric, plus '.', '-', or '_' */
    /* Don't allow ".." to appear in a property name */
    for (i = 0; i < namelen; i++) {
        if (name[i] == '.') {
            // i=0 is guaranteed to never have a dot. See above.
            if (name[i-1] == '.') return false;
            continue;
        }
        if (name[i] == '_' || name[i] == '-') continue;
        if (name[i] >= 'a' && name[i] <= 'z') continue;
        if (name[i] >= 'A' && name[i] <= 'Z') continue;
        if (name[i] >= '0' && name[i] <= '9') continue;
        return false;
    }

    return true;
}

int property_set_impl(const char *name, const char *value)
{
    int rc;
    size_t namelen = strlen(name);
    size_t valuelen = strlen(value);

    if (!is_legal_property_name(name, namelen)) return -1;
    if (valuelen >= PROP_VALUE_MAX) return -1;

    prop_info* pi = (prop_info*) __system_property_find(name);
    if (trigger_prop) {
        if (pi != 0) {
            rc = __system_property_update(pi, value, valuelen);
        } else {
            rc = __system_property_add(name, namelen, value, valuelen);
        }
    } else {
        if (pi != 0) {
            if (!strncmp(name, "ro.", 3)) __system_property_del(name);
        }
        rc = __system_property_set(name, value);
    }

    if (rc != 0) {
        fprintf(stderr, "property_set(\"%s\", \"%s\") failed\n", name, value);
    }

    return rc;
}

static void load_properties(char *data, const char *filter)
{
    char *key, *value, *eol, *sol, *tmp, *fn;
    size_t flen = 0;

    if (filter) {
        flen = strlen(filter);
    }

    sol = data;
    while ((eol = strchr(sol, '\n'))) {
        key = sol;
        *eol++ = 0;
        sol = eol;

        while (isspace(*key)) key++;
        if (*key == '#') continue;

        tmp = eol - 2;
        while ((tmp > key) && isspace(*tmp)) *tmp-- = 0;

        if (!strncmp(key, "import ", 7) && flen == 0) {
            fn = key + 7;
            while (isspace(*fn)) fn++;

            key = strchr(fn, ' ');
            if (key) {
                *key++ = 0;
                while (isspace(*key)) key++;
            }

            load_properties_from_file(fn, key);
        } else {
            value = strchr(key, '=');
            if (!value) continue;
            *value++ = 0;

            tmp = value - 2;
            while ((tmp > key) && isspace(*tmp)) *tmp-- = 0;

            while (isspace(*value)) value++;

            if (flen > 0) {
                if (filter[flen - 1] == '*') {
                    if (strncmp(key, filter, flen - 1)) continue;
                } else {
                    if (strcmp(key, filter)) continue;
                }
            }

            if (!property_set_impl(key, value)) {
                printf("Set %s=%s\n", key, value);
            }
        }
    }
}

bool ReadFdToString(int fd, std::string* content) {
    content->clear();
    char buf[BUFSIZ];
    ssize_t n;
    while ((n = TEMP_FAILURE_RETRY(read(fd, &buf[0], sizeof(buf)))) > 0) {
        content->append(buf, n);
    }
    return (n == 0) ? true : false;
}

bool read_file(const char* path, std::string* content) {
    content->clear();

    int fd = TEMP_FAILURE_RETRY(open(path, O_RDONLY|O_NOFOLLOW|O_CLOEXEC));
    if (fd == -1) {
        return false;
    }

    bool okay = ReadFdToString(fd, content);
    close(fd);
    return okay;
}

static void load_properties_from_file(const char* filename, const char* filter) {
    std::string data;
    if (!read_file(filename, &data)) {
        fprintf(stderr, "Error reading %s\n", filename);
        return;
    }
    data.push_back('\n');
    load_properties(&data[0], filter);
}

bool is_delprops(char* name) {
    int full_length = strlen(name);
    char* start = strrchr(name, '/');
    if (start == nullptr) {
        start = name;
    } else {
        start++;
    }

    if ((start + strlen("delprops") == name + full_length) && !strcmp(start, "delprops")) {
        return true;
    }

    return false;
}

int usage(char* name) {
    if (is_delprops(name)) {
        printf("Usage:\n");
        printf("  %s name_1 name_2 ...\n", name);
        printf("     Remove properties name_1 name_2 .. with spaces separated\n\n");
    } else {
        printf("Usage:\n");
        printf("  %s --import filename\n", name);
        printf("     Read properties from file: [filename]\n\n");
        printf("  %s name value\n", name);
        printf("     Set property [name] to [value]\n\n");
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int import_prop = 0, args = 2;
    char *name, *value;
    int rc = 0;

    if (__system_properties_init()) {
        fprintf(stderr, "Error initializing system properties..\n");
        return 1;
    }

    if (is_delprops(argv[0])) {
        if (argc < 2) {
            return usage(argv[0]);
        }

        for (int i = 1; i < argc; i++) {
            name = argv[i];
            if (!is_legal_property_name(name, strlen(name))) {
                fprintf(stderr, "Illegal property name \'%s\'\n", name);
                rc = 1;
                continue;
            }

            if (__system_property_del(name)) {
                fprintf(stderr, "Error deleting property \'%s\'\n", name);
                rc = 1;
                continue;
            }
        }
    } else {
        if (argc < 3) {
            return usage(argv[0]);
        }

        for (int i = 1; i < argc; ++i) {
            if (!strcmp("-t", argv[i])) {
                trigger_prop = 1;
            } else if (!strcmp("--import", argv[i])) {
                import_prop = 1;
                args = 1;
            } else {
                if (i + args < argc) {
                    return usage(argv[0]);
                }

                if (import_prop) {
                    name = argv[i];
                } else {
                    name = argv[i];
                    value = argv[i + 1];
                    i++;
                }
            }
        }

        if (import_prop) {
            load_properties_from_file(name, nullptr);
        } else {
            if (property_set_impl(name, value)) {
                return 1;
            }
        }
    }

    return rc;
}
