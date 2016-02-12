#include <pebble.h>

typedef enum {
    PunchNow,
    EditTime,
} MenuType;


typedef struct {
    MenuType type;
} Context;

Window *window;
static TextLayer *s_time_layer;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;

//ActionFunc
static void punch_now() {

}

static void edit_time() {

}

//ActionMenu
static void action_performed_callback(ActionMenu *action_menu,
                                      const ActionMenuItem *action, void *context) {
    // An action that was created with this callback assigned was chosen
    MenuType type = (MenuType) action_menu_item_get_action_data(action);
    switch (type) {
        case PunchNow:
            punch_now();
            APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Selected %d", 0);
            break;
        case EditTime:
            edit_time();
            APP_LOG(APP_LOG_LEVEL_DEBUG, "EditTime Selected %d", 1);
            break;
    }
}

static void init_action_menu() {
    s_root_level = action_menu_level_create(2);
    action_menu_level_add_action(s_root_level, "Punch Now", action_performed_callback, (void *) PunchNow);
    action_menu_level_add_action(s_root_level, "Edit Time", action_performed_callback, (void *) EditTime);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    // Configure the ActionMenu Window about to be shown
    ActionMenuConfig config = (ActionMenuConfig) {
            .root_level = s_root_level,
            .colors = {
                    .background = PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite),
                    .foreground = GColorBlack,
            },
            .align = ActionMenuAlignCenter
    };

    // Show the ActionMenu
    s_action_menu = action_menu_open(&config);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

//Time
void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[12];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                         "%H:%M:%S" : "%I:%M:%S", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}


static void main_window_load(Window *window) {
    GRect bounds = layer_get_bounds(window_get_root_layer(window));
    window_set_click_config_provider(window, click_config_provider);
    s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

    //TimeLayer
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Add  child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
}

void init() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload});
    window_stack_push(window, true);
    init_action_menu();
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void deinit() {
    window_destroy(window);
}

int main() {
    init();
    app_event_loop();
    deinit();
    return 0;
}