#include <pebble.h>

#define BACKGROUND_COLOR GColorVividCerulean

#define IMAGE_GROUND_ORIGIN_X 0
#define IMAGE_GROUND_ORIGIN_Y 147

#ifdef PBL_PLATFORM_BASALT
	#define IMAGE_DUCK_ORIGIN_X 3
	
	#define TEXT_HOURS_ORIGIN_X 93
	#define TEXT_HOURS_ORIGIN_Y (IMAGE_DUCK_ORIGIN_Y + 10)

	#define TEXT_MINUTES_ORIGIN_X TEXT_HOURS_ORIGIN_X
	#define TEXT_MINUTES_ORIGIN_Y (IMAGE_DUCK_ORIGIN_Y + 70)
#endif

#ifdef PBL_PLATFORM_CHALK
	#define IMAGE_DUCK_ORIGIN_X 41
	
	#define TEXT_HOURS_ORIGIN_X (IMAGE_DUCK_ORIGIN_X - 29)
	#define TEXT_HOURS_ORIGIN_Y (IMAGE_DUCK_ORIGIN_Y + 41)

	#define TEXT_MINUTES_ORIGIN_X (IMAGE_DUCK_ORIGIN_X + 84)
	#define TEXT_MINUTES_ORIGIN_Y (IMAGE_DUCK_ORIGIN_Y + 57)
#endif

#define IMAGE_DUCK_ORIGIN_Y (IMAGE_GROUND_ORIGIN_Y - 136)

#define IMAGE_EYES_LOOK_LEFT_ORIGIN_X 35
#define IMAGE_EYES_LOOK_RIGHT_ORIGIN_X 41
#define IMAGE_EYES_ORIGIN_Y 12

#define EYES_ANIM_DURATION 200
#define DUCK_IN_ANIM_DURATION 1000
#define DUCK_IN_ANIM_DELAY 200

static Window *main_window;

static GBitmap *ground_bitmap;
static GBitmap *duck_bitmap;
static GBitmap *eyes_bitmap;

static BitmapLayer *ground_layer;
static BitmapLayer *duck_layer;
static BitmapLayer *eyes_layer;

static TextLayer *hours_layer;
static TextLayer *minutes_layer;

static GRect ground_bounds;
static GRect duck_bounds;
static GRect duck_bounds_off_screen;
static GRect eyes_bounds_look_left;
static GRect eyes_bounds_look_right;

static void anim_eyes_look_right() {
	Layer *layer = bitmap_layer_get_layer(eyes_layer);
	GRect bounds_from = layer_get_frame(layer);

	PropertyAnimation *eyes_look_right_prop_anim = property_animation_create_layer_frame(layer, &bounds_from, &eyes_bounds_look_right);
	Animation *eyes_look_right_anim = property_animation_get_animation(eyes_look_right_prop_anim);
	animation_set_curve(eyes_look_right_anim, AnimationCurveEaseInOut);
	animation_set_duration(eyes_look_right_anim, EYES_ANIM_DURATION);

	animation_schedule(eyes_look_right_anim);
}

static void anim_eyes_look_left() {
	Layer *layer = bitmap_layer_get_layer(eyes_layer);
	GRect bounds_from = layer_get_frame(layer);

	PropertyAnimation *eyes_look_left_prop_anim = property_animation_create_layer_frame(layer, &bounds_from, &eyes_bounds_look_left);
	Animation *eyes_look_left_anim = property_animation_get_animation(eyes_look_left_prop_anim);
	animation_set_curve(eyes_look_left_anim, AnimationCurveEaseInOut);
	animation_set_duration(eyes_look_left_anim, EYES_ANIM_DURATION);

	animation_schedule(eyes_look_left_anim);
}

static void anim_duck_in() {
	PropertyAnimation *duck_in_prop_anim = property_animation_create_layer_frame(bitmap_layer_get_layer(duck_layer), &duck_bounds_off_screen, &duck_bounds);
	Animation *duck_in_anim = property_animation_get_animation(duck_in_prop_anim);
	animation_set_curve(duck_in_anim, AnimationCurveEaseOut);
	animation_set_delay(duck_in_anim, DUCK_IN_ANIM_DELAY);
	animation_set_duration(duck_in_anim, DUCK_IN_ANIM_DURATION);

	animation_schedule(duck_in_anim);
}

static void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);

	static char hours_buffer[3];
	static char minutes_buffer[3];

	strftime(hours_buffer, sizeof(hours_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
	strftime(minutes_buffer, sizeof(minutes_buffer), "%M", tick_time);

	//Update time text
	text_layer_set_text(hours_layer, hours_buffer);
	text_layer_set_text(minutes_layer, minutes_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	if(tick_time->tm_min%2 == 0) {
		anim_eyes_look_right();
	} else {
		anim_eyes_look_left();
	}
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(main_window);

  	window_set_background_color(main_window, BACKGROUND_COLOR);

  	//Loading bitmaps
	ground_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GROUND);
	duck_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DUCK);
	eyes_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EYES);

	//Compute bounds
	ground_bounds = GRect(
		IMAGE_GROUND_ORIGIN_X,
		IMAGE_GROUND_ORIGIN_Y,
		gbitmap_get_bounds(ground_bitmap).size.w,
		gbitmap_get_bounds(ground_bitmap).size.h
	);
	duck_bounds = GRect(
		IMAGE_DUCK_ORIGIN_X,
		IMAGE_DUCK_ORIGIN_Y,
		gbitmap_get_bounds(duck_bitmap).size.w,
		gbitmap_get_bounds(duck_bitmap).size.h
	);
	duck_bounds_off_screen = GRect(
		IMAGE_DUCK_ORIGIN_X,
		IMAGE_GROUND_ORIGIN_Y,
		gbitmap_get_bounds(duck_bitmap).size.w,
		gbitmap_get_bounds(duck_bitmap).size.h
	);
	eyes_bounds_look_right = GRect(
		IMAGE_EYES_LOOK_RIGHT_ORIGIN_X,
		IMAGE_EYES_ORIGIN_Y,
		gbitmap_get_bounds(eyes_bitmap).size.w,
		gbitmap_get_bounds(eyes_bitmap).size.h
	);
	eyes_bounds_look_left = GRect(
		IMAGE_EYES_LOOK_LEFT_ORIGIN_X,
		IMAGE_EYES_ORIGIN_Y,
		gbitmap_get_bounds(eyes_bitmap).size.w,
		gbitmap_get_bounds(eyes_bitmap).size.h
	);

	//Create bitmap layers
	ground_layer = bitmap_layer_create(ground_bounds);
	duck_layer = bitmap_layer_create(duck_bounds_off_screen);
	eyes_layer = bitmap_layer_create(eyes_bounds_look_right);

	//Set bitamp to layers
	bitmap_layer_set_bitmap(ground_layer, ground_bitmap);
	bitmap_layer_set_bitmap(duck_layer, duck_bitmap);
	bitmap_layer_set_bitmap(eyes_layer, eyes_bitmap);

	//Set transparency
	bitmap_layer_set_compositing_mode(duck_layer, GCompOpSet);
	bitmap_layer_set_compositing_mode(eyes_layer, GCompOpSet);

	//Create hours text layers
	hours_layer = text_layer_create(GRect(TEXT_HOURS_ORIGIN_X, TEXT_HOURS_ORIGIN_Y, 60, 60));
	text_layer_set_background_color(hours_layer, GColorClear);
  	text_layer_set_text_color(hours_layer, GColorBlack);
  	text_layer_set_font(hours_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
	text_layer_set_text(hours_layer, "00");

	//Create minutes text layers
	minutes_layer = text_layer_create(GRect(TEXT_MINUTES_ORIGIN_X, TEXT_MINUTES_ORIGIN_Y, 60, 60));
	text_layer_set_background_color(minutes_layer, GColorClear);
  	text_layer_set_text_color(minutes_layer, GColorBlack);
  	text_layer_set_font(minutes_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
	text_layer_set_text(minutes_layer, "00");

	//Add bitmap layers
	layer_add_child(window_layer, bitmap_layer_get_layer(duck_layer));
	layer_add_child(bitmap_layer_get_layer(duck_layer), bitmap_layer_get_layer(eyes_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(ground_layer));

	//Add text layers
	layer_add_child(window_layer, text_layer_get_layer(hours_layer));
	layer_add_child(window_layer, text_layer_get_layer(minutes_layer));

	anim_duck_in();
}

static void main_window_unload(Window *window) {
	//Unload text layers
	text_layer_destroy(hours_layer);
	text_layer_destroy(minutes_layer);

	//Unload bitmap layers
	bitmap_layer_destroy(ground_layer);
	bitmap_layer_destroy(duck_layer);
	bitmap_layer_destroy(eyes_layer);

	//Unload bitmap
	gbitmap_destroy(ground_bitmap);
	gbitmap_destroy(duck_bitmap);
	gbitmap_destroy(eyes_bitmap);
}

static void init(void) {
	main_window = window_create();
	window_set_window_handlers(main_window, (WindowHandlers) {
	    .load = main_window_load,
	    .unload = main_window_unload
	});
	window_stack_push(main_window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	update_time();
}

static void deinit(void) {
	window_destroy(main_window);
}

int main(void) {
	init();
  	app_event_loop();
  	deinit();
}