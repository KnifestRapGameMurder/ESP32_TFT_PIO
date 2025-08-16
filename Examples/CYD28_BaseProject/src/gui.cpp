#include "gui.h"
#include "wifi_ui.h"

// WiFi UI objects
static lv_obj_t *wifi_list;
static lv_obj_t *status_label;
static lv_obj_t *password_ta;
static lv_obj_t *connect_btn;
static lv_obj_t *scan_btn;
static lv_obj_t *keyboard;

static int selected_network = -1;
static lv_timer_t *status_timer;

// Event handlers
static void wifi_list_event_cb(lv_event_t * e);
static void connect_btn_event_cb(lv_event_t * e);
static void scan_btn_event_cb(lv_event_t * e);
static void password_ta_event_cb(lv_event_t * e);
static void status_timer_cb(lv_timer_t * timer);

static lv_obj_t *screenMain;

uint8_t userData;

// ---------------- main tabview ----------------------------------------
lv_obj_t * tabview;
lv_obj_t * tabAudio;
lv_obj_t * tabSD;
lv_obj_t * tabLDR;
lv_obj_t * tabRGB;
lv_obj_t * tabSYS;

// ------------------ TAB AUDIO -----------------------------------------	
const char *btnLabels[] = {"SAMPLE1", "SAMPLE2", "SAMPLE3","\n", "SAMPLE4", "SAMPLE5", "SAMPLE6", "\n", "SAMPLE7", "SAMPLE8", "STOP", NULL};
static lv_obj_t * btnMatrix;
static lv_obj_t * volSlider;
static lv_obj_t * volSlider_label;
static lv_obj_t * VUmeter;
static lv_obj_t * VUmeterR;
static lv_obj_t * VUmeterL;
static lv_obj_t * label_AudioStatus;
static void event_handler_btnsAudio(lv_event_t * e);
static void volSlider_event_cb(lv_event_t * e);
lv_timer_t *VU_updateTimer;
void VU_updateTimer_cb(lv_timer_t *t);
// ------------------ TAB SD ---------------------------------------
static lv_obj_t * btns_SD;
static lv_obj_t * label_SD_ls;
const char *btnSDLabels[] = {"REFRESH", NULL};
static void event_handler_btnsSD(lv_event_t * e);
// ------------------ TAB LDR ---------------------------------------
lv_obj_t *LDR_label_txt;
lv_obj_t *LDR_label_val;

lv_obj_t *LDR_label_thresTxt;
lv_obj_t *LDR_label_thresVal;

lv_obj_t *LDR_label_dark;

lv_obj_t *LDR_chart;
lv_chart_series_t *LDR_series;
lv_chart_series_t *LDR_ser_slow;

lv_timer_t *LDR_updateTimer;
void LDR_updateTimer_cb(lv_timer_t *t);

// ------------------ TAB RGB ---------------------------------------
lv_obj_t *RGB_btns;
const char *RGB_btns_labels[] = {"RED", "GREEN", "BLUE", NULL};

static void event_handler_btnsRGB(lv_event_t * e);
// ------------------ TAB SYS ---------------------------------------
lv_obj_t *label_sysinfo;
void printSysInfo(char * buf);
lv_timer_t *SYS_updateTimer; // used to periodically update the info page
void SYS_updateTimer_cb(lv_timer_t *t);

void gui_init(void)
{
    Serial.println("GUI: Creating main screen...");
    lv_obj_t *screenMain = lv_obj_create(NULL);
    
    Serial.println("GUI: Loading screen...");
    lv_scr_load(screenMain);

    // Title
    Serial.println("GUI: Creating title...");
    lv_obj_t *title = lv_label_create(screenMain);
    lv_label_set_text(title, "WiFi Manager");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    Serial.println("GUI: Title created");

    // Status label
    Serial.println("GUI: Creating status label...");
    status_label = lv_label_create(screenMain);
    lv_label_set_text(status_label, "Status: Disconnected");
    lv_obj_align(status_label, LV_ALIGN_TOP_LEFT, 10, 40);

    // Scan button
    scan_btn = lv_btn_create(screenMain);
    lv_obj_set_size(scan_btn, 80, 35);
    lv_obj_align(scan_btn, LV_ALIGN_TOP_RIGHT, -10, 35);
    lv_obj_add_event_cb(scan_btn, scan_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *scan_label = lv_label_create(scan_btn);
    lv_label_set_text(scan_label, "SCAN");
    lv_obj_center(scan_label);

    // WiFi list
    wifi_list = lv_list_create(screenMain);
    lv_obj_set_size(wifi_list, 300, 140);
    lv_obj_align(wifi_list, LV_ALIGN_TOP_MID, 0, 75);

    // Password input
    lv_obj_t *pwd_label = lv_label_create(screenMain);
    lv_label_set_text(pwd_label, "Password:");
    lv_obj_align(pwd_label, LV_ALIGN_BOTTOM_LEFT, 10, -80);

    password_ta = lv_textarea_create(screenMain);
    lv_obj_set_size(password_ta, 180, 35);
    lv_obj_align(password_ta, LV_ALIGN_BOTTOM_LEFT, 10, -50);
    lv_textarea_set_placeholder_text(password_ta, "Enter password");
    lv_textarea_set_password_mode(password_ta, true);
    lv_obj_add_event_cb(password_ta, password_ta_event_cb, LV_EVENT_CLICKED, NULL);

    // Connect button
    connect_btn = lv_btn_create(screenMain);
    lv_obj_set_size(connect_btn, 80, 35);
    lv_obj_align(connect_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -50);
    lv_obj_add_event_cb(connect_btn, connect_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_state(connect_btn, LV_STATE_DISABLED);
    
    lv_obj_t *connect_label = lv_label_create(connect_btn);
    lv_label_set_text(connect_label, "CONNECT");
    lv_obj_center(connect_label);

    // Create keyboard (initially hidden)
    keyboard = lv_keyboard_create(screenMain);
    lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES / 2);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

    // Status update timer
    status_timer = lv_timer_create(status_timer_cb, 1000, NULL);

    // Load main screen
    lv_scr_load(screenMain);
    
    // Force screen update
    Serial.println("GUI: Forcing screen refresh...");
    lv_refr_now(NULL);
    Serial.println("GUI: Screen refresh complete");

    // Initial scan
    wifi_ui_scan();
    update_wifi_list();
}

void update_wifi_list()
{
    // Clear existing list
    lv_obj_clean(wifi_list);
    
    int count = wifi_ui_get_network_count();
    if (count == 0) {
        lv_obj_t *btn = lv_list_add_btn(wifi_list, LV_SYMBOL_WIFI, "No networks found");
        lv_obj_add_state(btn, LV_STATE_DISABLED);
        return;
    }
    
    for (int i = 0; i < count; i++) {
        const char* ssid = wifi_ui_get_ssid(i);
        int rssi = wifi_ui_get_rssi(i);
        bool encrypted = wifi_ui_get_encrypted(i);
        
        char label[64];
        const char* signal_icon = (rssi > -50) ? LV_SYMBOL_WIFI : 
                                 (rssi > -70) ? "📶" : "📶";
        const char* lock_icon = encrypted ? "🔒" : "";
        
        snprintf(label, sizeof(label), "%s %s %s (%ddBm)", 
                signal_icon, lock_icon, ssid, rssi);
        
        lv_obj_t *btn = lv_list_add_btn(wifi_list, NULL, label);
        lv_obj_add_event_cb(btn, wifi_list_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
}

static void wifi_list_event_cb(lv_event_t * e)
{
    selected_network = (int)(intptr_t)lv_event_get_user_data(e);
    
    // Check if network needs password
    bool encrypted = wifi_ui_get_encrypted(selected_network);
    const char* ssid = wifi_ui_get_ssid(selected_network);
    
    // Try to load saved password
    char saved_password[64] = {0};
    bool has_saved = wifi_ui_load_credentials(ssid, saved_password, sizeof(saved_password));
    
    if (encrypted && !has_saved) {
        // Show password input
        lv_textarea_set_text(password_ta, "");
        lv_obj_clear_state(connect_btn, LV_STATE_DISABLED);
    } else {
        // Connect directly (open network or saved password)
        if (has_saved) {
            lv_textarea_set_text(password_ta, saved_password);
        } else {
            lv_textarea_set_text(password_ta, "");
        }
        lv_obj_clear_state(connect_btn, LV_STATE_DISABLED);
        
        // Auto-connect if we have credentials or it's open
        if (!encrypted || has_saved) {
            wifi_ui_connect(ssid, has_saved ? saved_password : "");
        }
    }
}

static void connect_btn_event_cb(lv_event_t * e)
{
    if (selected_network < 0) return;
    
    const char* ssid = wifi_ui_get_ssid(selected_network);
    const char* password = lv_textarea_get_text(password_ta);
    
    wifi_ui_connect(ssid, password);
}

static void scan_btn_event_cb(lv_event_t * e)
{
    Serial.println("SCAN button pressed!");
    wifi_ui_scan();
    update_wifi_list();
}

static void password_ta_event_cb(lv_event_t * e)
{
    lv_keyboard_set_textarea(keyboard, password_ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void status_timer_cb(lv_timer_t * timer)
{
    String status = "Status: " + wifi_ui_get_status();
    if (wifi_ui_is_connected()) {
        status += " - IP: " + WiFi.localIP().toString();
    }
    lv_label_set_text(status_label, status.c_str());
}