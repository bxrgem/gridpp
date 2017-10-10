#include <sstream>
#include "Field.h"
Field::Field(int nLat, int nLon, int nEns, float iFillValue) :
      mNLat(nLat), mNLon(nLon), mNEns(nEns) {
   if(Util::isValid(nLat) && Util::isValid(nLon) && Util::isValid(nEns)
         && nLat >= 0 && nLon >= 0 && nEns >= 0) {
      mValues.resize(nLat*nLon*nEns, iFillValue);
   }
   else {
      std::stringstream ss;
      ss << "Cannot create field of size [" << nLat << "," << nLon << "," << nEns << "]";
      Util::error(ss.str());
   }
}

std::vector<float> Field::operator()(unsigned int i, unsigned int j) const {
   std::vector<float> values(mValues.begin()+getIndex(i, j, 0), mValues.begin()+getIndex(i, j, mNEns-1)+1);
   return values;
}

int Field::getNumLat() const {
   return mNLat;
}
int Field::getNumLon() const {
   return mNLon;
}
int Field::getNumEns() const {
   return mNEns;
}

bool Field::operator==(const Field& iField) const {
   return mValues == iField.mValues;
}
bool Field::operator!=(const Field& iField) const {
   return mValues != iField.mValues;
}
