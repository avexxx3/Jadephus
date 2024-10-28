#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <cmath>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

struct Soldier {
  float x;      // X position of sprite
  float y;      // Y position of sprite
  int rotation; // Rotation angle in degrees
  int targetRotation;

  int originalRotation;
  int originalX, originalY;

  int stage = 0;
  // Stage = 0 : Standing still
  // Stage = 1 : Moving to center
  // Stage = 2 : Rotating to target
  // Stage = 3 : Rotating to -1 * original
  // Stage = 4 : Moving to original
  // Stage = 5 : Rotating to original

  int idleCounter = 0;
  int counter = 0;

  bool ALIVE = true;
  bool josephus = false;

  void updateRotation() {
    if (stage == 0 || stage == 1 || stage == 4) {
      return;
    }

    if (stage == 3) {
      if (counter++ < 20)
        return;
    }

    if (rotation == targetRotation) {
      stage = (stage + 1) % 6;
      return;
    }

    if (rotation == targetRotation + 1 || rotation == targetRotation - 1) {
      rotation = targetRotation;
      return;
    }

    if (rotation > targetRotation)
      rotation -= 2;
    else
      rotation += 2;
  }

  void moveSoldierToPosition() {
    if (stage == 1)
      moveSoldierToCenter();
    else if (stage == 4)
      moveSoldierToCenter(originalX, originalY);
  }

  void moveSoldierToCenter(float targetX = 0, float targetY = 0,
                           float speed = 2) {
    if (x == targetX && y == targetY) {
      if (stage == 1)
        stage = 2;
      else {
        stage = 5;
        targetRotation = (180 + rotation) % 360;
      }
    }

    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < speed) {
      x = targetX;
      y = targetY;
    } else {
      // Move the soldier towards the target
      x += (dx / distance) * speed;
      y += (dy / distance) * speed;
    }
  }

  void rotateToOriginal() {
    if (stage != 3)
      return;
    targetRotation = originalRotation;
  }

  void rotateSpriteToFace(const Soldier &targetSoldier) {
    if (stage != 1)
      return;

    float dx = targetSoldier.x - x;
    float dy = targetSoldier.y - y;

    float angle = atan2(dy, dx) * (180.0 / M_PI);

    if (angle < 0)
      angle += 360;

    targetRotation = angle;
  }

  void murder() {
    if (stage != 0)
      return;
    counter = 0;
    originalRotation = (180 + rotation) % 360;
    stage = 1;
    originalX = x;
    originalY = y;
  }
};

std::vector<Soldier> arrangeSpritesInCircle(int N, float radius,
                                            float padding) {
  std::vector<Soldier> sprites(N);
  float angleIncrement = (2 * M_PI) / N;

  for (int i = 0; i < N; ++i) {
    float angle = i * angleIncrement;
    sprites[i].x = radius * cos(angle - M_PI / 2);
    sprites[i].y = radius * sin(angle - M_PI / 2);

    sprites[i].rotation = sprites[i].targetRotation =
        int(angle * (180.0 / M_PI) + 90.0) % 360; // Face center
    sprites[i].idleCounter = i % 20;
  }

  return sprites;
}

std::vector<SDL_Texture *> getIdleTextures(SDL_Renderer *renderer,
                                           bool josephus = false) {
  std::vector<SDL_Texture *> textures;
  for (int i = 0; i < 20; i++) {
    std::string path = "res/idle/" + std::to_string(i) + ".png";
    SDL_Texture *texture = IMG_LoadTexture(renderer, path.c_str());
    textures.emplace_back(texture);
    if (josephus)
      SDL_SetTextureColorMod(texture, 174, 241, 140);
  }
  return textures;
}

std::vector<SDL_Texture *> getReloadTextures(SDL_Renderer *renderer,
                                             bool josephus = false) {
  std::vector<SDL_Texture *> textures;
  for (int i = 0; i < 20; i++) {
    std::string path = "res/reload/" + std::to_string(i) + ".png";
    SDL_Texture *texture = IMG_LoadTexture(renderer, path.c_str());
    textures.emplace_back(texture);
    if (josephus)
      SDL_SetTextureColorMod(texture, 174, 241, 140);
  }
  return textures;
}

std::vector<SDL_Texture *> getSuicideTextures(SDL_Renderer *renderer) {
  std::vector<SDL_Texture *> textures;
  for (int i = 0; i < 20; i++) {
    std::string path =
        "res/seppuku/survivor-meleeattack_knife_" + std::to_string(i) + ".png";
    SDL_Texture *texture = IMG_LoadTexture(renderer, path.c_str());
    textures.emplace_back(texture);
  }
  return textures;
}

std::vector<SDL_Texture *> getMoveTextures(SDL_Renderer *renderer,
                                           bool josephus = false) {
  std::vector<SDL_Texture *> textures;
  for (int i = 0; i < 20; i++) {
    std::string path = "res/move/" + std::to_string(i) + ".png";
    SDL_Texture *texture = IMG_LoadTexture(renderer, path.c_str());
    textures.emplace_back(texture);
    if (josephus)
      SDL_SetTextureColorMod(texture, 174, 241, 140);
  }
  return textures;
}

int Josephus(int n, int k) {
  if (n == 1)
    return 1;
  return (Josephus(n - 1, k) + k - 1) % n + 1;
}

int VEL = 50;

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return -1;
  }

  int N = 41; // Number of sprites
  if (argc > 1) {
    N = std::atoi(argv[1]);
    if (N <= 1) {
      std::cerr << "Invalid number of soldiers. Using default value of 41."
                << std::endl;
      N = 41;
    }
  }

  int k = 2;
  if (argc > 2) {
    k = std::atoi(argv[2]);
    if (k <= 1 || k > N) {
      std::cerr << "Invalid K. Using default value of 2." << std::endl;
      k = 2;
    }
  }

  int movingSoldier = 0;
  int targetSoldier = k - 1;

  int josephus = Josephus(N, k);

  float spriteWidth = 289.0 / (N >= 16 ? N / 7 : 2);
  float spriteHeight = 224.0 / (N >= 16 ? N / 7 : 2);

  std::queue<int> josephusQueue;
  for (int i = 0; i < N; i++)
    josephusQueue.push(i);

  // Retrieve display dimensions
  SDL_DisplayMode displayMode;
  SDL_GetCurrentDisplayMode(0, &displayMode);
  int minDimension = std::min(displayMode.w, displayMode.h);
  int screenWidth = static_cast<int>(minDimension * 0.9);
  int screenHeight = screenWidth;

  // Adjust window size based on screen dimension
  SDL_Window *window = SDL_CreateWindow(
      "Circle of Sprites", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      screenWidth, screenHeight, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    return -1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  std::vector<SDL_Texture *> idleTextures = getIdleTextures(renderer);
  std::vector<SDL_Texture *> movingTextures = getMoveTextures(renderer);
  std::vector<SDL_Texture *> reloadTextures = getReloadTextures(renderer);
  std::vector<SDL_Texture *> suicideTextures = getSuicideTextures(renderer);

  std::vector<SDL_Texture *> idleJosTextures = getIdleTextures(renderer, true);
  std::vector<SDL_Texture *> movingJosTextures =
      getMoveTextures(renderer, true);
  std::vector<SDL_Texture *> reloadJosTextures =
      getReloadTextures(renderer, true);

  SDL_Texture *deadTexture = IMG_LoadTexture(renderer, "res/dead.png");
  SDL_SetTextureColorMod(deadTexture, 255, 0, 0);

  float maxRadius = (screenWidth / 2.0f) - spriteWidth / 2.0f;
  float initialRadius = (N * spriteWidth) / (2 * M_PI);
  float paddingFactor = 0.1; // Base padding factor
  float radius = std::min(initialRadius, maxRadius);
  float angularPadding = (paddingFactor * (M_PI / N));

  auto soldiers = arrangeSpritesInCircle(N, radius, 0);

  bool quit = false;
  SDL_Event event;

  soldiers[josephus - 1].josephus = true;

  float movementSpeed = 2.0f; // Speed of movement

  unsigned int currentTime = SDL_GetTicks64();
  unsigned int lastTime = currentTime;
  int counter = 0;
  bool next = false;
  int numberKilled = 0;

  soldiers[0].murder();

  int flag = 0;

  targetSoldier = josephusQueue.front();
  while (flag <= k - 1) {
    targetSoldier = josephusQueue.front();
    josephusQueue.pop();
    if (flag != k - 1)
      josephusQueue.push(targetSoldier);
    flag++;
  }

  while (!quit) {
    currentTime = SDL_GetTicks64();
    if (currentTime < lastTime + (500.0 / VEL))
      continue;

    lastTime = currentTime;
    counter = (counter + 1) % 20;

    // Handle events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        quit = true;

      if (event.type == SDL_KEYDOWN)
        switch (event.key.keysym.sym) {
        case SDLK_1:
          VEL = VEL * 1.1;
          break;
        case SDLK_2:
          VEL *= 0.9;
          break;
        }
    }

    if (josephusQueue.size() < 1)
      continue;

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    if (soldiers[movingSoldier].stage == 0 && !next) {
      next = true;
    } else if (soldiers[movingSoldier].stage == 0 && next) {
      movingSoldier = targetSoldier;
      next = false;

      int flag = 0;
      while (flag <= k - 1) {
        targetSoldier = josephusQueue.front();
        josephusQueue.pop();
        if (flag != k - 1)
          josephusQueue.push(targetSoldier);
        flag++;
      }

      while (true) {
        movingSoldier = (movingSoldier + 1) % N;
        if (soldiers[movingSoldier].ALIVE)
          break;
      }

      soldiers[movingSoldier].murder();
    }

    if (targetSoldier != movingSoldier) {
      soldiers[movingSoldier].moveSoldierToPosition();
      soldiers[movingSoldier].rotateSpriteToFace(soldiers[targetSoldier]);
      soldiers[movingSoldier].rotateToOriginal();
      soldiers[movingSoldier].updateRotation();
    } else {
      if (soldiers[movingSoldier].counter >= 19) {
        soldiers[movingSoldier].stage = 0;
        soldiers[movingSoldier].ALIVE = false;
        continue;
      }
    }

    if (soldiers[movingSoldier].stage == 3)
      soldiers[targetSoldier].ALIVE = false;

    // Render each sprite within the viewport
    for (auto &soldier : soldiers) {
      SDL_Rect destRect;
      destRect.w = static_cast<int>(spriteWidth);
      destRect.h = static_cast<int>(spriteHeight);

      // Center the circle within the viewport
      destRect.x = screenWidth / 2.0 + soldier.x - spriteWidth / 2;
      destRect.y = screenHeight / 2.0 + soldier.y - spriteHeight / 2;

      SDL_Texture *texture;

      if (soldier.idleCounter++ == 19)
        soldier.idleCounter = 0;

      int counter = soldier.idleCounter;

      if (targetSoldier == movingSoldier &&
          soldiers[movingSoldier].rotation == soldier.rotation &&
          !soldiers[movingSoldier].josephus) {
        texture = suicideTextures[soldier.counter++];
      } else if (!soldier.ALIVE)
        texture = deadTexture;
      else if (soldier.stage == 1 || soldier.stage == 4) {
        texture = soldier.josephus ? movingJosTextures[counter]
                                   : movingTextures[counter];
      } else if (soldier.counter > 0 && soldier.counter < 20) {
        texture = soldier.josephus ? reloadJosTextures[soldier.counter]
                                   : reloadTextures[soldier.counter];
      } else {
        texture =
            soldier.josephus ? idleJosTextures[counter] : idleTextures[counter];
      }

      SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, soldier.rotation,
                       nullptr, SDL_FLIP_NONE);
    }

    // Present the updated screen
    SDL_RenderPresent(renderer);
  }

  // Clean ups
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
