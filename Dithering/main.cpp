#include <stdio.h>
#include <stdlib.h>
#include <allegro5\allegro5.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_native_dialog.h>
#include <allegro5\allegro_image.h>

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_EVENT ev;
ALLEGRO_BITMAP *inputBitmap;
ALLEGRO_BITMAP *outputBitmap;
int XpicSize, YpicSize;
unsigned char thresholdLevel[9] = { 0, 70, 0, 127, 127, 127, 0, 0, 100 };
char dithering_mode = 0;
#define rgb(x) al_map_rgb((x), (x), (x))

ALLEGRO_COLOR operator+ (ALLEGRO_COLOR c1, ALLEGRO_COLOR c2)
{
	ALLEGRO_COLOR result;
	result = c1;
	result.r += c2.r;
	result.g += c2.g;
	result.b += c2.b;
	if(result.r < 0) result.r = 0.0;
	if(result.g < 0) result.g = 0.0;
	if(result.b < 0) result.b = 0.0;
	if(result.r > 1) result.r = 1.0;
	if(result.g > 1) result.g = 1.0;
	if(result.b > 1) result.b = 1.0;
	return result;
}

ALLEGRO_COLOR operator- (ALLEGRO_COLOR c1, ALLEGRO_COLOR c2)
{
	ALLEGRO_COLOR result;
	result = c1;
	result.r -= c2.r;
	result.g -= c2.g;
	result.b -= c2.b;
	if(result.r < 0) result.r = 0.0;
	if(result.g < 0) result.g = 0.0;
	if(result.b < 0) result.b = 0.0;
	if(result.r > 1) result.r = 1.0;
	if(result.g > 1) result.g = 1.0;
	if(result.b > 1) result.b = 1.0;
	return result;
}

ALLEGRO_COLOR operator* (ALLEGRO_COLOR c1, float multiple)
{
	ALLEGRO_COLOR result;
	result = c1;
	result.r *= multiple;
	result.g *= multiple;
	result.b *= multiple;
	if(result.r < 0) result.r = 0.0;
	if(result.g < 0) result.g = 0.0;
	if(result.b < 0) result.b = 0.0;
	if(result.r > 1) result.r = 1.0;
	if(result.g > 1) result.g = 1.0;
	if(result.b > 1) result.b = 1.0;
	return result;
}

ALLEGRO_COLOR operator/ (ALLEGRO_COLOR c1, float divider)
{
	ALLEGRO_COLOR result;
	result = c1;
	result.r /= divider;
	result.g /= divider;
	result.b /= divider;	
	if(result.r < 0) result.r = 0.0;
	if(result.g < 0) result.g = 0.0;
	if(result.b < 0) result.b = 0.0;
	if(result.r > 1) result.r = 1.0;
	if(result.g > 1) result.g = 1.0;
	if(result.b > 1) result.b = 1.0;
	return result;
}

int initialize_allegro(int XwindowSize, int YwindowSize) //preparing allegro libraries before working, creates main display
{	
	display = al_create_display((XwindowSize < 520 ? 520 : XwindowSize), YwindowSize);
	if(!display) 
	{
		al_show_native_message_box(display, "Error", "Error", "failed to create display!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}

	event_queue = al_create_event_queue();
	if(!event_queue) 
	{
		al_show_native_message_box(display, "Error", "Error", "failed to create event_queue!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		al_destroy_display(display);
		return -1;
	}
	al_flush_event_queue(event_queue);

	al_init_font_addon();
	al_init_ttf_addon();

	font = al_load_ttf_font("C:\\Windows\\Fonts\\cour.ttf", 17, 0);
	if(!font)
	{
		al_destroy_event_queue(event_queue);
		al_show_native_message_box(display, "Error", "Error", "Failed to initialize font!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		al_destroy_display(display);
		return -1;
	}

	al_init_primitives_addon();
	al_install_keyboard();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());	
	al_init_font_addon();
	return 1;
}

void redraw() // draws everything in their places in main window.
{
#define pos(x) (YpicSize+127+17*x)	// macro helping to calculate text vertical position
	ALLEGRO_COLOR al_red = al_map_rgb(255, 100, 100);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	al_set_target_backbuffer(display);

	al_clear_to_color(al_map_rgb(20, 20, 75));			//clear display and set bg color
	al_draw_bitmap(inputBitmap, 0, 20, 0);				// drawing input image
	al_draw_bitmap(outputBitmap, XpicSize+10, 20, 0);	// drawing output image
	al_draw_text(font, al_map_rgb(200, 200, 200), 0, 0, 0, "Original");
	al_draw_text(font, al_map_rgb(200, 200, 200), XpicSize+10, 0, 0, "Processed");
	if(dithering_mode != 0 && dithering_mode != 2 && dithering_mode != 6 && dithering_mode != 7)
	{
		//double call (but with 1px shift) as simple bold
		al_draw_textf(font, al_map_rgb(255, 255, 200), 100, 30+YpicSize, 0, "Threshold level: %d", thresholdLevel[dithering_mode]);
		al_draw_textf(font, al_map_rgb(255, 255, 200), 101, 30+YpicSize, 0, "Threshold level: %d", thresholdLevel[dithering_mode]);
	}
	al_draw_text(font, al_white, 10, pos(-4), 0, "Usage (keyboard):");
	al_draw_text(font, al_white, 10, pos(-3), 0, "use [digits] to choose dithering type");
	al_draw_text(font, al_white, 10, pos(-2), 0, "use [+] [-] to change variable if possible");
	al_draw_text(font, al_white, 10, pos(-1), 0, "press [s] to SAVE, [Q] or [ESC] to QUIT");

	// modes list
	al_draw_text(font, al_white, 10, pos(1), 0, "Mode:");
	al_draw_text(font, (dithering_mode==1 ? al_red : al_white), 10, pos(2), 0, "1. Simple threshold dithering");
	al_draw_text(font, (dithering_mode==2 ? al_red : al_white), 10, pos(3), 0, "2. Random dithering");
	al_draw_text(font, (dithering_mode==3 ? al_red : al_white), 10, pos(4), 0, "3. Single direction error diffusion dith.");
	al_draw_text(font, (dithering_mode==4 ? al_red : al_white), 10, pos(5), 0, "4. Changing direction single ED dithering");
	al_draw_text(font, (dithering_mode==5 ? al_red : al_white), 10, pos(6), 0, "5. Sierra Lite dithering");
	al_draw_text(font, (dithering_mode==6 ? al_red : al_white), 10, pos(7), 0, "6. Ordered 4x4 dithering");
	al_draw_text(font, (dithering_mode==7 ? al_red : al_white), 10, pos(8), 0, "7. Ordered 8x8 dithering");
	al_draw_text(font, (dithering_mode==8 ? al_red : al_white), 10, pos(9), 0, "8. Floyd-Steinberg dithering");

	al_flip_display();	//apply changes and show result to user (everything from above is being drawn in backbuffer)
}

void simple_threshold_dithering(char random)
{
	if(random)
		srand(time(NULL));
	al_set_target_bitmap(outputBitmap);
	// store image in memory in write-only mode
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			//for every pixel of bitmap read its brightness and check whether it is above or below given level and change this pixel to pure white or black.
			pixel_read = al_get_pixel(inputBitmap, x, y); // pixel_read.r is a float type with values 0.0 to 1.0 max
			if(pixel_read.r <= (random ? (rand()%255)/255.0 : thresholdLevel[1]/255.0) )
				al_put_pixel(x, y, al_black);
			else
				al_put_pixel(x, y, al_white);
		}
	}
	al_unlock_bitmap(outputBitmap);
}

void sierra_lite_dithering()
{
	ALLEGRO_BITMAP *cloneBitmap;	// we will work on copy to not change the original bitmap
	cloneBitmap = al_clone_bitmap(inputBitmap);
	outputBitmap = al_clone_bitmap(inputBitmap);
	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);
	al_lock_bitmap(cloneBitmap, al_get_bitmap_format(cloneBitmap), ALLEGRO_LOCK_READWRITE);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;

	for(int y=0; y < YpicSize; y++)
	{
		for(int x=0; x < XpicSize; x++)
		{
			pixel_read = al_get_pixel(cloneBitmap, x, y);
			if(pixel_read.r <= thresholdLevel[5]/255.0)
			{
				al_set_target_bitmap(cloneBitmap);
				// spread stolen brightness among pixel-neighbors in strict way
				if(x != 0)
					al_put_pixel((x-1), (y+1), al_get_pixel(cloneBitmap, (x-1), (y+1)) + (pixel_read * 1 / 4));	// under left
				if(x < XpicSize-1)
					al_put_pixel((x+1), y, al_get_pixel(cloneBitmap, (x+1), y) + (pixel_read * 2 / 4));			// to the right
				if(y < YpicSize-1)
					al_put_pixel(x, (y+1), al_get_pixel(cloneBitmap, x, (y+1)) + (pixel_read * 1 / 4));			// under
				al_set_target_bitmap(outputBitmap);
				al_put_pixel(x, y, al_black);
			}
			else
			{
				al_set_target_bitmap(cloneBitmap);
				if(x != 0)
					al_put_pixel((x-1), (y+1), al_get_pixel(cloneBitmap, (x-1), (y+1)) - (pixel_read * 1 / 4));	// under left
				if(x < XpicSize-1)
					al_put_pixel((x+1), y, al_get_pixel(cloneBitmap, (x+1), y) - (pixel_read * 2 / 4));			// to the right
				if(y < YpicSize-1)
					al_put_pixel(x, (y+1), al_get_pixel(cloneBitmap, x, (y+1)) - (pixel_read * 1 / 4));			// under
				al_set_target_bitmap(outputBitmap);
				al_put_pixel(x, y, al_white);			}
		}
	}
	al_unlock_bitmap(cloneBitmap);
	al_destroy_bitmap(cloneBitmap);
	al_unlock_bitmap(outputBitmap);

}

void floyd_steinberg_dithering()
{
	ALLEGRO_BITMAP *cloneBitmap;
	cloneBitmap = al_clone_bitmap(inputBitmap);
	outputBitmap = al_clone_bitmap(inputBitmap);
	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);
	al_lock_bitmap(cloneBitmap, al_get_bitmap_format(cloneBitmap), ALLEGRO_LOCK_READWRITE);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;

	for(int y=0; y < YpicSize; y++)
	{
		for(int x=0; x < XpicSize; x++)
		{
			pixel_read = al_get_pixel(cloneBitmap, x, y);
			if(pixel_read.r <= thresholdLevel[8]/255.0)
			{
				al_set_target_bitmap(cloneBitmap);
				if(x != 0)
					al_put_pixel((x-1), (y+1), al_get_pixel(cloneBitmap, (x-1), (y+1)) + (pixel_read * 3 / 16));	// under left
				if(x < XpicSize-1)
					al_put_pixel((x+1), y, al_get_pixel(cloneBitmap, (x+1), y) + (pixel_read * 7 / 16));			// to the right
				if(y < YpicSize-1)
					al_put_pixel(x, (y+1), al_get_pixel(cloneBitmap, x, (y+1)) + (pixel_read * 5 / 16));			// under
				if(x < XpicSize-1 && y < YpicSize-1)
					al_put_pixel((x+1), (y+1), al_get_pixel(cloneBitmap, (x+1), (y+1)) + (pixel_read / 16));		// under right
				al_set_target_bitmap(outputBitmap);
				al_put_pixel(x, y, al_black);
			}
			else
			{
				al_set_target_bitmap(cloneBitmap);
				if(x != 0)
					al_put_pixel((x-1), (y+1), al_get_pixel(cloneBitmap, (x-1), (y+1)) - (pixel_read * 3 / 16));	// under left
				if(x < XpicSize-1)
					al_put_pixel((x+1), y, al_get_pixel(cloneBitmap, (x+1), y) - (pixel_read * 7 / 16));			// to the right
				if(y < YpicSize-1)
					al_put_pixel(x, (y+1), al_get_pixel(cloneBitmap, x, (y+1)) - (pixel_read * 5 / 16));			// under
				if(x < XpicSize-1 && y < YpicSize-1)
					al_put_pixel((x+1), (y+1), al_get_pixel(cloneBitmap, (x+1), (y+1)) - (pixel_read / 16));		// under right
				al_set_target_bitmap(outputBitmap);
				al_put_pixel(x, y, al_white);			}
		}
	}
	al_unlock_bitmap(cloneBitmap);
	al_destroy_bitmap(cloneBitmap);
	al_unlock_bitmap(outputBitmap);
}


void one_direction_dithering()
{
	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;
	float error = 0;

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			pixel_read = al_get_pixel(inputBitmap, x, y);
			if(pixel_read.r + error <= thresholdLevel[3]/255.0 )
			{
				error += pixel_read.r;
				al_put_pixel(x, y, al_black);
			}
			else
			{
				error -= (1-pixel_read.r);
				al_put_pixel(x, y, al_white);
			}
		}
		error = 0.0;
	}
	al_unlock_bitmap(outputBitmap);
}

void one_direction_zigzag_dithering()
{

	outputBitmap = al_clone_bitmap(inputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);
	al_set_target_bitmap(outputBitmap);
	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;
	float error = 0;	// variable to save stolen or excess brigtness

	for(int y=0; y<YpicSize; y++)
	{
		int x;
		for( ((y%2) ? x=0 : x=XpicSize-1); ((y%2) ? x<XpicSize : x>=0); ((y%2) ? x++ : x--)) //every pass the direction of operation changes
		{
			pixel_read = al_get_pixel(inputBitmap, x, y);
			if(pixel_read.r + error <= thresholdLevel[4]/255.0 )
			{
				error += pixel_read.r;
				al_put_pixel(x, y, al_black);
			}
			else
			{
				error -= (1-pixel_read.r);
				al_put_pixel(x, y, al_white);
			}
		}
	}
	al_unlock_bitmap(outputBitmap);
}

void ordered_4x4_dithering()
{
	// mode in which threshold for specific pixel is being read from algorithm coordinates modulo array_size
	float bayer_map[4][4] = 
	{	{15*1,  15*9,  15*3,  15*11},
		{15*13, 15*5,  15*15, 15*7 },
		{15*4,  15*12, 15*2,  15*10},
		{15*16, 15*8,  15*14, 15*6 }	};

	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			pixel_read = al_get_pixel(inputBitmap, x, y);
			if(pixel_read.r <= bayer_map[x%4][y%4]/255.0 )
				al_put_pixel(x, y, al_black);
			else
				al_put_pixel(x, y, al_white);
		}
	}
	al_unlock_bitmap(outputBitmap);
}

void ordered_8x8_dithering()
{
	float bayer_map[8][8] = 
	{	{4*1,  4*49, 4*13, 4*61, 4*4,  4*52, 4*16, 4*64, },
		{4*33, 4*17, 4*45, 4*29, 4*36, 4*20, 4*48, 4*32, },
		{4*9,  4*57, 4*5,  4*53, 4*12, 4*60, 4*8,  4*56, },
		{4*41, 4*25, 4*37, 4*21, 4*44, 4*28, 4*40, 4*24, },
		{4*3,  4*51, 4*15, 4*63, 4*2,  4*50, 4*14, 4*62, },
		{4*35, 4*19, 4*47, 4*31, 4*34, 4*18, 4*46, 4*30, },
		{4*11, 4*59, 4*7,  4*55, 4*10, 4*58, 4*6,  4*54, },
		{4*43, 4*27, 4*39, 4*23, 4*42, 4*26, 4*38, 4*22, },	};

	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			pixel_read = al_get_pixel(inputBitmap, x, y);
			if(pixel_read.r <= bayer_map[x%8][y%8]/255.0 )
				al_put_pixel(x, y, al_black);
			else
				al_put_pixel(x, y, al_white);
		}
	}
	al_unlock_bitmap(outputBitmap);
}

void dither(char mode)
{
	thresholdLevel[0] = 0;
	al_draw_text(font, al_map_rgb(200, 255, 200), XpicSize+20, 30, 0, "working...");
	al_flip_display();
	switch(mode)
	{
	case 0: outputBitmap = al_clone_bitmap(inputBitmap); break;
	case 1: simple_threshold_dithering(0); break;
	case 2: simple_threshold_dithering(1); break;	// RANDOM dithering
	case 3: one_direction_dithering(); break;
	case 4: one_direction_zigzag_dithering(); break;
	case 5: sierra_lite_dithering(); break;
	case 6: ordered_4x4_dithering(); break;
	case 7: ordered_8x8_dithering(); break;
	case 8: floyd_steinberg_dithering(); break;
	}
	redraw();
}

int main(int argc, char **argv)
{
	if(!al_init()) 
	{
		al_show_native_message_box(display, "Error", "Error", "Failed to initialize allegro!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	// image is loaded to determine size of window.
	al_init_image_addon();
	if(argc > 1)
		inputBitmap = al_load_bitmap(argv[1]);
	else
		inputBitmap = al_load_bitmap("input.bmp");
	if(!inputBitmap)
	{
		al_show_native_message_box(display, "Error", "Input file not found", "Could not load image neither looking for input.bmp file nor filename given in first program argument.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return 0;
	}
	XpicSize = al_get_bitmap_width(inputBitmap);
	YpicSize = al_get_bitmap_height(inputBitmap);
	// now we can freely initialize rest of allegro modules and create window
	initialize_allegro( XpicSize*2+10, YpicSize+320 );

	outputBitmap = al_clone_bitmap(inputBitmap);
	if(!outputBitmap)
	{
		al_show_native_message_box(display, "Error", "Runtime error", "Error creating clone of bitmap.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return 0;
	}

	redraw();

	while(1)
	{
		al_wait_for_event(event_queue, &ev);	// wait for some key press or window close signal
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && (ev.keyboard.keycode == ALLEGRO_KEY_Q || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)))
			return 0;  //exit program
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_0:
			case ALLEGRO_KEY_PAD_0: dithering_mode = 0;	break;
			case ALLEGRO_KEY_1:
			case ALLEGRO_KEY_PAD_1: dithering_mode = 1; break;
			case ALLEGRO_KEY_2:
			case ALLEGRO_KEY_PAD_2:	dithering_mode = 2;	break;
			case ALLEGRO_KEY_3:
			case ALLEGRO_KEY_PAD_3:	dithering_mode = 3;	break;
			case ALLEGRO_KEY_4:
			case ALLEGRO_KEY_PAD_4:	dithering_mode = 4;	break;
			case ALLEGRO_KEY_5:
			case ALLEGRO_KEY_PAD_5:	dithering_mode = 5;	break;
			case ALLEGRO_KEY_6:
			case ALLEGRO_KEY_PAD_6:	dithering_mode = 6;	break; 
			case ALLEGRO_KEY_7:
			case ALLEGRO_KEY_PAD_7:	dithering_mode = 7; break; 
			case ALLEGRO_KEY_8:
			case ALLEGRO_KEY_PAD_8:	dithering_mode = 8; break; 
			case ALLEGRO_KEY_PAD_PLUS: 
				thresholdLevel[dithering_mode]+=3;
				break;
			case ALLEGRO_KEY_PAD_MINUS: 
				thresholdLevel[dithering_mode]-=3;
			break;
			case ALLEGRO_KEY_S:	//image saving operations
				if(al_save_bitmap("your_dithered_image.bmp", outputBitmap))
				{
					al_draw_filled_rectangle(0, 0, 550, 40, al_map_rgb(20, 75, 20));
					al_draw_text(font, al_map_rgb(150, 255, 150), 10, 10, 0, "Saved as your_dithered_image.bmp, you can continue");
					al_flip_display();
					continue;
				}
				break;
			}
			thresholdLevel[0] != 1;
				dither(dithering_mode);
		} // if end
	} //inf loop end
}