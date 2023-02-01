#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 400
#define SEED_COUNT 10
#define SEED_RADIUS 5
#define NEAREST_SEED_COUNT 2

#define OUTPUT_FILE_PATH "output2.ppm"

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
static Color32 palette[] = {
    GRUVBOX_BRIGHT_RED,
    GRUVBOX_BRIGHT_GREEN,
    GRUVBOX_BRIGHT_YELLOW,
    GRUVBOX_BRIGHT_BLUE,
    GRUVBOX_BRIGHT_PURPLE,
    GRUVBOX_BRIGHT_AQUA,
    GRUVBOX_BRIGHT_ORANGE,
};
#define PALETTE_COUNT (sizeof(palette)/sizeof(palette[0]))

void fill_image(Color32 color) {
    // 0xAABBGGRR
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

Color32 average_color(Neighbor* neighbors, size_t color_count) {
    unsigned int r = 0, g = 0, b = 0;
    for(size_t c = 0; c < color_count; c++) {
        r += (palette[neighbors[c].seed_index % PALETTE_COUNT] & 0x0000FF) >> 0;
        g += (palette[neighbors[c].seed_index % PALETTE_COUNT] & 0x00FF00) >> 8;
        b += (palette[neighbors[c].seed_index % PALETTE_COUNT] & 0xFF0000) >> 16;
    }
    r /= color_count;
    g /= color_count;
    b /= color_count;
    return (b << 16) + (g << 8) + (r << 0);
}

void sort_lowest_distance_neighbors(Neighbor* neighbors, size_t neighbor_count) {
    int min_distance_index;
    for (size_t i = 0; i < neighbor_count - 1; i++) {
        min_distance_index = i;
        for (size_t j = i+1; j < neighbor_count; j++)
            if (neighbors[j].pixel_distance < neighbors[min_distance_index].pixel_distance)
                min_distance_index = j;
        Neighbor temp = neighbors[i];
        neighbors[i] = neighbors[min_distance_index];
        neighbors[min_distance_index] = temp;
    }
}

void render_voronoi(int nearest_count) {
    nearest_count = (nearest_count < SEED_COUNT) ? nearest_count : SEED_COUNT; 
    nearest_count = (nearest_count > 1) ? nearest_count : 1; 
    Neighbor nearest[SEED_COUNT]; // I will sort the neighbors and average the top 'x' colors for each pixel

    for(int y = 0; y < HEIGHT; y++) {
        for(int x = 0; x < WIDTH; x++) {
            for(size_t i = 0; i < SEED_COUNT; i++) {
                Neighbor n = { .pixel_distance=sqrt_dist(seeds[i].x, x, seeds[i].y, y), .seed_index=i};
                nearest[i] = n;
            }
            sort_lowest_distance_neighbors(nearest, SEED_COUNT);
            image[y][x] = average_color(nearest, nearest_count);
        }
    }
}

int main() {
    srand(time(0));
    generate_random_seeds();
    fill_image(BACKGROUND_COLOR);
    render_voronoi(2);
    render_seeds(COLOR_BLACK);
    save_image_as_ppm(OUTPUT_FILE_PATH);

    return 0;
}
