#include "selection.h"
#include "data.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Selection_Panel::Month_Map::Month_Map ()
   : tokens ("Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec")
{
   insert (make_pair ("Jan", 0));
   insert (make_pair ("Feb", 1));
   insert (make_pair ("Mar", 2));
   insert (make_pair ("Apr", 3));
   insert (make_pair ("May", 4));
   insert (make_pair ("Jun", 5));
   insert (make_pair ("Jul", 6));
   insert (make_pair ("Aug", 7));
   insert (make_pair ("Sep", 8));
   insert (make_pair ("Oct", 9));
   insert (make_pair ("Nov", 10));
   insert (make_pair ("Dec", 11));
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

void
Selection_Panel::Status::set_all (const bool on)
{
   for (auto& i : *this) { i.second = on; }
}

void
Selection_Panel::Status::set (const Tokens& tokens)
{
   set_all (false);
   for (const Dstring& str : tokens)
   {
      try { at (str) = true; } catch (std::out_of_range soer) { } 
   }
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

const Selection_Panel::Status&
Selection_Panel::get_status () const
{
   return status;
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
      gwsb_free.render_queue_draw ();
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

   gwsb_free.render_queue_draw ();

}

Month_Panel::Month_Panel (Gwsb_Free& gwsb_free,
                          const Real margin,
                          const Real spacing)
   : Selection_Panel (gwsb_free, margin, spacing)
{

   for (const Dstring& str : month_map.tokens)
   {
      Dbutton* button_ptr = new Dbutton (gwsb_free, str, 12);
      button_ptr->get_full_str_signal ().connect (sigc::mem_fun (
         *this, &Selection_Panel::handle));
      button_ptr_map.insert (make_pair (str, button_ptr));
      pack_back (*button_ptr);
      status.insert (make_pair (str, false));
   }

   decrement_button.get_signal ().connect (sigc::mem_fun (
      *this, &Month_Panel::decrement));
   increment_button.get_signal ().connect (sigc::mem_fun (
      *this, &Month_Panel::increment));

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

}

void
Month_Panel::increment ()
{
   if (status.zero_on ()) { return; }
   const Integer m = month_map[status.last ()];
   const Dstring& next_month = month_map.tokens[(m + 1) % 12];
   set_value (next_month);
}

void
Month_Panel::decrement ()
{
   if (status.zero_on ()) { return; }
   const Integer m = month_map[status.first ()];
   const Dstring& prev_month = month_map.tokens[(m - 1 + 12) % 12];
   set_value (prev_month);
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
      *this, &Hour_Panel::decrement));
   increment_button.get_signal ().connect (sigc::mem_fun (
      *this, &Hour_Panel::increment));

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

}

void
Hour_Panel::increment ()
{
   if (status.zero_on ()) { return; }
   Integer h = (stoi (status.last ()) + 1) % 24;
   set_value (Dstring::render ("%02dZ", h));

}

void
Hour_Panel::decrement ()
{
   if (status.zero_on ()) { return; }
   Integer h = (stoi (status.first ()) - 1 + 24) % 24;
   set_value (Dstring::render ("%02dZ", h));
}

Station_Panel::Station_Panel (Gwsb& gwsb,
                              const Tokens& station_tokens,
                              const Real margin,
                              const Real spacing)
   : Dgrid_Box (gwsb, margin, spacing),
     gwsb (gwsb),
     led_color (1, 0, 0)
{

   Integer id = 0;
   const Integer n = 9;

   for (const string& station : station_tokens)
   {
      const Integer i = id % n;
      const Integer j = id / n;
      Dbutton* button_ptr = new Dbutton (gwsb, station, 12);
      button_ptr_map.insert (make_pair (station, button_ptr));
      button_ptr->get_str_signal ().connect (sigc::mem_fun (
         gwsb, &Gwsb::set_station));
      pack (*button_ptr, Index_2D (i, j));
      id++;
   }

}

Station_Panel::~Station_Panel ()
{
   for (auto& i : button_ptr_map) { delete i.second; }
}

