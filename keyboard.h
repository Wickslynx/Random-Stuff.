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

  static struct termios otermios; // original termios, will be saved to be able to be restored later.
  static bool tsetup = false;
  int uin_fd;


  int make_uinput() {
    int uin_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    
    if (uin_fd < 0) {
      ERROR("No uinput module found.");
      return -1;
    }

     ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY); // enable key events.
      for (int i = 0; i < 256; i++) { // enable all keys, maybe change in future to only nessecary keys??
        ioctl(uinput_fd, UI_SET_KEYBIT, i);
    }

    struct uin_setup usetup; // setup
    memset(&usetup, 0, sizeof(usetup));
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "Virtual Keyboard");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 1;
    usetup.id.product = 1;
    usetup.id.version = 1;

    ioctl(uinput_fd, UI_DEV_SETUP, &usetup);
    ioctl(uinput_fd, UI_DEV_CREATE);

    return uin_fd; 
  }

  void send_key(int keycode, int pressed) { // 1 for pressed, 0 for release.
    struct input_event ev; 
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);
    
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = pressed;
    write(uinput_fd, &ev, sizeof(ev));

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
