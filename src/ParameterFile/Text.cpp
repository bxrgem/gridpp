#include "Text.h"
#include <fstream>
#include <sstream>
#include "../Util.h"
#include <assert.h>
#include <set>
#include <fstream>
#include <algorithm>

ParameterFileText::ParameterFileText(const Options& iOptions, bool iIsNew) : ParameterFile(iOptions, iIsNew) {
   if(iIsNew)
      return;
   std::ifstream ifs(getFilename().c_str(), std::ifstream::in);
   if(!ifs.good()) {
      return;
   }
   int numParameters = Util::MV;
   int counter = 0;
   std::set<int> times;
   std::vector<std::string> columnNames;
   int timePos = Util::MV;
   int latPos = Util::MV;
   int lonPos = Util::MV;
   int elevPos = Util::MV;
   while(ifs.good()) {
      char line[10000];
      ifs.getline(line, 10000, '\n');
      if(ifs.good() && line[0] != '#') {
         std::stringstream ss(line);
         if(columnNames.size() == 0) {
            int currCounter = 0;
            while(ss.good()) {
               std::string value;
               bool status = ss >> value;
               if(!status) {
                  Util::error("Could not read header from file '" + mFilename + "'");
               }
               columnNames.push_back(value);
               if(value == "time")
                  timePos = currCounter;
               else if(value == "lat")
                  latPos = currCounter;
               else if(value == "lon")
                  lonPos = currCounter;
               else if(value == "elev")
                  elevPos = currCounter;
               currCounter++;
            }
         }
         else {
            // Loop over each value
            std::vector<float> values;
            int currCounter = 0;
            int time = 0;
            float lat, lon, elev;
            while(ss.good()) {
               if(currCounter == timePos) {
                  bool status = ss >> time;
                  if(!status) {
                     Util::error("Could not read time from file '" + mFilename + "'");
                  }
               }
               else if(currCounter == latPos) {
                  bool status = ss >> lat;
                  if(!status) {
                     Util::error("Could not read lat from file '" + mFilename + "'");
                  }
               }
               else if(currCounter == lonPos) {
                  bool status = ss >> lon;
                  if(!status) {
                     Util::error("Could not read lon from file '" + mFilename + "'");
                  }
               }
               else if(currCounter == elevPos) {
                  bool status = ss >> elev;
                  if(!status) {
                     Util::error("Could not read elev from file '" + mFilename + "'");
                  }
               }
               else {
                  float value;
                  bool status  = ss >> value;
                  if(!status) {
                     Util::error("Could not read value from file '" + mFilename + "'");
                  }
                  values.push_back(value);
               }
               currCounter++;
            }
            times.insert(time);
            Location location(0,0,0);
            if(Util::isValid(lat) && Util::isValid(lon) && Util::isValid(elev)) {
               location = Location(lat, lon, elev);
            }

            if(numParameters == Util::MV)
               numParameters = values.size();
            else if(values.size() != numParameters) {
               std::stringstream ss;
               ss << "Parameter file '" + getFilename() + "' is corrupt, because it does not have the same"
                  << " number of columns on each line" << std::endl;
               Util::error(ss.str());
            }
            Parameters parameters(values);
            setParameters(parameters, time, location);
            counter++;
         }
      }
   }
   ifs.close();
   mTimes = std::vector<int>(times.begin(), times.end());
   std::sort(mTimes.begin(), mTimes.end());

   std::stringstream ss;
   ss << "Reading " << mFilename << ". Found " << counter << " parameter sets.";
   Util::status(ss.str());

   recomputeTree();
}

std::vector<int> ParameterFileText::getTimes() const {
   return mTimes;
}

bool ParameterFileText::isFixedSize() const {
   return true;
}

bool ParameterFileText::isValid(std::string iFilename) {
   std::ifstream ifs(iFilename.c_str(), std::ifstream::in);
   if(!ifs.good()) {
      return false;
   }
   return true;
}

bool ParameterFileText::isReadable() const {
   std::ifstream ifs(getFilename().c_str(), std::ifstream::in);
   if(!ifs.good()) {
      return false;
   }
   return true;
}

void ParameterFileText::write() const {
   write(mFilename);
}
void ParameterFileText::write(const std::string& iFilename) const {
   std::string filename = iFilename;
   std::ofstream ofs(filename.c_str(), std::ios_base::trunc);
   if(!ofs.good()) {
      Util::error("Cannot write parameters to " + filename);
   }

   std::map<Location, std::vector<Parameters> >::const_iterator it;
   // Loop over times
   if(isLocationDependent())
      ofs << "# time lat lon elev" << std::endl;
   else
      ofs << "# time" << std::endl;
   for(int i = 0; i < getNumParameters(); i++) {
      ofs << " param" << i;
   }
   for(it = mParameters.begin(); it != mParameters.end(); it++) {
      // Loop over locations
      const Location& location = it->first;
      for(int i = 0; i < it->second.size(); i++) {
         int time = i;
         const Parameters& parameters = it->second[i];
         if(parameters.size() != 0) {
            ofs << time;
            if(isLocationDependent()) {
               ofs << " " << location.lat() << " " << location.lon() << " " << location.elev();
            }
            // Loop over parameter values
            for(int i = 0; i < parameters.size(); i++) {
               ofs << " " << parameters[i];
            }
            ofs << std::endl;
         }
      }
   }
   ofs.close();
}

bool ParameterFileText::isLocationDependent() const {
   return mParameters.size() > 1;
}

std::string ParameterFileText::description() {
   std::stringstream ss;
   ss << Util::formatDescription("-p text", "Simple ascii text file with space separated entries. Each line represents one lead time and/or location, and each column one parameter. If lat/lon is missing, then parameters are assumed global. If time is missing, then the same parameters are used for all times.") << std::endl;
   ss << Util::formatDescription("", "time lat lon elev param1 param2 ... paramN") << std::endl;
   ss << Util::formatDescription("", "offset1 a b ... N") << std::endl;
   ss << Util::formatDescription("", "...") << std::endl;
   ss << Util::formatDescription("", "offsetM a b ... N") << std::endl;
   return ss.str();
}
