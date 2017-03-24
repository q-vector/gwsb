#ifndef GWSB_DATA_H
#define GWSB_DATA_H

#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>

namespace gwsb
{

   class Criteria;
   class Gwsb;

   class Record
   {

      public:

         Dtime
         dtime;

         Wind
         gradient_wind;

         Real
         gradient_temperature;

         Wind
         wind;

         Record (const Dtime& dtime,
                 const Wind& gradient_wind,
                 const Real gradient_temperature,
                 const Wind& wind);

   };

   class Hourly_Data : public vector<Record>
   {
   };

   class Monthly_Data : public map<Integer, Hourly_Data>
   {

      public:
   
         Monthly_Data ();

         void
         add (const Integer hour,
              const Record& record);

   };

   class Station_Data : public map<Integer, Monthly_Data>
   {

      private:

         void
         add (const Integer month,
              const Integer hour,
              const Record& record);

      public:

         Station_Data ();

         void
         read (const Dstring& file_path);

         void
         feed (Wind_Rose& wind_rose,
               const Gwsb& gwsb) const;

         vector<Record>*
         get_record_vector_ptr (const Gwsb& gwsb) const;

   };

   class Data : public map<Dstring, Station_Data>
   {

      private:

         const Dstring
         data_path;

         Tokens
         station_tokens;

         void
         survey ();

      public:

         Data (const Dstring& data_path,
               const Dstring& station_string);

         const Tokens&
         get_station_tokens () const;

         Station_Data&
         get_station_data (const Dstring& station);

   };

};

#endif /* DENISE_GTKMM_H */

