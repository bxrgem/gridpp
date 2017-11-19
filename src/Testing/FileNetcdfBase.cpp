#include "../File/Arome.h"
#include "../Util.h"
#include "../Downscaler/Downscaler.h"
#include "../Calibrator/Zaga.h"
#include <gtest/gtest.h>

// For each test it is safe to assume that 10x10_copy.nc is identical to 10x10.nc
// After the test is done, it is safe to assume that 10x10_copy.nc is again reverted.
namespace {
   class FileNetcdfBase : public ::testing::Test {
      public:
         void reset10x10() const {
            Util::copy("testing/files/10x10.nc", "testing/files/10x10_copy.nc");
         };
         virtual void SetUp() {
            reset10x10();
         };
         virtual void TearDown() {
            reset10x10();
         };

      protected:
   };

   TEST_F(FileNetcdfBase, overwriteAttribute) {
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      file.setGlobalAttribute("history", "test512");
      EXPECT_EQ("test512", file.getGlobalAttribute("history"));
   }
   TEST_F(FileNetcdfBase, addAttribute) {
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      file.setGlobalAttribute("history2", "test123");
      EXPECT_EQ("test123", file.getGlobalAttribute("history2"));
   }
   TEST_F(FileNetcdfBase, missingAttribute) {
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      std::string att = file.getGlobalAttribute("qowhoiqfhoiqhdow");
      EXPECT_EQ("", att);
   }
   TEST_F(FileNetcdfBase, appendAttribute) {
      // Check that appending and prepending works
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      file.setGlobalAttribute("history", "empty");
      file.prependGlobalAttribute("history",  "testing");
      file.appendGlobalAttribute("history",  "testing2");
      EXPECT_EQ("testing\nempty\ntesting2", file.getGlobalAttribute("history"));

      // Writing should not thrown an error
      std::vector<Variable::Type> vars;
      file.write(vars);
   }
   TEST_F(FileNetcdfBase, appendAttributeEmpty) {
      // Check that appending and prepending to an empty attribute works
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      file.prependGlobalAttribute("history71623",  "value321");
      file.appendGlobalAttribute("history99311",  "value15");
      EXPECT_EQ("value321", file.getGlobalAttribute("history71623"));
      EXPECT_EQ("value15",  file.getGlobalAttribute("history99311"));
   }
   TEST_F(FileNetcdfBase, setAttribute) {
      // Check that appending and prepending to an empty attribute works
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      file.setGlobalAttribute("att1",     "value93824");
      file.appendGlobalAttribute("att1",  "append");
      file.setGlobalAttribute("att1",     "value321192839819");

      file.setAttribute("air_temperature_2m", "att1", "value71");
      file.setAttribute("air_temperature_2m", "att1", "value72");
      file.setAttribute("air_temperature_2m", "att1", "value73");

      file.setGlobalAttribute("att2",  "value15");
      std::vector<Variable::Type> vars;
      vars.push_back(Variable::T);
      file.write(vars);
      FileArome file2 = FileArome("testing/files/10x10_copy.nc");
      EXPECT_EQ("value321192839819", file.getGlobalAttribute("att1"));
      EXPECT_EQ("value15",  file.getGlobalAttribute("att2"));
      EXPECT_EQ("value73",  file.getAttribute("air_temperature_2m", "att1"));
      EXPECT_EQ("",  file.getAttribute("air_temperature_2m", "att2"));
   }
   TEST_F(FileNetcdfBase, inandoutOfDataMode) {
      // Check that we can go in and out of data mode without error
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      // Define attributes
      file.setAttribute("air_temperature_2m", "att1", "value71");
      EXPECT_EQ("value71",  file.getAttribute("air_temperature_2m", "att1"));
      // Read data
      FieldPtr field = file.getField(Variable::T, 0);
      std::vector<Variable::Type> vars(1,Variable::T);
      // Write data
      file.write(vars);

      // Add more attributes
      file.setAttribute("air_temperature_2m", "att1", "value72");
      EXPECT_EQ("value72",  file.getAttribute("air_temperature_2m", "att1"));
   }
   TEST_F(FileNetcdfBase, setAttributeError) {
      ::testing::FLAGS_gtest_death_test_style = "threadsafe";
      Util::setShowError(false);

      FileArome file = FileArome("testing/files/10x10_copy.nc");

      // Variable do not exist
      EXPECT_DEATH(file.setAttribute("nonvalid_variable", "units", "value93824"), ".*");
      EXPECT_DEATH(file.getAttribute("q", "att1"), ".*");
   }
   TEST_F(FileNetcdfBase, setLongAttribute) {
      // Attempt to create a really long attribute
      {
         FileArome file = FileArome("testing/files/10x10_copy.nc");
         std::stringstream ss;
         for(int i = 0; i < 1e7; i++) {
            ss << "1234567890";
         }
         ss << "1234";
         std::string value = ss.str();
         file.appendGlobalAttribute("history", value);
         std::vector<Variable::Type> vars(1,Variable::T);
         file.write(vars);
      }
      // Make sure the attribute hasn't been set to the really long value
      FileArome file = FileArome("testing/files/10x10_copy.nc");
      std::string value = file.getGlobalAttribute("history");
      EXPECT_TRUE(value.size() < 1e8);
   }
   TEST_F(FileNetcdfBase, createNewVariable) {
      FileArome file("testing/files/10x10_copy.nc");
      std::vector<Variable::Type> vars;
      vars.push_back(Variable::Pop6h);
      std::vector<float> pars(8,0);
      Parameters par(pars);
      ParameterFileSimple parFile(par);
      file.initNewVariable(Variable::Pop6h);
      CalibratorZaga cal(Variable::Pop6h, Options("outputPop=1 neighbourhoodSize=1 fracThreshold=0.4 popThreshold=0.5 6h=1"));
      cal.calibrate(file, &parFile);
      file.write(vars);
   }
}
int main(int argc, char **argv) {
     ::testing::InitGoogleTest(&argc, argv);
       return RUN_ALL_TESTS();
}
