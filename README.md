# Termbox2

A small library in C for building text-based user interfaces. This is a (non-compatible) fork of nsf's [original work](https://github.com/nsf/termbox), with a few changes:

 - Better input handling, so Ctrl/Alt/Shift key combos are property detected
 - Additional init options, (e.g. skip clearing full screen, cursor hiding, etc)
 - True color support
 - Some other stuff that was cherry picked from the forks across Github

Important: This fork isn't a drop-in replacement for the original fork, as several methods have changed and/or have been renamed.

## Installation

First thing's first:

    git clone github.com/tomas/termbox
    cd termbox

Now run cmake to generate the Makefiles.

    mkdir build
    cd build
    cmake ..

And release the kraken:

    make
    make install

## Usage

Termbox has a very clean interface. Here's your basic 'hello world':

```c
#include <unistd.h> // for sleep()
#include "termbox.h"

int main(int argc, char **argv) {
  if (tb_init() != 0)
    return 1; // couldn't initialize our screen

  // set up our colors
  int bg = TB_BLACK;
  int fg = tb_rgb(0xFF6600); // orange

  // get the screen resolution
  int w = tb_width();
  int h = tb_height();

  // calculate the coordinates for the center of the screen
  int x = (w/2);
  int y = (h/2);

  // put some text into those coordinates
  tb_string(x-10, y, bg, fg, "Hello world!");

  // flush the output to the screen
  tb_render();

  // wait a few secs
  sleep(3);

  // and finally, shutdown the screen so we exit cleanly
  tb_shutdown();

  return 0;
}
```

Ok, now let's capture some input. Instead of the `sleep()` statement above, we could do this:

```c
...
tb_render();

// set up a counter and enable mouse input
int clicks = 0;
tb_enable_mouse();

// now, listen for events
struct tb_event ev;
while (tb_poll_event(&ev) != -1) {
  switch (ev.type) {
    case TB_EVENT_KEY:
      // we got a keyboard event. if the user hit the ESC
      // key or Ctrl-C, then exit the program
      if (ev.key == TB_KEY_ESC || ev.key == TB_KEY_CTRL_C) {
        goto done;
      }
      break;

    case TB_EVENT_MOUSE:
      // ok, looks like we got a mouse event
      if (ev.key == TB_KEY_MOUSE_LEFT) {
        // increase the counter and show a message on screen.
        // we'll also include the reported mouse coordinates.
        tb_printf((w/2)-6, h/2, bg_color, fg_color,
          "Click number %d! (%d, %d)", ++clicks, ev.x, ev.y);

        tb_render();
      }
      break;
  }
}

done: // shutdown logic
tb_shutdown();
...
```

For more information, take a look at [the demos](https://github.com/tomas/termbox/tree/master/demos) or check the [termbox.h](https://github.com/tomas/termbox/blob/master/src/termbox.h) header for the full termbox API.

## License

MIT.

