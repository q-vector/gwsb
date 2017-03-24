#include <gtkmm/messagedialog.h>
#include "data.h"
#include "selection.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Bottom_Panel::Bottom_Panel (Gwsb& gwsb)
   : Drawer_Panel (gwsb, true, 12),
     gwsb (gwsb),
     noise_button (gwsb, "+Noise", 12),
     outline_button (gwsb, "Outline", 12),
     save_button (gwsb, "Save", 12)
{

   add_drawer ("Option");
   add_drawer ("Tool");

   noise_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_refresh));
   outline_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::render_refresh));
   save_button.get_signal ().connect (sigc::mem_fun (
      gwsb, &Gwsb::save_image));

   add_widget_ptr ("Option", &noise_button);
   add_widget_ptr ("Option", &outline_button);
   add_widget_ptr ("Tool", &save_button);

}

Bottom_Panel::~Bottom_Panel ()
{
}

bool
Bottom_Panel::with_noise () const
{
   return noise_button.is_switched_on ();
}

bool
Bottom_Panel::with_outline () const
{
   return outline_button.is_switched_on ();
}

void
Gwsb::pack ()
{

   const Real margin = 6;
   const Real title_height = 40;

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

   station_panel.being_packed (sp_anchor, sp_width, sp_height);
   station_panel.pack ();

   month_panel.being_packed (mp_anchor, mp_width, mp_height);
   month_panel.pack ();

   hour_panel.being_packed (hp_anchor, hp_width, hp_height);
   hour_panel.pack ();

   bottom_panel.being_packed (bp_anchor, bp_width, bp_height);
   bottom_panel.pack ();

   const Real viewport_width = width - hp_width - mp_width - margin * 4;
   const Real viewport_height = height - title_height - sp_height - margin * 3;
   const Real viewport_x = width - viewport_width - margin;
   const Real viewport_y = height - viewport_height - margin;
   const Real origin_x = viewport_x + viewport_width / 2;
   const Real origin_y = viewport_y + viewport_height / 2;
   const Point_2D origin (origin_x, origin_y);

   viewport.index_2d.i = Integer (round (viewport_x));
   viewport.index_2d.j = Integer (round (viewport_y));
   viewport.size_2d.i = Integer (round (viewport_width));
   viewport.size_2d.j = Integer (round (viewport_height));

   const Real max_radius = std::min (viewport_width, viewport_height) * 0.475;
   wind_disc.set_position (origin, max_radius);

   this->packed = true;

}

void
Gwsb::render_scatter_plot (const RefPtr<Context> cr,
                           const Wind_Disc& wind_disc,
                           const Real dir_scatter,
                           const vector<Record>& record_vector) const
{

   const Real scatter_ring_size = 8;

   Real alpha = 50.0 / wind_disc.get_total_count ();
   if (alpha < 0.04) { alpha = 0.04; }
   if (alpha > 0.30) { alpha = 0.30; }

   const Ring ring (scatter_ring_size);

   const Wind_Disc::Transform& t = wind_disc.get_transform ();
   const Real max_speed = t.get_max_speed ();
   const Real calm_threshold = wind_disc.get_thresholds ().front ().value;

   const Transform_2D& transform = wind_disc.get_transform ();

   for (const Record& record : record_vector)
   {

      const Real gradient_temperature = record.gradient_temperature;
      const Wind& wind = record.wind;
      const Real multiplier = 0.51444444;
      const Real speed = wind.get_speed () / multiplier;

      if (wind.is_naw ()) { continue; }
      if (speed < calm_threshold) { continue; }
      if (speed > max_speed) { continue; }

      Real hue = -.027777 * (gradient_temperature)  + 0.69444;
      if (hue < 0) { hue = 0; }
      if (hue > 0.833) { hue = 0.833; }
      const Color& color = Color::hsb (hue, 0.4, 0.8, alpha);
      const Color& color_a2 = Color::hsb (hue, 0.4, 0.8, alpha * 2);
cout << hue << " " << gradient_temperature << endl;

      const Real r = random (dir_scatter, -dir_scatter);
      const Real direction = wind.get_direction () + r;

      ring.cairo (cr, transform.transform (Point_2D (direction, speed)));
      color.cairo (cr);
      cr->fill_preserve ();
      color_a2.cairo (cr);
      cr->stroke ();

   }


}

Gwsb::Gwsb (Gtk::Window* window_ptr,
            const Dstring& data_path,
            const Dstring& station_string,
            const Size_2D& size_2d,
            const Dstring& gwsb_dir_path,
            const Integer number_of_directions,
            const Tuple& threshold_tuple,
            const Tuple& speed_label_tuple,
            const Real max_speed)
   : Dcanvas (*window_ptr),
     wind_disc (number_of_directions,
                threshold_tuple,
                Point_2D (size_2d.i / 2, size_2d.j / 2),
                size_2d.j / 4,
                speed_label_tuple,
                max_speed),
     window_ptr (window_ptr),
     data (data_path, station_string),
     month_panel (*this, 0, 6),
     hour_panel (*this, 0, 6),
     station_panel (*this, 0, 6),
     bottom_panel (*this),
     viewport (Size_2D (0, 0)),
     selecting_gradient_wind (false)
{

   Gdk::EventMask event_mask = (Gdk::SCROLL_MASK);
   event_mask |= (Gdk::POINTER_MOTION_MASK);
   event_mask |= (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
   event_mask |= (Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
   set_events (event_mask);

   // This basically sets the minimum size
   set_size_request (size_2d.i, size_2d.j);
   set_can_focus ();

   set_preferred_size (size_2d.i, size_2d.j);
   being_packed (Point_2D (0, 0), size_2d.i, size_2d.j);

   const Dtime now;
   month_panel.set_value (now.get_month (), false);
   hour_panel.set_value (now.get_hour (), false);

   register_widget (month_panel);
   register_widget (hour_panel);
   register_widget (station_panel);
   register_widget (bottom_panel);

}

Gwsb::~Gwsb ()
{
}

const Data&
Gwsb::get_data () const
{
   return data;
}

const set<Index_2D>&
Gwsb::get_gradient_wind_index_set () const
{
   return gradient_wind_index_set;
}

set<Integer>
Gwsb::get_month_set () const
{
   return month_panel.get_value_set ();
}

set<Integer>
Gwsb::get_hour_set () const
{
   return hour_panel.get_value_set ();
}

bool
Gwsb::add_gradient_wind_index (const Index_2D& index_2d,
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
Gwsb::delete_gradient_wind_index (const Index_2D& index_2d)
{
   set<Index_2D>::iterator iterator = gradient_wind_index_set.find (index_2d);
   if (iterator != gradient_wind_index_set.end ())
   {
      gradient_wind_index_set.erase (iterator);
   }
}

void
Gwsb::clear_gradient_wind_index ()
{
   gradient_wind_index_set.clear ();
}

bool
Gwsb::match_gradient_wind (const Wind& gradient_wind) const
{

   if (gradient_wind_index_set.size () == 0) { return true; }

   typedef set<Index_2D>::const_iterator Iterator;
   const Index_2D& gw_index = wind_disc.get_index (gradient_wind);

   for (Iterator iterator = gradient_wind_index_set.begin ();
        iterator != gradient_wind_index_set.end (); iterator++)
   {
      const Index_2D& i2d = *(iterator);
      if (i2d == gw_index) { return true; }
   }

   return false;

}

bool
Gwsb::on_key_pressed (const Dkey_Event& event)
{

   switch (event.value)
   {

      case GDK_KEY_Up:
      {
         decrement_month ();
         render ();
         queue_draw ();
         break;
      }

      case GDK_KEY_Down:
      {
         increment_month ();
         render ();
         queue_draw ();
         break;
      }

      case GDK_KEY_Left:
      {
         decrement_hour ();
         render ();
         queue_draw ();
         break;
      }

      case GDK_KEY_Right:
      {
         increment_hour ();
         render ();
         queue_draw ();
         break;
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

}

bool
Gwsb::on_mouse_button_pressed (const Dmouse_Button_Event& event)
{

   if (Dcontainer::on_mouse_button_pressed (event)) { return true; }
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

         render ();
         queue_draw ();
         return true;
         break;

      }

   }

   return false;

}

bool
Gwsb::on_mouse_motion (const Dmouse_Motion_Event& event)
{

   if (Dcontainer::on_mouse_motion (event)) { return true; }
   const Point_2D& point = event.point;

   if (selecting_gradient_wind)
   {

      try
      {
         const Wind& gradient_wind = wind_disc.get_wind (point);
         const Index_2D& index_2d = wind_disc.get_index (gradient_wind);
         if (add_gradient_wind_index (index_2d, false, false))
         {
            render ();
            queue_draw ();
            return true;
         }
      }
      catch (const Exception& e)
      {
      }

   }

   return false;

}

bool
Gwsb::on_mouse_button_released (const Dmouse_Button_Event& event)
{

   if (Dcontainer::on_mouse_button_released (event)) { return true; }
   const Point_2D& point = event.point;

   bool processed = false;

/*
   switch (event.button)
   {

      case 1:
      {

         try
         {
            const bool control_pressed = (event.state & GDK_CONTROL_MASK);
            if (control_pressed)
            {
               const Wind& gradient_wind = wind_disc.get_wind (point);
               const Index_2D& index_2d = wind_disc.get_index (gradient_wind);
               if (!dragging_gradient_wind)
               {
                  delete_gradient_wind_index (index_2d);
                  render ();
                  refresh ();
                  processed = true;
               }
            }
         }
         catch (const Exception& e)
         {
         }
         break;

      }
   }
*/
      
   selecting_gradient_wind = false;
   return processed;

}

void
Gwsb::increment_month ()
{
   set<Integer> month_set = month_panel.get_value_set ();
   if (month_set.size () == 0) { return; }
   const Integer last_month = *(month_set.rbegin ());
   const Integer month = ((last_month) % 12) + 1;
   month_panel.set_value (month);
}

void
Gwsb::decrement_month ()
{
   set<Integer> month_set = month_panel.get_value_set ();
   if (month_set.size () == 0) { return; }
   const Integer first_month = *(month_set.begin ());
   const Integer month = ((first_month + 10) % 12) + 1;
   month_panel.set_value (month);
}

void
Gwsb::increment_hour ()
{
   set<Integer> hour_set = hour_panel.get_value_set ();
   if (hour_set.size () == 0) { return; }
   const Integer last_hour = *(hour_set.rbegin ());
   const Integer hour = (last_hour + 1) % 24;
   hour_panel.set_value (hour);
}

void
Gwsb::decrement_hour ()
{
   set<Integer> hour_set = hour_panel.get_value_set ();
   if (hour_set.size () == 0) { return; }
   const Integer first_hour = *(hour_set.begin ());
   const Integer hour = (first_hour + 23) % 24;
   hour_panel.set_value (hour);
}

void
Gwsb::render ()
{

   if (!packed) { pack (); }

   wind_disc.clear ();
   const Dstring& station = station_panel.get_station ();
   Station_Data& station_data = data.get_station_data (station);
   station_data.feed (wind_disc, *this);

   const vector<Record>* record_vector_ptr =
      station_data.get_record_vector_ptr (*this);
   const vector<Record>& record_vector = *record_vector_ptr;
   const RefPtr<Context> cr = Context::create (image_surface);

   {
      const Color& color_0 = Color::hsb (0.6, 0.4, 0.8);
      const Color& color_1 = Color::hsb (0.6, 0.1, 0.4);
      const Point_2D point_0 (0, 0);
      const Point_2D point_1 (0, height);
      Color_Gradient (color_0, color_1, point_0, point_1).cairo (cr);
      cr->paint ();
   }

   {
      Color (0.0, 0.0, 0.0, 0.4).cairo (cr);
      const Point_2D point (viewport.index_2d.i, viewport.index_2d.j);
      Rect (point, viewport.size_2d.i, viewport.size_2d.j).cairo (cr);
      cr->fill_preserve ();
      Color ("black").cairo (cr);
      cr->stroke ();
   }

   const bool outline = (bottom_panel.with_outline ());
   const Real dir_noise = (bottom_panel.with_noise () ? 5 : 0);
   wind_disc.render (cr, 0.33, outline, dir_noise);

   render_scatter_plot (cr, wind_disc, 5, record_vector);

   for (const Index_2D& index_2d : gradient_wind_index_set)
   {
      wind_disc.render_index (cr, index_2d);
   }

   const Dstring& month_string = month_panel.get_string ();
   const Dstring& hour_string = hour_panel.get_string ();
   title.set (month_string, station, hour_string);
   set_foreground_ready (false);

   const Integer total_count = wind_disc.get_total_count ();
   const Dstring& str = Dstring::render ("%d points", total_count);
   const Color& color_bg = Color::hsb (0.0, 0.0, 0.4, 0.2);
   const Color& color_fg = Color::hsb (0.0, 0.0, 0.8, 0.8);

   const Real margin = 6;
   cr->set_font_size (12);
   Label label (str, Point_2D (width - 2 * margin, 100 + 3 * margin), 'r', 't');
   color_bg.cairo (cr);
   label.set_offset (Point_2D (2, -2));
   label.cairo (cr);
   color_fg.cairo (cr);
   label.set_offset (Point_2D (0, -0));
   label.cairo (cr);

   Dcanvas::cairo (cr);

}

void
Gwsb::render_refresh ()
{

   const Dstring& station = station_panel.get_station ();
   const Dstring& month_string = month_panel.get_string ();
   const Dstring& hour_string = hour_panel.get_string ();
   title.set (month_string, station, hour_string);
   set_foreground_ready (false);

   render ();
   queue_draw ();
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

