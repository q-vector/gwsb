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
     day_of_year_threshold_button (gwsb, true, 12),
     hour_threshold_button (gwsb, true, 12),
     noise_button (gwsb, "Noise", 12, false),
     outline_button (gwsb, "Outline", 12, false),
     cluster_button (gwsb, "Clusters", 12, true),
     percentages_button (gwsb, "Percentages", 12, false),
     clear_clusters_button (gwsb, "Clusters", 12),
     save_button (gwsb, "Save", 12)
{

   day_of_year_threshold_button.add_token ("5 days");
   day_of_year_threshold_button.add_token ("10 days");
   day_of_year_threshold_button.add_token ("15 days");
   day_of_year_threshold_button.add_token ("30 days");
   day_of_year_threshold_button.add_token ("45 days");
   day_of_year_threshold_button.add_token ("60 days");
   day_of_year_threshold_button.add_token ("90 days");

   hour_threshold_button.add_token ("0 hr");
   hour_threshold_button.add_token ("1 hr");
   hour_threshold_button.add_token ("2 hr");
   hour_threshold_button.add_token ("3 hr");

   day_of_year_threshold_button.get_update_signal ().connect (
      sigc::mem_fun (gwsb, &Gwsb::render_queue_draw));
   hour_threshold_button.get_update_signal ().connect (
      sigc::mem_fun (gwsb, &Gwsb::render_queue_draw));

   clear_clusters_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::clear_clusters));

   noise_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_queue_draw));
   outline_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_queue_draw));
   cluster_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_queue_draw));
   percentages_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_queue_draw));

   save_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::save_image));

   add_widget_ptr ("Threshold", &day_of_year_threshold_button);
   add_widget_ptr ("Threshold", &hour_threshold_button);

   add_widget_ptr ("Clear", &clear_clusters_button);

   add_widget_ptr ("Show", &noise_button);
   add_widget_ptr ("Show", &outline_button);
   add_widget_ptr ("Show", &cluster_button);
   add_widget_ptr ("Show", &percentages_button);

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

void
Option_Panel::toggle_cluster ()
{
   cluster_button.toggle ();
   gwsb.queue_draw ();
}

void
Option_Panel::toggle_percentages ()
{
   percentages_button.toggle ();
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

bool
Option_Panel::with_cluster () const
{
   return cluster_button.is_switched_on ();
}

bool
Option_Panel::with_percentages () const
{
   return percentages_button.is_switched_on ();
}

Integer
Option_Panel::get_day_of_year_threshold () const
{
   return stoi (Tokens (day_of_year_threshold_button.get_str ())[0]);
}

Integer
Option_Panel::get_hour_threshold () const
{
   return stoi (Tokens (hour_threshold_button.get_str ())[0]);
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

   const Real bp_width = 360;
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
Gwsb::render_histogram (const RefPtr<Context>& cr,
                        const set<Record>& record_set,
                        const Predictor& predictor) const
{

   Histogram_1D histogram_1d (1, 0.5);

   const Size_2D size_2d (80, 260);
   const Index_2D index_2d (width - 40 - size_2d.i, 120);
   const Box_2D box_2d (index_2d, size_2d);

   for (const Record& record : record_set)
   {
      const Wind& wind = record.wind;
      const Real d = wind.get_direction ();
      const Real s = wind.get_speed () / 0.51444444;
      const Transform_2D& transform = wind_disc.get_transform ();
      const Point_2D& p = transform.transform (Point_2D (d, s));
      histogram_1d.increment (record.temperature_925);
   }

   if (histogram_1d.size () == 0) { return; }

   const denise::Histogram::Axis& axis = histogram_1d.get_axis ();
   const Domain_1D domain_x (*axis.begin () - 0.8, *axis.rbegin () + 0.8);
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
   label.cairo (cr, Color::gray (0.2, 0.7),
      Color::gray (0.8, 0.9), Point_2D (-3, 3));

   for (Integer i = 0; i < clusters.size (); i++)
   {

      const Cluster& cluster = clusters.get_cluster (i);
      const Histogram_1D& histogram = cluster.histogram;

      if (histogram_1d.size () > 0)
      {
         cr->save ();
         cr->set_line_width (3);
         Color (i, 0.8).cairo (cr);
         histogram.render_outline (cr, transform);
         cr->restore ();
      }

      {
         const Integer dj = (i + 1) * 15;
         const Integer count = histogram.get_number_of_points ();
         const Dstring& str = Dstring::render (fmt, count);
         Label label (str, anchor + Point_2D (0, dj), 'l', 't');
         label.cairo (cr, Color (i, 0.9), Color (i, 0.5), Point_2D (-3, 3));
      }

   }

   if (!gsl_isnan (predictor.temperature_925))
   {
      const Real t_925 = predictor.temperature_925;
      const Point_2D& t_p = transform.transform (Point_2D (0, t_925));
      const Dstring& t_str = Dstring::render ("%.1f\u00b0 C \u2192", t_925);
      Color::black ().cairo (cr);
      Label (t_str, t_p + Point_2D (-20, 0), 'r', 'c').cairo (cr);
   }

}

void
Gwsb::render (const RefPtr<Context>& cr,
              const Dtime& dtime,
              const Predictor& predictor)
{

   const bool with_outline = option_panel.with_outline ();
   const Real with_noise = option_panel.with_noise ();
   const Real with_cluster = option_panel.with_cluster ();
   const Real with_percentages = option_panel.with_percentages ();

   const Dstring& month_str = dtime.get_string ("%b");
   const Dstring& hour_str = dtime.get_string ("%HZ");
   const Dstring& date_str = dtime.get_string ("%Y.%m.%d (%a)");
   const Dstring& time_str = dtime.get_string ("%H:%M UTC");

   const Wind_Disc::Transform& t = wind_disc.get_transform ();

   title.set (date_str, station, time_str);

   wind_disc.clear ();
   Station_Data& station_data = data.get_station_data (station);
   const Record::Set* record_set_ptr = get_record_set_ptr (dtime, predictor);
   record_set_ptr->feed (wind_disc);

   const Real hue = 0.33;
   const Real dir_noise = (with_noise ? 5 : 0);
   wind_disc.render_bg (cr);

   for (Cluster* cluster_ptr : clusters) { cluster_ptr->histogram.clear (); }
   record_set_ptr->render_scatter_plot (cr, t, dir_noise, clusters);

   if (with_outline) { wind_disc.render_percentage_d (cr, hue); }
   if (with_percentages) { wind_disc.render_percentages (cr); }

   render_histogram (cr, *record_set_ptr, predictor);

   // render predictor
   {

      const Real direction = predictor.wind_925.get_direction ();
      const Real speed = predictor.wind_925.get_speed () / 0.5144444;
      const Ring ring (t.get_length (wind_925_threshold / 0.5144444));
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

   if (with_cluster) { clusters.render (cr, 0.4); }
   clusters.render_defining (cr);

   {
      const Point_2D anchor (viewport.get_nw () + Index_2D (10, 10));
      const Integer day_of_year_threshold =
         option_panel.get_day_of_year_threshold ();
      const Integer hour_threshold = option_panel.get_hour_threshold ();
      const Dstring& doy_str = Dstring::render ("+/- %d days",
         day_of_year_threshold);
      const Dstring& hour_str = Dstring::render ("+/- %d h", hour_threshold);
      cr->save ();
      cr->set_font_size (12);
      Label (doy_str, anchor, 'l', 't').cairo (cr, Color::gray (0.2, 0.7),
         Color::gray (0.8, 0.9), Point_2D (-3, 3));
      Label (hour_str, anchor + Point_2D (0, 15), 'l', 't').cairo (
         cr, Color::gray (0.2, 0.7), Color::gray (0.8, 0.9), Point_2D (-3, 3));
      cr->restore ();
   }

   set_foreground_ready (false);

   delete record_set_ptr;


}

Gwsb::Gwsb (Gtk::Window* window_ptr,
            const Size_2D& size_2d,
            const Tokens& station_tokens,
            const Data& data,
            Wind_Disc& wind_disc)
   : Dcanvas (*window_ptr),
     wind_disc (wind_disc),
     window_ptr (window_ptr),
     station_panel (*this, station_tokens, 0, 6),
     option_panel (*this),
     time_chooser (*this, 12),
     station (station_tokens.front ()),
     data (data),
     wind_925_threshold (5 * 0.514444)
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
      *this, &Gwsb_Sequence::render_queue_draw));

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

//void
//Gwsb::set_station (const Dstring& station)
//{
//}

Record::Set*
Gwsb::get_record_set_ptr (const Dtime& dtime,
                          const Predictor& predictor)
{

   const Integer day_of_year = stoi (dtime.get_string ("%j"));
   const Integer hour = stoi (dtime.get_string ("%H"));

   const Option_Panel& op = option_panel;
   const Integer day_of_year_threshold = op.get_day_of_year_threshold ();
   const Integer hour_threshold = op.get_hour_threshold ();

   Station_Data& station_data = data.get_station_data (station);

   return station_data.get_record_set_ptr (day_of_year,
      day_of_year_threshold, hour, hour_threshold,
      predictor.wind_925, wind_925_threshold);

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

      case GDK_KEY_P:
      case GDK_KEY_p:
      {
         option_panel.toggle_percentages ();
         return true;
      }

      case GDK_KEY_C:
      case GDK_KEY_c:
      {
         option_panel.toggle_cluster ();
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
      clear_clusters ();
      return true;
   }

   if (event.control () && !clusters.is_defining ())
   {
      clusters.defining = clusters.size ();
      clusters.push_back (new Cluster ());
      clusters.get_defining_cluster ().add (point);
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

   if (clusters.is_defining ())
   {
      clusters.get_cluster (clusters.defining).add (point);
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

   if (clusters.is_defining ())
   {
      Cluster& cluster = clusters.get_defining_cluster ();
      if (cluster.size () > 2) { cluster.add (point); }
      else { clusters.remove (); }
      clusters.defining = -1; 
      render_queue_draw ();
      return true;
   }

   return false;

}

bool
Gwsb::on_mouse_scroll (const Dmouse_Scroll_Event& event)
{

   if (Dcontainer::on_mouse_scroll (event)) { return true; }
   const Point_2D& point = event.point;

   if (out_of_bounds (point)) { return false; }

   if (event.control ())
   {

      switch (event.direction)
      {

         case GDK_SCROLL_UP:
         {
            wind_925_threshold *= 1.02;
            render_queue_draw ();
            return true;
            break;
         }

         case GDK_SCROLL_DOWN:
         {
            wind_925_threshold /= 1.02;
            render_queue_draw ();
            return true;
            break;
         }

      }

   }

   return false;

}

void
Gwsb::render_background_buffer (const RefPtr<Context>& cr)
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
Gwsb::clear_clusters ()
{
   clusters.clear ();
   render_queue_draw ();
}

Gwsb_Sequence::Gwsb_Sequence (Gtk::Window* window_ptr,
                              const Size_2D& size_2d,
                              const Predictor::Sequence::Map& sequence_map,
                              const Data& data,
                              Wind_Disc& wind_disc)
   : Gwsb (window_ptr,
           size_2d,
           sequence_map.get_station_tokens (),
           data,
           wind_disc),
     sequence_map (sequence_map)
{
   set_station (sequence_map.get_station_tokens ().front ());
}

Gwsb_Sequence::~Gwsb_Sequence ()
{
}

void
Gwsb_Sequence::set_station (const Dstring& station)
{
   this->station = station;
   const Predictor::Sequence& sequence = sequence_map.at (station);
   const set<Dtime>& time_set = sequence.get_time_set ();
   const Time_Chooser::Shape time_chooser_shape (time_set);
   time_chooser.set_shape (time_chooser_shape);
   render_queue_draw ();
}

Record::Set*
Gwsb_Sequence::get_record_set_ptr ()
{

   const Dtime& dtime = time_chooser.get_time ();
   const Predictor::Sequence& sequence = sequence_map.at (station);
   const Predictor& predictor = sequence.at (dtime);

   return Gwsb::get_record_set_ptr (dtime, predictor);

}

void
Gwsb_Sequence::render_image_buffer (const RefPtr<Context>& cr)
{

   const Predictor::Sequence& sequence = sequence_map.at (station);
   const Dtime& dtime = time_chooser.get_time ();
   const Predictor& predictor = sequence.at (dtime);

   render (cr, dtime, predictor);

}

Gwsb_Free::Gwsb_Free (Gtk::Window* window_ptr,
                    const Size_2D& size_2d,
                    const Tokens& station_tokens,
                    const Data& data,
                    Wind_Disc& wind_disc)
   : Gwsb (window_ptr, size_2d, station_tokens, data, wind_disc),
     predictor (Wind (GSL_NAN, GSL_NAN), GSL_NAN),
     defining_predictor (false)
{

   set<Dtime> time_set;
   for (Integer j = 0; j < 365; j++)
   {
      const Dtime epoch (1970, 1, 1);
      time_set.insert (epoch.t + j * 24);
   }
   const Time_Chooser::Shape time_chooser_shape (time_set);
   time_chooser.set_shape (time_chooser_shape);

   set_station (station_tokens.front ());

}

Gwsb_Free::~Gwsb_Free ()
{
}

void
Gwsb_Free::set_station (const Dstring& station)
{
   this->station = station;
   render_queue_draw ();
}

Record::Set*
Gwsb_Free::get_record_set_ptr ()
{

   const Dtime& dtime = time_chooser.get_time ();

   return Gwsb::get_record_set_ptr (dtime, predictor);

}

bool
Gwsb_Free::on_mouse_button_pressed (const Dmouse_Button_Event& event)
{

   if (Gwsb::on_mouse_button_pressed (event)) { return true; }
   const Point_2D& point = event.point;

   const Wind_Disc::Transform& transform = wind_disc.get_transform ();
   const Wind& w = transform.get_wind (point);
   const bool no_wind_925 = predictor.wind_925.is_naw ();
   const Real difference = (w - predictor.wind_925).get_speed ();
   const bool near_wind_925 = (difference <= wind_925_threshold);
   const bool double_click = (event.type == GDK_2BUTTON_PRESS);

   if (near_wind_925 || (no_wind_925 && double_click))
   {
      defining_predictor = true;
      const Point_2D& w = transform.reverse (point);
      predictor.wind_925 = Wind::direction_speed (w.x, w.y * 0.514444);
      render_queue_draw ();
      return true;
   }
   else
   if (!no_wind && !near_wind_925 && double_click)
   {
      defining_predictor = false;
      predictor.wind_925 = Wind (GSL_NAN, GSL_NAN);
      render_queue_draw ();
      return true;
   }

   return false;

}

bool
Gwsb_Free::on_mouse_motion (const Dmouse_Motion_Event& event)
{

   if (Gwsb::on_mouse_motion (event)) { return true; }
   const Point_2D& point = event.point;

   if (defining_predictor)
   {
      const Transform_2D& transform = wind_disc.get_transform ();
      const Point_2D& w = transform.reverse (point);
      predictor.wind_925 = Wind::direction_speed (w.x, w.y * 0.514444);
      render_queue_draw ();
      return true;
   }

   return false;

}

bool
Gwsb_Free::on_mouse_button_released (const Dmouse_Button_Event& event)
{

   if (Gwsb::on_mouse_button_released (event)) { return true; }
   const Point_2D& point = event.point;

   if (defining_predictor)
   {
      defining_predictor = false;
      return true;
   }

   return false;

}

void
Gwsb_Free::render_image_buffer (const RefPtr<Context>& cr)
{

   const Dtime& dtime = time_chooser.get_time ();

   render (cr, dtime, predictor);

}

