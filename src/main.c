#include "pebble.h"
#include "menu.h"
#include "event.h"

Menu *menu;
int updates = 0;
bool first_tick = false;


void view_update(int size, char *title, int index, char *row_title, char *row_subtitle, int data_int, char *data_char) {
    menu_update(menu, size, title, index, row_title, row_subtitle, data_int, data_char);
    updates++;
    if(updates >= size) {
        updates = 0;
        menu_hide_load_image(menu);
        vibes_short_pulse();
    }
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    if(first_tick) {
        event_tick_handler(menu->size, menu->data_int);
        for(int i = 0; i < menu->size; i++) {
            char buf[32];
            if(((int*)menu->data_int)[i] > 0) {
                snprintf(buf, 32, "%dmin - %s", ((int*)menu->data_int)[i], ((char**)menu->data_char)[i]);
                buf[31] = '\0';
            } else {
                snprintf(buf, 32, "Nu - %s", ((char**)menu->data_char)[i]);
                buf[31] = '\0';
            }
            menu_update(menu, menu->size, menu->title, i, buf, menu->row_subtitle[i], ((int*)menu->data_int)[i], ((char**)menu->data_char)[i]);
        }
        menu_layer_reload_data(menu->layer);
    }
    first_tick = true;
}

void remove_callback_handler(void *data) {
    event_next_batch();
    Menu* temp = data;
    menu = temp;
    tick_timer_service_unsubscribe();
}

void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    char *click_data;
    int row_clicked = cell_index->row + 1;

    if (cell_index->section == 0) {
        click_data = "Nearby Station";
        row_clicked--;
    } else
        click_data = menu->row_title[cell_index->row];

    event_set_click_data(click_data);

    Menu *temp = menu;
    menu = menu_create(RESOURCE_ID_SLEBBLE_LOADING_BLACK, (MenuCallbacks) {
            .select_click = NULL,
            .remove_callback = &remove_callback_handler,
    });

    menu->menu = temp;

    updates = 0;
    if (app_comm_get_sniff_interval() == SNIFF_INTERVAL_NORMAL)
        app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    send_appmessage(row_clicked);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

int main(void) {

    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);

    menu = menu_create(RESOURCE_ID_SLEBBLE_START_BLACK, (MenuCallbacks){
            .select_click = &select_callback,
            .remove_callback = &remove_callback_handler,
    });

    menu->nearby = true;
    event_set_view_update(&view_update);

    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

    app_event_loop();
}
