#include <pebble.h>

uint32_t clock_key = 0;
uint32_t DATE_KEY = 0;
uint32_t CLOCK_KEY_1 = 1;
uint32_t CLOCK_KEY_2 = 2;
uint32_t CLOCK_KEY_3 = 3;
uint32_t CLOCK_KEY_4 = 4;

typedef enum {
    PunchNow,
    EditTime,
    PushTime,
    DeleteTime1,
    DeleteTime2,
    DeleteTime3,
    DeleteTime4,
} MenuType;

typedef struct {
    MenuType type;
} Context;

Window *window;
static TextLayer *s_time_layer, *punch_time_layer_1, *punch_time_layer_2, *punch_time_layer_3, *punch_time_layer_4;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level, *s_delete_level;

static int key;

static char clock_string_1[6], clock_string_2[6], clock_string_3[6], clock_string_4[6], data_string[11];

static void send() {
    DictionaryIterator *iter;
    AppMessageResult result = app_message_outbox_begin(&iter);
    if(result == APP_MSG_OK) {
        // Construct the message
        dict_write_cstring(iter, DATE_KEY, data_string);
        if (strcmp(clock_string_1, "")) {
            dict_write_cstring(iter, CLOCK_KEY_1, clock_string_1);
        }
        if (strcmp(clock_string_2, "")) {
            dict_write_cstring(iter, CLOCK_KEY_2, clock_string_2);
        }
        if (strcmp(clock_string_3, "")) {
            dict_write_cstring(iter, CLOCK_KEY_3, clock_string_3);
        }
        if (strcmp(clock_string_4, "")) {
            dict_write_cstring(iter, CLOCK_KEY_4, clock_string_4);
        }
        app_message_outbox_send();
    } else {
        // The outbox cannot be used right now
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int) result);
    }
}

void get_date() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    strftime(data_string, sizeof(data_string), "%a %d %b", tick_time);
}

//ActionMenu
static void action_performed_callback(ActionMenu *action_menu,
                                      const ActionMenuItem *action, void *context) {
    // An action that was created with this callback assigned was chosen
    MenuType type = (MenuType) action_menu_item_get_action_data(action);
    switch (type) {
        case PunchNow:
            key++;
            if (key == 1) {
                clock_copy_time_string(clock_string_1, 6);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Layer %d", 1);
                persist_write_string(CLOCK_KEY_1, clock_string_1);
                text_layer_set_text(punch_time_layer_1, clock_string_1);
                get_date();
            } else if (key == 2) {
                clock_copy_time_string(clock_string_2, 6);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Layer %d", 2);
                persist_write_string(CLOCK_KEY_2, clock_string_2);
                text_layer_set_text(punch_time_layer_2, clock_string_2);
            } else if (key == 3) {
                clock_copy_time_string(clock_string_3, 6);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Layer %d", 3);
                persist_write_string(CLOCK_KEY_3, clock_string_3);
                text_layer_set_text(punch_time_layer_3, clock_string_3);
            } else if (key == 4) {
                clock_copy_time_string(clock_string_4, 6);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Layer %d", 4);
                persist_write_string(CLOCK_KEY_4, clock_string_4);
                text_layer_set_text(punch_time_layer_4, clock_string_4);
                key = 0;
            }
//            APP_LOG(APP_LOG_LEVEL_DEBUG, "PunchTime Selected %d", 0);
            break;
        case EditTime:
            //TODO using the pebble number window https://developer.pebble.com/docs/c/User_Interface/Window/NumberWindow/
            APP_LOG(APP_LOG_LEVEL_DEBUG, "EditTime Selected %d", 1);
            break;
        case PushTime:
            APP_LOG(APP_LOG_LEVEL_DEBUG, "PushTime %d", 1);
            send();
            break;
        case DeleteTime1:
            //TODO Conform before deleting
            //https://github.com/pebble-examples/ui-patterns/blob/master/src/windows/dialog_choice_window.c
            persist_delete(CLOCK_KEY_1);
            text_layer_set_text(punch_time_layer_1, "");
            break;
        case DeleteTime2:
            persist_delete(CLOCK_KEY_2);
            text_layer_set_text(punch_time_layer_2, "");
            break;
        case DeleteTime3:
            persist_delete(CLOCK_KEY_3);
            text_layer_set_text(punch_time_layer_3, "");
            break;
        case DeleteTime4:
            persist_delete(CLOCK_KEY_4);
            text_layer_set_text(punch_time_layer_4, "");
            break;
    }
}

static void init_action_menu() {
    s_root_level = action_menu_level_create(4);
    action_menu_level_add_action(s_root_level, "Punch Now", action_performed_callback, (void *) PunchNow);
    action_menu_level_add_action(s_root_level, "Edit Time", action_performed_callback, (void *) EditTime);
    action_menu_level_add_action(s_root_level, "Push Time", action_performed_callback, (void *) PushTime);

    s_delete_level = action_menu_level_create(4);
    action_menu_level_add_child(s_root_level, s_delete_level, "Delete Time");
    action_menu_level_add_action(s_delete_level, "Time 1", action_performed_callback, (void *) DeleteTime1);
    action_menu_level_add_action(s_delete_level, "Time 2", action_performed_callback, (void *) DeleteTime2);
    action_menu_level_add_action(s_delete_level, "Time 3", action_performed_callback, (void *) DeleteTime3);
    action_menu_level_add_action(s_delete_level, "Time 4", action_performed_callback, (void *) DeleteTime4);
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

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message received! %d", 0);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! %d", 0);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! %d", 0);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success! %d", 0);
    APP_LOG(APP_LOG_LEVEL_INFO, "Now delete all persists %d", 1);
    persist_delete(CLOCK_KEY_1);
    persist_delete(CLOCK_KEY_2);
    persist_delete(CLOCK_KEY_3);
    persist_delete(CLOCK_KEY_4);
//    text_layer_set_text(punch_time_layer_1, "");
//    text_layer_set_text(punch_time_layer_2, "");
//    text_layer_set_text(punch_time_layer_3, "");
//    text_layer_set_text(punch_time_layer_4, "");
}


void init() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload});
    window_stack_push(window, true);
    init_action_menu();
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    const int inbox_size = 128;
    const int outbox_size = 128;
    app_message_open(inbox_size, outbox_size);

    if (persist_exists(CLOCK_KEY_1)) {
        int x = persist_read_string(CLOCK_KEY_1, clock_string_1, sizeof(clock_string_1));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist 1 %d", x);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", clock_string_1);
        text_layer_set_text(punch_time_layer_1, clock_string_1);
    }
    if (persist_exists(CLOCK_KEY_2)) {
        int x = persist_read_string(CLOCK_KEY_2, clock_string_2, sizeof(clock_string_2));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist 2 %d", x);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", clock_string_2);
        text_layer_set_text(punch_time_layer_2, clock_string_2);
    }
    if (persist_exists(CLOCK_KEY_3)) {
        int x = persist_read_string(CLOCK_KEY_3, clock_string_3, sizeof(clock_string_3));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist 3 %d", x);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", clock_string_3);
        text_layer_set_text(punch_time_layer_3, clock_string_3);
    }
    if (persist_exists(CLOCK_KEY_4)) {
        int x = persist_read_string(CLOCK_KEY_4, clock_string_4, sizeof(clock_string_4));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist 4 %d", x);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", clock_string_4);
        text_layer_set_text(punch_time_layer_4, clock_string_4);
    }
    if (persist_exists(clock_key)) {
        key = persist_read_int(clock_key);
    } else {
        key = 0;
    }
}

void deinit() {
    if (strcmp(clock_string_1, "")) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist deinit %d", 1);
        persist_write_string(CLOCK_KEY_1, clock_string_1);
    }
    if (strcmp(clock_string_2, "")) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist deinit %d", 2);
        persist_write_string(CLOCK_KEY_2, clock_string_2);
    }
    if (strcmp(clock_string_3, "")) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist deinit %d", 3);
        persist_write_string(CLOCK_KEY_3, clock_string_3);
    }
    if (strcmp(clock_string_4, "")) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside Presist deinit %d", 4);
        persist_write_string(CLOCK_KEY_4, clock_string_4);
    }
    persist_write_int(clock_key, key);
    window_destroy(window);
}

int main() {
    init();
    app_event_loop();
    deinit();
    return 0;
}