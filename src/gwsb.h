#ifndef GWSB_GWSB_H
#define GWSB_GWSB_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include "selection.h"
#include "data.h"

namespace gwsb
{

   class Bottom_Panel : public Drawer_Panel
   {

      private:

         Gwsb_Free&
         gwsb_free;

         Dtoggle_Button
         noise_button;

         Dtoggle_Button
         outline_button;

         Dbutton
         save_button;

      public:

         Bottom_Panel (Gwsb_Free& gwsb_free);

         ~Bottom_Panel ();

         bool
         with_noise () const;

         bool
         with_outline () const;

   };

   class Gwsb : public Dcanvas
   {

      protected:

         Gtk::Window*
         window_ptr;

         Box_2D
         viewport;

         Data
         data;

         Wind_Disc
         wind_disc;

         void
         pack ();

         static void
         render_bg (const RefPtr<Context>& cr,
                    const Real width,
                    const Real height,
                    const Box_2D& viewport);

         static void
         render_count (const RefPtr<Context>& cr,
                       const Integer count,
                       const Box_2D& viewport);

      public:

         Gwsb (Gtk::Window* window_ptr,
               const Size_2D& size_2d,
               const Data& data,
               const Wind_Disc& wind_disc);

         ~Gwsb ();

         const Data&
         get_data () const;

         Criteria&
         get_criteria ();

         void
         save_image ();

         bool
         save (const Dstring& file_path);

   };

   class Gwsb_Free : public Gwsb
   {

      protected:

         Station_Panel
         station_panel;

         Month_Panel
         month_panel;

         Hour_Panel
         hour_panel;

         Bottom_Panel
         bottom_panel;

         set<Index_2D>
         gradient_wind_index_set;

         bool
         selecting_gradient_wind;

      public:

         Gwsb_Free (Gtk::Window* window_ptr,
                    const Size_2D& size_2d,
                    const Data& data,
                    const Wind_Disc& wind_disc);

         void
         pack ();

         const set<Index_2D>&
         get_gradient_wind_index_set () const;

         set<Integer>
         get_month_set () const;

         set<Integer>
         get_hour_set () const;

         bool
         add_gradient_wind_index (const Index_2D& index_2d,
                                  const bool clear_first = false,
                                  const bool delete_if_present = false);

         void
         delete_gradient_wind_index (const Index_2D& index_2d);

         void
         clear_gradient_wind_index ();

         bool
         match_gradient_wind (const Wind& gradient_wind) const;

         bool
         on_key_pressed (const Dkey_Event& event);

         bool
         on_mouse_button_pressed (const Dmouse_Button_Event& event);

         bool
         on_mouse_motion (const Dmouse_Motion_Event& event);

         bool
         on_mouse_button_released (const Dmouse_Button_Event& event);

         void
         increment_month ();

         void
         decrement_month ();

         void
         increment_hour ();

         void
         decrement_hour ();

         static void
         render (const RefPtr<Context>& cr,
                 const Wind_Disc& wind_disc,
                 const Record::Set& record_set,
                 const set<Index_2D>& gradient_wind_index_set,
                 const Box_2D& viewport,
                 const bool outline,
                 const bool with_noise);

         void
         render ();

         void
         render_refresh ();

   };

};

#endif /* GWSB_GWSB_H */
