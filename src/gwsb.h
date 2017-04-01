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

      public:

         Option_Panel (Gwsb& gwsb);

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

         Wind_Disc&
         wind_disc;

         Station_Panel
         station_panel;

         Option_Panel
         option_panel;

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
               const Tokens& station_tokens,
               const Data& data,
               Wind_Disc& wind_disc);

         ~Gwsb ();

         const Data&
         get_data () const;

         void
         save_image ();

         bool
         save (const Dstring& file_path);

         virtual const Tokens&
         get_station_tokens () const = 0; 

         virtual bool
         on_key_pressed (const Dkey_Event& event);

         virtual bool
         on_mouse_button_pressed (const Dmouse_Button_Event& event);

         virtual bool
         on_mouse_motion (const Dmouse_Motion_Event& event);

         virtual bool
         on_mouse_button_released (const Dmouse_Button_Event& event);

         virtual void
         render () = 0;

         virtual void
         render_refresh () = 0;

   };

   class Gwsb_Free : public Gwsb
   {

      protected:

         Month_Panel
         month_panel;

         Hour_Panel
         hour_panel;

         set<Index_2D>
         gradient_wind_index_set;

         bool
         selecting_gradient_wind;

      public:

         Gwsb_Free (Gtk::Window* window_ptr,
                    const Size_2D& size_2d,
                    const Data& data,
                    Wind_Disc& wind_disc);

         void
         pack ();

         virtual const Tokens&
         get_station_tokens () const; 

         const set<Index_2D>&
         get_gradient_wind_index_set () const;

         const Selection_Panel::Status&
         get_month_status () const;

         const Selection_Panel::Status&
         get_hour_status () const;

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

   class Gwsb_Sequence : public Gwsb
   {

      protected:

         Nwp_Gw::Sequence::Map
         sequence_map;

         Time_Chooser
         time_chooser;

         Real
         gradient_wind_threshold;

      public:

         Gwsb_Sequence (Gtk::Window* window_ptr,
                        const Size_2D& size_2d,
                        const Nwp_Gw::Sequence::Map& sequence_map,
                        const Data& data,
                        Wind_Disc& wind_disc);

         void
         pack ();

         virtual const Tokens&
         get_station_tokens () const; 

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

         static void
         render (const RefPtr<Context>& cr,
                 const Wind_Disc& wind_disc,
                 const Record::Set& record_set,
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
