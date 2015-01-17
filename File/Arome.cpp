#include "Ec.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "../Util.h"

FileArome::FileArome(std::string iFilename) : FileNetcdf(iFilename) {
   // Set dimensions
   NcDim* dTime = getDim("time");
   NcDim* dLon  = getDim("y");
   NcDim* dLat  = getDim("x");
   mNTime = dTime->size();
   mNLat  = dLat->size();
   mNLon  = dLon->size();

   mLats = getLatLonVariable("latitude");
   mLons = getLatLonVariable("longitude");
   // mElevs = getLatLonVariable("altitude");

   Util::status( "File '" + iFilename + " 'has dimensions " + getDimenionString());
}

FieldPtr FileArome::getFieldCore(Variable::Type iVariable, int iTime) const {
   std::string variable = getVariableName(iVariable);
   // Not cached, retrieve data
   NcVar* var = getVar(variable);
   int nTime = mNTime;
   int nLat  = mNLat;
   int nLon  = mNLon;

   long count[5] = {1, 1, nLon, nLat};
   float* values = new float[nTime*nLat*nLon];
   var->set_cur(iTime, 0, 0, 0);
   var->get(values, count);
   float MV = getMissingValue(var);

   float offset = getOffset(var);
   float scale = getScale(var);
   int index = 0;
   FieldPtr field = getEmptyField();
   for(int lon = 0; lon < nLon; lon++) {
      for(int lat = 0; lat < nLat; lat++) {
         float value = values[index];
         if(Util::isValid(MV) && value == MV) {
            // Field has missing value indicator and the value is missing
            // Save values using our own internal missing value indicator
            value = Util::MV;
         }
         else {
            value = scale*values[index] + offset;
         }
         (*field)[lat][lon][0] = value;
         index++;
      }
   }
   delete[] values;
   return field;
}

FileArome::~FileArome() {
}

void FileArome::writeCore(std::vector<Variable::Type> iVariables) {
   for(int v = 0; v < iVariables.size(); v++) {
      Variable::Type varType = iVariables[v];
      std::string variable = getVariableName(varType);
      NcVar* var;
      if(hasVariable(varType)) {
         var = getVar(variable);
      }
      else {
         // Create variable
         NcDim* dTime    = getDim("time");
         NcDim* dSurface = getDim("height0");
         NcDim* dLon     = getDim("y");
         NcDim* dLat     = getDim("x");
         var = mFile.add_var(variable.c_str(), ncFloat, dTime, dSurface, dLon, dLat);
      }
      float MV = getMissingValue(var); // The output file's missing value indicator
      for(int t = 0; t < mNTime; t++) {
         float offset = getOffset(var);
         float scale = getScale(var);
         FieldPtr field = getField(varType, t);
         if(field != NULL) { // TODO: Can't be null if coming from reference
            float* values = new float[mNTime*mNLat*mNLon];

            int index = 0;
            for(int lon = 0; lon < mNLon; lon++) {
               for(int lat = 0; lat < mNLat; lat++) {
                  float value = (*field)[lat][lon][0];
                  if(Util::isValid(MV) && !Util::isValid(value)) {
                     // Field has missing value indicator and the value is missing
                     // Save values using the file's missing indicator value
                     value = MV;
                  }
                  else {
                     value = ((*field)[lat][lon][0] - offset)/scale;
                  }
                  values[index] = value;
                  index++;
               }
            }
            var->set_cur(t, 0, 0, 0);
            var->put(values, 1, 1, mNLon, mNLat);
         }
      }
   }
}


std::string FileArome::getVariableName(Variable::Type iVariable) const {
   if(iVariable == Variable::PrecipAcc) {
      return "precipitation_amount_acc";
   }
   else if(iVariable == Variable::Cloud) {
      return "cloud_area_fraction";
   }
   else if(iVariable == Variable::T) {
      return "air_temperature_2m";
   }
   else if(iVariable == Variable::Precip) {
      return "precipitation_amount";
   }
   else if(iVariable == Variable::U) {
      return "x_wind_10m";
   }
   else if(iVariable == Variable::V) {
      return "y_wind_10m";
   }
   return "";
}

vec2 FileArome::getLats() const {
   return mLats;
}
vec2 FileArome::getLons() const {
   return mLons;
}
vec2 FileArome::getElevs() const {
   return mElevs;
}

vec2 FileArome::getLatLonVariable(std::string iVar) const {
   NcVar* var = getVar(iVar);
   long count[2] = {getNumLon(), getNumLat()};
   float* values = new float[getNumLon()*getNumLat()];
   var->get(values, count);
   vec2 grid;
   grid.resize(getNumLat());
   for(int i = 0; i < getNumLat(); i++) {
      grid[i].resize(getNumLon());
      for(int j = 0; j < getNumLon(); j++) {
         int index = j*getNumLat() + i;
         grid[i][j] = values[index];
      }
   }
   return grid;
}
