#include <pebble.h>

#include "cellular_automata.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_wolfram_layer;

static const int num_rules = 14;
static const int interesting_rules[14] = {
     18,  22,  26,  30,  60,
     82,  86,  89,  90, 102,
    110, 122, 124, 126
};
static uint8_t rule = 0;


static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[18];
    strftime(s_buffer,
             sizeof(s_buffer),
             clock_is_24h_style() ? "%H:%M, %d.%m.%Y" : "%I:%M, %d.%m.%Y",
             tick_time
    );

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void wolfram_handler(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(s_wolfram_layer);
}

static void wolfram_rule_update(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    int pixel_size = 4;

    // w and h should both be 144
    int width = bounds.size.w / pixel_size;
    int height = bounds.size.h / pixel_size;

    // allocate "out" once
    static int b = 1;
    static uint8_t *out;
    if(b) {
        out = (uint8_t *) malloc(width*height * sizeof(uint8_t));
        b = 0;
    }

    // with 1/3 change start with random seed instead of one seed in the center
    int random = !(rand()%3);
    // fill the buffer out with a bitmap of the cellular automaton
    ca_update_rolling_buffer(rule, width, height, out, random);

    // display the buffer on the screen
    // draw the background
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // draw the cells
    graphics_context_set_fill_color(ctx, GColorWhite);
    for(int j=0; j<height; ++j)
        for(int i=0; i<width; ++i)
            if(out[j*height + i])
            {
                GRect pixel = GRect(i*pixel_size, j*pixel_size,
                                    pixel_size, pixel_size);
                graphics_fill_rect(ctx, pixel, 0, GCornerNone);
            }
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create the TextLayer with specific bounds
    s_time_layer = text_layer_create(
        GRect(0, 0, bounds.size.w, 24));

    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Create Wolfram rule Layer
    s_wolfram_layer = layer_create(GRect(0, 24, 144, 144));
    // choose a new random rule at every load
    rule = interesting_rules[rand()%num_rules];
    layer_set_update_proc(s_wolfram_layer, wolfram_rule_update);

    // Add to Window
    layer_add_child(window_get_root_layer(window), s_wolfram_layer);
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    layer_destroy(s_wolfram_layer);
}

static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);

    // Make sure the time is displayed from the start
    update_time();

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    tick_timer_service_subscribe(SECOND_UNIT, wolfram_handler);
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
