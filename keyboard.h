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


#ifndef KEYBOARD_NO_STDLIB // Well, I'm unable to make it work without the stdlib.. A project for the future.
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#define ERROR(message) do { fprintf(stderr, "ERROR: %s\n", message); } while(0)
#else
#define bool 1 : 0
#define true 1
#define false 0
#define ERROR(message) for (;;) {} // Do nothing.
#endif

  

typedef struct {
  bool (*on_press)(const char* key);
  bool (*press_and_release)(const char* key);
  bool (*wait)(const char* key);
  bool (*write)(const char* keys);
} keyboard_t;


bool on_press(const char* key);
bool press_and_release(const char* key);
bool wait_key(const char* key);
bool write_keys(const char* keys);


keyboard_t keyboard = {
  .on_press = on_press,
  .press_and_release = press_and_release,
  .wait = wait_key,
  .write = write_keys
};




#if defined(__linux__)
  #include <linux/uinput.h>
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <sys/select.h>
  #include <sys/time.h>
  #include <unistd.h>

  static struct termios otermios; // original termios, will be saved to be able to be restored later.
  static bool tsetup = false;
  int uin_fd;

  static int get_keycode(const char* key) {
    if (key == NULL || strlen(key) == 0) {
      return 0;
    }

    if (strlen(key) == 1) {
      char c = key[0];
      if (c >= 'a' && c <= 'z') return KEY_A + (c - 'a');
      if (c >= 'A' && c <= 'Z') return KEY_A + (c - 'A'); // For capital letters
      if (c >= '0' && c <= '9') return KEY_0 + (c - '0');
      switch (c) {
        case ' ': return KEY_SPACE;
        case '\n': return KEY_ENTER;
        case '\t': return KEY_TAB;
        case '.': return KEY_DOT;
        case ',': return KEY_COMMA;
        case '/': return KEY_SLASH;
        case '\\': return KEY_BACKSLASH;
        case '-': return KEY_MINUS;
        case '=': return KEY_EQUAL;
        case '[': return KEY_LEFTBRACE;
        case ']': return KEY_RIGHTBRACE;
        case ';': return KEY_SEMICOLON;
        case '\'': return KEY_APOSTROPHE;
        case '`': return KEY_GRAVE;
        case '<': return KEY_COMMA; // Shifted comma
        case '>': return KEY_DOT;   // Shifted dot
        case '?': return KEY_SLASH; // Shifted slash
        case ':': return KEY_SEMICOLON; // Shifted semicolon
        case '"': return KEY_APOSTROPHE; // Shifted apostrophe
        case '{': return KEY_LEFTBRACE; // Shifted left brace
        case '}': return KEY_RIGHTBRACE; // Shifted right brace
        case '|': return KEY_BACKSLASH; // Shifted backslash
        case '_': return KEY_MINUS; // Shifted minus
        case '+': return KEY_EQUAL; // Shifted equal
        case '!': return KEY_1; // Shifted 1
        case '@': return KEY_2; // Shifted 2
        case '#': return KEY_3; // Shifted 3
        case '$': return KEY_4; // Shifted 4
        case '%': return KEY_5; // Shifted 5
        case '^': return KEY_6; // Shifted 6
        case '&': return KEY_7; // Shifted 7
        case '*': return KEY_8; // Shifted 8
        case '(': return KEY_9; // Shifted 9
        case ')': return KEY_0; // Shifted 0
        default: break;
      }
    } else {
      // Handle special keys with string names
      if (strcmp(key, "enter") == 0) return KEY_ENTER;
      if (strcmp(key, "esc") == 0) return KEY_ESC;
      if (strcmp(key, "space") == 0) return KEY_SPACE;
      if (strcmp(key, "tab") == 0) return KEY_TAB;
      if (strcmp(key, "backspace") == 0) return KEY_BACKSPACE;
      if (strcmp(key, "delete") == 0) return KEY_DELETE;
      if (strcmp(key, "home") == 0) return KEY_HOME;
      if (strcmp(key, "end") == 0) return KEY_END;
      if (strcmp(key, "page up") == 0) return KEY_PAGEUP;
      if (strcmp(key, "page down") == 0) return KEY_PAGEDOWN;
      if (strcmp(key, "up") == 0) return KEY_UP;
      if (strcmp(key, "down") == 0) return KEY_DOWN;
      if (strcmp(key, "left") == 0) return KEY_LEFT;
      if (strcmp(key, "right") == 0) return KEY_RIGHT;
      if (strcmp(key, "f1") == 0) return KEY_F1;
      if (strcmp(key, "f2") == 0) return KEY_F2;
      // ... add more function keys as needed
      if (strcmp(key, "left shift") == 0) return KEY_LEFTSHIFT;
      if (strcmp(key, "right shift") == 0) return KEY_RIGHTSHIFT;
      if (strcmp(key, "left ctrl") == 0) return KEY_LEFTCTRL;
      if (strcmp(key, "right ctrl") == 0) return KEY_RIGHTCTRL;
      if (strcmp(key, "left alt") == 0) return KEY_LEFTALT;
      if (strcmp(key, "right alt") == 0) return KEY_RIGHTALT;
      if (strcmp(key, "caps lock") == 0) return KEY_CAPSLOCK;
      if (strcmp(key, "num lock") == 0) return KEY_NUMLOCK;
      if (strcmp(key, "scroll lock") == 0) return KEY_SCROLLLOCK;
    }
    
    ERROR("Unknown key: unrecognized key string.");
    return 0; // Indicate an unknown key
  }


  int make_uinput() {
    int uin_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    
    if (uin_fd < 0) {
      ERROR("No uinput module found.");
      return -1;
    }

     ioctl(uin_fd, UI_SET_EVBIT, EV_KEY); // enable key events.
      for (int i = 0; i < 256; i++) { // enable all keys, maybe change in future to only nessecary keys??
        ioctl(uin_fd, UI_SET_KEYBIT, i);
    }

    struct uinput_setup usetup; // setup
    memset(&usetup, 0, sizeof(usetup));
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "Virtual Keyboard");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 1;
    usetup.id.product = 1;
    usetup.id.version = 1;

    ioctl(uin_fd, UI_DEV_SETUP, &usetup);
    ioctl(uin_fd, UI_DEV_CREATE);

    return uin_fd; 
  }

  void send_key(int keycode, int pressed) { // 1 for pressed, 0 for release.
    struct input_event ev; 
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);
    
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = pressed;
    write(uin_fd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(uin_fd, &ev, sizeof(ev));
  }


  bool write_keys(const char* keys) {
   if (make_uinput() < 0) return false;
    
    for (int i = 0; keys[i] != '\0'; i++) {
      char single_key[2] = {keys[i], '\0'};
      if (!press_and_release(single_key)) return false;
      usleep(100000); // 100ms delay
    }
    return true;
  }

  bool press_and_release(const char* key) {
    if (make_uinput() < 0) return false;
    
    int keycode = get_keycode(key);
    if (keycode == 0) return false;
    
    send_key(keycode, 1); // press
    usleep(50000); // 50ms
    send_key(keycode, 0); // release
    return true;
  }

   bool wait_key(const char* key) {
    ERROR("Unsupported plattform: {LINUX}");
  }

  bool on_press(const char* key) { 
    ERROR("Unsupported plattform: {LINUX}");
  }



#elif defined(__AURORAOS__)
  #include "~/kernel/drivers/keyboard.h"

  bool on_press(const char* key) { 
    ERROR("Unsupported plattform: {AuroraOS}");
  }

  bool write(const char* keys) {
    ERROR("Unsupported plattform: {AuroraOS}");
  }

  bool press_and_release(const char* key) {
    ERROR("Unsupported plattform: {AuroraOS}");
  }

  bool wait(const char* key) {
    ERROR("Unsupported plattform: {AuroraOS}");
  }

#elif defined(__WIN32 || __WIN64)
    bool on_press(const char* key) { 
    ERROR("Unsupported plattform: {WINDOWS}");
  }

  bool write(const char* keys) {
    ERROR("Unsupported plattform: {WINDOWS}");
  }

  bool press_and_release(const char* key) {
    ERROR("Unsupported plattform: {WINDOWS}");
  }

  bool wait(const char* key) {
    ERROR("Unsupported plattform: {WINDOWS}");
  }
#else
  bool on_press(const char* key) { 
    ERROR("Unsupported plattform: {UNKNOWN}");
  }

  bool write(const char* keys) {
    ERROR("Unsupported plattform: {UNKNOWN}");
  }

  bool press_and_release(const char* key) {
    ERROR("Unsupported plattform: {UNKNOWN}");
  }

  bool wait(const char* key) {
    ERROR("Unsupported plattform: {UNKNOWN}");
  }
#endif

#ifdef __cplusplus
}
#endif

#endif
