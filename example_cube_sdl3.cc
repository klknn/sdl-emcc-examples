#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <vector>

struct Point {
  double x = 0, y = 0, z = 0;
};

typedef std::vector<double> Vector;
typedef std::vector<Vector> Matrix;

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
  SDL_RenderLine(renderer, points[i].x, points[i].y, points[j].x, points[j].y);
}

Matrix getRotationMatrix() {
  double alpha = 0.001;
  Matrix rotationX = {{1, 0, 0},
                      {0, SDL_cos(alpha), -SDL_sin(alpha)},
                      {0, SDL_sin(alpha), SDL_cos(alpha)}};

  double beta = 0.002;
  Matrix rotationY = {{SDL_cos(beta), 0, SDL_sin(beta)},
                      {0, 1, 0},
                      {-SDL_sin(beta), 0, SDL_cos(beta)}};

  double gamma = 0.003;
  Matrix rotationZ = {{SDL_cos(gamma), -SDL_sin(gamma), 0},
                      {SDL_sin(gamma), SDL_cos(gamma), 0},
                      {0, 0, 1}};

  return dot(rotationZ, dot(rotationY, rotationX));
}

int WIDTH = 1000;
int HEIGHT = 1000;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
std::vector<Point> points = {{-1, 1, 1},  {1, 1, 1},   {1, -1, 1},
                             {-1, -1, 1}, {-1, 1, -1}, {1, 1, -1},
                             {1, -1, -1}, {-1, -1, -1}};

Point screenShift{WIDTH / 2.0, HEIGHT / 2.0};
Point screenShiftOpposite{-WIDTH / 2.0, -HEIGHT / 2.0};
Matrix rotationXYZ = getRotationMatrix();

SDL_AppResult SDL_AppIterate(void *appstate) {
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
  SDL_Delay(3);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  default:
    return SDL_APP_CONTINUE;
  }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  SDL_Log("AppInit");
  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("error initializing SDL: %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (SDL_CreateWindowAndRenderer("GAME", WIDTH, HEIGHT,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
                                  &window, &renderer) == false) {
    SDL_Log("error creating SDL window and renderer: %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  // https://wiki.libsdl.org/SDL3/README-emscripten#rendering
  SDL_SetRenderVSync(renderer, 1);

  int scale = 200;
  for (Point &p : points) {
    p.x = (scale * p.x + screenShift.x);
    p.y = (scale * p.y + screenShift.y);
    p.z = (scale * p.z + screenShift.z);
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_DestroyWindow(window);
}
