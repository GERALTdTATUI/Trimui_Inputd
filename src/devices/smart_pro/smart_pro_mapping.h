#pragma once

#include <stdbool.h>
#include <linux/input-event-codes.h>

/* Optional per-physical-control overrides loaded at Smart Pro startup. */
int smart_pro_mapping_load(void);
unsigned short smart_pro_mapping_button_code(const char *button, unsigned short default_code);
unsigned short smart_pro_mapping_trigger_axis_code(const char *trigger, unsigned short default_code);
