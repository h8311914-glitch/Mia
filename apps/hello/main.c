/** Only for the file containing the real main() function, before importing anything else */
#define MIA_MAIN

/** Includes most of Mia */
#include "mia.h"


/** Context Data for the game view */
struct context {
    // بازیکن
    vec3 player_pos;
    float player_vy;
    float player_width;
    float player_height;
    float player_depth;
    
    // موانع
    vec3 obs_pos[3];
    vec3 obs_size[3];
    float obs_speed[3];
    vec4 obs_color[3];
    
    int score;
    bool game_over;
    float ground_y;
    
    // برای رندر
    oobj logo_tex;
    oobj render_obj;
};

/** ساخت یک تکسچر خالی برای رندر بازی */
static oobj create_game_texture(oobj parent, int width, int height) {
    return RTex_new(parent, NULL, width, height);
}

/** رسم یک مکعب ساده در فضای 2D (برای سادگی) */
static void draw_cube_2d(oobj tex, vec2 pos, vec2 size, vec4 color) {
    RTex_rect(tex, pos, size, color);
}

/**
 * Called once at start of this AView
 */
static void setup(oobj view) {
    struct context *C = o_user_new0(view, *C, 1);
    
    o_log("بازی سه‌بعدی افقی با Mia شروع شد!");
    
    // تنظیمات اولیه بازیکن
    C->player_pos = vec3_(-5.0f, 0.5f, 0.0f);
    C->player_vy = 0.0f;
    C->player_width = 0.8f;
    C->player_height = 0.8f;
    C->player_depth = 0.8f;
    
    // تنظیم موانع
    C->obs_pos[0] = vec3_(3.0f, 0.5f, 0.0f);
    C->obs_size[0] = vec3_(1.0f, 0.8f, 0.8f);
    C->obs_speed[0] = 3.0f;
    C->obs_color[0] = vec4_(1, 0, 0, 1);
    
    C->obs_pos[1] = vec3_(6.0f, 1.5f, 0.0f);
    C->obs_size[1] = vec3_(0.8f, 1.0f, 0.8f);
    C->obs_speed[1] = 4.0f;
    C->obs_color[1] = vec4_(1, 0.5, 0, 1);
    
    C->obs_pos[2] = vec3_(9.0f, 0.5f, 0.0f);
    C->obs_size[2] = vec3_(1.2f, 0.6f, 0.8f);
    C->obs_speed[2] = 3.5f;
    C->obs_color[2] = vec4_(0.5, 0, 0, 1);
    
    C->score = 0;
    C->game_over = false;
    C->ground_y = -1.0f;
}

/**
 * Called every frame before rendering
 */
static void update(oobj view, oobj tex, float dt) {
    struct context *C = o_user(view);
    
    if (!C->game_over) {
        // ---- کنترل بازیکن با کلیدها ----
        if (AKey_down('d') || AKey_down(AKEY_RIGHT)) {
            C->player_pos.x += 5.0f * dt;
        }
        if (AKey_down('a') || AKey_down(AKEY_LEFT)) {
            C->player_pos.x -= 5.0f * dt;
        }
        
        // پرش
        if ((AKey_pressed(AKEY_SPACE) || AKey_pressed(AKEY_UP) || AKey_pressed('w')) 
            && C->player_pos.y <= 0.5f) {
            C->player_vy = 3.5f;
        }
        
        // گرانش
        C->player_vy -= 6.0f * dt;
        C->player_pos.y += C->player_vy * dt;
        if (C->player_pos.y < 0.5f) {
            C->player_pos.y = 0.5f;
            C->player_vy = 0.0f;
        }
        
        // حرکت موانع
        for (int i = 0; i < 3; i++) {
            C->obs_pos[i].x -= C->obs_speed[i] * dt;
            
            // اگر مانع از صفحه خارج شد، دوباره از راست بیاد
            if (C->obs_pos[i].x < -8.0f) {
                C->obs_pos[i].x = 8.0f + (float)(rand() % 4);
                C->obs_pos[i].y = 0.5f + (float)(rand() % 3) / 2.0f;
                C->obs_speed[i] = 3.0f + (float)(rand() % 4);
                C->score++;
            }
        }
        
        // ---- برخورد با موانع ----
        for (int i = 0; i < 3; i++) {
            float dx = fabs(C->player_pos.x - C->obs_pos[i].x);
            float dy = fabs(C->player_pos.y - C->obs_pos[i].y);
            float dz = fabs(C->player_pos.z - C->obs_pos[i].z);
            
            float halfW = C->player_width/2 + C->obs_size[i].x/2;
            float halfH = C->player_height/2 + C->obs_size[i].y/2;
            float halfD = C->player_depth/2 + C->obs_size[i].z/2;
            
            if (dx < halfW && dy < halfH && dz < halfD) {
                C->game_over = true;
            }
        }
    }
    
    // ریستارت با F5
    if (C->game_over && AKey_pressed(AKEY_F5)) {
        C->player_pos = vec3_(-5.0f, 0.5f, 0.0f);
        C->player_vy = 0.0f;
        C->obs_pos[0] = vec3_(3.0f, 0.5f, 0.0f);
        C->obs_pos[1] = vec3_(6.0f, 1.5f, 0.0f);
        C->obs_pos[2] = vec3_(9.0f, 0.5f, 0.0f);
        C->score = 0;
        C->game_over = false;
    }
}

/**
 * Called every frame to render
 */
static void render(oobj view, oobj tex, float dt) {
    struct context *C = o_user(view);
    
    // پاک کردن صفحه با رنگ آبی آسمانی
    RTex_clear_full(tex, vec4_(0.53f, 0.81f, 0.98f, 1.0f));
    
    // رسم زمین (نوار سبز پایین)
    RTex_rect(tex, vec2_(-10.0f, C->ground_y), vec2_(20.0f, 0.5f), vec4_(0, 0.7, 0, 1));
    
    // رسم بازیکن (مکعب آبی)
    vec2 player_pos_2d = vec2_(C->player_pos.x, C->player_pos.y);
    vec2 player_size = vec2_(C->player_width, C->player_height);
    RTex_rect(tex, vec2_sub(player_pos_2d, vec2_scale(player_size, 0.5f)), player_size, vec4_(0, 0, 1, 1));
    RTex_rect_border(tex, vec2_sub(player_pos_2d, vec2_scale(player_size, 0.5f)), player_size, vec4_(0, 0, 0.5, 1), 0.05f);
    
    // رسم موانع
    for (int i = 0; i < 3; i++) {
        vec2 obs_pos_2d = vec2_(C->obs_pos[i].x, C->obs_pos[i].y);
        vec2 obs_size = vec2_(C->obs_size[i].x, C->obs_size[i].y);
        RTex_rect(tex, vec2_sub(obs_pos_2d, vec2_scale(obs_size, 0.5f)), obs_size, C->obs_color[i]);
        RTex_rect_border(tex, vec2_sub(obs_pos_2d, vec2_scale(obs_size, 0.5f)), obs_size, vec4_(0.2, 0.2, 0.2, 1), 0.05f);
    }
    
    // نمایش امتیاز
    char score_text[32];
    sprintf(score_text, "امتیاز: %d", C->score);
    vec2 text_pos = vec2_(-8.0f, 4.0f);
    vec4 white = vec4_(1, 1, 1, 1);
    RTex_text(tex, text_pos, 0.8f, white, score_text);
    
    // Game Over
    if (C->game_over) {
        RTex_text(tex, vec2_(-4.0f, 0.0f), 1.5f, vec4_(1, 0, 0, 1), "GAME OVER!");
        RTex_text(tex, vec2_(-4.5f, -1.0f), 0.8f, vec4_(0.3, 0.3, 0.3, 1), "F5 برای شروع دوباره");
    }
    
    // راهنمای کنترل
    RTex_text(tex, vec2_(-8.0f, -4.0f), 0.5f, vec4_(0.3, 0.3, 0.3, 1), "کنترل: A/D یا چپ/راست | Space/Up برای پرش");
}

/**
 * Starting point of the mia app
 */
static void app_main(oobj root) {
    x_install();
    
    /** AScene with the AView of this main.c hello world file */
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    AScene_opaque_set(scene, false);
}

/**
 * Actual c main entry point.
 */
int main(int argc, char **argv) {
    struct a_app_run_options options = a_app_run_options_default();
    options.mic_enable = true;
    options.log_level = O_LOG_INFO;
    
    a_app_run(app_main, &options);
    return 0;
}
