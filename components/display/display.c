
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tftspi.h"
#include "tft.h"
#include "driver/gpio.h"

static struct tm *tm_info;
static char tmp_buff[64];
static time_t time_now, time_last = 0;

#define GDEMO_TIME 1000
#define GDEMO_INFO_TIME 5000

static void _checkTime() {
    time(&time_now);
    if (time_now > time_last) {
        color_t last_fg, last_bg;
        time_last = time_now;
        tm_info = localtime(&time_now);
        sprintf(tmp_buff, "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

        TFT_saveClipWin();
        TFT_resetclipwin();

        Font curr_font = tft_cfont;
        last_bg = tft_bg;
        last_fg = tft_fg;
        tft_fg = TFT_YELLOW;
        tft_bg = (color_t){64, 64, 64};
        TFT_setFont(DEFAULT_FONT, NULL);

        TFT_fillRect(1, tft_height - TFT_getfontheight() - 8, tft_width - 3, TFT_getfontheight() + 6, tft_bg);
        TFT_print(tmp_buff, CENTER, tft_height - TFT_getfontheight() - 5);

        tft_cfont = curr_font;
        tft_fg = last_fg;
        tft_bg = last_bg;

        TFT_restoreClipWin();
    }
}

static int Wait(int ms) {
    uint8_t tm = 1;
    if (ms < 0) {
        tm = 0;
        ms *= -1;
    }
    if (ms <= 50) {
        vTaskDelay(ms / portTICK_RATE_MS);
        //if (_checkTouch()) return 0;
    } else {
        for (int n = 0; n < ms; n += 50) {
            vTaskDelay(50 / portTICK_RATE_MS);
            if (tm)
                _checkTime();
            //if (_checkTouch()) return 0;
        }
    }
    return 1;
}

static unsigned int rand_interval(unsigned int min, unsigned int max) {
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

static void _dispTime() {
    Font curr_font = tft_cfont;
    if (tft_width < 240)
        TFT_setFont(DEF_SMALL_FONT, NULL);
    else
        TFT_setFont(DEFAULT_FONT, NULL);

    time(&time_now);
    time_last = time_now;
    tm_info = localtime(&time_now);
    sprintf(tmp_buff, "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    TFT_print(tmp_buff, CENTER, tft_height - TFT_getfontheight() - 5);

    tft_cfont = curr_font;
}

static void disp_header(char *info) {
    TFT_fillScreen(TFT_BLACK);
    TFT_resetclipwin();

    tft_fg = TFT_YELLOW;
    tft_bg = (color_t){64, 64, 64};

    if (tft_width < 240)
        TFT_setFont(DEF_SMALL_FONT, NULL);
    else
        TFT_setFont(DEFAULT_FONT, NULL);
    TFT_fillRect(0, 0, tft_width - 1, TFT_getfontheight() + 8, tft_bg);
    TFT_drawRect(0, 0, tft_width - 1, TFT_getfontheight() + 8, TFT_CYAN);

    TFT_fillRect(0, tft_height - TFT_getfontheight() - 9, tft_width - 1, TFT_getfontheight() + 8, tft_bg);
    TFT_drawRect(0, tft_height - TFT_getfontheight() - 9, tft_width - 1, TFT_getfontheight() + 8, TFT_CYAN);

    TFT_print(info, CENTER, 4);
    _dispTime();

    tft_bg = TFT_BLACK;
    TFT_setclipwin(0, TFT_getfontheight() + 9, tft_width - 1, tft_height - TFT_getfontheight() - 10);
}

static void update_header(char *hdr, char *ftr) {
    color_t last_fg, last_bg;

    TFT_saveClipWin();
    TFT_resetclipwin();

    Font curr_font = tft_cfont;
    last_bg = tft_bg;
    last_fg = tft_fg;
    tft_fg = TFT_YELLOW;
    tft_bg = (color_t){64, 64, 64};
    if (tft_width < 240)
        TFT_setFont(DEF_SMALL_FONT, NULL);
    else
        TFT_setFont(DEFAULT_FONT, NULL);

    if (hdr) {
        TFT_fillRect(1, 1, tft_width - 3, TFT_getfontheight() + 6, tft_bg);
        TFT_print(hdr, CENTER, 4);
    }

    if (ftr) {
        TFT_fillRect(1, tft_height - TFT_getfontheight() - 8, tft_width - 3, TFT_getfontheight() + 6, tft_bg);
        if (strlen(ftr) == 0)
            _dispTime();
        else
            TFT_print(ftr, CENTER, tft_height - TFT_getfontheight() - 5);
    }

    tft_cfont = curr_font;
    tft_fg = last_fg;
    tft_bg = last_bg;

    TFT_restoreClipWin();
}

static color_t random_color() {
    color_t color;
    color.r = (uint8_t)rand_interval(8, 252);
    color.g = (uint8_t)rand_interval(8, 252);
    color.b = (uint8_t)rand_interval(8, 252);
    return color;
}

//---------------------
static void font_demo() {
    int x, y, n;
    uint32_t end_time;

    disp_header("FONT DEMO");

    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        y = 4;
        for (int f = DEFAULT_FONT; f < FONT_7SEG; f++) {
            tft_fg = random_color();
            TFT_setFont(f, NULL);
            TFT_print("Welcome to ESP32", 4, y);
            y += TFT_getfontheight() + 4;
            n++;
        }
    }
    sprintf(tmp_buff, "%d STRINGS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    disp_header("ROTATED FONT DEMO");

    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        for (int f = DEFAULT_FONT; f < FONT_7SEG; f++) {
            tft_fg = random_color();
            TFT_setFont(f, NULL);
            x = rand_interval(8, tft_dispWin.x2 - 8);
            y = rand_interval(0, (tft_dispWin.y2 - tft_dispWin.y1) - TFT_getfontheight() - 2);
            tft_font_rotate = rand_interval(0, 359);

            TFT_print("Welcome to ESP32", x, y);
            n++;
        }
    }
    tft_font_rotate = 0;
    sprintf(tmp_buff, "%d STRINGS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    disp_header("7-SEG FONT DEMO");

    int ms = 0;
    int last_sec = 0;
    uint32_t ctime = clock();
    end_time = clock() + GDEMO_TIME * 2;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        y = 12;
        ms = clock() - ctime;
        time(&time_now);
        tm_info = localtime(&time_now);
        if (tm_info->tm_sec != last_sec) {
            last_sec = tm_info->tm_sec;
            ms = 0;
            ctime = clock();
        }

        tft_fg = TFT_ORANGE;
        sprintf(tmp_buff, "%02d:%02d:%03d", tm_info->tm_min, tm_info->tm_sec, ms);
        TFT_setFont(FONT_7SEG, NULL);
        if ((tft_width < 240) || (tft_height < 240))
            set_7seg_font_atrib(8, 1, 1, TFT_DARKGREY);
        else
            set_7seg_font_atrib(12, 2, 1, TFT_DARKGREY);
        //TFT_clearStringRect(12, y, tmp_buff);
        TFT_print(tmp_buff, CENTER, y);
        n++;

        tft_fg = TFT_GREEN;
        y += TFT_getfontheight() + 12;
        if ((tft_width < 240) || (tft_height < 240))
            set_7seg_font_atrib(9, 1, 1, TFT_DARKGREY);
        else
            set_7seg_font_atrib(14, 3, 1, TFT_DARKGREY);
        sprintf(tmp_buff, "%02d:%02d", tm_info->tm_sec, ms / 10);
        //TFT_clearStringRect(12, y, tmp_buff);
        TFT_print(tmp_buff, CENTER, y);
        n++;

        tft_fg = random_color();
        y += TFT_getfontheight() + 8;
        set_7seg_font_atrib(6, 1, 1, TFT_DARKGREY);
        getFontCharacters((uint8_t *)tmp_buff);
        //TFT_clearStringRect(12, y, tmp_buff);
        TFT_print(tmp_buff, CENTER, y);
        n++;
    }
    sprintf(tmp_buff, "%d STRINGS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    disp_header("WINDOW DEMO");

    TFT_saveClipWin();
    TFT_resetclipwin();
    TFT_drawRect(38, 48, (tft_width * 3 / 4) - 36, (tft_height * 3 / 4) - 46, TFT_WHITE);
    TFT_setclipwin(40, 50, tft_width * 3 / 4, tft_height * 3 / 4);

    if ((tft_width < 240) || (tft_height < 240))
        TFT_setFont(DEF_SMALL_FONT, NULL);
    else
        TFT_setFont(UBUNTU16_FONT, NULL);
    tft_text_wrap = 1;
    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        tft_fg = random_color();
        TFT_print("This text is printed inside the window.\nLong line can be wrapped to the next line.\nWelcome to ESP32", 0, 0);
        n++;
    }
    tft_text_wrap = 0;
    sprintf(tmp_buff, "%d STRINGS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    TFT_restoreClipWin();
}

//---------------------
static void rect_demo() {
    int x, y, w, h, n;

    disp_header("RECTANGLE DEMO");

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(4, tft_dispWin.x2 - 4);
        y = rand_interval(4, tft_dispWin.y2 - 2);
        w = rand_interval(2, tft_dispWin.x2 - x);
        h = rand_interval(2, tft_dispWin.y2 - y);
        TFT_drawRect(x, y, w, h, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d RECTANGLES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("FILLED RECTANGLE", "");
    TFT_fillWindow(TFT_BLACK);
    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(4, tft_dispWin.x2 - 4);
        y = rand_interval(4, tft_dispWin.y2 - 2);
        w = rand_interval(2, tft_dispWin.x2 - x);
        h = rand_interval(2, tft_dispWin.y2 - y);
        TFT_fillRect(x, y, w, h, random_color());
        TFT_drawRect(x, y, w, h, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d RECTANGLES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//----------------------
static void pixel_demo() {
    int x, y, n;

    disp_header("DRAW PIXEL DEMO");

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(0, tft_dispWin.x2);
        y = rand_interval(0, tft_dispWin.y2);
        TFT_drawPixel(x, y, random_color(), 1);
        n++;
    }
    sprintf(tmp_buff, "%d PIXELS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//---------------------
static void line_demo() {
    int x1, y1, x2, y2, n;

    disp_header("LINE DEMO");

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x1 = rand_interval(0, tft_dispWin.x2);
        y1 = rand_interval(0, tft_dispWin.y2);
        x2 = rand_interval(0, tft_dispWin.x2);
        y2 = rand_interval(0, tft_dispWin.y2);
        TFT_drawLine(x1, y1, x2, y2, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d LINES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//----------------------
static void aline_demo() {
    int x, y, len, angle, n;

    disp_header("LINE BY ANGLE DEMO");

    x = (tft_dispWin.x2 - tft_dispWin.x1) / 2;
    y = (tft_dispWin.y2 - tft_dispWin.y1) / 2;
    if (x < y)
        len = x - 8;
    else
        len = y - 8;

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        for (angle = 0; angle < 360; angle++) {
            TFT_drawLineByAngle(x, y, 0, len, angle, random_color());
            n++;
        }
    }

    TFT_fillWindow(TFT_BLACK);
    end_time = clock() + GDEMO_TIME;
    while ((clock() < end_time) && (Wait(0))) {
        for (angle = 0; angle < 360; angle++) {
            TFT_drawLineByAngle(x, y, len / 4, len / 4, angle, random_color());
            n++;
        }
        for (angle = 0; angle < 360; angle++) {
            TFT_drawLineByAngle(x, y, len * 3 / 4, len / 4, angle, random_color());
            n++;
        }
    }
    sprintf(tmp_buff, "%d LINES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//--------------------
static void arc_demo() {
    uint16_t x, y, r, th, n, i;
    float start, end;
    color_t color, fillcolor;

    disp_header("ARC DEMO");

    x = (tft_dispWin.x2 - tft_dispWin.x1) / 2;
    y = (tft_dispWin.y2 - tft_dispWin.y1) / 2;

    th = 6;
    uint32_t end_time = clock() + GDEMO_TIME;
    i = 0;
    while ((clock() < end_time) && (Wait(0))) {
        if (x < y)
            r = x - 4;
        else
            r = y - 4;
        start = 0;
        end = 20;
        n = 1;
        while (r > 10) {
            color = random_color();
            TFT_drawArc(x, y, r, th, start, end, color, color);
            r -= (th + 2);
            n++;
            start += 30;
            end = start + (n * 20);
            i++;
        }
    }
    sprintf(tmp_buff, "%d ARCS", i);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("OUTLINED ARC", "");
    TFT_fillWindow(TFT_BLACK);
    th = 8;
    end_time = clock() + GDEMO_TIME;
    i = 0;
    while ((clock() < end_time) && (Wait(0))) {
        if (x < y)
            r = x - 4;
        else
            r = y - 4;
        start = 0;
        end = 350;
        n = 1;
        while (r > 10) {
            color = random_color();
            fillcolor = random_color();
            TFT_drawArc(x, y, r, th, start, end, color, fillcolor);
            r -= (th + 2);
            n++;
            start += 20;
            end -= n * 10;
            i++;
        }
    }
    sprintf(tmp_buff, "%d ARCS", i);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//-----------------------
static void circle_demo() {
    int x, y, r, n;

    disp_header("CIRCLE DEMO");

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(8, tft_dispWin.x2 - 8);
        y = rand_interval(8, tft_dispWin.y2 - 8);
        if (x < y)
            r = rand_interval(2, x / 2);
        else
            r = rand_interval(2, y / 2);
        TFT_drawCircle(x, y, r, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d CIRCLES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("FILLED CIRCLE", "");
    TFT_fillWindow(TFT_BLACK);
    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(8, tft_dispWin.x2 - 8);
        y = rand_interval(8, tft_dispWin.y2 - 8);
        if (x < y)
            r = rand_interval(2, x / 2);
        else
            r = rand_interval(2, y / 2);
        TFT_fillCircle(x, y, r, random_color());
        TFT_drawCircle(x, y, r, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d CIRCLES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//------------------------
static void ellipse_demo() {
    int x, y, rx, ry, n;

    disp_header("ELLIPSE DEMO");

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(8, tft_dispWin.x2 - 8);
        y = rand_interval(8, tft_dispWin.y2 - 8);
        if (x < y)
            rx = rand_interval(2, x / 4);
        else
            rx = rand_interval(2, y / 4);
        if (x < y)
            ry = rand_interval(2, x / 4);
        else
            ry = rand_interval(2, y / 4);
        TFT_drawEllipse(x, y, rx, ry, random_color(), 15);
        n++;
    }
    sprintf(tmp_buff, "%d ELLIPSES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("FILLED ELLIPSE", "");
    TFT_fillWindow(TFT_BLACK);
    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(8, tft_dispWin.x2 - 8);
        y = rand_interval(8, tft_dispWin.y2 - 8);
        if (x < y)
            rx = rand_interval(2, x / 4);
        else
            rx = rand_interval(2, y / 4);
        if (x < y)
            ry = rand_interval(2, x / 4);
        else
            ry = rand_interval(2, y / 4);
        TFT_fillEllipse(x, y, rx, ry, random_color(), 15);
        TFT_drawEllipse(x, y, rx, ry, random_color(), 15);
        n++;
    }
    sprintf(tmp_buff, "%d ELLIPSES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("ELLIPSE SEGMENTS", "");
    TFT_fillWindow(TFT_BLACK);
    end_time = clock() + GDEMO_TIME;
    n = 0;
    int k = 1;
    while ((clock() < end_time) && (Wait(0))) {
        x = rand_interval(8, tft_dispWin.x2 - 8);
        y = rand_interval(8, tft_dispWin.y2 - 8);
        if (x < y)
            rx = rand_interval(2, x / 4);
        else
            rx = rand_interval(2, y / 4);
        if (x < y)
            ry = rand_interval(2, x / 4);
        else
            ry = rand_interval(2, y / 4);
        TFT_fillEllipse(x, y, rx, ry, random_color(), (1 << k));
        TFT_drawEllipse(x, y, rx, ry, random_color(), (1 << k));
        k = (k + 1) & 3;
        n++;
    }
    sprintf(tmp_buff, "%d SEGMENTS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//-------------------------
static void triangle_demo() {
    int x1, y1, x2, y2, x3, y3, n;

    disp_header("TRIANGLE DEMO");

    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x1 = rand_interval(4, tft_dispWin.x2 - 4);
        y1 = rand_interval(4, tft_dispWin.y2 - 2);
        x2 = rand_interval(4, tft_dispWin.x2 - 4);
        y2 = rand_interval(4, tft_dispWin.y2 - 2);
        x3 = rand_interval(4, tft_dispWin.x2 - 4);
        y3 = rand_interval(4, tft_dispWin.y2 - 2);
        TFT_drawTriangle(x1, y1, x2, y2, x3, y3, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d TRIANGLES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("FILLED TRIANGLE", "");
    TFT_fillWindow(TFT_BLACK);
    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        x1 = rand_interval(4, tft_dispWin.x2 - 4);
        y1 = rand_interval(4, tft_dispWin.y2 - 2);
        x2 = rand_interval(4, tft_dispWin.x2 - 4);
        y2 = rand_interval(4, tft_dispWin.y2 - 2);
        x3 = rand_interval(4, tft_dispWin.x2 - 4);
        y3 = rand_interval(4, tft_dispWin.y2 - 2);
        TFT_fillTriangle(x1, y1, x2, y2, x3, y3, random_color());
        TFT_drawTriangle(x1, y1, x2, y2, x3, y3, random_color());
        n++;
    }
    sprintf(tmp_buff, "%d TRIANGLES", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

//---------------------
static void poly_demo() {
    uint16_t x, y, rot, oldrot;
    int i, n, r;
    uint8_t sides[6] = {3, 4, 5, 6, 8, 10};
    color_t color[6] = {TFT_WHITE, TFT_CYAN, TFT_RED, TFT_BLUE, TFT_YELLOW, TFT_ORANGE};
    color_t fill[6] = {TFT_BLUE, TFT_NAVY, TFT_DARKGREEN, TFT_DARKGREY, TFT_LIGHTGREY, TFT_OLIVE};

    disp_header("POLYGON DEMO");

    x = (tft_dispWin.x2 - tft_dispWin.x1) / 2;
    y = (tft_dispWin.y2 - tft_dispWin.y1) / 2;

    rot = 0;
    oldrot = 0;
    uint32_t end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        if (x < y)
            r = x - 4;
        else
            r = y - 4;
        for (i = 5; i >= 0; i--) {
            TFT_drawPolygon(x, y, sides[i], r, TFT_BLACK, TFT_BLACK, oldrot, 1);
            TFT_drawPolygon(x, y, sides[i], r, color[i], color[i], rot, 1);
            r -= 16;
            if (r <= 0) {
                break;
            };
            n += 2;
        }
        Wait(100);
        oldrot = rot;
        rot = (rot + 15) % 360;
    }
    sprintf(tmp_buff, "%d POLYGONS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);

    update_header("FILLED POLYGON", "");
    rot = 0;
    end_time = clock() + GDEMO_TIME;
    n = 0;
    while ((clock() < end_time) && (Wait(0))) {
        if (x < y)
            r = x - 4;
        else
            r = y - 4;
        TFT_fillWindow(TFT_BLACK);
        for (i = 5; i >= 0; i--) {
            TFT_drawPolygon(x, y, sides[i], r, color[i], fill[i], rot, 2);
            r -= 16;
            if (r <= 0) {
                break;
            }
            n += 2;
        }
        Wait(500);
        rot = (rot + 15) % 360;
    }
    sprintf(tmp_buff, "%d POLYGONS", n);
    update_header(NULL, tmp_buff);
    Wait(-GDEMO_INFO_TIME);
}

void refreshDisplay() {
    poly_demo();
    aline_demo();
    arc_demo();
    circle_demo();
    ellipse_demo();
    font_demo();
    line_demo();
    pixel_demo();
    rect_demo();
    triangle_demo();
}

void initDisplay() {
}
