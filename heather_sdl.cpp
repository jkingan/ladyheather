#define EXTERN extern

#include "heather.ch"
// #include "heathfnt.ch"

extern unsigned char h_medium_font[];
extern unsigned char h_small_font[];
extern unsigned char h_large_font[];
extern unsigned char h_font_12[];

#ifdef __MACH__    // OSX time functions
   #include <mach/clock.h>
   #include <mach/mach.h>
#endif


extern unsigned char * dot_font;  // pointer to character font in use


#ifdef WINDOWS
C8 szAppName[256 + 1] = "Lady Heather's Disciplined Oscillator Control Program - "VERSION "";

u08 timer_set;     // flag set if dialog timer has been set
u08 path_help;     // flag set if help message has been seen before
#endif


u32 RGB_NATIVE(int r, int g, int b)
{
    return (((u32)r) << 24)       | (((u32)g) << 16)      | (((u32)b) << 8) | 0x000000FF;
}
u32 get_sdl_color(u08 color)
{
    if(color == 0xFF) {
        return RGB_NATIVE(0, 0, 35); // special plot area background color highlight
    } else {
        return palette[color];
    }
}

void dot(int x, int y, u08 color)
{
    if(!display) return;
    int bpp = display->format->BytesPerPixel;
    Uint8 * p = (Uint8 *)display->pixels + y * display->pitch + x * bpp;

    *(Uint32 *)p = get_sdl_color(color);
}

#define KBD_Q_SIZE 16
int kbd_queue[KBD_Q_SIZE + 1];
int kbd_in, kbd_out;

void add_kbd(int key)
{
    // add a keystroke to the keyboard queue

    if(++kbd_in >= KBD_Q_SIZE) {
        kbd_in = 0;
    }
    if(kbd_in == kbd_out) { // kbd queue is full
        if(--kbd_in < 0) {
            kbd_in = KBD_Q_SIZE - 1;
        }
    } else { // put keystoke in the queue
        kbd_queue[kbd_in] = key;
    }
}

int sdl_getch(void)
{
    int key;

    // get a keystroke from the keyboard queue

    if(kbd_in == kbd_out) {
        return 0;                           // no keys in the queue

    }
    if(++kbd_out >= KBD_Q_SIZE) {
        kbd_out = 0;                        // get to next queue entry
    }
    key = kbd_queue[kbd_out];               // get keystroke from queue
    return key;                             // return it
}

int sdl_kbhit(void)
{
    // return true if a keystoke is available
    reset_kbd_timer();
    return (kbd_in != kbd_out); // return true if anything is in the keyboard queue
}

static bool sdl_initialized = 0;

void init_screen(int why)
{
    unsigned char * vfx_font;
    int font_height;
    int i;
    unsigned int j;

    // Initialize the graphics screen

    config_screen(3); // initialize screen rendering variables

    // setup text drawing using WIN_VFX fonts
    if(user_font_size == 0) {
        if(big_plot && (ebolt == 0)) {
            vfx_font = &h_medium_font[0];
            user_font_size = 14;
        } else {
            vfx_font = &h_font_12[0];
            user_font_size = 12;
        }
    } else if(user_font_size <= 8) {
        vfx_font = &h_small_font[0];
        user_font_size = 8;
    } else if(user_font_size <= 12) {
        vfx_font = &h_font_12[0];
        user_font_size = 12;
    } else if(user_font_size <= 14) {
        vfx_font = &h_medium_font[0];
        user_font_size = 14;
    } else if(user_font_size <= 16) {
        vfx_font = &h_large_font[0];
        user_font_size = 16;
    } else {
        vfx_font = &h_font_12[0];
        user_font_size = 12;
    }

    font_height = user_font_size;
    TEXT_HEIGHT = font_height;
    TEXT_WIDTH = 8;  // !!!!!!!
    if(font_height <= 12) {
        small_font = 2;
    } else {
        small_font = 0;
    }

    dot_font = (unsigned char *)(void *)vfx_font;

    /*  Window variables  */

    int x, y;
    unsigned int width, height;

    int flags = (go_fullscreen ? (SDL_WINDOW_FULLSCREEN_DESKTOP) : 0);
    flags |= SDL_WINDOW_RESIZABLE;

    if(false == sdl_initialized) {
        SDL_StartTextInput();
        SDL_Init(SDL_INIT_VIDEO);
        sdl_initialized = true;
        SDL_Rect r;
        SDL_GetDisplayBounds(0, &r);
        display_width = r.w;
        display_height = r.h;

        SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1");


    }

    width = SCREEN_WIDTH;
    height = SCREEN_HEIGHT;

    if(ne_window) {
        SDL_SetWindowSize(ne_window, width, height);
    } else {
        ne_window = SDL_CreateWindow("Lady Heather",
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     width, height,
                                     flags);
    }

    sdl_window_width = width;
    sdl_window_height = height;

    if(ne_renderer) {
        SDL_DestroyRenderer(ne_renderer);
    }

    ne_renderer = SDL_CreateRenderer(ne_window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(ne_renderer, width, height);

    if(ne_texture) {
        SDL_DestroyTexture(ne_texture);
    }

    ne_texture = SDL_CreateTexture(ne_renderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   width, height);

    sdl_texture_width = width;
    sdl_texture_height = height;

    SDL_SetTextureBlendMode(ne_texture, SDL_BLENDMODE_NONE);


    if(display) {
        SDL_FreeSurface(display);
    }

    display = SDL_CreateRGBSurface(0, width, height, 32,
                                   0xFF000000,
                                   0x00FF0000,
                                   0x0000FF00,
                                   0x000000FF);

    SDL_SetSurfaceBlendMode(display, SDL_BLENDMODE_NONE);

    SDL_SetWindowFullscreen(ne_window, flags);

    // setup screen palette
    memset(palette, 0xff, sizeof(palette));
    setup_palette();

    config_screen(why); // re-initialize screen rendering variables
                      // to reflect any changes due to font size

    printf("screen configured\n");
    erase_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    printf("screen init done\n");
}

void kill_screen(void)
{
    if(display) {
        SDL_FreeSurface(display);
        display = 0;
    }

    if(ne_texture) {
        SDL_DestroyTexture(ne_texture);
        ne_texture = 0;
    }

    if(ne_renderer) {
        SDL_DestroyRenderer(ne_renderer);
        ne_renderer = 0;
    }

    if(ne_window) {
        SDL_DestroyWindow(ne_window);
        ne_window = 0;
    }
}

void refresh_page(void)
{
    if(!display) return;
    
    SDL_UpdateTexture(ne_texture, NULL, display->pixels, display->pitch);
    SDL_RenderClear(ne_renderer);
    SDL_RenderCopy(ne_renderer, ne_texture, NULL, NULL);

    SDL_RenderPresent(ne_renderer);
}

u08 get_pixel(COORD x, COORD y)
{
    u32 pixel;
    int i;

    int bpp = display->format->BytesPerPixel;
    u32* p = (u32*)((Uint8 *)display->pixels + y * display->pitch + x * bpp);

    for(i = 0; i < 16; i++) { // convert screen value to color code
        if(*p == palette[i]) {
            return i;
        }
    }
    return 0;
}

int get_sdl_event()
{
    int i;

    if(display == 0) {
        return 0;
    }
    SDL_PumpEvents();
    SDL_Event e;

    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            shut_down(0);
            return 6;
        } else if(e.type == SDL_WINDOWEVENT) {
            if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
                sdl_window_width = e.window.data1;
                sdl_window_height = e.window.data2;

                printf("New size is %d %d\n", sdl_window_width, sdl_window_height);

                if(0 == sdl_scaling) {
                    new_width = sdl_window_width;
                    new_height = sdl_window_height;
                    need_resize = 1;
                    return 2;
                }
            }
        } else if(e.type == SDL_TEXTINPUT) {
            const char * text = e.text.text;
            while(*text) {
                add_kbd(*text++);
            }
        } else if(e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            if(key ==  SDLK_HOME) {
                add_kbd(HOME_CHAR);
            } else if(key ==  SDLK_UP) {
                add_kbd(UP_CHAR);
            } else if(key ==  SDLK_PAGEUP) {
                add_kbd(PAGE_UP);
            } else if(key ==  SDLK_LEFT) {
                add_kbd(LEFT_CHAR);
            } else if(key ==  SDLK_RIGHT) {
                add_kbd(RIGHT_CHAR);
            } else if(key ==  SDLK_END) {
                add_kbd(END_CHAR);
            } else if(key ==  SDLK_DOWN) {
                add_kbd(DOWN_CHAR);
            } else if(key ==  SDLK_PAGEDOWN) {
                add_kbd(PAGE_DOWN);
            } else if(key ==  SDLK_INSERT) {
                add_kbd(INS_CHAR);
            } else if(key ==  SDLK_DELETE) {
                add_kbd(DEL_CHAR);
            } else if(key ==  SDLK_BACKSPACE) {
                add_kbd(0x08);
            } else if(key ==  SDLK_TAB) {
                add_kbd(0x09);
            } else if(key ==  SDLK_RETURN) {
                add_kbd(0x0D);
            } else if(key ==  SDLK_CLEAR) {
                add_kbd(0x0B);
            } else if(key ==  SDLK_RETURN) {
                add_kbd(0x0D);
            } else if(key ==  SDLK_ESCAPE) {
                add_kbd(0x1B);
            } else if(key ==  SDLK_PAUSE) {
                break_flag = 1;
            } else if(key ==  SDLK_F1) {
                add_kbd(F1_CHAR);
            } else if(key ==  SDLK_F2) {
                add_kbd(F2_CHAR);
            } else if(key ==  SDLK_F3) {
                add_kbd(F3_CHAR);
            } else if(key ==  SDLK_F4) {
                add_kbd(F4_CHAR);
            } else if(key ==  SDLK_F5) {
                add_kbd(F5_CHAR);
            } else if(key ==  SDLK_F6) {
                add_kbd(F6_CHAR);
            } else if(key ==  SDLK_F7) {
                add_kbd(F7_CHAR);
            } else if(key ==  SDLK_F8) {
                add_kbd(F8_CHAR);
            } else if(key ==  SDLK_F9) {
                add_kbd(F9_CHAR);
            } else if(key ==  SDLK_F12) {
                add_kbd(0);
            }
            return 3;
        }
    }

    return 0;
}

int screen_active()
{
    return display != 0;
}
