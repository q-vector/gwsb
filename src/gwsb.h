#ifndef GWSB_GWSB_H
#define GWSB_GWSB_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include "selection.h"
#include "data.h"
#include "gw.h"

namespace gwsb
{

   class Station_Panel : public Dgrid_Box
   {

      private:

         Gwsb&
         gwsb;

         map<Dstring, Dbutton*>
         button_ptr_map;

         const Color
         led_color;

      public:

         Station_Panel (Gwsb& gwsb,
                        const Tokens& station_tokens,
                        const Real margin,
                        const Real spacing);

         ~Station_Panel ();

   };

   class Option_Panel : public Drawer_Panel
   {

      private:

         Gwsb&
         gwsb;

         Dtoggle_Button
         noise_button;

         Dtoggle_Button
         outline_button;

         Dbutton
         save_button;

         Dbutton
         histogram_button;

      public:

         Option_Panel (Gwsb& gwsb);

         void
         toggle_noise ();

         void
         toggle_outline ();

         bool
         with_noise () const;

         bool
         with_outline () const;

   };

   class Gwsb : public Dcanvas
   {

      protected:

         class Histogram : public Dcanvas
         {

            private:

               Gwsb&
               gwsb;


            public:

               Histogram (Gtk::Window& gtk_window,
                          Gwsb& gwsb);

               void
               render_image_buffer (const RefPtr<Context>& cr);

         };

         Gtk::Window*
         window_ptr;

         Groups
         groups;

         Data
         data;

         Predictor::Sequence::Map
         sequence_map;

         Wind_Disc&
         wind_disc;

         Dstring
         station;

         Station_Panel
         station_panel;

         Option_Panel
         option_panel;

         Time_Chooser
         time_chooser;

         Real
         gradient_wind_threshold;

         void
         pack ();

         void
         render_bg (const RefPtr<Context>& cr);

         void
         render_histogram (const RefPtr<Context>& cr,
                           const set<Record>& record_set,
                           const Predictor& predictor) const;

      public:

         Gwsb (Gtk::Window* window_ptr,
               const Size_2D& size_2d,
               const Predictor::Sequence::Map& sequence_map,
               const Data& data,
               Wind_Disc& wind_disc);

         ~Gwsb ();

         const Data&
         get_data () const;

         void
         save_image ();

         bool
         save (const Dstring& file_path);

         void
         set_station (const Dstring& station);

         const Tokens&
         get_station_tokens () const; 

         Record::Set*
         get_record_set_ptr ();

         bool
         on_key_pressed (const Dkey_Event& event);

         bool
         on_mouse_button_pressed (const Dmouse_Button_Event& event);

         bool
         on_mouse_motion (const Dmouse_Motion_Event& event);

         bool
         on_mouse_button_released (const Dmouse_Button_Event& event);

         bool
         on_mouse_scroll (const Dmouse_Scroll_Event& event);

         void
         render_background_buffer (const RefPtr<Context>& cr);

         void
         render_image_buffer (const RefPtr<Context>& cr);

         void
         spawn_histogram ();

   };

};

#endif /* GWSB_GWSB_H */
