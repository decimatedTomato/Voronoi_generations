#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <math.h>

#define WIDTH 800 * 4
#define HEIGHT 400 * 4
#define SEED_COUNT 3
#define SEED_RADIUS 5

#define OUTPUT_FILE_PATH "output/output4.ppm"

#define COLOR_RED 0xFF0000FF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFFFF0000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000

#define COLOR_EMMA 0xFF33FF18
#define BACKGROUND_COLOR 0x181818

#define GRUVBOX_BRIGHT_RED 0xFF3449FB
#define GRUVBOX_BRIGHT_GREEN 0xFF26BBB8
#define GRUVBOX_BRIGHT_YELLOW 0xFF2FBDFA
#define GRUVBOX_BRIGHT_BLUE 0xFF98A583
#define GRUVBOX_BRIGHT_PURPLE 0xFF9B86D3
#define GRUVBOX_BRIGHT_AQUA 0xFF7CC08E
#define GRUVBOX_BRIGHT_ORANGE 0xFF1980FE

typedef uint32_t Color32;

typedef struct {
    int x, y;
} Point;

static Color32 image[HEIGHT][WIDTH];
static Point seeds[SEED_COUNT];

static Color32 palette_peach[] = {
    GRUVBOX_BRIGHT_RED,
    GRUVBOX_BRIGHT_GREEN,
    GRUVBOX_BRIGHT_YELLOW,
    GRUVBOX_BRIGHT_BLUE,
    GRUVBOX_BRIGHT_PURPLE,
    GRUVBOX_BRIGHT_AQUA,
    GRUVBOX_BRIGHT_ORANGE,
};
#define PALETTE_COUNT_PEACH (sizeof(palette_peach)/sizeof(palette_peach[0]))
// static Color32 palette_boring[] = {
//     COLOR_RED,
//     COLOR_GREEN,
//     COLOR_BLUE,
// };
// #define PALETTE_COUNT_BORING (sizeof(palette_boring)/sizeof(palette_boring[0]))
// static Color32 palette_grayscale[] = {
//     COLOR_BLACK,
//     COLOR_WHITE,
// };
// #define PALETTE_COUNT_GRAYSCALE (sizeof(palette_grayscale)/sizeof(palette_grayscale[0]))

void fill_image(Color32 color) {
    for(size_t y = 0; y < HEIGHT; y++) {
        for(size_t x = 0; x < WIDTH; x++) {
            image[y][x] = color;
        }
    }
}

void save_image_as_ppm(const char* file_path) {
    FILE* f = fopen(file_path, "wb");
    if(f == NULL) {
        fprintf(stderr, "ERROR: could not write into file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT);
    for(size_t y = 0; y < HEIGHT; y++) {
        for(size_t x = 0; x < WIDTH; x++) {
            uint32_t pixel = image[y][x];
            uint8_t bytes[3] = {
                (pixel & 0x0000FF) >> 0,
                (pixel & 0x00FF00) >> 8,
                (pixel & 0xFF0000) >> 16,
            };
            fwrite(bytes, sizeof(bytes), 1, f);
        }
    }
    fclose(f);
}

void generate_random_seeds() {
    for (size_t i = 0; i < SEED_COUNT; i++) {
        seeds[i].x = rand() % WIDTH;
        seeds[i].y = rand() % HEIGHT;
    }
}

int sqrt_dist(int x1, int x2, int y1, int y2) {
    int dx = x1 - x2;
    int dy = y1 - y2;
    return dx*dx + dy*dy;
}

void fill_circle(int cx, int cy, int r, Color32 color) {
    int x0 = cx - r;
    int y0 = cy - r;
    int x1 = cx + r;
    int y1 = cy + r;
    for (int x = x0; x < x1; x++) {
        if(x < 0 || x >= WIDTH) continue;
        for (int y = y0; y < y1; y++) {
            if(y < 0 || y >= HEIGHT) continue;
            if(sqrt_dist(cx, x, cy, y) <= r*r) {
                image[y][x] = color;
            }
        }   
    }
}

void render_seeds(Color32 seed_color) {
    for(size_t i = 0; i < SEED_COUNT; i++) {
        fill_circle(seeds[i].x, seeds[i].y, SEED_RADIUS, seed_color);
    }
}

typedef struct Neighbor {
    int pixel_distance;
    int seed_index;
} Neighbor;

// This does not quite work the way I wanted, it frequently overflows
Color32 average_weighted_color(Neighbor* seed_infos, size_t color_count, Color32* palette, size_t palette_length) {
    double r = 0, g = 0, b = 0;
    double total_distance = 0;
    for(size_t c = 0; c < color_count; c++) {
        total_distance += seed_infos[c].pixel_distance * seed_infos[c].pixel_distance;
    }
    for(size_t c = 0; c < color_count; c++) {
        r += ((palette[seed_infos[c].seed_index % palette_length] & 0x0000FF) >> 0)  * (seed_infos[c].pixel_distance * seed_infos[c].pixel_distance / total_distance);
        g += ((palette[seed_infos[c].seed_index % palette_length] & 0x00FF00) >> 8) * (seed_infos[c].pixel_distance * seed_infos[c].pixel_distance / total_distance);
        b += ((palette[seed_infos[c].seed_index % palette_length] & 0xFF0000) >> 16) * (seed_infos[c].pixel_distance * seed_infos[c].pixel_distance / total_distance);
    }
    unsigned int R = (r > 255) ? 255 : (unsigned int) r;
    unsigned int G = (g > 255) ? 255 : (unsigned int) g;
    unsigned int B = (b > 255) ? 255 : (unsigned int) b;
    return (B << 16) + (G << 8) + (R << 0);
}

void render_voronoi(Color32* palette, size_t palette_length) {
    Neighbor seed_info[SEED_COUNT];

    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            for(size_t i = 0; i < SEED_COUNT; i++) {
                Neighbor n = { .pixel_distance=sqrt_dist(seeds[i].x, x, seeds[i].y, y), .seed_index=i};
                seed_info[i] = n;
            }
            image[y][x] = average_weighted_color(seed_info, SEED_COUNT, palette, palette_length);
        }
    }
}

int main() {
    srand(time(0));
    generate_random_seeds();
    fill_image(BACKGROUND_COLOR);
    // render_voronoi(palette_boring, PALETTE_COUNT_BORING);
    render_voronoi(palette_peach, PALETTE_COUNT_PEACH);
    // render_voronoi(palette_grayscale, PALETTE_COUNT_GRAYSCALE);
    // render_seeds(COLOR_BLACK);
    save_image_as_ppm(OUTPUT_FILE_PATH);
    return 0;
}
