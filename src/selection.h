#ifndef GWSB_SELECTION_H
#define GWSB_SELECTION_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include "data.h"

namespace gwsb
{

   class Gwsb;
   class Gwsb_Free;
   class Month_Panel;
   class Hour_Panel;

   class Selection_Panel : public Dpack_Box
   {

      protected:

         class Month_Map : public map<Dstring, Integer>
         {

            public:

               Month_Map ();

               Integer
               get_integer (const Dstring& str) const;

         };

         class Status : public map<Dstring, bool>
         {

            public:

               Integer
               number_on () const;

               bool
               zero_on () const;

               bool
               one_on () const;

               const Dstring&
               last () const;

               const Dstring&
               first () const;

         };

         Month_Map
         month_map;

         Dpack_Box
         box;

         Dbutton
         decrement_button;

         Dbutton
         increment_button;

         Gwsb_Free&
         gwsb_free;

         map<Dstring, Dbutton*>
         button_ptr_map;

         Status
         status;

         const Color
         led_color;

      public:

         Selection_Panel (Gwsb_Free& gwsb_free,
                          const Real margin,
                          const Real spacing);

         ~Selection_Panel ();

         void
         handle (const Dstring& str,
                 const Devent& event);

         void
         set_value (const Dstring& str,
                    const bool render_and_refrsh = true);

         void
         toggle (const Dstring& str);

         void
         increment ();

         void
         decrement ();

   };

   class Month_Panel : public Selection_Panel
   {

      public:

         Month_Panel (Gwsb_Free& gwsb_free,
                      const Real margin,
                      const Real spacing);

         string
         get_string () const;

   };

   class Hour_Panel : public Selection_Panel
   {

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

         Gwsb&
         gwsb;

         string
         station;

         map<Dstring, Dbutton*>
         button_ptr_map;

         const Color
         led_color;

      public:

         Station_Panel (Gwsb& gwsb,
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
