#include <pebble.h>

typedef enum {
    PunchNow,
    EditTime,
} MenuType;


typedef struct {
    MenuType type;
} Context;

Window *window;
static TextLayer *s_time_layer, *punch_time_layer_1, *punch_time_layer_2, *punch_time_layer_3, *punch_time_layer_4;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;

static int key = 0;
static char clock_string[20];

//ActionMenu
static void action_performed_callback(ActionMenu *action_menu,
                                      const ActionMenuItem *action, void *context) {
    // An action that was created with this callback assigned was chosen
    MenuType type = (MenuType) action_menu_item_get_action_data(action);
    switch (type) {
        case PunchNow:
            key++;
            if (key == 1) {
                clock_copy_time_string(clock_string, 20);
                text_layer_set_text(punch_time_layer_1, clock_string);
            } else if (key == 2) {
                clock_copy_time_string(clock_string, 20);
                text_layer_set_text(punch_time_layer_2, clock_string);
            } else if (key == 3) {
                clock_copy_time_string(clock_string, 20);
                text_layer_set_text(punch_time_layer_3, clock_string);
            } else if (key == 4) {
                clock_copy_time_string(clock_string, 20);
                text_layer_set_text(punch_time_layer_4, clock_string);
            }
            APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Selected %d", 0);
            break;
        case EditTime:
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
    //TextLayer Create
    s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 22), bounds.size.w, 50));
    punch_time_layer_1 = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(108, 72), bounds.size.w, 14));
    punch_time_layer_2 = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(118, 86), bounds.size.w, 14));
    punch_time_layer_3 = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(128, 100), bounds.size.w, 14));
    punch_time_layer_4 = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(138, 114), bounds.size.w, 14));

    //TimeLayer
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    //PunchTimeLayer
    text_layer_set_text_color(punch_time_layer_1, GColorBlack);
    text_layer_set_text_color(punch_time_layer_2, GColorBlack);
    text_layer_set_text_color(punch_time_layer_3, GColorBlack);
    text_layer_set_text_color(punch_time_layer_4, GColorBlack);
    text_layer_set_text_alignment(punch_time_layer_1, GTextAlignmentCenter);
    text_layer_set_text_alignment(punch_time_layer_2, GTextAlignmentCenter);
    text_layer_set_text_alignment(punch_time_layer_3, GTextAlignmentCenter);
    text_layer_set_text_alignment(punch_time_layer_4, GTextAlignmentCenter);

    // Add  child layer to the Window's root layer
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(punch_time_layer_1));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(punch_time_layer_2));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(punch_time_layer_3));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(punch_time_layer_4));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(punch_time_layer_1);
    text_layer_destroy(punch_time_layer_2);
    text_layer_destroy(punch_time_layer_3);
    text_layer_destroy(punch_time_layer_4);
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