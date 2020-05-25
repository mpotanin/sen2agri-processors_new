otb_create_application(
  NAME           CropMaskTrainImagesClassifier
  SOURCES        otbTrainImagesClassifier.cxx otbTrainImagesClassifier.h
                 ../Filters/CropMaskPreprocessing.h
                 ../Filters/CropMaskNDVIPreprocessing.h
                 ../Filters/CropMaskFeaturesSupervised.hxx
                 ../Filters/ComputeNDVIFilter.h
                 ../Filters/CropMaskSupervisedRedEdgeFeaturesFilter.h
                 ../Filters/TemporalMerging.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES}
                 MACCSMetadata
                 TimeSeriesReader)
include_directories(../../Common/OTBExtensions)
#[[
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_CropMaskTrainImagesClassifier DESTINATION usr/lib/otb/applications/)
]]