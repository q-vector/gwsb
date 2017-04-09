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

         Spin_Button
         day_of_year_threshold_button;

         Spin_Button
         hour_threshold_button;

         Dtoggle_Button
         noise_button;

         Dtoggle_Button
         outline_button;

         Dtoggle_Button
         cluster_button;

         Dtoggle_Button
         percentages_button;

         Dbutton
         clear_clusters_button;

         Dbutton
         save_button;

      public:

         Option_Panel (Gwsb& gwsb);

         void
         toggle_noise ();

         void
         toggle_outline ();

         void
         toggle_cluster ();

         void
         toggle_percentages ();

         bool
         with_noise () const;

         bool
         with_outline () const;

         bool
         with_cluster () const;

         bool
         with_percentages () const;

         Integer
         get_day_of_year_threshold () const;

         Integer
         get_hour_threshold () const;

   };

   class Gwsb : public Dcanvas
   {

      protected:

         Gtk::Window*
         window_ptr;

         Clusters
         clusters;

         Data
         data;

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
         wind_925_threshold;

         virtual void
         pack ();

         void
         render_histogram (const RefPtr<Context>& cr,
                           const set<Record>& record_set,
                           const Predictor& predictor) const;

         void
         render (const RefPtr<Context>& cr,
                 const Dtime& dtime,
                 const Predictor& predictor);


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

         virtual void
         set_station (const Dstring& station) = 0;

         virtual Record::Set*
         get_record_set_ptr (const Dtime& dtime,
                             const Predictor& predictor);

         virtual bool
         on_key_pressed (const Dkey_Event& event);

         virtual bool
         on_mouse_button_pressed (const Dmouse_Button_Event& event);

         virtual bool
         on_mouse_motion (const Dmouse_Motion_Event& event);

         virtual bool
         on_mouse_button_released (const Dmouse_Button_Event& event);

         virtual bool
         on_mouse_scroll (const Dmouse_Scroll_Event& event);

         virtual void
         render_background_buffer (const RefPtr<Context>& cr);

         virtual void
         clear_clusters ();

   };

   class Gwsb_Sequence : public Gwsb
   {

      protected:

         Predictor::Sequence::Map
         sequence_map;

      public:

         Gwsb_Sequence (Gtk::Window* window_ptr,
                        const Size_2D& size_2d,
                        const Predictor::Sequence::Map& sequence_map,
                        const Data& data,
                        Wind_Disc& wind_disc);

         ~Gwsb_Sequence ();

         void
         set_station (const Dstring& station);

         Record::Set*
         get_record_set_ptr ();

         void
         render_image_buffer (const RefPtr<Context>& cr);

   };

   class Gwsb_Free : public Gwsb
   {

      protected:

         Predictor
         predictor;

         bool
         defining_predictor;

      public:

         Gwsb_Free (Gtk::Window* window_ptr,
                    const Size_2D& size_2d,
                    const Tokens& station_tokens,
                    const Data& data,
                    Wind_Disc& wind_disc);

         ~Gwsb_Free ();

         void
         set_station (const Dstring& station);

         Record::Set*
         get_record_set_ptr ();

         bool
         on_mouse_button_pressed (const Dmouse_Button_Event& event);

         bool
         on_mouse_motion (const Dmouse_Motion_Event& event);

         bool
         on_mouse_button_released (const Dmouse_Button_Event& event);

         void
         render_image_buffer (const RefPtr<Context>& cr);

   };

};

#endif /* GWSB_GWSB_H */
