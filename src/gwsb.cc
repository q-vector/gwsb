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
     noise_button (gwsb, "+Noise", 12, false),
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

   viewport = size_2d;
   viewport.shrink (50, 40, title.get_height () + 10, 10);

}

void
Gwsb::Histogram::render_image_buffer (const RefPtr<Context>& cr)
{

   Record::Set* record_set_ptr = gwsb.get_record_set_ptr ();

   Histogram_1D histogram_1d (1, 0.5);

   for (const Record& record : *record_set_ptr)
   {
      histogram_1d.increment (record.gradient_temperature);
   }

   Color::white ().cairo (cr);
   cr->paint ();

   const denise::Histogram::Axis& axis = histogram_1d.get_axis ();
   const Domain_1D domain_x (*axis.begin (), *axis.rbegin ());
   const Domain_1D domain_y (0, histogram_1d.get_max_value ());

   const Cartesian_Transform_2D transform (domain_y, domain_x, viewport);

   histogram_1d.render (cr, transform, "%.0f", "%.0f",
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
   const Real origin_x = viewport_x + viewport_width * 0.42;
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
   const Color& color_0 = Color::hsb (0.6, 0.0, 0.9);
   const Color& color_1 = Color::hsb (0.6, 0.0, 0.7);
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
Gwsb::render_histogram (const RefPtr<Context>& cr,
                        const set<Record>& record_set,
                        const Nwp_Gw& nwp_gw) const
{

   Histogram_1D histogram_1d (1, 0.5);
   const Wind_Disc::Transform& t = wind_disc.get_transform ();

   const Size_2D size_2d (80, 260);
   const Index_2D index_2d (width - 40 - size_2d.i, 120);
   const Box_2D box_2d (index_2d, size_2d);

   for (const Record& record : record_set)
   {
      const Wind& wind = record.wind;
      const Real d = wind.get_direction ();
      const Real s = wind.get_speed () / 0.51444444;
      const Point_2D& p = t.transform (Point_2D (d, s));
      histogram_1d.increment (record.gradient_temperature);
   }

   if (histogram_1d.size () == 0) { return; }

   const denise::Histogram::Axis& axis = histogram_1d.get_axis ();
   const Domain_1D domain_x (*axis.begin (), *axis.rbegin ());
   const Domain_1D domain_y (0, histogram_1d.get_max_value ());
   const Cartesian_Transform_2D transform (domain_y, domain_x, box_2d);

   histogram_1d.render (cr, transform, "%.0f", "%.0f",
      Color::gray (0.5), Color::black (), Color::black ());

   const Integer count = histogram_1d.get_number_of_points ();
   const Dstring fmt (count == 1 ? "%d point" : "%d points");
   const Dstring& str = Dstring::render (fmt, count);
   const Color& color_fg = Color::gray (0.2, 0.7);
   const Color& color_bg = Color::gray (0.8, 0.9);

   const Point_2D& anchor = Point_2D (width - 50 - size_2d.i, 120 + 260 + 10);
   cr->set_font_size (12);
   Label label (str, anchor, 'l', 't');
   label.cairo (cr, Color::gray (0.2, 0.7), Color::gray (0.8, 0.9), Point_2D (-3, 3));

   for (Integer i = 0; i < groups.size (); i++)
   {

      const Group& group = groups.get_group (i);
      Histogram_1D histogram_1d (1, 0.5);

      for (const Record& record : record_set)
      {
         const Wind& wind = record.wind;
         const Real d = wind.get_direction ();
         const Real s = wind.get_speed () / 0.51444444;
         const Point_2D& p = t.transform (Point_2D (d, s));
         if (!group.contains (p)) { continue; }
         histogram_1d.increment (record.gradient_temperature);
      }

      if (histogram_1d.size () > 0)
      {
         cr->save ();
         cr->set_line_width (3);
         Color (i, 0.8).cairo (cr);
         histogram_1d.render_outline (cr, transform);
         cr->restore ();
      }

      {
         const Integer dj = (i + 1) * 15;
         const Integer count = histogram_1d.get_number_of_points ();
         const Dstring& str = Dstring::render (fmt, count);
         Label label (str, anchor + Point_2D (0, dj), 'l', 't');
         label.cairo (cr, Color (i, 0.9), Color (i, 0.5), Point_2D (-3, 3));

      }

   }

   const Point_2D& t_p = transform.transform (Point_2D (0, nwp_gw.temperature));
   const Dstring& t_str = Dstring::render ("%.1f\u00b0 C \u2192", nwp_gw.temperature);
   Color::black ().cairo (cr);
   Label (t_str, t_p + Point_2D (-20, 0), 'r', 'c').cairo (cr);

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
     gradient_wind_threshold (5 * 0.514444)
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
   const Integer day_of_year = stoi (dtime.get_string ("%j"));
   const Integer hour = stoi (dtime.get_string ("%H"));

   Station_Data& station_data = data.get_station_data (station);

   return station_data.get_record_set_ptr (day_of_year, 15,
      hour, 0, nwp_gw, gradient_wind_threshold);

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

   if (event.type == GDK_3BUTTON_PRESS)
   {
      groups.clear ();
      render_queue_draw ();
      return true;
   }

   if (event.control () && !groups.is_defining ())
   {
      groups.defining = groups.size ();
      groups.push_back (new Group ());
      groups.get_defining_group ().add (point);
      render_queue_draw ();
      return true;
   }

   return false;

}

bool
Gwsb::on_mouse_motion (const Dmouse_Motion_Event& event)
{

   if (Dcontainer::on_mouse_motion (event)) { return true; }
   const Point_2D& point = event.point;

   if (groups.is_defining ())
   {
      groups.get_group (groups.defining).add (point);
      render_queue_draw ();
      return true;
   }

   return false;

}

bool
Gwsb::on_mouse_button_released (const Dmouse_Button_Event& event)
{

   if (Dcontainer::on_mouse_button_released (event)) { return true; }
   const Point_2D& point = event.point;

   if (groups.is_defining ())
   {
      Group& group = groups.get_defining_group ();
      if (group.size () > 2) { group.add (point); }
      else { groups.remove (); }
      groups.defining = -1; 
      render_queue_draw ();
      return true;
   }

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
         return true;
         break;
      }

      case GDK_SCROLL_DOWN:
      {
         gradient_wind_threshold /= 1.02;
         render_queue_draw ();
         return true;
         break;
      }

   }

   return Dcanvas::on_mouse_scroll (event);

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

   const Wind_Disc::Transform& t = wind_disc.get_transform ();

   title.set (date_str, station, time_str);

   wind_disc.clear ();
   Station_Data& station_data = data.get_station_data (station);
   const Record::Set* record_set_ptr = get_record_set_ptr ();
   record_set_ptr->feed (wind_disc);

   const Real hue = 0.33;
   const Real dir_noise = (with_noise ? 5 : 0);
   wind_disc.render_bg (cr);
   record_set_ptr->render_scatter_plot (cr, t, dir_noise, groups);
   if (outline) { wind_disc.render_percentage_d (cr, hue); }
   wind_disc.render_percentages (cr);

   render_histogram (cr, *record_set_ptr, nwp_gw);

   // render nwp_gw
   {

      const Real direction = nwp_gw.get_direction ();
      const Real speed = nwp_gw.get_speed () / 0.5144444;
      const Ring ring (t.get_length (gradient_wind_threshold / 0.5144444));
      const Point_2D p = t.transform (Point_2D (direction, speed));

      cr->save ();
      cr->set_line_width (4);
      cr->set_font_size (24);
      Color::black (0.4).cairo (cr);
      ring.cairo (cr, p);
      cr->stroke ();

      Label ("G", p, 'c', 'c').cairo (cr, Color::black (0.6),
         Color::black (0.3), Point_2D (2, 2));
      cr->restore ();

   }

   groups.cairo (cr);

   set_foreground_ready (false);

   delete record_set_ptr;


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

