#ifndef GWSB_SELECTION_H
#define GWSB_SELECTION_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>

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

               const Tokens
               tokens;

               Month_Map ();

               const Tokens&
               get_tokens () const;

               Integer
               get_integer (const Dstring& str) const;

         };

      public:

         class Status : public map<Dstring, bool>
         {

            public:

               void
               set_all (const bool on);

               void
               set (const Tokens& tokens);

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

      protected:

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

         const Status&
         get_status () const;

         void
         handle (const Dstring& str,
                 const Devent& event);

         void
         set_value (const Dstring& str,
                    const bool render_and_refrsh = true);

         void
         toggle (const Dstring& str);

         virtual void
         increment () = 0;

         virtual void
         decrement () = 0;

   };

   class Month_Panel : public Selection_Panel
   {

      public:

         Month_Panel (Gwsb_Free& gwsb_free,
                      const Real margin,
                      const Real spacing);

         string
         get_string () const;

         void
         increment ();

         void
         decrement ();

   };

   class Hour_Panel : public Selection_Panel
   {

      public:

         Hour_Panel (Gwsb_Free& gwsb_free,
                     const Real margin,
                     const Real spacing);

         string
         get_string () const;

         void
         increment ();

         void
         decrement ();

   };

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

};

#endif /* GWSB_SELECTION_H */
