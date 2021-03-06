#include "display.h"
#include "parser.h"
#include <Keypad.h>

RA8875 tft = RA8875(RA8875_CS, NULL);//Teensy

static const byte ROWS = 2; //2 rows
static const byte COLS = 4; //4 columns

static byte rowPins[ROWS] = {4, 3};
static byte colPins[COLS] = {8, 7, 6, 5};

/*
 keymap
 1 = prev component
 2 = none
 3 = up move
 4 =  enter
 5 = nex component
 6 = left move
 7 = down move
 8 = right move
*/
static char keys[ROWS][COLS] = {
        {'1', '2', '3', '4'},
        {'5', '6', '7', '8'}
};

static lv_disp_buf_t disp_buf;
static lv_color_t buf1[LV_HOR_RES_MAX * 80];
static lv_color_t buf2[LV_HOR_RES_MAX * 80];


lv_obj_t *console;
static lv_obj_t *line_input;
static lv_indev_t * my_indev;
static lv_group_t * g;
static Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint16_t width = (area->x2 - area->x1 + 1);
    uint16_t height = (area->y2 - area->y1 + 1);

    tft.writeRect(area->x1, area->y1, width, height, (uint16_t *) color_p);

    lv_disp_flush_ready(disp);

}

static bool my_button_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    char key = keypad.getKey();

    if (key != NO_KEY){
        switch (key) {
            case '8':
                data->key = LV_KEY_RIGHT;
                break;
            case '6':
                data->key = LV_KEY_LEFT;
                break;
            case '3':
                data->key = LV_KEY_UP;
                break;
            case '7':
                data->key = LV_KEY_DOWN;
                break;
            case '4':
                data->key = LV_KEY_ENTER;
                break;
            case '5':
                data->key = LV_KEY_NEXT;
                break;
            case '1':
                data->key = LV_KEY_PREV;
                break;
        }
        data->state = LV_INDEV_STATE_PR;
    }else{
        data->state = LV_INDEV_STATE_REL;
    }
    return false;
}

static void kb_event_cb(lv_obj_t *event_kb, lv_event_t event) {
    lv_kb_def_event_cb(event_kb, event);
    if (event == LV_EVENT_APPLY) {
        lv_ta_add_text(console, lv_ta_get_text(line_input));
        lv_ta_add_text(console, "\n");
        parserParse(lv_ta_get_text(line_input));
        lv_ta_add_text(console, "\n>");
        lv_ta_set_text(line_input, "");
    }
}

void display_hw_init(void){
    tft.begin(RA8875_800x480);
}

void display_gl_init(void){
    lv_init();

    g = lv_group_create();

    lv_disp_buf_init(&disp_buf, buf1, buf2, LV_HOR_RES_MAX * 80);

    lv_theme_t *th = lv_theme_alien_init(210, NULL);
    lv_theme_set_current(th);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);
}

void display_gl_keypad_init(void){
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);      /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = my_button_driver_read;
    my_indev = lv_indev_drv_register(&indev_drv);
    lv_indev_set_group(my_indev, g);
};

void display_ui_draw(void){


    console = lv_ta_create(lv_scr_act(), NULL);
    lv_obj_set_size(console, 800, 205);
    lv_obj_align(console, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_ta_set_cursor_type(console, LV_CURSOR_BLOCK);
    lv_ta_set_text(console, "Ponyxis kernel\n>");    /*Set an initial text*/
    lv_group_add_obj(g, console);

    line_input = lv_ta_create(lv_scr_act(), NULL);
    lv_obj_set_size(line_input, 800, 32);
    lv_obj_align(line_input, NULL, LV_ALIGN_CENTER, 0, -16);
    lv_ta_set_cursor_type(line_input, LV_CURSOR_BLOCK);
    lv_ta_set_text(line_input, "");    /*Set an initial text*/
    lv_group_add_obj(g, line_input);

    /*Create styles for the keyboard*/
    static lv_style_t rel_style, pr_style;

    lv_style_copy(&rel_style, &lv_style_btn_rel);
    rel_style.body.radius = 0;
    rel_style.body.border.width = 1;

    lv_style_copy(&pr_style, &lv_style_btn_pr);
    pr_style.body.radius = 0;
    pr_style.body.border.width = 1;

    /*Create a keyboard and apply the styles*/
    lv_obj_t *kb = lv_kb_create(lv_scr_act(), NULL);
    lv_kb_set_cursor_manage(kb, true);
    lv_kb_set_style(kb, LV_KB_STYLE_BG, &lv_style_transp_tight);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &rel_style);
    lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &pr_style);
    lv_obj_set_event_cb(kb, kb_event_cb);
    lv_kb_set_ta(kb, line_input);
    lv_group_add_obj(g, kb);

}

void display_gl_task(void){
    lv_task_handler();
}