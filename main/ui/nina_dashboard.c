#include "nina_dashboard.h"
#include <stdio.h>

static lv_obj_t *scr_main;
static lv_obj_t *view_summary;
static lv_obj_t *view_detail_1;
static lv_obj_t *view_detail_2;

static int current_view = 0; // 0: Summary, 1: Detail 1, 2: Detail 2

// UI Objects for Instance 1 (Summary)
static lv_obj_t *lbl_sum_1_name;
static lv_obj_t *lbl_sum_1_status;
static lv_obj_t *bar_sum_1_progress;

// UI Objects for Instance 2 (Summary)
static lv_obj_t *lbl_sum_2_name;
static lv_obj_t *lbl_sum_2_status;
static lv_obj_t *bar_sum_2_progress;

// UI Objects for Instance 1 (Detail)
static lv_obj_t *lbl_det_1_title;
static lv_obj_t *lbl_det_1_target;
static lv_obj_t *lbl_det_1_camera;
static lv_obj_t *lbl_det_1_mount;
static lv_obj_t *lbl_det_1_guider;
static lv_obj_t *bar_det_1_progress;

// UI Objects for Instance 2 (Detail)
static lv_obj_t *lbl_det_2_title;
static lv_obj_t *lbl_det_2_target;
static lv_obj_t *lbl_det_2_camera;
static lv_obj_t *lbl_det_2_mount;
static lv_obj_t *lbl_det_2_guider;
static lv_obj_t *bar_det_2_progress;

static lv_style_t style_dark;
static lv_style_t style_title;
static lv_style_t style_text;
static lv_style_t style_bar_bg;
static lv_style_t style_bar_indic;

static void create_styles(void) {
    lv_style_init(&style_dark);
    lv_style_set_bg_color(&style_dark, lv_color_hex(0x121212)); // Dark gray
    lv_style_set_text_color(&style_dark, lv_color_hex(0xE0E0E0));

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_20);
    lv_style_set_text_color(&style_title, lv_color_hex(0xFFFFFF));

    lv_style_init(&style_text);
    lv_style_set_text_font(&style_text, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_text, lv_color_hex(0xB0B0B0));

    lv_style_init(&style_bar_bg);
    lv_style_set_bg_color(&style_bar_bg, lv_color_hex(0x303030));
    lv_style_set_radius(&style_bar_bg, 4);

    lv_style_init(&style_bar_indic);
    lv_style_set_bg_color(&style_bar_indic, lv_color_hex(0x2196F3)); // Blue
    lv_style_set_radius(&style_bar_indic, 4);
}

static lv_obj_t* create_summary_card(lv_obj_t *parent, const char *title, lv_obj_t **lbl_status, lv_obj_t **bar_progress) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, lv_pct(90), 120);
    lv_obj_add_style(card, &style_dark, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x333333), 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *lbl_title = lv_label_create(card);
    lv_label_set_text(lbl_title, title);
    lv_obj_add_style(lbl_title, &style_title, 0);

    *lbl_status = lv_label_create(card);
    lv_label_set_text(*lbl_status, "Status: Disconnected");
    lv_obj_add_style(*lbl_status, &style_text, 0);

    *bar_progress = lv_bar_create(card);
    lv_obj_set_size(*bar_progress, lv_pct(100), 10);
    lv_obj_add_style(*bar_progress, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(*bar_progress, &style_bar_indic, LV_PART_INDICATOR);
    lv_bar_set_value(*bar_progress, 0, LV_ANIM_OFF);

    return card;
}

static void create_detail_view(lv_obj_t *parent, const char *title, 
                               lv_obj_t **lbl_target, lv_obj_t **lbl_camera, 
                               lv_obj_t **lbl_mount, lv_obj_t **lbl_guider,
                               lv_obj_t **bar_progress) {
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x000000), 0); // Black background

    lv_obj_t *lbl_t = lv_label_create(parent);
    lv_label_set_text(lbl_t, title);
    lv_obj_add_style(lbl_t, &style_title, 0);
    lv_obj_set_style_text_font(lbl_t, &lv_font_montserrat_24, 0);
    lv_obj_set_style_pad_bottom(lbl_t, 20, 0);

    *lbl_target = lv_label_create(parent);
    lv_label_set_text(*lbl_target, "Target: --");
    lv_obj_add_style(*lbl_target, &style_text, 0);

    *lbl_camera = lv_label_create(parent);
    lv_label_set_text(*lbl_camera, "Camera: --");
    lv_obj_add_style(*lbl_camera, &style_text, 0);

    *lbl_mount = lv_label_create(parent);
    lv_label_set_text(*lbl_mount, "Mount: --");
    lv_obj_add_style(*lbl_mount, &style_text, 0);

    *lbl_guider = lv_label_create(parent);
    lv_label_set_text(*lbl_guider, "Guider: --");
    lv_obj_add_style(*lbl_guider, &style_text, 0);

    *bar_progress = lv_bar_create(parent);
    lv_obj_set_size(*bar_progress, lv_pct(100), 20);
    lv_obj_set_style_margin_top(*bar_progress, 20, 0);
    lv_obj_add_style(*bar_progress, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(*bar_progress, &style_bar_indic, LV_PART_INDICATOR);
}

void nina_dashboard_init(void) {
    create_styles();

    scr_main = lv_obj_create(NULL);
    lv_obj_add_style(scr_main, &style_dark, 0);

    // Summary View
    view_summary = lv_obj_create(scr_main);
    lv_obj_set_size(view_summary, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(view_summary, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(view_summary, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(view_summary, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(view_summary, 0, 0);

    create_summary_card(view_summary, "NINA Instance 1", &lbl_sum_1_status, &bar_sum_1_progress);
    lv_obj_t *spacer = lv_obj_create(view_summary); // Spacer
    lv_obj_set_size(spacer, 10, 20);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
    create_summary_card(view_summary, "NINA Instance 2", &lbl_sum_2_status, &bar_sum_2_progress);

    // Detail View 1
    view_detail_1 = lv_obj_create(scr_main);
    lv_obj_set_size(view_detail_1, lv_pct(100), lv_pct(100));
    create_detail_view(view_detail_1, "Instance 1", &lbl_det_1_target, &lbl_det_1_camera, &lbl_det_1_mount, &lbl_det_1_guider, &bar_det_1_progress);
    lv_obj_add_flag(view_detail_1, LV_OBJ_FLAG_HIDDEN);

    // Detail View 2
    view_detail_2 = lv_obj_create(scr_main);
    lv_obj_set_size(view_detail_2, lv_pct(100), lv_pct(100));
    create_detail_view(view_detail_2, "Instance 2", &lbl_det_2_target, &lbl_det_2_camera, &lbl_det_2_mount, &lbl_det_2_guider, &bar_det_2_progress);
    lv_obj_add_flag(view_detail_2, LV_OBJ_FLAG_HIDDEN);

    lv_scr_load(scr_main);
}

void nina_dashboard_cycle_view(void) {
    current_view = (current_view + 1) % 3;

    if (current_view == 0) {
        lv_obj_clear_flag(view_summary, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view_detail_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view_detail_2, LV_OBJ_FLAG_HIDDEN);
    } else if (current_view == 1) {
        lv_obj_add_flag(view_summary, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(view_detail_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view_detail_2, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(view_summary, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(view_detail_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(view_detail_2, LV_OBJ_FLAG_HIDDEN);
    }
}

void nina_dashboard_update(int instance_index, const NinaData *data) {
    lv_obj_t *lbl_status_sum, *bar_progress_sum;
    lv_obj_t *lbl_target, *lbl_camera, *lbl_mount, *lbl_guider, *bar_progress_det;

    if (instance_index == 0) {
        lbl_status_sum = lbl_sum_1_status;
        bar_progress_sum = bar_sum_1_progress;
        lbl_target = lbl_det_1_target;
        lbl_camera = lbl_det_1_camera;
        lbl_mount = lbl_det_1_mount;
        lbl_guider = lbl_det_1_guider;
        bar_progress_det = bar_det_1_progress;
    } else {
        lbl_status_sum = lbl_sum_2_status;
        bar_progress_sum = bar_sum_2_progress;
        lbl_target = lbl_det_2_target;
        lbl_camera = lbl_det_2_camera;
        lbl_mount = lbl_det_2_mount;
        lbl_guider = lbl_det_2_guider;
        bar_progress_det = bar_det_2_progress;
    }

    if (!data->connected) {
        lv_label_set_text(lbl_status_sum, "Status: Disconnected");
        lv_bar_set_value(bar_progress_sum, 0, LV_ANIM_OFF);
        
        lv_label_set_text(lbl_target, "Target: --");
        lv_label_set_text(lbl_camera, "Camera: Disconnected");
        lv_label_set_text(lbl_mount, "Mount: Disconnected");
        lv_label_set_text(lbl_guider, "Guider: --");
        lv_bar_set_value(bar_progress_det, 0, LV_ANIM_OFF);
        return;
    }

    if (data->exposure_iterations > 0) {
        lv_label_set_text_fmt(lbl_status_sum, "Status: %s\nTarget: %s\nSeq: %d/%d", 
            data->camera_status, data->target_name, data->exposure_count, data->exposure_iterations);
        
        lv_label_set_text_fmt(lbl_target, "Target: %s\nSeq: %d/%d", 
            data->target_name, data->exposure_count, data->exposure_iterations);
            
        lv_label_set_text_fmt(lbl_camera, "Camera: %s\n%s %.0fs", 
            data->camera_status, data->exposure_type, data->exposure_time);
    } else {
        lv_label_set_text_fmt(lbl_status_sum, "Status: %s\nTarget: %s", data->camera_status, data->target_name);
        lv_label_set_text_fmt(lbl_target, "Target: %s", data->target_name);
        lv_label_set_text_fmt(lbl_camera, "Camera: %s", data->camera_status);
    }

    lv_bar_set_value(bar_progress_sum, (int32_t)(data->exposure_progress * 100), LV_ANIM_ON);

    lv_label_set_text_fmt(lbl_mount, "Mount: %s\nRA: %.2f Dec: %.2f", data->mount_status, data->ra, data->dec);
    lv_label_set_text_fmt(lbl_guider, "Guider RMS: %.2f", data->guider_error_rms);
    lv_bar_set_value(bar_progress_det, (int32_t)(data->exposure_progress * 100), LV_ANIM_ON);
}