#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>

#include <gtk/gtk.h>

int grid_data[25][25] = {0}; // declare a 2d array of cells
bool currently_drawing = false; // whether mouse is held down on grid
int draw_cell_type; // whether we're painting dead or alive cells

// just a function to test buttons
static void print_ah_hummus(GtkWidget *widget, gpointer data) {
	g_print("ah, hummus.\n");
}

// who remembers turtle from python :D
static void draw_grid(GtkDrawingArea *area, cairo_t *pen, int w, int h, gpointer data) {
        // draw cells
        for (int row = 0; row < 25; row++) {
                for (int col = 0; col < 25; col++) {
                        // if cell is off, make it gray; if cell is on, make it green
                        if (grid_data[row][col] == 0) cairo_set_source_rgb(pen, 0.6, 0.6, 0.6);
                        else if (grid_data[row][col] == 1) cairo_set_source_rgb(pen, 0, 0.6, 0);
                        else cairo_set_source_rgb(pen, 1, 0.9, 0.1);
			
                        // draw cell
                        cairo_rectangle(pen, (col * w / 25), (row * h / 25), (w / 25), (h / 25));
                        // fill cell
                        cairo_fill_preserve(pen);
			
                        // draw cell border
                        cairo_set_source_rgb(pen, 1, 1, 1);
                        cairo_stroke(pen);
                }
        }
}

// function that changes cell at location
static void change_cell(GtkWidget *area, double x, double y) {
        // find row and column of cell that was clicked
        int row = (y / (gtk_widget_get_height(area) / 25));
        int col = (x / (gtk_widget_get_width(area) / 25));
	
        if (row >= 0 && row <= 25 && col >= 0 && col <= 25) { // make sure cell is in grid
                grid_data[row][col] = draw_cell_type; // change int value of that cell to the current type of cell we're drawing
                gtk_widget_queue_draw(area); // redraw grid when function ends
        }
}

// when cell is clicked, set is_drawing to true and run change_cell
static void cell_click(GtkGestureClick *gesture, int num_clicks, double x, double y, gpointer data) {
	GtkWidget *area = GTK_WIDGET(data); // find which widget click was on
	currently_drawing = true; // drawing is happening
	
	// find row and column of cell that was clicked
	int row = (y / (gtk_widget_get_height(area) / 25));
	int col = (x / (gtk_widget_get_width(area) / 25));
	
	if (row >= 0 && row <= 25 && col >= 0 && col <= 25) { // make sure cell is in grid
		draw_cell_type = !grid_data[row][col]; // set the type of cell we're drawing
		change_cell(area, x, y); // change cell
	}
}

// when click is released, set is_drawing to false again
static void cell_release(GtkGestureClick *gesture, int num_clicks, double x, double y, gpointer data) {
	currently_drawing = false;
}

// when mouse moves while is_drawing is true, change cell underneath mouse
static void mouse_moving(GtkEventControllerMotion *controller, double x, double y, gpointer data) {
	if (currently_drawing) {
		GtkWidget *area = GTK_WIDGET(data);
		change_cell((area), x, y);
	}
}

static void init_window(GtkApplication *app, gpointer user_data) {	
	GtkWidget *window = gtk_application_window_new(app); // initialize window
	gtk_window_set_title(GTK_WINDOW(window), "game of life"); // set title
	gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000); // set size (in pixels)
	
	GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // initialize main box that will have box and grid
	gtk_widget_set_margin_top(main_box, 10); // put some padding on top
	gtk_widget_set_margin_bottom(main_box, 10); // put some padding on bottom
	gtk_window_set_child(GTK_WINDOW(window), main_box); // put main box in window
	
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10); // initialize box to put buttons in (horizontal buttons 10 pixels apart)
	gtk_widget_set_halign(box, GTK_ALIGN_CENTER); // horizontally align box to center
	gtk_widget_set_valign(box, GTK_ALIGN_START); // vertically align box to top
	gtk_box_append(GTK_BOX(main_box), box); // adds box to main box
	
	GtkWidget *start_button = gtk_button_new_with_label("start");
	gtk_box_append(GTK_BOX(box), start_button);
	
	GtkWidget *stop_button = gtk_button_new_with_label("stop");
	gtk_box_append(GTK_BOX(box), stop_button);
	
	GtkWidget *step_button = gtk_button_new_with_label("step");
	gtk_box_append(GTK_BOX(box), step_button);
	
	GtkWidget *important_button = gtk_button_new_with_label("hummus?"); // initialize most important button 
        g_signal_connect(important_button, "clicked", G_CALLBACK(print_ah_hummus), NULL); // when button is clicked, run  print_ah_hummus (which has no parameters, so null is passed) 
        g_signal_connect_swapped(important_button, "clicked", G_CALLBACK(gtk_window_destroy), window); // when button is clicked, eat the fucking window 
        gtk_box_append(GTK_BOX(box), important_button); // adds button to box
	
	GtkWidget *drawing_area = gtk_drawing_area_new(); // initialize drawing area (that will overlap with grid)
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_grid, NULL, NULL); // connect drawing area with function that draws grid
	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), 725); // set drawing area width
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), 725); // set drawing area height
	gtk_widget_set_halign(drawing_area, GTK_ALIGN_CENTER); // horizontally align drawing area to center
	gtk_widget_set_valign(drawing_area, GTK_ALIGN_CENTER); // vertically align drawing area to center
	gtk_widget_set_hexpand(drawing_area, TRUE); // allow drawing area to expand horizontally
	gtk_widget_set_vexpand(drawing_area, TRUE); // allow drawing area to expand vertically
	gtk_box_append(GTK_BOX(main_box), drawing_area); // adds drawing area to main box
	
	GtkGesture *click = gtk_gesture_click_new(); // initialize a new gesuture that recognizes clicks
	g_signal_connect(click, "pressed", G_CALLBACK(cell_click), drawing_area); // when user clicks on drawing area, run cell_click
	g_signal_connect(click, "released", G_CALLBACK(cell_release), drawing_area); // when user releases on drawing area, run cell_release
	gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(click)); // adds "click" signal to drawing area
	
	GtkEventController *movement = gtk_event_controller_motion_new(); // initialize a new controller that recognizes mouse movement
	g_signal_connect(movement, "motion", G_CALLBACK(mouse_moving), drawing_area); // when mouse is moving on drawing area, run mouse_moving
	gtk_widget_add_controller(drawing_area, movement); // adds "moving" signal to drawing area
	
	gtk_window_present(GTK_WINDOW(window)); // make sure user can see window by forcing it to the top of everything
}

int main(int argc, char** argv) {
	GtkApplication *game_of_life; // declare application
	int status; // declare status
	
	game_of_life = gtk_application_new("org.gtk.game_of_life", G_APPLICATION_DEFAULT_FLAGS); // initialize application
	g_signal_connect(game_of_life, "activate", G_CALLBACK(init_window), NULL); // connect activate signal with init_window (last argument to init_window was null)
	status = g_application_run(G_APPLICATION(game_of_life), argc, argv); // launch application
	g_object_unref(game_of_life); // free memory of application
	
	return status;
}
