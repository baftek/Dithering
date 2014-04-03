#include <stdio.h>
#include <stdio.h>
//#include <thread>
//#include <conio.h>
#include <allegro5\allegro5.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_native_dialog.h>
#include <allegro5\allegro_image.h>
using namespace std;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_EVENT ev;
ALLEGRO_BITMAP *inputBitmap;
ALLEGRO_BITMAP *outputBitmap;
FILE *f;
int XpicSize, YpicSize;
unsigned char ditheringVariables[5];
char dithering_mode = 0;

ALLEGRO_COLOR operator+(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2)
{
	ALLEGRO_COLOR result;
	result.a = c1.a + c2.a;
	result.r = c1.r + c2.r;
	result.g = c1.g + c2.g;
	result.b = c1.b + c2.b;
	return result;
}

ALLEGRO_COLOR operator/(ALLEGRO_COLOR c1, int div)
{
	ALLEGRO_COLOR result;
	result.a = c1.a / div;
	result.r = c1.r / div;
	result.g = c1.g / div;
	result.b = c1.b / div;
	return result;
}


int initialize_allegro(int XwindowSize, int YwindowSize)
{	
	//if(!al_init()) 
	//{
	//	al_show_native_message_box(display, "Error", "Error", "Failed to initialize allegro!", 
 //                                NULL, ALLEGRO_MESSAGEBOX_ERROR);
	//	return -1;
	//}

	display = al_create_display(XwindowSize, YwindowSize);
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

	//timer = al_create_timer(1.0 / freq);
	//if(!timer) 
	//{
	//	al_destroy_display(display);
	//	al_destroy_event_queue(event_queue);
	//	al_destroy_font(font);
	//	al_show_native_message_box(display, "Error", "Error", "Failed to initialize timer!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
	//	return -1;
	//}


	al_init_primitives_addon();
	al_install_keyboard();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//al_register_event_source(event_queue, al_get_timer_event_source(timer));
	
	al_init_font_addon();
	return 1;
}

void redraw()
{
	al_set_target_backbuffer(display);

	al_clear_to_color(al_map_rgb(20, 20, 75));
	al_draw_bitmap(inputBitmap, 0, 20, 0);
	al_draw_bitmap(outputBitmap, XpicSize+10, 20, 0);
	al_draw_text(font, al_map_rgb(200, 200, 200), 0, 0, 0, "Original");
	al_draw_text(font, al_map_rgb(200, 200, 200), XpicSize+10, 0, 0, "Processed");
	switch(dithering_mode)
	{
	case 1: al_draw_textf(font, al_map_rgb(255, 255, 255), 400, 30+YpicSize, 0, "Threshold level: %d", ditheringVariables[1]); break;
	case 3: 
	case 2: al_draw_textf(font, al_map_rgb(255, 255, 255), 400, 30+YpicSize, 0, ((ditheringVariables[2] == 255) ? "Number of ranges: %d max" : "Number of ranges: %d"), ditheringVariables[2]); break;
	}
	al_flip_display();
}

void threshold_dithering()
{
	//outputBitmap = al_clone_bitmap(inputBitmap);
	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR al_black = al_map_rgb(0, 0, 0);
	ALLEGRO_COLOR al_white = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR pixel_read;
	//unsigned char value_read;
	//unsigned char value_over_3;

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			pixel_read = al_get_pixel(outputBitmap, x, y);
			//value_read = (unsigned char)(pixel_read.r*255);
			//value_over_3 = value_read/3;
			if((unsigned char)(pixel_read.r*255) <= ditheringVariables[1])
				{
					al_put_pixel(x, y, al_black);
					//al_put_pixel((x+1)%255, y, pixel_read + value));
				}
			else
				al_put_pixel(x, y, al_white);
		}
	}
	al_unlock_bitmap(outputBitmap);
}

void simple_ranges_dithering()
{
	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR pixel_read; 
	unsigned char widhtOfRange = 255 / ditheringVariables[2];

#define rgb(x) al_map_rgb((x), (x), (x))

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			int i = 0, j = 0;
			pixel_read = al_get_pixel(inputBitmap, x, y);
			al_put_pixel(x, y, rgb((unsigned char)(pixel_read.r*255) - ((unsigned char)(pixel_read.r*255) % widhtOfRange)));
		}
	}
	al_unlock_bitmap(outputBitmap);
}

void extended_ranges_dithering()
{
	al_set_target_bitmap(outputBitmap);
	al_lock_bitmap(outputBitmap, al_get_bitmap_format(outputBitmap), ALLEGRO_LOCK_WRITEONLY);

	ALLEGRO_COLOR pixel_read; 
	unsigned char widhtOfRange = 255 / ditheringVariables[2];

#define rgb(x) al_map_rgb((x), (x), (x))

	for(int y=0; y<YpicSize; y++)
	{
		for(int x=0; x<XpicSize; x++)
		{
			int i = 0, j = 0;
			pixel_read = al_get_pixel(inputBitmap, x, y);
			al_put_pixel(x, y, rgb((unsigned char)(pixel_read.r*255) - ((unsigned char)(pixel_read.r*255) % widhtOfRange)));
		}
	}
	al_unlock_bitmap(outputBitmap);}

void dither(char mode)
{
	switch(mode)
	{
	case 1: threshold_dithering(); break;
	case 2: simple_ranges_dithering(); break;
	case 3: extended_ranges_dithering(); break;
	}
}
//dodajac nowy algorytm: dither(), zmienna wlasna +/-, napis zmiennej
// TODO: algorytm z dzieleniem na sasiadow, lista algorytmow, 

int main()
{


	ditheringVariables[1] = 50;
	ditheringVariables[2] = 15;
	if(!al_init()) 
	{
		al_show_native_message_box(display, "Error", "Error", "Failed to initialize allegro!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	al_init_image_addon();

	inputBitmap = al_load_bitmap("BandW_butterfly.bmp");
	if(!inputBitmap)
	{
		al_show_native_message_box(display, "Error", "Error", "Could not load image neither looking for input.bmp file nor filename given in first program argument.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return 0;
	}
	XpicSize = al_get_bitmap_width(inputBitmap);
	YpicSize = al_get_bitmap_height(inputBitmap);
	initialize_allegro( XpicSize*2+10, YpicSize+300 );

	outputBitmap = al_clone_bitmap(inputBitmap);
	if(!outputBitmap)
	{
		al_show_native_message_box(display, "Error", "Error", "Error creating clone of bitmap.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return 0;
	}

	redraw();

	al_lock_bitmap(inputBitmap, al_get_bitmap_flags(inputBitmap), ALLEGRO_LOCK_READONLY);

	while(1)
	{
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && (ev.keyboard.keycode == ALLEGRO_KEY_Q || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)))
			return 0;  //exit program
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_1:
			case ALLEGRO_KEY_PAD_1: 
					dithering_mode = 1; // threshold mode
					break;
			case ALLEGRO_KEY_2:
			case ALLEGRO_KEY_PAD_2:
					dithering_mode = 2; // simple ranges mode
					break;
			case ALLEGRO_KEY_3:
			case ALLEGRO_KEY_PAD_3:
					dithering_mode = 3; // simple ranges mode
					break;
	/*		case ALLEGRO_KEY_4:
			case ALLEGRO_KEY_PAD_4:
			case ALLEGRO_KEY_5:
			case ALLEGRO_KEY_PAD_5:
			case ALLEGRO_KEY_6:
			case ALLEGRO_KEY_PAD_6: */
			case ALLEGRO_KEY_PAD_PLUS: 
				switch(dithering_mode)
				{
				case 1: ditheringVariables[1]+=3; break;
				case 3:
				case 2: if(ditheringVariables[2] != 255) ditheringVariables[2]++; break;
				}				
			break;
			case ALLEGRO_KEY_PAD_MINUS: 
				switch(dithering_mode)
				{
				case 1: ditheringVariables[1]-=3; break;
				case 3: 
				case 2: if(ditheringVariables[2] != 0) ditheringVariables[2]--; break;
				}
			break;

			}
			dither(dithering_mode);
			redraw();
			//al_rest(0.1); 
		}
	}

}
