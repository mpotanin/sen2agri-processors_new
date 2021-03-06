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

  Program:   phenotb
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(phenoParameterCostFunctionInstance);
  REGISTER_TEST(phenoNormalizedSigmoid);
  REGISTER_TEST(phenoNormalizedSigmoidMetrics);
  REGISTER_TEST(phenoEarlyEmergence);
  REGISTER_TEST(phenoEarlyEmergenceCB);
  REGISTER_TEST(phenoTwoCycleFittingFunctor);
}
