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
static guint timer_id = 0; // id of the timer when simulation is automatically updating

// variables to store cell mouse is in when held down (for interpolation)
int mouse_row = -1;
int mouse_col = -1;

// function to reset grid
static void reset_grid(GtkButton *button, gpointer data) {
	if (timer_id != 0) g_source_remove(timer_id); timer_id = 0; // if timer is going, reset it
	GtkWidget *area = GTK_WIDGET(data);
	for (int r = 0; r < 25; r++) for (int c = 0; c < 25; c++) grid_data[r][c] = 0;
	gtk_widget_queue_draw(area);
}

// draw line from one cell to the next using interpolation (using bresenham's algorithm)
static void draw_line(GtkWidget *area, int row1, int col1, int row2, int col2) {
	// total change in x and y (1 unit = 1 cell)
	int dx = abs(col2 - col1);
	int dy = abs(row2 - row1);
	
	// determine whether line is going up/down and left/right
	int sx = ((col1 < col2) ? 1 : -1); // 1 = right; -1 = left
	int sy = ((row1 > row2) ? 1 : -1); // 1 = up; -1 = down
	
	int dev = dx - dy; // deviation from the perfect line between the 2 points
	
	while (1) { // keep moving until last cell is reached and pray i didnt fuck up so it doesnt inf loop
		if (row1 >= 0 && row1 < 25 && col1 >= 0 && col1 < 25) grid_data[row1][col1] = draw_cell_type; // change current cell
		if (row1 == row2 && col1 == col2) break; // break if we're at the second point
		// decide where to move next
		int dev2 = dev * 2;
		if (dev2 > -dy) { // move horizontally?
			dev -= dy;
			col1 += sx;
		}
		if (dev2 < dx) { // move vertically?
			dev += dx;
			row1 -= sy;
		}
	}
	
	gtk_widget_queue_draw(area);
}

// just a function to test buttons
static void print_owo(GtkWidget *widget, gpointer data) {
	g_print("owo\n");
}

// who remembers turtle from python :D
static void draw_grid(GtkDrawingArea *area, cairo_t *pen, int w, int h, gpointer data) {
        // draw cells
        for (int row = 0; row < 25; row++) {
                for (int col = 0; col < 25; col++) {
                        // if cell is off, make it gray; if cell is on, make it green
                        if (grid_data[row][col] == 0) cairo_set_source_rgb(pen, 0.6, 0.6, 0.6);
                        else if (grid_data[row][col] == 1) cairo_set_source_rgb(pen, 0, 0.6, 0);
                        else cairo_set_source_rgb(pen, 1, 0.9, 0.1); // if yellow is in the grid something has gone horribly wrong
			
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

// when cell is clicked, set is_drawing to true and run change_cell
static void cell_click(GtkGestureClick *gesture, int num_clicks, double x, double y, gpointer data) {
	if (timer_id != 0) return; // cant draw while simulation is moving
	
	GtkWidget *area = GTK_WIDGET(data); // find which widget click was on
	
	// find row and column of cell that was clicked
	int row = (y / (gtk_widget_get_height(area) / 25.0));
	int col = (x / (gtk_widget_get_width(area) / 25.0));
	
	if (row >= 0 && row < 25 && col >= 0 && col < 25) { // make sure cell is in grid
		currently_drawing = true; // drawing is happening
		
		// update mouse positions
		mouse_row = row;
		mouse_col = col;
		
		draw_cell_type = !grid_data[row][col]; // set the type of cell we're drawing
		
		grid_data[row][col] = draw_cell_type; // change cell that was clicked
		gtk_widget_queue_draw(area); // redraw area asap
	}
}

// when click is released, set is_drawing to false again
static void cell_release(GtkGestureClick *gesture, int num_clicks, double x, double y, gpointer data) {
	currently_drawing = false;
	// mouse is not held down
	mouse_row = -1;
	mouse_col = -1;
}

// when mouse moves while is_drawing is true, change cell(s) underneath mouse
static void mouse_moving(GtkEventControllerMotion *controller, double x, double y, gpointer data) {
	if (currently_drawing && timer_id == 0) {
		GtkWidget *area = GTK_WIDGET(data);
		
		int row = (y / (gtk_widget_get_height(area) / 25.0));
        	int col = (x / (gtk_widget_get_width(area) / 25.0));
		
		// force coordinates to be in grid
		if (row < 0) row = 0; if (row >= 25) row = 24;
		if (col < 0) col = 0; if (col >= 25) col = 24;
		
		draw_line(area, mouse_row, mouse_col, row, col); // draw line between where mouse was and where mouse is now)
		// update mouse positions
		mouse_row = row;
		mouse_col = col;
	}
}

// actual game of life code that goes to next step
static void next_step() {
	int next_grid[25][25] = {0}; // where next frame goes
	
	for (int row = 0; row < 25; row++) {
		for (int col = 0; col < 25; col++) {
			int neighbors = 0; // number of alive neighbors
			
			// check 8 neighboring cells 
			for (int n_row = row - 1; n_row <= row + 1; n_row++) {
				for (int n_col = col - 1; n_col <= col + 1; n_col++) {
					if (n_row == row && n_col == col) continue; // skip the cell itself
					
					// if neighbor is in grid, add it to next grid
					if (n_row >= 0 && n_row < 25 && n_col >= 0 && n_col < 25) neighbors += grid_data[n_row][n_col];
				}
			}
			
			// actual game of life rules, deciding what cells change
			if (grid_data[row][col] == 1) {
				if (neighbors == 2 || neighbors == 3) next_grid[row][col] = 1;
				else next_grid[row][col] = 0;
			}
			else if (grid_data[row][col] == 0) {
				if (neighbors == 3) next_grid[row][col] = 1;
			}
			else next_grid[row][col] = 2; // should never happen
		}
	}
	
	// when done, update original grid data
	for (int r = 0; r < 25; r++) for (int c = 0; c < 25; c++) grid_data[r][c] = next_grid[r][c];
}

// next step but with arguments for step button callback
static void step(GtkButton *button, gpointer data) {
	if (timer_id != 0) return; // cant step while simulation is moving
	GtkWidget *area = GTK_WIDGET(data);
	next_step();
	gtk_widget_queue_draw(area);
}

// update grid when half a second has passed
static gboolean update_grid(gpointer data) {
	GtkWidget *area = GTK_WIDGET(data);
	next_step();
	gtk_widget_queue_draw(area);
	return G_SOURCE_CONTINUE; // keep running timer
}

// start automatic grid updating
static void start_grid(GtkButton *button, gpointer data) {
	// make sure only one timer is going at a time
	if (timer_id == 0) timer_id = g_timeout_add(200, update_grid, data);
}

// stop automatic grid updating
static void stop_grid(GtkButton *button, gpointer data) {
	if (timer_id != 0) { // make sure timer is going
		// stop and reset timer
		g_source_remove(timer_id);
		timer_id = 0;
	}
}

static void init_window(GtkApplication *app, gpointer user_data) {	
	GtkWidget *window = gtk_application_window_new(app); // initialize window
	gtk_window_set_title(GTK_WINDOW(window), "game of life"); // set title
	gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000); // set size (in pixels)
	
	GtkWidget *drawing_area = gtk_drawing_area_new(); // initialize drawing area (that will overlap with grid)
	
	GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // initialize main box that will have box and grid
	gtk_widget_set_margin_top(main_box, 10); // put some padding on top
	gtk_widget_set_margin_bottom(main_box, 10); // put some padding on bottom
	gtk_window_set_child(GTK_WINDOW(window), main_box); // put main box in window
	
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10); // initialize box to put buttons in (horizontal buttons 10 pixels apart)
	gtk_widget_set_halign(box, GTK_ALIGN_CENTER); // horizontally align box to center
	gtk_widget_set_valign(box, GTK_ALIGN_START); // vertically align box to top
	gtk_box_append(GTK_BOX(main_box), box); // adds box to main box
	
	GtkWidget *start_button = gtk_button_new_with_label("start");
	g_signal_connect(start_button, "clicked", G_CALLBACK(start_grid), drawing_area);
	gtk_box_append(GTK_BOX(box), start_button);
	
	GtkWidget *stop_button = gtk_button_new_with_label("stop");
	g_signal_connect(stop_button, "clicked", G_CALLBACK(stop_grid), drawing_area);
	gtk_box_append(GTK_BOX(box), stop_button);
	
	GtkWidget *step_button = gtk_button_new_with_label("step");
	g_signal_connect(step_button, "clicked", G_CALLBACK(step), drawing_area);
	gtk_box_append(GTK_BOX(box), step_button);
	
	GtkWidget *reset_button = gtk_button_new_with_label("reset");
	g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_grid), drawing_area);
	gtk_box_append(GTK_BOX(box), reset_button);
	
	GtkWidget *important_button = gtk_button_new_with_label("close"); // initialize most important button 
        g_signal_connect(important_button, "clicked", G_CALLBACK(print_owo), NULL); // when button is clicked, run  print_ah_hummus (which has no parameters, so null is passed) 
        g_signal_connect_swapped(important_button, "clicked", G_CALLBACK(gtk_window_destroy), window); // when button is clicked, eat the fucking window 
        gtk_box_append(GTK_BOX(box), important_button); // adds button to box
	
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
