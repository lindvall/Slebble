#include "menu.h"

uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
    return 1;
}

uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    Menu* menu = data;
    return menu->size;
}

int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    // This is a define provided in pebble.h that you may use for the default height
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    Menu* menu = data;
    menu_cell_basic_header_draw(ctx, cell_layer, menu->title);
}

void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    Menu* menu = data;
    menu_cell_basic_draw(ctx, cell_layer, &(menu->row_title)[cell_index->row], &(menu->row_subtitle)[cell_index->row], NULL);
}

void startmenu_window_load(Window *window) {
    Menu* menu = window_get_user_data(window);

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    menu->layer = menu_layer_create(bounds);

    menu_layer_set_callbacks(menu->layer, menu, (MenuLayerCallbacks){
            .get_num_sections = menu_get_num_sections_callback,
            .get_num_rows = menu_get_num_rows_callback,
            .get_header_height = menu_get_header_height_callback,
            .draw_header = menu_draw_header_callback,
            .draw_row = menu_draw_row_callback,
            .select_click = menu->callbacks.select_click,
    });

    menu->load_image = gbitmap_create_with_resource(menu->load_image_resource_id);
    menu->load_layer = bitmap_layer_create(bounds);
    bitmap_layer_set_background_color(menu->load_layer, GColorWhite);
    bitmap_layer_set_bitmap(menu->load_layer, menu->load_image);

    layer_add_child(window_layer, menu_layer_get_layer(menu->layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(menu->load_layer));

}

void startmenu_window_unload(Window *window) {
    Menu* menu = window_get_user_data(window);
    window_stack_remove(window, true);
    window_destroy(window);
    menu_layer_destroy(menu->layer);

    menu->callbacks.remove_callback(menu);
}

Menu* menu_create(uint32_t load_image_resource_id, MenuCallbacks callbacks) {
    Menu* menu = malloc(sizeof(Menu));
    menu->window = window_create();
    menu->load_image_resource_id = load_image_resource_id;
    menu->callbacks = callbacks;

    window_set_user_data(menu->window, menu);

    window_set_window_handlers(menu->window, (WindowHandlers) {
            .load = startmenu_window_load,
            .unload = startmenu_window_unload,
    });

    window_stack_push(menu->window, true);

    return menu;
}