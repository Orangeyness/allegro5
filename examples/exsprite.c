/*
 *    Example program for the Allegro library, by Grzegorz Ludorowski.
 *
 *    This example demonstrates how to use datafiles, various sprite drawing
 *    routines and flicker-free animation.
 *
 *    A short explanation for beginners.
 *    Why did I do animate () routine in that way?
 *    As you probably know, VIDEO RAM is much slower than "normal" RAM, so
 *    it's advisable to reduce VRAM blits to a minimum.
 *    So, drawing sprite on the screen (I mean in VRAM) and then clearing
 *    a background for him is not very fast. I've used a different method
 *    which is much faster, but require a bit more memory.
 *    I clear a buffer (it's a normal BITMAP), then I draw sprite to it, and
 *    after all I blit only one time this buffer to the screen. So, I'm using
 *    a single VRAM blit instead of blitting/clearing background and drawing
 *    a sprite on it. It's a good method even when I have to restore.
 *    background And of course, it completely remove flickering effect.
 *    When one uses a big (ie. 800x600 background) and draws something on
 *    it, it's wise to use a copy of background somewhere in memory and
 *    restore background using this "virtual background". When blitting from
 *    VRAM in SVGA modes, it's probably, that drawing routines have to switch
 *    banks on video card. I think, I don't have to remind how slow is it.
 */


#include "allegro.h"
#include "running.h"



/* pointer to data file */
DATAFILE *running_data;

/* current sprite frame number */
int frame_number = 0;

/* pointer to a sprite buffer, where sprite will be drawn */
BITMAP *sprite_buffer;

/* a boolean - if true, skip to next part */
int next;



void animate()
{
   /* waits for vertical retrace interrupt */
   vsync();
   vsync();

   /* blits sprite buffer to screen */
   blit(sprite_buffer, screen, 0, 0, 120, 80, 85, 85);

   /* clears sprite buffer with color 0 */
   clear(sprite_buffer);

   /* if SPACE key pressed set a next flag */
   if (keypressed())
      next = TRUE;
   else
      next = FALSE;

   /* increase frame number, or if it's equal 9 (last frame) set it to 0 */
   if (frame_number == 9)
      frame_number = 0;
   else
      frame_number++;
}



int main(int argc, char *argv[])
{
   char datafile_name[256];
   int angle = 0;

   allegro_init();
   install_keyboard();
   set_gfx_mode(GFX_SAFE, 320, 200, 0, 0);

   /* loads datafile and sets user palette saved in datafile */
   replace_filename(datafile_name, argv[0], "running.dat", sizeof(datafile_name));
   running_data = load_datafile(datafile_name);
   if (!running_data) {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Error loading %s!\n", datafile_name);
      return 1;
   }

   set_palette(running_data[PALETTE_001].dat);

   /* create and clear a bitmap for sprite buffering */
   sprite_buffer = create_bitmap(85, 85);
   clear(sprite_buffer);

   /* write current sprite drawing method */
   textout(screen, font, "Press a key for next part...", 40, 10, 1);
   textout(screen, font, "Using draw_sprite", 1, 190, 15);

   do {
      draw_sprite(sprite_buffer, running_data[frame_number].dat, 0, 0);
      animate();
   } while (!next);

   clear_keybuf();
   rectfill(screen, 0, 190, 320, 200, 0);
   textout(screen, font, "Using draw_sprite_h_flip", 1, 190, 15);

   do {
      draw_sprite_h_flip(sprite_buffer, running_data[frame_number].dat, 0, 0);
      animate();
   } while (!next);

   clear_keybuf();
   rectfill(screen, 0, 190, 320, 200, 0);
   textout(screen, font, "Using draw_sprite_v_flip", 1, 190, 15);

   do {
      draw_sprite_v_flip(sprite_buffer, running_data[frame_number].dat, 0, 0);
      animate();
   } while (!next);

   clear_keybuf();
   rectfill(screen, 0, 190, 320, 200, 0);
   textout(screen, font, "Using draw_sprite_vh_flip", 1, 190, 15);

   do {
      draw_sprite_vh_flip(sprite_buffer, running_data[frame_number].dat, 0, 0);
      animate();
   } while (!next);

   clear_keybuf();
   rectfill(screen, 0, 190, 320, 200, 0);
   textout(screen, font, "Now with rotating - rotate_sprite", 1, 190, 15);

   do {
      /* The last argument to rotate_sprite() is a fixed point type,
       * so I had to use itofix() routine (integer to fixed).
       */
      rotate_sprite(sprite_buffer, running_data[frame_number].dat, 0, 0, itofix(angle));
      animate();
      angle += 4;
   } while (!next);

   clear_keybuf();
   rectfill(screen, 0, 190, 320, 200, 0);
   textout(screen, font, "Now using rotate_sprite_v_flip", 1, 190, 15);

   do {
      /* The last argument to rotate_sprite_v_flip() is a fixed point type,
       * so I had to use itofix() routine (integer to fixed).
       */
      rotate_sprite_v_flip(sprite_buffer, running_data[frame_number].dat, 0, 0, itofix(angle));
      animate();
      angle += 4;
   } while (!next);

   unload_datafile(running_data);
   return 0;
}

END_OF_MAIN();
