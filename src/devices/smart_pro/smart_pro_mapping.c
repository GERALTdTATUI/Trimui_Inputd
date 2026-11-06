#define _DEFAULT_SOURCE
#include "smart_pro_mapping.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SP_MAPPING_MAX_OVERRIDES 16

struct button_override {
    char name[16];
    unsigned short code;
};

static struct button_override button_overrides[SP_MAPPING_MAX_OVERRIDES];
static size_t button_override_count;
static unsigned short l2_axis_code = ABS_Z;
static unsigned short r2_axis_code = ABS_RZ;

struct key_name {
    const char *name;
    unsigned short code;
};

static const struct key_name key_names[] = {
    {"BTN_EAST", BTN_SOUTH}, {"BTN_SOUTH", BTN_EAST},
    {"BTN_NORTH", BTN_NORTH}, {"BTN_WEST", BTN_WEST},
    {"BTN_TL", BTN_TL}, {"BTN_TR", BTN_TR},
    {"BTN_TL2", BTN_TL2}, {"BTN_TR2", BTN_TR2},
    {"BTN_SELECT", BTN_SELECT}, {"BTN_START", BTN_START},
    {"BTN_MODE", BTN_MODE}, {"BTN_THUMBL", BTN_THUMBL},
    {"BTN_THUMBR", BTN_THUMBR},
};

static char *trim(char *s)
{
    while (isspace((unsigned char)*s)) ++s;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) --end;
    *end = '\0';
    return s;
}

static bool valid_button_name(const char *name)
{
    static const char *const names[] = {
        "A", "B", "X", "Y", "L1", "R1", "SELECT", "START", "MODE"
    };
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(name, names[i]) == 0) return true;
    return false;
}

static int parse_key_code(const char *name, unsigned short *code)
{
    for (size_t i = 0; i < sizeof(key_names) / sizeof(key_names[0]); ++i) {
        if (strcmp(name, key_names[i].name) == 0) {
            *code = key_names[i].code;
            return 0;
        }
    }
    return -1;
}

static int parse_axis_code(const char *name, unsigned short *code)
{
    if (strcmp(name, "ABS_Z") == 0) *code = ABS_Z;
    else if (strcmp(name, "ABS_RZ") == 0) *code = ABS_RZ;
    else return -1;
    return 0;
}

int smart_pro_mapping_load(void)
{
    const char *path = getenv("TRIMUI_INPUTD_CONFIG");
    if (!path || !*path) path = "/mnt/UDISK/trimui_inputd.conf";

    button_override_count = 0;
    l2_axis_code = ABS_Z;
    r2_axis_code = ABS_RZ;

    FILE *file = fopen(path, "r");
    if (!file) {
        if (errno == ENOENT) return 0;
        fprintf(stderr, "Smart Pro mapping: cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }

    char *line = NULL;
    size_t capacity = 0;
    unsigned int line_number = 0;
    int result = 0;
    while (getline(&line, &capacity, file) >= 0) {
        ++line_number;
        char *entry = trim(line);
        if (!*entry || *entry == '#' || *entry == ';') continue;
        char *equals = strchr(entry, '=');
        if (!equals) {
            fprintf(stderr, "%s:%u: expected CONTROL=CODE\n", path, line_number);
            result = -1;
            continue;
        }
        *equals++ = '\0';
        char *control = trim(entry);
        char *value = trim(equals);

        if (strcmp(control, "L2") == 0 || strcmp(control, "R2") == 0) {
            unsigned short axis_code;
            if (parse_axis_code(value, &axis_code) < 0) {
                fprintf(stderr, "%s:%u: %s accepts ABS_Z or ABS_RZ only\n", path, line_number, control);
                result = -1;
            } else if (control[1] == '2' && control[0] == 'L') {
                l2_axis_code = axis_code;
            } else {
                r2_axis_code = axis_code;
            }
            continue;
        }

        unsigned short key_code;
        if (!valid_button_name(control) || parse_key_code(value, &key_code) < 0) {
            fprintf(stderr, "%s:%u: unknown control or key code: %s=%s\n", path, line_number, control, value);
            result = -1;
            continue;
        }

        size_t slot = button_override_count;
        for (size_t i = 0; i < button_override_count; ++i) {
            if (strcmp(button_overrides[i].name, control) == 0) { slot = i; break; }
        }
        if (slot == button_override_count) {
            if (button_override_count >= SP_MAPPING_MAX_OVERRIDES) {
                fprintf(stderr, "%s:%u: too many mapping entries\n", path, line_number);
                result = -1;
                continue;
            }
            ++button_override_count;
        }
        snprintf(button_overrides[slot].name, sizeof(button_overrides[slot].name), "%s", control);
        button_overrides[slot].code = key_code;
    }

    free(line);
    if (ferror(file)) result = -1;
    fclose(file);
    return result;
}

unsigned short smart_pro_mapping_button_code(const char *button, unsigned short default_code)
{
    for (size_t i = 0; i < button_override_count; ++i)
        if (strcmp(button_overrides[i].name, button) == 0) return button_overrides[i].code;
    return default_code;
}

unsigned short smart_pro_mapping_trigger_axis_code(const char *trigger, unsigned short default_code)
{
    if (strcmp(trigger, "L2") == 0) return l2_axis_code;
    if (strcmp(trigger, "R2") == 0) return r2_axis_code;
    return default_code;
}
