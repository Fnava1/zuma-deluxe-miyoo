#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <SDL2/SDL.h>

#include "font8x8.h"
#include "pack_pak_native.h"
#include "mo3_decode.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;

static int  g_progress_pct = 0;
static char g_status_msg[256] = "Inicializando instalador...";
static char g_detail_msg[256] = "";

static const SDL_Color COLOR_BG         = { 11, 23, 29, 255 };
static const SDL_Color COLOR_PANEL      = { 18, 38, 48, 255 };
static const SDL_Color COLOR_PANEL_EDGE = { 34, 70, 88, 255 };
static const SDL_Color COLOR_GOLD       = { 229, 169, 59, 255 };
static const SDL_Color COLOR_WHITE      = { 240, 244, 248, 255 };
static const SDL_Color COLOR_GRAY       = { 140, 160, 175, 255 };
static const SDL_Color COLOR_GREEN      = { 52, 199, 89, 255 };
static const SDL_Color COLOR_RED        = { 255, 69, 58, 255 };

static void draw_char(SDL_Renderer *ren, char c, int x, int y, int scale, SDL_Color color) {
    if ((unsigned char)c > 127) c = '?';
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    for (int row = 0; row < 8; row++) {
        char b = font8x8_basic[(unsigned char)c][row];
        for (int col = 0; col < 8; col++) {
            if (b & (1 << col)) {
                SDL_Rect r = { x + col * scale, y + row * scale, scale, scale };
                SDL_RenderFillRect(ren, &r);
            }
        }
    }
}

static void draw_text(SDL_Renderer *ren, const char *text, int x, int y, int scale, SDL_Color color) {
    int cur_x = x;
    for (size_t i = 0; text && text[i]; i++) {
        if (text[i] == '\n') {
            y += 10 * scale;
            cur_x = x;
            continue;
        }
        draw_char(ren, text[i], cur_x, y, scale, color);
        cur_x += 8 * scale;
    }
}

static void draw_text_centered(SDL_Renderer *ren, const char *text, int center_x, int y, int scale, SDL_Color color) {
    if (!text) return;
    int len = (int)strlen(text);
    int w = len * 8 * scale;
    draw_text(ren, text, center_x - w / 2, y, scale, color);
}

static void pump_events() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            exit(1);
        }
    }
}

static void render_screen(bool is_error, const char *error_folder) {
    pump_events();

    // 1. Clear background
    SDL_SetRenderDrawColor(g_renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
    SDL_RenderClear(g_renderer);

    // 2. Main card panel
    SDL_Rect panel = { 30, 25, 580, 430 };
    SDL_SetRenderDrawColor(g_renderer, COLOR_PANEL.r, COLOR_PANEL.g, COLOR_PANEL.b, 255);
    SDL_RenderFillRect(g_renderer, &panel);

    SDL_SetRenderDrawColor(g_renderer, COLOR_PANEL_EDGE.r, COLOR_PANEL_EDGE.g, COLOR_PANEL_EDGE.b, 255);
    SDL_RenderDrawRect(g_renderer, &panel);

    // 3. Header title
    draw_text_centered(g_renderer, "ZUMA DELUXE", 320, 50, 3, COLOR_GOLD);
    draw_text_centered(g_renderer, "CONFIGURACION DE RECURSOS - ONION OS", 320, 85, 1, COLOR_WHITE);

    // Divider line
    SDL_SetRenderDrawColor(g_renderer, COLOR_GOLD.r, COLOR_GOLD.g, COLOR_GOLD.b, 150);
    SDL_RenderDrawLine(g_renderer, 80, 110, 560, 110);

    if (is_error) {
        // Error state UI
        draw_text_centered(g_renderer, "ARCHIVOS DEL JUEGO INCOMPLETOS", 320, 140, 2, COLOR_RED);

        char buf[256];
        if (error_folder && error_folder[0]) {
            snprintf(buf, sizeof(buf), "Falta la carpeta obligatoria: '%s'", error_folder);
        } else {
            snprintf(buf, sizeof(buf), "No se encontraron los datos originales del juego.");
        }
        draw_text_centered(g_renderer, buf, 320, 185, 1, COLOR_WHITE);

        draw_text_centered(g_renderer, "Para jugar en tu Miyoo Mini, copia los archivos", 320, 230, 1, COLOR_GRAY);
        draw_text_centered(g_renderer, "de tu juego de PC (Steam / GOG / CD) en la carpeta:", 320, 250, 1, COLOR_GRAY);

        draw_text_centered(g_renderer, "/Roms/PORTS/Games/Zuma Deluxe/", 320, 280, 1, COLOR_GOLD);

        draw_text_centered(g_renderer, "Carpetas requeridas:", 320, 320, 1, COLOR_GRAY);
        draw_text_centered(g_renderer, "fonts, images, levels, properties, sounds, music", 320, 340, 1, COLOR_WHITE);

        draw_text_centered(g_renderer, "Pulsa [B] o [MENU] para volver a OnionOS.", 320, 400, 1, COLOR_GOLD);
    } else {
        // Normal installation progress UI
        draw_text_centered(g_renderer, g_status_msg, 320, 170, 2, COLOR_WHITE);
        if (g_detail_msg[0]) {
            draw_text_centered(g_renderer, g_detail_msg, 320, 210, 1, COLOR_GRAY);
        }

        // Progress bar container
        SDL_Rect bar_bg = { 100, 260, 440, 30 };
        SDL_SetRenderDrawColor(g_renderer, 6, 12, 16, 255);
        SDL_RenderFillRect(g_renderer, &bar_bg);
        SDL_SetRenderDrawColor(g_renderer, COLOR_GOLD.r, COLOR_GOLD.g, COLOR_GOLD.b, 255);
        SDL_RenderDrawRect(g_renderer, &bar_bg);

        // Progress bar fill
        if (g_progress_pct > 0) {
            int fill_w = (int)((436 * g_progress_pct) / 100);
            if (fill_w > 436) fill_w = 436;
            SDL_Rect bar_fill = { 102, 262, fill_w, 26 };
            SDL_Color fill_color = (g_progress_pct >= 100) ? COLOR_GREEN : COLOR_GOLD;
            SDL_SetRenderDrawColor(g_renderer, fill_color.r, fill_color.g, fill_color.b, 255);
            SDL_RenderFillRect(g_renderer, &bar_fill);
        }

        // Percentage text
        char pct_str[32];
        snprintf(pct_str, sizeof(pct_str), "%d%%", g_progress_pct);
        draw_text_centered(g_renderer, pct_str, 320, 310, 2, COLOR_GOLD);

        // Advice text
        draw_text_centered(g_renderer, "Optimizando assets en main.pak para maxima velocidad...", 320, 380, 1, COLOR_GRAY);
        draw_text_centered(g_renderer, "Este proceso solo se realiza una vez en el primer arranque.", 320, 405, 1, COLOR_GRAY);
    }

    SDL_RenderPresent(g_renderer);
}

static void on_pack_progress(int pct, const char *msg) {
    g_progress_pct = pct;
    if (msg) {
        strncpy(g_detail_msg, msg, sizeof(g_detail_msg) - 1);
        g_detail_msg[sizeof(g_detail_msg) - 1] = '\0';
    }
    render_screen(false, NULL);
}

static bool check_directory(const char *dir) {
    struct stat st;
    return (stat(dir, &st) == 0 && (st.st_mode & S_IFDIR));
}

static bool check_file(const char *file) {
    struct stat st;
    return (stat(file, &st) == 0 && (st.st_mode & S_IFREG));
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    g_window = SDL_CreateWindow(
        "Zuma Deluxe Installer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    g_renderer = SDL_CreateRenderer(
        g_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!g_renderer) {
        g_renderer = SDL_CreateRenderer(g_window, -1, 0);
    }
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }

    // 1. Check if required directories exist
    const char *REQUIRED[] = { "fonts", "images", "levels", "properties", "sounds" };
    const char *missing_folder = NULL;

    for (size_t i = 0; i < 5; i++) {
        if (!check_directory(REQUIRED[i])) {
            missing_folder = REQUIRED[i];
            break;
        }
    }

    if (missing_folder) {
        render_screen(true, missing_folder);
        // Wait for user input to exit cleanly without freezing
        bool waiting = true;
        Uint32 start_ticks = SDL_GetTicks();
        while (waiting) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_QUIT) {
                    waiting = false;
                } else if (ev.type == SDL_KEYDOWN) {
                    waiting = false;
                }
            }
            // Auto-timeout after 15 seconds
            if (SDL_GetTicks() - start_ticks > 15000) {
                waiting = false;
            }
            SDL_Delay(50);
        }
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 2; // Missing files
    }

    // 2. Decode soundtrack if needed
    if (!check_file("music/zuma.it")) {
        if (check_file("music/zuma.mo3")) {
            strncpy(g_status_msg, "Decodificando banda sonora (MO3 -> IT)...", sizeof(g_status_msg) - 1);
            strncpy(g_detail_msg, "music/zuma.mo3 -> music/zuma.it", sizeof(g_detail_msg) - 1);
            g_progress_pct = 10;
            render_screen(false, NULL);

            int mo3_res = convert_mo3_to_it("music/zuma.mo3", "music/zuma.it", "libs/libunmo3.so");
            if (mo3_res != 0) {
                // Try fallback to local ./libs/
                mo3_res = convert_mo3_to_it("music/zuma.mo3", "music/zuma.it", "./libs/libunmo3.so");
            }
        }
    }

    // 3. Pack main.pak container
    if (!check_file("main.pak")) {
        strncpy(g_status_msg, "Empaquetando recursos del juego...", sizeof(g_status_msg) - 1);
        g_progress_pct = 20;
        render_screen(false, NULL);

        int pack_res = pack_pak_native(".", "main.pak", NULL, 0, on_pack_progress);
        if (pack_res != 0) {
            render_screen(true, "Error al generar main.pak");
            SDL_Delay(5000);
            SDL_DestroyRenderer(g_renderer);
            SDL_DestroyWindow(g_window);
            SDL_Quit();
            return 3;
        }
    }

    // 4. Mark setup as completed
    FILE *flag = fopen(".installed", "w");
    if (flag) {
        fputs("installed=true\n", flag);
        fclose(flag);
    }

    // Create userdata directory if missing
#ifdef _WIN32
    CreateDirectoryA("userdata", NULL);
#else
    mkdir("userdata", 0777);
#endif

    // 5. Completion screen
    g_progress_pct = 100;
    strncpy(g_status_msg, "Instalacion completada!", sizeof(g_status_msg) - 1);
    strncpy(g_detail_msg, "Iniciando Zuma Deluxe...", sizeof(g_detail_msg) - 1);
    render_screen(false, NULL);

    SDL_Delay(1200);

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0; // Success
}
