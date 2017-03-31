#ifndef GWSB_GW_H
#define GWSB_GW_H

#include <set>
#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include <denise/stat.h>
#include "data.h"

using namespace std;

namespace gwsb
{

   class Criteria;
   class Gwsb;

   class Nwp_Gw : public Wind
   {

      public:

         Dstring
         station;

         Dtime
         base_time;

         Real
         forecast_hour;

         Nwp_Gw (const Dstring& station,
                 const Dtime& base_time,
                 const Real forecast_hour,
                 const Wind& wind);

         Dtime
         get_dtime () const;

         bool
         operator == (const Nwp_Gw& nwp_gw) const;
         
         bool
         operator > (const Nwp_Gw& nwp_gw) const;
         
         bool
         operator < (const Nwp_Gw& nwp_gw) const;

         void
         render (const RefPtr<Context>& cr,
                 const Wind_Disc& wind_disc) const;

         class Sequence : public set<Nwp_Gw>
         {

            public:

               vector<Dtime>
               get_time_vector () const;

               void
               run (Data& data,
                    const Size_2D& size_2d,
                    const Wind_Disc& wind_disc) const;

            class Map : public map <Dstring, Sequence>
            {

               private:

                  Tokens
                  keys;

               public:

                  Map (const Dstring& dir_path);

                  void
                  ingest (const Dstring& sequence_file_path);

                  void
                  run (Data& data,
                       const Size_2D& size_2d,
                       const Wind_Disc& wind_disc) const;

            };

         };

   };

};

#endif /* GWSB_GW_H */

