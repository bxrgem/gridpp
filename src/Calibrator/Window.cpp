#include "Window.h"
#include "../Util.h"
#include "../File/File.h"
CalibratorWindow::CalibratorWindow(const Variable& iVariable, const Options& iOptions) :
      Calibrator(iVariable, iOptions),
      mLength(7),
      mBefore(false),
      mStatType(Util::StatTypeMean),
      mEdgePolicy(EdgePolicyCompute),
      mQuantile(Util::MV) {
   iOptions.getValue("length", mLength);
   iOptions.getValue("before", mBefore);
   std::string edgePolicy;
   iOptions.getValue("edgePolicy", edgePolicy);
   if(edgePolicy == "compute")
      mEdgePolicy = EdgePolicyCompute;
   else if(edgePolicy == "missing")
      mEdgePolicy = EdgePolicyMissing;
   else {
      std::stringstream ss;
      ss << "Cannot understand edgePolicy=" << edgePolicy;
      Util::error(ss.str());
   }

   if(mLength < 1) {
      std::stringstream ss;
      ss << "CalibratorWindow: 'length' (" << mLength << ") must be > 0";
      Util::error(ss.str());
   }
   if(!mBefore && mLength % 2 != 1) {
      std::stringstream ss;
      ss << "CalibratorWindow: 'length' (" << mLength << ") must be an odd number if before=0";
      Util::error(ss.str());
   }

   std::string op;
   if(iOptions.getValue("stat", op)) {
      if(op == "mean") {
         mStatType = Util::StatTypeMean;
      }
      else if(op == "min") {
         mStatType = Util::StatTypeQuantile;
         mQuantile = 0;
      }
      else if(op == "max") {
         mStatType = Util::StatTypeQuantile;
         mQuantile = 1;
      }
      else if(op == "median") {
         mStatType = Util::StatTypeQuantile;
         mQuantile = 0.5;
      }
      else if(op == "std") {
         mStatType = Util::StatTypeStd;
      }
      else if(op == "quantile"){
         mStatType = Util::StatTypeQuantile;
         if(!iOptions.getValue("quantile", mQuantile)) {
            Util::error("CalibratorWindow: option 'quantile' is required");
         }
         if(!Util::isValid(mQuantile) || mQuantile < 0 || mQuantile > 1) {
            Util::error("CalibratorWindow: 'quantile' must be on the interval [0,1]");
         }
      }
      else {
         Util::error("CalibratorWindow: Unrecognized value for 'stat'");
      }
   }
   iOptions.check();
}
bool CalibratorWindow::calibrateCore(File& iFile, const ParameterFile* iParameterFile) const {
   int nLat = iFile.getNumY();
   int nLon = iFile.getNumX();
   int nEns = iFile.getNumEns();
   int nTime = iFile.getNumTime();

   // Get all fields
   std::vector<FieldPtr> fields(nTime);
   std::vector<FieldPtr> fieldsOrig(nTime);
   for(int t = 0; t < nTime; t++) {
      fieldsOrig[t]     = iFile.getField(mVariable, t);
      fields[t] = iFile.getEmptyField();
   }

   std::vector<float> window;
   window.resize(mLength, Util::MV);
   for(int t = 0; t < nTime; t++) {
      #pragma omp parallel for
      for(int i = 0; i < nLat; i++) {
         for(int j = 0; j < nLon; j++) {
            for(int e = 0; e < nEns; e++) {
               int index = 0;
               int halfLength = (mLength - 1) / 2;
               int start = std::max(0,t-halfLength);
               int end = std::min(nTime-1, t + halfLength);
               if(mBefore) {
                  start = std::max(0, t - (mLength - 1));
                  end = std::min(nTime-1, t);
               }
               assert(end - start + 1 <= mLength);
               if(mEdgePolicy == EdgePolicyMissing && (end - start + 1 != mLength)) {
                  (*fields[t])(i,j,e) = Util::MV;
               }
               else {
                  for(int tt = start; tt <= end; tt++) {
                     float curr = (*fieldsOrig[tt])(i,j,e);
                     window[index] = curr;
                     index++;
                  }
                  (*fields[t])(i,j,e) = Util::calculateStat(window, mStatType, mQuantile);
               }
            }
         }
      }
   }
   for(int t = 0; t < nTime; t++) {
      iFile.addField(fields[t], mVariable, t);
   }
   return true;
}

std::string CalibratorWindow::description(bool full) {
   std::stringstream ss;
   ss << Util::formatDescription("-c window","Applies a statistical operator to values within a temporal window. Any missing values are ignored when computing the statistic.") << std::endl;
   if(full) {
      ss << Util::formatDescription("   length=required", "Length of the window (in number of timesteps) to apply operator on (must be 0 or greater).") << std::endl;
      ss << Util::formatDescription("   before=false", "If false, the window is centered on each leadtime (if length is an even number, then it is shiftet such that it includes one extra future leadtime). If true, then the window ends and includes at the leadtime.") << std::endl;
      ss << Util::formatDescription("   stat=mean", "What statistical operator should be applied to the window? One of 'mean', 'median', 'min', 'max', 'std', or 'quantile'. 'std' is the population standard deviation.") << std::endl;
      ss << Util::formatDescription("   edgePolicy=compute", "What policy should be used on edges? Either 'compute' to compute as usual, or 'missing' to set missing value if the window is not full.") << std::endl;
      ss << Util::formatDescription("   quantile=undef", "If stat=quantile is selected, what quantile (number on the interval [0,1]) should be used?") << std::endl;
   }
   return ss.str();
}
