#ifndef GWSB_GWSB_H
#define GWSB_GWSB_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
//#include "selection.h"
#include "data.h"
#include "predictor.h"

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

         Predictor::Sequence::Map
         sequence_map;

         Real
         wind_925_threshold;

         Predictor
         predictor;

         bool
         defining_predictor;

         virtual void
         pack ();

         void
         render_histogram (const RefPtr<Context>& cr,
                           const Record::Set& record_set,
                           const Predictor& predictor) const;

         void
         render_scatter_plot (const RefPtr<Context>& cr,
                              const Record::Set& record_set,
                              const Real dir_scatter) const;

         void
         render_predictor (const RefPtr<Context>& cr,
                           const Predictor& predictor,
                           const bool faint) const;

         void
         render (const RefPtr<Context>& cr,
                 const Dtime& dtime,
                 const Predictor& predictor);


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

         virtual void
         set_station (const Dstring& station);

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
         update_predictor ();

         void
         render_image_buffer (const RefPtr<Context>& cr);

         virtual void
         render_background_buffer (const RefPtr<Context>& cr);

         virtual void
         clear_clusters ();

   };

};

#endif /* GWSB_GWSB_H */
