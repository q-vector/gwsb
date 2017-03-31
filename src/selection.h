#ifndef GWSB_SELECTION_H
#define GWSB_SELECTION_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include "data.h"

namespace gwsb
{

   class Gwsb_Free;
   class Month_Panel;
   class Hour_Panel;
   class Station_Panel;

   class Selection_Panel : public Dpack_Box
   {

      protected:

         class Button : public Dbutton
         {

            private:

               const Integer
               value;

               Selection_Panel&
               selection_panel;

            public:

               Button (Selection_Panel& selection_panel,
                       const Integer value,
                       const Gwsb_Free& gwsb_free,
                       const string& str,
                       const Real font_size);

               void
               clicked (const Dmouse_Button_Event& event);

         };

         Dpack_Box
         box;

         Dbutton
         decrement_button;

         Dbutton
         increment_button;

         Gwsb_Free&
         gwsb_free;

         map<Integer, Dbutton*>
         button_ptr_map;

         map<Integer, bool>
         status_map;

         map<Integer, string>
         string_map;

         const Color
         led_color;

      public:

         Selection_Panel (Gwsb_Free& gwsb_free,
                          const Real margin,
                          const Real spacing);

         ~Selection_Panel ();

         set<Integer>
         get_value_set () const;

         void
         set_value (const Integer value,
                    const bool render_and_refrsh = true);

         void
         toggle (const Integer month);

   };

   class Month_Panel : public Selection_Panel
   {

      private:

         map<Integer, string>
         string_map;

         void
         add_month (const Integer month);

      public:

         Month_Panel (Gwsb_Free& gwsb_free,
                      const Real margin,
                      const Real spacing);

         string
         get_string () const;

   };

   class Hour_Panel : public Selection_Panel
   {

      private:

         map<Integer, string>
         string_map;

         void
         add_hour (const Integer hour);

      public:

         Hour_Panel (Gwsb_Free& gwsb_free,
                     const Real margin,
                     const Real spacing);

         string
         get_string () const;

   };

   class Station_Panel : public Dgrid_Box
   {

      private:

         class Button : public Dbutton
         {

            private:

               const string
               station;

               Station_Panel&
               station_panel;

            public:

               Button (Station_Panel& station_panel,
                       const string& station,
                       const Gwsb_Free& gwsb_free,
                       const Real font_size);

               void
               clicked (const Dmouse_Button_Event& event);

         };

         Gwsb_Free&
         gwsb_free;

         string
         station;

         map<string, Dbutton*>
         button_ptr_map;

         const Color
         led_color;

      public:

         Station_Panel (Gwsb_Free& gwsb_free,
                        const Real margin,
                        const Real spacing);

         ~Station_Panel ();

         void
         set_station (const string& station);

         const string&
         get_station () const;

   };

};

#endif /* GWSB_SELECTION_H */
