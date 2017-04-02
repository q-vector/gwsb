#include <gtkmm/messagedialog.h>
#include <denise/histogram.h>
#include "data.h"
#include "selection.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

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

Option_Panel::Option_Panel (Gwsb& gwsb)
   : Drawer_Panel (gwsb, true, 12),
     gwsb (gwsb),
     noise_button (gwsb, "+Noise", 12, true),
     outline_button (gwsb, "Outline", 12, false),
     histogram_button (gwsb, "Histogram", 12),
     save_button (gwsb, "Save", 12)
{

   add_drawer ("Option");
   add_drawer ("Tool");

   noise_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_queue_draw));
   outline_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_queue_draw));
   save_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::save_image));
   histogram_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::spawn_histogram));

   add_widget_ptr ("Option", &noise_button);
   add_widget_ptr ("Option", &outline_button);

   add_widget_ptr ("Tool", &histogram_button);
   add_widget_ptr ("Tool", &save_button);

}

void
Option_Panel::toggle_noise ()
{
   noise_button.toggle ();
   gwsb.queue_draw ();
}

void
Option_Panel::toggle_outline ()
{
   outline_button.toggle ();
   gwsb.queue_draw ();
}

bool
Option_Panel::with_noise () const
{
   return noise_button.is_switched_on ();
}

bool
Option_Panel::with_outline () const
{
   return outline_button.is_switched_on ();
}

Gwsb::Histogram::Histogram (Gtk::Window& gtk_window,
                            Gwsb& gwsb)
   : Dcanvas (gtk_window),
     gwsb (gwsb)
{

   const Size_2D size_2d (300, 600);

   set_size_request (size_2d.i, size_2d.j);
   set_can_focus ();

   set_preferred_size (size_2d.i, size_2d.j);
   being_packed (Point_2D (0, 0), size_2d.i, size_2d.j);

}

void
Gwsb::Histogram::render_image_buffer (const RefPtr<Context>& cr)
{

   Record::Set* record_set_ptr = gwsb.get_record_set_ptr ();
   cout << record_set_ptr->size () << endl; 

   Histogram_1D histogram_1d (1);

   for (const Record& record : *record_set_ptr)
   {
      histogram_1d.increment (record.gradient_temperature);
   }

   Color::white ().cairo (cr);
   cr->paint ();

   const denise::Histogram::Axis& axis = histogram_1d.get_axis ();
   const Domain_1D domain_x (*axis.begin () - 10, *axis.rbegin () + 10);
   const Domain_1D domain_y (-10, histogram_1d.get_max_value () + 10);
   const Cartesian_Transform_2D transform (domain_x, domain_y, 250, 500);

   histogram_1d.render (cr, transform, domain_y, "%.1f", "%.1f",
      Color::red (), Color::black (), Color::black());

   delete record_set_ptr;

}

void
Gwsb::pack ()
{

   const Real margin = 6;
   const Real title_height = title.get_height ();

   const Real sp_anchor_x = margin;
   const Real sp_anchor_y = title_height + margin; 
   const Real sp_width = width - 2 * margin;
   const Real sp_height = 60;
   const Point_2D sp_anchor (sp_anchor_x, sp_anchor_y);

   const Real tc_anchor_x = margin; 
   const Real tc_anchor_y = sp_anchor_y + sp_height + margin; 
   const Real tc_width = 60 + margin + 60;
   const Real tc_height = height - tc_anchor_y - margin;
   const Point_2D tc_anchor (tc_anchor_x, tc_anchor_y);

   const Real bp_width = 200;
   const Real bp_height = 20;
   const Real bp_anchor_x = width - bp_width - margin;
   const Real bp_anchor_y = height - bp_height - margin;
   const Point_2D bp_anchor (bp_anchor_x, bp_anchor_y);

   const Real viewport_width = width - tc_width - margin * 3;
   const Real viewport_height = height - title_height - sp_height - margin * 3;
   const Real viewport_x = width - viewport_width - margin;
   const Real viewport_y = height - viewport_height - margin;
   const Real origin_x = viewport_x + viewport_width / 2;
   const Real origin_y = viewport_y + viewport_height / 2;
   const Point_2D origin (origin_x, origin_y);

   station_panel.being_packed (sp_anchor, sp_width, sp_height);
   station_panel.pack ();

   option_panel.being_packed (bp_anchor, bp_width, bp_height);
   option_panel.pack ();

   time_chooser.being_packed (tc_anchor, tc_width, tc_height);
   time_chooser.pack ();

   viewport.index_2d.i = Integer (round (viewport_x));
   viewport.index_2d.j = Integer (round (viewport_y));
   viewport.size_2d.i = Integer (round (viewport_width));
   viewport.size_2d.j = Integer (round (viewport_height));

   const Real max_radius = std::min (viewport_width, viewport_height) * 0.475;
   wind_disc.set_position (origin, max_radius);

   this->packed = true;

}

void
Gwsb::render_bg (const RefPtr<Context>& cr)
{

//   const Color& color_0 = Color::hsb (0.6, 0.4, 0.8);
//   const Color& color_1 = Color::hsb (0.6, 0.1, 0.4);
   const Color& color_0 = Color::hsb (0.6, 0.4, 0.9);
   const Color& color_1 = Color::hsb (0.6, 0.1, 0.7);
   const Point_2D point_0 (0, 0);
   const Point_2D point_1 (0, height);
   Color_Gradient (color_0, color_1, point_0, point_1).cairo (cr);
   cr->paint ();

   Color::white (0.4).cairo (cr);
   const Point_2D point (viewport.index_2d.i, viewport.index_2d.j);
   Rect (point, viewport.size_2d.i, viewport.size_2d.j).cairo (cr);
   cr->fill ();

}

void
Gwsb::render_count (const RefPtr<Context>& cr,
                    const Integer count,
                    const Box_2D& viewport)
{

   const Dstring fmt (count == 1 ? "%d point" : "%d points");
   const Dstring& str = Dstring::render (fmt, count);
   const Color& color_fg = Color::gray (0.2, 0.7);
   const Color& color_bg = Color::gray (0.8, 0.9);

   const Real padding = 8;
   const Point_2D ne (viewport.get_ne ());
   const Point_2D& anchor = ne + Point_2D (-padding, padding);
   cr->set_font_size (12);
   Label label (str, anchor, 'r', 't');
   label.cairo (cr, color_fg, color_bg, Point_2D (-3, 3));

}

Gwsb::Gwsb (Gtk::Window* window_ptr,
            const Size_2D& size_2d,
            const Nwp_Gw::Sequence::Map& sequence_map,
            const Data& data,
            Wind_Disc& wind_disc)
   : Dcanvas (*window_ptr),
     wind_disc (wind_disc),
     window_ptr (window_ptr),
     station (sequence_map.get_station_tokens ().front ()),
     station_panel (*this, sequence_map.get_station_tokens (), 0, 6),
     option_panel (*this),
     data (data),
     sequence_map (sequence_map),
     time_chooser (*this, 12),
     viewport (Size_2D (0, 0)),
     gradient_wind_threshold (7)
{

   Gdk::EventMask event_mask = (Gdk::SCROLL_MASK);
   event_mask |= (Gdk::POINTER_MOTION_MASK);
   event_mask |= (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
   event_mask |= (Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
   set_events (event_mask);

   // This basically sets the minimum size
   set_size_request (size_2d.i, size_2d.j);
   set_can_focus ();

   time_chooser.get_signal ().connect (sigc::mem_fun (
      *this, &Gwsb::render_queue_draw));
   set_station (sequence_map.get_station_tokens ().front ());

   register_widget (station_panel);
   register_widget (option_panel);
   register_widget (time_chooser);

   set_preferred_size (size_2d.i, size_2d.j);
   being_packed (Point_2D (0, 0), size_2d.i, size_2d.j);

   pack ();

}

Gwsb::~Gwsb ()
{
}

const Data&
Gwsb::get_data () const
{
   return data;
}

void
Gwsb::save_image ()
{

   using namespace Gtk;

   FileChooserDialog* dialog_ptr = new FileChooserDialog (
      "Save Image...", FILE_CHOOSER_ACTION_SAVE);

   FileChooserDialog& dialog = *dialog_ptr;
   dialog.set_transient_for (*window_ptr);

   dialog.add_button (Stock::CANCEL, RESPONSE_CANCEL);
   dialog.add_button (Stock::SAVE, RESPONSE_OK);

   Glib::RefPtr<Gtk::FileFilter> filter_png = Gtk::FileFilter::create ();
   filter_png->set_name ("PNG Image Format (*.png)");
   filter_png->add_mime_type ("image/png");
   dialog.add_filter (filter_png);

   Glib::RefPtr<Gtk::FileFilter> filter_pdf = Gtk::FileFilter::create ();
   filter_pdf->set_name ("PDF Format (*.pdf)");
   filter_pdf->add_mime_type ("application/pdf");
   dialog.add_filter (filter_pdf);

   Glib::RefPtr<Gtk::FileFilter> filter_svg = Gtk::FileFilter::create ();
   filter_svg->set_name ("SVG Image Format (*.svg)");
   filter_svg->add_mime_type ("image/svg");
   dialog.add_filter (filter_svg);

   int result = dialog.run ();

   switch (result)
   {
      case RESPONSE_OK:
      {
         if (!save (dialog.get_filename ()))
         {
            const Dstring str_a ("PNG, SVG, or PDF Formats Only");
            const Dstring str_b ("Ensure your file name ends with .png, .svg, or .pdf. Abort.");
            Gtk::MessageDialog* d_ptr = new Gtk::MessageDialog (*window_ptr, str_a);
            d_ptr->set_secondary_text (str_b);
            d_ptr->run ();
            delete d_ptr;
            delete dialog_ptr;
            save_image ();
         }

         break;
      }
   }

   delete dialog_ptr;

}

bool
Gwsb::save (const Dstring& file_path)
{

   const Size_2D& size_2d = get_size_2d ();
   const Tokens tokens (file_path, ".");

   const Dstring& file_extension = tokens.back ();

   if (file_extension == "png")
   {
      RefPtr<ImageSurface> surface = Cairo::ImageSurface::create (
         FORMAT_RGB24, size_2d.i, size_2d.j);
      const RefPtr<Context> cr = Context::create (surface);
      cairo (cr);
      image_surface->write_to_png (file_path);
      return true;
   }
   else
   if (file_extension == "pdf")
   {
      RefPtr<PdfSurface> surface = Cairo::PdfSurface::create (
         file_path, size_2d.i, size_2d.j);
      const RefPtr<Context> cr = Context::create (surface);
      cairo (cr);
      return true;
   }
   else
   if (file_extension == "svg")
   {
      RefPtr<SvgSurface> surface = Cairo::SvgSurface::create (
         file_path, size_2d.i, size_2d.j);
      const RefPtr<Context> cr = Context::create (surface);
      cairo (cr);
      return true;
   }

   return false;

}

const Tokens&
Gwsb::get_station_tokens () const
{
   return sequence_map.get_station_tokens ();
}

void
Gwsb::set_station (const Dstring& station)
{
   this->station = station;
   const Nwp_Gw::Sequence& sequence = sequence_map.at (station);
   const set<Dtime>& time_set = sequence.get_time_set ();
   const Time_Chooser::Shape time_chooser_shape (time_set);
   time_chooser.set_shape (time_chooser_shape);
   render_queue_draw ();
}

Record::Set*
Gwsb::get_record_set_ptr ()
{

   const Dtime& dtime = time_chooser.get_time ();
   const Nwp_Gw::Sequence& sequence = sequence_map.at (station);
   const Nwp_Gw& nwp_gw = sequence.at (dtime);
   const Dstring& month_str = dtime.get_string ("%b");
   const Dstring& hour_str = dtime.get_string ("%HZ");

   Station_Data& station_data = data.get_station_data (station);

   return station_data.get_record_set_ptr (month_str,
      hour_str, nwp_gw, gradient_wind_threshold);

}

bool
Gwsb::on_key_pressed (const Dkey_Event& event)
{

   if (Dcontainer::on_key_pressed (event)) { return true; }

   switch (event.value)
   {

      case GDK_KEY_Left:
      {
         time_chooser.step_backward ();
         return true;
      }

      case GDK_KEY_Right:
      {
         time_chooser.step_forward ();
         return true;
      }

      case GDK_KEY_N:
      case GDK_KEY_n:
      {
         option_panel.toggle_noise ();
         return true;
      }

      case GDK_KEY_O:
      case GDK_KEY_o:
      {
         option_panel.toggle_outline ();
         return true;
      }

      case GDK_KEY_Q:
      case GDK_KEY_q:
      {
         exit (0);
         //delete window_ptr;
         //delete this;
         break;
      }

   }

   return false;

}

bool
Gwsb::on_mouse_button_pressed (const Dmouse_Button_Event& event)
{

   if (Dcontainer::on_mouse_button_pressed (event)) { return true; }
   const Point_2D& point = event.point;

   return false;

}

bool
Gwsb::on_mouse_motion (const Dmouse_Motion_Event& event)
{

   if (Dcontainer::on_mouse_motion (event)) { return true; }
   const Point_2D& point = event.point;

   return false;

}

bool
Gwsb::on_mouse_button_released (const Dmouse_Button_Event& event)
{

   if (Dcontainer::on_mouse_button_released (event)) { return true; }
   const Point_2D& point = event.point;

   return false;

}

bool
Gwsb::on_mouse_scroll (const Dmouse_Scroll_Event& event)
{

   const Point_2D point (event.point.x - anchor.x, event.point.y - anchor.y);
   if (out_of_bounds (point)) { return false; }

   switch (event.direction)
   {

      case GDK_SCROLL_UP:
      {
         gradient_wind_threshold *= 1.02;
         render_queue_draw ();
         break;
      }

      case GDK_SCROLL_DOWN:
      {
         gradient_wind_threshold /= 1.02;
         render_queue_draw ();
         break;
      }

   }

   return Gwsb::on_mouse_scroll (event);

}

void
Gwsb::render_background_buffer (const RefPtr<Context>& cr)
{
   render_bg (cr);
}

void
Gwsb::render_image_buffer (const RefPtr<Context>& cr)
{

   const bool outline = (option_panel.with_outline ());
   const Real with_noise = option_panel.with_noise ();

   const Nwp_Gw::Sequence& sequence = sequence_map.at (station);
   const Dtime& dtime = time_chooser.get_time ();
   const Nwp_Gw& nwp_gw = sequence.at (dtime);

   const Dstring& month_str = dtime.get_string ("%b");
   const Dstring& hour_str = dtime.get_string ("%HZ");
   const Dstring& date_str = dtime.get_string ("%Y.%m.%d (%a)");
   const Dstring& time_str = dtime.get_string ("%H:%M UTC");

   title.set (date_str, station, time_str);

   wind_disc.clear ();
   Station_Data& station_data = data.get_station_data (station);
   const Record::Set* record_set_ptr = get_record_set_ptr ();
   record_set_ptr->feed (wind_disc);

   const Real hue = 0.33;
   const Real dir_noise = (with_noise ? 5 : 0);
   wind_disc.render_bg (cr);
   record_set_ptr->render_scatter_plot (cr, wind_disc, dir_noise);
   if (outline) { wind_disc.render_percentage_d (cr, hue); }
   render_count (cr, record_set_ptr->size (), viewport);
   wind_disc.render_percentages (cr);

   delete record_set_ptr;

   // render nwp_gw
   {

      const Wind_Disc::Transform& t = wind_disc.get_transform ();

      const Real direction = nwp_gw.get_direction ();
      const Real speed = nwp_gw.get_speed ();
      const Ring ring (t.get_length (gradient_wind_threshold));

      const Point_2D p = t.transform (Point_2D (direction, speed));

      cr->save ();
      cr->set_line_width (4);
      ring.cairo (cr, p);
      Color::black (0.0).cairo (cr);
      cr->fill_preserve ();
      Color::black (0.4).cairo (cr);
      cr->stroke ();

      cr->set_font_size (24);
      Label ("G", p, 'c', 'c').cairo (cr, Color::black (0.6),
         Color::black (0.3), Point_2D (2, 2));
      cr->restore ();

   }

   set_foreground_ready (false);

}

void
Gwsb::spawn_histogram ()
{

   Gtk::Window* window_ptr = new Gtk::Window ();
   Histogram* histogram_ptr = new Histogram (*window_ptr, *this);

   window_ptr->set_title ("Histogram");
   window_ptr->add (*histogram_ptr);
   window_ptr->set_resizable (false);

   window_ptr->show_all_children ();
   window_ptr->show ();

}

/*
Gwsb_Free::Gwsb_Free (Gtk::Window* window_ptr,
                      const Size_2D& size_2d,
                      const Data& data,
                      Wind_Disc& wind_disc)
   : Gwsb (window_ptr,
           size_2d,
           data.get_station_tokens (),
           data,
           wind_disc),
     month_panel (*this, 0, 6),
     hour_panel (*this, 0, 6),
     selecting_gradient_wind (false)
{

   const Dtime now;
   month_panel.set_value (now.get_string ("%b"), false);
   hour_panel.set_value (now.get_string ("%02dZ"), false);

   register_widget (month_panel);
   register_widget (hour_panel);

   pack ();

}

void
Gwsb_Free::pack ()
{

   Gwsb::pack ();

   const Real margin = 6;
   const Real title_height = title.get_height ();

   const Real sp_anchor_x = margin;
   const Real sp_anchor_y = title_height + margin; 
   const Real sp_width = width - 2 * margin;
   const Real sp_height = 60;
   const Point_2D sp_anchor (sp_anchor_x, sp_anchor_y);

   const Real mp_anchor_x = margin; 
   const Real mp_anchor_y = sp_anchor_y + sp_height + margin; 
   const Real mp_width = 60;
   const Real mp_height = height - mp_anchor_y - margin;
   const Point_2D mp_anchor (mp_anchor_x, mp_anchor_y);

   const Real hp_anchor_x = mp_anchor_x + mp_width + margin; 
   const Real hp_anchor_y = mp_anchor_y; 
   const Real hp_width = 60;
   const Real hp_height = height - hp_anchor_y - margin;
   const Point_2D hp_anchor (hp_anchor_x, hp_anchor_y);

   const Real bp_width = 200;
   const Real bp_height = 20;
   const Real bp_anchor_x = width - bp_width - margin;
   const Real bp_anchor_y = height - bp_height - margin;
   const Point_2D bp_anchor (bp_anchor_x, bp_anchor_y);

   month_panel.being_packed (mp_anchor, mp_width, mp_height);
   month_panel.pack ();

   hour_panel.being_packed (hp_anchor, hp_width, hp_height);
   hour_panel.pack ();

   this->packed = true;

}

const Tokens&
Gwsb_Free::get_station_tokens () const
{
   return data.get_station_tokens ();
}

void
Gwsb_Free::set_station (const Dstring& station)
{
   this->station = station;
   render_queue_draw ();
}

const set<Index_2D>&
Gwsb_Free::get_gradient_wind_index_set () const
{
   return gradient_wind_index_set;
}

const Selection_Panel::Status&
Gwsb_Free::get_month_status () const
{
   return month_panel.get_status ();
}

const Selection_Panel::Status&
Gwsb_Free::get_hour_status () const
{
   return hour_panel.get_status ();
}

bool
Gwsb_Free::add_gradient_wind_index (const Index_2D& index_2d,
                                    const bool clear_first,
                                    const bool delete_if_present)
{

   if (clear_first) { clear_gradient_wind_index (); }

   set<Index_2D>::iterator iterator = gradient_wind_index_set.find (index_2d);
   if (iterator != gradient_wind_index_set.end ())
   {
      if (delete_if_present)
      {
         gradient_wind_index_set.erase (iterator);
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      gradient_wind_index_set.insert (index_2d);
      return true;
   }

}

void
Gwsb_Free::delete_gradient_wind_index (const Index_2D& index_2d)
{
   set<Index_2D>::iterator iterator = gradient_wind_index_set.find (index_2d);
   if (iterator != gradient_wind_index_set.end ())
   {
      gradient_wind_index_set.erase (iterator);
   }
}

void
Gwsb_Free::clear_gradient_wind_index ()
{
   gradient_wind_index_set.clear ();
}

bool
Gwsb_Free::match_gradient_wind (const Wind& gradient_wind) const
{

   if (gradient_wind_index_set.size () == 0) { return true; }

   const Index_2D& gw_index = wind_disc.get_index (gradient_wind);

   for (const Index_2D& i2d : gradient_wind_index_set)
   {
      if (i2d == gw_index) { return true; }
   }

   return false;

}

bool
Gwsb_Free::on_key_pressed (const Dkey_Event& event)
{

   switch (event.value)
   {

      case GDK_KEY_Up:
      {
         month_panel.decrement ();
         return true;
      }

      case GDK_KEY_Down:
      {
         month_panel.increment ();
         return true;
      }

      case GDK_KEY_Left:
      {
         hour_panel.decrement ();
         return true;
      }

      case GDK_KEY_Right:
      {
         hour_panel.increment ();
         return true;
      }

   }

   return Gwsb::on_key_pressed (event);

}

bool
Gwsb_Free::on_mouse_button_pressed (const Dmouse_Button_Event& event)
{

   const Point_2D& point = event.point;

   switch (event.button)
   {

      case 1:
      {

         try
         {

            const Wind& gradient_wind = wind_disc.get_wind (point);
            const Index_2D& index_2d = wind_disc.get_index (gradient_wind);

            const bool control_pressed = (event.state & GDK_CONTROL_MASK);
            const bool clear_first = !control_pressed;
            add_gradient_wind_index (index_2d, clear_first, true);

            selecting_gradient_wind = true;

         }
         catch (const Exception& e)
         {
            if (viewport.contains (point.x, point.y))
            {
               clear_gradient_wind_index ();
            }
         }

         render_queue_draw ();
         return true;
         break;

      }

   }

   return Gwsb::on_mouse_button_pressed (event);

}

bool
Gwsb_Free::on_mouse_motion (const Dmouse_Motion_Event& event)
{

   const Point_2D& point = event.point;

   if (selecting_gradient_wind)
   {

      try
      {
         const Wind& gradient_wind = wind_disc.get_wind (point);
         const Index_2D& index_2d = wind_disc.get_index (gradient_wind);
         if (add_gradient_wind_index (index_2d, false, false))
         {
            render_queue_draw ();
            return true;
         }
      }
      catch (const Exception& e)
      {
      }

   }

   return Gwsb::on_mouse_motion (event);

}

bool
Gwsb_Free::on_mouse_button_released (const Dmouse_Button_Event& event)
{

   if (Dcontainer::on_mouse_button_released (event)) { return true; }
   const Point_2D& point = event.point;

   bool processed = false;

//   switch (event.button)
//   {
//
//      case 1:
//      {
//
//         try
//         {
//            const bool control_pressed = (event.state & GDK_CONTROL_MASK);
//            if (control_pressed)
//            {
//               const Wind& gradient_wind = wind_disc.get_wind (point);
//               const Index_2D& index_2d = wind_disc.get_index (gradient_wind);
//               if (!dragging_gradient_wind)
//               {
//                  delete_gradient_wind_index (index_2d);
//                  render ();
//                  refresh ();
//                  processed = true;
//               }
//            }
//         }
//         catch (const Exception& e)
//         {
//         }
//         break;
//
//      }
//   }
      
   selecting_gradient_wind = false;
   return processed;

}

void
Gwsb_Free::render (const RefPtr<Context>& cr,
                   const Wind_Disc& wind_disc,
                   const Record::Set& record_set,
                   const set<Index_2D>& gradient_wind_index_set,
                   const Box_2D& viewport,
                   const bool outline,
                   const bool with_noise)
{

   wind_disc.render_bg (cr);

   const Real dir_noise = (with_noise ? 5 : 0);
   record_set.render_scatter_plot (cr, wind_disc, dir_noise);

   const Real hue = 0.33;
   if (outline) { wind_disc.render_percentage_d (cr, hue); }
   render_count (cr, record_set.size (), viewport);

   wind_disc.render_percentages (cr);
   wind_disc.render_index_set (cr, gradient_wind_index_set);

}

void
Gwsb_Free::cairo (const RefPtr<Context>& cr)
{

   const Dstring& month_string = month_panel.get_string ();
   const Dstring& hour_string = hour_panel.get_string ();
   const bool outline = (option_panel.with_outline ());
   const Real with_noise = option_panel.with_noise ();

   title.set (month_string, station, hour_string);

   Station_Data& station_data = data.get_station_data (station);
   const Record::Set* record_set_ptr = station_data.get_record_set_ptr (*this);

   wind_disc.clear ();
   record_set_ptr->feed (wind_disc);
   render (cr, wind_disc, *record_set_ptr,
      gradient_wind_index_set, viewport, outline, with_noise);
   Dcanvas::cairo (cr);

   delete record_set_ptr;

}
*/

