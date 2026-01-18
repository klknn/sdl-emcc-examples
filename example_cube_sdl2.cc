// https://gist.github.com/omaraflak/165f6583807c5f7cb16559777b6c7ed1
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

struct Point {
  double x = 0, y = 0, z = 0;
};

using Vector = std::vector<double>;
using Matrix = std::vector<Vector>;

Matrix dot(const Matrix &a, const Matrix &b) {
  Matrix result = Matrix(a.size(), Vector(b[0].size(), 0));
  for (int i = 0; i < a.size(); i++) {
    for (int j = 0; j < b[0].size(); j++) {
      for (int k = 0; k < b.size(); k++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
  return result;
}

Point transform(const Matrix &matrix, const Point &point) {
  Matrix p = {{point.x}, {point.y}, {point.z}};
  Matrix r = dot(matrix, p);
  return {r[0][0], r[1][0], r[2][0]};
}

Point translate(const Point &shift, const Point &point) {
  return {point.x + shift.x, point.y + shift.y, point.z + shift.z};
}

void connect(SDL_Renderer *const renderer, const std::vector<Point> &points,
             int i, int j) {
  SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[j].x,
                     points[j].y);
}

Matrix getRotationMatrix() {
  double alpha = 0.001;
  Matrix rotationX = {
      {1, 0, 0}, {0, cos(alpha), -sin(alpha)}, {0, sin(alpha), cos(alpha)}};

  double beta = 0.002;
  Matrix rotationY = {
      {cos(beta), 0, sin(beta)}, {0, 1, 0}, {-sin(beta), 0, cos(beta)}};

  double gamma = 0.003;
  Matrix rotationZ = {
      {cos(gamma), -sin(gamma), 0}, {sin(gamma), cos(gamma), 0}, {0, 0, 1}};

  return dot(rotationZ, dot(rotationY, rotationX));
}

int WIDTH = 1000;
int HEIGHT = 1000;
bool close = false;
std::vector<Point> points;
Point screenShift{WIDTH / 2.0, HEIGHT / 2.0};
Point screenShiftOpposite{-WIDTH / 2.0, -HEIGHT / 2.0};
Matrix rotationXYZ = getRotationMatrix();
SDL_Renderer *renderer;

void mainloop() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      close = true;
    }
  }

  for (Point &p : points) {
    p = translate(screenShiftOpposite, p);
    p = transform(rotationXYZ, p);
    p = translate(screenShift, p);
  }

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  for (int i = 0; i < 4; i++) {
    connect(renderer, points, i, (i + 1) % 4);
    connect(renderer, points, i + 4, ((i + 1) % 4) + 4);
    connect(renderer, points, i, i + 4);
  }
  SDL_RenderPresent(renderer);
  // SDL_Delay(3);
}

int main() {
  // Initialize.
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("error initializing SDL: %s\n", SDL_GetError());
  }

  SDL_Window *window = SDL_CreateWindow(
      "GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  points = {{-1, 1, 1},  {1, 1, 1},  {1, -1, 1},  {-1, -1, 1},
            {-1, 1, -1}, {1, 1, -1}, {1, -1, -1}, {-1, -1, -1}};

  int scale = 200;

  for (Point &p : points) {
    p.x = (scale * p.x + screenShift.x);
    p.y = (scale * p.y + screenShift.y);
    p.z = (scale * p.z + screenShift.z);
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(mainloop, 0, 1);
#else
  while (!close) {
    mainloop();
  }
#endif

  SDL_DestroyWindow(window);
  SDL_Quit();
}
