
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif
#include "common/exif.h"
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/canonmn.hpp>
#include <sstream>
#include <cassert>
#include <glib.h>

// inspired by ufraw_exiv2.cc:

static void dt_strlcpy_to_utf8(char *dest, size_t dest_max,
	Exiv2::ExifData::iterator &pos, Exiv2::ExifData& exifData)
{
  std::string str = pos->print(&exifData);
  // std::stringstream ss;
  // (void)Exiv2::ExifTags::printTag(ss, 0x0016, Exiv2::canonIfdId, pos->value(), &exifData);
  // (void)Exiv2::CanonMakerNote::printCsLensType(ss, pos->value(), &exifData);
  // std::ostream &os, uint16_t tag, IfdId ifdId, const Value &value, const ExifData *pExifData=0)
  // str = ss.str();

  char *s = g_locale_to_utf8(str.c_str(), str.length(),
      NULL, NULL, NULL);
  if ( s!=NULL ) {
    g_strlcpy(dest, s, dest_max);
    g_free(s);
  } else {
    g_strlcpy(dest, str.c_str(), dest_max);
  }
}

int dt_exif_read(dt_image_t *img, const char* path)
{
  /* Redirect exiv2 errors to a string buffer */
  // std::ostringstream stderror;
  // std::streambuf *savecerr = std::cerr.rdbuf();
  // std::cerr.rdbuf(stderror.rdbuf());

  try
  {
    Exiv2::Image::AutoPtr image;
    image = Exiv2::ImageFactory::open(path);
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty())
    {
      std::string error(path);
      error += ": no exif data found in the file";
      throw Exiv2::Error(1, error);
    }

    /* List of tag names taken from exiv2's printSummary() in actions.cpp */
    Exiv2::ExifData::iterator pos;
    /* Read shutter time */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime")))
        != exifData.end() ) {
      // dt_strlcpy_to_utf8(uf->conf->shutterText, max_name, pos, exifData);
      img->exif_exposure = pos->toFloat ();
    } else if ( (pos=exifData.findKey(
            Exiv2::ExifKey("Exif.Photo.ShutterSpeedValue")))
        != exifData.end() ) {
      // uf_strlcpy_to_utf8(uf->conf->shutterText, max_name, pos, exifData);
      img->exif_exposure = 1.0/pos->toFloat ();
    }
    /* Read aperture */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.FNumber")))
        != exifData.end() ) {
      img->exif_aperture = pos->toFloat ();
    } else if ( (pos=exifData.findKey(
            Exiv2::ExifKey("Exif.Photo.ApertureValue")))
        != exifData.end() ) {
      img->exif_aperture = pos->toFloat ();
    }
    /* Read ISO speed */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(Exiv2::ExifKey(
              "Exif.CanonSi.ISOSpeed"))) != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Nikon1.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Nikon2.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Nikon3.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(
            Exiv2::ExifKey("Exif.MinoltaCsNew.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(
            Exiv2::ExifKey("Exif.MinoltaCsOld.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat ();
    } else if ( (pos=exifData.findKey(
            Exiv2::ExifKey("Exif.MinoltaCs5D.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat();
    } else if ( (pos=exifData.findKey(Exiv2::ExifKey(
              "Exif.MinoltaCs7D.ISOSpeed")))
        != exifData.end() ) {
      img->exif_iso = pos->toFloat();
    }
    /* Read focal length */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.FocalLength")))
        != exifData.end() ) {
      img->exif_focal_length = pos->toFloat();
    }
#if 0
    /* Read focal length in 35mm equivalent */
    if ( (pos=exifData.findKey(Exiv2::ExifKey(
              "Exif.Photo.FocalLengthIn35mmFilm")))
        != exifData.end() ) {
      img->exif_focal_length = pos->toFloat ();
    }
#endif
    /** read image orientation */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Image.Orientation")))
        != exifData.end() ) {
      const int orient = pos->toLong();
      switch(orient)
      {
        case 1:
          img->orientation = 0 | 0 | 1;
          break;
        case 2:
          img->orientation = 0 | 2 | 1;
          break;
        case 3:
          img->orientation = 0 | 2 | 0;
          break;
        case 4:
          img->orientation = 0 | 0 | 0;
          break;
        case 5:
          img->orientation = 4 | 0 | 0;
          break;
        case 6:
          img->orientation = 4 | 2 | 0;
          break;
        case 7:
          img->orientation = 4 | 2 | 1;
          break;
        case 8:
          img->orientation = 4 | 0 | 1;
          break;
        default:
          img->orientation = 0;
          break;
      }
    }
    /* Read lens name */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Nikon3.Lens")))
        != exifData.end() )
    {
      dt_strlcpy_to_utf8(img->exif_lens, 30, pos, exifData);
    }
    else if (((pos = exifData.findKey(Exiv2::ExifKey("Exif.CanonCs.LensType"))) != exifData.end()) ||
             ((pos = exifData.findKey(Exiv2::ExifKey("Exif.Canon.0x0095")))     != exifData.end()))
    {
      dt_strlcpy_to_utf8(img->exif_lens, 30, pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Minolta.LensID"))) != exifData.end() )
    {
      dt_strlcpy_to_utf8(img->exif_lens, 30, pos, exifData);
    }
    else if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Pentax.LensType"))) != exifData.end() )
    {
      dt_strlcpy_to_utf8(img->exif_lens, 30, pos, exifData);
    }
#if 0
    /* Read flash mode */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.Flash")))
        != exifData.end() ) {
      uf_strlcpy_to_utf8(uf->conf->flashText, max_name, pos, exifData);
    }
    /* Read White Balance Setting */
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.WhiteBalance")))
        != exifData.end() ) {
      uf_strlcpy_to_utf8(uf->conf->whiteBalanceText, max_name, pos, exifData);
    }
#endif

    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Image.Make")))
        != exifData.end() ) {
      dt_strlcpy_to_utf8(img->exif_maker, 32, pos, exifData);
    }
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Image.Model")))
        != exifData.end() ) {
      dt_strlcpy_to_utf8(img->exif_model, 32, pos, exifData);
    }
    if ( (pos=exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal")))
        != exifData.end() ) {
      dt_strlcpy_to_utf8(img->exif_datetime_taken, 20, pos, exifData);
    }

    // std::cerr.rdbuf(savecerr);

    // std::cout << "time c++: " << img->exif_datetime_taken << std::endl;
    // std::cout << "lens c++: " << img->exif_lens << std::endl;
    // std::cout << "lensptr : " << (long int)(img->exif_lens) << std::endl;
    // std::cout << "imgptr  : " << (long int)(img) << std::endl;
    return 0;
  }
  catch (Exiv2::AnyError& e)
  {
    // std::cerr.rdbuf(savecerr);
    std::string s(e.what());
    std::cerr << "[exiv2] " << s << std::endl;
    return 1;
  }
}

int dt_exif_read_blob(uint8_t *buf, const char* path)
{
  try
  {
    Exiv2::Image::AutoPtr image;
    image = Exiv2::ImageFactory::open(path);
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty())
    {
      std::string error(path);
      error += ": no exif data found in ";
      error += path;
      throw Exiv2::Error(1, error);
    }
    exifData["Exif.Image.Orientation"] = uint16_t(1);
    exifData["Exif.Photo.UserComment"]
        = "developed using "PACKAGE_NAME"-"PACKAGE_VERSION;
#if 1//EXIV2_TEST_VERSION(0,17,91)		/* Exiv2 0.18-pre1 */
    Exiv2::Blob blob;
    Exiv2::ExifParser::encode(blob, Exiv2::bigEndian, exifData);
    const int length = blob.size();
    memcpy(buf, "Exif\000\000", 6);
    if(length > 0 && length < 65534)
      memcpy(buf+6, &(blob[0]), length);
#else
    Exiv2::DataBuf buf(exifData.copy());
    const int length = buf.size_;
    memcpy(buf, buf.pData_, buf.size_);
#endif
    return length;
  }
  catch (Exiv2::AnyError& e)
  {
    // std::cerr.rdbuf(savecerr);
    std::string s(e.what());
    std::cerr << "[exiv2] " << s << std::endl;
    return 0;
  }
}
