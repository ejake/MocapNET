/** @file benchmark.cpp
 *  @brief This is a benchmark tool to get some performance estimates out of MocapNET, using the flag --cpu you can force testing on CPU
 *  In order to correctly measure time we get input/output pairs that are declared in
 * testCodeInput.hpp, testCodeOutput.hpp, testCodeJSONInput.hpp, testCodeJSONOutput.hpp and run them through MocapNET ( runMocapNET call )
 * for each run of the network we record the time taken and then we also extract the average time per sample.
 * There are also two internal side tests to check if the input "compression" to NDSM matrices is working correctly.
 *  @author Ammar Qammaz (AmmarkoV)
 */
#include "../MocapNETLib/mocapnet.hpp"
#include "testCodeInput.hpp"
#include "testCodeOutput.hpp"
#include "testCodeJSONInput.hpp"
#include "testCodeJSONOutput.hpp"


#include "../MocapNETLib/tools.h"
#include "../MocapNETLib/jsonCocoSkeleton.h"
#include "../MocapNETLib/jsonMocapNETHelpers.hpp"

#include <iostream>
#include <vector>
#include <math.h>
#include <string.h>

#define NORMAL   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */



int testMocapNETJSONCompression()
{
 unsigned int errors=0;
 unsigned int width=1920,height=1080;
 char path[] = "frames/dance.webm-data/";

  struct skeletonCOCO * skeleton= (struct skeletonCOCO * ) malloc(sizeof(struct skeletonCOCO));
  if (skeleton!=0)
  {
   unsigned int i=0;
   unsigned int frameLimit=1;
   char filePathOfJSONFile[1024];
   while (i<frameLimit)
    {
     snprintf(filePathOfJSONFile,1024,"%s/colorFrame_0_%05u_keypoints.json",path,i);

     std::vector<float> ourCompressed;
     if (parseJsonCOCOSkeleton(filePathOfJSONFile,skeleton))
       {
        std::vector<float> inputValues171 = flattenskeletonCOCOToVector(skeleton,width,height);
        ourCompressed = prepareMocapNETInputFromUncompressedInput(inputValues171); //This works for sure..
       }

       std::vector<float> groundValues749;
       unsigned int inputCounter = 749 * i;
       for (unsigned int z=0; z<749; z++)
        {
         groundValues749.push_back(MocapNETTestJSONRawInput[inputCounter+z]);
        }

       for (unsigned int z=0; z<749; z++)
        {
           float distance=0.0;
           //---------------------------------------------------------------------------
           if (ourCompressed[z]>groundValues749[z]) { distance=ourCompressed[z]-groundValues749[z]; } else
                                                    { distance=groundValues749[z]-ourCompressed[z]; }
           //---------------------------------------------------------------------------
           if (distance<=0.01) { fprintf(stderr,GREEN "%u %s(%0.2f) " NORMAL,z,MocapNETInputUncompressedAndCompressedArrayNames[z],distance);  } else
           if (distance<5.0)   { fprintf(stderr,YELLOW "%u %s(%0.2f [c%0.4f] [g%0.4f]) " NORMAL,z,MocapNETInputUncompressedAndCompressedArrayNames[z],distance,ourCompressed[z],groundValues749[z]); ++errors; } else
                               { fprintf(stderr,RED "%u %s(%0.2f) " NORMAL,z,MocapNETInputUncompressedAndCompressedArrayNames[z],distance);    ++errors;  }
           //fprintf(stderr," ( %0.2f vs %0.2f ) ",ourCompressed[z],inputValues749[z]);
        }
        fprintf(stderr,"\n");

      ++i;
     }
    free(skeleton);
   }

  return (errors==0);
}
//-------------------------------------------------------------------------------------------------

int testMocapNETCompression()
{
  unsigned int errors=0;
  std::vector<float> inputValues749;
  std::vector<float> inputValues171;
  for (unsigned int i=0; i<MocapNETTestInputNumberOfSamples; i++)
      {
        inputValues749.clear();
        inputValues171.clear();
        unsigned int inputCounter = 749 * i;
        for (unsigned int z=0; z<171; z++)
        {
         inputValues171.push_back(MocapNETTestInput[inputCounter+z]);
        }
        for (unsigned int z=0; z<749; z++)
        {
         inputValues749.push_back(MocapNETTestInput[inputCounter+z]);
        }

        std::vector<float> ourCompressed = prepareMocapNETInputFromUncompressedInput(inputValues171);

        fprintf(stderr,"Sample %u :",i);


        if (inputValues749.size()!=749)
        {
           fprintf(stderr,RED "Utterly Wrong inputValues 749.." NORMAL);
           return 0;
        }
        if (ourCompressed.size()!=749)
        {
           fprintf(stderr,RED "Utterly Wrong ourCompressed %lu .." NORMAL,ourCompressed.size());
           return 0;
        }

        for (unsigned int z=0; z<749; z++)
        {
           float distance=0.0;
           //---------------------------------------------------------------------------
           if (ourCompressed[z]>inputValues749[z]) { distance=ourCompressed[z]-inputValues749[z]; } else
                                                   { distance=inputValues749[z]-ourCompressed[z]; }
           //---------------------------------------------------------------------------
           if (distance<0.1) { fprintf(stderr,GREEN "%u %s(%0.2f) " NORMAL,z,MocapNETInputUncompressedAndCompressedArrayNames[z],distance);  } else
           if (distance<5.0) { fprintf(stderr,YELLOW "%u %s(%0.2f) " NORMAL,z,MocapNETInputUncompressedAndCompressedArrayNames[z],distance); ++errors; } else
                             { fprintf(stderr,RED "%u %s(%0.2f) " NORMAL,z,MocapNETInputUncompressedAndCompressedArrayNames[z],distance);    ++errors;  }
           //fprintf(stderr," ( %0.2f vs %0.2f ) ",ourCompressed[z],inputValues749[z]);
        }
        fprintf(stderr,"\n");
      }

   return (errors==0);
}
//-------------------------------------------------------------------------------------------------











int main(int argc, char *argv[])
{
  int useCPUOnly=1;
  for (int i=0; i<argc; i++)
  {
    //if (strcmp(argv[i],"--cpu")==0)      { setenv("CUDA_VISIBLE_DEVICES", "", 1);  } else
    if (strcmp(argv[i],"--gpu")==0)      { useCPUOnly=0;  } else
    if (strcmp(argv[i],"--test")==0)     { testMocapNETCompression(); exit(0);     } else
    if (strcmp(argv[i],"--testJSON")==0) { testMocapNETJSONCompression(); exit(0); }
  }

  struct MocapNET mnet={0};
  if ( loadMocapNET(&mnet,"test",useCPUOnly) )
  {
   std::vector<float> inputValues;
   std::vector<float> outputValuesExpected;
   if (MocapNETTestInputNumberOfSamples!=MocapNETTestOutputNumberOfSamples)
   {
     std::cerr<<"Wrong number of input/output samples .. \n";
   } else
   {
    unsigned int numberOfRepetitions=5;
    float totalTime=0.0;
    for (int u=0; u<numberOfRepetitions; u++)
     {
      unsigned int inputCounter=0;
      unsigned int outputCounter=0;
      for (int i=0; i<MocapNETTestInputNumberOfSamples; i++)
      {
       inputValues.clear();
       //--------------------------------------------------------
       for (int z=0; z<MocapNETTestInputElementsPerSample; z++)
       {
         inputValues.push_back(MocapNETTestInput[inputCounter]);
         ++inputCounter;
       }

       outputValuesExpected.clear();
       //--------------------------------------------------------
       for (int z=0; z<MocapNETTestOutputElementsPerSample; z++)
       {
         outputValuesExpected.push_back(MocapNETTestOutput[outputCounter]);
         ++outputCounter;
       }

       long startTime = GetTickCountMicrosecondsMN();
       //--------------------------------------------------------
        std::vector<float>  result = runMocapNET(&mnet,inputValues);
       //--------------------------------------------------------
       long endTime = GetTickCountMicrosecondsMN();

       float sampleTime = (float) (endTime-startTime)/1000;
       totalTime+=sampleTime;

        float mae=0.0;
       //--------------------------------------------------------
        for (int z=0; z<MocapNETTestOutputElementsPerSample; z++)
        {
          float roundedExpected = round(outputValuesExpected[z]*1000)/1000;
          float roundedResult = round(result[z]*1000)/1000;

          if (z!=4) //Ignore 4th because it has orientation trick..
           {
            float difference = roundedExpected-roundedResult;
            difference = difference * difference;
            mae+=difference;
           }
        }
       mae/=MocapNETTestOutputElementsPerSample-1;

       if (mae<3) { fprintf(stderr,GREEN);  } else
       if (mae<5) { fprintf(stderr,YELLOW); } else
                  { fprintf(stderr,RED);    }
       fprintf(stderr,"Sample %u/%u - %0.4fms - mae %0.4f \n" NORMAL, i , MocapNETTestInputNumberOfSamples , sampleTime , mae);
      }
     }

     float averageTime=(float) totalTime/(numberOfRepetitions*MocapNETTestInputNumberOfSamples);
     if (averageTime==0.0) { averageTime=0.000001; } //Take care of division by zero
     std::cerr<<"\nTotal "<<totalTime<<"ms for "<<(numberOfRepetitions*MocapNETTestInputNumberOfSamples)<<" samples - Average "<<averageTime<<"ms - "<<1000/averageTime<<" fps\n";
   }

   unloadMocapNET(&mnet);
  }
}
