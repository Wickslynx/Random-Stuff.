#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/* 

Supported operating systems:
- Linux
- Windows
- AuroraOS

Methods:
keyboard.on_press("char"); - Will return smth if pressed.
keyboard.press_and_release("char") - press and release a single char.
keyboard.write("sentence"); - write to do it.
keyboard.wait('esc'); - wait untill you press escape.
keyboard.hotkey("page up, page down", dosmth); - not supported rn.

*/


#ifndef KEYBOARD_NO_STDLIB
#include <stdio.h>
#include <stdlib.h>
#define ERROR (const char message) printf("ERROR:" + message)
#else
#define ERROR (const char message) for (;;) {} // Do nothing.
#endif

typedef struct {
  bool (*on_press)(const char* key);
  bool (*press_and_release)(const char* key);
  bool (*wait)(const char* key);
  bool (*write)(const char* keys);
} keyboard;


#ifdef __WIN32__ || __WIN64__
  #include <windows.h>
  bool on_press(const char* key) {
    if (!HWND) {init_windows()}
    return true;
  }
#elif defined(__LINUX__)
  bool on_press(const char* key) {
   
  } 
  bool write(const char* keys) {
    
  }

#elif defined(__AURORAOS__)
  #include "~/kernel/drivers/keyboard.h"

#else
  bool on_press(const char* key) { 
    ERROR("Unsupported plattform: {PLATFORM_MAC}");
  }

  bool write(const char* keys) {
    ERROR("Unsupported plattform: {PLATFORM_MAC}");
  }

  bool press_and_release(const char* key) {
    ERROR("Unsupported plattform: {PLATFORM_MAC}");
  }
  
#endif

#ifdef __cplusplus
}
#endif

#endif
