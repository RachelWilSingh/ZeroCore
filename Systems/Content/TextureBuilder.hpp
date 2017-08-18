///////////////////////////////////////////////////////////////////////////////
///
/// \file TextureBuilder.hpp
/// 
/// Authors: Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

const String ZTexLoader = "TextureZTex";

const uint TextureFileId = 'ztex';
const uint TextureFileVersion = 1;

class TextureHeader
{
public:
  uint mFileId;
  uint mFileVersion;
  uint mType;
  uint mFormat;
  uint mMipCount;
  uint mTotalDataSize;
  uint mCompression;
  uint mAddressingX;
  uint mAddressingY;
  uint mFiltering;
  uint mAnisotropy;
  uint mMipMapping;
};

class MipHeader
{
public:
  uint mFace;
  uint mLevel;
  uint mWidth;
  uint mHeight;
  uint mDataOffset;
  uint mDataSize;
};

//-----------------------------------------------------------Texture Builder Enums
DeclareEnum25(TextureFormat, None,
  R8,   RG8,   RGB8,   RGBA8,              // byte
  R16,  RG16,  RGB16,  RGBA16,             // short
  R16f, RG16f, RGB16f, RGBA16f,            // half float
  R32f, RG32f, RGB32f, RGBA32f,            // float
  SRGB8, SRGB8A8,                          // gamma
  Depth16, Depth24, Depth32, Depth32f,     // depth
  Depth24Stencil8, Depth32fStencil8Pad24); // depth-stencil

// Face identifiers for TextureCube, None is used for Texture2D
DeclareEnum7(TextureFace, None, PositiveX, PositiveY, PositiveZ, NegativeX, NegativeY, NegativeZ);

/// Type of the texture, must match sampler type in shaders
/// Texture2D - Standard 2 dimensional texture
/// TextureCube - Uses texture as a cubemap
///   Faces are extracted from the image using aspect ratio to determine layout
DeclareEnum2(TextureType, Texture2D, TextureCube);

/// Block compression, lossy hardware supported formats with very high memory savings
/// None - No compression will be used
/// BC1 - RGB stored at 1/2 byte per pixel
///   Used for color maps that don't need alpha, normal maps
/// BC2 - RGB w/ low precision alpha stored at 1 byte per pixel
///   No common usages
/// BC3 - RGB w/ alpha stored at 1 byte per pixel
///   Used for color maps that need alpha
/// BC4 - R stored at 1/2 byte per pixel
///   Used for single channel maps like height, specular, roughness
/// BC5 - RG stored at 1 byte per pixel
///   Used for two channel maps like normals with reconstructed Z
/// BC6 - RGB floats stored at 1 byte per pixel
///   Used for high dynamic range images
DeclareEnum7(TextureCompression, None, BC1, BC2, BC3, BC4, BC5, BC6);

/// How to address the texture with uv's outside of the range [0, 1]
/// Clamp - Uses the last pixel at the border of the image
/// Repeat - Wraps to the opposite side and continues to sample the image
/// Mirror - Similar to Repeat but reverses image direction
DeclareEnum3(TextureAddressing, Clamp, Repeat, Mirror);

/// How pixels are sampled when viewing image at a different size
/// Nearest - Gets the closest pixel unaltered
/// Bilinear - Gets the 4 closest pixels and linearly blends between them
/// Trilinear - Same as bilinear with an additional linear blend between mip levels
DeclareEnum3(TextureFiltering, Nearest, Bilinear, Trilinear);

/// How pixels are sampled when the ratio of pixels viewed along its u/v directions is not 1:1
/// (Typically when viewing a texture at an angle)
/// The options represent how large of a ratio will be accounted for when sampling
/// x1 = 1:1 (no anisotropy), x16 = 16:1 (high anisotropy), x16 being the best quality
DeclareEnum5(TextureAnisotropy, x1, x2, x4, x8, x16);

/// Progressively scaled down versions of the image are produced
/// to preserve image integrity when viewed at smaller scales
/// None - No mipmaps are generated
/// PreGenerated - Mipmaps are generated by the engine
///   Uses higher quality filtering than the gpu
///   Required for cubemaps in order to get perspective correct filtering over face edges
/// GpuGenerated - Mipmaps are generated by the gpu at load time
DeclareEnum3(TextureMipMapping, None, PreGenerated, GpuGenerated);

uint GetPixelSize(TextureFormat::Enum format);

/// Information about a processed image.
class TextureInfo : public ContentComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;

  /// File type extension of the source image.
  String GetFileType();
  String mFileType;

  /// Decompressed pixel format used to process image.
  String GetLoadFormat();
  String mLoadFormat;

  /// Width and height of the image, or of each face if used as a cubemap.
  String GetDimensions();
  String mDimensions;

  /// Total data size on hardware accounting for compression and pre-generated mips if applicable.
  String GetSize();
  String mSize;
};

class ShowPremultipliedAlphaFilter : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Filter(Member* prop, HandleParam instance) override;
};

class ShowGammaCorrectionFilter : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Filter(Member* prop, HandleParam instance) override;
};

/// Configuration for how an image file should be processed for use as a Texture resource.
class TextureBuilder : public BuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // BuilderComponent Interface

  void Serialize(Serializer& stream) override;
  void Initialize(ContentComposition* item) override;
  void Generate(ContentInitializer& initializer) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void Rename(StringParam newName) override;

  // Properties

  /// Name for the Texture resource.
  String Name;
  /// Type of Texture that the image will be used for.
  TextureType::Enum mType;
  /// Block compression method to use if hardware supports it.
  TextureCompression::Enum mCompression;
  /// How to treat uv coordinates outside of [0, 1] along the Texture's width.
  TextureAddressing::Enum mAddressingX;
  /// How to treat uv coordinates outside of [0, 1] along the Texture's height.
  TextureAddressing::Enum mAddressingY;
  /// How samples should be blended under minification/magnification.
  TextureFiltering::Enum mFiltering;
  /// Max ratio of anisotropy that filtering will account for at oblique viewing angles.
  TextureAnisotropy::Enum mAnisotropy;
  /// If downsampled versions of the texture (mip maps) should be generated.
  TextureMipMapping::Enum mMipMapping;
  /// If color data should be stored pre-multiplied by alpha, applied before other operations.
  bool mPremultipliedAlpha;
  /// If color data should be stored in linear color space instead of sRGB color space.
  /// Important for albedo values used in lighting.
  bool mGammaCorrection;

  // Internal

  String GetOutputFile();

  ResourceId mResourceId;
};

//----------------------------------------------------- Height To Normal Builder
//DeclareEnum2(NormalGeneration, AverageRGB, Alpha);
//DeclareEnum3(NormalFilter, n3x3, n5x5, dudv);
//
//class HeightToNormalBuilder : public ContentComponent
//{
//public:
//  ZeroDeclareType(HeightToNormalBuilder);
//  static void InitializeMeta(MetaType* meta);
//  float mBumpiness;
//  NormalGeneration::Type mNormalSource;
//  NormalFilter::Type mNormalFilter;
//  bool mStoreHeightInAlpha;
//  //BuilderComponent Interface
//  void Generate(ContentInitializer& initializer) override;
//  void Serialize(Serializer& stream) override;
//};

} // namespace Zero
