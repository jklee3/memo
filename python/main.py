#!/usr/bin/env python

from gtk import *
from gobject import *

class Memo:
	def __init__(self):
		self.window = Window(WINDOW_TOPLEVEL)
		self.window.set_title ("Memo")
		self.window.show ()
		self.window.connect ("destroy", self.destroy)

		self.main_hbox = HBox (False, 0)
		self.main_hbox.show ()
		self.window.add (self.main_hbox)

		self.left_vbox = VBox (False, 0)
		self.left_vbox.show ()
		self.main_hbox.pack_start (self.left_vbox, False, False, 0)

		self.calendar = Calendar ()
		self.calendar.show ()
		self.calendar.connect ("day-selected", self.calendar_selected)
		self.left_vbox.pack_start (self.calendar, False, False, 0)

		self.sticky_label = Label ("Sticky Memo")
		self.sticky_label.show ()
		self.left_vbox.pack_start (self.sticky_label, False, False, 0)

		self.main_text_tag_table = TextTagTable ()
		self.main_text_buffer = TextBuffer (self.main_text_tag_table)
		self.main_text_view = TextView (self.main_text_buffer)
		self.main_text_view.show ()
		self.main_text_view.set_size_request (300, 300)
		self.left_vbox.pack_start (self.main_text_view, False, False, 0)

		self.daily_label = Label ("Daily Memo")
		self.daily_label.show ()
		self.left_vbox.pack_start (self.daily_label, False, False, 0)
		
		self.daily_text_tag_table = TextTagTable ()
		self.daily_text_buffer = TextBuffer (self.daily_text_tag_table)
		self.daily_text_view = TextView (self.daily_text_buffer)
		self.daily_text_view.show ()
		self.daily_text_view.set_size_request (300, 300)
		self.left_vbox.pack_start (self.daily_text_view, False, False, 0)

		self.button_box = HBox (False, 0)
		self.button_box.show ()
		self.left_vbox.pack_start (self.button_box, False, False, 0)
		
		self.save_button = Button ("Save")
		self.save_button.show ()
		self.save_button.connect ("clicked", self.save_button_clicked)
		self.button_box.pack_start (self.save_button, True, True, 0)
		
		self.edit_button = Button ("Edit")
		self.edit_button.show ()
		self.edit_button.connect ("clicked", self.edit_button_clicked)
		self.button_box.pack_start (self.edit_button, True, True, 0)
		
		self.right_vbox = VBox (False, 0)
		self.right_vbox.show ()
		self.main_hbox.pack_start (self.right_vbox, True, True, 0)
		
		self.search_frame = Frame ("Search")
		self.search_frame.show ()
		self.right_vbox.pack_start (self.search_frame, False, False, 0)
		
		self.search_hbox = HBox (False, 0)
		self.search_hbox.show ()
		self.search_frame.add (self.search_hbox)
		
		self.search_keyword_label = Label ("Keyword :")
		self.search_keyword_label.show ()
		self.search_hbox.pack_start (self.search_keyword_label, True, True, 0)
		self.search_entry = Entry ()
		self.search_entry.show ()
		self.search_hbox.pack_start (self.search_entry, True, True, 0)

		self.search_button = Button ("Show")
		self.search_button.show ()
		self.search_button.connect ("clicked", self.search_button_clicked)
		self.search_hbox.pack_start (self.search_button, True, True, 0)

		self.list_frame = Frame ("Memos")
		self.list_frame.show ()
		self.list_frame.set_border_width (2)
		self.right_vbox.pack_start (self.list_frame, False, False, 0)

		self.list_vbox = VBox (False, 0)
		self.list_vbox.show ()
		self.list_vbox.set_border_width (3)
		self.list_frame.add (self.list_vbox)

		self.list_swindow = ScrolledWindow ()
		self.list_swindow.show ()
		self.list_swindow.set_size_request (420, 790)
		self.list_swindow.set_policy (POLICY_AUTOMATIC, POLICY_AUTOMATIC)
		self.list_vbox.pack_start (self.list_swindow, False, False, 0)

		self.list_tree_store = TreeStore (TYPE_STRING, TYPE_STRING, TYPE_STRING, TYPE_STRING, TYPE_INT)
		self.list_tree_view = TreeView (self.list_tree_store)
		self.list_tree_view.show ()
		self.list_tree_view.columns_autosize ()
		self.list_tree_view.set_rules_hint (True)
		self.list_tree_view.set_headers_visible (True)

		self.list_tree_view_time_column = TreeViewColumn ()
		self.list_tree_view_time_column.set_title ("Time")
		self.list_tree_view_time_column.set_resizable (False)
		self.list_tree_view_time_column.set_clickable (False)
		self.list_tree_view.append_column (self.list_tree_view_time_column)
		
		self.list_tree_view_time_renderer = CellRendererText ()
		self.list_tree_view_time_column.pack_start (self.list_tree_view_time_renderer, False)
		self.list_tree_view_time_column.add_attribute (self.list_tree_view_time_renderer, "markup", 0)
			
		self.list_tree_view_memo_column = TreeViewColumn ()
		self.list_tree_view_memo_column.set_title ("Time")
		self.list_tree_view_memo_column.set_resizable (False)
		self.list_tree_view_memo_column.set_clickable (False)
		self.list_tree_view.append_column (self.list_tree_view_memo_column)
		
		self.list_tree_view_memo_renderer = CellRendererText ()
		self.list_tree_view_memo_column.pack_start (self.list_tree_view_memo_renderer, False)
		self.list_tree_view_memo_column.add_attribute (self.list_tree_view_memo_renderer, "markup", 3)
		
		self.list_swindow.add (self.list_tree_view)

		self.init_tree_view_time ()
		

	def destroy (self, widget, data=None):
		main_quit ()

	def calendar_selected (self, widget, data=None):
		print "calendar selected"

	def save_button_clicked (self, widget, data=None):
		print "save button clicked"

	def edit_button_clicked (self, widget, data=None):
		print "edit button clicked"
	
	def search_button_clicked (self, widget, data=None):
		print "search button clicked"
	
	def init_tree_view_time (self):
		for x in range (0, 48):
			if (x % 2):
				time = "             30"
			else:
				if (x == 0):
					time = "AM %02d : 00" % (x / 2)
				elif (x == 24):
					time = "FM %02d : 00" % 12
				elif (x > 24):
					time = "      %02d : 00" % ((x - 24) / 2)
				else:
					time = "      %02d : 00" % (x / 2)

			self.list_tree_store.append (None, [time, "", "", "", 0])

	def main (self):
		main ()

if __name__=="__main__":
	memo = Memo ()
	memo.main ()
