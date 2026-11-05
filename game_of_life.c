#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

#include <gtk/gtk.h>

// just a function to test buttons
static void print_ah_hummus(GtkWidget *widget, gpointer data) {
	g_print("ah, hummus.\n");
}

static void init_window(GtkApplication *app, gpointer user_data) {
	GtkWidget *window; // declare window inside application
	GtkWidget *button; // declare button inside application
	GtkWidget *box; // declare box inside application (to put buttons in)
	
	window = gtk_application_window_new(app); // initialize window
	gtk_window_set_title(GTK_WINDOW(window), "game of life"); // set title
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 500); // set size (in pixels)
	
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); // initialize box (horizontal buttons 0 pixels apart)
	gtk_widget_set_halign(box, GTK_ALIGN_CENTER); // horizontally align box to center
	gtk_widget_set_valign(box, GTK_ALIGN_CENTER); // vertically align box to center
	
	gtk_window_set_child(GTK_WINDOW(window), box); // adds box to window
	
	button = gtk_button_new_with_label("hummus?"); // initialize button
	
	g_signal_connect(button, "clicked", G_CALLBACK(print_ah_hummus), NULL); // connect clicked signal of button with print_ah_hummus (which has no parameters, so null is passed)
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy), window); // connect clicked signal of button with eating the fucking window
	
	gtk_box_append(GTK_BOX(box), button); // adds button to box
	
	gtk_window_present(GTK_WINDOW(window)); // make sure user can see window by forcing it to the top of everything
}

int main(int argc, char** argv) {
	GtkApplication *game_of_life; // declare application
	int status;
	
	game_of_life = gtk_application_new("org.gtk.game_of_life", G_APPLICATION_DEFAULT_FLAGS); // initialize application
	g_signal_connect(game_of_life, "activate", G_CALLBACK(init_window), NULL); // connect activate signal with init_window (last argument to init_window was null)
	status = g_application_run(G_APPLICATION(game_of_life), argc, argv); // launch application
	g_object_unref(game_of_life); // free memory of application
	
	return status;
}
