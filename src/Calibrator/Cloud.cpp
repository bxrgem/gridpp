#include "Cloud.h"
#include "../Util.h"
#include "../File/File.h"
CalibratorCloud::CalibratorCloud(const Variable& iVariable, const Options& iOptions) :
      Calibrator(iVariable, iOptions),
      mPrecipType(Variable::Precip) {
}
bool CalibratorCloud::calibrateCore(File& iFile, const ParameterFile* iParameterFile) const {
   int nLat = iFile.getNumLat();
   int nLon = iFile.getNumLon();
   int nEns = iFile.getNumEns();
   int nTime = iFile.getNumTime();

   // Loop over offsets
   for(int t = 0; t < nTime; t++) {
      const Field& precip = *iFile.getField(mPrecipType, t);
      Field& cloud        = *iFile.getField(mVariable, t);

      // TODO: Figure out which cloudless members to use. Ideally, if more members
      // need precip, we should pick members that already have clouds, so that we minimize
      // our effect on the cloud cover field.

      #pragma omp parallel for
      for(int i = 0; i < nLat; i++) {
         for(int j = 0; j < nLon; j++) {
            // Turn on clouds if needed, i.e don't allow a member to
            // have precip without cloud cover.
            for(int e = 0; e < nEns; e++) {
               float currPrecip = precip(i,j,e);
               float currCloud  = cloud(i,j,e);
               if(Util::isValid(currPrecip) && Util::isValid(currCloud)) {
                  cloud(i,j,e)  = currCloud;
                  // std::cout << "currPrecip = " << currPrecip << std::endl;
                  if(currPrecip > 0 && currCloud < 1) {
                     cloud(i,j,e) = 1;
                  }
               }
            }
         }
      }
   }
   return true;
}
std::string CalibratorCloud::description() {
   std::stringstream ss;
   ss << Util::formatDescription("-c cloud", "Ensures that every ensemble member with precipitation also has complete cloud cover.") << std::endl;
   return ss.str();
}
