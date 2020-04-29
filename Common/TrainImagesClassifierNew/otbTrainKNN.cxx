/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
/*=========================================================================
 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
 See OTBCopyright.txt for details.


 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "otbTrainImagesClassifier.h"


namespace otb
{
namespace Wrapper
{
#ifdef OTB_USE_OPENCV
  void TrainImagesClassifier::InitKNNParams()
  {
    AddChoice("classifier.knn", "KNN classifier");
    SetParameterDescription("classifier.knn", "This group of parameters allows to set KNN classifier parameters. "
        "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/k_nearest_neighbors.html}.");

    //K parameter
    AddParameter(ParameterType_Int, "classifier.knn.k", "Number of Neighbors");
    SetParameterInt("classifier.knn.k", 32);
    SetParameterDescription("classifier.knn.k","The number of neighbors to use.");

  }


  void TrainImagesClassifier::TrainKNN(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample)
  {
    KNNType::Pointer knnClassifier = KNNType::New();
    knnClassifier->SetInputListSample(trainingListSample);
    knnClassifier->SetTargetListSample(trainingLabeledListSample);
    knnClassifier->SetK(GetParameterInt("classifier.knn.k"));

    knnClassifier->Train();
    knnClassifier->Save(GetParameterString("io.out"));
  }
#endif
} //end namespace wrapper
} //end namespace otb
