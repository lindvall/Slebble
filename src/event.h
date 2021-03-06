#pragma once

#include "pebble.h"

extern void event_next_batch();
extern void send_appmessage(int index);
extern void event_set_click_data(char* data);
extern void event_set_view_update(void (*update)(int, char*, int, char*, char*, int, char*));
extern void event_tick_handler(int size, void *data);
extern void in_received_handler(DictionaryIterator *iter, void *context);
extern void in_dropped_handler(AppMessageResult reason, void *context);