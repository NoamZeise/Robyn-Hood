#ifndef APP_H
#define APP_H

#include <atomic>
#include <iostream>
#include <thread>

#ifdef GFX_ENV_VULKAN
  #ifndef GLFW_INCLUDE_VULKAN
  #define GLFW_INCLUDE_VULKAN
  #endif
  #ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
  #define GLM_FORCE_DEPTH_ZERO_TO_ONE
  #endif
#endif

#ifdef GFX_ENV_OPENGL
  #include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <audio.h>
#include <input.h>
#include <timer.h>
#include <glmhelper.h>

#ifdef GFX_ENV_VULKAN
  #include "vulkan-render/render.h"
  #include "vulkan-render/resources/resources.h"
#endif

#ifdef GFX_ENV_OPENGL
  #include "opengl-render/render.h"
  #include "opengl-render/resources/resources.h"
#endif

#include <config.h>

#include "camera.h"
#include "game/logic.h"

//#define TIME_APP_DRAW_UPDATE
//#define MULTI_UPDATE_ON_SLOW_DRAW


const float FADE_TIME = 300.0f;
const float FADE_MAX = 0.6f;
const float FADE_START = 2300.0f;

class App {
public:
  App();
  ~App();
  void run();
  void resize(int windowWidth, int windowHeight);

  static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
  static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
  static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
  static void error_callback(int error, const char *description);

  Input input;

private:
  void loadAssets();
  void update();
  void postUpdate();
  void draw();
  void drawEndScreen();

  glm::vec2 appToScreen(glm::vec2 pos);
  glm::vec2 correctedPos(glm::vec2 pos);
  glm::vec2 correctedMouse();

  GLFWwindow *mWindow;
  Render *mRender;
  int mWindowWidth, mWindowHeight;
  Input previousInput;
  Timer timer;
  Audio::Manager audioManager;
  Camera::RoomFollow2D cam2d;

  std::thread submitDraw;
  std::atomic<bool> finishedDrawSubmit;
  
  float camScale = 0.50f;

  GameLogic gameLogic;
  Resource::Font endScreenFont;
  bool paused = false;
  Sprite cursor;
  Resource::Texture pixel;
  float timeSincePause = 0.0f;

    bool playingEndMusic = false;
    Resource::Texture endScreen;
    float timeSinceStart = 0.0f;
    
};

#endif
