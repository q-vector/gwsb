#include "selection.h"
#include "data.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Selection_Panel::Button::Button (Selection_Panel& selection_panel,
                                 const Integer value,
                                 const Gwsb& gwsb,
                                 const string& str,
                                 const Real font_size)
   : Dbutton (gwsb, str, font_size),
     selection_panel (selection_panel),
     value (value)
{
}

void
Selection_Panel::Button::clicked (const Dmouse_Button_Event& event)
{
   const bool control_pressed = (event.state & GDK_CONTROL_MASK);
   if (control_pressed) { selection_panel.toggle (value); }
   else { selection_panel.set_value (value); }
}

Selection_Panel::Selection_Panel (Gwsb& gwsb,
                                  const Real margin,
                                  const Real spacing)
   : Dpack_Box (gwsb, margin, spacing, false),
     gwsb (gwsb),
     led_color (1, 0, 0),
     box (gwsb, 0, 6, true),
     decrement_button (gwsb, "-", 12),
     increment_button (gwsb, "+", 12)
{
   box.pack_back (decrement_button);
   box.pack_back (increment_button);
   pack_back (box);
}

Selection_Panel::~Selection_Panel ()
{

   typedef map<Integer, Dbutton*>::iterator Iterator;
   for (Iterator iterator = button_ptr_map.begin ();
        iterator != button_ptr_map.end (); iterator++)
   {
      Dbutton* button_ptr = iterator->second;
      delete button_ptr;
   }

}

set<Integer>
Selection_Panel::get_value_set () const
{

   std::set<Integer> value_set;

   for (map<Integer, bool>::const_iterator iterator = status_map.begin ();
        iterator != status_map.end (); iterator++)
   {
      const Integer& value = iterator->first;
      const bool& is_on = iterator->second;
      if (is_on) { value_set.insert (value); }
   }

   return value_set;

}

void
Selection_Panel::set_value (const Integer value,
                            const bool render_and_refresh)
{

   for (map<Integer, bool>::iterator iterator = status_map.begin ();
        iterator != status_map.end (); iterator++)
   {

      const Integer v = iterator->first;
      bool& is_on = iterator->second;
      is_on = (v == value);

      Dbutton& button = *(button_ptr_map[v]);
      if (is_on) { button.set_led_color (led_color); }
      else { button.set_led_color (Color (GSL_NAN, GSL_NAN, GSL_NAN)); }

   }

   if (render_and_refresh)
   {
      gwsb.render_refresh ();
   }

}

void
Selection_Panel::toggle (const Integer value)
{

   bool& is_on = status_map[value];
   is_on = !is_on;

   Dbutton& button = *(button_ptr_map[value]);
   if (is_on) { button.set_led_color (led_color); }
   else { button.set_led_color (Color (GSL_NAN, GSL_NAN, GSL_NAN)); }

   gwsb.render ();
   gwsb.queue_draw ();

}

void
Month_Panel::add_month (const Integer month)
{
   const string& str = string_map[month].substr (0, 3);
   Button* button_ptr = new Button (*this, month, gwsb, str, 12);
   button_ptr_map.insert (make_pair (month, button_ptr));
   pack_back (*button_ptr);
}

Month_Panel::Month_Panel (Gwsb& gwsb,
                          const Real margin,
                          const Real spacing)
   : Selection_Panel (gwsb, margin, spacing)
{

   string_map.insert (make_pair (1, "January"));
   string_map.insert (make_pair (2, "Feburary"));
   string_map.insert (make_pair (3, "March"));
   string_map.insert (make_pair (4, "April"));
   string_map.insert (make_pair (5, "May"));
   string_map.insert (make_pair (6, "June"));
   string_map.insert (make_pair (7, "July"));
   string_map.insert (make_pair (8, "August"));
   string_map.insert (make_pair (9, "September"));
   string_map.insert (make_pair (10, "October"));
   string_map.insert (make_pair (11, "November"));
   string_map.insert (make_pair (12, "December"));

   for (Integer month = 1; month <= 12; month++)
   {
      add_month (month);
      status_map.insert (make_pair (month, false));
   }

   decrement_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::decrement_month));
   increment_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::increment_month));

}

string
Month_Panel::get_string () const
{

   const set<Integer>& month_set = get_value_set ();

   if (month_set.size () == 1)
   {
      const Integer month = *(month_set.begin ());
      const string& month_string = string_map.find (month)->second;
      return month_string;
   }
   else
   {
      return "Multiple Months";
   }

}

void
Hour_Panel::add_hour (const Integer hour)
{
   const string& str = string_map[hour];
   Button* button_ptr = new Button (*this, hour, gwsb, str, 12);
   button_ptr_map.insert (make_pair (hour, button_ptr));
   pack_back (*button_ptr);
}

Hour_Panel::Hour_Panel (Gwsb& gwsb,
                        const Real margin,
                        const Real spacing)
   : Selection_Panel (gwsb, margin, spacing)
{

   for (Integer hour = 0; hour < 24; hour++)
   {
      const Dstring& str = Dstring::render ("%02dZ", hour);
      string_map.insert (make_pair (hour, str));
      add_hour (hour);
      status_map.insert (make_pair (hour, false));
   }

   decrement_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::decrement_hour));
   increment_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::increment_hour));

}

string
Hour_Panel::get_string () const
{

   const set<Integer>& hour_set = get_value_set ();

   if (hour_set.size () == 1)
   {
      const Integer hour = *(hour_set.begin ());
      const Dstring& hour_string = Dstring::render ("%02dZ", hour);
      return hour_string;
   }
   else
   {
      return "Multiple Hours";
   }

}

Station_Panel::Button::Button (Station_Panel& station_panel,
                               const string& station,
                               const Gwsb& gwsb,
                               const Real font_size)
   : Dbutton (gwsb, station, font_size),
     station_panel (station_panel),
     station (station)
{
}

void
Station_Panel::Button::clicked (const Dmouse_Button_Event& event)
{
   const bool control_pressed = (event.state & GDK_CONTROL_MASK);
   station_panel.set_station (station);
}

Station_Panel::Station_Panel (Gwsb& gwsb,
                              const Real margin,
                              const Real spacing)
   : Dgrid_Box (gwsb, margin, spacing),
     gwsb (gwsb),
     led_color (1, 0, 0)
{

   const Data& data = gwsb.get_data ();
   const Tokens& station_tokens = data.get_station_tokens ();

   station = station_tokens.front ();

   const Integer n = 8;

   for (Tokens::const_iterator iterator = station_tokens.begin ();
        iterator != station_tokens.end (); iterator++)
   {
      const string& station = *(iterator);
      const Integer id = distance (station_tokens.begin (), iterator);
      const Integer i = id % n;
      const Integer j = id / n;
      Button* button_ptr = new Button (*this, station, gwsb, 12);
      button_ptr_map.insert (make_pair (station, button_ptr));
      pack (*button_ptr, Index_2D (i, j));
   }

}

Station_Panel::~Station_Panel ()
{

   typedef map<string, Dbutton*>::iterator Iterator;
   for (Iterator iterator = button_ptr_map.begin ();
        iterator != button_ptr_map.end (); iterator++)
   {
      Dbutton* button_ptr = iterator->second;
      delete button_ptr;
   }

}

void
Station_Panel::set_station (const string& station)
{
   this->station = station;
   gwsb.render ();
   gwsb.queue_draw ();
}

const string&
Station_Panel::get_station () const
{
   return station;
}

