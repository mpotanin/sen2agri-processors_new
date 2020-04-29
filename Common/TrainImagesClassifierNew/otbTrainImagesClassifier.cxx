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

void TrainImagesClassifier::DoInit()
{
    SetName("TrainImagesClassifierNew");
    SetDescription(
                "Train a classifier from multiple pairs of images and training vector data.");

    // Documentation
    SetDocName("Train a classifier from multiple images");
    SetDocLongDescription(
                "This application performs a classifier training from multiple pairs of input images and training vector data. "
                "Samples are composed of pixel values in each band optionally centered and reduced using an XML statistics file produced by "
                "the ComputeImagesStatistics application.\n The training vector data must contain polygons with a positive integer field "
                "representing the class label. The name of this field can be set using the \"Class label field\" parameter. Training and validation "
                "sample lists are built such that each class is equally represented in both lists. One parameter allows to control the ratio "
                "between the number of samples in training and validation sets. Two parameters allow to manage the size of the training and "
                "validation sets per class and per image.\n Several classifier parameters can be set depending on the chosen classifier. In the "
                "validation process, the confusion matrix is organized the following way: rows = reference labels, columns = produced labels. "
                "In the header of the optional confusion matrix output file, the validation (reference) and predicted (produced) class labels"
                " are ordered according to the rows/columns of the confusion matrix.\n This application is based on LibSVM and on OpenCV Machine Learning "
                "classifiers, and is compatible with OpenCV 2.3.1 and later.");
    SetDocLimitations("None");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso("OpenCV documentation for machine learning http://docs.opencv.org/modules/ml/doc/ml.html ");

    AddDocTag(Tags::Learning);

    //Group IO
    AddParameter(ParameterType_Group, "io", "Input and output data");
    SetParameterDescription("io", "This group of parameters allows to set input and output data.");
    AddParameter(ParameterType_InputImageList, "io.il", "Input Image List");
    SetParameterDescription("io.il", "A list of input images.");
    AddParameter(ParameterType_InputVectorData, "io.vd", "Input Vector Data");
    SetParameterDescription("io.vd", "Vector data to select the training samples.");
    AddParameter(ParameterType_InputFilename, "io.imstat", "Input XML image statistics file");
    MandatoryOff("io.imstat");
    SetParameterDescription("io.imstat",
                            "Input XML file containing the mean and the standard deviation of the input images.");
    AddParameter(ParameterType_OutputFilename, "io.confmatout", "Output confusion matrix");
    SetParameterDescription("io.confmatout", "Output file containing the confusion matrix (.csv format).");
    MandatoryOff("io.confmatout");
    AddParameter(ParameterType_OutputFilename, "io.out", "Output model");
    SetParameterDescription("io.out", "Output file containing the model estimated (.txt format).");

    //LBU
    // Add the possibility to use a raster to describe the training samples.
    MandatoryOff("io.vd");
    AddParameter(ParameterType_InputImageList, "io.rs", "Training samples in a raster");
    SetParameterDescription("io.rs", "Either a single raster or one for each input image containing the training samples.");
    MandatoryOff("io.rs");
    AddParameter(ParameterType_Int, "nodatalabel", "No data label");
    SetParameterDescription("nodatalabel", "The label of the ignored pixels from the raster");
    MandatoryOff("nodatalabel");
    SetDefaultParameterInt("nodatalabel", 0);
    //LBU

    // Elevation
    ElevationParametersHandler::AddElevationParameters(this, "elev");

    //Group Sample list
    AddParameter(ParameterType_Group, "sample", "Training and validation samples parameters");
    SetParameterDescription("sample",
                            "This group of parameters allows to set training and validation sample lists parameters.");

    AddParameter(ParameterType_Int, "sample.mt", "Maximum training sample size per class");
    //MandatoryOff("mt");
    SetDefaultParameterInt("sample.mt", 1000);
    SetParameterDescription("sample.mt", "Maximum size per class (in pixels) of the training sample list (default = 1000) (no limit = -1). If equal to -1, then the maximal size of the available training sample list per class will be equal to the surface area of the smallest class multiplied by the training sample ratio.");
    AddParameter(ParameterType_Int, "sample.mv", "Maximum validation sample size per class");
    // MandatoryOff("mv");
    SetDefaultParameterInt("sample.mv", 1000);
    SetParameterDescription("sample.mv", "Maximum size per class (in pixels) of the validation sample list (default = 1000) (no limit = -1). If equal to -1, then the maximal size of the available validation sample list per class will be equal to the surface area of the smallest class multiplied by the validation sample ratio.");

    AddParameter(ParameterType_Int, "sample.bm", "Bound sample number by minimum");
    SetDefaultParameterInt("sample.bm", 1);
    SetParameterDescription("sample.bm", "Bound the number of samples for each class by the number of available samples by the smaller class. Proportions between training and validation are respected. Default is true (=1).");


    AddParameter(ParameterType_Empty, "sample.edg", "On edge pixel inclusion");
    SetParameterDescription("sample.edg",
                            "Takes pixels on polygon edge into consideration when building training and validation samples.");
    MandatoryOff("sample.edg");

    AddParameter(ParameterType_Float, "sample.vtr", "Training and validation sample ratio");
    SetParameterDescription("sample.vtr",
                            "Ratio between training and validation samples (0.0 = all training, 1.0 = all validation) (default = 0.5).");
    SetParameterFloat("sample.vtr", 0.5);

    AddParameter(ParameterType_String, "sample.vfn", "Name of the discrimination field");
    SetParameterDescription("sample.vfn", "Name of the field used to discriminate class labels in the input vector data files.");
    SetParameterString("sample.vfn", "Class");

    AddParameter(ParameterType_Choice, "classifier", "Classifier to use for the training");
    SetParameterDescription("classifier", "Choice of the classifier to use for the training.");

    //Group LibSVM
#ifdef OTB_USE_LIBSVM 
    InitLibSVMParams();
#endif

#ifdef OTB_USE_OPENCV
    InitSVMParams();
    InitBoostParams();
    InitDecisionTreeParams();
    InitGradientBoostedTreeParams();
    InitNeuralNetworkParams();
    InitNormalBayesParams();
    InitRandomForestsParams();
    InitKNNParams();
#endif

    AddRANDParameter();
    // Doc example parameter settings
    SetDocExampleParameterValue("io.il", "QB_1_ortho.tif");
    SetDocExampleParameterValue("io.vd", "VectorData_QB1.shp");
    SetDocExampleParameterValue("io.imstat", "EstimateImageStatisticsQB1.xml");
    SetDocExampleParameterValue("sample.mv", "100");
    SetDocExampleParameterValue("sample.mt", "100");
    SetDocExampleParameterValue("sample.vtr", "0.5");
    SetDocExampleParameterValue("sample.edg", "false");
    SetDocExampleParameterValue("sample.vfn", "Class");
    SetDocExampleParameterValue("classifier", "libsvm");
    SetDocExampleParameterValue("classifier.libsvm.k", "linear");
    SetDocExampleParameterValue("classifier.libsvm.c", "1");
    SetDocExampleParameterValue("classifier.libsvm.opt", "false");
    SetDocExampleParameterValue("io.out", "svmModelQB1.txt");
    SetDocExampleParameterValue("io.confmatout", "svmConfusionMatrixQB1.csv");
}

void TrainImagesClassifier::DoUpdateParameters()
{
    // Nothing to do here : all parameters are independent
}


void TrainImagesClassifier::LogConfusionMatrix(ConfusionMatrixCalculatorType* confMatCalc)
{
    ConfusionMatrixCalculatorType::ConfusionMatrixType matrix = confMatCalc->GetConfusionMatrix();

    // Compute minimal width
    size_t minwidth = 0;

    for (unsigned int i = 0; i < matrix.Rows(); i++)
    {
        for (unsigned int j = 0; j < matrix.Cols(); j++)
        {
            std::ostringstream os;
            os << matrix(i, j);
            size_t size = os.str().size();

            if (size > minwidth)
            {
                minwidth = size;
            }
        }
    }

    MapOfIndicesType mapOfIndices = confMatCalc->GetMapOfIndices();

    MapOfIndicesType::const_iterator it = mapOfIndices.begin();
    MapOfIndicesType::const_iterator end = mapOfIndices.end();

    for (; it != end; ++it)
    {
        std::ostringstream os;
        os << "[" << it->second << "]";

        size_t size = os.str().size();
        if (size > minwidth)
        {
            minwidth = size;
        }
    }

    // Generate matrix string, with 'minwidth' as size specifier
    std::ostringstream os;

    // Header line
    for (size_t i = 0; i < minwidth; ++i)
        os << " ";
    os << " ";

    it = mapOfIndices.begin();
    end = mapOfIndices.end();
    for (; it != end; ++it)
    {
        os << "[" << it->second << "]" << " ";
    }

    os << std::endl;

    // Each line of confusion matrix
    for (unsigned int i = 0; i < matrix.Rows(); i++)
    {
        ConfusionMatrixCalculatorType::ClassLabelType label = mapOfIndices[i];
        os << "[" << std::setw(minwidth - 2) << label << "]" << " ";
        for (unsigned int j = 0; j < matrix.Cols(); j++)
        {
            os << std::setw(minwidth) << matrix(i, j) << " ";
        }
        os << std::endl;
    }

    otbAppLogINFO("Confusion matrix (rows = reference labels, columns = produced labels):\n" << os.str());
}

void TrainImagesClassifier::Classify(ListSampleType::Pointer validationListSample, LabelListSampleType::Pointer predictedList)
{
    //Classification
    ModelPointerType model = MachineLearningModelFactoryType::CreateMachineLearningModel(GetParameterString("io.out"),
                                                                                         MachineLearningModelFactoryType::ReadMode);

    if (model.IsNull())
    {
        otbAppLogFATAL(<< "Error when loading model " << GetParameterString("io.out"));
    }

    model->Load(GetParameterString("io.out"));
    model->SetInputListSample(validationListSample);
    model->SetTargetListSample(predictedList);
    model->PredictAll();
}

void TrainImagesClassifier::DoExecute()
{
    GetLogger()->Debug("Entering DoExecute\n");
    //Create training and validation for list samples and label list samples
    ConcatenateLabelListSampleFilterType::Pointer concatenateTrainingLabels =
            ConcatenateLabelListSampleFilterType::New();
    ConcatenateListSampleFilterType::Pointer concatenateTrainingSamples = ConcatenateListSampleFilterType::New();
    ConcatenateLabelListSampleFilterType::Pointer concatenateValidationLabels =
            ConcatenateLabelListSampleFilterType::New();
    ConcatenateListSampleFilterType::Pointer concatenateValidationSamples = ConcatenateListSampleFilterType::New();

    MeasurementType meanMeasurementVector;
    MeasurementType stddevMeasurementVector;

    //--------------------------
    // Load measurements from images
    unsigned int nbBands = 0;
    //Iterate over all input images

    FloatVectorImageListType* imageList = GetParameterImageList("io.il");
    auto sampleMt = GetParameterInt("sample.mt");
    auto sampleMv = GetParameterInt("sample.mv");
    if (sampleMt != -1)
    {
        sampleMt *= imageList->Size();
    }
    if (sampleMv != -1)
    {
        sampleMv *= imageList->Size();
    }

    std::map<int, int> classPixels;

    if (this->HasValue("io.vd")) {
        VectorDataType::Pointer vectorData = GetParameterVectorData("io.vd");
        // read the Vectordata
        vectorData->Update();

        auto ok = false;
        std::string errors;
        //Iterate over all input images
        otbAppLogINFO("Number of inputs " << imageList->Size() << std::endl);
        std::cerr << "Number of inputs " << imageList->Size() << std::endl;
        ListSampleGeneratorType::ClassesSizeType classesSize;
        // Setup the DEM Handler
        otb::Wrapper::ElevationParametersHandler::SetupDEMHandlerFromElevationParameters(this, "elev");

        std::cerr << "Computing class counts" << std::endl;

        typedef otb::ObjectList<VectorDataReprojectionType> VectorDataReprojectionListType;
        typedef otb::ObjectList<typename VectorDataReprojectionType::OutputVectorDataType> VectorDataListType;
        VectorDataReprojectionListType::Pointer vectorDataReprojectionList = VectorDataReprojectionListType::New();
        VectorDataListType::Pointer vectorDataList = VectorDataListType::New();
        typedef otb::ObjectList<ListSampleGeneratorType> ListSampleGeneratorListType;
        ListSampleGeneratorListType::Pointer listSampleGenerators = ListSampleGeneratorListType::New();
        for (unsigned int imgIndex = 0; imgIndex < imageList->Size(); ++imgIndex)
        {
            FloatVectorImageType::Pointer image = imageList->GetNthElement(imgIndex);
            image->UpdateOutputInformation();

            std::cerr << "Image " << imgIndex << " vector length " << image->GetNumberOfComponentsPerPixel() << std::endl;

            VectorDataReprojectionType::Pointer vdreproj = VectorDataReprojectionType::New();
            vectorDataReprojectionList->PushBack(vdreproj);

            vdreproj->SetInputImage(image);
            vdreproj->SetInput(vectorData);
            vdreproj->SetUseOutputSpacingAndOriginFromImage(false);
            vdreproj->Update();

            vectorDataList->PushBack(vdreproj->GetOutput());
            ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();
            sampleGenerator->SetInput(image);
            sampleGenerator->SetInputVectorData(vectorDataList->GetNthElement(imgIndex));

            sampleGenerator->SetClassKey(GetParameterString("sample.vfn"));
            sampleGenerator->SetMaxTrainingSize(sampleMt);
            sampleGenerator->SetMaxValidationSize(sampleMv);
            sampleGenerator->SetValidationTrainingProportion(GetParameterFloat("sample.vtr"));
            sampleGenerator->SetBoundByMin(GetParameterInt("sample.bm")!=0);

            // take pixel located on polygon edge into consideration
            if (IsParameterEnabled("sample.edg"))
            {
                sampleGenerator->SetPolygonEdgeInclusion(true);
            }

            sampleGenerator->GenerateClassStatistics();

            for (const auto &entry : sampleGenerator->GetClassesSize()) {
                classesSize[entry.first] += entry.second;
            }
        }

        for (const auto &entry : classesSize) {
            std::cerr << entry.first << ' ' << entry.second << '\n';
        }
        std::cerr << std::endl;

        for (unsigned int imgIndex = 0; imgIndex < imageList->Size(); ++imgIndex)
        {
            try {
                otbAppLogINFO("Processing input " << imgIndex << std::endl);
                std::cerr << "Processing input " << imgIndex << std::endl;
                FloatVectorImageType::Pointer image = imageList->GetNthElement(imgIndex);

                if (imgIndex == 0)
                {
                    nbBands = image->GetNumberOfComponentsPerPixel();
                }

                std::cerr << "Image " << imgIndex << " vector length " << image->GetNumberOfComponentsPerPixel() << std::endl;


                //Sample list generator
                ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();
                listSampleGenerators->PushBack(sampleGenerator);

                sampleGenerator->SetInput(image);
                sampleGenerator->SetInputVectorData(vectorDataList->GetNthElement(imgIndex));

                sampleGenerator->SetClassKey(GetParameterString("sample.vfn"));

                sampleGenerator->SetMaxTrainingSize(sampleMt);
                sampleGenerator->SetMaxValidationSize(sampleMv);
                sampleGenerator->SetValidationTrainingProportion(GetParameterFloat("sample.vtr"));
                sampleGenerator->SetBoundByMin(GetParameterInt("sample.bm")!=0);
                sampleGenerator->SetClassesSize(classesSize);

                // take pixel located on polygon edge into consideration
                if (IsParameterEnabled("sample.edg"))
                {
                    sampleGenerator->SetPolygonEdgeInclusion(true);
                }

                sampleGenerator->Update();

                for (const auto &entry : sampleGenerator->GetClassesSamplesNumberTraining()) {
                    std::cerr << "Tile pixels of class " << entry.first << ": " << entry.second << '\n';
                    classPixels[entry.first] += entry.second;
                }

                std::cerr << "Training samples: " << sampleGenerator->GetTrainingListSample()->Size() << '\n';
                std::cerr << "Validation samples: " << sampleGenerator->GetValidationListSample()->Size() << '\n';

                //Concatenate training and validation samples from the image
                concatenateTrainingLabels->AddInput(sampleGenerator->GetTrainingListLabel());
                concatenateTrainingSamples->AddInput(sampleGenerator->GetTrainingListSample());
                concatenateValidationLabels->AddInput(sampleGenerator->GetValidationListLabel());
                concatenateValidationSamples->AddInput(sampleGenerator->GetValidationListSample());

                ok = true;
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                otbAppLogWARNING(<< e.what());
                errors += e.what();
                errors += '\n';
            } catch (...) {
                otbAppLogWARNING("Unknown error");
                errors += "Unknown error";
            }
        }

        if (!ok) {
            itkExceptionMacro("Unable to train classifier: " << errors);
        }
    } else if (this->HasValue("io.rs")) {
        std::vector<std::string> referenceRasters = GetParameterStringList("io.rs");

        typedef ImageFileReader<Int32ImageType> ImageReaderType;
        ImageReaderType::Pointer firstReader = ImageReaderType::New();
        firstReader->SetFileName(referenceRasters[0]);
        firstReader->UpdateOutputInformation();

        std::cerr << "Computing class counts" << std::endl;

        ListSampleGeneratorRasterType::ClassesSizeType classesSize;
        typedef otb::ObjectList<ListSampleGeneratorRasterType> ListSampleGeneratorRasterListType;
        ListSampleGeneratorRasterListType::Pointer listSampleGeneratorsRaster = ListSampleGeneratorRasterListType::New();

        for (unsigned int imgIndex = 0; imgIndex < imageList->Size(); ++imgIndex)
        {
            FloatVectorImageType::Pointer image = imageList->GetNthElement(imgIndex);
            image->UpdateOutputInformation();

            ImageReaderType::Pointer reader;
            Int32ImageType::Pointer raster;
            if (imgIndex == 0 || referenceRasters.size() == 1) {
                raster = firstReader->GetOutput();
            } else {
                reader = ImageReaderType::New();
                reader->SetFileName(referenceRasters[imgIndex]);
                reader->UpdateOutputInformation();
                raster = reader->GetOutput();
            }

            raster->SetRequestedRegionToLargestPossibleRegion();
            raster->PropagateRequestedRegion();
            raster->UpdateOutputData();

            //Sample list generator
            ListSampleGeneratorRasterType::Pointer sampleGenerator = ListSampleGeneratorRasterType::New();
            listSampleGeneratorsRaster->PushBack(sampleGenerator);

            sampleGenerator->SetInput(image);
            sampleGenerator->SetInputRaster(raster);

            sampleGenerator->SetNoDataLabel(GetParameterInt("nodatalabel"));
            sampleGenerator->SetMaxTrainingSize(sampleMt);
            sampleGenerator->SetMaxValidationSize(sampleMv);
            sampleGenerator->SetValidationTrainingProportion(GetParameterFloat("sample.vtr"));
            sampleGenerator->SetBoundByMin(GetParameterInt("sample.bm")!=0);

            sampleGenerator->GenerateClassStatistics();

            for (const auto &entry : sampleGenerator->GetClassesSize()) {
                classesSize[entry.first] += entry.second;
            }
        }

        for (const auto &entry : classesSize) {
            std::cerr << entry.first << ' ' << entry.second << '\n';
        }
        std::cerr << std::endl;

        //Iterate over all input images
        for (unsigned int imgIndex = 0; imgIndex < imageList->Size(); ++imgIndex)
        {
            otbAppLogINFO("Processing input " << imgIndex << std::endl);

            FloatVectorImageType::Pointer image = imageList->GetNthElement(imgIndex);
            image->UpdateOutputInformation();

            if (imgIndex == 0)
            {
                nbBands = image->GetNumberOfComponentsPerPixel();
            }

            ImageReaderType::Pointer reader;
            Int32ImageType::Pointer raster;
            if (imgIndex == 0 || referenceRasters.size() == 1) {
                raster = firstReader->GetOutput();
            } else {
                reader = ImageReaderType::New();
                reader->SetFileName(referenceRasters[imgIndex]);
                raster = reader->GetOutput();
            }

            //Sample list generator
            ListSampleGeneratorRasterType::Pointer sampleGenerator = ListSampleGeneratorRasterType::New();

            sampleGenerator->SetInput(image);
            sampleGenerator->SetInputRaster(raster);

            sampleGenerator->SetNoDataLabel(GetParameterInt("nodatalabel"));
            sampleGenerator->SetMaxTrainingSize(sampleMt);
            sampleGenerator->SetMaxValidationSize(sampleMv);
            sampleGenerator->SetValidationTrainingProportion(GetParameterFloat("sample.vtr"));
            sampleGenerator->SetBoundByMin(GetParameterInt("sample.bm")!=0);
            sampleGenerator->SetClassesSize(classesSize);
            sampleGenerator->Update();

            for (const auto &entry : sampleGenerator->GetClassesSamplesNumberTraining()) {
                std::cerr << "Tile pixels of class " << entry.first << ": " << entry.second << '\n';
                classPixels[entry.first] += entry.second;
            }

            std::cerr << "Training samples: " << sampleGenerator->GetTrainingListSample()->Size() << '\n';
            std::cerr << "Validation samples: " << sampleGenerator->GetValidationListSample()->Size() << '\n';

            //Concatenate training and validation samples from the image
            concatenateTrainingLabels->AddInput(sampleGenerator->GetTrainingListLabel());
            concatenateTrainingSamples->AddInput(sampleGenerator->GetTrainingListSample());
            concatenateValidationLabels->AddInput(sampleGenerator->GetValidationListLabel());
            concatenateValidationSamples->AddInput(sampleGenerator->GetValidationListSample());
        }
    } else {
        otbAppLogFATAL("No samples provided! ");
    }

    // Update
    concatenateTrainingSamples->Update();
    concatenateTrainingLabels->Update();
    concatenateValidationSamples->Update();
    concatenateValidationLabels->Update();

    for (const auto &entry : classPixels) {
        std::cerr << "Total pixels of class " << entry.first << ": " << entry.second << '\n';
    }

    std::cerr << "Total training samples: " << concatenateTrainingSamples->GetOutput()->Size() << '\n';
    std::cerr << "Total validation samples: " << concatenateValidationSamples->GetOutput()->Size() << '\n';

    if (concatenateTrainingSamples->GetOutput()->Size() == 0)
    {
        otbAppLogFATAL("No training samples, cannot perform SVM training.");
    }

    if (concatenateValidationSamples->GetOutput()->Size() == 0)
    {
        otbAppLogWARNING("No validation samples.");
    }

    if (IsParameterEnabled("io.imstat"))
    {
        StatisticsReader::Pointer statisticsReader = StatisticsReader::New();
        statisticsReader->SetFileName(GetParameterString("io.imstat"));
        meanMeasurementVector = statisticsReader->GetStatisticVectorByName("mean");
        stddevMeasurementVector = statisticsReader->GetStatisticVectorByName("stddev");
    }
    else
    {
        meanMeasurementVector.SetSize(nbBands);
        meanMeasurementVector.Fill(0.);
        stddevMeasurementVector.SetSize(nbBands);
        stddevMeasurementVector.Fill(1.);
    }

    // Shift scale the samples
    ShiftScaleFilterType::Pointer trainingShiftScaleFilter = ShiftScaleFilterType::New();
    trainingShiftScaleFilter->SetInput(concatenateTrainingSamples->GetOutput());
    trainingShiftScaleFilter->SetShifts(meanMeasurementVector);
    trainingShiftScaleFilter->SetScales(stddevMeasurementVector);
    trainingShiftScaleFilter->Update();

    ListSampleType::Pointer validationListSample=ListSampleType::New();

    //Test if the validation test is empty
    if ( concatenateValidationSamples->GetOutput()->Size() != 0 )
    {
        ShiftScaleFilterType::Pointer validationShiftScaleFilter = ShiftScaleFilterType::New();
        validationShiftScaleFilter->SetInput(concatenateValidationSamples->GetOutput());
        validationShiftScaleFilter->SetShifts(meanMeasurementVector);
        validationShiftScaleFilter->SetScales(stddevMeasurementVector);
        validationShiftScaleFilter->Update();
        validationListSample = validationShiftScaleFilter->GetOutput();
    }

    ListSampleType::Pointer listSample;
    LabelListSampleType::Pointer labelListSample;
    //--------------------------
    // Balancing training sample (if needed)
    // if (IsParameterEnabled("sample.b"))
    //   {
    //   // Balance the list sample.
    //   otbAppLogINFO("Number of training samples before balancing: " << concatenateTrainingSamples->GetOutput()->Size())
    //   BalancingListSampleFilterType::Pointer balancingFilter = BalancingListSampleFilterType::New();
    //   balancingFilter->SetInput(trainingShiftScaleFilter->GetOutput());
    //   balancingFilter->SetInputLabel(concatenateTrainingLabels->GetOutput());
    //   balancingFilter->SetBalancingFactor(GetParameterInt("sample.b"));
    //   balancingFilter->Update();
    //   listSample = balancingFilter->GetOutput();
    //   labelListSample = balancingFilter->GetOutputLabelSampleList();
    //   otbAppLogINFO("Number of samples after balancing: " << balancingFilter->GetOutput()->Size());

    //   }
    // else
    //   {
    listSample = trainingShiftScaleFilter->GetOutput();
    labelListSample = concatenateTrainingLabels->GetOutput();
    otbAppLogINFO("Number of training samples: " << concatenateTrainingSamples->GetOutput()->Size());
    //  }
    //--------------------------
    // Split the data set into training/validation set
    ListSampleType::Pointer trainingListSample = listSample;
    LabelListSampleType::Pointer trainingLabeledListSample = labelListSample;

    LabelListSampleType::Pointer validationLabeledListSample = concatenateValidationLabels->GetOutput();
    otbAppLogINFO("Size of training set: " << trainingListSample->Size());
    otbAppLogINFO("Size of validation set: " << validationListSample->Size());
    otbAppLogINFO("Size of labeled training set: " << trainingLabeledListSample->Size());
    otbAppLogINFO("Size of labeled validation set: " << validationLabeledListSample->Size());

    //--------------------------
    // Estimate model
    //--------------------------
    LabelListSampleType::Pointer predictedList = LabelListSampleType::New();
    const std::string classifierType = GetParameterString("classifier");

    if (classifierType == "libsvm")
    {
#ifdef OTB_USE_LIBSVM
        TrainLibSVM(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module LIBSVM is not installed. You should consider turning OTB_USE_LIBSVM on during cmake configuration.");
#endif
    }
    else if (classifierType == "svm")
    {
#ifdef OTB_USE_OPENCV
        TrainSVM(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "boost")
    {
#ifdef OTB_USE_OPENCV
        TrainBoost(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "dt")
    {
#ifdef OTB_USE_OPENCV
        TrainDecisionTree(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "gbt")
    {
#ifdef OTB_USE_OPENCV
        TrainGradientBoostedTree(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "ann")
    {
#ifdef OTB_USE_OPENCV
        TrainNeuralNetwork(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "bayes")
    {
#ifdef OTB_USE_OPENCV
        TrainNormalBayes(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "rf")
    {
#ifdef OTB_USE_OPENCV
        TrainRandomForests(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }
    else if (classifierType == "knn")
    {
#ifdef OTB_USE_OPENCV
        TrainKNN(trainingListSample, trainingLabeledListSample);
#else
        otbAppLogFATAL("Module OPENCV is not installed. You should consider turning OTB_USE_OPENCV on during cmake configuration.");
#endif
    }


    //--------------------------
    // Performances estimation
    //--------------------------
    ListSampleType::Pointer performanceListSample=ListSampleType::New();
    LabelListSampleType::Pointer performanceLabeledListSample=LabelListSampleType::New();

    //Test the input validation set size
    if(validationLabeledListSample->Size() != 0)
    {
        performanceListSample = validationListSample;
        performanceLabeledListSample = validationLabeledListSample;
    }
    else
    {
        otbAppLogWARNING("The validation set is empty. The performance estimation is done using the input training set in this case.");
        performanceListSample = trainingListSample;
        performanceLabeledListSample = trainingLabeledListSample;
    }

    Classify(performanceListSample, predictedList);

    ConfusionMatrixCalculatorType::Pointer confMatCalc = ConfusionMatrixCalculatorType::New();

    std::cout << "predicted list size == " << predictedList->Size() << std::endl;
    std::cout << "validationLabeledListSample size == " << performanceLabeledListSample->Size() << std::endl;
    confMatCalc->SetReferenceLabels(performanceLabeledListSample);
    confMatCalc->SetProducedLabels(predictedList);

    confMatCalc->Compute();

    otbAppLogINFO("training performances");
    LogConfusionMatrix(confMatCalc);

    for (unsigned int itClasses = 0; itClasses < confMatCalc->GetNumberOfClasses(); itClasses++)
    {
        ConfusionMatrixCalculatorType::ClassLabelType classLabel = confMatCalc->GetMapOfIndices()[itClasses];

        otbAppLogINFO("Precision of class [" << classLabel << "] vs all: " << confMatCalc->GetPrecisions()[itClasses]);
        otbAppLogINFO("Recall of class    [" << classLabel << "] vs all: " << confMatCalc->GetRecalls()[itClasses]);
        otbAppLogINFO(
                    "F-score of class   [" << classLabel << "] vs all: " << confMatCalc->GetFScores()[itClasses] << "\n");
    }
    otbAppLogINFO("Global performance, Kappa index: " << confMatCalc->GetKappaIndex());


    if (this->HasValue("io.confmatout"))
    {
        // Writing the confusion matrix in the output .CSV file

        MapOfIndicesType::iterator itMapOfIndicesValid, itMapOfIndicesPred;
        ClassLabelType labelValid = 0;

        ConfusionMatrixType confusionMatrix = confMatCalc->GetConfusionMatrix();
        MapOfIndicesType mapOfIndicesValid = confMatCalc->GetMapOfIndices();

        unsigned int nbClassesPred = mapOfIndicesValid.size();

        /////////////////////////////////////////////
        // Filling the 2 headers for the output file
        const std::string commentValidStr = "#Reference labels (rows):";
        const std::string commentPredStr = "#Produced labels (columns):";
        const char separatorChar = ',';
        std::ostringstream ossHeaderValidLabels, ossHeaderPredLabels;

        // Filling ossHeaderValidLabels and ossHeaderPredLabels for the output file
        ossHeaderValidLabels << commentValidStr;
        ossHeaderPredLabels << commentPredStr;

        itMapOfIndicesValid = mapOfIndicesValid.begin();

        while (itMapOfIndicesValid != mapOfIndicesValid.end())
        {
            // labels labelValid of mapOfIndicesValid are already sorted in otbConfusionMatrixCalculator
            labelValid = itMapOfIndicesValid->second;

            otbAppLogINFO("mapOfIndicesValid[" << itMapOfIndicesValid->first << "] = " << labelValid);

            ossHeaderValidLabels << labelValid;
            ossHeaderPredLabels << labelValid;

            ++itMapOfIndicesValid;

            if (itMapOfIndicesValid != mapOfIndicesValid.end())
            {
                ossHeaderValidLabels << separatorChar;
                ossHeaderPredLabels << separatorChar;
            }
            else
            {
                ossHeaderValidLabels << std::endl;
                ossHeaderPredLabels << std::endl;
            }
        }

        std::ofstream outFile;
        outFile.open(this->GetParameterString("io.confmatout").c_str());
        outFile << std::fixed;
        outFile.precision(10);

        /////////////////////////////////////
        // Writing the 2 headers
        outFile << ossHeaderValidLabels.str();
        outFile << ossHeaderPredLabels.str();
        /////////////////////////////////////

        unsigned int indexLabelValid = 0, indexLabelPred = 0;

        for (itMapOfIndicesValid = mapOfIndicesValid.begin(); itMapOfIndicesValid != mapOfIndicesValid.end(); ++itMapOfIndicesValid)
        {
            indexLabelPred = 0;

            for (itMapOfIndicesPred = mapOfIndicesValid.begin(); itMapOfIndicesPred != mapOfIndicesValid.end(); ++itMapOfIndicesPred)
            {
                // Writing the confusion matrix (sorted in otbConfusionMatrixCalculator) in the output file
                outFile << confusionMatrix(indexLabelValid, indexLabelPred);
                if (indexLabelPred < (nbClassesPred - 1))
                {
                    outFile << separatorChar;
                }
                else
                {
                    outFile << std::endl;
                }
                ++indexLabelPred;
            }

            ++indexLabelValid;
        }

        outFile.close();
    } // END if (this->HasValue("io.confmatout"))

    // TODO: implement hyperplane distance classifier and performance validation (cf. object detection) ?

}


}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::TrainImagesClassifier)
