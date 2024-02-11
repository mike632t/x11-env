/*
 *
 * x11-env.c
 *
 * Copyright(C) 2013   MEJT
 *
 * Creates an X11 window and attempts to list some 'well known' environment 
 * vairables  on  the screen before waiting for the user to press a key  or 
 * click in the window.  
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.   If not, see <http://www.gnu.org/licenses/>.
 * 
 * Displays some text in a window on the screen.
 *
 * On VMS:
 * Compile with 'cc x11-env' 
 * Link using 'link x11-env, x11-env.opt/opt'
 *
 * On Linux:
 * Compile and link with 'gcc x11-env.c -o x11-env -lX11'
 *
 * 11 Feb 24   0.1      - Initial version - MT
 *
 */

#include <X11/Xlib.h> /* XOpenDisplay(), etc. */
#include <stdio.h> /* fprintf(), etc. */
#include <stdlib.h> /* getenv(), etc. */
#include <string.h> /* strlen(), etc. */

Display *h_display; /* Pointer to X display structure. */
Window x_application_window; /* Application window structure. */
Window x_root_window; /* Root window structure. */
Window x_child_window; /* Child window returned from XTranslateCoodinates */
XEvent x_event;
XFontStruct *h_font;

int i_status = 0;
int i_count = 0;
int i_size = 0;
int i_screen; /* Default screen number */
unsigned int i_window_width = 320; /* Window width in pixels. */
unsigned int i_window_height = 192; /* Window height in pixels. */
unsigned int i_window_border = 4; /* Window's border width. */
unsigned int i_background_colour; /* Window's background colour. */
unsigned int i_colour_depth; /* Window's colour depth. */

int i_window_left, i_window_top; /* Location of the window's top-left corner - relative to parent window. */
unsigned int i_text_width;
unsigned int i_text_width;
unsigned int i_offset, i_position; /* Position of text */
char *s_text;
char *s_display_name = ""; /* Just use the default display */

const char *s_names[] = {"HOME", "XDG_DATA_HOME", "XDG_CONFIG_HOME", "XDG_STATE_HOME", "XDG_SESSION_TYPE"}; /* List of environment vairables to display */

int main(int argc, char *argv[])
{
   h_display = XOpenDisplay(s_display_name); /*   Open a display. */
   if (h_display) /*   If successful create and display a new window. */
   {
      x_root_window = DefaultRootWindow(h_display); /* Get the ID of the root window of the screen. */
      i_screen = DefaultScreen(h_display); /* Get the default screen for our X server. */

      /* Create the application window, as a child of the root window. */
      x_application_window = XCreateSimpleWindow(h_display, RootWindow(h_display, i_screen), 
         i_window_width, i_window_height,  /* Window position -igore ? */
         i_window_width, /* Window width */
         i_window_height, /* Window height */
         i_window_border, /* Border width - ignored ? */
         BlackPixel(h_display, i_screen), /* Preferred method to set border colour to black */
         /* i_background_colour); /* Background colour - RGB value. */
         WhitePixel(h_display, i_screen)); /* Preferred method to set background colour to white */

      XStoreName(h_display, x_application_window, argv[0]); /* Set the window title */
      XMapWindow(h_display, x_application_window); /*   Show the window */
      XSync(h_display, False); /* Flush all pending requests to the X server, and wait until they are processed by the X server. */

      XSelectInput(h_display, x_application_window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask); /* Select which events we want to be passed to the program */

      h_font = XQueryFont(h_display, XGContextFromGC(DefaultGC(h_display, i_screen))); /* Get the default font properties */

      while (1) /* Main program event loop */
      {
         XNextEvent(h_display, &x_event); /* Wait until the next event */
         if (x_event.type == Expose) 
         {
            /* Get window geometry - not everything will always be the same as the values we requested for when we created the window */
            XGetGeometry(h_display, x_application_window, 
               &RootWindow(h_display, i_screen),
               &i_window_left, &i_window_top,
               &i_window_width,
               &i_window_height,
               &i_window_border,
               &i_colour_depth);

            i_size = (int)(sizeof(s_names)/sizeof(s_names[0])); /* Get number of environment vairables in the list */
            
            /* Display the environment vairables in the window and on stdout */
            i_position = h_font->ascent + i_window_border;
            for (i_count = 0; i_count < i_size; i_count++)
            {
               fprintf(stdout, "%s = %s\n", s_names[i_count], getenv(s_names[i_count])); 
               i_offset = i_window_border;
               
               s_text = (char *) s_names[i_count];
               XDrawString(h_display, x_application_window,
                  DefaultGC(h_display, i_screen),
                  i_offset, i_position,
                  s_text, strlen(s_text));
               i_offset += XTextWidth(h_font, s_text, strlen(s_text));
               
               s_text = " = ";
               XDrawString(h_display, x_application_window,
                  DefaultGC(h_display, i_screen),
                  i_offset, i_position,
                  s_text, strlen(s_text));
               i_offset += XTextWidth(h_font, s_text, strlen(s_text));

               s_text = getenv(s_names[i_count]);
               if (!s_text) s_text = "(null)"; /* Display '(null)' if a null string is returned */
               XDrawString(h_display, x_application_window,
                  DefaultGC(h_display, i_screen),
                  i_offset, i_position,
                  s_text, strlen(s_text));

               i_position += h_font->ascent + h_font->descent; /* Advance to next 'line' */
            }
            XFlush(h_display); /* Update the display */
         }
         if (x_event.type == KeyPress) break; /* Exit if a key is pressed */
         if (x_event.type == ButtonRelease) break; /* Exit if a mouse button is clicked */
      }
      XCloseDisplay(h_display); /* Close the connection to X server */
   }
   else 
   {
      fprintf(stderr, "%s: cannot connect to X server '%s'\n", argv[0], s_display_name); 
      i_status = -1;
   }
   return i_status;
}
