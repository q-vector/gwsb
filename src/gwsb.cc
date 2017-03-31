#include <gtkmm/messagedialog.h>
#include "data.h"
#include "selection.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Bottom_Panel::Bottom_Panel (Gwsb_Free& gwsb_free)
   : Drawer_Panel (gwsb_free, true, 12),
     gwsb_free (gwsb_free),
     noise_button (gwsb_free, "+Noise", 12),
     outline_button (gwsb_free, "Outline", 12),
     save_button (gwsb_free, "Save", 12)
{

   add_drawer ("Option");
   add_drawer ("Tool");

   noise_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::render_queue_draw));
   outline_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::render_queue_draw));
   save_button.get_signal ().connect (sigc::mem_fun (
      gwsb_free, &Gwsb_Free::save_image));

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
Gwsb::render_bg (const RefPtr<Context>& cr,
                 const Real width,
                 const Real height,
                 const Box_2D& viewport)
{

//   const Color& color_0 = Color::hsb (0.6, 0.4, 0.8);
//   const Color& color_1 = Color::hsb (0.6, 0.1, 0.4);
   const Color& color_0 = Color::hsb (0.6, 0.4, 0.9);
   const Color& color_1 = Color::hsb (0.6, 0.1, 0.7);
   const Point_2D point_0 (0, 0);
   const Point_2D point_1 (0, height);
   Color_Gradient (color_0, color_1, point_0, point_1).cairo (cr);
   cr->paint ();

   Color (1.0, 1.0, 1.0, 0.4).cairo (cr);
   const Point_2D point (viewport.index_2d.i, viewport.index_2d.j);
   Rect (point, viewport.size_2d.i, viewport.size_2d.j).cairo (cr);
   cr->fill_preserve ();
   Color::black ().cairo (cr);
   cr->stroke ();

}

void
Gwsb::render_count (const RefPtr<Context>& cr,
                    const Integer count,
                    const Box_2D& viewport)
{

   const Dstring fmt (count == 1 ? "%d point" : "%d points");
   const Dstring& str = Dstring::render (fmt, count);
   const Color& color_fg = Color::hsb (0.0, 0.0, 0.2, 0.7);
   const Color& color_bg = Color::hsb (0.0, 0.0, 0.8, 0.9);

   const Real padding = 8;
   const Point_2D ne (viewport.get_ne ());
   const Point_2D& anchor = ne + Point_2D (-padding, padding);
   cr->set_font_size (12);
   Label label (str, anchor, 'r', 't');
   label.cairo (cr, color_fg, color_bg, Point_2D (-3, 3));

}

Gwsb::Gwsb (Gtk::Window* window_ptr,
            const Size_2D& size_2d,
            const Data& data,
            Wind_Disc& wind_disc)
   : Dcanvas (*window_ptr),
     wind_disc (wind_disc),
     window_ptr (window_ptr),
     data (data),
     viewport (Size_2D (0, 0))
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

Gwsb_Free::Gwsb_Free (Gtk::Window* window_ptr,
                      const Size_2D& size_2d,
                      const Data& data,
                      Wind_Disc& wind_disc)
   : Gwsb (window_ptr, size_2d, data, wind_disc),
     station_panel (*this, 0, 6),
     month_panel (*this, 0, 6),
     hour_panel (*this, 0, 6),
     bottom_panel (*this),
     selecting_gradient_wind (false)
{

   const Dtime now;
   month_panel.set_value ("Mar", false);
   hour_panel.set_value ("10Z", false);

   register_widget (station_panel);
   register_widget (month_panel);
   register_widget (hour_panel);
   register_widget (bottom_panel);

}

void
Gwsb_Free::pack ()
{

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

const set<Index_2D>&
Gwsb_Free::get_gradient_wind_index_set () const
{
   return gradient_wind_index_set;
}

set<Integer>
Gwsb_Free::get_month_set () const
{
   return month_panel.get_value_set ();
}

set<Integer>
Gwsb_Free::get_hour_set () const
{
   return hour_panel.get_value_set ();
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
Gwsb_Free::on_mouse_button_pressed (const Dmouse_Button_Event& event)
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
Gwsb_Free::on_mouse_motion (const Dmouse_Motion_Event& event)
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
Gwsb_Free::on_mouse_button_released (const Dmouse_Button_Event& event)
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
Gwsb_Free::increment_month ()
{
   set<Integer> month_set = month_panel.get_value_set ();
   if (month_set.size () == 0) { return; }
   const Integer last_month = *(month_set.rbegin ());
   const Integer month = ((last_month) % 12) + 1;
   month_panel.set_value (month);
}

void
Gwsb_Free::decrement_month ()
{
   set<Integer> month_set = month_panel.get_value_set ();
   if (month_set.size () == 0) { return; }
   const Integer first_month = *(month_set.begin ());
   const Integer month = ((first_month + 10) % 12) + 1;
   month_panel.set_value (month);
}

void
Gwsb_Free::increment_hour ()
{
   set<Integer> hour_set = hour_panel.get_value_set ();
   if (hour_set.size () == 0) { return; }
   const Integer last_hour = *(hour_set.rbegin ());
   const Integer hour = (last_hour + 1) % 24;
   hour_panel.set_value (hour);
}

void
Gwsb_Free::decrement_hour ()
{
   set<Integer> hour_set = hour_panel.get_value_set ();
   if (hour_set.size () == 0) { return; }
   const Integer first_hour = *(hour_set.begin ());
   const Integer hour = (first_hour + 23) % 24;
   hour_panel.set_value (hour);
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
Gwsb_Free::render ()
{


   if (!packed) { pack (); }

   const Dstring& station = station_panel.get_station ();
   const Dstring& month_string = month_panel.get_string ();
   const Dstring& hour_string = hour_panel.get_string ();
   const bool outline = (bottom_panel.with_outline ());
   const Real with_noise = bottom_panel.with_noise ();

   title.set (month_string, station, hour_string);
   set_foreground_ready (false);

   wind_disc.clear ();
   Station_Data& station_data = data.get_station_data (station);
   station_data.feed (wind_disc, *this);
   const Record::Set* record_set_ptr = station_data.get_record_set_ptr (*this);

   const RefPtr<Context> cr = Context::create (image_surface);
   render_bg (cr, width, height, viewport);
   render (cr, wind_disc, *record_set_ptr,
      gradient_wind_index_set, viewport, outline, with_noise);
   Dcanvas::cairo (cr);

   delete record_set_ptr;
}

void
Gwsb_Free::render_refresh ()
{

   const Dstring& station = station_panel.get_station ();
   const Dstring& month_string = month_panel.get_string ();
   const Dstring& hour_string = hour_panel.get_string ();

   title.set (month_string, station, hour_string);
   set_foreground_ready (false);

   render ();
   queue_draw ();

}

