/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.

  Some parts of this code are derived from ITK. See ITKCopyright.txt
  for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANT2ABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef otbStreamingStatisticsVectorImageFilter_h
#define otbStreamingStatisticsVectorImageFilter_h

#include "otbPersistentImageFilter.h"
#include "otbPersistentFilterStreamingDecorator.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkImageRegionSplitter.h"
#include "itkVariableSizeMatrix.h"
#include "itkVariableLengthVector.h"

namespace otb
{

/** \class PersistentStreamingStatisticsVectorImageFilterEx
 * \brief Compute covariance & correlation of a large image using streaming
 *
 *  This filter persists its temporary data. It means that if you Update it n times on n different
 * requested regions, the output statistics will be the statitics of the whole set of n regions.
 *
 * To reset the temporary data, one should call the Reset() function.
 *
 * To get the statistics once the regions have been processed via the pipeline, use the Synthetize() method.
 *
 * \sa PersistentImageFilter
 * \ingroup Streamed
 * \ingroup Multithreaded
 * \ingroup MathematicalStatisticsImageFilters
 *
 *
 * \ingroup OTBStatistics
 */
template<class TInputImage, class TPrecision >
class ITK_EXPORT PersistentStreamingStatisticsVectorImageFilterEx :
  public PersistentImageFilter<TInputImage, TInputImage>
{
public:
  /** Standard Self typedef */
  typedef PersistentStreamingStatisticsVectorImageFilterEx           Self;
  typedef PersistentImageFilter<TInputImage, TInputImage> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(PersistentStreamingStatisticsVectorImageFilterEx, PersistentImageFilter);

  /** Image related typedefs. */
  typedef TInputImage                           ImageType;
  typedef typename ImageType::Pointer           InputImagePointer;
  typedef typename ImageType::RegionType        RegionType;
  typedef typename ImageType::SizeType          SizeType;
  typedef typename ImageType::IndexType         IndexType;
  typedef typename ImageType::PixelType         PixelType;
  typedef typename ImageType::InternalPixelType InternalPixelType;

  typedef TPrecision                            PrecisionType;
  typedef PrecisionType                         RealType;

  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Smart Pointer type to a DataObject. */
  typedef typename itk::DataObject::Pointer DataObjectPointer;
  typedef itk::ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;

  /** Type to use for computations. */
  typedef itk::VariableSizeMatrix<PrecisionType>        MatrixType;
  typedef itk::VariableLengthVector<PrecisionType>      RealPixelType;

  /** Type of DataObjects used for outputs */
  typedef itk::SimpleDataObjectDecorator<RealType>      RealObjectType;
  typedef itk::SimpleDataObjectDecorator<IndexType>     IndexObjectType;
  typedef itk::SimpleDataObjectDecorator<PixelType>     PixelObjectType;
  typedef itk::SimpleDataObjectDecorator<RealPixelType> RealPixelObjectType;
  typedef itk::SimpleDataObjectDecorator<MatrixType>    MatrixObjectType;

  /** Return the computed Min */
  PixelType GetMinimum() const
  {
    return this->GetMinOutput()->Get();
  }
  PixelObjectType* GetMinimumOutput();
  const PixelObjectType* GetMinimumOutput() const;

  /** Return the computed Max index */
  PixelType GetMaximum() const
  {
    return this->GetMaxOutput()->Get();
  }
  PixelObjectType* GetMaximumOutput();
  const PixelObjectType* GetMaximumOutput() const;

  /** Return the global mean of all the internal pixel values
   * (flattening the multispectral image as a 1D-vector) */
  RealType GetComponentMean() const
  {
    return this->GetComponentMeanOutput()->Get();
  }
  RealObjectType* GetComponentMeanOutput();
  const RealObjectType* GetComponentMeanOutput() const;

  /** Return the global correlation of all the internal pixel values
   * (flattening the multispectral image as a 1D-vector) */
  RealType GetComponentCorrelation() const
  {
    return this->GetComponentCorrelationOutput()->Get();
  }
  RealObjectType* GetComponentCorrelationOutput();
  const RealObjectType* GetComponentCorrelationOutput() const;

  /** Return the global covariance of all the internal pixel values
   * (flattening the multispectral image as a 1D-vector) */
  RealType GetComponentCovariance() const
  {
    return this->GetComponentCovarianceOutput()->Get();
  }
  RealObjectType* GetComponentCovarianceOutput();
  const RealObjectType* GetComponentCovarianceOutput() const;

  /** Return the computed Mean. */
  RealPixelType GetMean() const
  {
    return this->GetMeanOutput()->Get();
  }
  RealPixelObjectType* GetMeanOutput();
  const RealPixelObjectType* GetMeanOutput() const;

  /** Return the computed Sum. */
  RealPixelType GetSum() const
  {
    return this->GetSumOutput()->Get();
  }
  RealPixelObjectType* GetSumOutput();
  const RealPixelObjectType* GetSumOutput() const;

  /** Return the computed Covariance. */
  MatrixType GetCorrelation() const
  {
    return this->GetCorrelation()->Get();
  }
  MatrixObjectType* GetCorrelationOutput();
  const MatrixObjectType* GetCorrelationOutput() const;

  /** Return the computed Covariance. */
  MatrixType GetCovariance() const
  {
    return this->GetCovarianceOutput()->Get();
  }
  MatrixObjectType* GetCovarianceOutput();
  const MatrixObjectType* GetCovarianceOutput() const;

  /** Make a DataObject of the correct type to be used as the specified
   * output.
   */
  DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx) ITK_OVERRIDE;
  using Superclass::MakeOutput;

  void Reset(void) ITK_OVERRIDE;

  void Synthetize(void) ITK_OVERRIDE;

  itkSetMacro(EnableMinMax, bool);
  itkGetMacro(EnableMinMax, bool);

  itkSetMacro(EnableFirstOrderStats, bool);
  itkGetMacro(EnableFirstOrderStats, bool);

  itkSetMacro(EnableSecondOrderStats, bool);
  itkGetMacro(EnableSecondOrderStats, bool);

  itkSetMacro(IgnoreInfiniteValues, bool);
  itkGetMacro(IgnoreInfiniteValues, bool);

  itkSetMacro(IgnoreUserDefinedValue, bool);
  itkGetMacro(IgnoreUserDefinedValue, bool);

  itkSetMacro(UserIgnoredValue, InternalPixelType);
  itkGetMacro(UserIgnoredValue, InternalPixelType);

  itkSetMacro(UseUnbiasedEstimator, bool);
  itkGetMacro(UseUnbiasedEstimator, bool);

protected:
  PersistentStreamingStatisticsVectorImageFilterEx();

  ~PersistentStreamingStatisticsVectorImageFilterEx() ITK_OVERRIDE {}

  /** Pass the input through unmodified. Do this by Grafting in the
   *  AllocateOutputs method.
   */
  void AllocateOutputs() ITK_OVERRIDE;

  void GenerateOutputInformation() ITK_OVERRIDE;

  void PrintSelf(std::ostream& os, itk::Indent indent) const ITK_OVERRIDE;

  /** Multi-thread version GenerateData. */
  void  ThreadedGenerateData(const RegionType& outputRegionForThread, itk::ThreadIdType threadId) ITK_OVERRIDE;

private:
  PersistentStreamingStatisticsVectorImageFilterEx(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  bool m_EnableMinMax;
  bool m_EnableFirstOrderStats;
  bool m_EnableSecondOrderStats;


  /* use an unbiased estimator to compute the covariance */
  bool m_UseUnbiasedEstimator;

  std::vector<PixelType>     m_ThreadMin;
  std::vector<PixelType>     m_ThreadMax;
  std::vector<RealType>      m_ThreadFirstOrderComponentAccumulators;
  std::vector<RealType>      m_ThreadSecondOrderComponentAccumulators;
  std::vector<RealPixelType> m_ThreadFirstOrderAccumulators;
  std::vector<MatrixType>    m_ThreadSecondOrderAccumulators;

  /* Ignored values */
  bool m_IgnoreInfiniteValues;
  bool m_IgnoreUserDefinedValue;
  InternalPixelType m_UserIgnoredValue;
  std::vector<itk::VariableLengthVector<unsigned int>>  m_IgnoredInfinitePixelCount;
  std::vector<itk::VariableLengthVector<unsigned int>>  m_IgnoredUserPixelCount;

}; // end of class PersistentStreamingStatisticsVectorImageFilterEx

/**===========================================================================*/

/** \class StreamingStatisticsVectorImageFilter
 * \brief This class streams the whole input image through the PersistentStatisticsImageFilter.
 *
 * This way, it allows computing the first and second order global statistics of this image. It calls the
 * Reset() method of the PersistentStreamingStatisticsVectorImageFilterEx before streaming the image and the
 * Synthetize() method of the PersistentStreamingStatisticsVectorImageFilterEx after having streamed the image
 * to compute the statistics. The accessor on the results are wrapping the accessors of the
 * internal PersistentStreamingStatisticsVectorImageFilterEx.
 * By default infinite values are ignored, use IgnoreInfiniteValues accessor to consider
 * infinite values in the computation.
 *
 * \sa PersistentStreamingStatisticsVectorImageFilterEx
 * \sa PersistentImageFilter
 * \sa PersistentFilterStreamingDecorator
 * \sa StreamingImageVirtualWriter
 * \ingroup Streamed
 * \ingroup Multithreaded
 * \ingroup MathematicalStatisticsImageFilters
 *
 * \ingroup OTBStatistics
 */

template<class TInputImage, class TPrecision = typename itk::NumericTraits<typename TInputImage::InternalPixelType>::RealType>
class ITK_EXPORT StreamingStatisticsVectorImageFilterEx :
  public PersistentFilterStreamingDecorator<PersistentStreamingStatisticsVectorImageFilterEx<TInputImage, TPrecision> >
{
public:
  /** Standard Self typedef */
  typedef StreamingStatisticsVectorImageFilterEx Self;
  typedef PersistentFilterStreamingDecorator
  <PersistentStreamingStatisticsVectorImageFilterEx<TInputImage, TPrecision> > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(StreamingStatisticsVectorImageFilterEx, PersistentFilterStreamingDecorator);

  typedef TInputImage                                 InputImageType;
  typedef typename Superclass::FilterType             StatFilterType;

  /** Type of DataObjects used for outputs */
  typedef typename StatFilterType::PixelType           PixelType;
  typedef typename StatFilterType::RealType            RealType;
  typedef typename StatFilterType::RealObjectType      RealObjectType;
  typedef typename StatFilterType::RealPixelType       RealPixelType;
  typedef typename StatFilterType::RealPixelObjectType RealPixelObjectType;
  typedef typename StatFilterType::MatrixType          MatrixType;
  typedef typename StatFilterType::MatrixObjectType    MatrixObjectType;

  typedef typename StatFilterType::InternalPixelType   InternalPixelType;

  using Superclass::SetInput;
  void SetInput(InputImageType * input)
  {
    this->GetFilter()->SetInput(input);
  }
  const InputImageType * GetInput()
  {
    return this->GetFilter()->GetInput();
  }

  /** Return the computed Minimum. */
  RealPixelType GetMinimum() const
  {
    return this->GetFilter()->GetMinimumOutput()->Get();
  }
  RealPixelObjectType* GetMinimumOutput()
  {
    return this->GetFilter()->GetMinimumOutput();
  }
  const RealPixelObjectType* GetMinimumOutput() const
  {
    return this->GetFilter()->GetMinimumOutput();
  }

  /** Return the computed Maximum. */
  RealPixelType GetMaximum() const
  {
    return this->GetFilter()->GetMaximumOutput()->Get();
  }
  RealPixelObjectType* GetMaximumOutput()
  {
    return this->GetFilter()->GetMaximumOutput();
  }
  const RealPixelObjectType* GetMaximumOutput() const
  {
    return this->GetFilter()->GetMaximumOutput();
  }

  /** Return the computed Mean. */
  RealPixelType GetMean() const
  {
    return this->GetFilter()->GetMeanOutput()->Get();
  }
  RealPixelObjectType* GetMeanOutput()
  {
    return this->GetFilter()->GetMeanOutput();
  }
  const RealPixelObjectType* GetMeanOutput() const
  {
    return this->GetFilter()->GetMeanOutput();
  }

  /** Return the computed Sum. */
  RealPixelType GetSum() const
  {
    return this->GetFilter()->GetSumOutput()->Get();
  }
  RealPixelObjectType* GetSumOutput()
  {
    return this->GetFilter()->GetSumOutput();
  }
  const RealPixelObjectType* GetSumOutput() const
  {
    return this->GetFilter()->GetSumOutput();
  }

  /** Return the computed Covariance. */
  MatrixType GetCovariance() const
  {
    return this->GetFilter()->GetCovarianceOutput()->Get();
  }
  MatrixObjectType* GetCovarianceOutput()
  {
    return this->GetFilter()->GetCovarianceOutput();
  }
  const MatrixObjectType* GetCovarianceOutput() const
  {
    return this->GetFilter()->GetCovarianceOutput();
  }

  /** Return the computed Covariance. */
  MatrixType GetCorrelation() const
  {
    return this->GetFilter()->GetCorrelationOutput()->Get();
  }
  MatrixObjectType* GetCorrelationOutput()
  {
    return this->GetFilter()->GetCorrelationOutput();
  }
  const MatrixObjectType* GetCorrelationOutput() const
  {
    return this->GetFilter()->GetCorrelationOutput();
  }

  /** Return the computed Mean. */
  RealType GetComponentMean() const
  {
    return this->GetFilter()->GetComponentMeanOutput()->Get();
  }
  RealObjectType* GetComponentMeanOutput()
  {
    return this->GetFilter()->GetComponentMeanOutput();
  }
  const RealObjectType* GetComponentMeanOutput() const
  {
    return this->GetFilter()->GetComponentMeanOutput();
  }

  /** Return the computed Covariance. */
  RealType GetComponentCovariance() const
  {
    return this->GetFilter()->GetComponentCovarianceOutput()->Get();
  }
  RealObjectType* GetComponentCovarianceOutput()
  {
    return this->GetFilter()->GetComponentCovarianceOutput();
  }
  const RealObjectType* GetComponentCovarianceOutput() const
  {
    return this->GetFilter()->GetComponentCovarianceOutput();
  }

  /** Return the computed Covariance. */
  RealType GetComponentCorrelation() const
  {
    return this->GetFilter()->GetComponentCorrelationOutput()->Get();
  }
  RealObjectType* GetComponentCorrelationOutput()
  {
    return this->GetFilter()->GetComponentCorrelationOutput();
  }
  const RealObjectType* GetComponentCorrelationOutput() const
  {
    return this->GetFilter()->GetComponentCorrelationOutput();
  }

  otbSetObjectMemberMacro(Filter, EnableMinMax, bool);
  otbGetObjectMemberMacro(Filter, EnableMinMax, bool);

  otbSetObjectMemberMacro(Filter, EnableFirstOrderStats, bool);
  otbGetObjectMemberMacro(Filter, EnableFirstOrderStats, bool);

  otbSetObjectMemberMacro(Filter, EnableSecondOrderStats, bool);
  otbGetObjectMemberMacro(Filter, EnableSecondOrderStats, bool);

  otbSetObjectMemberMacro(Filter, IgnoreInfiniteValues, bool);
  otbGetObjectMemberMacro(Filter, IgnoreInfiniteValues, bool);

  otbSetObjectMemberMacro(Filter, IgnoreUserDefinedValue, bool);
  otbGetObjectMemberMacro(Filter, IgnoreUserDefinedValue, bool);

  otbSetObjectMemberMacro(Filter, UserIgnoredValue, InternalPixelType);
  otbGetObjectMemberMacro(Filter, UserIgnoredValue, InternalPixelType);

  otbSetObjectMemberMacro(Filter, UseUnbiasedEstimator, bool);
  otbGetObjectMemberMacro(Filter, UseUnbiasedEstimator, bool);

protected:
  /** Constructor */
  StreamingStatisticsVectorImageFilterEx() {}

  /** Destructor */
  ~StreamingStatisticsVectorImageFilterEx() ITK_OVERRIDE {}

private:
  StreamingStatisticsVectorImageFilterEx(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};

} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbStreamingStatisticsVectorImageFilterEx.txx"
#endif

#endif
