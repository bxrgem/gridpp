#ifndef FILE_NETCDF_H
#define FILE_NETCDF_H
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include "File.h"
#include "../Variable.h"

//! Represents a Netcdf data file
class FileNetcdf : public File {
   public:
      FileNetcdf(std::string iFilename, const Options& iOptions, bool iReadOnly=false);
      ~FileNetcdf();

      virtual std::string getVariableName(Variable::Type iVariable) const = 0;

      //! Add attribute to a variable (overwrite if existing). Variable must exist
      void setAttribute(std::string iVariable, std::string iName, std::string iValue);

      //! Get string attribute belowing to a variable. Variable must exist.
      //! Returns "" if attribute does not exist.
      std::string getAttribute(std::string iVariable, std::string iName);

      //! Add global attribute to file (overwrite if existing)
      void setGlobalAttribute(std::string iName, std::string iValue);

      //! Add global attribute to file (append to attribute if existing)
      void appendGlobalAttribute(std::string iName, std::string iValue);

      //! Add global attribute to file (prepend to attribute if existing)
      void prependGlobalAttribute(std::string iName, std::string iValue);

      //! Get global string attribute. Returns "" if non-existant.
      std::string getGlobalAttribute(std::string iName);
   protected:
      float getScale(int iVar) const;
      float getOffset(int iVar) const;
      int mFile;

      // Does this file contain the variable?
      bool hasVariableCore(Variable::Type iVariable) const;
      bool hasVariableCore(std::string iVariable) const;

      //! Add attribute to a variable (overwrite if existing)
      void setAttribute(int iVar, std::string iName, std::string iValue);
      std::string getAttribute(int iVar, std::string iName);

      int    getDim(std::string iDim) const;
      int    getVar(std::string iVar) const;
      int    getDimSize(std::string iDim) const;
      int    getDimSize(int iDim) const;
      int    getNumDims(int iVar) const;
      bool   hasDim(std::string iDim) const;
      bool   hasVar(std::string iVar) const;
      static bool   hasDim(int iFile, std::string iDim);
      static bool   hasVar(int iFile, std::string iVar);
      float getMissingValue(int iVar) const;
      void  setMissingValue(int iVar, float iValue)const ;
      void defineTimes();
      void defineReferenceTime();
      void defineGlobalAttributes();
      void writeTimes();
      void writeReferenceTime();
      void handleNetcdfError(int status, std::string message="") const;

      // Netcdf file must be in define mode when adding variables and attributes
      // but in data mode when data is read or written. However it is not clear how
      // to know which mode the file is currently in and an error occurs when attempting
      // to switch to the same mode the file currently is in. Keep track of this in mInDataMode
      void startDefineMode() const;
      void startDataMode() const;
      mutable bool mInDataMode;
      const static int mMaxAttributeLength = 100000000;
};
#include "Ec.h"
#include "Arome.h"
#endif
