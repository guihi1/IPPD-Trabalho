#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RGB_COMPONENT_COLOR 255
#define DIVS 8

typedef struct {
  unsigned char red, green, blue;
} PPMPixel;

typedef struct {
  int x, y;
  PPMPixel *data;
} PPMImage;

static PPMImage *readPPM(const char *filename) {
  char buff[16];
  PPMImage *img;
  FILE *fp;
  int c, rgb_comp_color;

  // Open file in binary mode
  fp = fopen(filename, "rb");
  if (!fp) {
    perror("Cannot open file");
    exit(1);
  }

  if (!fgets(buff, sizeof(buff), fp)) {
    perror("File read error");
    exit(1);
  }

  if (buff[0] != 'P' || buff[1] != '6') {
    fprintf(stderr, "Invalid image format (must be 'P6')\n");
    exit(1);
  }

  img = (PPMImage *)malloc(sizeof(PPMImage));
  if (!img) {
    fprintf(stderr, "Unable to allocate memory\n");
    exit(1);
  }

  c = getc(fp);
  while (c == '#') { // Skip comments
    while (getc(fp) != '\n')
      ;
    c = getc(fp);
  }

  ungetc(c, fp);
  if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
    fprintf(stderr, "Invalid image size (error loading)\n");
    exit(1);
  }

  if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
    fprintf(stderr, "Invalid rgb component (error loading)\n");
    exit(1);
  }

  if (rgb_comp_color != RGB_COMPONENT_COLOR) {
    fprintf(stderr, "Image does not have 8-bits components\n");
    exit(1);
  }

  while (fgetc(fp) != '\n')
    ;
  img->data = (PPMPixel *)malloc(img->x * img->y * sizeof(PPMPixel));
  if (!img->data) {
    fprintf(stderr, "Unable to allocate memory\n");
    exit(1);
  }

  if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
    fprintf(stderr, "Error loading image.\n");
    exit(1);
  }

  fclose(fp);
  return img;
}

void Histogram(PPMImage *image, float *h) {

  int i = 0, j, k, l, count;
  int rows, cols;

  long n = image->y * image->x;

  cols = image->x;
  rows = image->y;

#pragma omp parallel for
  for (i = 0; i < n; i++) {
    image->data[i].red = floor((image->data[i].red * DIVS) / 256);
    image->data[i].blue = floor((image->data[i].blue * DIVS) / 256);
    image->data[i].green = floor((image->data[i].green * DIVS) / 256);
  }

  count = 0;
// #pragma omp parallel for collapse(3)
#pragma omp parallel for
  for (j = 0; j <= DIVS - 1; j++) {
    for (k = 0; k <= DIVS - 1; k++) {
      for (l = 0; l <= DIVS - 1; l++) {
        count = 0;
        for (i = 0; i < n; i++) {
          if (image->data[i].red == j && image->data[i].green == k &&
              image->data[i].blue == l) {
            count++;
          }
        }
        h[DIVS * DIVS * j + DIVS * k + l] = (float)count / n;
      }
    }
  }
}

int main(int argc, char *argv[]) {

  int i;

  PPMImage *image = readPPM(argv[1]);

  float *h = (float *)malloc(sizeof(float) * DIVS * DIVS * DIVS);

  for (i = 0; i < DIVS * DIVS * DIVS; i++)
    h[i] = 0.0;

  clock_t inicio = clock();
  Histogram(image, h);
  clock_t fim = clock();

  double tempo_execucao = (double)(fim - inicio) / CLOCKS_PER_SEC;
  printf("Tempo de execução: %f segundos\n", tempo_execucao);

  for (i = 0; i < DIVS * DIVS * DIVS; i++) {
    printf("%0.3f ", h[i]);
  }
  printf("\n");
  free(h);

  return 0;
}
