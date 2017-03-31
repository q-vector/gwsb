#include "selection.h"
#include "data.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Selection_Panel::Month_Map::Month_Map ()
{
   insert (make_pair ("Jan", 1));
   insert (make_pair ("Feb", 2));
   insert (make_pair ("Mar", 3));
   insert (make_pair ("Apr", 4));
   insert (make_pair ("May", 5));
   insert (make_pair ("Jun", 6));
   insert (make_pair ("Jul", 7));
   insert (make_pair ("Aug", 8));
   insert (make_pair ("Sep", 9));
   insert (make_pair ("Oct", 10));
   insert (make_pair ("Nov", 11));
   insert (make_pair ("Dec", 12));
}

Integer
Selection_Panel::Month_Map::get_integer (const Dstring& str) const
{
   auto i = this->find (str);
   if (i != end ()) { return i->second; }
   return -1;
}

Integer
Selection_Panel::Status::number_on () const
{
   Integer n;
   for (auto& i : *this) { if (i.second) { n++; } }
   return n;
}

bool
Selection_Panel::Status::zero_on () const
{
   return (number_on () == 0);
}

bool
Selection_Panel::Status::one_on () const
{
   return (number_on () == 1);
}

const Dstring&
Selection_Panel::Status::last () const
{
   for (auto i = rbegin (); i != rend (); i++)
   {
      if (i->second) { return i->first; }
   }
}

const Dstring&
Selection_Panel::Status::first () const
{
   for (auto& i : *this) { if (i.second) { return i.first; } }
}

Selection_Panel::Selection_Panel (Gwsb_Free& gwsb_free,
                                  const Real margin,
                                  const Real spacing)
   : Dpack_Box (gwsb_free, margin, spacing, false),
     gwsb_free (gwsb_free),
     led_color (1, 0, 0),
     box (gwsb_free, 0, 6, true),
     decrement_button (gwsb_free, "-", 12),
     increment_button (gwsb_free, "+", 12)
{

   box.pack_back (decrement_button);
   box.pack_back (increment_button);
   pack_back (box);
}

Selection_Panel::~Selection_Panel ()
{
   for (auto& i : button_ptr_map) { delete i.second; }
}

void
Selection_Panel::handle (const Dstring& str,
                         const Devent& event)
{
   if (event.control ()) { toggle (str); }
   else { set_value (str); }
}

void
Selection_Panel::set_value (const Dstring& str,
                            const bool render_and_refresh)
{

   for (auto& i : status)
   {

      const Dstring& s = i.first;
      bool& is_on = i.second;
      is_on = (s == str);

      Dbutton& button = *(button_ptr_map[s]);
      if (is_on) { button.set_led_color (led_color); }
      else { button.set_led_color (Color (GSL_NAN, GSL_NAN, GSL_NAN)); }

   }

   if (render_and_refresh)
   {
      gwsb_free.render ();
      gwsb_free.queue_draw ();
   }

}

void
Selection_Panel::toggle (const Dstring& str)
{

   bool& is_on = status[str];
   is_on = !is_on;

   Dbutton& button = *(button_ptr_map[str]);
   if (is_on) { button.set_led_color (led_color); }
   else { button.set_led_color (Color (GSL_NAN, GSL_NAN, GSL_NAN)); }

   gwsb_free.render ();
   gwsb_free.queue_draw ();

}

void
Selection_Panel::increment ()
{
   if (status.zero_on ()) { return; }
   auto iterator = (month_map.find (status.last ()))++;
   if (iterator == month_map.end ()) { iterator = month_map.begin (); }
   set_value (iterator->first);
}

void
Selection_Panel::decrement ()
{
   if (status.zero_on ()) { return; }
   auto iterator = month_map.find (status.first ());
   if (iterator == month_map.begin ()) { iterator = month_map.end (); }
   iterator--;
   set_value (iterator->first);
}

Month_Panel::Month_Panel (Gwsb_Free& gwsb_free,
                          const Real margin,
                          const Real spacing)
   : Selection_Panel (gwsb_free, margin, spacing)
{

   for (auto& i : month_map)
   {
      const Dstring& str = i.first;
      Dbutton* button_ptr = new Dbutton (gwsb_free, str, 12);
      button_ptr->get_full_str_signal ().connect (sigc::mem_fun (
         *this, &Selection_Panel::handle));
      button_ptr_map.insert (make_pair (str, button_ptr));
      pack_back (*button_ptr);
      status.insert (make_pair (str, false));
   }

   decrement_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::decrement_month));
   increment_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::increment_month));

}

string
Month_Panel::get_string () const
{

   Dstring s;

   for (auto& i : status)
   {
      const Dstring& str = i.first;
      const bool is_on = i.second;
      if (is_on) { s += ":" + str; }
   }

   return s.substr (1);

/*
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
*/

}

Hour_Panel::Hour_Panel (Gwsb_Free& gwsb_free,
                        const Real margin,
                        const Real spacing)
   : Selection_Panel (gwsb_free, margin, spacing)
{

   for (Integer hour = 0; hour < 24; hour++)
   {
      const Dstring& str = Dstring::render ("%02dZ", hour);
      Dbutton* button_ptr = new Dbutton (gwsb_free, str, 12);
      button_ptr->get_full_str_signal ().connect (sigc::mem_fun (
         *this, &Selection_Panel::handle));
      button_ptr_map.insert (make_pair (str, button_ptr));
      pack_back (*button_ptr);
      status.insert (make_pair (str, false));
   }

   decrement_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::decrement_hour));
   increment_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::increment_hour));

}

string
Hour_Panel::get_string () const
{

   Dstring s;

   for (auto& i : status)
   {
      const Dstring& str = i.first;
      const bool is_on = i.second;
      if (is_on) { s += ":" + str; }
   }

   return s.substr (1);

/*
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
*/

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

   Integer id = 0;
   const Integer n = 9;

   for (const string& station : station_tokens)
   {
      const Integer i = id % n;
      const Integer j = id / n;
      Dbutton* button_ptr = new Dbutton (gwsb, station, 12);
      button_ptr_map.insert (make_pair (station, button_ptr));
      button_ptr->get_str_signal ().connect (sigc::mem_fun (
         *this, &Station_Panel::set_station));
      pack (*button_ptr, Index_2D (i, j));
      id++;
   }

}

Station_Panel::~Station_Panel ()
{
   for (auto& i : button_ptr_map) { delete i.second; }
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

