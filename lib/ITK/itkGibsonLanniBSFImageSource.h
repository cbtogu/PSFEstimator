/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGibsonLanniBSFImageSource.h,v $
  Language:  C++
  Date:      $Date: 2010/05/24 19:01:22 $
  Version:   $Revision: 1.11 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkGibsonLanniBSFImageSource_h
#define __itkGibsonLanniBSFImageSource_h

#include "itkGibsonLanniPSFImageSource.h"
#include "itkNumericTraits.h"
#include "itkParametricImageSource.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkRotationalExtrusionTransform.h"
#include "itkShiftScaleImageFilter.h"
#include "itkSphereConvolutionFilter.h"


#define DelegateSetMacro(name, type) \
virtual void Set##name(type data) \
{ \
  if (data != m_PSFSource->Get##name()) { \
    m_PSFSource->Set##name(data); \
    this->Modified(); \
  } \
}

#define DelegateGetMacro(name, type) \
virtual type Get##name() const \
{ \
  return m_PSFSource->Get##name(); \
}

#define DelegateSetGetMacro(name, type) \
  DelegateSetMacro(name, type) \
  DelegateGetMacro(name, type)


namespace itk
{

/** \class GibsonLanniBSFImageSource
 * \brief Generates a synthetic point-spread function according to the
 * Gibson-Lanni model and convolves it with a spherical bead shape.
 *
 * The Gibson-Lanni point-spread function model takes into account optical
 * path differences caused by imaging conditions that differ from the design
 * conditions of an objective in a widefield fluorescence microscope. This
 * image source generates images according to this model. IMPORTANT: Please
 * pay attention to the units each method expects. Some take nanometers, some
 * take micrometers, and some take millimeters.
 *
 * Typically, measuring the point-spread function of an image is done by
 * taking an image stack of a sub-diffraction limit bead (100-150nm). This
 * results in a point-spread function that is convolved with the spherical
 * bead shape. This class calculates the theoretical bead-spread function.
 * It is essentially a wrapper around the GibsonLanniPSFImageSource and
 * SphereConvolutionFilter.
 *
 * \ingroup DataSources Multithreaded
 * \sa GibsonLanniPSFImageSource.
 */
template <class TOutputImage>
class ITK_EXPORT GibsonLanniBSFImageSource :
  public ParametricImageSource<TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef GibsonLanniBSFImageSource           Self;
  typedef ParametricImageSource<TOutputImage> Superclass;
  typedef SmartPointer<Self>                  Pointer;
  typedef SmartPointer<const Self>            ConstPointer;

  /** Typedef for output types. */
  typedef TOutputImage                             OutputImageType;
  typedef typename OutputImageType::PixelType      PixelType;
  typedef typename OutputImageType::RegionType     RegionType;
  typedef typename OutputImageType::PointType      PointType;
  typedef typename OutputImageType::PointValueType PointValueType;
  typedef typename PointType::VectorType           VectorType;
  typedef typename OutputImageType::SpacingType    SpacingType;
  typedef typename OutputImageType::IndexType      IndexType;
  typedef typename OutputImageType::SizeType       SizeType;
  typedef typename OutputImageType::SizeValueType  SizeValueType;


  itkStaticConstMacro(ImageDimension, unsigned int,
		      TOutputImage::ImageDimension);

  typedef GibsonLanniPSFImageSource<TOutputImage>
    PSFSourceType;
  typedef typename PSFSourceType::Pointer
    PSFSourcePointer;
  typedef ResampleImageFilter< typename PSFSourceType::OutputImageType, TOutputImage, double >
    ExtrusionFilterType;
  typedef typename ExtrusionFilterType::Pointer
    ExtrusionFilterPointer;
  typedef SphereConvolutionFilter<TOutputImage,TOutputImage>
    ConvolverType;
  typedef typename ConvolverType::Pointer
    ConvolverPointer;
  typedef ShiftScaleImageFilter<TOutputImage, TOutputImage>
    RescaleImageFilterType;
  typedef typename RescaleImageFilterType::Pointer
    RescaleImageFilterPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(GibsonLanniBSFImageSource,ParametricImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef typename Superclass::ParametersValueType ParametersValueType;
  typedef typename Superclass::ParametersType      ParametersType;

  /** Specify the size of the output image. */
  void SetSize(const SizeType & size)
  {
    if (size != m_Size)
      {
      m_Size = size;
      this->Modified();
      }
    m_Convolver->SetSize(size);
  }

  /** Get the size of the output image. */
  itkGetConstReferenceMacro(Size, SizeType);

  /** Specify the spacing of the output image (in nanometers). */
  void SetSpacing(const SpacingType & spacing)
  {
    if (spacing != m_Spacing)
      {
      m_Spacing = spacing;
      this->Modified();
      }
    m_Convolver->SetSpacing(spacing);
  }

  /** Get the spacing of the output image (in nanometers). */
  itkGetConstReferenceMacro(Spacing, SpacingType);

  /** Specify the origin of the output image (in nanometers). */
  virtual void SetOrigin(const PointType & origin)
  {
    if (origin != m_Origin)
      {
      m_Origin = origin;
      this->Modified();
      }
    m_Convolver->SetOrigin(origin);
  }

  /** Get the origin of the output image (in nanometers). */
  itkGetConstReferenceMacro(Origin, PointType);

  /** Specify the point source center (in nanometers). */
  virtual void SetBeadCenter(const PointType & center)
  {
    if (center != m_Convolver->GetSphereCenter())
      {
      m_Convolver->SetSphereCenter(center);
      this->Modified();
      }
  }

  /** Get the point source center (in nanometers). */
  virtual const PointType & GetBeadCenter() const
  {
    return m_Convolver->GetSphereCenter();
  }

  /** Specify the bead radius (in nanometers). */
  void SetBeadRadius(double radius)
  {
    if (radius != m_Convolver->GetSphereRadius())
      {
      m_Convolver->SetSphereRadius(radius);
      this->Modified();
      }
  }

  /** Get the bead radius. */
  double GetBeadRadius() const
  {
    return m_Convolver->GetSphereRadius();
  }

  /** Specify the shear in the X direction. */
  void SetShearX(double shear)
  {
    if (shear != m_Convolver->GetShearX())
      {
      m_Convolver->SetShearX(shear);
      this->Modified();
      }
  }

  /** Get the shear in the X direction. */
  double GetShearX() const
  {
    return m_Convolver->GetShearX();
  }

  /** Specify the shear in the Y direction. */
  void SetShearY(double shear)
  {
    if (shear != m_Convolver->GetShearY())
      {
      m_Convolver->SetShearY(shear);
      this->Modified();
      }
  }

  /** Get the shear in the Y direction. */
  double GetShearY() const
  {
    return m_Convolver->GetShearY();
  }

  /** Set/get the background value. */
  itkSetMacro(IntensityShift, double);
  itkGetConstMacro(IntensityShift, double);

  /** Set/get the maximum intensity. */
  itkSetMacro(IntensityScale, double);
  itkGetConstMacro(IntensityScale, double);

  /** Set/get the emission wavelength (in nanometers). */
  DelegateSetGetMacro(EmissionWavelength, double);

  /** Set/get the numerical aperture (unitless). */
  DelegateSetGetMacro(NumericalAperture, double);

  /** Set/get the magnification (unitless). */
  DelegateSetGetMacro(Magnification, double);

  /** Set/get the design cover slip refractive index (unitless). */
  DelegateSetGetMacro(DesignCoverSlipRefractiveIndex, double);

  /** Set/get the actual cover slip refractive index (unitless). */
  DelegateSetGetMacro(ActualCoverSlipRefractiveIndex, double);

  /** Set/get the design cover slip thickness (in micrometers). */
  DelegateSetGetMacro(DesignCoverSlipThickness, double);

  /** Set/get the actual cover slip thickness (in micrometers). */
  DelegateSetGetMacro(ActualCoverSlipThickness, double);

  /** Set/get the design immersion oil refractive index (unitless). */
  DelegateSetGetMacro(DesignImmersionOilRefractiveIndex, double);

  /** Set/get the actual immersion oil refractive index (unitless). */
  DelegateSetGetMacro(ActualImmersionOilRefractiveIndex, double);

  /** Set/get the design immersion oil thickness (in micrometers). */
  DelegateSetGetMacro(DesignImmersionOilThickness, double);

  /** Set/get the design specimen layer refractive index (unitless). */
  DelegateSetGetMacro(DesignSpecimenLayerRefractiveIndex, double);

  /** Set/get the actual specimen layer refractive index (unitless). */
  DelegateSetGetMacro(ActualSpecimenLayerRefractiveIndex, double);

  /** Set/get the actual point source depth in the specimen layer (in nanometers). */
  DelegateSetGetMacro(ActualPointSourceDepthInSpecimenLayer, double);

  /** Set/get the design distance from the back focal plane to the detector (in millimeters). */
  DelegateSetGetMacro(DesignDistanceFromBackFocalPlaneToDetector, double);

  /** Set/get the actual distance from the back focal plane to the detector (in millimeters). */
  DelegateSetGetMacro(ActualDistanceFromBackFocalPlaneToDetector, double);

  /** Expects the parameters argument to contain values for ALL parameters. */
  virtual void SetParameters(const ParametersType& parameters);

  /** Gets the full parameters list. */
  virtual ParametersType GetParameters() const;

  /** Gets the total number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;

  /** Get/set the z-coordinate of the image z-plane at the given index. */
  void SetZCoordinate(unsigned int index, double coordinate);
  double GetZCoordinate(unsigned int);

  /** Get/set use of custom z coordinates. */
  void SetUseCustomZCoordinates(bool use);
  bool GetUseCustomZCoordinates();

protected:
  GibsonLanniBSFImageSource();
  ~GibsonLanniBSFImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** This class is implicitly multi-threaded because its member filters
   * are mulithreaded, so we go with a "single-threaded"
   * implementation here. */
  virtual void GenerateData();

  virtual void GenerateOutputInformation();


private:
  GibsonLanniBSFImageSource(const GibsonLanniBSFImageSource&); //purposely not implemented
  void operator=(const GibsonLanniBSFImageSource&); //purposely not implemented

  SizeType    m_Size;           // Size of the output image
  SpacingType m_Spacing;        // Spacing of the output image
  PointType   m_Origin;         // Origin of the output image
  double      m_IntensityShift; // Additive background constant
  double      m_IntensityScale; // The maximum intensity value

  PSFSourcePointer          m_PSFSource;
  ExtrusionFilterPointer    m_ExtrusionFilter;
  ConvolverPointer          m_Convolver;
  RescaleImageFilterPointer m_RescaleFilter;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGibsonLanniBSFImageSource.txx"
#endif

#endif
